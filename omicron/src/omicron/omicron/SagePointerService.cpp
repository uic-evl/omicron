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
#include "omicron/SagePointerService.h"
#include "omicron/Tcp.h"
#include "omicron/StringUtils.h"
#include "omicron/ServiceManager.h"

using namespace omicron;


namespace omicron {
///////////////////////////////////////////////////////////////////////////////////////////////////
class SagePointerConnection: public TcpConnection
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////
	SagePointerConnection(ConnectionInfo ci, SagePointerService* service): 
		TcpConnection(ci),
			myService(service),
			myButtonFlags(0)
	{
		if(myService->forceSourceId()) mySourceId = myService->getForcedSourceId();
		else mySourceId = ci.id;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	virtual void handleData()
	{
		// Read ip address
		readUntil(myBuffer, BufferSize, ':');

		// Read user name
		if(readUntil(myBuffer, BufferSize, ' ') > 0)
		{
			myName = myBuffer;
		}
		
		// Read device name
		readUntil(myBuffer, BufferSize, ' ');

		// Read command
		readUntil(myBuffer, BufferSize, ' ');
		int cmd = atoi(myBuffer);

		switch(cmd)
		{
		case 1:
			{
				handleMoveMessage();
				break;
			}
		case 2:
			{
				handleButtonMessage();
				break;
			}
		case 3:
			{
				handleWheelMessage();
				break;
			}
		case 4:
			handleInfoMessage();
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	virtual void handleClosed()
	{
		ofmsg("Connection closed (id=%1%)", %getConnectionInfo().id);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	void handleWheelMessage()
	{
		// Read wheel
		readUntil(myBuffer, BufferSize, '\n');
		int wheel = atoi(myBuffer);

        myService->lockEvents();
		Event* evt = myService->writeHead();
		evt->reset(Event::Zoom, Service::Pointer, mySourceId);
		evt->setExtraDataType(Event::ExtraDataIntArray);
		evt->setExtraDataInt(0, wheel);
		myService->unlockEvents();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	void handleButtonMessage()
	{
		// Read button
		readUntil(myBuffer, BufferSize, ' ');
		int btn = atoi(myBuffer);

		// Read pressed
		readUntil(myBuffer, BufferSize, '\n');
		int pressed = atoi(myBuffer);

        myService->lockEvents();
		Event* evt = myService->writeHead();
		evt->reset(pressed == 1 ? Event::Down : Event::Up, Service::Pointer, mySourceId);
		evt->setPosition(myPosition[0], myPosition[1]);

		if(pressed == 1)
		{
			if(btn == 1) myButtonFlags |= Event::Left;
			if(btn == 2) myButtonFlags |= Event::Right;
			if(btn == 3) myButtonFlags |= Event::Middle;
		}
		else
		{
			if(btn == 1) myButtonFlags &= ~Event::Left;
			if(btn == 2) myButtonFlags &= ~Event::Right;
			if(btn == 3) myButtonFlags &= ~Event::Middle;
		}

		evt->setFlags(myButtonFlags);
		myService->unlockEvents();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	void handleMoveMessage()
	{
		// Read x
		readUntil(myBuffer, BufferSize, ' ');
		float x = atof(myBuffer);

		// Read y
		readUntil(myBuffer, BufferSize, '\n');
		float y = atof(myBuffer);

		myPosition[0] = x;
		myPosition[1] = (1 - y);

        myService->lockEvents();
		Event* evt = myService->writeHead();
		evt->reset(Event::Move, Service::Pointer, mySourceId);
		evt->setPosition(myPosition[0], myPosition[1]);
		evt->setFlags(myButtonFlags);

		myService->unlockEvents();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	void handleInfoMessage()
	{
		// Read r
		readUntil(myBuffer, BufferSize, ' ');
		int r = atoi(myBuffer);

		// Read g
		readUntil(myBuffer, BufferSize, ' ');
		int g = atoi(myBuffer);

		// Read b
		readUntil(myBuffer, BufferSize, '\n');
		int b = atoi(myBuffer);

        myService->lockEvents();
		Event* evt = myService->writeHead();
		evt->reset(Event::Update, Service::Pointer, mySourceId);
		evt->setPosition((float)r/255, (float)g/255, (float)b/255);
		evt->setExtraDataType(Event::ExtraDataString);
		evt->setExtraDataString(myName);
		myService->unlockEvents();
	}

private:
	static const int BufferSize = 1024;
	char myBuffer[BufferSize];

	SagePointerService* myService;
	uint myButtonFlags;
	String myName;
	Vector2f myPosition;
	int mySourceId;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SagePointerServer: public TcpServer
{
public:
	SagePointerServer(SagePointerService* service):
		myService(service)
		{}

	virtual TcpConnection* createConnection(const ConnectionInfo& ci)
	{
		ofmsg("New sage pointer connection (id=%1%)", %ci.id);
		SagePointerConnection* conn = new SagePointerConnection(ci, myService);
	    return conn;
	}

private:
	SagePointerService* myService;
};
};

///////////////////////////////////////////////////////////////////////////////////////////////////
SagePointerService::SagePointerService():
	myForcedSourceId(-1)
{
	myServer = new SagePointerServer(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SagePointerService::~SagePointerService()
{
	delete myServer;
	myServer = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SagePointerService::setup(Setting& settings)
{
	myForcedSourceId = Config::getIntValue("forcedSourceId", settings, -1);
	myServer->setPort(20005);
	myServer->initialize();
	myServer->start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SagePointerService::poll() 
{
	myServer->poll();
}
