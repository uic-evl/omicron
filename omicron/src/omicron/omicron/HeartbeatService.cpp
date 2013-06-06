/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
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
#include "omicron/HeartbeatService.h"

#include <time.h>

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
HeartbeatService::HeartbeatService():
	myRate(1.0f),
	myLastEventTime(0.0f)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void HeartbeatService::setup(Setting& settings)
{
	// This function is called before initializing the service, to setup its parameters. The settings class passed as
	// argument contains all the options relative to this service (usually loaded from a system configuration file)

	// If rate parameter is present, use it instead of the default value
	if(settings.exists("rate"))
	{
		myRate = settings["rate"];
	}

	mySeqNumber = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HeartbeatService::poll() 
{
	// Get the current system clock time in seconds.
	float curt = (float)((double)clock() / CLOCKS_PER_SEC);

	float interval = 1.0f / myRate;

	// If we need an update (depending on the rate) send out a new event now.
	if(curt - myLastEventTime > interval)
	{
		// Lock the event list and get the first free event slot in the event buffer
		lockEvents();
		Event* evt = writeHead();

		// Reset the event slot: this effectively creates a new simple event with the specified type and class 
		// To see a slightly more complex event setup (with position and additional values stored) check out
		// the KeyboardService or MouseService class.
		evt->reset(Event::Update, Service::Generic);
		evt->setPosition(mySeqNumber, mySeqNumber + 0.1f, mySeqNumber + 0.2f);
		evt->setOrientation(mySeqNumber + 0.3f, mySeqNumber + 0.4f, mySeqNumber + 0.5f, mySeqNumber + 0.6f);

		// We are done: unlock the event list.
		unlockEvents();

		myLastEventTime = curt;
		mySeqNumber++;
	}
}
