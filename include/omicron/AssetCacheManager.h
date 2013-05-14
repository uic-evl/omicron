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
 *-------------------------------------------------------------------------------------------------
 * The asset cache manager connects to a remote cache service and synchronizes a list of files
 * with it.
 *************************************************************************************************/
#ifndef __ASSET_CACHE_MANAGER__
#define __ASSET_CACHE_MANAGER__

#include "omicron/osystem.h"
#include "omicron/Lock.h"

namespace omicron {
	class CacheSyncThread;
	class CacheConnection;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//! The asset cache manager connects to a remote cache service and synchronizes a list of files
	//! with it.
	class OMICRON_API AssetCacheManager: public ReferenceType
	{
	public:
		friend class CacheSyncThread;
		friend class CacheConnection;

	public:
		AssetCacheManager();
		virtual ~AssetCacheManager();

		//! Set the name of the cache handled by this manager.
		//! Must be done before starting a sync.
		void setCacheName(const String& value) { myCacheName = value; }
		String cacheName() { return myCacheName; }

		//! Sets the hostname of the machine running the cache service
		//! Must be done before starting a sync.
		void addCacheHost(const String&);
		void clearCacheHosts();

		//! Sets the listening port of the cache service.
		//! Must be done before starting a sync.
		void setCachePort(uint value) { myCachePort = value; }
		uint getCachePort() { return myCachePort; } 

		//! Add a file name to the list of files that need to be synchronized.
		//! Must be done before starting a sync.
		void addFileToCacheList(const String& file);
		void clearCacheFileList();

		//! When set to true, forces overwriting of files in the cache. Useful for
		//! replacing cache contents.
		//! Must be done before starting a sync.
		void setForceOverwrite(bool value) { myForceOverwrite = true; }
		bool isForceOverwriteEnabled() { return myForceOverwrite; }

		//! Start synching.
		//! This is a blocking call. It will not return until synching is done.
		void sync();
		//! Start synching. This call is nonblocking and will return immediately.
		//! Use isSynching to check for completion.
		void startSync();
		bool isSynching() { return mySynching; }

		//! When set to true, output detailed messages about synching.
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
		bool myForceOverwrite;
		bool myVerbose;
	};
}; // namespace omicron

#endif