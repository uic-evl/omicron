/******************************************************************************
* THE OMICRON SDK
*-----------------------------------------------------------------------------
* Copyright 2010-2018		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Arthur Nishimoto		anishimoto42@gmail.com
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2018, Electronic Visualization Laboratory,
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
#define OMICRON_USE_INPUTSERVER

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
	#include <fcntl.h> // Non-blocking socket
#endif

#ifdef OMICRON_OS_WIN     
#define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %d\n", WSAGetLastError());
#define SOCKET_CLOSE(sock) closesocket(sock);
#define SOCKET_CLEANUP() WSACleanup();
#define SOCKET_INIT() \
            int iResult;    \
            iResult = WSAStartup(MAKEWORD(2,2), &wsaData); \
            if (iResult != 0) { \
                printf("OmicronConnectorClient: WSAStartup failed: %d\n", iResult); \
            }
#else
#define SOCKET_CLOSE(sock) close(sock);
#define SOCKET_CLEANUP()
#define SOCKET_INIT()
#define SOCKET int
#define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %s\n", strerror(errno));
#define SOCKET_ERROR            (-1)
#define INVALID_SOCKET            (0)
#endif

enum DataMode { data_omicron, data_omicron_legacy, data_omicron_in, data_tactile, data_omicronV2, data_omicronV3 };

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
	sockaddr_in senderAddr;

	int senderAddrSize;
	struct timeval timeout;

	DataMode clientMode;
	// 0 = NetClient sends data out to remote (default)
	// 1 = NetClient sends legacy data out to remote
	// 2 = NetClient receiving data from remote
	// 3 = NetClient sends TacTile data out to remote

	bool tcpConnected;
	bool udpConnected;

	const char* clientAddress;
	int clientPort;
	int clientFlags;

	enum ClientFlags
	{
		DataIn = 1 << 0,
		ServiceTypePointer = 1 << 1,
		ServiceTypeMocap = 1 << 2,
		ServiceTypeKeyboard = 1 << 3,
		ServiceTypeController = 1 << 4,
		ServiceTypeUi = 1 << 5,
		ServiceTypeGeneric = 1 << 6,
		ServiceTypeBrain = 1 << 7,
		ServiceTypeWand = 1 << 8,
		ServiceTypeSpeech = 1 << 9,
		ServiceTypeImage = 1 << 10,
		AlwaysTCP = 1 << 11,
		AlwaysUDP = 1 << 12
	};
public:
	NetClient(const char* address, int port, int flags = 2046)
	{
		clientAddress = address;
		clientPort = port;

		clientMode = data_omicron;
		updateFlags(flags);

		// Create a UDP socket for sending data
		udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);

		printf("NetClient %s:%i created for streaming data out...\n", address, port);
		udpConnected = true;
	}

	NetClient(const char* address, int port, SOCKET clientSocket, int flags)
	{
		clientAddress = address;
		clientPort = port;

		clientMode = data_omicron;
		updateFlags(flags);

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

	NetClient(const char* address, int port, DataMode mode, SOCKET clientSocket, int flags)
	{
		clientAddress = address;
		clientPort = port;

		clientMode = mode;
		updateFlags(flags);

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

		if (clientMode == data_omicron_legacy)
		{
			printf("Legacy NetClient %s:%i created...\n", address, port);
		}
		else if (clientMode == data_omicron_in)
		{
			senderAddr.sin_family = AF_INET;
			senderAddr.sin_port = htons(port);
			senderAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			senderAddrSize = sizeof(senderAddr);

			int iResult = ::bind(udpSocket, (const sockaddr*)&senderAddr, sizeof(senderAddr));
			if (iResult == SOCKET_ERROR) {
				PRINT_SOCKET_ERROR("NetClient: data_omicron_in - bind failed");
			}
			else
			{
				printf("NetClient %s:%i created. Client to stream data to this machine.\n", address, port);
			}

		}
		else if (clientMode == data_tactile)
		{
			printf("TacTile NetClient %s:%i created...\n", address, port);
		}
		else
		{
			printf("NetClient %s:%i created...\n", address, port);
		}
	}// CTOR

	void updateClientSocket(SOCKET clientSocket)
	{
		tcpSocket = clientSocket;
		tcpConnected = true;
	}

	void sendEvent(char* eventPacket, int length)
	{
		if (isFlagEnabled(ClientFlags::AlwaysTCP))
		{
			sendMsg(eventPacket, length);
		}
		else
		{
			// Send a datagram to the receiver
			sendto(udpSocket,
				eventPacket,
				length,
				0,
				(const struct sockaddr*)&recvAddr,
				sizeof(recvAddr)
			);
		}
	}// SendEvent

	int recvEvent(char* eventPacket, int length)
	{
		int result;

		// Create a set of fd_set to store sockets
		fd_set ReadFDs, WriteFDs, ExceptFDs;

		// Set collections to null
		FD_ZERO(&ReadFDs);
		FD_ZERO(&WriteFDs);
		FD_ZERO(&ExceptFDs);

		FD_SET(udpSocket, &ReadFDs);
		FD_SET(udpSocket, &ExceptFDs);

		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		result = select(udpSocket + 1, &ReadFDs, &WriteFDs, &ExceptFDs, &timeout);
		if (result > 0)
		{
			return recvfrom(udpSocket,
				eventPacket,
				length,
				0,
				(sockaddr *)&senderAddr,
				(socklen_t*)&senderAddrSize
			);
		}
		else
		{
			return result;
		}
	}// recvEvent

	void sendMsg(char* eventPacket, int length)
	{
		if (isFlagEnabled(ClientFlags::AlwaysUDP))
		{
			sendEvent(eventPacket, length);
		}
		else
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
		}
	}// SendMsg

	void updateFlags(int flags)
	{
		if (clientFlags != flags)
		{
			printf("NetClient %s:%i updated flags.\n", clientAddress, clientPort);
		}

		clientFlags = flags;
		/*
		printf("NetClient %s:%i has flags.\n", clientAddress, clientPort);
		printf("   Flag: DataIn %i\n", (clientFlags & ClientFlags::DataIn) == ClientFlags::DataIn);
		printf("   Flag: ServicePointerEnabled %i\n", (clientFlags & ClientFlags::ServicePointer) == ClientFlags::ServicePointer);
		printf("   Flag: ServiceMocapEnabled %i\n", (clientFlags & ClientFlags::ServiceMocap) == ClientFlags::ServiceMocap);
		printf("   Flag: ServiceKeyboardEnabled %i\n", (clientFlags & ClientFlags::ServiceKeyboard) == ClientFlags::ServiceKeyboard);
		printf("   Flag: ServiceControllerEnabled %i\n", (clientFlags & ClientFlags::ServiceController) == ClientFlags::ServiceController);
		printf("   Flag: ServiceUIEnabled %i\n", (clientFlags & ClientFlags::ServiceUI) == ClientFlags::ServiceUI);
		printf("   Flag: ServiceGenericEnabled %i\n", (clientFlags & ClientFlags::ServiceGeneric) == ClientFlags::ServiceGeneric);
		printf("   Flag: ServiceBrainEnabled %i\n", (clientFlags & ClientFlags::ServiceBrain) == ClientFlags::ServiceBrain);
		printf("   Flag: ServiceWandEnabled %i\n", (clientFlags & ClientFlags::ServiceWand) == ClientFlags::ServiceWand);
		printf("   Flag: ServiceSpeechEnabled %i\n", (clientFlags & ClientFlags::ServiceSpeech) == ClientFlags::ServiceSpeech);
		printf("   Flag: ServiceImageEnabled %i\n", (clientFlags & ClientFlags::ServiceImage) == ClientFlags::ServiceImage);
		printf("   Flag: AlwaysTCP %i\n", (clientFlags & ClientFlags::AlwaysTCP) == ClientFlags::AlwaysTCP);
		printf("   Flag: AlwaysUDP %i\n", (clientFlags & ClientFlags::AlwaysUDP) == ClientFlags::AlwaysUDP);
		*/

		if(isFlagEnabled(ClientFlags::DataIn))
		{
			clientMode = data_omicron_in;
		}
	}

	DataMode getMode()
	{
		return clientMode;
	}

	void setMode(DataMode mode)
	{
		clientMode = mode;
	}

	bool isReceivingData()
	{
		return clientMode == data_omicron_in;
	}// isReceivingData

	bool isFlagEnabled(ClientFlags flag)
	{
		return (clientFlags & flag) == flag;
	}

	bool requestedServiceType(omicron::Service::ServiceType type)
	{
		switch (type)
		{
		case(omicron::Service::ServiceType::Pointer):
			return (clientFlags & ClientFlags::ServiceTypePointer) == ClientFlags::ServiceTypePointer;
		case(omicron::Service::ServiceType::Mocap):
			return (clientFlags & ClientFlags::ServiceTypeMocap) == ClientFlags::ServiceTypeMocap;
		case(omicron::Service::ServiceType::Keyboard):
			return (clientFlags & ClientFlags::ServiceTypeKeyboard) == ClientFlags::ServiceTypeKeyboard;
		case(omicron::Service::ServiceType::Controller):
			return (clientFlags & ClientFlags::ServiceTypeController) == ClientFlags::ServiceTypeController;
		case(omicron::Service::ServiceType::Ui):
			return (clientFlags & ClientFlags::ServiceTypeUi) == ClientFlags::ServiceTypeUi;
		case(omicron::Service::ServiceType::Generic):
			return (clientFlags & ClientFlags::ServiceTypeGeneric) == ClientFlags::ServiceTypeGeneric;
		case(omicron::Service::ServiceType::Brain):
			return (clientFlags & ClientFlags::ServiceTypeBrain) == ClientFlags::ServiceTypeBrain;
		case(omicron::Service::ServiceType::Wand):
			return (clientFlags & ClientFlags::ServiceTypeWand) == ClientFlags::ServiceTypeWand;
		case(omicron::Service::ServiceType::Speech):
			return (clientFlags & ClientFlags::ServiceTypeSpeech) == ClientFlags::ServiceTypeSpeech;
		case(omicron::Service::ServiceType::Image):
			return (clientFlags & ClientFlags::ServiceTypeImage) == ClientFlags::ServiceTypeImage;
		}
		return false;
	}

	void dispose()
	{
		int iResult;
		if (udpConnected)
		{
			iResult = SOCKET_CLOSE(udpSocket);
			if (iResult == -1)
			{
				PRINT_SOCKET_ERROR("NetClient: Failed to close udpSocket");
			}
			else
			{
				printf("NetClient: Cleaned up udpSocket\n");
			}
		}
	}
};

