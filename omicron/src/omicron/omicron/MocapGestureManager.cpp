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

#include "omicron/MocapGestureManager.h"

using namespace omicron;
class GestureService;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mocap Gesture Manager
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MocapGestureManager::handRotateGestureTriggered = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MocapGestureManager::MocapGestureManager( Service* service )
{
	gestureService = service;
	//omsg("MocapGestureManager: MocapGestureManager()");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MocapGestureManager::setup(Setting& settings)
{
	handRotateGestureSeparationTrigger = Config::getFloatValue("twoHandedRotateGestureMaxDistance", settings, 0.4f);
	useRadians = Config::getBoolValue("useRadians", settings, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MocapGestureManager::poll()
{
	//ofmsg("UserList size: %1%", %userList.size() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MocapGestureManager::processEvent( const Event& e )
{
	if( e.getServiceType() == Service::Mocap && e.getType() == Event::Update ){
		int userID = e.getSourceId();
		int ID = 0;
		Vector3f pos = e.getPosition();
		Quaternion ori = e.getOrientation();
		
		Vector3f lastPos = pos;
		Quaternion lastOri = ori;

		bool useOpenNI = true;

		if( useOpenNI )
		{

		// New user
		if( userList.count(userID) == 0 )
		{
			MocapUser newUser;
			newUser.userID = userID;
			
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_HEAD) )
				newUser.head.position = e.getExtraDataVector3(Event::OMICRON_SKEL_HEAD);
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_LEFT_HAND) )
				newUser.leftHand.position = e.getExtraDataVector3(Event::OMICRON_SKEL_LEFT_HAND);
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_RIGHT_HAND) )
				newUser.rightHand.position = e.getExtraDataVector3(Event::OMICRON_SKEL_RIGHT_HAND);
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_SPINE) )
				newUser.spine.position = e.getExtraDataVector3(Event::OMICRON_SKEL_SPINE);
			userList[userID] = newUser;
		}
		else // Existing user
		{
			MocapUser user = userList[userID];

			// --- Get Joint Data -----------------------------------------------------------------------------------------------------
			user.head.lastPosition = user.head.position;
			user.leftHand.lastPosition = user.leftHand.position;
			user.rightHand.lastPosition = user.rightHand.position;

			if( !e.isExtraDataNull(Event::OMICRON_SKEL_HEAD) )
				user.head.position = e.getExtraDataVector3(Event::OMICRON_SKEL_HEAD);
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_LEFT_HAND) )
				user.leftHand.position = e.getExtraDataVector3(Event::OMICRON_SKEL_LEFT_HAND);
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_RIGHT_HAND) )
				user.rightHand.position = e.getExtraDataVector3(Event::OMICRON_SKEL_RIGHT_HAND);
			if( !e.isExtraDataNull(Event::OMICRON_SKEL_SPINE) )
				user.spine.position = e.getExtraDataVector3(Event::OMICRON_SKEL_SPINE);
			// ------------------------------------------------------------------------------------------------------------------------

			// --- Calculations for single-handed swipe/click gestures ----------------------------------------------------------------
			float xDiff = user.rightHand.position[0] - user.rightHand.lastPosition[0];
			float yDiff = user.rightHand.position[1] - user.rightHand.lastPosition[1];
			float zDiff = user.rightHand.position[2] - user.rightHand.lastPosition[2];
			
			float zDiff2 = user.leftHand.position[2] - user.leftHand.lastPosition[2];
			
			float curt = (float)((double)clock() / CLOCKS_PER_SEC);

			float swipeThreshold = 0.08f;
			float clickThreshold = 0.08f;
			/*
			if( -xDiff > swipeThreshold )
				ofmsg("%1% Swipe left", %curt);
			else if( xDiff > swipeThreshold )
				ofmsg("%1% Swipe right", %curt);
			else if( -yDiff > swipeThreshold )
				ofmsg("%1% Swipe down", %curt);
			else if( yDiff > swipeThreshold )
				ofmsg("%1% Swipe up", %curt);
			*/
			// ------------------------------------------------------------------------------------------------------------------------

			// Super crude text formatting
			//omsg("\n\n\n");
			//ofmsg("Spine pos (%1% %2% %3%)", %user.spine.position[0] %user.spine.position[1] %user.spine.position[2] );
			//ofmsg("Left hand pos (%1% %2% %3%)", %user.leftHand.position[0] %user.leftHand.position[1] %user.leftHand.position[2] );
			//ofmsg("Right hand pos (%1% %2% %3%)", %user.rightHand.position[0] %user.rightHand.position[1] %user.rightHand.position[2] );
			

			// --- Calculations for two-handed rotation gesture -----------------------------------------------------------------------
			
			float handToHandDist = sqrt( pow(user.leftHand.position[0] - user.rightHand.position[0], 2) +
									pow(user.leftHand.position[1] - user.rightHand.position[1], 2) +
									pow(user.leftHand.position[2] - user.rightHand.position[2], 2) );
			Vector3f gestureRotation;

			//ofmsg("Hand-to-hand distance %1% m", %handToHandDist);

			if( handToHandDist <= handRotateGestureSeparationTrigger && user.leftHand.position[1] > user.spine.position[1] && user.rightHand.position[1] > user.spine.position[1] )
			{
				if( !handRotateGestureTriggered )
				{
					initialRotation[0] = 0;
					initialRotation[1] = 0;
					initialRotation[2] = 0;
				}

				Vector3f handMidpoint;
				handMidpoint[0] = (user.leftHand.position[0] + user.rightHand.position[0]) / 2;
				handMidpoint[1] = (user.leftHand.position[1] + user.rightHand.position[1]) / 2;
				handMidpoint[2] = (user.leftHand.position[2] + user.rightHand.position[2]) / 2;

				float pitch = 0;
				float yaw = atan2( user.rightHand.position[2] - handMidpoint[2], user.rightHand.position[0] - handMidpoint[0] );
				float roll = atan2( user.rightHand.position[1] - handMidpoint[1], user.rightHand.position[0] - handMidpoint[0] );

				if( !useRadians )
				{
					pitch = pitch * 180 / 3.141597f;
					yaw = yaw * 180 / 3.141597f;
					roll = roll * 180 / 3.141597f;
				}

				gestureRotation[0] = pitch;
				gestureRotation[1] = yaw;
				gestureRotation[2] = roll;

				if( !handRotateGestureTriggered )
					initialRotation = gestureRotation; // NOTE: Current implementation works for only one user at a time! Fix later.
				handRotateGestureTriggered = true;
			}
			else if( user.leftHand.position[1] < user.spine.position[1] && user.rightHand.position[1] < user.spine.position[1] )
			{
				handRotateGestureTriggered = false;
			}
			// ------------------------------------------------------------------------------------------------------------------------

			// --- Event generation ---------------------------------------------------------------------------------------------------
			if( handRotateGestureTriggered )
			{
				Vector3f handMidpoint;
				handMidpoint[0] = (user.leftHand.position[0] + user.rightHand.position[0]) / 2;
				handMidpoint[1] = (user.leftHand.position[1] + user.rightHand.position[1]) / 2;
				handMidpoint[2] = (user.leftHand.position[2] + user.rightHand.position[2]) / 2;

				//ofmsg("Hand center pos (%1% %2% %3%)", %handMidpoint[0] %handMidpoint[1] %handMidpoint[2] );

				float pitch = 0;
				float yaw = atan2( user.rightHand.position[2] - handMidpoint[2], user.rightHand.position[0] - handMidpoint[0] );
				float roll = atan2( user.rightHand.position[1] - handMidpoint[1], user.rightHand.position[0] - handMidpoint[0] );

				if( !useRadians )
				{
					pitch = pitch * 180 / 3.141597f;
					yaw = yaw * 180 / 3.141597f;
					roll = roll * 180 / 3.141597f;
				}

				gestureRotation[0] = pitch;
				gestureRotation[1] = yaw;
				gestureRotation[2] = roll;

				float yawDiff = yaw - initialRotation[1];
				float rollDiff = roll - initialRotation[2];

				//ofmsg("Yaw: %1% diff: %2%", %yaw %yawDiff);
				//ofmsg("Roll: %1% diff: %2%", %roll %rollDiff);

				//omsg("Hand rotation gesture triggered");
				generateRotateGesture( Event::Rotate, userID, Event::OMICRON_SKEL_LEFT_HAND, user.leftHand.position,
														Event::OMICRON_SKEL_RIGHT_HAND, user.rightHand.position,
														gestureRotation, initialRotation
														);
			}
			else
			{
				//omsg("");
			}
			// ------------------------------------------------------------------------------------------------------------------------
			//omsg("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

			//float clickTimeout = 
			if( -zDiff > clickThreshold && curt - lastClickTime > 0.5f )
			{
				lastClickTime = curt;
				//ofmsg("%1% Right Click", %curt);

				generateGesture( Event::Click, userID, Event::OMICRON_SKEL_RIGHT_HAND, user.rightHand.position );
			}
			//else if( -zDiff2 > clickThreshold )
			//	omsg("Left Click");

			//ofmsg("RightHand pos(%1%) xDiff (%2%) ", %user.rightHand.position %xDiff );

			userList[userID] = user;
		}
		
		}
		else
		{
		// New user
		if( userList.count(userID) == 0 )
		{
			MocapUser newUser;
			newUser.userID = userID;

			Joint newJoint;
			newJoint.ID = ID;
			newJoint.userID = userID;
			newJoint.position = pos;
			newJoint.orientation = ori;

			newUser.joints[ID] = newJoint;

			userList[userID] = newUser;
		}
		else // Existing user
		{
			MocapUser user = userList[userID];

			// New joint
			if( user.joints.count(ID) == 0 )
			{
				Joint newJoint;
				newJoint.ID = ID;
				newJoint.userID = userID;
				newJoint.position = pos;
				newJoint.orientation = ori;

				user.joints[ID] = newJoint;
			}
			else // Existing joint
			{
				Joint joint;
			
				joint.lastPosition = joint.position;
				joint.lastOrientation = joint.orientation;

				joint.position = pos;
				joint.orientation = ori;

				user.joints[ID] = joint;
				lastPos = joint.lastPosition;
				lastOri = joint.lastOrientation;
			}

			userList[userID] = user;
		}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MocapGestureManager::generateGesture( Event::Type gesture, int userID, int jointID, Vector3f position )
{
	Event* evt;

	evt = gestureService->writeHead();
	evt->reset(gesture, Service::Mocap, userID);
	evt->setPosition(position);
	
	evt->setExtraDataType(Event::ExtraDataIntArray);
	evt->setExtraDataInt(0, jointID);
	gestureService->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MocapGestureManager::generateRotateGesture( Event::Type gesture, int userID, int jointID_0, Vector3f position_0, int jointID_1, Vector3f position_1, Vector3f rotation, Vector3f initRotation )
{
	Vector3f handMidpoint;
	handMidpoint[0] = (position_0[0] + position_1[0]) / 2;
	handMidpoint[1] = (position_0[1] + position_1[1]) / 2;
	handMidpoint[2] = (position_0[2] + position_1[2]) / 2;

	Event* evt;

	evt = gestureService->writeHead();
	evt->reset(gesture, Service::Mocap, userID);
	evt->setPosition(handMidpoint);
	
	evt->setExtraDataType(Event::ExtraDataVector3Array);
	evt->setExtraDataVector3(0, position_0);
	evt->setExtraDataVector3(1, position_1);
	evt->setExtraDataVector3(2, rotation);
	evt->setExtraDataVector3(3, initRotation);
	gestureService->unlockEvents();
}