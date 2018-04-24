/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2017		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2017, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omicron/NetService.h"
using namespace omicron;

NetService* NetService::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
NetService::NetService()
{
	mysInstance = this;
	myClient = new omicronConnector::OmicronConnectorClient(this);
	connected = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::setup(Setting& settings)
{
	serverAddress = Config::getStringValue("serverIP", settings, "localhost");
	serverPort = Config::getIntValue("msgPort", settings, 27000); 
	dataPort = Config::getIntValue("dataPort", settings, 7000); 
	dataStreamOut = Config::getBoolValue("dataStreamOut", settings, false);
	showDebug = Config::getBoolValue("debug", settings, false);
	reconnectDelay = Config::getIntValue("reconnectDelay", settings, 5000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::initialize() 
{
	init = clock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::poll()
{
	timer = (clock() - init) / (double)CLOCKS_PER_SEC;

	if( !connected )
	{
		printf("NetService: Connecting to %s on port %d \n", serverAddress.c_str(), serverPort);
		if (dataStreamOut)
		{
			connected = myClient->connect(serverAddress.c_str(), serverPort, dataPort, 1);
			streamClient = new NetClient(serverAddress.c_str(), dataPort);
		}
		else
		{
			connected = myClient->connect(serverAddress.c_str(), serverPort, dataPort);
		}
		if (!connected)
		{
			printf("NetService: Attempting reconnection in %d second(s)\n", reconnectDelay / 1000);
#ifdef OMICRON_OS_WIN
			Sleep(reconnectDelay);
#else
			sleep(reconnectDelay / 1000);
#endif
		}
	}
	else if(dataStreamOut)
	{
		ServiceManager* serviceManager = mysInstance->getManager();
		int eventCount = serviceManager->getAvailableEvents();
		serviceManager->lockEvents(); // Lock the main event list
		for (int evtNum = 0; evtNum < eventCount; evtNum++)
		{
			// Get the event
			Event* e = serviceManager->getEvent(evtNum);

			// Only send events that are not processed to prevent resending events
			if (!e->isProcessed())
			{
				if (showDebug)
				{
					printf("NetService: Data out: (id, x, y, z) %d %f %f %f\n", e->getSourceId(), e->getPosition().x(), e->getPosition().y(), e->getPosition().z());
				}

				char* eventPacket = InputServer::createOmicronPacketFromEvent(e);

				streamClient->sendEvent(eventPacket, DEFAULT_BUFLEN);
			}
		}
		serviceManager->unlockEvents();
	}

	// Pings
	if (connected && timer > 3)
	{
		connected = myClient->sendMsg("ping");
		if (!connected)
		{
			printf("NetService: Client %s disconnected \n", serverAddress.c_str());
			myClient->dispose();
			if (dataStreamOut)
			{
				streamClient->dispose();
			}
		}
		init = clock();
	}

	myClient->poll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::dispose() 
{
	myClient->dispose();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::onEvent(const omicronConnector::EventData& ed)
{
	Event* e = mysInstance->writeHead();
	e->deserialize(&ed);

	if (showDebug)
	{
		printf("NetService: Data in: (id, x, y, z) %d %f %f %f\n", e->getSourceId(), e->getPosition().x(), e->getPosition().y(), e->getPosition().z());
	}
    
	/*// NOTE: original event service id is substituted by NetService own service id.
	// This is made because in the local context, original service ids have no meaning, so all events are marked
	// as being originated from NetService.
	evt->reset((Event::Type)ed.type, (Service::ServiceType)ed.serviceType, ed.sourceId, mysInstance->getServiceId());
	evt->setPosition(ed.posx, ed.posy, ed.posz);
	evt->setOrientation(ed.orw, ed.orx, ed.ory, ed.orz);
	evt->setFlags(ed.flags);
	evt->setExtraData((Event::ExtraDataType)ed.extraDataType, ed.extraDataItems, ed.extraDataMask, (void*)ed.extraData);*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::setServer(const String& address, int port) 
{
	serverAddress = address;
	serverPort = port;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::setDataport(int port) 
{
	printf("Dataport set to '%d'\n", port);
	dataPort = port;
}
