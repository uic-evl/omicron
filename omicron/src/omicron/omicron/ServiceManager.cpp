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
#include "omicron/ServiceManager.h"
#include "omicron/StringUtils.h"

// Input services
#include "omicron/HeartbeatService.h"
#include "omicron/WandService.h"
#include "omicron/SagePointerService.h"
#include "omicron/GestureService.h"

// NOTE: OSCService needs to be included before NetService to avoid template
// errors within osc/udp.h
#ifdef OMICRON_USE_SOUND
	#include "omicron/SoundManager.h"
#endif
#ifdef OMICRON_USE_DIRECTINPUT
	#include "omicron/DirectInputService.h"
	#include "omicron/XInputService.h"
	#include "omicron/LegacyDirectInputService.h"
	#include "omicron/WiimoteService.h"
#endif
#ifdef OMICRON_USE_NATURAL_POINT
	#include "omicron/NaturalPointService.h"
#endif
#ifdef OMICRON_USE_KEYBOARD
	#include "omicron/KeyboardService.h"
#endif
#ifdef OMICRON_USE_MOUSE
	#include "omicron/MouseService.h"
#endif
#ifdef OMICRON_USE_NETSERVICE
	#include "omicron/NetService.h"
	#include "omicron/LegacyNetService.h"
#endif
#ifdef OMICRON_USE_PQLABS
	#include "omicron/PQService.h"
#endif
#ifdef OMICRON_USE_OPTITRACK
	#include "omicron/OptiTrackService.h"
#endif
#ifdef OMICRON_USE_OPENNI
	#include "omicron/OpenNIService.h"
#endif
#ifdef OMICRON_USE_KINECT_FOR_WINDOWS
	#include "omicron/MSKinectService.h"
#endif
#ifdef OMICRON_USE_VRPN
	#include "omicron/VRPNService.h"
#endif
#ifdef OMICRON_USE_THINKGEAR
	#include "omicron/ThinkGearService.h"
#endif
using namespace omicron;
using namespace std;

// The maximum number of events stored in the event buffer.
const int ServiceManager::MaxEvents = OMICRON_MAX_EVENTS;

