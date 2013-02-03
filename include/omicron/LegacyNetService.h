/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
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
#ifndef __LEGACY_NET_SERVICE_H__
#define __LEGACY_NET_SERVICE_H__

#include "omicron/osystem.h"
#include "omicron/ServiceManager.h"

#include "pqlabs/PQMTClient.h"
using namespace PQ_SDK_MultiTouch;

#ifdef OMICRON_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h> // needed for close()
#include <string>
#endif

struct NetTouches{
	int ID;
	float xPos;
	float yPos;
	float xWidth;
	float yWidth;
	float timestamp;

	// Gestures
	int gestureType;

	// Split gesture
	float x1,y1,x2,y2;
	float initDistance;
};

namespace omicron
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LegacyNetService: public Service
{
public:
	// Allocator function
	LegacyNetService();
	static LegacyNetService* New() { return new LegacyNetService(); }

public:
	virtual void setup(Setting& settings);
	virtual void initialize();
	virtual void poll();
	virtual void dispose();
	void setServer(const char*,int);
	void setDataport(int);
	void setScreenResolution(int,int);
	void setTouchTimeout(float);
private:
	void initHandshake();
	void parseDGram(int);
private:
	LegacyNetService* mysInstance;
#ifdef OMICRON_OS_WIN	
    WSADATA wsaData;
	SOCKET ConnectSocket;
	SOCKET RecvSocket;	
#else
	int ConnectSocket;
	int RecvSocket;
#endif
	struct timeval timeout;
	sockaddr_in SenderAddr;

	const char* serverAddress;
	int msgPort;
	int dataPort;
	float touchTimeout;

	#define DEFAULT_BUFLEN 512
	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;

	int SenderAddrSize;
	int recvbuflen;
	bool readyToReceive;
	int screenX;
	int screenY;

	std::map<int,NetTouches> touchlist;
	std::map<int,NetTouches> swaplist;
};

}; // namespace omicron

#endif
