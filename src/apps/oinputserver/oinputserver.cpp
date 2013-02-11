/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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

#include <stdio.h>

#ifdef WIN32
	#define OMICRON_OS_WIN
	#pragma comment(lib, "Ws2_32.lib")
#endif

#ifdef OMICRON_OS_WIN
	#include <winsock2.h>
	#include <ws2tcpip.h>

	// VRPN Server (for CalVR)
#ifdef OMICRON_USE_VRPN
	#include "omicron/vrpn/vrpn_tracker.h"
	#include "omicron/VRPNDevice.h"
#endif
#else
	#include <stdlib.h>
	#include <string.h>
	#include <netdb.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <unistd.h> // needed for close()
	#include <string>
#endif

#ifdef OMICRON_OS_WIN
	#define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %d\n", WSAGetLastError());
	#define SOCKET_CLOSE(sock) closesocket(sock);
	#define SOCKET_CLEANUP() WSACleanup();
	#define SOCKET_INIT() \
		iResult = WSAStartup(MAKEWORD(2,2), &wsaData); \
		if (iResult != 0) { \
			printf("%s: WSAStartup failed: %d\n", typeid(*this).name(), iResult); \
			return; \
		} else { \
			printf("%s: Winsock initialized \n",  typeid(*this).name()); \
		}
#else
	#define SOCKET_CLOSE(sock) close(sock);
	#define SOCKET_CLEANUP()
	#define SOCKET_INIT()
	#define SOCKET int
	#define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %s\n", strerror(errno));
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR   -1
	#define ioctlsocket ioctl // Used for setting socket blocking mode
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Based on Winsock UDP Server Example:
// http://msdn.microsoft.com/en-us/library/ms740148
// Also based on Beej's Guide to Network Programming:
// http://beej.us/guide/bgnet/output/html/multipage/clientserver.html
class NetClient
{
private:
	SOCKET sendSocket;
	sockaddr_in recvAddr;
	bool legacyMode;

public:
	NetClient( const char* address, int port )
	{
		legacyMode = false;

		// Create a UDP socket for sending data
		sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);
		printf("NetClient %s:%i created...\n", address, port);
	}// CTOR

	NetClient( const char* address, int port, int legacy )
	{
		legacyMode = legacy;

		// Create a UDP socket for sending data
		sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);

		if( legacyMode )
		{
			printf("Legacy NetClient %s:%i created...\n", address, port);
		}
		else
		{
			printf("NetClient %s:%i created...\n", address, port);
		}
	}// CTOR

	void sendEvent(char* eventPacket, int length)
	{
		// Send a datagram to the receiver
		sendto(sendSocket, 
			eventPacket, 
			length, 
			0,
            (const struct sockaddr*)&recvAddr,
			sizeof(recvAddr));
	}// SendEvent

	void setLegacy(bool value)
	{
		legacyMode = value;
	}// setLegacy

	bool isLegacy()
	{
		return legacyMode;
	}// isLegacy
};

