/**************************************************************************************************
 * THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __TOUCH_GESTURE_MANAGER_H__
#define __TOUCH_GESTURE_MANAGER_H__

#include "osystem.h"
#include "ServiceManager.h"
#include <set>

using namespace std;

struct Touch{
	int ID;
	float xPos;
	float yPos;
	float xWidth;
	float yWidth;
	int timestamp;

	// Gestures
	int gestureType;
};

namespace omicron {
	

	// Holds the initial and current center of mass of touches
	// as well as all touches in the group
	class TouchGroup
	{
		private:
			//TouchGestureManager* parent;
			Touch centerTouch;

			bool remove;
			int gestureFlag;
			int ID;
			float xPos;
			float yPos;
			float init_xPos;
			float init_yPos;
			
			float initialDiameter;
			float longRangeDiameter;
			float diameter;

			int lastUpdated; // Milliseconds

			map<int,Touch> touchList;
			map<int,Touch> longRangeTouchList;

			// Maybe replace these with a function call to generate on demand?
			map<int,Touch> idleTouchList;
			map<int,Touch> movingTouchList;
		public:
			TouchGroup(int);
			int getID();

			bool isInsideGroup( Event::Type eventType, float x, float y, int id );

			void addTouch( Event::Type eventType, float x, float y, int ID );
			void addLongRangeTouch( Event::Type eventType, float x, float y, int ID );

			void process();
			int getTouchCount();
			Touch getCenterTouch();
			int getGestureFlag();
			bool isRemovable();
			void setRemove();
	};

	class TouchGestureManager
	{

	public:
		TouchGestureManager();
		void registerPQService(Service*);
		void setMaxTouchIDs(int);
		void poll();
		
		bool addTouch(Event::Type eventType, Touch touch);
		void setNextID( int ID );
	private:
		Service* pqsInstance;
		Lock* touchListLock;
		Lock* touchGroupListLock;
		map<int,Touch> touchList;
		map<int,TouchGroup*> touchGroupList;
		set<int> groupedIDs;

		// Replaces PQService functionality if GestureManager is enabled
		int touchID[1000]; // Max IDs assigned before resetting
		static int maxTouches; // Should be same number as touchID array init
		static int nextID;
		
		static int deadTouchDelay; 

		bool addTouchGroup( Event::Type eventType, float xPos, float yPos, int id );

		void generatePQServiceEvent(Event::Type eventType, Touch touch, int advancedGesture);
	};
}

#endif