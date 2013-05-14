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
#include "omicron/AssetCacheManager.h"
#include "omicron/Tcp.h"
#include "omicron/StringUtils.h"
#include "omicron/DataManager.h"

#include <fstream>

using namespace omicron;

namespace omicron {
///////////////////////////////////////////////////////////////////////////////////////////////////
class CacheConnection: public TcpConnection
{
public:
	CacheConnection(const ConnectionInfo& info): TcpConnection(info), done(false)
	{}

	virtual void handleConnected()
	{
		omsg("Connected: setting cache");

		String& cacheName = myAssetCacheManager->myCacheName;

		sendMessage("CHCS", (void*)cacheName.c_str(), cacheName.size());

		// If we are not in overwrite mode, ask for files
		if(!myAssetCacheManager->isForceOverwriteEnabled())
		{
			foreach(String file, myAssetCacheManager->myFileList)
			{
				sendMessage("CHCA", (void*)file.c_str(), file.size());
			}
			// Tell the server we are done requesting files. The server will either reply 
			// with a done message too if no cache sync is needed, or it will send us file
			// requests for each file it is missing.
			sendMessage("CHCD", NULL, 0);
		}
		else
		{
			omsg("Force push");
			// If we are, just push files.
			foreach(String file, myAssetCacheManager->myFileList)
			{
				sendFile(file.c_str());
			}
			sendMessage("CHCD", NULL, 0);
		}
	}

	virtual void handleData()
	{
		// Read message header.
		char header[4];
		read((byte*)myBuffer, 4);
		memcpy(header, myBuffer, 4);

		//ofmsg("DATA: %1%%2%%3%%4%", %myBuffer[0] %myBuffer[1] %myBuffer[2] %myBuffer[3]);

		// Read data length.
		int dataSize;
		read((byte*)myBuffer, 4);
		memcpy(&dataSize, myBuffer, 4);

		// Read data.
		read((byte*)myBuffer, dataSize);
		myBuffer[dataSize] = '\0';

		if(!strncmp(header, "CHCD", 4)) 
		{
			omsg("Done message received");
			// DOne.
			done = true;
			waitClose();
		}
		if(!strncmp(header, "CHCR", 4)) 
		{
			// File request.
			ofmsg("File requested: %1%", %myBuffer);
			// Send file name
			sendMessage("CHCP", (void*)myBuffer, strlen(myBuffer));

			sendFile(myBuffer);
		}
	}

	virtual void handleError(const ConnectionError& err)
	{
		TcpConnection::handleError(err);
		done = true;
	}

	void sendMessage(const char* header, void* data, int size)
	{
		write((void*)header, 4);
		write(&size, sizeof(int));
		write(data, size);
	}

	void sendFile(const char* filename)
	{
		unsigned int datasize = 0;
		// Send file data.
		String fullPath;
		if(DataManager::findFile(filename, fullPath))
		{
			// Get the file size.
			FILE* f = fopen(fullPath.c_str(), "rb");
			fseek(f, 0, SEEK_END);
			unsigned int sz = ftell(f);
			fclose(f);
				
			ofmsg("Sending file size: %1% bytes", %sz);
			write(&sz, sizeof(unsigned int));

			const unsigned int buff_size = 16384; //size of the send buffer

			FILE* file = fopen(fullPath.c_str(), "rb");
			//std::fstream file(fullPath); //we open this file

			char* buff = new char[buff_size]; //creating the buffer
			unsigned int count = 0; //counter
			while( !feof(file) ) 
			{ 
				memset(buff,0,buff_size); //cleanup the buffer
				size_t len = fread(buff, 1, buff_size, file);
				write(buff,len);
				count+=len; //increment counter
			}

			ofmsg("Sent bytes: %1%", %count);

			fclose(file); //close file
			delete(buff);  //delete buffer
		}
		else
		{
			ofwarn("File not found! %1%", %myBuffer);
			write(&datasize, sizeof(unsigned int));
		}
	}

	bool done;
	AssetCacheManager* myAssetCacheManager;
	char myBuffer[1024];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CacheSyncThread: public Thread
{
public:
	CacheSyncThread(AssetCacheManager* mng): myAssetCacheManager(mng)
	{}

	virtual void threadProc()
	{
		asio::io_service ioService;
		Ref<CacheConnection> conn = new CacheConnection(ConnectionInfo(ioService));
		conn->myAssetCacheManager = myAssetCacheManager;
		conn->open(myAssetCacheManager->myCacheHosts.front(), myAssetCacheManager->myCachePort);
		while(!conn->done)
		{
			ioService.run_one();
			conn->poll();
			osleep(10);
		}
		omsg("CacheSyncThread:threadProc(): END");
		myAssetCacheManager->mySynching = false;
	}

private:
	AssetCacheManager* myAssetCacheManager;
};

};


///////////////////////////////////////////////////////////////////////////////////////////////////
AssetCacheManager::AssetCacheManager():
	myCachePort(8090), myCacheName("defaultCache"), mySynching(false), myVerbose(false), myForceOverwrite(false)
{
	myThread = new CacheSyncThread(this);
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
AssetCacheManager::~AssetCacheManager()
{
	myThread->stop();
	delete myThread;
	myThread = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheManager::addCacheHost(const String& host)
{
	myCacheHosts.push_back(host);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheManager::clearCacheHosts()
{
	myCacheHosts.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheManager::addFileToCacheList(const String& file)
{
	myFileList.push_back(file);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheManager::clearCacheFileList()
{
	myFileList.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheManager::sync()
{
	if(!mySynching)
	{
		startSync();
		while(mySynching) osleep(500);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheManager::startSync()
{
	if(!mySynching)
	{
		mySynching = true;
		myThread->start();
	}
}


