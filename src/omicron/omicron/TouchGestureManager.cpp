/**************************************************************************************************
 * THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2014, Electronic Visualization Laboratory, University of Illinois at Chicago
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

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Time parameters (milliseconds)
int touchTimeout = 150; // Time since last update until touch group is automatically removed
int doubleClickDelay = 100; // Time between the first and second click to trigger DoubleClick event

int idleTimeout = 600; // Time before a non-moving touch is considered idle (should be greater than minPreviousPosTime)
float minPreviousPosTime = 500; // Time before prevPos of a touch is updated

// Distance parameters (ratio of screen size)
float minimumZoomDistance = 0.1; // Minimum distance between two touches to be considered for zoom gesture (differentiates between clicks and zooms)
float holdToSwipeThreshold = 0.02; // Minimum distance before a multi-touch hold gesture is considered a swipe

float minPreviousPosDistance = 0.0001; // Min distance for touch prevPos to be updated

float zoomGestureMultiplier = 10;

// User Flags: Advanced Touch Gestures Flags
const int GESTURE_UNPROCESSED = EventBase::Flags::User << 1; // Not yet identified (allows the first single touch to generate a down event)
const int GESTURE_SINGLE_TOUCH = EventBase::Flags::User << 2;
const int GESTURE_BIG_TOUCH = EventBase::Flags::User << 3;
const int GESTURE_FIVE_FINGER_HOLD = EventBase::Flags::User << 4;
const int GESTURE_FIVE_FINGER_SWIPE = EventBase::Flags::User << 5;
const int GESTURE_THREE_FINGER_HOLD = EventBase::Flags::User << 6;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Group
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGroup::TouchGroup(TouchGestureManager* gm, int ID){
	gestureManager = gm;
	//printf("TouchGroup %d created\n", ID);

	initialDiameter = 0.5; // Cyber-Commons = 0.2?
	longRangeDiameter = 0.6; // Cyber-Commons = 0.35?
	diameter = initialDiameter;
	this->ID = ID;

	groupHandedness = NONE;

	centerTouch.ID = ID;
	eventType = Event::Null;
	gestureFlag = GESTURE_UNPROCESSED;
	remove = false;

	timeb tb;
	ftime( &tb );
	lastUpdated = tb.millitm + (tb.time & 0xfffff) * 1000;

	touchListLock = new Lock();

	fiveFingerGestureTriggered = false;
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
bool TouchGroup::isInsideGroup( Event::Type eventType, float x, float y, int touchID ){
	// Check if touch is inside radius of TouchGroup
	if( x > xPos - diameter/2 && x < xPos + diameter/2 && y > yPos - diameter/2 && y < yPos + diameter/2 ){
		addTouch( eventType, x, y, touchID );
		return true;
	} else if( x > xPos - longRangeDiameter/2 && x < xPos + longRangeDiameter/2 && y > yPos - longRangeDiameter/2 && y < yPos + longRangeDiameter/2 ){
		addLongRangeTouch( eventType, x, y, touchID );
		return false;
	} else {
		if( longRangeTouchList.count( touchID ) > 0 )
			longRangeTouchList.erase( touchID );
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGroup::addTouch( Event::Type eventType, float x, float y, int touchID ){
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;

	if( eventType == Event::Up ){ // If up cleanup touch

		touchList.erase( touchID );
		idleTouchList.erase( touchID );
		movingTouchList.erase( touchID );
		ofmsg("TouchGroup %1% removed touch ID %2% new size: %3%", %ID %touchID %getTouchCount() );

	} else {

		Touch t;
		t.xPos = x;
		t.yPos = y;
		t.ID = touchID;
		t.timestamp = curTime;

		touchListLock->lock();


		if( touchList.size() == 0 ) // Initial touch group
		{ 
			// New touch down
			gestureFlag = GESTURE_SINGLE_TOUCH;
			gestureManager->generatePQServiceEvent( Event::Down, t, gestureFlag );

			t.state = t.ACTIVE;
			t.prevPosResetTime = curTime;

			touchList[touchID] = t;
			ofmsg("TouchGroup %1% down event", %ID );
		}
		else
		{
			// Update touch
			t.state = touchList[touchID].state;
			t.lastXPos = touchList[touchID].xPos;
			t.lastYPos = touchList[touchID].yPos;
			touchList[touchID] = t;
		}

		touchListLock->unlock();			
	}
	lastUpdated = curTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGroup::addLongRangeTouch( Event::Type eventType, float x, float y, int ID ){
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
			t.ID = ID;
			t.timestamp = lastUpdated;
			longRangeTouchList[ID] = t;
		} else { // Add new touch
			Touch t;
			t.xPos = x;
			t.yPos = y;
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
	xPos = 0;
	yPos = 0;

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
			xPos += t.xPos;
			yPos += t.yPos;

			// Determine if touch is idle
			t.prevPosTimer = curTime - t.prevPosResetTime;
			if( t.prevPosTimer > idleTimeout )
			{
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

	xPos /= touchList.size();
	yPos /= touchList.size();

	touchListLock->unlock();

	// Only update the center position if group still has active touches
	// This allows for the empty touch up event to use the last good position
	if( touchList.size() == 0 )
	{
		if( timeSinceLastUpdate > touchTimeout )
			setRemove();
		return;
	}

	centerTouch.xPos = xPos;
	centerTouch.yPos = yPos;
	gestureManager->generatePQServiceEvent( Event::Move, centerTouch, gestureFlag );

	// Determine the farthest point from the group center (thumb?)
	int farthestTouchID = -1;
	float farthestTouchDistance = 0;

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
	generateGestures();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gesture Tracking
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of touches in the group
void TouchGroup::generateGestures(){

	if( touchList.size() == 1 )
    {
		gestureFlag = GESTURE_SINGLE_TOUCH;
		Touch t = touchList[ID];
		ofmsg("TouchGroup ID: %1% single touch", %ID);
		ofmsg("   size: %1%, %2%", %t.xWidth %t.yWidth);
	}
		

	// 5-finger gesture
	if( touchList.size() == 5 && idleTouchList.size() > 3 && !fiveFingerGestureTriggered )
    {
		gestureManager->generatePQServiceEvent( Event::Down, centerTouch, GESTURE_FIVE_FINGER_HOLD );
		fiveFingerGestureTriggered = true;

		ofmsg("TouchGroup ID: %1% 5-finger gesture triggered", %ID);
		if( groupHandedness == LEFT )
			omsg("   - Left-hand detected");
		else if( groupHandedness == RIGHT )
			omsg("   - Right-hand detected");
    }
    else if( touchList.size() == 5 && idleTouchList.size() > 3 && fiveFingerGestureTriggered )
    {
		gestureManager->generatePQServiceEvent( Event::Move, centerTouch, GESTURE_FIVE_FINGER_HOLD );
		//ofmsg("TouchGroup ID: %1% 5-finger gesture hold", %ID);
    }
	else if( touchList.size() != 5 && fiveFingerGestureTriggered )
    {
		gestureManager->generatePQServiceEvent( Event::Up, centerTouch, GESTURE_FIVE_FINGER_HOLD );
		fiveFingerGestureTriggered = false;

		ofmsg("TouchGroup ID: %1% 5-finger gesture ended", %ID);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of touches in the group
int TouchGroup::getTouchCount(){
	return touchList.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the center of the touch group
Touch TouchGroup::getCenterTouch(){
	return centerTouch;
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
			generatePQServiceEvent( Event::Up, tg->getCenterTouch(), tg->getGestureFlag() );
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

	// Let the touch groups determine if the touch is new or an update
	addTouchGroup( Event::Down, x, y, ID );
	touchList[ID] = touch;

	// TODO: Allow for pass-though of touch points
	// or allow for limited used of the gesture manager
	// i.e. used just for lost/abandoned touch ID cleanup
	/*
	touchListLock->lock();

	// Touch point pass-through
	if( touchList.count(ID) == 0 && eventType == Event::Down ){
		//generatePQServiceEvent( Event::Down, touch, GESTURE_SINGLE_TOUCH );

		//touchList[touch.ID] = touch;
	} else if( touchList.count(ID) >= 1 && eventType == Event::Move ){
		//touchList[touch.ID] = touch;

		//generatePQServiceEvent( Event::Move, touch, GESTURE_SINGLE_TOUCH );
	} else if( touchList.count(ID) >= 1 && eventType == Event::Up ){
		//touchList.erase( touch.ID );

		//generatePQServiceEvent( Event::Up, touch, GESTURE_SINGLE_TOUCH );
	} else {
		// Error correcting steps for missing touch data, dropped packets, etc.
		if( touchList.count(ID) == 1 && eventType == Event::Down ) {
			printf("TouchGestureManager: Should-Not-Happen-Warning: Touch down received for existing touch %d.\n", ID);
			touchListLock->unlock();
			return false;
		} else if( touchList.count(ID) == 0 && eventType == Event::Move ) {
			printf("TouchGestureManager: Should-Not-Happen-Warning: Touch move received for non-existant touch %d.\n", ID);

			// If a touch move for non-existant touch detected, create the down event
			//touchList[ID] = touch;
			//generatePQServiceEvent( Event::Down, touch );
			//printf("(DB) New Touch ID %d pos: %f %f width: %f %f\n", ID, x, y, xW, yW);
			touchListLock->unlock();
			return false;
		} else if( touchList.count(ID) == 0 && eventType == Event::Up ) {
			printf("TouchGestureManager: Should-Not-Happen-Warning: Touch up received for non-existant touch %d.\n", ID);
			touchListLock->unlock();
			return false;
		} else {
			printf("TouchGestureManager: Should-Not-Happen-Warning: Unknown case for touch %d.\n", ID);
			touchListLock->unlock();
			return false;
		}
		//printf("Touchlist count for ID %d: %d. Received eventType: %d\n", ID, touchList.count(ID), eventType );
	}

	touchListLock->unlock();
	*/
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TouchGestureManager::addTouchGroup( Event::Type eventType, float xPos, float yPos, int ID )
{
	touchGroupListLock->lock();

	// Check if new touch is inside an existing TouchGroup
	map<int,TouchGroup*>::iterator it;
	for ( it = touchGroupList.begin() ; it != touchGroupList.end(); it++ ){
		TouchGroup* tg = (*it).second;
		int groupID = (*it).first;

		if( tg->isInsideGroup( eventType, xPos, yPos, ID ) )
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
		newGroup->addTouch( eventType, xPos, yPos, ID );

		touchGroupList[ID] = newGroup;
		
		groupedIDs.insert(ID);
		touchGroupListLock->unlock();
		
		return true;
	}
	touchGroupListLock->unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::generatePQServiceEvent( Event::Type eventType, Touch touch, int gesture )
{
	if( pqsInstance ){
		pqsInstance->lockEvents();

		Event* evt = pqsInstance->writeHead();

		switch(eventType)
		{
		case Event::Down:
			evt->reset(Event::Down, Service::Pointer, touch.ID);
			break;
		case Event::Move:
			evt->reset(Event::Move, Service::Pointer, touch.ID);
			break;
		case Event::Up:
			evt->reset(Event::Up, Service::Pointer, touch.ID);
			break;
		}		
		evt->setPosition( Vector3f(touch.xPos, touch.yPos, 0 ) );

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);
		evt->setFlags( gesture );

		pqsInstance->unlockEvents();
		
	} else {
		printf("TouchGestureManager: No PQService Registered\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::generateZoomEvent( Event::Type eventType, Touch touch, float zoomDelta )
{
	if( pqsInstance ){
		pqsInstance->lockEvents();

		Event* evt = pqsInstance->writeHead();
		evt->reset(Event::Zoom, Service::Pointer, touch.ID);
	
		evt->setPosition(touch.xPos, touch.yPos);

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);
		evt->setExtraDataFloat(2, zoomDelta);

		switch(eventType)
		{
			case( Event::Down ): evt->setExtraDataFloat(3, 1); break;
			case( Event::Move ): evt->setExtraDataFloat(3, 2); break;
			case( Event::Up ): evt->setExtraDataFloat(3, 3); break;
		}

		pqsInstance->unlockEvents();
	} else {
		printf("TouchGestureManager: No PQService Registered\n");
	}
}
