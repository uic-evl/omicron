/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
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
#include "omicron/GestureService.h"

using namespace omicron;

GestureService* GestureService::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
GestureService::GestureService()
{
	mysInstance = this;
	mocapManager = new MocapGestureManager(mysInstance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GestureService::setup(Setting& settings)
{
	mocapManager->setup(settings);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GestureService::initialize() 
{
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GestureService::poll()
{
	mocapManager->poll();
	ServiceManager* serviceManager = mysInstance->getManager();

	int av = serviceManager->getAvailableEvents();
	if(av != 0)
	{
		// TODO: Instead of copying the event list, we can lock the main one.
		Event evts[OMICRON_MAX_EVENTS];
		serviceManager->getEvents(evts, OMICRON_MAX_EVENTS);
		for( int evtNum = 0; evtNum < av; evtNum++ )
		{
			mocapManager->processEvent(evts[evtNum]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GestureService::dispose() 
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GestureService::onEvent(const omicronConnector::EventData& ed)
{
	Event* evt = mysInstance->writeHead();
	// NOTE: original event service id is substituted by GestureService own service id.
	// This is made because in the local context, original service ids have no meaning, so all events are marked
	// as being originated from GestureService.
	evt->reset((Event::Type)ed.type, (Service::ServiceType)ed.serviceType, ed.sourceId, mysInstance->getServiceId());
	evt->setPosition(ed.posx, ed.posy, ed.posz);
	evt->setOrientation(ed.orw, ed.orx, ed.ory, ed.orz);
	evt->setFlags(ed.flags);
	evt->setExtraData((Event::ExtraDataType)ed.extraDataType, ed.extraDataItems, ed.extraDataMask, (void*)ed.extraData);
}
