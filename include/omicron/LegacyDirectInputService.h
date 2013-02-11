/********************************************************************************************************************** 
* THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2013							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto							anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * [LICENSE NOTE]
 *---------------------------------------------------------------------------------------------------------------------
 * Uses the Microsoft DirectInput component of the DirectX API to access game controllers.
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __LEGACY_DIRECTINPUT__SERVICE_H__
#define __LEGACY_DIRECTINPUT__SERVICE_H__

#include "omicron/osystem.h"
#include "omicron/ServiceManager.h"
#include "omicron/dinput.h"
#include "omicron/wiimote.h"

//#include "resource.h" // contained defines below
#define IDI_MAIN                        102
#define IDD_JOYST_IMM                   103
#define IDR_ACCELERATOR1                103
#define IDC_CLOSE                       1001
#define IDC_X_AXIS                      1010
#define IDC_Y_AXIS                      1011
#define IDC_Z_AXIS                      1012
#define IDC_X_AXIS_TEXT                 1013
#define IDC_Y_AXIS_TEXT                 1014
#define IDC_Z_AXIS_TEXT                 1015
#define IDC_X_ROT_TEXT                  1016
#define IDC_Y_ROT_TEXT                  1017
#define IDC_Z_ROT_TEXT                  1018
#define IDC_SLIDER0_TEXT                1019
#define IDC_X_ROT                       1020
#define IDC_Y_ROT                       1021
#define IDC_Z_ROT                       1022
#define IDC_SLIDER1_TEXT                1023
#define IDC_POV0_TEXT                   1024
#define IDC_POV1_TEXT                   1025
#define IDC_POV2_TEXT                   1026
#define IDC_POV3_TEXT                   1027
#define IDC_SLIDER0                     1030
#define IDC_SLIDER1                     1031
#define IDC_POV                         1040
#define IDC_POV0                        1040
#define IDC_BUTTONS                     1041
#define IDC_POV1                        1042
#define IDC_POV2                        1043
#define IDC_POV3                        1044

namespace omicron
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LegacyDirectInputService: public Service
{
public:
	// Allocator function
	static LegacyDirectInputService* New() { return new LegacyDirectInputService(); }

	enum ControllerType { Xbox360, PS3, Wiimote, Wii_Nunchuk, Wii_MotionPlus };
public:
	void setup(Setting& settings);
	OMICRON_API virtual void initialize();
	virtual void poll();
	OMICRON_API virtual void dispose();
	void checkForNewControllers();

	//! Sets the data update interval, in seconds. This is the interval at which this service will generate events
	//! If set to zero, the service will generate events as fast as possible.
	void setUpdateInterval(float value);
	//! @see setUpdateInterval
	float getUpdateInterval();
private:
	static LegacyDirectInputService* mysInstance;
	float myUpdateInterval;

	wiimote remote;
	

};

}; // namespace omicron

#endif