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
#include "omicron/DirectXInputService.h"

using namespace omicron;

LPDIRECTINPUT8 sDI;

int gNumControllers = 0;

struct ControllerInfo{
	DirectXInputService::ControllerType type;
	bool connected;
	bool dataSet;
	bool callbackSet;
	bool rangeSet;
	int controllerID;
	uint myButtonState;
	unsigned long guidInstanceID;
	LPDIRECTINPUTDEVICE8 deviceInterface;
};

std::map<int,ControllerInfo*> controllerInfo;
typedef std::map<int,ControllerInfo*>::iterator controllerIter;
//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK enumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    //HWND hDlg = ( HWND )pContext;

    static int nSliderCount = 0;  // Number of returned slider controls
    static int nPOVCount = 0;     // Number of returned POV controls

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        // Set the range for the axis
		for( int i = 0; i < gNumControllers; i++ )
		{
			if( FAILED( controllerInfo[i]->deviceInterface->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
				return DIENUM_STOP;
		}
    }

    return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK enumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
	// Iterate through added controllers to find controller callback belongs to
	for( int i = 0; i < controllerInfo.size(); i++ )
	{
		ControllerInfo* controller = controllerInfo[i];
		
		// Is this the right controller?
		if( controller->guidInstanceID == pdidInstance->guidInstance.Data1 )
		{
			if( !controller->connected ){
				printf("DirectXInputService: Controller ID %d reconnected.\n",controller->controllerID);
				controller->connected = true;
			}
			return DIENUM_CONTINUE; // Controller exists do not add again
		}
	}

	HRESULT hr; // Result flag
	printf("DirectXInputService: EnumJoysticksCallback called.\n");

	ControllerInfo* newController = new ControllerInfo();
	newController->controllerID = gNumControllers;
	newController->guidInstanceID = pdidInstance->guidInstance.Data1;

	// Obtain an interface to the enumerated joystick.
	if( FAILED( hr = sDI->CreateDevice( pdidInstance->guidInstance, &newController->deviceInterface, NULL ) ) ){
		printf("DirectXInputService: Failed to enumerate joystick.\n");
		return DIENUM_CONTINUE;
	}
	else
	{
		if( pdidInstance->guidProduct.Data1 == 44106846 )
		{ 
			// 'Xbox360' product ID: 44106846
			printf("DirectXInputService: New Xbox 360 controller detected.\n",pdidInstance->guidProduct.Data1);

			newController->type = omicron::DirectXInputService::Xbox360;
		} 
		else if( pdidInstance->guidProduct.Data1 == 50890888 )
		{ 
			// 'PS3 Sixaxis' product ID: 44106846
			printf("DirectXInputService: New PS3 Sixaxis controller detected.\n",pdidInstance->guidProduct.Data1);

			newController->type = omicron::DirectXInputService::PS3;
		}
		else 
		{
			printf("DirectXInputService: New controller detected with GUID %d.\n",pdidInstance->guidProduct.Data1);

			newController->type = omicron::DirectXInputService::Invalid;			
		}
		printf("DirectXInputService: Controller GUID: %u created as ID %d.\n",newController->guidInstanceID,gNumControllers);
		controllerInfo[gNumControllers] = newController;

		gNumControllers++;
		return DIENUM_CONTINUE;
	}

	// Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    //return DIENUM_STOP;
	return DIENUM_CONTINUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DirectXInputService* DirectXInputService::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DirectXInputService::setup(Setting& settings)
{
	myUpdateInterval = Config::getFloatValue("updateInterval", settings, 0.01f);
	myCheckControllerInterval = Config::getFloatValue("checkControllerInterval", settings, 2.00f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DirectXInputService::initialize() 
{
	printf("DirectXInputService: Initialize\n");
	mysInstance = this;

	sDI = NULL; // Pointer to DirectInput8 interface

	HRESULT hr; // Result flag

	// Register with the DirectInput subsystem and get a pointer
	if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&sDI, NULL ) ) )
	{
		printf("DirectXInputService: Failed to register with DirectInput.\n");
		return;
	}
	
	checkForNewControllers();

	if( gNumControllers == 0 )
	{
		printf("DirectXInputService: No joysticks detected.\n");
        return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DirectXInputService::checkForNewControllers() 
{
	HRESULT hr; // Result flag

	//DI_ENUM_CONTEXT enumContext;
	// Enumerate through devices
	if(FAILED(hr = sDI->EnumDevices(DI8DEVCLASS_GAMECTRL, enumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY)))
	{
		printf("DirectXInputService: Failed to enumerate input devices.\n");
	}
	
	 // Make sure we got a joystick
	for(controllerIter iterator = controllerInfo.begin(); iterator != controllerInfo.end(); iterator++)
	{
		if( NULL == iterator->second )
		{
			printf("DirectXInputService: Joystick not found.\n");
			return;
		}
	}

	// Set the data format to "simple joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	for(controllerIter iterator = controllerInfo.begin(); iterator != controllerInfo.end(); iterator++)
	{
		ControllerInfo* controller = iterator->second;
		//if( controllerInfo[i]->dataSet == true )
		//	continue;

		if( FAILED( hr = controller->deviceInterface->SetDataFormat( &c_dfDIJoystick2 ) ) )
		{
			switch(hr)
			{
				//case(DIERR_ACQUIRED):
				//	printf("DirectXInputService: Failed set joystick %d data format. Already acquired.\n", controller->controllerID); break;
				case(DIERR_INVALIDPARAM):
					printf("DirectXInputService: Failed set joystick %d data format. Invalid parameter.\n", controller->controllerID); break;
				case(DIERR_NOTINITIALIZED):
					printf("DirectXInputService: Failed set joystick %d data format. Not initialized.\n", controller->controllerID); break;
			}
			continue;
		}
		printf("DirectXInputService: Joystick %d data format set.\n", controller->controllerID);
		controller->dataSet = true;
	}
	
	// Enumerate the joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    for(controllerIter iterator = controllerInfo.begin(); iterator != controllerInfo.end(); iterator++)
	{
		if(FAILED(hr = iterator->second->deviceInterface->EnumObjects( enumObjectsCallback, NULL, DIDFT_ALL)))
		{
			printf("DirectXInputService: Failed set joystick callback.\n");
			return;
		}
		iterator->second->callbackSet = true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DirectXInputService::poll() 
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

	HRESULT hr;
	DIJOYSTATE2 js;           // DInput joystick state 
	//LPDIDEVICEINSTANCE inputInfo;

	for(controllerIter iterator = controllerInfo.begin(); iterator != controllerInfo.end(); iterator++) {
		ControllerInfo* controller = iterator->second;
		
		LPDIRECTINPUTDEVICE8 curJoystick = controller->deviceInterface;

		if(NULL == curJoystick || !controller->connected) return;

		// Poll the device to read the current state
		hr = curJoystick->Poll();
		if(FAILED(hr))
		{
			if( hr == DIERR_NOTINITIALIZED )
			{
				printf("DirectXInputService: Failed to get input's device state for controller %d. Not initialized.\n",controller->controllerID);
			}

			// DInput is telling us that the input stream has been
			// interrupted. We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done. We
			// just re-acquire and try again.
			hr = curJoystick->Acquire();
			while( hr == DIERR_INPUTLOST )
			{
				hr = curJoystick->Acquire();
			}
			
			if( hr == DIERR_INVALIDPARAM )
			{
				printf("DirectXInputService: Failed to get input's device state for controller %d. Invalid parameter.\n",controller->controllerID);
				continue;
			}
			if( hr == DIERR_NOTINITIALIZED )
			{
				printf("DirectXInputService: Failed to get input's device state for controller %d. Not initialized.\n",controller->controllerID);
			}
			if( hr == DIERR_OTHERAPPHASPRIO )
			{
				printf("DirectXInputService: Failed to get input's device state for controller %d. Other app has priority.\n",controller->controllerID);
			}


			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			continue; // We continue so other connected controllers can still be checked
		}

		// Get the input's device state
		if(FAILED( hr = curJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), &js )))
		{
			printf("DirectXInputService: Failed to get input's device state for controller %d. Likely disconnected.\n",controller->controllerID);
			controller->connected = false;
			controllerInfo.erase( controller->controllerID );
			break; // The device should have been acquired during the Poll()
		}

		// Get the input's device info
		DIDEVICEINSTANCE* info = new DIDEVICEINSTANCE();
		hr = curJoystick->GetDeviceInfo( info );

		//printf("New Event from ControllerID %d with GUID %u\n",controller->controllerID, js.lX);

		lockEvents();

		Event* evt = writeHead();

		uint curButtonState = 0;
		if(controller->type == Xbox360)
		{
			// A Button
			if(js.rgbButtons[0] & 0x80) curButtonState |= Event::Button1;
			// B Button
			if(js.rgbButtons[1] & 0x80) curButtonState |= Event::Button2;
			// X Button
			if(js.rgbButtons[2] & 0x80) curButtonState |= Event::Button3;
			// Y Button
			if(js.rgbButtons[3] & 0x80) curButtonState |= Event::Button4;

			// Left Shoulder Button
			if(js.rgbButtons[4] & 0x80) curButtonState |= Event::Button5;
			// Right Shoulder Button
			if(js.rgbButtons[5] & 0x80) curButtonState |= Event::Button6;

			// Left Analog Pad Pressed
			if(js.rgbButtons[8] & 0x80) curButtonState |= Event::Button7;

			// Right Analog Pad Pressed Missing (would be 9)
			// We could use SpecialButton3 but we leave it out for now.

			// Back Button
			if(js.rgbButtons[6] & 0x80) curButtonState |= Event::SpecialButton1;
			// Start Button
			if(js.rgbButtons[7] & 0x80) curButtonState |= Event::SpecialButton2;

			signed long dpad = (signed long)js.rgdwPOV[0];
			if(dpad == 0) curButtonState |= Event::ButtonUp;
			if(dpad == 9000) curButtonState |= Event::ButtonRight;
			if(dpad == 18000) curButtonState |= Event::ButtonDown;
			if(dpad == 27000) curButtonState |= Event::ButtonLeft;
		}
		else if(controller->type == PS3)
		{
			// Triangle Button
			if(js.rgbButtons[0] & 0x80) curButtonState |= Event::Button1;
			// Circle Button
			if(js.rgbButtons[1] & 0x80) curButtonState |= Event::Button2;
			// Cross Button
			if(js.rgbButtons[2] & 0x80) curButtonState |= Event::Button3;
			// Square Button
			if(js.rgbButtons[3] & 0x80) curButtonState |= Event::Button4;

			// Left Shoulder Button (L1)
			if(js.rgbButtons[4] & 0x80) curButtonState |= Event::Button5;
			// Left Analog Pad Pressed (L3)
			if(js.rgbButtons[9] & 0x80) curButtonState |= Event::Button6;

			// NOTE: We are NOT mapping right shoulder, trigger and analog pad buttons.
			// This implementation is mostly targeted at PS3 Move controllers.
			// We could use SpecialButton3 but we leave it out for now.

			// Select Button
			if(js.rgbButtons[8] & 0x80) curButtonState |= Event::SpecialButton1;
			// Start Button
			if(js.rgbButtons[11] & 0x80) curButtonState |= Event::SpecialButton2;
			
			// PS Button
			if(js.rgbButtons[12] & 0x80) curButtonState |= Event::SpecialButton3;

			signed long dpad = (signed long)js.rgdwPOV[0];
			if(dpad == 0) curButtonState |= Event::ButtonUp;
			if(dpad == 9000) curButtonState |= Event::ButtonRight;
			if(dpad == 18000) curButtonState |= Event::ButtonDown;
			if(dpad == 27000) curButtonState |= Event::ButtonLeft;
		}

		if(curButtonState != controller->myButtonState)
		{
			// If button state is bigger than previous state, it means one additional bit has been
			// set - so send a down event.
			if(curButtonState > controller->myButtonState)
			{
				evt->reset(Event::Down, Service::Controller, controller->controllerID);
				if(isDebugEnabled()) omsg("DirectX button down");
			}
			else
			{
				evt->reset(Event::Up, Service::Controller, controller->controllerID);
				if(isDebugEnabled()) omsg("DirectX button up");
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

		evt->setExtraDataFloat(0, js.lX / 1000.0f);  // Left analog (-left, +right)
		evt->setExtraDataFloat(1, js.lY / 1000.0f);  // Left analog (-up, +down)
		evt->setExtraDataFloat(2, js.lRx / 1000.0f); // Right analog (-left, +right)
		evt->setExtraDataFloat(3, js.lRy / 1000.0f); // Right analog (-up, +down)
		evt->setExtraDataFloat(4, js.lZ / 1000.0f); // Trigger 2 (+left, -right)
		
		//printf("Created controller %d event \n", j);
		unlockEvents();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DirectXInputService::dispose() 
{
	mysInstance = NULL;
}