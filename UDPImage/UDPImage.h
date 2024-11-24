/*
 * UDPImage.h
 *
 * Class for JPEG encoded image send through UDP socket
 * 
 * Author: Alejandro Suarez, asuarezfm@us.es
 * Date: January 2014
 * Revision date: August 2014
 * Updated: November 2024
 */


#ifndef UDPIMAGE_H_
#define UDPIMAGE_H_

// Standard library
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// LINUX library
#include <sys/time.h>

// OpenCV library
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Specific library


// Constants
#define IMAGE_BUFFER_LENGTH			10240
#define JPEG_QUALITY				25


// Namespaces
using namespace std;
using namespace cv;


// Structure definition
typedef struct imgHeader
{
	char headerStr[8];		// "JPEG_IMG" character sequence
	int imgSize;			// Size in bytes of the image
	int packetSize;			// Size in bytes of the packets
	uint16_t checksum;		// 16-bit XOR checksum applied to the bytes of the encoded image
} IMG_HEADER;


class UDPImage
{
public:
	/**************** PUBLIC VARIABLES *****************/
	

	/***************** PUBLIC METHODS *****************/

	/* Constructor */
	UDPImage();

	/* Destructor */
	~UDPImage();

	/* Initializes socket */
	int init(char * hostName, int port);

	/* Send the image passed as argument throught the socket */
	int sendImage(const cv::Mat &img, int packetLength);



private:
	/***************** PRIVATE VARIABLES *****************/

    struct sockaddr_in addrServer;
    struct hostent * server;
    int socketClient;
    int portNum;


	/***************** PRIVATE METHODS *****************/

	/* Compute the 16 bit checksum applying XOR operation over image buffer */
	uint16_t computeChecksum(const vector<uchar> &imgBuffer);
	
	/* Send the encoded image contained in imgBuffer using the client socket. The maximum size of the UDP
	 * datagram is specified with the third argument. */
	int sendImageInPackets(const vector<uchar> &imgBuffer, int packetLength);


};

#endif
