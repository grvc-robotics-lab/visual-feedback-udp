/*
 * UDP Encoded Image Receiver
 *
 * Author: Alejandro Suárez Fernández-Miranda
 * Date: August 2014
 * Organization: University of Sevilla, GRVC
 */

// Standard library
#include <iostream>
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// Open CV library
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


// Constant definition
#define MAX_CONNECTION_QUEUE_SIZE	32
#define BUFFER_LENGTH				65536
#define WINDOW_NAME					"Camera Display"


// Structure definition
typedef struct
{
	char headerStr[8];		// "JPEG_IMG" character sequence
	int imgSize;			// Size in bytes of the image
	int packetSize;			// Size in bytes of the packets
	uint16_t checksum;		// 16-bit XOR checksum applied to the bytes of the encoded image
} __attribute__((packed)) DATA_PACKET_ENCODED_IMAGE;


// Funcion declaration
uint16_t computeChecksum(const std::vector<uchar> &imgBuffer);

using namespace std;
using namespace cv;


void error(const string &msg)
{
    cout << msg;
    exit(1);
}


int main(int argc, char *argv[])
{
	DATA_PACKET_ENCODED_IMAGE dataPacketEncodedImage;
	int socketServer = -1;
	int portNum = -1;
	struct sockaddr_in addrServer;
	struct sockaddr_in addrClient;
	socklen_t addrLength;
	char buffer[BUFFER_LENGTH];
	vector<uchar> vectorImg;
	int imgLength = 0;
	int dataReceived = 0;
	int receivedBytes = 0;
	int key = 0;
	int error = 0;


	cout << "UDP Image Receiver" << endl;
	cout << "Author: Alejandro Suarez Fernandez-Miranda" << endl;
	cout << "Date: August 2014" << endl;
	cout << "Organization: University of Seville, GRVC" << endl;
	cout << "------------------------------------------" << endl;
	
	 // Check if the arguments are correct
	if (argc != 2)
	{
	 	cout << "ERROR: invalid number of arguments" << endl;
	 	cout << "Ussage: " << argv[0] << " UDP_Listening_Port" << endl;
	 	error = 1;
	}

	 // Open the socket in stream mode
	 if(error == 0)
	 {
	 	cout << "Opening socket...";
     	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
     	if (socketServer < 0)
     	{
     	   cout << endl << "ERROR: could not open socket" << endl;
     	   error = 1;
     	}
     	else
     		cout << "OK" << endl;
     }

	// Set listenning address and port for server
	if(error == 0)
	{
    	bzero((char*)&addrServer, sizeof(addrServer));
    	portNum = atoi(argv[1]);
    	addrServer.sin_family = AF_INET;
    	addrServer.sin_addr.s_addr = INADDR_ANY;
    	addrServer.sin_port = htons(portNum);

	 	// Associates the address to the socket
	 	cout << "Binding socket...";
     	if (bind(socketServer, (struct sockaddr*)&addrServer, sizeof(addrServer)) < 0)
     	{
			cout << endl << "ERROR: could not associate address to socket" << endl;
			error = 2;
		}
		else
			cout << "OK" << endl;
	}

	/************************************************************************/
	cout << "Waiting data..." << endl;
	namedWindow(WINDOW_NAME, 1);
	moveWindow(WINDOW_NAME, 100, 100);
	while(error == 0 && key != 'q')
	{
		dataReceived = recvfrom(socketServer, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&addrServer, &addrLength);
		if (dataReceived > 0)
		{
			if(dataReceived == sizeof(DATA_PACKET_ENCODED_IMAGE))
			{
				bcopy(buffer, &dataPacketEncodedImage, sizeof(DATA_PACKET_ENCODED_IMAGE));
				char headerStr[9];
				strncpy(headerStr, dataPacketEncodedImage.headerStr, 8);
				headerStr[8] = '\0';
				cout << "Header string: " << headerStr << endl;
				cout << "Image size in bytes: " << dataPacketEncodedImage.imgSize << endl;
				cout << "Packet size in bytes: " << dataPacketEncodedImage.packetSize << endl;
				cout << "Received checksum: " << dataPacketEncodedImage.checksum << endl;
				
				// Wait to receive all bytes of the encoded image
				vectorImg.clear();
				receivedBytes = 0;
				while (receivedBytes < dataPacketEncodedImage.imgSize)
				{
					dataReceived = recvfrom(socketServer, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&addrServer, &addrLength);
					receivedBytes += dataReceived;
					for(int k = 0; k < dataReceived; k++)
						vectorImg.push_back(buffer[k]);
				}
				
				if(vectorImg.size() != dataPacketEncodedImage.imgSize)
					cout << endl << "Image size does not match expected size" << endl;
				else
				{
					if(computeChecksum(vectorImg) == dataPacketEncodedImage.checksum)
					{
						Mat img = imdecode(vectorImg, IMREAD_COLOR);
						imshow(WINDOW_NAME, img);
						key = waitKey(2);
					}
					else
						cout << "ERROR: checksum is not correct" << endl;
				}
				
				cout << "----------------" << endl;
			}
		}
		
		key = waitKey(2);
	}
	/************************************************************************/

	
	// Close the socket
	close(socketServer);

     return 0; 
}


/*
 * Compute the 16 bit checksum applying XOR operation over image buffer
 */
uint16_t computeChecksum(const vector<uchar> &imgBuffer)
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