namespace omicron {
	
///////////////////////////////////////////////////////////////////////////////
class OMICRON_API InputServer
{
public:
    virtual void handleEvent(const Event& evt);
    virtual bool handleLegacyEvent(const Event& evt);
	virtual bool handleTacTileEvent(const Event& evt);
    void startConnection(Config* cfg);
    SOCKET startListening();
    // VRPN Server (for CalVR)
    void loop();

	static char* createOmicronPacketFromEvent(const Event*);
	static omicronConnector::EventData createOmicronEventDataFromEventPacket(char*);

	void setServiceManager(ServiceManager*);

protected:
    void sendToClients(char*);
    void createClient(const char*, int, DataMode mode, SOCKET, int flags = 2046);
private:
    const char* serverPort;
    SOCKET listenSocket;    

	const static char* handshake;
	const static char* omicronHandshake;
	const static char* omicronV2Handshake;
	const static char* omicronStreamInHandshake;
	const static char* legacyHandshake;
	const static char* tactileHandshake;
	const static char* omicronV3Handshake;

    char eventPacket[DEFAULT_BUFLEN];
    char legacyPacket[DEFAULT_BUFLEN];
	char tacTilePacket[DEFAULT_BUFLEN];

	char eventPacketLarge[DEFAULT_LRGBUFLEN];

	bool validLegacyEvent;
	bool validTacTileEvent;

    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen;
    
    // Collection of unique clients (IP/port combinations)
    std::map<char*,NetClient*> netClients;

    bool checkForDisconnectedClients;

    bool showEventStream;
    bool showStreamSpeed;
	bool showEventMessages;
	bool showIncomingStream;
	bool showIncomingMessages;
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