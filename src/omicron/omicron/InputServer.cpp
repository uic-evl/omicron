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
#include "omicron/InputServer.h"
#include "omicron/StringUtils.h"
#include <vector>

#include <time.h>
using namespace omicron;

#include <stdio.h>

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////
// Based on Winsock UDP Server Example:
// http://msdn.microsoft.com/en-us/library/ms740148
// Also based on Beej's Guide to Network Programming:
// http://beej.us/guide/bgnet/output/html/multipage/clientserver.html
class NetClient
{
private:
    SOCKET sendSocket;
    SOCKET msgSocket;
    sockaddr_in recvAddr;
	
	InputServer::DataMode legacyMode;

    bool connected;

    const char* clientAddress;
    int clientPort;
public:
    NetClient( const char* address, int port, SOCKET clientSocket )
    {
		createSocketConnection(address, port, InputServer::DataMode::Omicron, clientSocket);
		printf("NetClient %s:%i created...\n", address, port);
    }// CTOR

	NetClient(const char* address, int port, InputServer::DataMode mode, SOCKET clientSocket)
    {
		createSocketConnection(address, port, mode, clientSocket);
		if (mode == InputServer::DataMode::Omicron_Legacy)
		{
			printf("Legacy NetClient %s:%i created...\n", address, port);
		}
		else if (mode == InputServer::DataMode::Omicron)
		{
			printf("NetClient %s:%i created...\n", address, port);
		}
    }// CTOR

	void createSocketConnection(const char* address, int port, InputServer::DataMode mode, SOCKET clientSocket)
	{
		clientAddress = address;
		clientPort = port;

		legacyMode = mode;
		connected = true;

		// Create a UDP socket for sending data
		sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Set up the RecvAddr structure with the IP address of
		// the receiver
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_port = htons(port);
		recvAddr.sin_addr.s_addr = inet_addr(address);

		msgSocket = clientSocket;
	}

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

    void sendMsg(char* eventPacket, int length)
    {
        // Ping the client to see if still active
        int result = sendto(msgSocket, 
            eventPacket, 
            length, 
            0,
            (const struct sockaddr*)&recvAddr,
            sizeof(recvAddr));

        if( result == SOCKET_ERROR )
        {
            connected = false;
        }
    }// SendMsg

	void setLegacy(InputServer::DataMode value)
    {
        legacyMode = value;
    }// setLegacy

	InputServer::DataMode getDataMode()
	{
		return legacyMode;
	}// getDataMode

    bool isLegacy()
    {
		return legacyMode == InputServer::DataMode::Omicron_Legacy;
    }// isLegacy

    bool isConnected()
    {
        return connected;
    }// isConnected
};

#define OI_WRITEBUF(type, buf, offset, val) *((type*)&buf[offset]) = val; offset += sizeof(type);

///////////////////////////////////////////////////////////////////////////////
// Creates an event packet from an Omicron event. Returns the buffer offset.
char* InputServer::createOmicronEventPacket(const Event* evt)
{
    int offset = 0;
    
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getTimestamp()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getSourceId()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getDeviceTag()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getServiceType()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getType()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getFlags()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getPosition().x()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getPosition().y()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getPosition().z()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getOrientation().w()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getOrientation().x()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getOrientation().y()); 
    OI_WRITEBUF(float, eventPacket, offset, evt->getOrientation().z()); 
        
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getExtraDataType()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getExtraDataItems()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt->getExtraDataMask());
        
    if(evt->getExtraDataType() != Event::ExtraDataNull)
    {
        memcpy(&eventPacket[offset], evt->getExtraDataBuffer(), evt->getExtraDataSize());
    }
    offset += evt->getExtraDataSize();

    return eventPacket;
}

///////////////////////////////////////////////////////////////////////////////
//
void InputServer::sendToClients(char* eventPacket, int priority)
{
    
}

