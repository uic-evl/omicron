/******************************************************************************
 * THE OMICRON PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2014, Electronic Visualization Laboratory,  
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
 *  A service merges mocap and gamepad data to generate 6DOF wand events.
 ******************************************************************************/
#include "omicron/WandService.h"
#include "omicron/Config.h"
#include "omicron/ServiceManager.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////
WandService::WandService():
    myDebug(false),
    myRaySourceId(-1),
    myControllerService(NULL),
    myControllerSourceId(0),
    myPointerXAxisId(2),
    myPointerYAxisId(3)
{
}

///////////////////////////////////////////////////////////////////////////////
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

    // If we have a ray pointer mapper section, use it to setup a ray to 
    // point mapper.
    if(settings.exists("pointer"))
    {
        Setting& sptr = settings["pointer"];
        myPointerXAxisId = Config::getIntValue("xAxisId", sptr, myPointerXAxisId);
        myPointerYAxisId = Config::getIntValue("xAxisId", sptr, myPointerYAxisId);
        myRayPointMapper = RayPointMapper::create(sptr);
    }
}

///////////////////////////////////////////////////////////////////////////////
void WandService::initialize()
{
    setPollPriority(Service::PollLast);
}

///////////////////////////////////////////////////////////////////////////////
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
            myWandUserId = evt->getUserId();
            if(myDebug)
            {
                Vector3f dir = myWandOrientation * -Vector3f::UnitZ();
                ofmsg("Wand ray origin %1%  orientation %2%", %myWandPosition %dir);
            }
        }
        // Attach the mocap ray to wand.
        if(evt->isFrom(myControllerService, myControllerSourceId))
        {
            myFlags = evt->getFlags();
            myExtraDataType = evt->getExtraDataType();
            myExtraDataItems = evt->getExtraDataItems();
            myExtraDataValidMask = evt->getExtraDataMask();
            void* myExtraData = evt->getExtraDataBuffer();
            myType = evt->getType();

            if(myDebug)
            {
                if( evt->isButtonDown(EventBase::Button2) )
                    ofmsg("myRaySourceId %1% serviceName %2% myControllerSourceId %3%", %myRaySourceId %getName() %myControllerSourceId);
            }
            // Re-map the controller event as Wand event with the MocapId
            evt->reset( myType, Service::Wand, myRaySourceId, getServiceId(), myWandUserId );
            evt->setPosition(myWandPosition);
            evt->setOrientation(myWandOrientation);
            evt->setFlags(myFlags);
            evt->setExtraData( myExtraDataType, myExtraDataItems, myExtraDataValidMask, myExtraData );
            
            // If we have a ray to point mapper, save 2D point data in the event.
            if(myRayPointMapper != NULL)
            {
                Ray r(myWandPosition, myWandOrientation * -Vector3f::UnitZ());
                Vector2f pt = myRayPointMapper->getPointFromRay(r);
                evt->setExtraDataFloat(myPointerXAxisId, pt[0]);
                evt->setExtraDataFloat(myPointerYAxisId, pt[1]);
            }
        }
    }

    unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////
void WandService::dispose()
{
}

