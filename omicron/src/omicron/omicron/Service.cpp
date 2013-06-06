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
#include "omicron/Service.h"
#include "omicron/ServiceManager.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Service::doSetup(ServiceManager* mng, Setting& settings)
{
	myManager = mng;

	// set the service name.
	if(settings.exists("name"))
	{
		myName = (const char*)settings["name"];
	}
	else
	{
		myName = settings.getName();
	}

	// set the debug flag.
	if(settings.exists("debug"))
	{
		myDebug = (bool)settings["debug"];
	}

	// call service specific setup method
	setup(settings);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Service::doInitialize(int serviceId)
{
	if(!myInitialized)
	{
		myId = serviceId;
		initialize();
	}
	else
	{
		ofwarn("Service::doInitialize: %1% already initialized.", %getName());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Service::lockEvents() 
{ 
	myManager->lockEvents(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Service::unlockEvents() 
{ 
	myManager->unlockEvents(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Event* Service::writeHead()
{ 
	Event* evt = myManager->writeHead();
	// Before returning the event, set its service Id.
	evt->myServiceId = myId;
	return evt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Event* Service::readHead()
{ 
	return myManager->readHead();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Event* Service::readTail()
{ 
	return myManager->readTail(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Event* Service::getEvent(int index)
{
	return myManager->getEvent(index);
}
