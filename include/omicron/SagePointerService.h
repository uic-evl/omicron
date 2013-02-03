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
#ifndef __SAGE_POINTER_SERVICE_H__
#define __SAGE_POINTER_SERVICE_H__

#include "osystem.h"
#include "Service.h"

namespace omicron {
	class SagePointerServer;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Implements a service able to receive pointer updates from the SAGE pointer application
	class SagePointerService: public Service
	{
	friend class SagePointerConnection;
	public:
		//! Allocator function (will be used to register the service inside SystemManager)
		static SagePointerService* New() { return new SagePointerService(); }

	public:
		SagePointerService();
		~SagePointerService();

		virtual void setup(Setting& settings);
		virtual void poll();

		bool forceSourceId() { return myForcedSourceId != -1; }
		int getForcedSourceId() { return myForcedSourceId; }

	private:
		// NOTE: this class is using the obsolete version of using a TcpServer.
		// The correct way is to derive BasicPortholeService from TcpServer directly 
		// (TcpServer derives from Service now)
		SagePointerServer* myServer;
		int myForcedSourceId;
	};
}; // namespace omega

#endif