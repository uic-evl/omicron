/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Brad McGinnis
 *  Alessandro Febretti		febret@gmail.com
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
#include "omicron/NaturalPointService.h"
#include "omicron/StringUtils.h"

using namespace omicron;

NaturalPointService* NaturalPointService :: myMoCap = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
NaturalPointService::NaturalPointService()
{
	pClient = NULL;
	strcpy ( localIP, "" );
	strcpy ( serverIP, "" );
	castType = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
NaturalPointService::~NaturalPointService()
{
	pClient->Uninitialize();
	delete pClient;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NaturalPointService::initialize()
{
	myMoCap = this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NaturalPointService::setup(Setting& settings)
{
	if( settings.exists( "serverIP" ) )
	{
		strcpy( serverIP, settings[ "serverIP" ] );
	}

	if( settings.exists( "localIP" ) )
	{
		strcpy( serverIP, settings[ "localIP" ] );
	}

	if( settings.exists( "castingType" ) )
	{
		castType = atoi( (const char*) settings[ "castingType" ] );
	}
	Vector3f refTranslation = Vector3f::Zero();
	Matrix3f refLinear = Matrix3f::Identity();
	if(settings.exists("referenceTransform"))
	{
		Setting& srt = settings["referenceTransform"];
		if(srt.exists("referenceTranslation"))
		{
			Setting& st = srt["referenceTranslation"];
			refTranslation.x() = (float)st[0];
			refTranslation.y() = (float)st[1];
			refTranslation.z() = (float)st[2];
		}
		if(srt.exists("referenceLinear"))
		{
			Setting& st = srt["referenceLinear"];
			for(int i = 0; i < 9; i++)
			{
				refLinear(i) = st[i];
			}
		}
	}

	myTransform.linear() = refLinear;
	myTransform.translation() = refTranslation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NaturalPointService::start()
{
	if( pClient == NULL)
	{
		pClient = new NatNetClient( castType ); //0 indicates that it is operating in multicast, if it is changed to 1 it will operate in unicast
	}
	
	//set call back functions
	pClient->SetMessageCallback(messageController);
	pClient->SetVerbosityLevel(Verbosity_Debug);
	pClient->SetDataCallback( frameController, pClient);

	//initialize pClient and ensure that it completed successfully
	int retCode = pClient->Initialize( localIP, serverIP);
	
	if ( retCode != ErrorCode_OK)
	{
		oferror("MOCAP: Unable to connect to server. Error code: %d. Exiting", %retCode);
		exit(1);
	}
	omsg("MOCAP: Initialization succeeded\n");

	// send/receive test request
	omsg("MOCAP: Sending Test Request");
	void* response;
	int nBytes;
	int iResult = pClient->SendMessageAndWait("TestRequest", &response, &nBytes);
	if (iResult == ErrorCode_OK)
	{
		ofmsg("MOCAP: Received: %s", %(char*)response);
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NaturalPointService::stop()
{
	pClient->Uninitialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NaturalPointService::dispose()
{
	delete myMoCap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void __cdecl NaturalPointService::messageController( int msgType, char* msg)
{
	//this is where you write messages, from the NatNet server, to the log
	//Log::Message("MOCAP: %s", msg);

	//this is commented out b/c it is printing messages at every frame and I am unsure if they are worth while
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void __cdecl NaturalPointService::frameController( sFrameOfMocapData* data, void *pUserData)
{
	if( myMoCap )
	{
		myMoCap->lockEvents();
		//process the frame data and store it in events, one event per rigid body
		for( int i = 0; i < data->nRigidBodies; i++)
		{
			//checks to see if the rigid body is being tracked, if all positional data is set to zero then it is more than likely not being tracked
			if ( ( data->RigidBodies[i].x == 0 ) && ( data->RigidBodies[i].y == 0 ) && ( data->RigidBodies[i].z == 0 ) )
			{
				//theEvent->type = Event::Untrace;
				//theEvent->position[0] = 0.0;
				//theEvent->position[1] = 0.0;
				//theEvent->position[2] = 0.0;

				//theEvent->orientation[0] = 0.0;
				//theEvent->orientation[1] = 0.0;
				//theEvent->orientation[2] = 0.0;
				//theEvent->orientation[3] = 0.0;
				//for( int k = 0; k < data->RigidBodies[i].nMarkers; k++)
				//{
				//	Vector3f aPoint;
				//	aPoint[0] = 0.0; //x
				//	aPoint[1] = 0.0; //y
				//	aPoint[2] = 0.0; //z
				//	theEvent->pointSet[k] = aPoint;
				//}
				continue;
			}

			//actions to process each rigid body into an event
			Event* theEvent = myMoCap->writeHead();

			theEvent->reset(Event::Move, Service::Mocap, data->RigidBodies[i].ID);

			//get x,y,z coordinates
			theEvent->setPosition(myMoCap->myTransform * Vector3f(
				data->RigidBodies[i].x,
				data->RigidBodies[i].y,
				data->RigidBodies[i].z));

			// Marker set uncommented for now, no real application using it at the moment.
			// in the future, make this a configurable option
			//get markerset data (the points that define the rigid body)
			//int numberOfMarkers = data->RigidBodies[i].nMarkers;
			//for( int j = 0; j < numberOfMarkers; j++)
			//{
			//	Vector3f aPoint;
			//	aPoint[0] = data->RigidBodies[i].Markers[j][0];//x
			//	aPoint[1] = data->RigidBodies[i].Markers[j][1];//y
			//	aPoint[2] = data->RigidBodies[i].Markers[j][2];//z
			//	theEvent->pointSet[j] = myTransform * aPoint;
			//}

			theEvent->setOrientation(
				data->RigidBodies[i].qw, 
				data->RigidBodies[i].qx, 
				data->RigidBodies[i].qy, 
				data->RigidBodies[i].qz);
		}
		myMoCap->unlockEvents();
	}
}
