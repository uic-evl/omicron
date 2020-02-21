/********************************************************************************************************************** 
 * THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2020							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto							anishimoto42@gmail.com
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

#include "omicron/OpenVRService.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OpenVRService* OpenVRService::mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenVRService::setup(Setting& settings)
{
	//myUpdateInterval = 0.0f;
	//if(settings.exists("updateInterval"))
	//{
	//	myUpdateInterval = settings["updateInterval"];
	//}

	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenVRService::initialize() 
{
	printf("OpenVRService: Initialize\n");
	mysInstance = this;

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Background);

	if (eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;
		ofmsg("Unable to init VR runtime: %1%", %vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return;
	}

	if (!m_pHMD->IsTrackedDeviceConnected(0)) {
		omsg("(OpenVR) HMD Not Connected");
		return;
	}

	// Ensure VR Compositor is avail, otherwise getting poses causes a crash (openvr v1.3.22)
	if (!vr::VRCompositor()) {
		ofmsg("%1% - Failed to initialize VR Compositor!", %__FUNCTION__);
		exit(EXIT_FAILURE);
	}
	else {
		omsg("Successfully initialized VR Compositor");
	}

	vr::EVRInputError inputError;

	// handle for left controller pose
	inputError = vr::VRInput()->GetActionHandle(actionDemoHandLeftPath, &m_actionDemoHandLeft);
	if (inputError != vr::VRInputError_None) {
		ofmsg("Error: Unable to get action handle: %1%", %inputError);
	}
	else {
		ofmsg("Successfully got %1% handle: %2%", %actionDemoHandLeftPath %m_actionDemoHandLeft);
	}

	// handle for right controller pose
	inputError = vr::VRInput()->GetActionHandle(actionDemoHandRightPath, &m_actionDemoHandRight);
	if (inputError != vr::VRInputError_None) {
		ofmsg("Error: Unable to get action handle: %1%", %inputError);
	}
	else {
		ofmsg("Successfully got %1% handle: %2%", %actionDemoHandRightPath %m_actionDemoHandRight);
	}

	
	inputError = vr::VRInput()->GetInputSourceHandle(inputHandLeftPath, &m_inputHandLeftPath);
	if (inputError != vr::VRInputError_None) {
		ofmsg("Error: Unable to get input handle: %1%", %inputError);
	}
	else {
		ofmsg("Successfully got %1% input handle: %2%", %inputHandLeftPath %m_inputHandLeftPath);
	}

	inputError = vr::VRInput()->GetInputSourceHandle(inputHandRightPath, &m_inputHandRightPath);
	if (inputError != vr::VRInputError_None) {
		ofmsg("Error: Unable to get input handle: %1%", %inputError);
	}
	else {
		ofmsg("Successfully got %1% input handle: %2%", %inputHandRightPath %m_inputHandRightPath);
	}

	PrintDevices();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenVRService::poll() 
{
	vr::EVRInputError inputError;

	for (unsigned int deviceId = 0; deviceId < vr::k_unMaxTrackedDeviceCount; deviceId++) {

		vr::TrackedDevicePose_t trackedDevicePose;
		vr::VRControllerState_t controllerState;

		vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(deviceId);

		if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceId)) {
			continue;
		}

		vr::VRSystem()->GetControllerStateWithPose(
			vr::TrackingUniverseStanding, deviceId, &controllerState,
			sizeof(controllerState), &trackedDevicePose);

		//perform actions with pose struct (see next section)
		bool bPoseIsValid = trackedDevicePose.bPoseIsValid;
		bool bDeviceIsConnected = trackedDevicePose.bDeviceIsConnected;
		//ofmsg("Validity: %1% DeviceIsConnected: %2%", %bPoseIsValid %bDeviceIsConnected);

		vr::HmdVector3_t position;
		vr::HmdQuaternion_t quaternion;

		// get the position and rotation
		position = GetPosition(trackedDevicePose.mDeviceToAbsoluteTracking);
		quaternion = GetRotation(trackedDevicePose.mDeviceToAbsoluteTracking);

		//ofmsg("%1% Pose x: %2% y: %3% z: %4%", %deviceId %position.v[0] % position.v[1] % position.v[2]);

		mysInstance->lockEvents();
		Event* evt = mysInstance->writeHead();
		evt->reset(Event::Update, Service::Mocap, deviceId);
		evt->setPosition(position.v[0], position.v[1], position.v[2]);
		evt->setOrientation(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
		evt->setExtraDataType(Event::ExtraDataFloatArray);
		mysInstance->unlockEvents();

		if (deviceClass == vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker) {
		}
	}

	/*
	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}

	vr::TrackedDevicePose_t devicePose;
	if (vr::VRSystem()->PollNextEventWithPose(vr::TrackingUniverseStanding, &event, sizeof(event), &devicePose))
	{
		ProcessVREvent(event);
	}

	// Process SteamVR action state
	// UpdateActionState is called each frame to update the state of the actions themselves. The application
	// controls which action sets are active with the provided array of VRActiveActionSet_t structs.
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_actionSetDemo;
	inputError = vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
	if (inputError == vr::VRInputError_None) {
		// ofmsg("%1% | UpdateActionState(): Ok", %actionDemoHandLeftPath);
	}
	else {
		ofmsg("%1% | UpdateActionState(): Error: ", %actionDemoHandLeftPath %inputError);
	}	
	vr::InputPoseActionData_t poseData;
	inputError = vr::VRInput()->GetPoseActionDataForNextFrame(m_actionDemoHandLeft, vr::TrackingUniverseStanding, &poseData, sizeof(poseData), vr::k_ulInvalidInputValueHandle);
	if (inputError == vr::VRInputError_None) {
		ofmsg("%1% | GetPoseActionData() Ok", %actionDemoHandLeftPath);

		if (poseData.bActive) {
			vr::VRInputValueHandle_t activeOrigin = poseData.activeOrigin;
			bool bPoseIsValid = poseData.pose.bPoseIsValid;
			bool bDeviceIsConnected = poseData.pose.bDeviceIsConnected;
			ofmsg("Origin: %1% Validity: %2% DeviceIsConnected: %3%", %activeOrigin %bPoseIsValid %bDeviceIsConnected);

			// Code below is old\
			vr::HmdVector3_t position;
			vr::HmdQuaternion_t quaternion;

			// get the position and rotation
			position = GetPosition(poseData.pose.mDeviceToAbsoluteTracking);
			quaternion = GetRotation(poseData.pose.mDeviceToAbsoluteTracking);

			// print the tracking data
			//if (printHmdTrackingData) {
			ofmsg("%1% Pose\nx: %2% y: %3% z: %4%", %actionDemoHandLeftPath %position.v[0] %position.v[1] %position.v[2]);
			//sprintf_s(buf, sizeof(buf), "\n%s Pose\nx: %.2f y: %.2f z: %.2f\n", actionDemoHandLeftPath, position.v[0], position.v[1], position.v[2]);
			//printf_s(buf);
			//sprintf_s(buf, sizeof(buf), "qw: %.2f qx: %.2f qy: %.2f qz: %.2f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
			//printf_s(buf);
			//}
		// <--- End of old code


		}
		else {
			ofmsg("%1% | action not avail to be bound", %actionDemoHandLeftPath);
			//sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoHandLeftPath);
			//printf_s(buf);
		}
	}
	else {
		//ofmsg("%1% | GetPoseActionData() Call Not Ok. Error: %2%", %actionDemoHandLeftPath %inputError);
		//sprintf_s(buf, sizeof(buf), "%s | GetPoseActionData() Call Not Ok. Error: %d\n", actionDemoHandLeftPath, inputError);
		//printf_s(buf);
	}

	// Process SteamVR controller state
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
		{
			m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
		}
	}
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenVRService::dispose() 
{
	mysInstance = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void OpenVRService::ProcessVREvent(const vr::VREvent_t & event)
{
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		//SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		ofmsg("Device %1% attached. Setting up render model.", %event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		ofmsg("Device %1% detached.", %event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		ofmsg("Device %1% updated.", %event.trackedDeviceIndex);
	}
	break;
	}
}
/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenVRService::generateEvent( int myConnectionID ) 
{
	
	int deviceStatus = 0;
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_BATTERY);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_POOR_SIGNAL);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_ATTENTION);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_MEDITATION);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_DELTA);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_THETA);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_ALPHA1);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_ALPHA2);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_BETA1);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_BETA2);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_GAMMA1);
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_GAMMA2);
	deviceStatus += TG_GetValueStatus(myConnectionID, 0);

	if( deviceStatus == 0 ) // No change in device, no new event
		return;

	float battery = TG_GetValue(myConnectionID, TG_DATA_BATTERY);
	float signal = TG_GetValue(myConnectionID, TG_DATA_POOR_SIGNAL);
	float attention = TG_GetValue(myConnectionID, TG_DATA_ATTENTION);
	float meditation = TG_GetValue(myConnectionID, TG_DATA_MEDITATION);
	float delta = TG_GetValue(myConnectionID, TG_DATA_DELTA);
	float theta = TG_GetValue(myConnectionID, TG_DATA_THETA);
	float alpha1 = TG_GetValue(myConnectionID, TG_DATA_ALPHA1);
	float alpha2 = TG_GetValue(myConnectionID, TG_DATA_ALPHA2);
	float beta1 = TG_GetValue(myConnectionID, TG_DATA_BETA1);
	float beta2 = TG_GetValue(myConnectionID, TG_DATA_BETA2);
	float gamma1 = TG_GetValue(myConnectionID, TG_DATA_GAMMA1);
	float gamma2 = TG_GetValue(myConnectionID, TG_DATA_GAMMA2);
	float blinkStrength = TG_GetValue(myConnectionID, 0);
	
	int eventSum = signal + attention + meditation + delta + theta;

	if( eventSum == 0 ) // Device is still connecting, don't send event.
		return;

	mysInstance->lockEvents();
	Event* evt = mysInstance->writeHead();
	evt->reset(Event::Update, Service::Brain, myConnectionID);
	evt->setExtraDataType(Event::ExtraDataFloatArray);
	//evt->setExtraDataFloat(0, battery); // Battery seems to always report 0 on the MindWave
	evt->setExtraDataFloat(0, signal);
	evt->setExtraDataFloat(1, attention);
	evt->setExtraDataFloat(2, meditation);
	evt->setExtraDataFloat(3, delta);
	evt->setExtraDataFloat(4, theta);
	evt->setExtraDataFloat(5, alpha1);
	evt->setExtraDataFloat(6, alpha2);
	evt->setExtraDataFloat(7, beta1);
	evt->setExtraDataFloat(8, beta2);
	evt->setExtraDataFloat(9, gamma1);
	evt->setExtraDataFloat(10, gamma2);
	evt->setExtraDataFloat(11, blinkStrength);
	mysInstance->unlockEvents();

	if( isDebugEnabled() )
	{
		omsg("OpenVRService:\n");
		ofmsg(" Battery: %1% Signal: %2% Attention: %3% Meditation %4% \n",
					%battery %signal %attention %meditation );
		ofmsg(" delta: %1% theta: %2% alpha1: %3% beta1 %4% \n\n",
				%delta %theta %alpha1 %beta1 );
	}
	
}
*/

