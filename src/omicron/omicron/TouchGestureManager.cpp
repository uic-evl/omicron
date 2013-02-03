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

#include "omicron/TouchGestureManager.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Group
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGroup::TouchGroup(int ID){
	initialDiameter = 500; // Currently in pixels -> TODO: Change to screen ratio
	longRangeDiameter = 4000;
	diameter = initialDiameter;
	this->ID = ID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TouchGroup::getID(){
	return ID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TouchGroup::isInsideGroup( Event::Type eventType, float x, float y, int id ){
	// Check if touch is inside radius of TouchGroup
	if( x > xPos - diameter/2 && x < xPos + diameter/2 && y > yPos - diameter/2 && y < yPos + diameter/2 ){
		addTouch( eventType, x, y, ID );
		return true;
	} else if( x > xPos - longRangeDiameter/2 && x < xPos + longRangeDiameter/2 && y > yPos - longRangeDiameter/2 && y < yPos + longRangeDiameter/2 ){
		addLongRangeTouch( eventType, x, y, ID );
		return false;
	} else {
		if( longRangeTouchList.count( ID ) > 0 )
			longRangeTouchList.erase( ID );
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGroup::addTouch( Event::Type eventType, float x, float y, int ID ){
	timeb tb;
	ftime( &tb );
	lastUpdated = tb.millitm + (tb.time & 0xfffff) * 1000;

	if( eventType == Event::Up ){ // If up cleanup touch
		touchList.erase( ID );
		idleTouchList.erase( ID );
		movingTouchList.erase( ID );
	} else {
		if( touchList.count( ID ) == 1 ){ // Update existing touch
			Touch t;
			t.xPos = x;
			t.yPos = y;
			t.ID = ID;
			t.timestamp = lastUpdated;
			touchList[ID] = t;
		} else { // Add new touch
			Touch t;
			t.xPos = x;
			t.yPos = y;
			t.ID = ID;
			t.timestamp = lastUpdated;
			touchList[ID] = t;

			// If touch was long range, remove from long range list
			if( longRangeTouchList.count( ID ) == 1 ){
				longRangeTouchList.erase( ID );
			}
		}

		// Update group position by determining average touch position
		xPos = 0;
		yPos = 0;
		float nTouches = 0.0;
		map<int,Touch>::iterator it;
		for ( it = touchList.begin() ; it != touchList.end(); it++ ){
			Touch touch = (*it).second;
			xPos += touch.xPos;
			yPos += touch.yPos;
			nTouches++;
		}
		
		xPos /= nTouches;
		yPos /= nTouches;
	}
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

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Gesture Manager
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TouchGestureManager::deadTouchDelay = 250;
int TouchGestureManager::maxTouches = 1000;
int TouchGestureManager::nextID = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchGestureManager::TouchGestureManager()
{
	//omsg("TouchGestureManager: TouchGestureManager()");
	touchListLock = new Lock();
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
	//map<int,TouchGroup*>::iterator it;
	//for ( it = touchGroupList.begin() ; it != touchGroupList.end(); it++ ){
	//	TouchGroup* group = (*it).second;
	//	//printf("TouchGroup %d size: %d \n", group->ID, group->touchList.size() );
	//}
/*
	touchListLock->lock();

	// Get time in milliseconds
	timeb tb;
	ftime( &tb );
	int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;

	//printf("TouchGestureManager: poll()\n");
	//printf("TouchGestureManager: poll() Time: %d\n", curTime);

	map<int,Touch> swapList;

	map<int,Touch>::iterator it;
	for ( it = touchList.begin() ; it != touchList.end(); it++ ){
		Touch t = (*it).second;

		if( curTime - t.timestamp < deadTouchDelay ){
			swapList[t.ID] = t;
		} else {
			t.timestamp = curTime;

			// Touch has expired - likely due to lost up event. Send new up event.
			t.ID = generatePQServiceEvent( Event::Up, t );
			printf("Auto Remove Touch ID %d\n", t.ID);
		}
		//printf( "TL: Touch ID %d \n", t.timestamp );
	}
	touchListLock->unlock();

	touchListLock->lock();
	touchList.clear();
	touchList = swapList;
	touchListLock->unlock();
*/
	//printf("TouchList size: %d\n", touchList.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// All touches considered for gestures are added by this function.
// This also serves to error correct touch data: invalid ranges, missing events, etc.
bool TouchGestureManager::addTouch(Event::Type eventType, Touch touch)
{
	int x = touch.xPos;
	int y = touch.yPos;
	int ID = touchID[touch.ID];

	touchListLock->lock();

	if( touchList.count(ID) == 0 && eventType == Event::Down ){
		touchID[touch.ID] = nextID;
		touch.ID = nextID;
		if( nextID < maxTouches - 100 ){
			nextID++;
		} else {
			nextID = 0;
		}
		generatePQServiceEvent( Event::Down, touch );
		//printf("New Touch ID %d pos: %f %f width: %f %f\n", touch.ID, touch.xPos, touch.yPos, touch.xWidth, touch.yWidth);
			
		addTouchGroup( eventType, x, y, touch.ID );
		touchList[touch.ID] = touch;
	} else if( touchList.count(ID) >= 1 && eventType == Event::Move ){
		touch.ID = ID;
		generatePQServiceEvent( Event::Move, touch );
		//printf("Updated Touch ID %d pos: %f %f width: %f %f\n", touch.ID, touch.xPos, touch.yPos, touch.xWidth, touch.yWidth);
			
		addTouchGroup( eventType, x, y, ID );
		touchList[ID] = touch;
	} else if( touchList.count(ID) >= 1 && eventType == Event::Up ){
		touch.ID = ID;
		generatePQServiceEvent( Event::Up, touch );
		//printf("Removed Touch ID %d pos: %f %f width: %f %f\n", touch.ID, touch.xPos, touch.yPos, touch.xWidth, touch.yWidth);
			
		touchList.erase( ID );
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
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TouchGestureManager::addTouchGroup( Event::Type eventType, float xPos, float yPos, int ID )
{
	// Check if new touch is inside an existing TouchGroup
	map<int,TouchGroup*>::iterator it;
	for ( it = touchGroupList.begin() ; it != touchGroupList.end(); it++ ){
		TouchGroup* tg = (*it).second;
		if( tg->isInsideGroup( eventType, xPos, yPos, ID ) ){
			if( eventType == Event::Down )
				printf("ID %d added to TouchGroup %d\n", ID, tg->getID() );
			if( eventType == Event::Up ){
				groupedIDs.erase(ID);
				printf("ID %d removed from TouchGroup %d\n", ID, tg->getID() );
			} else {
				groupedIDs.insert(ID);
			}
			return true;
		}
	}

	// If touch is not part of existing group, create new
	// TouchGroup using that touch ID
	if( groupedIDs.count(ID) == 0 ){
		TouchGroup* newGroup = new TouchGroup(ID);
		newGroup->addTouch( eventType, xPos, yPos, ID );
		touchGroupList[ID] = newGroup;
		
		printf("TouchGroup %d created\n", ID);

		if( eventType == Event::Up )
			groupedIDs.erase(ID);
		else
			groupedIDs.insert(ID);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TouchGestureManager::generatePQServiceEvent( Event::Type eventType, Touch touch )
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
		evt->setPosition(touch.xPos, touch.yPos);

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);
		pqsInstance->unlockEvents();
	} else {
		printf("TouchGestureManager: No PQService Registered\n");
	}
}