#define OI_WRITEBUF(type, buf, offset, val) *((type*)&buf[offset]) = val; offset += sizeof(type);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OInputServer
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Checks the type of event. If a valid event, creates an event packet and returns true. Else return false.
	virtual void handleEvent(const Event& evt)
	{
		// If the event has been processed locally (i.e. by a filter event service)
		if(evt.isProcessed()) return;

#ifdef OMICRON_USE_VRPN
		vrpnDevice->update(evt);
#endif
			
		int offset = 0;
			
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getTimestamp()); 
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getSourceId()); 
		OI_WRITEBUF(int, eventPacket, offset, evt.getServiceId()); 
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getServiceType()); 
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getType()); 
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getFlags()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getPosition().x()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getPosition().y()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getPosition().z()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getOrientation().w()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getOrientation().x()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getOrientation().y()); 
		OI_WRITEBUF(float, eventPacket, offset, evt.getOrientation().z()); 
		
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getExtraDataType()); 
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getExtraDataItems()); 
		OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getExtraDataMask());
		
		if(evt.getExtraDataType() != Event::ExtraDataNull)
		{
			memcpy(&eventPacket[offset], evt.getExtraDataBuffer(), evt.getExtraDataSize());
		}
		offset += evt.getExtraDataSize();
		
		handleLegacyEvent(evt);
		
		std::map<char*,NetClient*>::iterator itr = netClients.begin();
		while( itr != netClients.end() )
		{
			NetClient* client = itr->second;
			
			if( client->isLegacy() )
			{
				//client->sendEvent(legacyPacket, 512);
			}
			else
			{
				client->sendEvent(eventPacket, offset);
			}
			itr++;
		}
	}
	
	virtual bool handleLegacyEvent(const Event& evt)
	{
		//itoa(evt.getServiceType(), eventPacket, 10); // Append input type
		sprintf(legacyPacket, "%d", evt.getServiceType());

		strcat( legacyPacket, ":" );
		char floatChar[32];
		
		switch(evt.getServiceType())
		{
		case Service::Pointer:
			//printf(" Touch type %d \n", evt.getType()); 
			//printf("               at %f %f \n", x, y ); 

			// Converts gesture type to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getType());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts x to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition().x());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts y to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition().y());
			strcat( legacyPacket, floatChar );

			if( evt.getExtraDataItems() == 2){ // TouchPoint down/up/move
				// Converts xWidth to char, appends to eventPacket
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(0) );
				strcat( legacyPacket, floatChar );
				
				// Converts yWidth to char, appends to eventPacket
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(1) );
				strcat( legacyPacket, floatChar );
			} else { // Touch Gestures
				// Converts value to char, appends to eventPacket
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(0) );
				strcat( legacyPacket, floatChar );
				
				// Converts value to char, appends to eventPacket
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(1) );
				strcat( legacyPacket, floatChar );

				// Converts value to char, appends to eventPacket
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(2) );
				strcat( legacyPacket, floatChar );

				// Converts value to char, appends to eventPacket
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%f", evt.getExtraDataFloat(3) );
				strcat( legacyPacket, floatChar );

				if( evt.getType() == Event::Rotate ){
					// Converts rotation to char, appends to eventPacket
					strcat( legacyPacket, "," ); // Spacer
					sprintf(floatChar,"%f", evt.getExtraDataFloat(4) );
					strcat( legacyPacket, floatChar );
				} else if( evt.getType() == Event::Split ){
					// Converts values to char, appends to eventPacket
					strcat( legacyPacket, "," ); // Spacer
					sprintf(floatChar,"%f", evt.getExtraDataFloat(4) ); // Delta distance
					strcat( legacyPacket, floatChar );

					strcat( legacyPacket, "," ); // Spacer
					sprintf(floatChar,"%f", evt.getExtraDataFloat(5) ); // Delta ratio
					strcat( legacyPacket, floatChar );
				}
			}

			strcat( legacyPacket, " " ); // Spacer

			return true;
			break;

		case Service::Mocap:
		{
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts xPos to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition()[0]);
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts yPos to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition()[1]);
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts zPos to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getPosition()[2]);
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts xRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().x());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts yRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().y());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts zRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().z());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts wRot to char, appends to eventPacket
			sprintf(floatChar,"%f",evt.getOrientation().w());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, " " ); // Spacer
			return true;
			break;
		}

		case Service::Controller:
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// See DirectXInputService.cpp for parameter details
			
			for( int i = 0; i < evt.getExtraDataItems(); i++ ){
				sprintf(floatChar,"%d", (int)evt.getExtraDataFloat(i));
				strcat( legacyPacket, floatChar );
				if( i < evt.getExtraDataItems() - 1 )
					strcat( legacyPacket, "," ); // Spacer
				else
					strcat( legacyPacket, " " ); // Spacer
			}
			return true;
			break;
		case Service::Wand:
			// Converts event type to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getType());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Converts flags to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getFlags());
			strcat( legacyPacket, floatChar );
			strcat( legacyPacket, "," ); // Spacer

			// Due to packet size constraints, wand events will
			// be treated as controller events (wand mocap data can
			// be grabbed as mocap events)

			// See DirectXInputService.cpp for parameter details
			
			for( int i = 0; i < evt.getExtraDataItems(); i++ ){
				sprintf(floatChar,"%f", evt.getExtraDataFloat(i));
				strcat( legacyPacket, floatChar );
				if( i < evt.getExtraDataItems() - 1 )
					strcat( legacyPacket, "," ); // Spacer
				else
					strcat( legacyPacket, " " ); // Spacer
			}
			return true;
			break;
		case Service::Brain:
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( legacyPacket, floatChar );
			for( int i = 0; i < 12; i++ ){
				strcat( legacyPacket, "," ); // Spacer
				sprintf(floatChar,"%d", (int)evt.getExtraDataFloat(i));
				strcat( legacyPacket, floatChar );
			}
			return true;
			break;
		case Service::Generic:
			// Converts id to char, appends to eventPacket
			sprintf(floatChar,"%d",evt.getSourceId());
			strcat( legacyPacket, floatChar );
			return true;
			break;
		default: break;
		}

		return false;
	}

	void startConnection(Config* cfg);
	SOCKET startListening();

	// VRPN Server (for CalVR)
	void loop();
