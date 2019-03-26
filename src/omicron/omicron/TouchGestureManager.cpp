/**************************************************************************************************
 * THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2018		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2018, Electronic Visualization Laboratory, University of Illinois at Chicago
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

#include "omicron/TouchGestureManager.h"
#include "omicron/StringUtils.h"
#include "connector/omicronConnectorClient.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Time parameters (milliseconds)
int touchTimeout = 150; // Time since last update until a touch is automatically removed
int touchGroupTimeout = 250; // Time after last touch is removed when the group is automatically removed (double-click triggered if a new touch occurs in this time)

int idleTimeout = 1600; // Time before a non-moving touch is considered idle (should be greater than minPreviousPosTime)
float minPreviousPosTime = 1500; // Time before prevPos of a touch is updated

// Distance parameters (ratio of screen size)
float touchGroupInitialSize = 0.5f; // Desktop 0.5, Cyber-Commons = 0.2?
float touchGroupLongRangeDiameter = 0.6f; // Desktop 0.6, Cyber-Commons = 0.35?
float minimumZoomDistance = 0.1f; // Minimum distance between two touches to be considered for zoom gesture (differentiates between clicks and zooms)
float holdToSwipeThreshold = 0.02f; // Minimum distance before a multi-touch hold gesture is considered a swipe
float clickMaxDistance = 0.02f; // Maximum distance a touch group can be from it's initial point to be considered for a click event

float minPreviousPosDistance = 0.002f; // Min distance for touch prevPos to be updated (min distance for idle touch points to become active)

float zoomGestureMultiplier = 10;

// User Flags: Advanced Touch Gestures Flags
const int GESTURE_UNPROCESSED = EventBase::User << 1; // Not yet identified (allows the first single touch to generate a down event)
const int GESTURE_SINGLE_TOUCH = EventBase::User << 2;
const int GESTURE_BIG_TOUCH = EventBase::User << 3;
const int GESTURE_FIVE_FINGER_HOLD = EventBase::User << 4;
const int GESTURE_FIVE_FINGER_SWIPE = EventBase::User << 5;
const int GESTURE_THREE_FINGER_HOLD = EventBase::User << 6;
const int GESTURE_SINGLE_CLICK = EventBase::User << 7;
const int GESTURE_DOUBLE_CLICK = EventBase::User << 8;
const int GESTURE_MULTI_TOUCH = EventBase::User << 9;
const int GESTURE_ZOOM = EventBase::User << 10;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Group
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGroup::TouchGroup(TouchGestureManager* gm, int ID){
	gestureManager = gm;
	//printf("TouchGroup %d created\n", ID);

	initialDiameter = touchGroupInitialSize;
	longRangeDiameter = touchGroupLongRangeDiameter;
	diameter = initialDiameter;
	this->ID = ID;
	mainID = ID;

	groupHandedness = NONE;

	centerTouch.ID = ID;
	mainTouch.ID = ID;
	eventType = Event::Null;
	gestureFlag = GESTURE_UNPROCESSED;
	remove = false;

	timeb tb;
	ftime( &tb );
	lastUpdated = tb.millitm + (tb.time & 0xfffff) * 1000;

	touchListLock = new Lock();

	fiveFingerGestureTriggered = false;
	threeFingerGestureTriggered = false;
	zoomGestureTriggered = false;
	bigTouchGestureTriggered = false;
	doubleClickTriggered = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGroup::~TouchGroup(){
	delete touchListLock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TouchGroup::getID(){
	return ID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TouchGroup::isInsideGroup( Event::Type eventType, float x, float y, int touchID, float w, float h )
{
	// Check if touch is inside radius of TouchGroup
	if( x > centerTouch.xPos - diameter/2 && x < centerTouch.xPos + diameter/2 && y > centerTouch.yPos - diameter/2 && y < centerTouch.yPos + diameter/2 ){
		addTouch( eventType, x, y, touchID, w, h );
		return true;
	} else if( x > centerTouch.xPos - longRangeDiameter/2 && x < centerTouch.xPos + longRangeDiameter/2 && y > centerTouch.yPos - longRangeDiameter/2 && y < centerTouch.yPos + longRangeDiameter/2 ){
		addLongRangeTouch( eventType, x, y, touchID, w, h );
		return false;
	} else {
		if( longRangeTouchList.count( touchID ) > 0 )
			longRangeTouchList.erase( touchID );
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGroup::addTouch( Event::Type eventType, float x, float y, int touchID, float w, float h ){
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;

	if( eventType == Event::Up ){ // If up cleanup touch

		touchList.erase( touchID );
		idleTouchList.erase( touchID );
		movingTouchList.erase( touchID );
		ofmsg("TouchGroup %1% removed touch ID %2% new size: %3%", %ID %touchID %getTouchCount() );

		if (touchList.size() > 0 && touchID == mainID)
		{
			map<int, Touch>::iterator it;
			Touch t;
			for (it = touchList.begin(); it != touchList.end(); it++)
			{
				t = (*it).second;
				mainTouch = t;
				mainID = t.ID;
				mainTouch.groupID = ID;
				ofmsg("TouchGroup %1% removed main ID %2% NEW main ID: %3%", %ID %touchID %mainID);

				gestureManager->generatePQServiceEvent(Event::Move, mainTouch, touchList, gestureFlag);
				break;
			}
		}
	} else {

		Touch t;
		t.xPos = x;
		t.yPos = y;
		t.ID = touchID;
		t.xWidth = w;
		t.yWidth = h;
		t.timestamp = curTime;
		t.groupID = ID;

		touchListLock->lock();

		if( touchList.count(touchID) == 1 )
		{
			t.initXPos = touchList[touchID].initXPos;
			t.initYPos = touchList[touchID].initYPos;
		}
		else
		{
			t.initXPos = x;
			t.initYPos = y;
		}

		if( touchList.size() == 0 ) // Initial touch group
		{ 
			init_xPos = x;
			init_yPos = y;

			t.state = t.IDLE;
			t.idleTime = curTime;
			t.prevPosResetTime = curTime;

			touchList[touchID] = t;
			mainTouch = t;

			if( ID != touchID )
			{
				doubleClickTriggered = true;
				ofmsg("TouchGroup %1% double click event", %ID );
				gestureManager->generatePQServiceEvent( Event::Down, centerTouch, touchList, GESTURE_DOUBLE_CLICK );
			}
			else
			{
				// New touch down
				gestureFlag = GESTURE_SINGLE_TOUCH;
				gestureManager->generatePQServiceEvent( Event::Down, t, touchList, gestureFlag );

				ofmsg("TouchGroup %1% down event", %ID );
			}
		}
		else
		{
			// Update touch
			t.state = touchList[touchID].state;
			t.lastXPos = touchList[touchID].xPos;
			t.lastYPos = touchList[touchID].yPos;
			touchList[touchID] = t;

			if (mainID == touchID)
			{
				mainTouch = t;
			}
		}

		touchListLock->unlock();			
	}
	lastUpdated = curTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGroup::addLongRangeTouch( Event::Type eventType, float x, float y, int ID, float w, float h ){
	timeb tb;
	ftime( &tb );
	lastUpdated = tb.millitm + (tb.time & 0xfffff) * 1000;

	if( eventType == Event::Up ){ // If up cleanup touch
		longRangeTouchList.erase( ID );
	} else {
		if( longRangeTouchList.count( ID ) == 1 ){ // Update existing touch
			Touch t;
			t.xPos = x;
			t.yPos = y;
			t.xWidth = w;
			t.yWidth = h;
			t.ID = ID;
			t.timestamp = lastUpdated;
			longRangeTouchList[ID] = t;
		} else { // Add new touch
			Touch t;
			t.xPos = x;
			t.yPos = y;
			t.xWidth = w;
			t.yWidth = h;
			t.ID = ID;
			t.timestamp = lastUpdated;
			touchList[ID] = t;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checks the touch group for local gestures
void TouchGroup::process(){
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;
	int timeSinceLastUpdate = curTime-lastUpdated;

	touchListLock->lock();

	// Reset group center position
	float newCenterX = 0;
	float newCenterY = 0;

	map<int,Touch> activeTouchList;
	idleTouchList.clear();

	// Recalculate group center by averaging current touch points
	map<int,Touch>::iterator it;
	for ( it = touchList.begin() ; it != touchList.end(); it++ )
	{
		Touch t = (*it).second;

		// Check touch update time, if too long remove from list
		int lastTouchUpdate = curTime-t.timestamp;
		if( lastTouchUpdate < touchTimeout )
		{
			newCenterX += t.xPos;
			newCenterY += t.yPos;

			// Determine if touch is idle
			t.prevPosTimer = curTime - t.prevPosResetTime;
			if( t.prevPosTimer > idleTimeout )
			{
				if( t.state != t.IDLE )
					t.idleTime = curTime;
				t.state = t.IDLE;
				idleTouchList[t.ID] = t;
			}
			else
			{
				t.state = t.ACTIVE;
			}

			// Determine distance from previous point
			float distFromPrev = sqrt( abs( t.lastXPos - t.xPos ) * abs( t.lastXPos - t.xPos ) + abs( t.lastYPos - t.yPos ) * abs( t.lastYPos - t.yPos ) );
			//ofmsg("Touch ID %1% state: %2% dist from last: %3%", %t.ID %t.state %distFromPrev);

			// TODO: Eventally we'll want to determine the movement vector
			//if( distFromPrev > 0 )
			//  angle = atan2( prevYPos - curYPos, prevXPos - curXPos ) + PI;
			
			// If far enough from previous and enough time has passed, updated previous position
			// Or if previous position is too far away, update
			if( distFromPrev > minPreviousPosDistance ){
				t.prevPosResetTime = curTime;
				//t.lastXPos = t.xPos;
				//t.lastYPos = t.yPos;
			}
			activeTouchList[t.ID] = t;
		}
	}

	// Swap oldlist with active list
	touchList = activeTouchList;

	newCenterX /= touchList.size();
	newCenterY /= touchList.size();

	touchListLock->unlock();

	// Only update the center position if group still has active touches
	// This allows for the empty touch up event to use the last good position
	if( touchList.size() == 0 )
	{
		if( timeSinceLastUpdate > touchGroupTimeout )
		{
			float distFromInitPos = sqrt( abs( init_xPos - centerTouch.xPos ) * abs( init_xPos - centerTouch.xPos ) + abs(init_yPos - centerTouch.yPos ) * abs( init_yPos - centerTouch.yPos ) );
			//ofmsg("Touchgroup %1% distance from initPos %2%", %ID %distFromInitPos);
			
			if( distFromInitPos <= clickMaxDistance && !doubleClickTriggered && !bigTouchGestureTriggered )
			{
				ofmsg("Touchgroup %1% click event", %ID);
				gestureManager->generatePQServiceEvent( Event::Down, mainTouch, touchList, GESTURE_SINGLE_CLICK );
			}
			setRemove();
		}
		return;
	}

	centerTouch.xPos = newCenterX;
	centerTouch.yPos = newCenterY;
	centerTouch.initXPos = init_xPos;
	centerTouch.initYPos = init_yPos;

	// Double click should be the last gesture a touch group will generate
	// to prevent an accidental drag or zoom
	if( !doubleClickTriggered )
		gestureManager->generatePQServiceEvent(Event::Move, mainTouch, touchList, gestureFlag);

	// Determine the farthest point from the group center (thumb?)
	int farthestTouchID = -1;
	farthestTouchDistance = 0;

	for ( it = touchList.begin() ; it != touchList.end(); it++ )
	{
		Touch t = (*it).second;

		float curDistance = sqrt( abs( centerTouch.xPos - t.xPos ) * abs( centerTouch.xPos - t.xPos ) + abs( centerTouch.yPos - t.yPos ) * abs( centerTouch.yPos - t.yPos ) );
		if( curDistance > farthestTouchDistance ){
			farthestTouchDistance = curDistance;
			farthestTouchID = t.ID;
		}
	}

	//ofmsg("TouchGroup: %1% Touches: %2% Idle: %3%", %ID %touchList.size() %idleTouchList.size());
	if( touchList.count(farthestTouchID) != 0 )
	{
		Touch thumbPoint = touchList[farthestTouchID];
		if( touchList.size() >= 3 && xPos < thumbPoint.xPos )
		{
			//omsg("TouchGroup: Left-handed");
			groupHandedness = LEFT;
		}
		else if( touchList.size() >= 3 && xPos > thumbPoint.xPos )
		{
			//omsg("TouchGroup: Right-handed");
			groupHandedness = RIGHT;
		}
		else
		{
			//omsg("TouchGroup: Not-handed");
			groupHandedness = NONE;
		}
	}

	// Double click should be the last gesture a touch group will generate
	// to prevent an accidental drag or zoom
	if( !doubleClickTriggered )
		generateGestures();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gesture Tracking
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGroup::generateGestures(){
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;

	// Single finger gestures
	if( touchList.size() == 1 )
    {
		
		Touch t = touchList[ID];
		mainTouch.xWidth = t.xWidth;
		mainTouch.yWidth = t.yWidth;

		// Big touch
		float bigTouchMinSize = 0.05f;
		if( t.xWidth + t.yWidth > 0 )
		{
			if( t.xWidth > bigTouchMinSize )
			{
				
				gestureFlag =  GESTURE_BIG_TOUCH;

				if( !bigTouchGestureTriggered )
				{
					ofmsg("TouchGroup ID: %1% BIG touch", %ID);
					gestureManager->generatePQServiceEvent( Event::Down, mainTouch, touchList, gestureFlag );
					bigTouchGestureTriggered = true;
				}
				else
				{
					gestureManager->generatePQServiceEvent( Event::Move, mainTouch, touchList, gestureFlag );
				}
			}
			else
			{
				//ofmsg("TouchGroup ID: %1% single touch (idle count: %2% time: %3%)", %ID %idleTouchList.size() %(curTime-t.idleTime) );
				gestureFlag = GESTURE_SINGLE_TOUCH;
			}
			//ofmsg("   size: %1%, %2%", %t.xWidth %t.yWidth);
		}
	}
	else if( touchList.size() > 1 )
	{
		gestureFlag = GESTURE_MULTI_TOUCH;
	}

	// Basic 2-touch zoom
	if( touchList.size() == 2 && idleTouchList.size() <= 1 && !zoomGestureTriggered){
      zoomGestureTriggered = true;
      
      initialZoomDistance = farthestTouchDistance;
      zoomDistance = farthestTouchDistance;
      zoomLastDistance = initialZoomDistance;
      
	  gestureManager->generateZoomEvent(Event::Down, mainTouch, touchList, 0);
	  ofmsg("TouchGroup ID: %1% zoom start", %ID);
    } else if( touchList.size() < 2 && zoomGestureTriggered ){
      zoomGestureTriggered = false;
      
	  gestureManager->generateZoomEvent(Event::Up, mainTouch, touchList, 0);
	   ofmsg("TouchGroup ID: %1% zoom end", %ID);
    }

	if( zoomGestureTriggered )
	{
		zoomLastDistance = zoomDistance;
		zoomDistance = farthestTouchDistance;
      
		float zoomDelta = (zoomDistance - zoomLastDistance) * zoomGestureMultiplier;
		
		if( zoomDelta != 0 )
		{
			gestureManager->generateZoomEvent(Event::Move, mainTouch, touchList, zoomDelta);
			// ofmsg("TouchGroup ID: %1% zoom delta: %2%", %ID %zoomDelta);
		}
	}
	else
	{

		// 5-finger gesture
		if (touchList.size() == 5 && idleTouchList.size() > 3 && !fiveFingerGestureTriggered)
		{
			gestureManager->generatePQServiceEvent(Event::Down, mainTouch, touchList, GESTURE_FIVE_FINGER_HOLD);
			fiveFingerGestureTriggered = true;

			ofmsg("TouchGroup ID: %1% 5-finger gesture triggered", %ID);
			if (groupHandedness == LEFT)
				omsg("   - Left-hand detected");
			else if (groupHandedness == RIGHT)
				omsg("   - Right-hand detected");
		}
		else if (touchList.size() == 5 && idleTouchList.size() > 3 && fiveFingerGestureTriggered)
		{
			gestureManager->generatePQServiceEvent(Event::Move, mainTouch, touchList, GESTURE_FIVE_FINGER_HOLD);
			//ofmsg("TouchGroup ID: %1% 5-finger gesture hold", %ID);
		}
		else if (touchList.size() != 5 && fiveFingerGestureTriggered)
		{
			gestureManager->generatePQServiceEvent(Event::Up, mainTouch, touchList, GESTURE_FIVE_FINGER_HOLD);
			fiveFingerGestureTriggered = false;

			ofmsg("TouchGroup ID: %1% 5-finger gesture ended", %ID);
		}

		// 3-finger gesture
		if (touchList.size() == 3 && idleTouchList.size() > 2 && !threeFingerGestureTriggered)
		{
			gestureManager->generatePQServiceEvent(Event::Down, mainTouch, touchList, GESTURE_THREE_FINGER_HOLD);
			threeFingerGestureTriggered = true;

			ofmsg("TouchGroup ID: %1% 3-finger gesture triggered", %ID);
			if (groupHandedness == LEFT)
				omsg("   - Left-hand detected");
			else if (groupHandedness == RIGHT)
				omsg("   - Right-hand detected");
		}
		else if (touchList.size() == 3 && idleTouchList.size() > 2 && threeFingerGestureTriggered)
		{
			gestureManager->generatePQServiceEvent(Event::Move, mainTouch, touchList, GESTURE_THREE_FINGER_HOLD);
			//ofmsg("TouchGroup ID: %1% 5-finger gesture hold", %ID);
		}
		else if (touchList.size() != 3 && threeFingerGestureTriggered)
		{
			gestureManager->generatePQServiceEvent(Event::Up, mainTouch, touchList, GESTURE_THREE_FINGER_HOLD);
			threeFingerGestureTriggered = false;

			ofmsg("TouchGroup ID: %1% 3-finger gesture ended", %ID);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of touches in the group
int TouchGroup::getTouchCount(){
	return (int)touchList.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the center of the touch group
Touch TouchGroup::getCenterTouch(){
	return centerTouch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the initial touch of the touch group
Touch TouchGroup::getMainTouch() {
	return mainTouch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the gesture flag
int TouchGroup::getGestureFlag(){
	return gestureFlag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the remove flag
bool TouchGroup::isRemovable(){
	return remove;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the remove flag
void TouchGroup::setRemove(){
	remove = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets the zoom delta distance
float TouchGroup::getZoomDelta(){
	if( gestureFlag == Event::Zoom )
		return zoomDistance - zoomLastDistance;
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets the event type
Event::Type TouchGroup::getEventType(){
	return eventType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Gesture Manager
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGestureManager::TouchGestureManager()
{
	//omsg("TouchGestureManager: TouchGestureManager()");
	touchListLock = new Lock();
	touchGroupListLock = new Lock();
	runGestureThread = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::setup( Setting& settings )
{
	touchTimeout = Config::getIntValue("touchTimeout", settings, 150); // Time since last update until a touch is automatically removed
	touchGroupTimeout = Config::getIntValue("touchGroupTimeout", settings, 250); // Time after last touch is removed when the group is automatically removed (double-click triggered if a new touch occurs in this time)

	idleTimeout = Config::getIntValue("idleTimeout", settings, 1600); // Time before a non-moving touch is considered idle (should be greater than minPreviousPosTime)
	minPreviousPosTime = Config::getIntValue("minPreviousPosTime", settings, 1500); // Time before prevPos of a touch is updated

	// Distance parameters (ratio of screen size)
	touchGroupInitialSize = Config::getFloatValue("touchGroupInitialSize", settings, 0.5f); // Desktop 0.5, Cyber-Commons = 0.2?
	touchGroupLongRangeDiameter = Config::getFloatValue("touchGroupLongRangeDiameter", settings, 0.6f); // Desktop 0.6, Cyber-Commons = 0.35?
	minimumZoomDistance = Config::getFloatValue("minimumZoomDistance", settings, 0.1f); // Minimum distance between two touches to be considered for zoom gesture (differentiates between clicks and zooms)
	holdToSwipeThreshold = Config::getFloatValue("holdToSwipeThreshold", settings, 0.02f); // Minimum distance before a multi-touch hold gesture is considered a swipe
	clickMaxDistance = Config::getFloatValue("clickMaxDistance", settings, 0.02f); // Maximum distance a touch group can be from it's initial point to be considered for a click event

	minPreviousPosDistance = Config::getFloatValue("minPreviousPosDistance", settings, 0.002f); // Min distance for touch prevPos to be updated (min distance for idle touch points to become active)

	zoomGestureMultiplier = Config::getFloatValue("zoomGestureMultiplier", settings, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::registerPQService( Service* service )
{
	pqsInstance = service;
	omsg("TouchGestureManager: Registered with " + service->getName());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::poll()
{
	touchGroupListLock->lock();

	map<int,TouchGroup*> newTouchGroupList;

	// Check for empty TouchGroups
	map<int,TouchGroup*>::iterator it;
	for ( it = touchGroupList.begin() ; it != touchGroupList.end(); it++ ){
		TouchGroup* tg = (*it).second;
		tg->process();

		// Add non-empty groups to the list (ignoring empty groups)
		if( !tg->isRemovable() )
		{
			newTouchGroupList[tg->getID()] = tg;
		}
		else
		{
			ofmsg("TouchGestureManager: TouchGroup %1% empty. Removed.", %tg->getID());
			generatePQServiceEvent( Event::Up, tg->getMainTouch(), tg->getGestureFlag() );
		}
	}

	// Update the list
	touchGroupList = newTouchGroupList;

	touchGroupListLock->unlock();
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// All touches considered for gestures are added by this function.
// This also serves to error correct touch data: invalid ranges, missing events, etc.
bool TouchGestureManager::addTouch(Event::Type eventType, Touch touch)
{
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;

	float x = touch.xPos;
	float y = touch.yPos;
	float ID = touch.ID;
	float w = touch.xWidth;
	float h = touch.yWidth;

	// Let the touch groups determine if the touch is new or an update
	addTouchGroup(eventType, x, y, ID, w, h );
	touchList[ID] = touch;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TouchGestureManager::addTouchGroup( Event::Type eventType, float xPos, float yPos, int ID, float xWidth, float yWidth )
{
	touchGroupListLock->lock();

	// Check if new touch is inside an existing TouchGroup
	map<int,TouchGroup*>::iterator it;
	for ( it = touchGroupList.begin() ; it != touchGroupList.end(); it++ ){
		TouchGroup* tg = (*it).second;
		int groupID = (*it).first;

		if( tg->isInsideGroup( eventType, xPos, yPos, ID, xWidth, yWidth ) )
		{
			touchGroupListLock->unlock();
			return true;
		}
	}

	// If touch is not part of existing group, create new
	// TouchGroup using that touch ID
	if( groupedIDs.count(ID) == 0 ){
		ofmsg("TouchID %1% creating new TouchGroup %2%", %ID %ID);
		TouchGroup* newGroup = new TouchGroup(this, ID);
		newGroup->addTouch( eventType, xPos, yPos, ID, xWidth, yWidth );

		touchGroupList[ID] = newGroup;
		
		groupedIDs.insert(ID);
		touchGroupListLock->unlock();
		
		return true;
	}
	touchGroupListLock->unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::generatePQServiceEvent(Event::Type eventType, Touch mainTouch, int gesture)
{
	if (pqsInstance) {
		pqsInstance->lockEvents();

		Event* evt = pqsInstance->writeHead();

		Touch touch = mainTouch;

		switch (eventType)
		{
		case Event::Down:
			evt->reset(Event::Down, Service::Pointer, touch.groupID);
			break;
		case Event::Move:
			evt->reset(Event::Move, Service::Pointer, touch.groupID);
			break;
		case Event::Up:
			evt->reset(Event::Up, Service::Pointer, touch.groupID);
			break;
		}
		evt->setPosition(Vector3f(touch.xPos, touch.yPos, 0));
		evt->setFlags(gesture);

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);
		evt->setExtraDataFloat(2, touch.initXPos);
		evt->setExtraDataFloat(3, touch.initYPos);
		evt->setExtraDataFloat(4, -1); // Secondary event flag (used for zoom)
		evt->setExtraDataFloat(5, 1); // Includes self in count

		map<int, Touch>::iterator it;
		int extraDataIndex = 6;
		evt->setExtraDataFloat(extraDataIndex++, touch.ID);
		evt->setExtraDataFloat(extraDataIndex++, touch.xPos);
		evt->setExtraDataFloat(extraDataIndex++, touch.yPos);

		pqsInstance->unlockEvents();

	}
	else {
		printf("TouchGestureManager: No PQService Registered\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::generatePQServiceEvent(Event::Type eventType, Touch mainTouch, map<int, Touch> touchList, int gesture)
{
	if (pqsInstance){
		pqsInstance->lockEvents();

		Event* evt = pqsInstance->writeHead();

		Touch touch = mainTouch;

		switch (eventType)
		{
		case Event::Down:
			evt->reset(Event::Down, Service::Pointer, touch.groupID);
			break;
		case Event::Move:
			evt->reset(Event::Move, Service::Pointer, touch.groupID);
			break;
		case Event::Up:
			evt->reset(Event::Up, Service::Pointer, touch.groupID);
			break;
		}
		evt->setPosition(Vector3f(touch.xPos, touch.yPos, 0));
		evt->setFlags(gesture);

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);
		evt->setExtraDataFloat(2, touch.initXPos);
		evt->setExtraDataFloat(3, touch.initYPos);
		evt->setExtraDataFloat(4, -1); // Secondary event flag (used for zoom)
		evt->setExtraDataFloat(5, touchList.size()); // Includes self in count

		map<int, Touch>::iterator it;
		int extraDataIndex = 6;
		for (it = touchList.begin(); it != touchList.end(); it++)
		{
			Touch t = (*it).second;

			/*
			if (t.ID != touch.ID) // Does not include itself in this list
			{
				evt->setExtraDataFloat(extraDataIndex++, t.ID);
				evt->setExtraDataFloat(extraDataIndex++, t.xPos);
				evt->setExtraDataFloat(extraDataIndex++, t.yPos);
			}
			*/

			evt->setExtraDataFloat(extraDataIndex++, t.ID);
			evt->setExtraDataFloat(extraDataIndex++, t.xPos);
			evt->setExtraDataFloat(extraDataIndex++, t.yPos);
		}

		

		pqsInstance->unlockEvents();

	}
	else {
		printf("TouchGestureManager: No PQService Registered\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::generateZoomEvent( Event::Type eventType, Touch touch, map<int, Touch> touchList, float zoomDelta )
{
	if( pqsInstance ){
		pqsInstance->lockEvents();

		Event* evt = pqsInstance->writeHead();
		evt->reset(Event::Zoom, Service::Pointer, touch.ID);
	
		evt->setPosition(touch.xPos, touch.yPos);
		evt->setFlags(GESTURE_ZOOM);

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);
		evt->setExtraDataFloat(2, touch.initXPos);
		evt->setExtraDataFloat(3, touch.initYPos);

		switch(eventType)
		{
			case( Event::Down ): evt->setExtraDataFloat(4, 1); break;
			case( Event::Move ): evt->setExtraDataFloat(4, 2); break;
			case( Event::Up ): evt->setExtraDataFloat(4, 3); break;
		}

		evt->setExtraDataFloat(5, zoomDelta);

		map<int, Touch>::iterator it;
		int extraDataIndex = 6;
		for (it = touchList.begin(); it != touchList.end(); it++)
		{
			Touch t = (*it).second;

			evt->setExtraDataFloat(extraDataIndex++, t.ID);
			evt->setExtraDataFloat(extraDataIndex++, t.xPos);
			evt->setExtraDataFloat(extraDataIndex++, t.yPos);
		}

		pqsInstance->unlockEvents();
	} else {
		printf("TouchGestureManager: No PQService Registered\n");
	}
}
