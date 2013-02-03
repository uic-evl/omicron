/********************************************************************************************************************** 
* THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto							anishimoto42@gmail.com
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

#include "omicron/ThinkGearService.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ThinkGearService* ThinkGearService::mysInstance = NULL;
int ThinkGearService::driverVersion = 0;
int ThinkGearService::connectionID = 0;
const char* ThinkGearService::comPortName = "\\\\.\\COM10";
bool ThinkGearService::enableStreamLogging = false;
bool ThinkGearService::enableDataLogging = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ThinkGearService::setup(Setting& settings)
{
	if(settings.exists("comPortName"))
	{
		comPortName =  (const char*)settings["comPortName"];
	}
	
	//myUpdateInterval = 0.0f;
	//if(settings.exists("updateInterval"))
	//{
	//	myUpdateInterval = settings["updateInterval"];
	//}

	if( settings.exists("enableStreamLog") ){
		enableStreamLogging = settings["enableStreamLog"];
	}

	if( settings.exists("enableDataLog") ){
		enableDataLogging = settings["enableDataLog"];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ThinkGearService::initialize() 
{
	printf("ThinkGearService: Initialize\n");
	mysInstance = this;

	// Get ThinkGear driver version
	driverVersion = TG_GetDriverVersion();

	// Get the connection ID handle for ThinkGear
	connectionID = TG_GetNewConnectionId();

	if( connectionID < 0 ){
		printf("ThinkGearService: GetConnectionID failed to get ID. Error code: %d \n", connectionID);
		return;
	}
	
	int errorCode = 0;
	if( enableStreamLogging ){
		errorCode = TG_SetStreamLog( connectionID, "streamLog.txt" );
		if( errorCode < 0 ) {
			printf("ThinkGearService: Failed to set stream log. Error code: %d \n", errorCode);
			return;
		}
	}
	
	if( enableDataLogging ){
		errorCode = TG_SetDataLog( connectionID, "dataLog.txt" );
		if( errorCode < 0 ) {
			printf("ThinkGearService: Failed to set data log. Error code: %d \n", errorCode);
			return;
		}
	}

    errorCode = TG_Connect( connectionID, 
                          comPortName, 
                          TG_BAUD_9600, 
                          TG_STREAM_PACKETS );

    if( errorCode < 0 ) {
        printf("ThinkGearService: Failed to connect on '%s'. Error code: %d \n", comPortName, errorCode);
		return;
	} else {
		printf("ThinkGearService: Connected on com port '%s'.\n", comPortName);
		printf("ThinkGearService: Connecting to device (~10 seconds)... \n");
	}

	
	errorCode = TG_EnableBlinkDetection( connectionID, true );
	if( errorCode < 0 ) {
        printf("ThinkGearService: Failed enable blink detection. Error code: %d \n", errorCode);
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ThinkGearService::poll() 
{
	int errorCode = 0;
	int packetsToRead = 1; // Set the number of packets to read. -1 will attempt to read all available packets.
	errorCode = TG_ReadPackets( connectionID, packetsToRead );

	/* If TG_ReadPackets() was able to read a complete Packet of data... */
	if( errorCode == 1 ) {
		generateEvent( connectionID );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ThinkGearService::dispose() 
{
	mysInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ThinkGearService::generateEvent( int myConnectionID ) 
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
	deviceStatus += TG_GetValueStatus(myConnectionID, TG_DATA_BLINK_STRENGTH);

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
	float blinkStrength = TG_GetValue(myConnectionID, TG_DATA_BLINK_STRENGTH);
	
	int eventSum = signal + attention + meditation + delta + theta;

	if( eventSum == 0 ) // Device is still connecting, don't send event.
		return;
	//printf(" Battery: %f Signal: %f Attention: %f Meditation %f \n",
	//			battery, signal, attention, meditation );
	//printf(" delta: %f theta: %f alpha1: %f beta1 %f \n\n",
	//			delta, theta, alpha1, beta1 );
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
}