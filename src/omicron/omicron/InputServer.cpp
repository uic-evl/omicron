/******************************************************************************
 * THE OMICRON SDK
 *-----------------------------------------------------------------------------
 * Copyright 2010-2019		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2019, Electronic Visualization Laboratory,  
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
#include "omicron/ServiceManager.h"
#include <vector>

#include <time.h>
using namespace omicron;

#include <stdio.h>
#include <iostream>
#include <fstream>

#define OI_WRITEBUF(type, buf, offset, val) *((type*)&buf[offset]) = val; offset += sizeof(type);
#define OI_READBUF(type, buf, offset, val) val = *((type*)&buf[offset]); offset += sizeof(type);

const char* InputServer::handshake = "data_on";
const char* InputServer::omicronHandshake = "omicron_data_on";
const char* InputServer::omicronV2Handshake = "omicronV2_data_on";
const char* InputServer::omicronStreamInHandshake = "omicron_data_in";
const char* InputServer::legacyHandshake = "omicron_legacy_data_on";
const char* InputServer::tactileHandshake = "tactile_data_on";
const char* InputServer::omicronV3Handshake = "omicronV3_data_on";

///////////////////////////////////////////////////////////////////////////////
// Creates an event packet from an Omicron event. Returns the buffer.
char* InputServer::createOmicronPacketFromEvent(const Event* evt)
{
	int offset = 0;

	char* eventPacket = new char[DEFAULT_BUFLEN];

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

	if (evt->getExtraDataType() != Event::ExtraDataNull)
	{
		memcpy(&eventPacket[offset], evt->getExtraDataBuffer(), evt->getExtraDataSize());
	}
	offset += evt->getExtraDataSize();

	return eventPacket;
}

///////////////////////////////////////////////////////////////////////////////
// Creates EventData from an Omicron event packet. Returns the EventData.
omicronConnector::EventData InputServer::createOmicronEventDataFromEventPacket(char* eventPacket)
{
	int offset = 0;
	omicronConnector::EventData ed;

	OI_READBUF(unsigned int, eventPacket, offset, ed.timestamp);
	OI_READBUF(unsigned int, eventPacket, offset, ed.sourceId);
	OI_READBUF(unsigned int, eventPacket, offset, ed.deviceTag);
	OI_READBUF(unsigned int, eventPacket, offset, ed.serviceType);
	OI_READBUF(unsigned int, eventPacket, offset, ed.type);
	OI_READBUF(unsigned int, eventPacket, offset, ed.flags);
	OI_READBUF(float, eventPacket, offset, ed.posx);
	OI_READBUF(float, eventPacket, offset, ed.posy);
	OI_READBUF(float, eventPacket, offset, ed.posz);
	OI_READBUF(float, eventPacket, offset, ed.orw);
	OI_READBUF(float, eventPacket, offset, ed.orx);
	OI_READBUF(float, eventPacket, offset, ed.ory);
	OI_READBUF(float, eventPacket, offset, ed.orz);

	OI_READBUF(unsigned int, eventPacket, offset, ed.extraDataType);
	OI_READBUF(unsigned int, eventPacket, offset, ed.extraDataItems);
	OI_READBUF(unsigned int, eventPacket, offset, ed.extraDataMask);
	memcpy(ed.extraData, &eventPacket[offset], omicronConnector::EventData::ExtraDataSize);

	return ed;
}

///////////////////////////////////////////////////////////////////////////////
// Sets the ServiceManager for accessing event stream
void InputServer::setServiceManager(ServiceManager* sm)
{
	serviceManager = sm;
}

///////////////////////////////////////////////////////////////////////////////
//
void InputServer::sendToClients(char* eventPacket)
{
    
}

///////////////////////////////////////////////////////////////////////////////
// Checks the type of event. If a valid event, creates an event packet and sends to clients.
void InputServer::handleEvent(const Event& evt)
{
    // If the event has been processed locally (i.e. by a filter event service)
    if(evt.isProcessed()) return;
	//if (!serviceManager && evt.isProcessed()) return;

    timeb tb;
    ftime( &tb );
    int timestamp = tb.millitm + (tb.time & 0xfffff) * 1000;

#ifdef OMICRON_USE_VRPN
    vrpnDevice->update(&evt);
#endif
            
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

	memcpy(&eventPacket[offset], evt.getExtraDataBuffer(), evt.getExtraDataSize());

	offset += evt.getExtraDataSize();

	validTacTileEvent = handleTacTileEvent(evt);

    if( showStreamSpeed )
    {
        if( (timestamp - lastOutgoingEventTime) >= 1000 )
        {
            lastOutgoingEventTime = timestamp;
            ofmsg("oinputserver: Outgoing event stream %1% event(s)/sec", %eventCount );
            eventCount = 0;
        }
        else
        {
            eventCount++;
        }
    }

	if (showEventStream)
		printf("oinputserver: Event %d type: %d flags: %d sent at pos %f %f\n", evt.getSourceId(), evt.getType(), evt.getFlags(), evt.getPosition().x(), evt.getPosition().y());

	// Send to clients
	std::map<char*, NetClient*>::iterator itr = netClients.begin();
	while (itr != netClients.end())
	{
		NetClient* client = itr->second;

		// Only send service types client requested
		if (client->requestedServiceType(evt.getServiceType()))
		{

			if (client->getMode() == data_omicron_legacy)
			{
				//client->sendEvent(legacyPacket, 512);
			}
			else if (client->getMode() == data_tactile)
			{
				if (validTacTileEvent)
				{
					client->sendEvent(tacTilePacket, 512);
				}
			}
			else
			{
				if (evt.getType() == Event::Update || evt.getType() == Event::Move)
				{
					client->sendEvent(eventPacket, DEFAULT_BUFLEN);
				}
				else
				{
					// If client supports dual TCP/UDP (V2), send single events as TCP
					if (client->getMode() == data_omicronV2)
					{
						client->sendMsg(eventPacket, DEFAULT_BUFLEN);
					}
					else
					{
						// Legacy support, send single events as UDP
						client->sendEvent(eventPacket, DEFAULT_BUFLEN);
					}
				}
			}
		}
		itr++;
	}
}
    
///////////////////////////////////////////////////////////////////////////////
// Comma-separated string packet
// [serviceType]:[eventType],[sourceID],[x],[y],[other data varying by event type]
bool InputServer::handleLegacyEvent(const Event& evt)
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

///////////////////////////////////////////////////////////////////////////////
// Comma-separated string packet (TacTile version)
// Note dgram has a space appended to end
// d-Flag (OmegaTracker/TacTile Event)
// "[timestamp]:d:[id],[x],[y],[intensity] "
//
// Q-Flag (PQLabs Event)
// "[timestamp]:q:[id],[x],[y],[w],[h],[gesture],[intensity] "
// Gesture: 0 = down, 1 = move, 2 = up, 3 = hold
bool InputServer::handleTacTileEvent(const Event& evt)
{
	//itoa(evt.getServiceType(), eventPacket, 10); // Append input type
	sprintf(tacTilePacket, "%d", evt.getTimestamp());

	strcat(tacTilePacket, ":");

	strcat(tacTilePacket, "q");

	strcat(tacTilePacket, ":");

	char floatChar[32];

	switch (evt.getServiceType())
	{
	case Service::Pointer:
		//printf(" Touch type %d \n", evt.getType()); 
		//printf("               at %f %f \n", x, y ); 

		// Converts source id to char, appends to eventPacket
		sprintf(floatChar, "%d", evt.getSourceId());
		strcat(tacTilePacket, floatChar);
		strcat(tacTilePacket, ","); // Spacer

		// Converts x to char, appends to eventPacket
		sprintf(floatChar, "%f", evt.getPosition().x());
		strcat(tacTilePacket, floatChar);
		strcat(tacTilePacket, ","); // Spacer

		// Converts y to char, appends to eventPacket
		sprintf(floatChar, "%f", evt.getPosition().y());
		strcat(tacTilePacket, floatChar);

		if (evt.getExtraDataItems() == 2) { // TouchPoint down/up/move

			// Converts xWidth to char, appends to eventPacket
			strcat(tacTilePacket, ","); // Spacer
			sprintf(floatChar, "%f", evt.getExtraDataFloat(0));
			strcat(tacTilePacket, floatChar);

			// Converts yWidth to char, appends to eventPacket
			strcat(tacTilePacket, ","); // Spacer
			sprintf(floatChar, "%f", evt.getExtraDataFloat(1));
			strcat(tacTilePacket, floatChar);

		}

		switch (evt.getType())
		{
		case(EventBase::Down):
			// Converts gesture to char, appends to eventPacket
			strcat(tacTilePacket, ","); // Spacer
			sprintf(floatChar, "%d", 0);
			strcat(tacTilePacket, floatChar);
			break;
		case(EventBase::Move):
			// Converts gesture to char, appends to eventPacket
			strcat(tacTilePacket, ","); // Spacer
			sprintf(floatChar, "%d", 1);
			strcat(tacTilePacket, floatChar);
			break;
		case(EventBase::Up):
			// Converts gesture to char, appends to eventPacket
			strcat(tacTilePacket, ","); // Spacer
			sprintf(floatChar, "%d", 2);
			strcat(tacTilePacket, floatChar);
			break;
		}

		// Intensity (Not really used, just to match the old message format
		strcat(tacTilePacket, ","); // Spacer
		sprintf(floatChar, "%f", 1.0);
		strcat(tacTilePacket, floatChar);

		strcat(tacTilePacket, " "); // Spacer

		return true;
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
    serverIP = strdup(Config::getStringValue("serverListenIP", sCfg, "").c_str());

    checkForDisconnectedClients = Config::getBoolValue("checkForDisconnectedClients", sCfg, false );
    showEventStream = Config::getBoolValue("showEventStream", sCfg, false );
    showStreamSpeed = Config::getBoolValue("showStreamSpeed", sCfg, false );
	showEventMessages = Config::getBoolValue("showEventMessages", sCfg, false);
	showIncomingStream = Config::getBoolValue("showIncomingStream", sCfg, false);
	showIncomingMessages = Config::getBoolValue("showIncomingMessages", sCfg, false);

	logClientConnectionsToFile = Config::getBoolValue("logClientConnectionsToFile", sCfg, false);
	clientConnectLogFilePath = strdup(Config::getStringValue("clientLogPath", sCfg, "connectionLog.txt").c_str());

    if( checkForDisconnectedClients )
        omsg("Check for disconnected clients enabled.");

    listenSocket = INVALID_SOCKET;
    recvbuflen = DEFAULT_BUFLEN;

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

    // Initialize socket
    SOCKET_INIT();

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    if( strlen(serverIP) != 0 )
        iResult = getaddrinfo(strdup(serverIP), serverPort, &hints, &result);
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

	// Set listen socket as non-blocking
#ifdef OMICRON_OS_WIN
    u_long iMode = 1; // If iMode != 0, non-blocking mode is enabled.
    ioctlsocket(listenSocket,FIONBIO,&iMode);
#else
	fcntl(listenSocket, F_SETFL, O_NONBLOCK);
#endif

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
        printf("OInputServer: Client '%s' connecting...\n", clientAddress);
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
            char* inMessage;
            char* portCStr;
			char* flagsCStr;
            inMessage = new char[iResult];
            portCStr = new char[iResult];
			flagsCStr = new char[iResult];

			int readState = 0; // 0 = message, 1 = dataPort, 2 = clientParameters
            // Iterate through message string and
            // separate 'data_on,' from the port number
            int portIndex = iResult;
			int flagIndex = iResult;
            for( int i = 0; i < iResult; i++ )
            {
                if( recvbuf[i] == ',' && readState == 0)
                {
                    portIndex = i + 1;
                    inMessage[i] = '\0';
					readState = 1;
                }
				else if (recvbuf[i] == ',' && readState == 1)
				{
					flagIndex = i + 1;
					readState = 2;
				}
                else if( i < portIndex )
                {
                    inMessage[i] = recvbuf[i];
                } 
                else if(readState == 1)
                {
                    portCStr[i-portIndex] = recvbuf[i];
                    portCStr[i-portIndex+1] = '\n';
                }
				else if (readState == 2)
				{
					flagsCStr[i - flagIndex] = recvbuf[i];
					flagsCStr[i - flagIndex + 1] = '\n';
				}
            }

            // Make sure handshake is correct
            int dataPort = 7000; // default port

			std::ofstream clientLogFile;

			if (logClientConnectionsToFile)
			{
				timeb tb;
				ftime(&tb);
				int timestamp = tb.time;

				clientLogFile.open(clientConnectLogFilePath, std::ios::app);

				std::string clientAddrStr(clientAddress);
				std::string serverAddrStr(serverIP);
				clientLogFile << timestamp;
				clientLogFile << " " + serverAddrStr + ":";
				clientLogFile << atoi(serverPort);
				clientLogFile << " " + clientAddrStr;
				clientLogFile << ":";
				clientLogFile << atoi(portCStr);
			}

            if( strcmp(inMessage, legacyHandshake) == 0 )
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests omicron legacy data to be sent on port '%d'\n", clientAddress, dataPort);
                printf("OInputServer: WARNING - This server does not support legacy data!\n");
                createClient( clientAddress, dataPort, data_omicron_legacy, clientSocket );

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED LEGACY";
            }
            else if( strcmp(inMessage, omicronHandshake) == 0 )
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests omicron data to be sent on port '%d'\n", clientAddress, dataPort);
                createClient( clientAddress, dataPort, data_omicron, clientSocket );

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED OMICRON V1";
            }
			else if (strcmp(inMessage, omicronV2Handshake) == 0)
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests omicron 2.0 (Dual TCP/UDP) data to be sent on port '%d'\n", clientAddress, dataPort);
				createClient(clientAddress, dataPort, data_omicronV2, clientSocket);

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED OMICRON V2";
			}
			else if (strcmp(inMessage, omicronV3Handshake) == 0)
			{
				// Get data port number
				dataPort = atoi(portCStr);
				int flags = atoi(flagsCStr);
				printf("OInputServer: '%s' requests omicron 3.0 (Client flags) data to be sent on port '%d' with flag '%d'\n", clientAddress, dataPort, flags);
				createClient(clientAddress, dataPort, data_omicronV3, clientSocket, flags);

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED OMICRON V3";
			}
			else if (strcmp(inMessage, omicronStreamInHandshake) == 0)
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests to SEND omicron data to be RECEIVED on port '%d'\n", clientAddress, dataPort);
				createClient(clientAddress, dataPort, data_omicron_in, clientSocket);

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED OMICRON DATAIN";
			}
			else if (strcmp(inMessage, tactileHandshake) == 0)
			{
				// Get data port number
				dataPort = atoi(portCStr);
				printf("OInputServer: '%s' requests TacTile dgram data to be sent on port '%d'\n", clientAddress, dataPort);
				createClient(clientAddress, dataPort, data_tactile, clientSocket);

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED TACTILE";
			}
            else if( strcmp(inMessage, handshake) == 0 )
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests data (old handshake) to be sent on port '%d'\n", clientAddress, dataPort);
                createClient( clientAddress, dataPort, data_omicron, clientSocket );

				if (logClientConnectionsToFile) clientLogFile << " ACCEPTED OMICRON OLD";
            }
            else
            {
                // Get data port number
                dataPort = atoi(portCStr);
                printf("OInputServer: '%s' requests data to be sent on port '%d'\n", clientAddress, dataPort);
                printf("OInputServer: '%s' using unknown handshake '%s'\n", clientAddress, inMessage);

				if (logClientConnectionsToFile)
				{
					std::string inMsgStr(inMessage);
					clientLogFile << " REJECTED '";
					clientLogFile << inMsgStr;
					clientLogFile << "'";
				}
            }

			if (logClientConnectionsToFile)
			{
				clientLogFile << "\n";
				clientLogFile.close();
			}

            gotData = true;
            delete inMessage;
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

	std::map<char*, NetClient*>::iterator p;
	for (p = netClients.begin(); p != netClients.end(); p++)
	{
		char* clientAddress = p->first;
		NetClient* client = p->second;

		if ( client->isReceivingData() )
		{
			// Grab data from client
			int iresult = client->recvEvent(eventPacket, DEFAULT_BUFLEN);
			if (iresult > 0)
			{
				// Convert client packet to omicron event
				omicronConnector::EventData ed = createOmicronEventDataFromEventPacket(eventPacket);

				if (showIncomingStream)
				{
					printf("InputServer: Data in id: %d pos: %f %f %f\n", ed.sourceId, ed.posx, ed.posy, ed.posz);
				}
				if (showIncomingMessages && ed.serviceType == EventBase::ServiceTypeSpeech)
				{
					Event e;
					e.deserialize(&ed);
					printf("NetService: Speech in: (speech text, condidence) '%.*s' %f\n", e.getExtraDataItems(), e.getExtraDataString(), ed.posx);
				}

				// Add to local service manager's event list
				if (serviceManager)
				{
					serviceManager->lockEvents();
					Event* e = serviceManager->writeHead();
					e->deserialize(&ed);

					serviceManager->unlockEvents();
				}
			}
			else
			{
				// printf("InputServer: No data\n");
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void InputServer::createClient(const char* clientAddress, int dataPort, DataMode mode, SOCKET clientSocket, int flags)
{
	if (flags == -1)
	{
		flags = NetClient::GetDefaultFlag();
	}

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
	bool clientExists = false;
    std::map<char*, NetClient*>::iterator p;
    for(p = netClients.begin(); p != netClients.end(); p++) 
    {
        //printf( "%s \n", p->first );
        if( strcmp(p->first, addr) == 0 )
        {
            printf("OInputServer: NetClient already exists: %s \n", addr );
			clientExists = true;
			p->second->updateClientSocket(clientSocket);
			p->second->updateFlags(flags);

            // Check dataMode: if different, update client
            if( p->second->getMode() != mode )
            {
                if(mode == data_omicron_legacy)
                {
                    printf("OInputServer: NetClient %s now requesting to receive legacy omicron data \n", addr );
                    printf("OInputServer: WARNING - This server does not support legacy data!\n");
                }
				else if (mode == data_omicron_in)
				{
					printf("OInputServer: NetClient %s now requesting to send omicron data \n", addr);
				}
				else if (mode == data_tactile)
				{
					printf("OInputServer: NetClient %s now requesting to receive Tactile dgram data \n", addr);
				}
				else if (mode == data_omicronV2)
				{
					printf("OInputServer: NetClient '%s' now requesting omicron 2.0 (Dual TCP/UDP) data \n", addr);
				}
				else if (mode == data_omicronV3)
				{
					printf("OInputServer: NetClient '%s' now requesting omicron 3.0 (Client flags) data \n", addr);
				}
                else
                    printf("OInputServer: NetClient %s now requesting to receive omicron data \n", addr );
                p->second->setMode(mode);
            }
        }
    }

	if (!clientExists)
	{
		netClients[addr] = new NetClient(clientAddress, dataPort, mode, clientSocket, flags);
	}
}
