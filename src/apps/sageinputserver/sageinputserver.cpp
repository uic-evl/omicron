/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#include <omicron.h>
#include <vector>

#include <time.h>
using namespace omicron;

#ifdef WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
#define itoa _itoa
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Based on Winsock UDP Server Example:
// http://msdn.microsoft.com/en-us/library/ms740148
class NetClient
{
private:
	WSADATA wsaData;
	SOCKET SendSocket;
	sockaddr_in RecvAddr;
	int Port;
	//char SendBuf[1024];
	int BufLen;

public:
	NetClient::NetClient( const char* address, int port )
	{
		// Create a socket for sending data
		SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(port);
		RecvAddr.sin_addr.s_addr = inet_addr(address);
		printf("NetClient %s:%i created...\n", address, port);
	}// CTOR

	void NetClient::sendEvent(char* eventPacket, int length)
	{
		// Send a datagram to the receiver
		sendto(SendSocket, 
			eventPacket, 
			length, 
			0, 
			(SOCKADDR*) &RecvAddr, 
			sizeof(RecvAddr));
	}// SendEvent
};

#define OI_WRITEBUF(type, buf, offset, val) *((type*)&buf[offset]) = val; offset += sizeof(type);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GESTURE TYPES
enum GESTURE_TYPE
{
    GESTURE_NULL,           // nothing... default... unrecognizable gesture
    GESTURE_SINGLE_TOUCH,   // 1 finger at a time
    GESTURE_DOUBLE_CLICK,   // 1 finger, twice in a row
    GESTURE_BIG_TOUCH,      // 1 big blob
    GESTURE_ZOOM,           // 2 finger pinch gesture
    GESTURE_MULTI_TOUCH_HOLD,  // 4 or more fingers at a time in one place
    GESTURE_MULTI_TOUCH_SWIPE, // 4 or more fingers at a time moving
};

// 0 for no sage... for testing only
#define USE_SAGE 1


/*****************************************************************************/
// globals
/*****************************************************************************/

#define round(fp) (int)((fp) >= 0 ? (fp) + 0.5 : (fp) - 0.5)


/////////  sage communication stuff  /////////
#define DIM_PORT 20005
char sageHost[100];
char pqServer[100];
char myIP[16];
char msg[1024]; // will hold queued up messages
int sock;

int touchResW = 1600;
int touchResH = 1200;

bool sageConnected = false;

#ifndef SOCKET_ERROR
#define SOCKET_ERROR    -1
#endif

class SAGETouchServer{
public:
	void connectToSage();
	void handleEvent(const Event&);
	void queueMessage(char*);
	void sendToSage();
}; //class

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SAGETouchServer::connectToSage(){
	if (!USE_SAGE) 
		return;
	
    // socket stuff
    int error;
    struct sockaddr_in peer;
    struct hostent *myInfo;
    char myHostname[128];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(DIM_PORT);
    peer.sin_addr.s_addr = inet_addr(sageHost);

    sock = socket(AF_INET, SOCK_STREAM, 0);
	
    if (sock < 0)
    {
		printf("\nERROR: Couldn't open socket on port %d!\n", DIM_PORT);
		exit(0);
    } 

    // get my IP for the message header to SAGE
    gethostname(myHostname, 128);
    myInfo = gethostbyname(myHostname);
    sprintf(myIP, "%s", inet_ntoa(*(struct in_addr*) myInfo->h_addr));

    // ignore SIGPIPE
    //int set = 1;
    //setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    //setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&set, sizeof(int));
    //setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (void *)&set, sizeof(int));

	
    while ((error = connect(sock, (struct sockaddr*)&peer, sizeof(peer))) != 0)
    {
		// close the failed one...
		closesocket(sock);  
		sageConnected = false;

		printf("\nTrying to reconnect to sage on %s... failed, socket error code: %d", sageHost, errno);
		Sleep(1000);

		// recreate the socket...
		sock = socket(AF_INET, SOCK_STREAM, 0);

		// ignore SIGPIPE
		//int set = 1;
		//setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
		//setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&set, sizeof(int));		
		//setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (void *)&set, sizeof(int));
    }
	
    Sleep(1000);
    printf("\nConnected to sage on: %s\n", sageHost);
    sageConnected = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SAGETouchServer::handleEvent(const Event& evt){

	if( evt.getServiceType() == Service::Pointer ){
		char msgData[256];
		int id = evt.getSourceId();
		float xPos = evt.getPosition().x();
		float yPos = 1.0 - evt.getPosition().y(); // Flip y position for SAGE
		int eventType = evt.getType();
		bool validEvent = false;

		// Single touch
		if( eventType == Event::Down || eventType == Event::Move || eventType == Event::Up )
		{
			// Remap eventtype to match SAGE Touch lifepoint
			switch( eventType )
			{
				case Event::Down: eventType = 1; break; // Begin
				case Event::Move: eventType = 2; break; // Middle
				case Event::Up: eventType = 3; break; // End
			}

			sprintf(msgData, "%s:pqlabs%d pqlabs %d %f %f %d\n", 
					myIP, id, GESTURE_SINGLE_TOUCH, xPos, yPos, eventType);
			validEvent = true;
		}

		if( validEvent ){
			//printf(msgData);
			queueMessage(msgData);
			sendToSage();
		}
	}
}

void SAGETouchServer::queueMessage(char *newMsg) 
{
    if (!USE_SAGE) return;

    if (strlen(msg) == 0) {
	sprintf(msg, "%s", newMsg);
     }
    else {
	sprintf(msg, "%s%s", msg, newMsg);
    }
}

void SAGETouchServer::sendToSage()
{
    if (!USE_SAGE || strlen(msg) == 0) return;
    
    if (send(sock, msg, strlen(msg), 0) == SOCKET_ERROR)
    {
	printf("\nDisconnected from sage... reconnecting\n");

	connectToSage();   // reconnect automatically

    }
	    
    strcpy(msg,"\0");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main(int argc, char** argv)
{
	SAGETouchServer app;

	// Read config file name from command line or use default one.
	const char* cfgName = "sageTouch.cfg";
	if(argc == 2) cfgName = argv[1];

	Config* cfg = new Config(cfgName);

	DataManager* dm = DataManager::getInstance();
	// Add a default filesystem data source using current work dir.
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	ServiceManager* sm = new ServiceManager();
	sm->setupAndStart(cfg);

	Setting& stRoot = cfg->getRootSetting()["config"];

	strcpy( sageHost, cfg->getStringValue("sageHostIP", stRoot, "").c_str() );

	app.connectToSage();
	
	float delay = -0.01f; // Seconds to delay sending events (<= 0 disables delay)
	bool printOutput = false;

	//omsg("OInputServer: Starting to listen for clients...");
	while(true)
	{
		// TODO: Use StopWatch
		if( delay > 0.0 )
			Sleep(1000.0*delay); // Delay sending of data out

		sm->poll();

		// Start listening for clients (non-blocking)
		//app.startListening();

		// Get events
		int av = sm->getAvailableEvents();
		if(av != 0)
		{
			// TODO: Instead of copying the event list, we can lock the main one.
			Event evts[OMICRON_MAX_EVENTS];
			sm->getEvents(evts, OMICRON_MAX_EVENTS);
			for( int evtNum = 0; evtNum < av; evtNum++)
			{
				app.handleEvent(evts[evtNum]);
			}
			if( printOutput )
				printf("------------------------------------------------------------------------------\n");
		}
	}

	sm->stop();
	delete sm;
	delete cfg;
	delete dm;
}
