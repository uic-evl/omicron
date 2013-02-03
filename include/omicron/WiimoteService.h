/**************************************************************************************************
 * THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2012, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __WIIMOTE__SERVICE_H__
#define __WIIMOTE__SERVICE_H__

#include "omicron/osystem.h"
#include "omicron/Timer.h"
#include "omicron/ServiceManager.h"
//#include "omicron/dinput.h"
#include "omicron/wiimote.h"

namespace omicron {
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//! Reads input data from Wii controllers.
	class OMICRON_API WiimoteService: public Service
	{
	public:
		// Allocator function
		static WiimoteService* New() { return new WiimoteService(); }

		//! Wii button mappings
		enum ButtonFlag 
		{
			ButtonA = Event::Button1,
			ButtonB = Event::Button2,
			ButtonUp = Event::ButtonUp,
			ButtonDown = Event::ButtonDown,
			ButtonLeft = Event::ButtonLeft,
			ButtonRight = Event::ButtonRight,
			ButtonPlus = Event::Button5,
			ButtonMinus = Event::Button6,
			ButtonOne = Event::Button3,
			ButtonTwo = Event::Button4,
		};
		enum ControllerTypeFlag 
		{ 
			TypeWiimote = Event::User << 10,
			TypeNunchuk = Event::User << 11, 
			TypeMotionPlus = Event::User << 12
		};

	public:
		void setup(Setting& settings);
	
		virtual void initialize();
		virtual void poll();
		virtual void dispose();

		//! Sets the data update interval, in seconds. This is the interval at which this service will generate events
		//! If set to zero, the service will generate events as fast as possible.
		void setUpdateInterval(float value);
		//! @see setUpdateInterval
		float getUpdateInterval();

	private:
		uint pollButtonState();
		void writeWiimoteEvent();
		void writeNunchuckEvent();
		void writeMotionPlusEvent();
		void writePointerEvent();

	private:
		static WiimoteService* mysInstance;
		float myUpdateInterval;
		Timer myUpdateTimer;
		wiimote myWiimote;
		int myEventSourceId;
		uint myButtonState;
	};

}; // namespace omicron

#endif