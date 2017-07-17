/******************************************************************************
* THE OMICRON SDK
*-----------------------------------------------------------------------------
* Copyright 2010-2016		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Arthur Nishimoto		anishimoto42@gmail.com
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2016, Electronic Visualization Laboratory,
* University of Illinois at Chicago
* All rights reserved.
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer. Redistributions in binary
* form must reproduce the above copyright notice, this list of conditions and
* the following disclaimer in the documentation and/or other materials provided
* with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*-----------------------------------------------------------------------------
* What's in this file:
*	The omicron input server. It can be used to stream event data to remote
*  clients using NetService or the omicronConnector client.
******************************************************************************/
#ifndef __OMICRON_INPUT_SERVER__
#define __OMICRON_INPUT_SERVER__

#include "omicron/osystem.h"
#include "omicron/DataManager.h"
#include "omicron/Event.h"
#include "omicron/Config.h"

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
    #include <sys/ioctl.h>
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

///////////////////////////////////////////////////////////////////////////////
// Based on Winsock UDP Server Example:
// http://msdn.microsoft.com/en-us/library/ms740148
// Also based on Beej's Guide to Network Programming:
// http://beej.us/guide/bgnet/output/html/multipage/clientserver.html
class NetClient
{
private:
	SOCKET udpSocket;
	SOCKET tcpSocket;
	sockaddr_in recvAddr;
	int clientMode;
	// 0 = NetClient sends data out to remote (default)
	// 1 = NetClient sends legacy data out to remote
	// 2 = NetClient receiving data from remote

	bool tcpConnected;
	bool udpConnected;

	const char* clientAddress;
	int clientPort;
public:
	NetClient(const char* address, int port)
	{
		clientAddress = address;
		clientPort = port;

		clientMode = 0;

		// Create a UDP socket for sending data
		udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);
		bind(udpSocket, (const sockaddr*)&recvAddr, sizeof(recvAddr));


		printf("NetClient %s:%i created for streaming data out...\n", address, port);
		udpConnected = true;
	}

	NetClient(const char* address, int port, SOCKET clientSocket)
	{
		clientAddress = address;
		clientPort = port;

		clientMode = 0;
		

		// Create a UDP socket for sending data
		udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);
		printf("NetClient %s:%i created...\n", address, port);
		udpConnected = true;

		tcpSocket = clientSocket;
		tcpConnected = true;
	}// CTOR

	NetClient(const char* address, int port, int mode, SOCKET clientSocket)
	{
		clientAddress = address;
		clientPort = port;

		clientMode = mode;
		
		// Create a UDP socket for sending data
		udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);
		udpConnected = true;

		tcpSocket = clientSocket;
		tcpConnected = true;

		if (clientMode == 1)
		{
			printf("Legacy NetClient %s:%i created...\n", address, port);
		}
		else if (clientMode == 2)
		{
			// Set socket to receive data from any address on a specified port
			recvAddr.sin_family = AF_INET;
			recvAddr.sin_port = htons(port);
			recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			bind(udpSocket, (const sockaddr*)&recvAddr, sizeof(recvAddr));
			printf("NetClient %s:%i created. Client to stream data to this machine.\n", address, port);
		}
		else if (clientMode == 3)
		{
			printf("TacTile NetClient %s:%i created...\n", address, port);
		}
		else
		{
			printf("NetClient %s:%i created...\n", address, port);
		}
	}// CTOR

	void sendEvent(char* eventPacket, int length)
	{
		// Send a datagram to the receiver
		sendto(udpSocket,
			eventPacket,
			length,
			0,
			(const struct sockaddr*)&recvAddr,
			sizeof(recvAddr));
	}// SendEvent

	int recvEvent(char* eventPacket, int length)
	{
		return recv(udpSocket, eventPacket, length, 0);
	}// recvEvent

	void sendMsg(char* eventPacket, int length)
	{
		// Ping the client to see if still active
		int result = sendto(tcpSocket,
			eventPacket,
			length,
			0,
			(const struct sockaddr*)&recvAddr,
			sizeof(recvAddr));

		if (result == SOCKET_ERROR)
		{
			tcpConnected = false;
		}
	}// SendMsg

	void setLegacy(bool value)
	{
		if (value)
			clientMode = 1;
		else
			clientMode = 0;
	}// setLegacy

	bool isLegacy()
	{
		return clientMode == 1;
	}// isLegacy

	bool isReceivingData()
	{
		return clientMode == 2;

	}// isReceivingData
};

namespace omicron {
///////////////////////////////////////////////////////////////////////////////
class OMICRON_API InputServer
{
public:
    virtual void handleEvent(const Event& evt);
    virtual bool handleLegacyEvent(const Event& evt);
    void startConnection(Config* cfg);
    SOCKET startListening();
    // VRPN Server (for CalVR)
    void loop();

	static char* createOmicronPacketFromEvent(const Event*);
	static omicronConnector::EventData createOmicronEventDataFromEventPacket(char*);

	void setServiceManager(ServiceManager*);
protected:
    void sendToClients(char*, int);
    void createClient(const char*,int, int, SOCKET);
private:
    enum dataMode { omicron, omicron_legacy };

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

    bool checkForDisconnectedClients;

    bool showEventStream;
    bool showStreamSpeed;
	bool showEventMessages;
    int lastOutgoingEventTime;
    int eventCount;

	ServiceManager* serviceManager;
#ifdef OMICRON_USE_VRPN
    // VRPN Server (for CalVR)
    const char	*TRACKER_NAME;
    int TRACKER_PORT;
    vrpn_XInputGamepad	*vrpnDevice;
    vrpn_Tracker_Remote	*tkr;
    vrpn_Connection		*connection;
#endif
};
};

#endif