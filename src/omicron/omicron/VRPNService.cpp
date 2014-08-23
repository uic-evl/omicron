/******************************************************************************
 * THE OMICRON PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto	    anishimoto42@gmail.com
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
 *  A service that reads motion capture data through the VRPN protocol.
 ******************************************************************************/

#include "omicron/VRPNService.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////
// User routine to handle a tracker position update.  This is called when
// the tracker callback is called (when a message from its counterpart
// across the connection arrives).
/* // For additional information http://www.cs.unc.edu/Research/vrpn/vrpn_Tracker_Remote.html
typedef	struct {
    struct timeval	msg_time;	// Time of the report
    long		sensor;		// Which sensor is reporting
    double		pos[3];		// Position of the sensor
    double		quat[4];	// Orientation of the sensor
} vrpn_TRACKERCB; */
void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
    VRPNStruct* vs = ((VRPNStruct*)userdata);
    VRPNService* vrpnService = vs->vrnpService;

    vrpnService->generateEvent(t, vs->object_id, vs->userId, vs->jointId);
}

///////////////////////////////////////////////////////////////////////////////
void VRPNService::setup(Setting& settings)
{
    if(settings.exists("serverIP"))
    {
        server_ip =  (const char*)settings["serverIP"];
    }

    if(settings.exists("trackedObjects"))
    {
        Setting& strs = settings["trackedObjects"];
        for(int i = 0; i < strs.getLength(); i++)
        {
            Setting& str = strs[i];
            TrackerInfo trackerInfo;
            trackerInfo.object_name = (const char*)str["name"];
            if( str.exists("serverIP") ){
                trackerInfo.server_ip = (const char*)str["serverIP"]; // Use object specified IP
            } else {
                trackerInfo.server_ip = server_ip; // Use global IP
            }

            if(str.exists("userId"))
            {
                unsigned int uid = (unsigned int)str["userId"];
                trackerInfo.userId = (unsigned short)uid;
            }
            else
            {
                trackerInfo.userId = 0;
            }

            if(str.exists("jointId"))
            {
                String jointName = (const char*)str["jointId"];
                trackerInfo.jointId = Event::parseJointName(jointName);
            }
            else
            {
                trackerInfo.jointId = -1;
            }

            trackerInfo.trackableId = str["trackableID"];
            trackerNames.push_back(trackerInfo);

        }

        myUpdateInterval = 0.0f;
        if(settings.exists("updateInterval"))
        {
            myUpdateInterval = settings["updateInterval"];
        }
    }
    setPollPriority(Service::PollFirst);
}

///////////////////////////////////////////////////////////////////////////////
VRPNService* VRPNService::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
void VRPNService::initialize() 
{
    printf("VRPNService: Initialize\n");
    mysInstance = this;


    for(int i = 0; i < trackerNames.size(); i++)
    {
        TrackerInfo& t = trackerNames[i];
        char trackerName[256];
        strcpy(trackerName,t.object_name);
        strcat(trackerName,"@");
        strcat(trackerName,t.server_ip);
        ofmsg("Added %1%" , %trackerName);

        // Open the tracker: '[object name]@[tracker IP]'
        vrpn_Tracker_Remote *tkr = new vrpn_Tracker_Remote(trackerName);

        VRPNStruct* vrpnData = new VRPNStruct();
        vrpnData->object_name = t.object_name;
        vrpnData->object_id = t.trackableId;
        vrpnData->userId = t.userId;
        vrpnData->jointId = t.jointId;
        vrpnData->vrnpService = this;

        // Set up the tracker callback handler
        tkr->register_change_handler((void*)vrpnData, handle_tracker);

        // Add to tracker remote list
        trackerRemotes.push_back(tkr);
    }
}

///////////////////////////////////////////////////////////////////////////////
void VRPNService::poll() 
{
    static float lastt;
    float curt = (float)((double)clock() / CLOCKS_PER_SEC);
    if(curt - lastt > myUpdateInterval)
    {
        for(int i = 0; i < trackerRemotes.size(); i++)
        {
            vrpn_Tracker_Remote *tkr = trackerRemotes[i];
            // Let the tracker do it's thing
                   // It will call the callback funtions you registered above
                // as needed
            tkr->mainloop();
        }
        lastt = curt;
    }
}

///////////////////////////////////////////////////////////////////////////////
void VRPNService::dispose() 
{
    mysInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void VRPNService::generateEvent(vrpn_TRACKERCB t, int id, unsigned short userId, int jointId) 
{
     //static float lastt;
     //float curt = (float)((double)clock() / CLOCKS_PER_SEC);
     //if(curt - lastt > mysInstance->myUpdateInterval)
     //{
         mysInstance->lockEvents();
         Event* evt = mysInstance->writeHead();
         evt->reset(Event::Update, Service::Mocap, id, 0, userId);
         evt->setPosition(t.pos[0], t.pos[1], t.pos[2]);

        // //double euler[3];
        // //q_to_euler(euler, t.quat);
         evt->setOrientation(t.quat[3], t.quat[0], t.quat[1], t.quat[2]);

         if(jointId != -1)
         {
             evt->setExtraDataInt(0, jointId);
         }

         mysInstance->unlockEvents();
         //lastt = curt;
     //}
}