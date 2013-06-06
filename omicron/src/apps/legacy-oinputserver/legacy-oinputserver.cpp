/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
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
	char SendBuf[1024];
	int BufLen;

public:
	NetClient::NetClient( const char* address, int port ){
		BufLen = 100;

		// Create a socket for sending data
		SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(port);
		RecvAddr.sin_addr.s_addr = inet_addr(address);
		printf("NetClient %s:%i created...\n", address, port);
	}// CTOR

	void NetClient::sendEvent( char* eventPacket ){
		// Send a datagram to the receiver
		//printf("Service: Sending datagram '%s' to receiver...\n", eventPacket);
		sendto(SendSocket, 
		  eventPacket, 
		  strlen(eventPacket), 
		  0, 
		  (SOCKADDR *) &RecvAddr, 
		  sizeof(RecvAddr));
	}// SendEvent
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OInputServer
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Checks the type of event. If a valid event, creates an event packet and returns true. Else return false.
	virtual bool handleEvent(const Event& evt)
	{
		eventPacket = new char[256];
		
		itoa(evt.getServiceType(), eventPacket, 10); // Append input type
		strcat( eventPacket, ":" );
		char floatChar[32];
		
		switch(evt.getServiceType())
		{
		case Service::Pointer:
			//printf(" Touch type %d \n", evt.getType()); 
			//printf("               at %f %f \n", x, y ); 

			// Converts gesture type to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getType());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts x to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition()[0]);
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts y to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition()[1]);
			strcat( eventPacket, floatChar );

			if( evt.getExtraDataItems() == 2){ // TouchPoint down/up/move
				// Converts xWidth to char, appends to eventPacket
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(0) );
				strcat( eventPacket, floatChar );
				
				// Converts yWidth to char, appends to eventPacket
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(1) );
				strcat( eventPacket, floatChar );
			} else { // Touch Gestures
				// Converts value to char, appends to eventPacket
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(0) );
				strcat( eventPacket, floatChar );
				
				// Converts value to char, appends to eventPacket
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(1) );
				strcat( eventPacket, floatChar );

				// Converts value to char, appends to eventPacket
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(2) );
				strcat( eventPacket, floatChar );

				// Converts value to char, appends to eventPacket
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(3) );
				strcat( eventPacket, floatChar );

				if( evt.getType() == Event::Rotate ){
					// Converts rotation to char, appends to eventPacket
					strcat( eventPacket, "," ); // Spacer
					sprintf(floatChar,"%f", evt.getExtraDataFloat(4) );
					strcat( eventPacket, floatChar );
				} else if( evt.getType() == Event::Split ){
					// Converts values to char, appends to eventPacket
					strcat( eventPacket, "," ); // Spacer
					sprintf(floatChar,"%f", evt.getExtraDataFloat(4) ); // Delta distance
					strcat( eventPacket, floatChar );

					strcat( eventPacket, "," ); // Spacer
					sprintf(floatChar,"%f", evt.getExtraDataFloat(5) ); // Delta ratio
					strcat( eventPacket, floatChar );
				}
			}

			strcat( eventPacket, " " ); // Spacer

			return true;
			break;
		//case Service::Pointer:
		//	x = evt.position[0];
		//	y = evt.position[1];

		//	// Converts x y float to chars and appents to eventPacket char*
		//	sprintf(floatChar,"%f",x);
		//	strcat( eventPacket, floatChar );
		//	strcat( eventPacket, "," );
		//	sprintf(floatChar,"%f",y);
		//	strcat( eventPacket, floatChar );
		//	strcat( eventPacket, " " );
		//	return true;
		//	break;

		case Service::Mocap:
		{
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts xPos to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition().x());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts yPos to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition().y());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts zPos to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition().z());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts xRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().x());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts yRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().y());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts zRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().z());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// Converts zRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().w());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, " " ); // Spacer
			return true;
			break;
		}

		case Service::Controller:
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( eventPacket, floatChar );
			strcat( eventPacket, "," ); // Spacer

			// See DirectXInputService.cpp for parameter details
			
			for( int i = 0; i < evt.getExtraDataItems(); i++ ){
				sprintf(floatChar,"%d", (int)evt.getExtraDataFloat(i));
				strcat( eventPacket, floatChar );
				if( i < evt.getExtraDataItems() - 1 )
					strcat( eventPacket, "," ); // Spacer
				else
					strcat( eventPacket, " " ); // Spacer
			}
			return true;
			break;
		case Service::Brain:
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( eventPacket, floatChar );
			for( int i = 0; i < 12; i++ ){
				strcat( eventPacket, "," ); // Spacer
				sprintf(floatChar,"%d", (int)evt.getExtraDataFloat(i));
				strcat( eventPacket, floatChar );
			}
			return true;
			break;
		case Service::Generic:
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( eventPacket, floatChar );
			return true;
			break;
		default: break;
		}
		
		delete[] eventPacket;
		return false;
	}
	
	void startConnection();
	SOCKET startListening();
	char* getEvent();
	void sendToClients( char* );
