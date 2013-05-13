/**************************************************************************************************
 * THE OMICRON SDK
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
#ifndef __ASSET_CACHE_MANAGER__
#define __ASSET_CACHE_MANAGER__

#include "omicron/osystem.h"
#include "omicron/Lock.h"

namespace omicron {
	class CacheSyncThread;
	class CacheConnection;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API AssetCacheManager: public ReferenceType
	{
	public:
		friend class CacheSyncThread;
		friend class CacheConnection;

	public:
		AssetCacheManager();
		virtual ~AssetCacheManager();

		void setCacheName(const String& value) { myCacheName = value; }
		String cacheName() { return myCacheName; }

		void addCacheHost(const String&);
		void clearCacheHosts();

		void setCachePort(uint value) { myCachePort = value; }
		uint getCachePort() { return myCachePort; } 

		void addFileToCacheList(const String& file);
		void clearCacheFileList();
		
		void sync();
		void startSync();
		bool isSynching() { return mySynching; }

		void setVerbose(bool value) { myVerbose = value; }
		bool isVerbose() { return myVerbose; }

	private:
		CacheSyncThread* myThread;
		Lock myLock;

		String myCacheName;
		List<String> myCacheHosts;
		uint myCachePort;

		List<String> myFileList;
		bool mySynching;
		bool myVerbose;
	};
}; // namespace omicron

#endif