private:
	enum dataMode { omicron, omicron_legacy };
	void createClient(const char*,int,bool);

	const char* serverPort;
	SOCKET listenSocket;    
	
	#define DEFAULT_BUFLEN 512
	char eventPacket[DEFAULT_BUFLEN];
	char legacyPacket[DEFAULT_BUFLEN];
	
	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen;
	
	// Collection of unique clients (IP/port combinations)
	std::map<char*,NetClient*> netClients;

#ifdef OMICRON_USE_VRPN
	// VRPN Server (for CalVR)
	const char	*TRACKER_NAME;
	int TRACKER_PORT;
	vrpn_XInputGamepad	*vrpnDevice;
	vrpn_Tracker_Remote	*tkr;
	vrpn_Connection		*connection;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OInputServer::startConnection(Config* cfg)
{
#ifdef OMICRON_OS_WIN
	WSADATA wsaData;
#endif

	Setting& sCfg = cfg->lookup("config");
	serverPort = strdup(Config::getStringValue("serverPort", sCfg, "27000").c_str());
	listenSocket = INVALID_SOCKET;
	recvbuflen = DEFAULT_BUFLEN;
	int iResult;

#ifdef OMICRON_USE_VRPN
	// VRPN Server Test ///////////////////////////////////////////////
	TRACKER_NAME = strdup(Config::getStringValue("vrpnTrackerName", sCfg, "Device0").c_str());
	TRACKER_PORT = Config::getFloatValue("vrpnTrackerPort", sCfg, 3891);

	// explicitly open the connection
	ofmsg("OInputServer: Created VRPNDevice %1%", %TRACKER_NAME);
	ofmsg("              Port: %1%", %TRACKER_PORT);
	connection = vrpn_create_server_connection(TRACKER_PORT);
	vrpnDevice = new vrpn_XInputGamepad(TRACKER_NAME, connection, 1);
	///////////////////////////////////////////////////////////////////
#endif

	// Initialize Winsock
    SOCKET_INIT();

	struct addrinfo *result = NULL, *ptr = NULL, hints;

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, serverPort, &hints, &result);
	if (iResult != 0) 
	{
		ofmsg("OInputServer: getaddrinfo failed: %1%", %iResult);
        SOCKET_CLEANUP();
	} 
	else 
	{
		ofmsg("OInputServer: Server set to listen on port %1%", %serverPort);
	}

	// Create a SOCKET for the server to listen for client connections
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// If iMode != 0, non-blocking mode is enabled.
	u_long iMode = 1;

	ioctlsocket(listenSocket,FIONBIO,&iMode);
	
	if (listenSocket == INVALID_SOCKET) 
	{
        PRINT_SOCKET_ERROR("OInputServer::startConnection");
		freeaddrinfo(result);
        SOCKET_CLEANUP(); 
		return;
	} 
	else 
	{
		printf("OInputServer: Listening socket created.\n");
	}

	// Setup the TCP listening socket
	iResult = bind( listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) 
	{
        PRINT_SOCKET_ERROR("OInputServer::startConnection: bind failed");
		freeaddrinfo(result);

		SOCKET_CLOSE(listenSocket);
		SOCKET_CLEANUP();
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SOCKET OInputServer::startListening()
{
	SOCKET clientSocket;

	// Listen on socket
	if ( listen( listenSocket, SOMAXCONN ) == SOCKET_ERROR )
	{
		PRINT_SOCKET_ERROR("OInputServer::startListening: bind failed");
		SOCKET_CLOSE(listenSocket);
		SOCKET_CLEANUP();
		return 0;
	} 
	else
	{
		//printf("NetService: Listening on socket.\n");
	}

	clientSocket = INVALID_SOCKET;
	sockaddr_in clientInfo;
	int addrSize = sizeof(struct sockaddr);
	const char* clientAddress;

	// Accept a client socket
	clientSocket = accept(listenSocket, (struct sockaddr *)&clientInfo, (socklen_t*)&addrSize);

	if (clientSocket == INVALID_SOCKET) 
	{
		//printf("NetService: accept failed: %d\n", WSAGetLastError());
		// Commented out: We do not want to close the listen socket
		// since we are using a non-blocking socket until we are done listening for clients.
		//closesocket(listenSocket);
		//WSACleanup();
		//return NULL;
        return 0;
	} 
	else 
	{
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
	do 
	{
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			//printf("Service: Bytes received: %d\n", iResult);
			char* inMessage;
			char* portCStr;
			inMessage = new char[iResult];
			portCStr = new char[iResult];

			// Iterate through message string and
			// separate 'data_on,' from the port number
			int portIndex = iResult;
			for( int i = 0; i < iResult; i++ )
			{
				if( recvbuf[i] == ',' )
				{
					portIndex = i + 1;
					inMessage[i] = '\n';
				}
				else if( i < portIndex )
				{
					inMessage[i] = recvbuf[i];
				} 
				else 
				{
					portCStr[i-portIndex] = recvbuf[i];
				}
			}

			// Make sure handshake is correct
			char* handshake = "data_on";
			char* omicronHandshake = "omicron_data_on";
			char* legacyHandshake = "omicron_legacy_data_on";
			int dataPort = 7000; // default port

			if( strcmp(inMessage, legacyHandshake) == 1 )
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests omicron legacy data to be sent on port '%d'\n", clientAddress, dataPort);
				printf("OInputServer: WARNING - This server does not support legacy data!\n");
				createClient( clientAddress, dataPort, true );
			}
			else if( strcmp(inMessage, omicronHandshake) == 1 )
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests omicron data to be sent on port '%d'\n", clientAddress, dataPort);
				createClient( clientAddress, dataPort, false );
			}
			else if( strcmp(inMessage, handshake) == 1 )
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests data (old handshake) to be sent on port '%d'\n", clientAddress, dataPort);
				createClient( clientAddress, dataPort, false );
			}
			else
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests data to be sent on port '%d'\n", clientAddress, dataPort);
				printf("OInputServer: '%s' using unknown handshake '%s'\n", clientAddress, inMessage);
				createClient( clientAddress, dataPort, false );
			}

			gotData = true;
			delete inMessage;
			delete portCStr;
		} 
		else if (iResult == 0)
		{
			printf("OInputServer: Connection closing...\n");
		}
		else 
		{
			timer = time (NULL);
			if( timer > startTime + timeout )
			{
				printf("OInputServer: Handshake timed out\n");
				break;
			}
		}
	} while (!gotData);
	

	return clientSocket;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OInputServer::loop()
{
#ifdef OMICRON_USE_VRPN
	// VRPN connection
	connection->mainloop();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OInputServer::createClient(const char* clientAddress, int dataPort, bool legacy)
{
	// Generate a unique name for client "address:port"
	char* addr = new char[128];
	strcpy( addr, clientAddress );
	char buf[32];
	strcat( addr, ":" );

#ifdef OMICRON_OS_WIN
	strcat( addr, itoa(dataPort,buf,10) );
#else
	snprintf(buf, 32, "%d",dataPort);
    strcat(addr, buf);
#endif
	
	// Iterate through client map. If client name already exists,
	// do not add to list.
	std::map<char*, NetClient*>::iterator p;
	for(p = netClients.begin(); p != netClients.end(); p++) 
	{
		//printf( "%s \n", p->first );
		if( strcmp(p->first, addr) == 0 )
		{
			printf("OInputServer: NetClient already exists: %s \n", addr );

			// Check dataMode: if different, update client
			if( p->second->isLegacy() != legacy )
			{
				if( legacy )
				{
					printf("OInputServer: NetClient %s now using legacy omicron data \n", addr );
					printf("OInputServer: WARNING - This server does not support legacy data!\n");
				}
				else
					printf("OInputServer: NetClient %s now using omicron data \n", addr );
				p->second->setLegacy(legacy);
			}
			return;
		}
	}

	netClients[addr] = new NetClient( clientAddress, dataPort, legacy );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  main(int argc, char** argv)
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

	app.startConnection(cfg);

	omsg("OInputServer: Starting to listen for clients...");
	int i = 0;
	while(true)
	{
		sm->poll();
		app.loop();

		// Start listening for clients (non-blocking)
		app.startListening();

		// Get events
		int av = sm->getAvailableEvents();
		//ofmsg("------------------------loop %1%  av %2%", %i++ %av);
		if(av != 0)
		{
			// TODO: Instead of copying the event list, we can lock the main one.
			Event evts[OMICRON_MAX_EVENTS];
			sm->getEvents(evts, OMICRON_MAX_EVENTS);
			for( int evtNum = 0; evtNum < av; evtNum++)
			{
				app.handleEvent(evts[evtNum]);
			}
		}
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif	
	}

	sm->stop();
	delete sm;
	delete cfg;
	delete dm;
}
