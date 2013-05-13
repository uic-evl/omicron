/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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
#ifndef __ASSSET_CACHE_SERVICE_H__
#define __ASSSET_CACHE_SERVICE_H__

#include "omicron/Tcp.h"

namespace omicron {
	
	class AssetCacheService;
    
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API AssetCacheConnection: public TcpConnection
	{
	public:
		AssetCacheConnection(ConnectionInfo ci, AssetCacheService* server);

		virtual void handleData();
		virtual void handleClosed();
		virtual void handleConnected();

		void sendMessage(const char* header, void* data, int size);

	private:
		static const int BufferSize = 1024;
		char myBuffer[BufferSize];
		AssetCacheService* myServer;

		List<String> myQueuedFiles;

		String myCacheName;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API AssetCacheService: public TcpServer
	{
	public:
		static const int DefaultPort = 22500;
	public:
		AssetCacheService();
		virtual ~AssetCacheService();

		virtual void initialize();
		virtual void dispose();

		virtual TcpConnection* createConnection(const ConnectionInfo& ci);
		void closeConnection(AssetCacheConnection* conn);

		void setCacheRoot(const String& value) { myCacheRoot = value; }
		String getCacheRoot() { return myCacheRoot; }

	private:
		String myCacheRoot;
		List<AssetCacheConnection*> myConnections;
	};
}; // namespace omicron

#endif