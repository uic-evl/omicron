/********************************************************************************************************************** 
 * THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2013							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto							anishimoto42@gmail.com
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
#include "omicron/XInputService.h"
#include "omicron/StringUtils.h"

using namespace omicron;

struct XController{
	XInputService::ControllerType type;
	int controllerID;
	bool connected;
	uint myButtonState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XInputService* XInputService::mysInstance = NULL;
int XInputService::maxControllers = 4;

std::map<int,XController*> controllerInfo;
typedef std::map<int,XController*>::iterator controllerIter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XInputService::setup(Setting& settings)
{
	myUpdateInterval = Config::getFloatValue("updateInterval", settings, 0.01f);
	myCheckControllerInterval = Config::getFloatValue("checkControllerInterval", settings, 2.00f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XInputService::initialize() 
{
	omsg("XInputService: Initialize");
	mysInstance = this;

	checkForNewControllers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XInputService::checkForNewControllers() 
{
	for( int i = 0; i < maxControllers; i++ )
	{
		XINPUT_STATE _controllerState;
		ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
		DWORD Result = XInputGetState(i, &_controllerState);

		if( Result == ERROR_SUCCESS )
		{
			if( controllerInfo.count(i) == 0 )
			{
				ofmsg("XInputService: Controller ID %1% Connected", %i);
				XController* controller = new XController();
				controller->controllerID = i;
				controller->connected = true;
				controllerInfo[i] = controller;
			}
			else if( controllerInfo.count(i) == 1 )
			{
				XController* controller = controllerInfo[i];
				if( !controller->connected )
				{
					ofmsg("XInputService: Controller ID %1% Reconnected", %i);
					controller->connected = true;
				}
			}
		}
		else if( Result == ERROR_DEVICE_NOT_CONNECTED )
		{
			if( controllerInfo.count(i) == 1 )
			{
				XController* controller = controllerInfo[i];
				if( controller->connected )
				{
					ofmsg("XInputService: Controller ID %1% Disconnected", %i);
					controller->connected = false;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XInputService::poll() 
{
	static float lastt;
	static float checkControllerLastt;
	float curt = (float)((double)clock() / CLOCKS_PER_SEC);
	if(curt - lastt <= myUpdateInterval)
	{
		return;
	}
	
	if(curt - checkControllerLastt > myCheckControllerInterval)
	{
		checkForNewControllers();
		checkControllerLastt = curt;
	}
	lastt = curt;

	for(controllerIter iterator = controllerInfo.begin(); iterator != controllerInfo.end(); iterator++) {
		XController* controller = iterator->second;

		XINPUT_STATE _controllerState;
		ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
		DWORD Result = XInputGetState(controller->controllerID, &_controllerState);

		if( Result != ERROR_SUCCESS )
			return;

		lockEvents();

		Event* evt = writeHead();

		uint curButtonState = 0;
		// Triangle Button
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) curButtonState |= Event::Button1;
		// Circle Button
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) curButtonState |= Event::Button2;
		// Cross Button
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) curButtonState |= Event::Button3;
		// Square Button
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) curButtonState |= Event::Button4;
		// Left Shoulder Button (L1)
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) curButtonState |= Event::Button5;
		// Left Analog Pad Pressed (L3)
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) curButtonState |= Event::Button6;

		float analog4 = _controllerState.Gamepad.bLeftTrigger / 255.0f;

		// Analog 4 (Wand L2)
		if( analog4 > 0.5 )
			curButtonState |= Event::Button7;

		// NOTE: We are NOT mapping right shoulder, trigger and analog pad buttons.
		// This implementation is mostly targeted at PS3 Move controllers.
		// We could use SpecialButton3 but we leave it out for now.

		// Select Button
		//if(js.rgbButtons[8] & 0x80) curButtonState |= Event::SpecialButton1;
		// Start Button
		//if(js.rgbButtons[11] & 0x80) curButtonState |= Event::SpecialButton2;
		
		// PS Button
		//if(js.rgbButtons[12] & 0x80) curButtonState |= Event::SpecialButton3;

		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) curButtonState |= Event::ButtonUp;
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) curButtonState |= Event::ButtonRight;
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) curButtonState |= Event::ButtonDown;
		if(_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) curButtonState |= Event::ButtonLeft;

		if(curButtonState != controller->myButtonState)
		{
			// If button state is bigger than previous state, it means one additional bit has been
			// set - so send a down event.
			if(curButtonState > controller->myButtonState)
			{
				evt->reset(Event::Down, Service::Controller, controller->controllerID);
				if(isDebugEnabled()) ofmsg("Controller %1% button %2% down", %controller->controllerID %controller->myButtonState);
			}
			else
			{
				evt->reset(Event::Up, Service::Controller, controller->controllerID);
				if(isDebugEnabled()) ofmsg("Controller %1% button %2% up", %controller->controllerID %controller->myButtonState);
			}
			controller->myButtonState = curButtonState;
		}
		else
		{
			// Button state has not changed, just send an update event.
			evt->reset(Event::Update, Service::Controller, controller->controllerID);
		}

		evt->setFlags(controller->myButtonState);

		evt->setExtraDataType(Event::ExtraDataFloatArray);

		evt->setExtraDataFloat(0, _controllerState.Gamepad.sThumbLX / 32767.0f);  // Left analog (-left, +right)
		evt->setExtraDataFloat(1, _controllerState.Gamepad.sThumbLY / -32767.0f);  // Left analog (-up, +down)
		evt->setExtraDataFloat(2, _controllerState.Gamepad.sThumbRX / 32767.0f); // Right analog (-left, +right)
		evt->setExtraDataFloat(3, _controllerState.Gamepad.sThumbRY / 32767.0f); // Right analog (-up, +down)
		evt->setExtraDataFloat(4, _controllerState.Gamepad.bLeftTrigger / 255.0f); // Trigger 2 (+left, -right)
		
		//printf("Created controller %d event \n", j);
		unlockEvents();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XInputService::dispose() 
{
	mysInstance = NULL;
}