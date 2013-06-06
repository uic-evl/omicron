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
#ifndef __MOTION_CAPTURE_SERVICE_H__
#define __MOTION_CAPTURE_SERVICE_H__

#include "omicron/osystem.h"
#include "omicron/ServiceManager.h"
#include "natnet/NatNetTypes.h"
#include "natnet/NatNetClient.h"
#include "winsock2.h"

namespace omicron
{
	class NaturalPointService : public Service
	{
	public:
		// Allocator function
		static NaturalPointService* New() { return new NaturalPointService(); }

	public:
		NaturalPointService();
		~NaturalPointService();
		void setup( Setting& settings);
		static void __cdecl frameController(sFrameOfMocapData* data, void* pUserData);
		static void __cdecl messageController(int msgType, char* msg);
		virtual void initialize();
		virtual void start();//initialize and start service here
		virtual void stop();//destroy service instance to stop
		virtual void dispose();
		//may want to support the option to choose whether to have unicast or multicast networking
		//for now it is hard coded to multicast
	private:
		static double Pi;
		static NaturalPointService* myMoCap;
		NatNetClient* pClient;
		int castType;			//This determines wether the information is multicast or unicast across the network sockets. 0 = multicast and 1 = unicast
		char localIP[128];		//the IP address of this machine, it is found automatically if it is set to an empty string (e.g. "")
		char serverIP[128];		//Server's IP address assumed to be local if left blank

		// Reference frame transform
		AffineTransform3 myTransform;
	};//class MoCapService

};//namespace omicron
#endif