// Get the quaternion representing the rotation
vr::HmdQuaternion_t OpenVRService::GetRotation(vr::HmdMatrix34_t matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}

// Get the vector representing the position
vr::HmdVector3_t OpenVRService::GetPosition(vr::HmdMatrix34_t matrix) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

void OpenVRService::PrintDevices() {

	char buf[1024];
	sprintf_s(buf, sizeof(buf), "\nOpenVR Device list:\n---------------------------\n");
	printf_s(buf);

	// Loop over all conntected devices and print some stuff about them
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		// if no HMD is connected in slot 0 just skip the rest of the code
		// since a HMD must be present.
		if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
			continue;

		// Get what type of device it is and work with its data
		vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
		switch (trackedDeviceClass) {
		case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
			// print stuff for the HMD here, see controller stuff in next case block

			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Device %d: Class: [HMD]", unDevice);
			printf_s(buf);
			break;

		case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
			// do stuff for a controller here
			sprintf_s(buf, sizeof(buf), "Device %d: Class: [Controller]", unDevice);
			printf_s(buf);

			break;

		case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
			// do stuff for a generic tracker here

			sprintf_s(buf, sizeof(buf), "Device %d: Class: [GenericTracker]", unDevice);
			printf_s(buf);
			break;

		case vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference:
			/// do stuff for a tracking reference here

			sprintf_s(buf, sizeof(buf), "Device %d: Class: [TrackingReference]", unDevice);
			printf_s(buf);
			break;

		case vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect:
			/// do stuff for a display redirect here

			sprintf_s(buf, sizeof(buf), "Device %d: Class: [DisplayRedirect]", unDevice);
			printf_s(buf);
			break;

		case vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid:
			// do stuff for an invalid class

			sprintf_s(buf, sizeof(buf), "Device %d: Class: [Invalid]", unDevice);
			printf_s(buf);
			break;

		}

		// print some of the meta data for the device
		char manufacturer[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, manufacturer, sizeof(manufacturer));

		char modelnumber[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, modelnumber, sizeof(modelnumber));

		char serialnumber[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_SerialNumber_String, serialnumber, sizeof(serialnumber));

		bool canPowerOff = vr::VRSystem()->GetBoolTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_DeviceCanPowerOff_Bool);

		sprintf_s(buf, sizeof(buf), " %s - %s [%s] can power off: %d\n", manufacturer, modelnumber, serialnumber, canPowerOff);
		printf_s(buf);
	}
	sprintf_s(buf, sizeof(buf), "---------------------------\nEnd of device list\n\n");
	printf_s(buf);

}