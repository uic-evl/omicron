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

#include "omicron/TouchGestureManager.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Time parameters (milliseconds)
int touchTimeout = 150; // Time since last update until touch group is automatically removed
int doubleClickDelay = 100; // Time between the first and second click to trigger DoubleClick event

// Distance parameters (ratio of screen size)
float minimumZoomDistance = 0.1; // Minimum distance between two touches to be considered for zoom gesture (differentiates between clicks and zooms)
float holdToSwipeThreshold = 0.02; // Minimum distance before a multi-touch hold gesture is considered a swipe

float zoomGestureMultiplier = 10;

// User Flags: Advanced Touch Gestures Flags
const int GESTURE_UNPROCESSED = 1 << 19; // Not yet identified (allows the first single touch to generate a down event)
const int GESTURE_SINGLE_TOUCH = -1;
const int GESTURE_BIG_TOUCH = 1 << 16;
const int GESTURE_MULTI_TOUCH_HOLD = 1 << 17;
const int GESTURE_MULTI_TOUCH_SWIPE = 1 << 18;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Group
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGroup::TouchGroup(TouchGestureManager* gm, int ID){
	gestureManager = gm;
	//printf("TouchGroup %d created\n", ID);

	initialDiameter = 0.4;
	longRangeDiameter = 0.6;
	diameter = initialDiameter;
	this->ID = ID;

	centerTouch.ID = ID;
	eventType = Event::Null;
	gestureFlag = GESTURE_UNPROCESSED;
	remove = false;

	timeb tb;
	ftime( &tb );
	lastUpdated = tb.millitm + (tb.time & 0xfffff) * 1000;

	touchListLock = new Lock();
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
			gestureManager->generatePQServiceEvent( Event::Down, t, gestureFlag );
			touchList[touchID] = t;
			ofmsg("TouchGroup %1% down event", %ID );
		}
		else
		{
			// Update touch list
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

	// Recalculate group center by averaging current touch points
	map<int,Touch>::iterator it;
	for ( it = touchList.begin() ; it != touchList.end(); it++ )
	{
		Touch t = (*it).second;

		// Check touch update time, if too long remove from list
		int lastTouchUpdate = curTime-t.timestamp;
		if( lastTouchUpdate < touchTimeout )
		{
			activeTouchList[t.ID] = t;
			xPos += t.xPos;
			yPos += t.yPos;
		}
	}

	// Swap oldlist with active list
	touchList = activeTouchList;

	xPos /= touchList.size();
	yPos /= touchList.size();

	touchListLock->unlock();
	
	centerTouch.xPos = xPos;
	centerTouch.yPos = yPos;

	gestureManager->generatePQServiceEvent( Event::Move, centerTouch, gestureFlag );

	if( touchList.size() > 0 )
	{
		//ofmsg("Touchgroup ID: %1% center at %2%, %3% touchCount: %4% ", %ID %xPos %yPos %touchList.size());
	}
	else
	{
		//ofmsg("Touchgroup ID: %1% Time since last update %2%", %ID %timeSinceLastUpdate);
		if( timeSinceLastUpdate > touchTimeout )
			setRemove();
		
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
int TouchGestureManager::touchTimeout = 150;
int TouchGestureManager::maxTouches = 1000;
int TouchGestureManager::nextID = 0;

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
void TouchGestureManager::setMaxTouchIDs(int maxID)
{
	maxTouches = maxID;
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

	//generatePQServiceEvent( eventType, touch, GESTURE_SINGLE_TOUCH );

	/*
	touchListLock->lock();

	if( touchList.count(ID) == 0 && eventType == Event::Down ){
		touchID[touch.ID] = nextID;
		touch.ID = nextID;
		if( nextID < maxTouches - 100 ){
			nextID++;
		} else {
			nextID = 0;
		}
		generatePQServiceEvent( Event::Down, touch, GESTURE_SINGLE_TOUCH );

		touchList[touch.ID] = touch;
	} else if( touchList.count(ID) >= 1 && eventType == Event::Move ){
		touch.ID = ID;
		touchList[touch.ID] = touch;

		generatePQServiceEvent( Event::Move, touch, GESTURE_SINGLE_TOUCH );

	} else if( touchList.count(ID) >= 1 && eventType == Event::Up ){
		touch.ID = ID;
		touchList.erase( touch.ID );

		generatePQServiceEvent( Event::Up, touch, GESTURE_SINGLE_TOUCH );

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
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;
	int timeSinceLastSentEvent = curTime-timeLastEventSent;
	//ofmsg("Time since last event sent %1%", %timeSinceLastSentEvent);

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

		if( gesture != GESTURE_SINGLE_TOUCH )
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
