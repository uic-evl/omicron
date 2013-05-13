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
#include "omicron/AssetCacheService.h"
#include "omicron/StringUtils.h"
#include "omicron/DataManager.h"

#include <fstream>

using namespace omicron;

#ifdef OMEGA_OS_LINUX
const int AssetCacheService::DefaultPort;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
AssetCacheConnection::AssetCacheConnection(ConnectionInfo ci, AssetCacheService* server): 
	TcpConnection(ci),
	myServer(server)
{
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheConnection::handleData()
{
    // Read message header.
	char header[4];
    read((byte*)myBuffer, 4);
	memcpy(header, myBuffer, 4);

    // Read data length.
	int dataSize;
    read((byte*)myBuffer, 4);
	memcpy(&dataSize, myBuffer, 4);

 //   // Read data.
 //   read((byte*)myBuffer, dataSize);
	//myBuffer[dataSize] = '\0';

	if(!strncmp(header, "CHCS", 4)) 
	{
		//script command message
		String command(myBuffer);

		read((byte*)myBuffer, dataSize);
		myBuffer[dataSize] = '\0';

		myCacheName = myBuffer;
		ofmsg("AssetCacheConnection: cache name set to %1%", %myCacheName);
	}
	if(!strncmp(header, "CHCA", 4)) 
	{
		//script command message
		String command(myBuffer);

		read((byte*)myBuffer, dataSize);
		myBuffer[dataSize] = '\0';

		String fileName = myServer->getCacheRoot() + myCacheName + "/" + myBuffer;
		String fullFilePath;

		ofmsg("AssetCacheConnection: looking for file %1%", %fileName);

		if(!DataManager::findFile(fileName, fullFilePath))
		{
			ofmsg("File not found sending request for %1%", %myBuffer);
			sendMessage("CHCR", (void*)myBuffer, strlen(myBuffer));

			// Add the file to the queued list
			myQueuedFiles.push_back(myBuffer);
		}
	}
	if(!strncmp(header, "CHCD", 4)) 
	{
		// The client is done adding files. If we have no files in our request queue, tell the client we are done.
		if(myQueuedFiles.size() == 0)
		{
			sendMessage("CHCD", NULL, 0);
			close();
		}
		else
		{
			omsg("Waiting to finish file transfer...");
		}
	}
	if(!strncmp(header, "CHCP", 4)) 
	{
		// File data received.
		//script command message
		String command(myBuffer);

		read((byte*)myBuffer, dataSize);
		myBuffer[dataSize] = '\0';
		ofmsg("Receiving file %1%", %myBuffer);

		unsigned int fileSize = 0; 
		read((omicron::byte*)&fileSize, sizeof(unsigned int));

		String fileName = myServer->getCacheRoot() + myCacheName + "/" + myBuffer;

		const unsigned int buff_size = 65536; //the size of the read buffer
		char* buff = new char[buff_size];

		FILE* f = fopen(fileName.c_str(), "wb");

		unsigned int count = 0; //a counter
		while(count < fileSize)
		{ 
			unsigned int nextBlock = buff_size;
			if(count + nextBlock > fileSize) nextBlock = fileSize - count;
			size_t len = read((omicron::byte*)buff, nextBlock);
			if(len == 0)
			{
				ofwarn("Error reading file %1%, skipping.", %fileName);
				break;
			}
			count += len;
			ofmsg("Read a total of %1% bytes ", %count);
			fwrite(buff, 1, len, f);
		}

		fclose(f);

		myQueuedFiles.remove(myBuffer);
		// The client is done adding files. If we have no files in our request queue, tell the client we are done.
		if(myQueuedFiles.size() == 0)
		{
			omsg("File transfers done, closing connection.");
			sendMessage("CHCD", NULL, 0);
			close();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheConnection::handleClosed()
{
    //ofmsg("Mission control connection closed (id=%1%)", %getConnectionInfo().id);
	myServer->closeConnection(this);
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheConnection::handleConnected()
{
	TcpConnection::handleConnected();

    //ofmsg("Mission control connection open (id=%1%)", %getConnectionInfo().id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheConnection::sendMessage(const char* header, void* data, int size)
{
	write((void*)header, 4);
	write(&size, sizeof(int));
	write(data, size);
}

///////////////////////////////////////////////////////////////////////////////////////////
AssetCacheService::AssetCacheService():	
	myCacheRoot("./")
{
	setPort(DefaultPort);
}

///////////////////////////////////////////////////////////////////////////////////////////
AssetCacheService::~AssetCacheService() 
{
}

///////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheService::initialize() 
{
	TcpServer::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheService::dispose() 
{
	TcpServer::dispose();
}

///////////////////////////////////////////////////////////////////////////////////////////
TcpConnection* AssetCacheService::createConnection(const ConnectionInfo& ci)
{
	AssetCacheConnection* conn = new AssetCacheConnection(ci, this);
	myConnections.push_back(conn);
	return conn;
}

///////////////////////////////////////////////////////////////////////////////////////////
void AssetCacheService::closeConnection(AssetCacheConnection* conn)
{
	myConnections.remove(conn);
}
