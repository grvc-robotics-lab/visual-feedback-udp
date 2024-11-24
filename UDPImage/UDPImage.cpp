/*
 * UDP Image
 * 
 * Author: Alejandro Suarez, asuarezfm@us.es
 * Date: January 2014
 * Revision date: August 2014
 * Updated: November 2024
 */

#include "UDPImage.h"


/*
 * Constructor
 */
UDPImage::UDPImage()
{
}


/*
 * Destructor
 */
UDPImage::~UDPImage()
{
	// Close the socket
	close(socketClient);
}


/*
 * Initializes socket.
 *
 * Arguments:
 *		- hostName: string with the IP address of the host where image will be sent ("127.0.0.1" for local test)
 *		- port: port number where image will be sent
 *
 * Returned
 *		- error: zero if no error occurred
 */
int UDPImage::init(char * hostName, int port)
{
	int error = 0;
	
	
	// Open the socket in datagram mode
	cout << "Opening UDP socket for image send..." << endl;
    socketClient = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketClient < 0)
    {
    	cout << "ERROR: could not open socket" << endl;
	   	error = 1;
    }

	// Solve tha name of the server
	if(error == 0)
	{
    	server = gethostbyname(hostName);
    	if (server == NULL)
    	{
    	    cout << "ERROR: could not find host" << endl;
	    	error = 2;
    	}
    }

	// Set the address of the server
	if(error == 0)
	{
    	bzero((char*)&addrServer, sizeof(addrServer));
    	addrServer.sin_family = AF_INET;
    	bcopy((char*)server->h_addr, (char*)&addrServer.sin_addr.s_addr, server->h_length);
    	portNum = port;
    	addrServer.sin_port = htons(portNum);
    }
    
    
    return error;
}


/*
 * Send the image passed as argument throught the socket.
 *
 * Arguments:
 *		- img: OpenCV Mat image to send
 *		- packetLength: maximum length of the packets
 */
int UDPImage::sendImage(const cv::Mat &img, int packetLength)
{
	IMG_HEADER imageHeader;
    int n = 0;
    int k = 0;
    char buffer[256];
	vector<int> parameters = vector<int>(2);
	vector<uchar> encodedImageBuffer;
	uint16_t checksum = 0;
	char terminationStr[16];
	int error = 0;
	
	
	// Set image compression parameters
	parameters[0] = CV_IMWRITE_JPEG_QUALITY;
	parameters[1] = JPEG_QUALITY;
	
	// cout << "Encoding image...\n";
	encodedImageBuffer.clear();
	imencode(".jpg", img, encodedImageBuffer, parameters);
	
	// Compute the checksum
	checksum = computeChecksum(encodedImageBuffer);

	// Set the fields of the structure
	strncpy(imageHeader.headerStr, "JPEG_IMG", 8);
	imageHeader.imgSize = encodedImageBuffer.size();
	imageHeader.packetSize = packetLength;
	imageHeader.checksum = checksum;
	
	// Send the image header
	if(sendto(socketClient, (char*)&imageHeader, sizeof(imageHeader), 0, (struct sockaddr*)&addrServer, sizeof(addrServer)) < 0)
	{
		cout << "ERROR: could not send image header" << endl;
		error = 1;
	}
	
	// Send the encoded image
	if(sendImageInPackets(encodedImageBuffer, packetLength) != 0)
	{
		cout << "ERROR: Could not send the image" << endl;
		error = 2;
	}
	
	
	return error;
}



/*
 * Compute the 16 bit checksum applying XOR operation over image buffer
 */
uint16_t UDPImage::computeChecksum(const vector<uchar> &imgBuffer)
{
	uint16_t checksum = 0;
	uint16_t word = 0;
	int k = 0;
	
	
	if(imgBuffer.size() > 1)
	{
		for(k = 1; k < imgBuffer.size(); k++)
		{
			word = 0;
			word = imgBuffer[k];
			word = word << 8;
			word |= imgBuffer[k-1];
			checksum ^= word;
		}
	}
	
	
	return checksum;
}


/*
 * Send the encoded image contained in imgBuffer using the client socket. The maximum size of the UDP
 * datagram is specified with the third argument.
 */
int UDPImage::sendImageInPackets(const vector<uchar> &imgBuffer, int packetLength)
{
	int error = 0;
	int dataSent = 0;
	int packetNum = 0;
	int n = 0;
	uchar * packet = (uchar*)malloc(packetLength);
	
	
	// Check if error when allocating memory for packet buffer
	if(packet == NULL)
	{
		cout << "ERROR: malloc() returned a null pointer when sending a packet" << endl;
		error = 1;
	}
	else
	{
		do
		{
			// Copy the portion of image buffer into the packet buffer
			for(n = 0; n < packetLength && dataSent < imgBuffer.size(); n++)
			{
				packet[n] = imgBuffer[n + packetNum*packetLength];
				dataSent++;
			}
			packetNum++;
			
			// Send the packet
			if(sendto(socketClient, packet, n, 0, (struct sockaddr*)&addrServer, sizeof(addrServer)) < 0)
				error = 1;
			
			// Waits 5 ms
			usleep(5000);
		
		} while(dataSent < imgBuffer.size() && error == 0);
		
		// Memory release
		free(packet);
		packet = NULL;
	}
	
	return error;
}


