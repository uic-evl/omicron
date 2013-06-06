/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *  Alessandro Febretti		febret@gmail.com
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
#include "omicron/LegacyNetService.h"
using namespace omicron;

#include<stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LegacyNetService::LegacyNetService(){
	touchTimeout = 0.25; // seconds
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::setup(Setting& settings)
{
	if(settings.exists("serverIP"))
	{
		serverAddress = (const char*)settings["serverIP"];
	}
	if(settings.exists("msgPort"))
	{
		msgPort = settings["msgPort"];
	}
	if(settings.exists("dataPort"))
	{
		dataPort = settings["dataPort"];
	}
	if(settings.exists("screenX"))
	{
		screenX = settings["screenX"];
		printf("LegacyNetService: screenX set to %d\n", screenX);
	}
	if(settings.exists("screenY"))
	{
		screenY = settings["screenY"];
		printf("LegacyNetService: screenY set to %d\n", screenY);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::initialize() 
{
	mysInstance = this;
#ifdef OMICRON_OS_WIN     
	/*
	* Based on MSDN Winsock examples:
	* http://msdn.microsoft.com/en-us/library/ms738566(VS.85).aspx
	*/
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("LegacyNetService: WSAStartup failed: %d\n", iResult);
		return;
	} else {
		printf("LegacyNetService: Winsock initialized \n");
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	char charBuf[32];
	iResult = getaddrinfo(serverAddress, itoa(msgPort,charBuf,10), &hints, &result);
	if (iResult != 0) {
		printf("LegacyNetService: getaddrinfo failed: %d\n", iResult);
		WSACleanup();
	} else {
		printf("LegacyNetService: Client set to connect to address %s on port %d\n", serverAddress, msgPort);
	}

	// Create connection socket
	ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr=result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
		ptr->ai_protocol);

	// Connect to server.
	iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	if (ConnectSocket == INVALID_SOCKET) {
		printf("LegacyNetService: Error at socket(): %ld - Failed to connect to server\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("LegacyNetService: Unable to connect to server!\n");
		WSACleanup();
		return;
	} else {
		printf("LegacyNetService: Connected to server!\n");
	}
#else
	/*
	* Based on Beej's Guide to Network Programming:
	* http://beej.us/guide/bgnet/output/html/multipage/index.html
	*/
	printf("LegacyNetService: Initializing using linux\n");

	struct addrinfo hints, *res;

	// First, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Get the server address
	char charBuf[32];
	sprintf(charBuf,"%d", msgPort);
	iResult = getaddrinfo(serverAddress, charBuf, &hints, &res);

	// Generate the socket
	ConnectSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// Connect to server
	int result = connect(ConnectSocket, res->ai_addr, res->ai_addrlen);

	if (result == -1) {
		printf("LegacyNetService: Unable to connect to server '%s' on port '%s': %s\n", serverAddress, msgPort, strerror(errno));
		return;
	} else {
		printf("LegacyNetService: Connected to server '%s' on port '%s'!\n", serverAddress, msgPort);
	}
#endif
	initHandshake();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initializes handshake and tells server which port to start streaming data.
void LegacyNetService::initHandshake() 
{
	// Send handshake
	char sendbuf[50];

	sendbuf[0] = '\0';

	sprintf( sendbuf, "omicron_legacy_data_on,%d", dataPort );

	printf("LegacyNetService: Sending handshake: '%s'\n", sendbuf);

	iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);


#ifndef OMICRON_OS_WIN     
	if (iResult == -1) {
		printf("LegacyNetService: Send failed: %s\n", strerror(errno));
		return;
	}
#endif

#ifdef OMICRON_OS_WIN     
	if (iResult == SOCKET_ERROR) {
		printf("LegacyNetService: Send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	//printf("Bytes Sent: %ld\n", iResult);

	sockaddr_in RecvAddr;

	SenderAddrSize = sizeof(SenderAddr);

	// Create a UDP receiver socket to receive datagrams
	// 
	RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//-----------------------------------------------
	// Bind the socket to any address and the specified port.
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(dataPort);
	RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(RecvSocket, (SOCKADDR *) &RecvAddr, sizeof(RecvAddr));
#else
	/*
	* Based on Beej's Guide to Network Programming:
	* http://beej.us/guide/bgnet/output/html/multipage/index.html

	*/
	struct addrinfo hints, *res;

	// First, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	// Get the server address
	char charBuf[256];
	sprintf(charBuf,"%d", dataPort);
	getaddrinfo(NULL, charBuf, &hints, &res);

	// Generate the socket
	RecvSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	iResult = bind(RecvSocket, res->ai_addr, res->ai_addrlen );
	if (iResult == -1) {
		printf("LegacyNetService: Failed to bind socket on port %s: %s\n", dataPort, strerror(errno));
		return;
	}
#endif
	readyToReceive = true;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::poll()
{
	//-----------------------------------------------
	// Call the recvfrom function to receive datagrams
	// on the bound socket.
	//printf("About to check for data\n");
	if( readyToReceive ){
		int result;

		// Create a set of fd_set to store sockets
		fd_set ReadFDs, WriteFDs, ExceptFDs;

		// Set collections to null
		FD_ZERO(&ReadFDs);
		FD_ZERO(&WriteFDs);
		FD_ZERO(&ExceptFDs);

		FD_SET(RecvSocket, &ReadFDs);
		FD_SET(RecvSocket, &ExceptFDs);

		timeout.tv_sec  = 0;
		timeout.tv_usec = 0;

		do
		{
			// Check if UDP socket has data waiting to be read before
			// socket blocks to attempt to read.
			result = select(RecvSocket+1, &ReadFDs, &WriteFDs, &ExceptFDs, &timeout);
			// Possible return values:
			// -1: error occurred
			// 0: timed out
			// > 0: data ready to be read
			// if -1 WSAGetLastError() can return error code (Windows only)
			if( result > 0 ) parseDGram(result);
		} while(result > 0);
	}
	
	//-----------------------------------------------
	// Check touchlist for old touches (haven't been updated recently) and remove them
	//Event* evt;
	static float lastt;
	float curt = (float)((double)clock() / CLOCKS_PER_SEC);
			
	std::map<int, NetTouches>::iterator p;
	
	//printf("------------------\n");
	swaplist.clear();

	bool experimentalTouchGestures = false;

	for(p = touchlist.begin(); p != touchlist.end(); p++) {
		
		NetTouches touch = p->second;
		float timestamp = touch.timestamp;

		//printf("LegacyNetService: Current time %f Current touch timestamp: %f \n", curt, timestamp);
		//printf("LegacyNetService: Touch timeout %f \n", touchTimeout);
		if( curt - timestamp > touchTimeout ){ //if( curTime > ts ){
			mysInstance->lockEvents();
			// Touch will be removed from touchlist - send touch up event
			Event* newEvt = mysInstance->writeHead();
			newEvt->reset(Event::Up, Service::Pointer, touch.ID);
			newEvt->setPosition(touch.xPos * (float)screenX, touch.yPos * (float)screenY);

			newEvt->setExtraDataType(Event::ExtraDataFloatArray);
			newEvt->setExtraDataFloat(0, touch.xWidth * (float)screenX);
			newEvt->setExtraDataFloat(1, touch.yWidth * (float)screenY);

			mysInstance->unlockEvents();
			printf("Touch ID %d removed at (%f, %f)\n", touch.ID, touch.xPos, touch.yPos );
			swaplist.erase( touch.ID );
		} else if( experimentalTouchGestures ) {
			// Compare the still active touch with other active touches for gesture recognition
			// This is all hardcoded here - FIX to more elaborate system when time permits
			std::map<int, NetTouches>::iterator swapIt;
			for( swapIt = swaplist.begin(); swapIt != swaplist.end(); swapIt++ ){
				NetTouches swapTouch = swapIt->second;
				float timestamp = swapTouch.timestamp;
				int touchID1 = touch.ID;
				int touchID2 = swapTouch.ID;
				float x1 = touch.xPos * 8160;
				float y1 = touch.yPos * 2304;
				float x2 = swapTouch.xPos * 8160;
				float y2 = swapTouch.yPos * 2304;
				
				//distanceToTarget = sqrt( sq(abs( DESTINATION[X] - POSITION[X] )) + sq(abs( DESTINATION[Y] - POSITION[Y] )) );

				float distance = sqrt( (abs( x2 - x1 ) * abs( x2 - x1 )) + (abs( y2 - y1 ) * abs( y2 - y1 )) );
				float midpointX = (x2 + x1)/2.0f;
				float midpointY = (y2 + y1)/2.0f;
				float minZoomDistance = 30; // Size in pixels (This probably should be a screen ratio later on)
				float maxZoomDistance = 2000; // Max distance in fingers for a zoom gesture
				//printf("Dist between touch ID %d and %d : %f \n", touchID1, touchID2, distance );

				if(curt - lastt <= 0.05f ){
					continue;
				}
				
				if( distance <= 2000 && distance >= 30 && touchID1 != touchID2 ){
					if( touch.gestureType != Event::Split && swapTouch.gestureType != Event::Split ){
						// New split gesture - touch 1 ID becomes new gesture ID
						touch.gestureType = Event::Split;
						touch.x1 = x1;
						touch.y1 = y1;
						touch.x2 = x2;
						touch.y2 = y2;
						touch.initDistance = distance;

						// Generate event
						Vector3f pt1(
								swapTouch.xPos * (float)screenX,
								swapTouch.yPos * (float)screenY,
								0);
						Vector3f pt2(
								touch.xPos * (float)screenX,
								touch.yPos * (float)screenY,
								0);

						mysInstance->lockEvents();

						Event* newEvt = mysInstance->writeHead();
						newEvt->reset(Event::Split, Service::Pointer, touch.ID);
						newEvt->setPosition(touch.xPos * (float)screenX, touch.yPos * (float)screenY);

						newEvt->setExtraDataType(Event::ExtraDataFloatArray);
						newEvt->setExtraDataFloat(0, swapTouch.xPos * (float)screenX);
						newEvt->setExtraDataFloat(1, swapTouch.yPos * (float)screenY);
						newEvt->setExtraDataFloat(2, touch.xPos * (float)screenX);
						newEvt->setExtraDataFloat(3, touch.yPos * (float)screenY);
						newEvt->setPosition((pt1 + pt2) / 2);
						newEvt->setExtraDataFloat(4, 0); // delta distance
						newEvt->setExtraDataFloat(5, 0); // delta ratio
						mysInstance->unlockEvents();

						//printf("LegacyNetService: New split gesture ID %d distance: %d \n", touch.ID, 0);
					} else if( (touch.gestureType != Event::Split || swapTouch.gestureType != Event::Split) ){
						NetTouches gesture, other;
						if( touch.gestureType == Event::Split ){
							gesture = touch;
							other = swapTouch;
						} else {
							gesture = swapTouch;
							other = swapTouch;
						}

						// Ignore pairs with same ID or no change in distance
						if( gesture.initDistance == distance || gesture.ID == other.ID)
							continue;
						lastt = curt;
						printf("LegacyNetService: Existing split ID %d %d deltaDist: %f \n", gesture.ID, other.ID, gesture.initDistance/distance );

						// Generate event
						Vector3f pt1(
								swapTouch.xPos * (float)screenX,
								swapTouch.yPos * (float)screenY,
								0);
						Vector3f pt2(
								touch.xPos * (float)screenX,
								touch.yPos * (float)screenY,
								0);

						mysInstance->lockEvents();

						Event* newEvt = mysInstance->writeHead();
						newEvt->reset(Event::Split, Service::Pointer, gesture.ID);
						newEvt->setPosition(gesture.xPos * (float)screenX, gesture.yPos * (float)screenY);

						newEvt->setExtraDataType(Event::ExtraDataFloatArray);
						newEvt->setExtraDataFloat(0, swapTouch.xPos * (float)screenX);
						newEvt->setExtraDataFloat(1, swapTouch.yPos * (float)screenY);
						newEvt->setExtraDataFloat(2, touch.xPos * (float)screenX);
						newEvt->setExtraDataFloat(3, touch.yPos * (float)screenY);
						newEvt->setPosition((pt1 + pt2) / 2);
						newEvt->setExtraDataFloat(4, gesture.initDistance / distance ); // delta distance
						newEvt->setExtraDataFloat(5, distance); // delta ratio
						mysInstance->unlockEvents();
					}
				}

			}

			swaplist[touch.ID] = touch; // Copy active touches			
		} else {
			swaplist[touch.ID] = touch; // Copy active touches
		}

	}
	touchlist = swaplist;
	
	//printf("------------------\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::dispose() 
{
	// Close the socket when finished receiving datagrams
	printf("LegacyNetService: Finished receiving. Closing socket.\n");
#ifdef OMICRON_OS_WIN     
	iResult = closesocket(RecvSocket);
	if (iResult == SOCKET_ERROR) {
		printf("LegacyNetService: Closesocket failed with error %d\n", WSAGetLastError());
		return;
	}
	WSACleanup();
#else
	iResult = close(RecvSocket);
	if (iResult == -1) {
		printf("LegacyNetService: Closesocket failed with error %d\n", strerror(errno));
		return;
	}
#endif	
    printf("LegacyNetService: Shutting down.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::parseDGram(int result)
{
    
#ifdef OMICRON_OS_WIN     
	result = recvfrom(RecvSocket, 
		recvbuf,
		DEFAULT_BUFLEN-1,
		0, // If non-zero, socket is non-blocking
		(SOCKADDR *)&SenderAddr, 
		&SenderAddrSize);
#else
    socklen_t addr_len;
	struct sockaddr_storage their_addr;
	int numbytes;
	char s[INET6_ADDRSTRLEN];

	addr_len = sizeof their_addr;
	numbytes = recvfrom(RecvSocket,
		recvbuf,
		DEFAULT_BUFLEN-1,
		0,
		(struct sockaddr *)&their_addr,
		&addr_len);

	struct sockaddr *sa = (struct sockaddr *)&their_addr;
	const void *inAddr;
	if (sa->sa_family == AF_INET){
		inAddr = &(((struct sockaddr_in*)sa)->sin_addr);
	} else {
		inAddr = &(((struct sockaddr_in6*)sa)->sin6_addr);
	}

	//printf("listener: got packet from %s\n",
	//    inet_ntop(their_addr.ss_family,
	//    inAddr,
	//    s, sizeof s));
	//printf("listener: packet is %d bytes long\n", numbytes);
	recvbuf[numbytes] = '\0';
	//printf("listener: packet contains \"%s\"\n", recvbuf);
	result = numbytes;
#endif
	if( result > 0 ){
		int msgLen = result - 1;
		char* message = new char[msgLen];
		std::string msgStr = recvbuf;

		// Parse message out of datagram
		int lastIndex = 0;
		int inputType = -1;
		float params[32];
		int currentParam = 0;

						
		// Controller enum from DirectXInputService.h
		int Xbox360 = 0;
		int PS3 = 1;
		int Wiimote = 2;
		int Wii_Nunchuk = 3;
		int Wii_MotionPlus = 4;


		for(int i = 0; i < msgLen; i++ ){
			if( msgStr[i] == ':' ){
				inputType = atoi( msgStr.substr(lastIndex,i).c_str() );
				lastIndex = i + 1;
			}
			if( msgStr[i] == ',' ){
				params[currentParam] = atof( msgStr.substr(lastIndex,i).c_str() );
				lastIndex = i + 1;
				currentParam++;
			}
			message[i] = recvbuf[i];
			message[i+1] = '\0';
		}// for

		// Append last parameter
		params[currentParam] = atof( msgStr.substr(lastIndex,msgLen).c_str() );

		mysInstance->lockEvents();
		Event* evt;
		//printf("New Time %d \n", curTime );
		switch(inputType){
			case(Service::Generic): // Generic
				evt = mysInstance->writeHead();
				evt->reset(Event::Update, Service::Generic, (int)(params[0] + 0.5));
				break;
			case(Service::Mocap): // MoCap
				// Note: This uses an updated MoCap datagram (additional user field for Kinect)
				// 'MocapSevice:JointID,UserID,xPos,yPos,zPos,xRot,yRot,zRot,wRot '
				evt = mysInstance->writeHead();
				evt->reset(Event::Update, Service::Mocap, (int)(params[0] + 0.5));

				evt->setExtraDataType(Event::ExtraDataIntArray);
				evt->setExtraDataInt(1, params[1] );

				evt->setPosition(params[2], params[3], params[4]);

				evt->setOrientation(params[8], params[5], params[6], params[7]);

				break;
			case(Service::Pointer): // Touch (points only not gestures)
				evt = mysInstance->writeHead();
				
				NetTouches touch;
				touch.ID = (int)(params[1]);
				touch.xPos = params[2];
				touch.yPos = params[3];
				touch.xWidth = params[4];
				touch.yWidth = params[5];
				params[6] = (float)((double)clock() / CLOCKS_PER_SEC); // Set the timestamp
				touch.timestamp = params[6];
				
				//printf("New Time set %d \n", curTime );
				///printf("New Time param %d \n", (int)params[6] );
				if( (int)(params[0]) == Event::Down && touchlist.count(touch.ID) == 0 ){
					evt->reset(Event::Down, Service::Pointer, touch.ID);
					touchlist[touch.ID] = touch;
					//pair<map<int,float*>::iterator,bool> ret
					//ret = touchlist.insert (pair<int,float*>(params[1],params) );
					//printf("LegacyNetService: Touch ID %d - DOWN\n", touch.ID);
				}
				else if( (int)(params[0]) == Event::Move ){
					//if( touchlist.count(touch.ID) > 0 ){
						//printf("LegacyNetService: Touchlist ID %d count: %d \n", touch.ID, touchlist.count(touch.ID));
						evt->reset(Event::Move, Service::Pointer, touch.ID);
						touchlist[touch.ID] = touch;
						//printf("LegacyNetService: Touch ID %d - MOVE\n", touch.ID);
					//}
				}
				else if( (int)(params[0]) == Event::Up ){
					evt->reset(Event::Up, Service::Pointer, touch.ID);
					touchlist.erase( touch.ID );
					//printf("LegacyNetService: Touch ID %d - UP\n", touch.ID);
				}
				
				evt->setPosition(params[2] * (float)screenX, params[3] * (float)screenY);

				evt->setExtraDataType(Event::ExtraDataFloatArray);
				evt->setExtraDataFloat(0, params[4] * (float)screenX);
				evt->setExtraDataFloat(1, params[5] * (float)screenY);


				break;
			case(Service::Controller):
				/* // Currently broken will need to be updated
				evt = mysInstance->writeHead();
				evt->reset(Event::Up, Service::Controller, params[0]);

				evt->setExtraDataType(Event::ExtraDataFloatArray);
				evt->setExtraDataFloat(0, (params[1])); // ControllerType

				if( (int)(params[1]) == Xbox360 || (int)(params[1]) == PS3 ){
					evt->setExtraDataFloat(1, params[2]); 
					evt->setExtraDataFloat(2, params[3]);  // Left analog (-up, +down)

					evt->setExtraDataFloat(3, params[4]); // Right analog (-left, +right)
					evt->setExtraDataFloat(4, params[5]); // Right analog (-up, +down)

					evt->setExtraDataFloat(5, params[6]); // Trigger 2 (+left, -right)

					// Buttons
					for( int i = 0; i < 15; i++ )
					{
						evt->setExtraDataFloat(i + 6, params[i + 6]);
					}
					evt->setExtraDataFloat(19, params[22]); // DPad

					evt->setExtraDataFloat(20, params[23]); // Tilt (+left, -right)
					evt->setExtraDataFloat(21, params[24]); // Tilt (+back, -forward)

				}
				else if( (int)(params[1]) == Wiimote ){
					for( int i = 1; i < 29; i++ )
					{
						evt->setExtraDataFloat(i, params[i+1]);
					}
				}
				else if( (int)(params[1]) == Wii_Nunchuk ){
					for( int i = 1; i < 8; i++ )
					{
						evt->setExtraDataFloat(i, params[i+1]);
					}
				}
				else if( (int)(params[1]) == Wii_MotionPlus ){
					for( int i = 1; i < 4; i++ )
					{
						evt->setExtraDataFloat(i, params[i+1]);
					}
				}
				*/
				break;
			default:
				printf("LegacyNetService: Unsupported input type %d \n", inputType);
				break;
		}// switch
		
		mysInstance->unlockEvents();
		//printf("Receiving datagram '%s'\n", message);
	
	
	} else {
#ifdef OMICRON_OS_WIN     
		printf("recvfrom failed with error code '%d'\n", WSAGetLastError());
#else
		printf("recvfrom failed with error: '%s'\n", strerror(errno));
#endif
	}// if-else recv result
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::setServer(const char* address, int port) 
{
	printf("Server set to '%s' on message port '%d'\n", address, port);
	serverAddress = address;
	msgPort = port;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::setDataport(int port) 
{
	printf("Dataport set to '%d'\n", port);
	dataPort = port;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::setScreenResolution(int x, int y) 
{
	printf("Screen resolution set to %d %d\n", x, y);
	screenX = x;
	screenY = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LegacyNetService::setTouchTimeout( float secTimeout ) 
{
	printf("Touch timeout set to %d seconds\n", secTimeout);
	touchTimeout = secTimeout;
}