private:
	void createClient(const char*,int);

	WSADATA wsaData;
	const char* serverPort;
	SOCKET ListenSocket;
	
	char* eventPacket;
	
	#define DEFAULT_BUFLEN 512
	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen;
	
	// Collection of unique clients (IP/port combinations)
	std::map<char*,NetClient*> netClients;

	private:
		float rx;
		float ry;
		float rz;

		float x;
		float y;
		float z;

		float mx;
		float my;
		float mz;

		float lx;
		float ly;
		float lz;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Based on MSDN Winsock examples:
 * http://msdn.microsoft.com/en-us/library/ms738566(VS.85).aspx
 *
 * Non-blocking socket example:
 * http://www.win32developer.com/tutorial/winsock/winsock_tutorial_4.shtm
 */
void OInputServer::startConnection(){
	serverPort = "27000";
	ListenSocket = INVALID_SOCKET;
	recvbuflen = DEFAULT_BUFLEN;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("OInputServer: WSAStartup failed: %d\n", iResult);
		return;
	} else {
		printf("OInputServer: Winsock initialized \n");
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, serverPort, &hints, &result);
	if (iResult != 0) {
		printf("OInputServer: getaddrinfo failed: %d\n", iResult);
		WSACleanup();
	} else {
		printf("OInputServer: Server set to listen on port %s\n", serverPort);
	}

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// If iMode != 0, non-blocking mode is enabled.
	u_long iMode = 1;
	ioctlsocket(ListenSocket,FIONBIO,&iMode);

	if (ListenSocket == INVALID_SOCKET) {
		printf("OInputServer: Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	} else {
		printf("OInputServer: Listening socket created.\n");
	}

	// Setup the TCP listening socket
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("OInputServer: bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
}// startConnection

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SOCKET OInputServer::startListening(){
	SOCKET ClientSocket;

	// Listen on socket
	if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
		printf( "OInputServer: Error at bind(): %ld\n", WSAGetLastError() );
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	} else {
		//printf("NetService: Listening on socket.\n");
	}

	ClientSocket = INVALID_SOCKET;
	sockaddr_in clientInfo;
	int ret = sizeof(struct sockaddr);
	const char* clientAddress;

	// Accept a client socket
	ClientSocket = accept(ListenSocket, (SOCKADDR*)&clientInfo, &ret);

	if (ClientSocket == INVALID_SOCKET) {
		//printf("NetService: accept failed: %d\n", WSAGetLastError());
		// Commented out: We do not want to close the listen socket
		// since we are using a non-blocking socket until we are done listening for clients.
		//closesocket(ListenSocket);
		//WSACleanup();
		return NULL;
	} else {
		// Gets the clientInfo and extracts the IP address
		clientAddress = inet_ntoa(clientInfo.sin_addr);
		printf("OInputServer: Client '%s' Accepted.\n", clientAddress);
	}
	
	// Wait for client handshake
	// Here we constantly loop until data is received.
	// Because we're using a non-blocking socket, it is possible to attempt to receive before data is
	// sent, resulting in the 'recv failed' error that is commented out.
	bool gotData = false;
	float timer = 0.0f;
	float timeout = 500.0f; // milliseconds
	time_t startTime = time (NULL);

	printf("OInputServer: Waiting for client handshake\n");
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Service: Bytes received: %d\n", iResult);
			char* inMessage;
			char* portCStr;
			inMessage = new char[iResult];
			portCStr = new char[iResult];

			// Iterate through message string and
			// separate 'data_on,' from the port number
			int portIndex = -1;
			for( int i = 0; i < iResult; i++ ){
				if( recvbuf[i] == ',' ){
					portIndex = i + 1;
				} else if( i < portIndex ){
					inMessage[i] = recvbuf[i];
				} else {
					portCStr[i-portIndex] = recvbuf[i];
				}
			}

			// Make sure handshake is correct
			String handshake = "data_on";
			String handshakeLegacy = "omicron_legacy_data_on";
			int dataPort = 7000; // default port
			if( handshake.find(inMessage) || handshakeLegacy.find(inMessage) ){
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests data to be sent on port '%d'\n", clientAddress, dataPort);
				createClient( clientAddress, dataPort );
			}
			gotData = true;
			delete inMessage;
			delete portCStr;
		} else if (iResult == 0)
			printf("OInputServer: Connection closing...\n");
		else {
			timer = time (NULL);
			if( timer > startTime + timeout ){
				printf("OInputServer: Handshake timed out\n");
				break;
			}
			//printf("Service: recv failed: %d\n", WSAGetLastError());
			//closesocket(ClientSocket);
			//WSACleanup();
			//return NULL;
		}

	} while (!gotData);
	

	return ClientSocket;
}// startListening

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* OInputServer::getEvent(){
	return eventPacket;
}// getEvent

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OInputServer::createClient(const char* clientAddress, int dataPort){
	// Generate a unique name for client "address:port"
	char* addr = new char[128];
	strcpy( addr, clientAddress );
	char buf[32];
	strcat( addr, ":" );
	strcat( addr, itoa(dataPort,buf,10) );
	
	// Iterate through client map. If client name already exists,
	// do not add to list.
	std::map<char*, NetClient*>::iterator p;
	for(p = netClients.begin(); p != netClients.end(); p++) {
		printf( "%s \n", p->first );
		if( strcmp(p->first, addr) == 0 ){
			printf("OInputServer: NetClient already exists: %s \n", addr );
			return;
		}
	}

	netClients[addr] = new NetClient( clientAddress, dataPort );
	//printf("NetService: current nClients: %d \n", netClients.size() );
}// createClient

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OInputServer::sendToClients(char* event){
	// Iterate through all clients
	std::map<char*,NetClient*>::iterator itr = netClients.begin();
	while( itr != netClients.end() ){
		NetClient* client = itr->second;
		client->sendEvent( event );
		itr++;
	}// while
	delete[] event; // Clean up event after being set to all clients
}// createClient

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main(int argc, char** argv)
{
	OInputServer app;

	// Read config file name from command line or use default one.
	const char* cfgName = "oinputserver.cfg";
	if(argc == 2) cfgName = argv[1];

	Config* cfg = new Config(cfgName);

	DataManager* dm = DataManager::getInstance();
	// Add a default filesystem data source using current work dir.
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	ServiceManager* sm = new ServiceManager();
	sm->setupAndStart(cfg);

	app.startConnection();
	
	float delay = -0.01f; // Seconds to delay sending events (<= 0 disables delay)
	bool testStream = false;
	char* testPacket;

	bool printOutput = true;

	printf("OInputServer: Starting to listen for clients... \n");
	while(true){
		if( delay > 0.0 )
			Sleep(1000.0*delay); // Delay sending of data out

		sm->poll(); // Required for DirectInputService

		// Start listening for clients (non-blocking)
		app.startListening();

		// Get events
		int av = sm->getAvailableEvents();
		if(av != 0 && !testStream )
		{
			// @todo: Instead of copying the event list, we can lock the main one.
			Event evts[OMICRON_MAX_EVENTS];
			sm->getEvents(evts, OMICRON_MAX_EVENTS);
			for( int evtNum = 0; evtNum < av; evtNum++)
			{
				if( app.handleEvent(evts[evtNum]) ){ // is there an event?
					// Send event to clients
					if( printOutput )
						printf("%s\n", app.getEvent());
					app.sendToClients( app.getEvent() );
				}
			}
			if( printOutput )
				printf("------------------------------------------------------------------------------\n", app.getEvent());
		}// if
		else if( testStream ){
			testPacket = new char[99];
			// example touch string: '2:-10,0.5,0.5,0.1,0.1 '
			itoa(1, testPacket, 10); // Append input type
			strcat( testPacket, ":42,0.5,0.5,0.1,0.1 " );
			printf("OInputServer: main() ----- WARNING: TEST STREAM MODE ACTIVE -----\n",testPacket);
			printf("%s\n",testPacket);
			app.sendToClients( testPacket );
		}
		

	}// while

	sm->stop();
	delete sm;
	delete cfg;
	delete dm;
}