///////////////////////////////////////////////////////////////////////////////
// Checks the type of event. If a valid event, creates an event packet and sends to clients.
void InputServer::handleEvent(const Event& evt)
{
	// If the event has been processed locally (i.e. by a filter event service)
	if (evt.isProcessed()) return;

#ifdef OMICRON_USE_VRPN
	vrpnDevice->update(&evt);
#endif

	int priority = 0;
	if (evt.getType() == Event::Type::Update || evt.getType() == Event::Type::Move)
	{
		priority = 0;
	}
	else
	{
		priority = 1;
	}

	std::map<char*, NetClient*>::iterator itr = netClients.begin();
	while (itr != netClients.end())
	{
		NetClient* client = itr->second;

		switch(client->getDataMode())
		{
			case(DataMode::Omicron) : generateOmicronPacket(evt); break;
			case(DataMode::Omicron_Legacy) : generateLegacyPacket(evt); break;
			case(DataMode::TacTile) : generateTacTilePacket(evt); break;
			default: generateOmicronPacket(evt); break;
		}

		if (priority == 0) // UDP
		{
			client->sendEvent(eventPacket, DEFAULT_BUFLEN);
		}
		else if (priority == 1) // TCP
		{
			client->sendMsg(eventPacket, DEFAULT_BUFLEN);

			// Also send as UDP for legcy clients
			client->sendEvent(eventPacket, DEFAULT_BUFLEN);
		}
		itr++;
	}

	if (evt.getType() == Event::Type::Update || evt.getType() == Event::Type::Move)
	{
		if (showEventStream)
			printf("oinputserver: Event %d type: %d sent at pos %f %f\n", evt.getSourceId(), evt.getType(), evt.getPosition().x(), evt.getPosition().y());
		sendToClients(eventPacket, 0);
	}
	else
	{
		sendToClients(eventPacket, 0); // Also send to udp stream for legacy clients
		if (showEventMessages)
			printf("oinputserver: Event %d type: %d sent at pos %f %f\n", evt.getSourceId(), evt.getType(), evt.getPosition().x(), evt.getPosition().y());
		sendToClients(eventPacket, 1); // Send to TCP clients expecting reliable events

	}
}

///////////////////////////////////////////////////////////////////////////////
// Binary packet of standard Omicron event
bool InputServer::generateOmicronPacket(const Event& evt)
{
    timeb tb;
    ftime( &tb );
    int timestamp = tb.millitm + (tb.time & 0xfffff) * 1000;
            
    int offset = 0;
            
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getTimestamp()); 
    OI_WRITEBUF(unsigned int, eventPacket, offset, evt.getSourceId()); 
    OI_WRITEBUF(int, eventPacket, offset, evt.getDeviceTag()); 
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
    
	if (showStreamSpeed)
	{
		if ((timestamp - lastOutgoingEventTime) >= 1000)
		{
			lastOutgoingEventTime = timestamp;
			ofmsg("oinputserver: Outgoing event stream %1% event(s)/sec", %eventCount);
			eventCount = 0;
		}
		else
		{
			eventCount++;
		}
	}

	return true;
}
    
