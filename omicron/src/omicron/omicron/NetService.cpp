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
#include "omicron/NetService.h"
using namespace omicron;

NetService* NetService::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
NetService::NetService()
{
	mysInstance = this;
	myClient = new omicronConnector::OmicronConnectorClient(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::setup(Setting& settings)
{
	serverAddress = Config::getStringValue("serverIP", settings, "localhost");
	serverPort = Config::getIntValue("msgPort", settings, 27000); 
	dataPort = Config::getIntValue("dataPort", settings, 7000); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::initialize() 
{
	myClient->connect(serverAddress.c_str(), serverPort, dataPort);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetService::poll()
{
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
	Event* evt = mysInstance->writeHead();
	// NOTE: original event service id is substituted by NetService own service id.
	// This is made because in the local context, original service ids have no meaning, so all events are marked
	// as being originated from NetService.
	evt->reset((Event::Type)ed.type, (Service::ServiceType)ed.serviceType, ed.sourceId, mysInstance->getServiceId());
	evt->setPosition(ed.posx, ed.posy, ed.posz);
	evt->setOrientation(ed.orw, ed.orx, ed.ory, ed.orz);
	evt->setFlags(ed.flags);
	evt->setExtraData((Event::ExtraDataType)ed.extraDataType, ed.extraDataItems, ed.extraDataMask, (void*)ed.extraData);
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
	printf("Dataport set to '%s'\n", port);
	dataPort = port;
}
