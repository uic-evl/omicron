/**************************************************************************************************
 * THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
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
#ifndef __MOCAP_GESTURE_MANAGER_H__
#define __MOCAP_GESTURE_MANAGER_H__

#include "osystem.h"
#include "ServiceManager.h"
#include <set>

using namespace std;

namespace omicron {
	class GestureService;

	struct Joint{
		int ID; // Trackable / Joint ID
		int userID;
		Vector3f position;
		Quaternion orientation;

		Vector3f lastPosition;
		Quaternion lastOrientation;
	};

	struct MocapUser{
		int userID;

		// Convient access to user's head
		Vector3f position;
		Quaternion orientation;

		map<int,Joint> joints;

		Joint head;
		Joint leftHand;
		Joint rightHand;
		Joint spine;
	};

	class MocapGestureManager : public Service
	{

	public:
		MocapGestureManager( Service* );

		void setup(Setting& settings);
		void poll();
		void processEvent(const Event& e);
		void generateGesture( Event::Type, int userID, int jointID, Vector3f position );
		void generateRotateGesture( Event::Type, int userID, int jointID0, Vector3f position0, int jointID1, Vector3f position1, Vector3f rotation, Vector3f intialRotation );
	private:
		Service* gestureService;
		map<int,MocapUser> userList;

		float lastClickTime;

		// Gesture flags
		static bool handRotateGestureTriggered;

		// Gesture data
		// Two-handed rotation
		float handRotateGestureSeparationTrigger; // Maximum distance (in meters) between hands to trigger gesture
		bool useRadians; // Send data in radians or degrees
		Vector3f initialRotation;
	};
}

#endif