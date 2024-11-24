/*
 * Visual Feedback using UDP socket and OpenCV imencode function
 * 
 * Author: Alejandro Suarez, asuarezfm@us.es
 * Date: May 2017
 * Updated: November 2024
 */

// Standard library
#include <iostream>
#include <stdio.h>
#include <pthread.h>

// OpenCV library
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Specific library
#include "../UDPImage/UDPImage.h"


// Namespaces
using namespace std;
using namespace cv;

typedef struct thread_arg
{
	bool endFlag;			// Program execution ends if true
	bool displayFlag;		// Display captured frames on screen if true
	bool saveFlag;			// Save data on files if true
} THREAD_ARGS;


/* Thread function for reading commands from standard input */
static void* keyboardThreadFunction(void * args)
{
	THREAD_ARGS * threadArg = (THREAD_ARGS*)args;
	string command;
	
	
	cout << "Keyboard thread started" << endl;
	do
	{
		cout << ">> ";
		cin >> command;
		
		if(command == "quit" || command == "exit")
			threadArg->endFlag = true;
		else if(command == "display_on")
			threadArg->displayFlag = true;
		else if(command == "display_off")
			threadArg->displayFlag = false;
		else if(command == "start")
			threadArg->saveFlag = true;
		else if(command == "stop")
			threadArg->saveFlag = false;
		else if(command == "help")
		{
			cout << "Available commands:" << endl;
			cout << "quit | exit: terminate program execution" << endl;
			cout << "display_{on | off}: activate/deactivate video display" << endl;
			// cout << "start | stop: start/stop saving captured frames" << endl;
			cout << "help: show this help" << endl;
		}
		else
			cout << "ERROR: Command not recognized. Try help." << endl;
		
	} while(threadArg->endFlag == false);
	
	
	cout << "Keyboard thread terminated" << endl;
	
	
	return 0;
}


int main(int argc, char *argv[])
{
	int error = 0;
	
	
	cout << "Visual Feedback Program" << endl;
	cout << "Author: Alejandro Suarez, asuarezfm@us.es" << endl;
	cout << "Date: 8 May 2017" << endl;
	cout << "Updated: November 2024" << endl;
	cout << "Organization: University of Seville" << endl;
	cout << "------------------------------------------" << endl << endl;
	
	if(argc != 7)
	{
		cout << "ERROR: invalid number of arguments. See README.txt file." << endl;
		cout << "Input arguments are:" << endl;
		cout << "\t- UDP_ImageServer_IP" << endl;
		cout << "\t- UDP_ImageServer_Port" << endl;
		cout << "\t- CameraIndex" << endl;
		cout << "\t- FrameWidth" << endl;
		cout << "\t- FrameHeight" << endl;
		cout << "\t- Image feedback period in [ms]" << endl;
	}
	else
	{
		VideoCapture capture(atoi(argv[3]));
		UDPImage * udpImage = new UDPImage();
		Mat img;
		THREAD_ARGS threadArgs;
		pthread_t keyboardThread;
		struct timeval tini;
		struct timeval tend;
		double t = 0;
		
		// Set camera properties
		capture.set(CV_CAP_PROP_FRAME_WIDTH, atoi(argv[4]));
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, atoi(argv[5])); 
		capture.set(CV_CAP_PROP_FPS, 30);
		
		// Init UDP socket interface
		udpImage->init(argv[1], atoi(argv[2]));

		// Create thread
		threadArgs.endFlag = false;
		threadArgs.displayFlag = false;
		threadArgs.saveFlag = false;
		if(pthread_create(&keyboardThread, NULL, &keyboardThreadFunction, (void*)&threadArgs))
		{
			cout << "ERROR: Could not create keyboard thread!" << endl;
			error = 1;
		}
		
		// Init time stamp
		gettimeofday(&tini, NULL);
		
		while(error == 0 && threadArgs.endFlag == false)
		{
			capture >> img;
			imshow("Display", img);
			cvWaitKey(5);
			
			// Update time stamp
			gettimeofday(&tend, NULL);
			t = (tend.tv_sec - tini.tv_sec) + 1e-6*(tend.tv_usec - tini.tv_usec);
			if(t > 0.001*atoi(argv[6]))
			{
				// Reset timer
				gettimeofday(&tini, NULL);
				
				// Send image
				udpImage->sendImage(img, 8192);
			}
		}
		
		capture.release();
		udpImage->~UDPImage();
	}

	return error;
}

