/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2014, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __OMICRON_INPUT_SERVER__
#define __OMICRON_INPUT_SERVER__

#include "omicron/osystem.h"
#include "omicron/DataManager.h"
#include "omicron/Event.h"
#include "omicron/Config.h"

#ifdef WIN32
    #define OMICRON_OS_WIN
    #pragma comment(lib, "Ws2_32.lib")
#endif

#ifdef OMICRON_OS_WIN
    #include <winsock2.h>
    #include <ws2tcpip.h>

    // VRPN Server (for CalVR)
#ifdef OMICRON_USE_VRPN
    #include "omicron/vrpn/vrpn_tracker.h"
    #include "omicron/VRPNDevice.h"
#endif
#else
    #include <stdlib.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <unistd.h> // needed for close()
    #include <string>
#endif

#ifdef OMICRON_OS_WIN
    #define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %d\n", WSAGetLastError());
    #define SOCKET_CLOSE(sock) closesocket(sock);
    #define SOCKET_CLEANUP() WSACleanup();
    #define SOCKET_INIT() \
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData); \
        if (iResult != 0) { \
            printf("%s: WSAStartup failed: %d\n", typeid(*this).name(), iResult); \
            return; \
        } else { \
            printf("%s: Winsock initialized \n",  typeid(*this).name()); \
        }
#else
    #define SOCKET_CLOSE(sock) close(sock);
    #define SOCKET_CLEANUP()
    #define SOCKET_INIT()
    #define SOCKET int
    #define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %s\n", strerror(errno));
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR   -1
    #define ioctlsocket ioctl // Used for setting socket blocking mode
#endif

class NetClient;
namespace omicron {
///////////////////////////////////////////////////////////////////////////////
class OMICRON_API InputServer
{
public:
    virtual void handleEvent(const Event& evt);
    virtual bool handleLegacyEvent(const Event& evt);
    void startConnection(Config* cfg);
    SOCKET startListening();
    // VRPN Server (for CalVR)
    void loop();

protected:
	char* createOmicronEventPacket(const Event*);
	void sendToClients(char*);
	void createClient(const char*,int, bool, SOCKET);
private:
    enum dataMode { omicron, omicron_legacy };
    

    const char* serverPort;
    SOCKET listenSocket;    
    
    #define DEFAULT_BUFLEN 512
	char eventPacket[DEFAULT_BUFLEN];
	char legacyPacket[DEFAULT_BUFLEN];

    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen;
    
    // Collection of unique clients (IP/port combinations)
    std::map<char*,NetClient*> netClients;

    bool checkForDisconnectedClients;

    bool showEventStream;
    bool showStreamSpeed;
    int lastOutgoingEventTime;
    int eventCount;

#ifdef OMICRON_USE_VRPN
    // VRPN Server (for CalVR)
    const char	*TRACKER_NAME;
    int TRACKER_PORT;
    vrpn_XInputGamepad	*vrpnDevice;
    vrpn_Tracker_Remote	*tkr;
    vrpn_Connection		*connection;
#endif
};
};

#endif