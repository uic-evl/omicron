/********************************************************************************************************************** 
 * THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2020					Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto							anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * [LICENSE NOTE]
 *---------------------------------------------------------------------------------------------------------------------
 * 
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2020, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __OPENVR__SERVICE_H__
#define __OPENVR__SERVICE_H__

#include "omicron/osystem.h"
#include "omicron/ServiceManager.h"
#include "openvr/openvr.h"
#include <stdlib.h>
#include <stdio.h>

namespace omicron
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OMICRON_API OpenVRService: public Service
{
public:
	// Allocator function
	static OpenVRService* New() { return new OpenVRService(); }

public:
	void setup(Setting& settings);
	virtual void initialize();
	virtual void poll();
	virtual void dispose();

	//! Sets the data update interval, in seconds. This is the interval at which this service will generate events
	//! If set to zero, the service will generate events as fast as possible.
	//void setUpdateInterval(float value);
	//! @see setUpdateInterval
	//float getUpdateInterval();

	void OpenVRService::PrintDevices();
private:
	static OpenVRService* mysInstance;
	//float myUpdateInterval;

	vr::IVRSystem *m_pHMD;
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];
	
	vr::VRActionSetHandle_t m_actionSetDemo = vr::k_ulInvalidActionSetHandle;
	const char *actionSetDemoPath = "/actions/global";
	vr::VRActionHandle_t m_actionDemoHandLeft = vr::k_ulInvalidActionHandle;
	const char *actionDemoHandLeftPath = "/actions/demo/in/Hand_Left";
	vr::VRActionHandle_t m_actionDemoHandRight = vr::k_ulInvalidActionHandle;
	const char *actionDemoHandRightPath = "/actions/demo/in/Hand_Right";

	vr::VRInputValueHandle_t m_inputHandLeftPath = vr::k_ulInvalidInputValueHandle;
	const char *inputHandLeftPath = "/user/hand/left/pose/raw";
	vr::VRInputValueHandle_t m_inputHandRightPath = vr::k_ulInvalidInputValueHandle;
	const char *inputHandRightPath = "/user/hand/right/pose/raw";

	void ProcessVREvent(const vr::VREvent_t & event);

	vr::HmdQuaternion_t OpenVRService::GetRotation(vr::HmdMatrix34_t matrix);
	vr::HmdVector3_t OpenVRService::GetPosition(vr::HmdMatrix34_t matrix);
};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//inline void ThinkGearService::setUpdateInterval(float value) 
	//{ myUpdateInterval = value; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	//inline float ThinkGearService::getUpdateInterval() 
	//{ return myUpdateInterval; }
}; // namespace omicron

#endif