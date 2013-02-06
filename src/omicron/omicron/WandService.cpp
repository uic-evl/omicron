/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/
#include "omicron/WandService.h"
#include "omicron/Config.h"
#include "omicron/ServiceManager.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
WandService::WandService():
	myDebug(false),
	myRaySourceId(0),
	myControllerService(NULL),
	myControllerSourceId(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandService::setup(Setting& settings)
{
	String inputType;

	myDebug = Config::getBoolValue("debug", settings);

	myRaySourceId = Config::getIntValue("raySourceId", settings);

	String controllerSvc = Config::getStringValue("controllerService", settings);
	myControllerService = getManager()->findService(controllerSvc);
	if(myControllerService == NULL)
	{
		ofwarn("WandService::setup: could not find input service %1%", %controllerSvc);
	}
	
	myControllerSourceId = Config::getIntValue("controllerSourceId", settings);
	ofmsg("WandService::setup RayID: %1%  ControllerID %2%", %myRaySourceId %myControllerSourceId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandService::initialize()
{
	setPollPriority(Service::PollLast);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandService::poll()
{
	lockEvents();
	int numEvts = getManager()->getAvailableEvents();
	for(int i = 0; i < numEvts; i++)
	{
		Event* evt = getEvent(i);
		// Process mocap events.
		if(evt->isFrom(Service::Mocap, myRaySourceId))
		{
			// Do not mark the mocap event as processed, so clients that do not use the wand service can
			// still receive events from the wand rigid body
			//evt->setProcessed();
			myWandOrientation = evt->getOrientation();
			myWandPosition = evt->getPosition();
			if(myDebug)
			{
				Vector3f dir = myWandOrientation * -Vector3f::UnitZ();
				ofmsg("Wand ray origin %1%  orientation %2%", %myWandPosition %dir);
			}
		}
		// Attach the mocap ray to pointer.
		if(evt->isFrom(myControllerService, myControllerSourceId))
		{
			evt->setServiceType(Service::Wand);
			evt->setPosition(myWandPosition);
			evt->setOrientation(myWandOrientation);
		}
	}

	unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandService::dispose()
{
}