///////////////////////////////////////////////////////////////////////////////
// dgram string of format:
// Pointer: 'serviceType:eventType,x,y,w,h '
// Pointer (gesture): 'serviceType:eventType,x,y,extraData1,extraData2,etc '
bool InputServer::generateLegacyPacket(const Event& evt)
{
    //itoa(evt.getServiceType(), eventPacket, 10); // Append input type
    sprintf(eventPacket, "%d", evt.getServiceType());

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
        sprintf(floatChar,"%f",evt.getPosition().x());
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Converts y to char, appends to eventPacket
        sprintf(floatChar,"%f",evt.getPosition().y());
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

    case Service::Mocap:
    {
        // Converts id to char, appends to eventPacket
        sprintf(floatChar,"%d",evt.getSourceId());
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Converts xPos to char, appends to eventPacket
        sprintf(floatChar,"%f",evt.getPosition()[0]);
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Converts yPos to char, appends to eventPacket
        sprintf(floatChar,"%f",evt.getPosition()[1]);
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Converts zPos to char, appends to eventPacket
        sprintf(floatChar,"%f",evt.getPosition()[2]);
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

        // Converts wRot to char, appends to eventPacket
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
    case Service::Wand:
        // Converts event type to char, appends to eventPacket
        sprintf(floatChar,"%d",evt.getType());
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Converts id to char, appends to eventPacket
        sprintf(floatChar,"%d",evt.getSourceId());
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Converts flags to char, appends to eventPacket
        sprintf(floatChar,"%d",evt.getFlags());
        strcat( eventPacket, floatChar );
        strcat( eventPacket, "," ); // Spacer

        // Due to packet size constraints, wand events will
        // be treated as controller events (wand mocap data can
        // be grabbed as mocap events)

        // See DirectXInputService.cpp for parameter details
            
        for( int i = 0; i < evt.getExtraDataItems(); i++ ){
            sprintf(floatChar,"%f", evt.getExtraDataFloat(i));
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

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// dgram string of format:
// Pointer: 'timestamp:flag:id,x,y,w,h,gesture,intensity '
// Pointer (gesture): 'serviceType:eventType,x,y,extraData1,extraData2,etc '
bool InputServer::generateTacTilePacket(const Event& evt)
{
	itoa(evt.getTimestamp(), eventPacket, 10); // Append input type
	strcat(eventPacket, ":q:");

	char floatChar[32];

	switch (evt.getServiceType())
	{
	case Service::Pointer:
		//printf(" Touch type %d \n", evt.getType()); 
		//printf("               at %f %f \n", x, y ); 

		// Converts id to char, appends to eventPacket
		sprintf(floatChar, "%d", evt.getSourceId());
		strcat(eventPacket, floatChar);
		strcat(eventPacket, ","); // Spacer

		// Converts x to char, appends to eventPacket
		sprintf(floatChar, "%f", evt.getPosition()[0]);
		strcat(eventPacket, floatChar);
		strcat(eventPacket, ","); // Spacer

		// Converts y to char, appends to eventPacket
		sprintf(floatChar, "%f", evt.getPosition()[1]);
		strcat(eventPacket, floatChar);

		if (evt.getExtraDataItems() == 2){ // TouchPoint down/up/move
			// Converts xWidth to char, appends to eventPacket
			strcat(eventPacket, ","); // Spacer
			sprintf(floatChar, "%f", evt.getExtraDataFloat(0));
			strcat(eventPacket, floatChar);

			// Converts yWidth to char, appends to eventPacket
			strcat(eventPacket, ","); // Spacer
			sprintf(floatChar, "%f", evt.getExtraDataFloat(1));
			strcat(eventPacket, floatChar);

			// Converts eventType to TacTile gesture
			if (evt.getType() == EventBase::Type::Down)
			{
				strcat(eventPacket, ","); // Spacer
				strcat(eventPacket, "0");
			}
			else if (evt.getType() == EventBase::Type::Up)
			{
				strcat(eventPacket, ","); // Spacer
				strcat(eventPacket, "2");
			}
			else // Move
			{
				strcat(eventPacket, ","); // Spacer
				strcat(eventPacket, "1");
			}

			// Intensity
			strcat(eventPacket, ",1.0");
		}
		strcat(eventPacket, " "); // Spacer

		return true;
		break;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
void InputServer::startConnection(Config* cfg)
{
#ifdef OMICRON_OS_WIN
    WSADATA wsaData;
#endif

    lastOutgoingEventTime = 0;
    eventCount = 0;

    Setting& sCfg = cfg->lookup("config");
    serverPort = strdup(Config::getStringValue("serverPort", sCfg, "27000").c_str());
    String serverIP = Config::getStringValue("serverListenIP", sCfg, "");

    checkForDisconnectedClients = Config::getBoolValue("checkForDisconnectedClients", sCfg, false );
    showEventStream = Config::getBoolValue("showEventStream", sCfg, false );
    showStreamSpeed = Config::getBoolValue("showStreamSpeed", sCfg, false );
	showEventMessages = Config::getBoolValue("showEventMessages", sCfg, false);

    if( checkForDisconnectedClients )
        omsg("Check for disconnected clients enabled.");

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
    if( serverIP.length() != 0 )
        iResult = getaddrinfo(strdup(serverIP.c_str()), serverPort, &hints, &result);
    else
        iResult = getaddrinfo(NULL, serverPort, &hints, &result);

    if (iResult != 0) 
    {
        ofmsg("OInputServer: getaddrinfo failed: %1%", %iResult);
        SOCKET_CLEANUP();
    } 
    
    // Create a SOCKET for the server to listen for client connections
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    if( iResult == 0 ) 
    {
        ofmsg("OInputServer: Server listening on %1% port %2%", %serverIP %serverPort);
    }

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

///////////////////////////////////////////////////////////////////////////////
SOCKET InputServer::startListening()
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
    int timeout = 1; // seconds
    int startTime = time (NULL);
    int timeoutTime = startTime + timeout;

    printf("OInputServer: Waiting for client handshake\n");
    do 
    {
        iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            //printf("Service: Bytes received: %d\n", iResult);
			String inMessage;
            char* portCStr;
            inMessage = "";
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
                    inMessage += recvbuf[i];
                } 
                else 
                {
                    portCStr[i-portIndex] = recvbuf[i];
                    portCStr[i-portIndex+1] = '\n';
                }
            }

            // Make sure handshake is correct
            String handshake = "data_on";
			String omicronHandshake = "omicron_data_on";
			String legacyHandshake = "omicron_legacy_data_on";
			String tacTileHandshake = "tactile_data_on";
            int dataPort = 7000; // default port
			
			if (legacyHandshake.find(inMessage) == 0)
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests omicron legacy data to be sent on port '%d'\n", clientAddress, dataPort);
                printf("OInputServer: WARNING - Legacy data many not be supported for all service types!\n");
                createClient( clientAddress, dataPort, DataMode::Omicron_Legacy, clientSocket );
            }
			else if (omicronHandshake.find(inMessage) == 0)
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests omicron data to be sent on port '%d'\n", clientAddress, dataPort);
				createClient(clientAddress, dataPort, DataMode::Omicron, clientSocket);
            }
			else if (tacTileHandshake.find(inMessage) == 0)
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests TacTile data to be sent on port '%d'\n", clientAddress, dataPort);
				printf("OInputServer: WARNING - Only supports non-gesture pointer events!\n");
				createClient(clientAddress, dataPort, DataMode::TacTile, clientSocket);
			}
			else if (handshake.find(inMessage) == 0)
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests data (old handshake) to be sent on port '%d'\n", clientAddress, dataPort);
				createClient(clientAddress, dataPort, DataMode::Omicron, clientSocket);
            }
            else
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests data to be sent on port '%d'\n", clientAddress, dataPort);
                printf("OInputServer: '%s' using unknown handshake '%s'\n", clientAddress, inMessage);
				createClient(clientAddress, dataPort, DataMode::Omicron, clientSocket);
            }
			
            gotData = true;
            //delete inMessage;
            delete portCStr;
        } 
        else if (iResult == 0)
        {
            printf("OInputServer: Closing client connection...\n");
            break;
        }
        else 
        {
            int curTime = time (NULL);
            if( timeoutTime <= curTime )
            {
                printf("OInputServer: Handshake timed out\n");
                break;
            }
        }
    } while (!gotData);
    

    return clientSocket;
}

///////////////////////////////////////////////////////////////////////////////
void InputServer::loop()
{
#ifdef OMICRON_USE_VRPN
    // VRPN connection
    connection->mainloop();
#endif
}

///////////////////////////////////////////////////////////////////////////////
void InputServer::createClient(const char* clientAddress, int dataPort, DataMode legacy, SOCKET clientSocket)
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
			p->second->createSocketConnection(clientAddress, dataPort, legacy, clientSocket);

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

    netClients[addr] = new NetClient( clientAddress, dataPort, legacy, clientSocket );
}