///////////////////////////////////////////////////////////////////////////////////////////////////
ServiceManager::ServiceManager():
	myInitialized(false),
	myEventBuffer(NULL),
	myEventBufferHead(0),
	myEventBufferTail(0),
	myAvailableEvents(0),
	myDroppedEvents(0),
	myServiceIdCounter(0)
{
	myEventBufferLock = new Lock();
	registerDefaultServices();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ServiceManager::~ServiceManager()
{
	delete myEventBufferLock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::registerDefaultServices()
{
	// register standard input services.
	registerService("HeartbeatService", (ServiceAllocator)HeartbeatService::New);
	registerService("WandService", (ServiceAllocator)WandService::New);
	registerService("SagePointerService", (ServiceAllocator)SagePointerService::New);
	registerService("GestureService", (ServiceAllocator)GestureService::New);

#ifdef OMICRON_USE_DIRECTINPUT
	registerService("XInputService", (ServiceAllocator)XInputService::New);
	registerService("DirectInputService", (ServiceAllocator)DirectInputService::New);
	registerService("LegacyDirectInputService", (ServiceAllocator)LegacyDirectInputService::New);
	registerService("WiimoteService", (ServiceAllocator)WiimoteService::New);
#endif
#ifdef OMICRON_USE_MOUSE
	registerService("MouseService", (ServiceAllocator)MouseService::New);
#endif
#ifdef OMICRON_USE_KEYBOARD
	registerService("KeyboardService", (ServiceAllocator)KeyboardService::New);
#endif
#ifdef OMICRON_USE_NATURAL_POINT
	registerService("NaturalPointService", (ServiceAllocator)NaturalPointService::New);
#endif
#ifdef OMICRON_USE_NETSERVICE
	registerService("NetService", (ServiceAllocator)NetService::New);
	registerService("LegacyNetService", (ServiceAllocator)LegacyNetService::New);
#endif
#ifdef OMICRON_USE_PQLABS
	registerService("PQService", (ServiceAllocator)PQService::New);
#endif
#ifdef OMICRON_USE_VRPN
	registerService("VRPNService", (ServiceAllocator)VRPNService::New);
#endif
#ifdef OMICRON_USE_THINKGEAR
	registerService("ThinkGearService", (ServiceAllocator)ThinkGearService::New);
#endif
#ifdef OMICRON_USE_OPTITRACK
	registerService("OptiTrackService", (ServiceAllocator)OptiTrackService::New);
#endif
#ifdef OMICRON_USE_OPENNI
	registerService("OpenNIService", (ServiceAllocator)OpenNIService::New);
#endif
#ifdef OMICRON_USE_KINECT_FOR_WINDOWS
	registerService("MSKinectService", (ServiceAllocator)MSKinectService::New);
#endif
#ifdef OMICRON_USE_OSC
	//registerService("OSCService", (ServiceAllocator)OSCService::New);
#endif
////	 Kinda hack: run application initialize here because for now it is used to register services from
////	 external libraries, so it needs to run before setting up services from the config file.
////#ifdef OMICRON_USE_DISPLAY
////	 Initialize the application object (if present)
////	if(myApplication) myApplication->initialize();
////#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::setupAndStart(Config* cfg)
{
	if(!cfg->isLoaded()) cfg->load();

	// Instantiate services (for compatibility reasons, look under'input' and 'services' sections
	Setting& stRoot = cfg->getRootSetting()["config"];
	if(stRoot.exists("input"))
	{
		setup(stRoot["input"]);
	}
	else if(stRoot.exists("services"))
	{
		setup(stRoot["services"]);
	}
	else
	{
		owarn("Config/InputServices section missing from config file: No services created.");
		return;
	}
	initialize();
	start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::setup(Setting& settings)
{
	for(int i = 0; i < settings.getLength(); i++)
	{
		Setting& stSvc = settings[i];
		String classname = stSvc.getName();
		stSvc.lookupValue("class", classname);
		Service* svc = addService(classname);
		if(svc != NULL)
		{
			svc->doSetup(this, stSvc);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::registerService(const String& svcName, ServiceAllocator creationFunc)
{
	myServiceRegistry.insert(
		ServiceAllocatorDictionary::value_type(std::string(svcName), creationFunc));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ServiceAllocator ServiceManager::findServiceAllocator(const String& svcName)
{
	ServiceAllocatorDictionary::const_iterator elem = myServiceRegistry.find(svcName);
	if(elem == myServiceRegistry.end()) return NULL;
	return elem->second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::initialize()
{
	omsg("ServiceManager::initialize");

	myEventBuffer = new Event[MaxEvents];
	ofmsg("Event buffer allocated. Max events: %1%", %MaxEvents);

	foreach(Service* it, myServices)
	{
		it->doInitialize(myServiceIdCounter);
		myServiceIdCounter++;
	}

	myInitialized = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::dispose()
{
	foreach(Service* it, myServices)
	{
		it->dispose();
	}

	myServices.clear();

	delete[] myEventBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::start()
{
	foreach(Service* it, myServices)
	{
		it->start();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::stop()
{
	foreach(Service* it, myServices)
	{
		it->stop();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::poll()
{
	for(int pollPriority = Service::PollFirst; pollPriority <= Service::PollLast; pollPriority++)
	{
		foreach(Service* svc, myServices)
		{
			if(svc->getPollPriority() == pollPriority) svc->poll();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::addService(Service* service)
{
	myServices.push_back(service);
	if(!service->isInitialized())
	{
		service->doInitialize(myServiceIdCounter++);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Service* ServiceManager::addService(const String& svcClass)
{
	if(myInitialized)
	{
		owarn("ServiceManager::addService: cannot add services after service manager initialization.");
	}
	else
	{
		ServiceAllocator svcAllocator = findServiceAllocator(svcClass);
		if(svcAllocator != NULL)
		{
			// Input service found: create and setup it.
			Service* svc = svcAllocator();
			myServices.push_back(svc);

			ofmsg("Service added: %1%", %svcClass);

			return svc;
		}
		else
		{
			ofwarn("Service not found: %1%", %svcClass);
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceManager::incrementBufferIndex(int index)
{
	index++;
	if(index == ServiceManager::MaxEvents) index = 0;
	return index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceManager::decrementBufferIndex(int index)
{
	if(index == 0) index = ServiceManager::MaxEvents;
	index--;
	return index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceManager::getEvents(Event* ptr, int maxEvents)
{
	int returnedEvents = 0;
	Event* evt;
	
	lockEvents();
	do
	{
		evt = readTail();
		if(evt)	
		{
			memcpy(&ptr[returnedEvents], evt, sizeof(Event));
			returnedEvents++;
		}
	} while(evt && returnedEvents < maxEvents);
	unlockEvents();

	// This is a sort of HACK but it is needed to solve several issues with oinput server and
	// the circular buffer not being implemented correctly. Basically, a getEvents call resets 
	// the event buffer, regardless of how many events have been read. A better fix would require 
	// a deeper rewrite of several ServiceManager methods (namely, getting rid of the circular 
	// buffer machinery and just using a simple poll-read system with dropping of events that 
	// can't be queued.
	clearEvents();

	return returnedEvents;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::lockEvents()
{
	myEventBufferLock->lock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::unlockEvents()
{
	myEventBufferLock->unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Event* ServiceManager::getEvent(int index)
{
	oassert(index >= 0 && index < myAvailableEvents);
	return &myEventBuffer[index];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::clearEvents()
{
	myAvailableEvents = 0;
	myEventBufferHead = 0;
	myEventBufferTail = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Event* ServiceManager::writeHead()
{
	Event* evt = &myEventBuffer[myEventBufferHead];
	myEventBufferHead = incrementBufferIndex(myEventBufferHead);

	// This is not totally exact, we would need an event more to actually start dropping..
	if(myAvailableEvents == MaxEvents)
	{
		myDroppedEvents++;
	}
	else
	{
		myAvailableEvents++;
	}

	return evt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Event* ServiceManager::readHead()
{
	if(myAvailableEvents > 0)
	{
		Event* evt = &myEventBuffer[myEventBufferHead];
		myEventBufferHead = decrementBufferIndex(myEventBufferHead);
		myAvailableEvents--;
		return evt;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Event* ServiceManager::readTail()
{
	if(myAvailableEvents > 0)
	{
		Event* evt = &myEventBuffer[myEventBufferTail];
		myEventBufferTail = incrementBufferIndex(myEventBufferTail);
		myAvailableEvents--;
		return evt;
	}
	return NULL;
}
