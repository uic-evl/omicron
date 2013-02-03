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
// NOTE: This file does not use classic header #define guards, because it can be included
// multiple times in different configurations within the same translation unit.

// if OMICRON_CONNECTOR_LEAN_AND_MEAN, only define the omicron::EventBase and omicronConnector::EventData classes.
// Skip the OmicronConnectorClient class and all socket functionality.
#ifndef OMICRON_CONNECTOR_LEAN_AND_MEAN
	#ifdef WIN32
		#define OMICRON_OS_WIN
		#pragma comment(lib, "Ws2_32.lib")
	#endif

	#include <stdio.h>
	#ifdef OMICRON_OS_WIN
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#else
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

	#ifdef OMICRON_OS_WIN     
		#define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %d\n", WSAGetLastError());
		#define SOCKET_CLOSE(sock) closesocket(sock);
		#define SOCKET_CLEANUP() WSACleanup();
		#define SOCKET_INIT() \
			int iResult;    \
			iResult = WSAStartup(MAKEWORD(2,2), &wsaData); \
			if (iResult != 0) { \
				printf("OmicronConnectorClient: WSAStartup failed: %d\n", iResult); \
				return; \
			} else { \
				printf("OmicronConnectorClient: Winsock initialized \n"); \
			}

	#else
		#define SOCKET_CLOSE(sock) close(sock);
		#define SOCKET_CLEANUP()
		#define SOCKET_INIT()
		#define SOCKET int
		#define PRINT_SOCKET_ERROR(msg) printf(msg" - socket error: %s\n", strerror(errno));
	#endif

	#define OI_READBUF(type, buf, offset, val) val = *((type*)&buf[offset]); offset += sizeof(type);
#endif

#ifndef OMICRON_EVENTBASE_DEFINED
#define OMICRON_EVENTBASE_DEFINED
namespace omicron
{
	/** 
		* Yet another key code table to report keys in a window system
		* independent way. Ordinary keys (letters, numbers, etc) are reported
		* using the corresponding ascii code. The naming is oriented on the X11
		* keysym naming.
		*/
	enum KeyCode
	{
		KC_ESCAPE = 256,
		KC_BACKSPACE,
		KC_RETURN,
		KC_TAB,
		KC_HOME,
		KC_LEFT,
		KC_UP,
		KC_RIGHT,
		KC_DOWN,
		KC_PAGE_UP,
		KC_PAGE_DOWN,
		KC_END,
		KC_F1,
		KC_F2,
		KC_F3,
		KC_F4,
		KC_F5,
		KC_F6,
		KC_F7,
		KC_F8,
		KC_F9,
		KC_F10,
		KC_F11,
		KC_F12,
		KC_F13,
		KC_F14,
		KC_F15,
		KC_F16,
		KC_F17,
		KC_F18,
		KC_F19,
		KC_F20,
		KC_F21,
		KC_F22,
		KC_F23,
		KC_F24,
		KC_SHIFT_L,
		KC_SHIFT_R,
		KC_CONTROL_L,
		KC_CONTROL_R,
		KC_ALT_L,
		KC_ALT_R,
		KC_VOID = 0xFFFFFF /* == XK_VoidSymbol */
	};

	class EventBase
	{
	public:
		//! Enumerates the service classes supported by omicron. Each service class generates 
		//! events with the same structure.
		enum ServiceType { 
			ServiceTypePointer, 
			ServiceTypeMocap, 
			ServiceTypeKeyboard, 
			ServiceTypeController, 
			ServiceTypeUi, 
			ServiceTypeGeneric, 
			ServiceTypeBrain, 
			ServiceTypeWand, 
			ServiceTypeAudio }; 

		//! #PYAPI Supported event types.
		//! The python API exposed this enum in the EventType object.
		enum Type 
		{ 
			//! Select: generated when the source of the event gets selected or activated.
			//! Used primarily for use iterface controls.
			Select = 0,
			//! Toggle: generated when some boolean state in the event source changes. Can represent
			//! state changes in physical switches and buttons, or in user interface controls like
			//! check boxes and radio buttons.
			Toggle = 1,
			//!ChangeValue: generated when the source of an event changes it's internal value or state.
			//! Different from Update because ChangeValue is not usually fired at regular intervals,
			//! while Update events are normally sent at a constant rate.
			ChangeValue = 2,
			//! Update: Generated when the soruce of an event gets updated (what 'update') means depends
			//! on the event source.
			Update = 3,
			//! Move: Generated whenever the source of an event moves.
			Move = 4, 
			//! Down: generated when the source of an event goes to a logical 'down' state (i.e. touch on a surface or 
			//! a mouse button press count as Down events)
			Down = 5, 
			//! Up: generated when the source of an event goes to a logical 'up' state (i.e. remove touch from a surface or 
			//! a mouse button release count as Up events)
			Up = 6, 
			//! Trace: generated when a new object is identified by the device managed by the input service 
			//! (i.e head tracking, or a mocap system rigid body).
			Trace = 7,
			//! Alternate name for Trace events
			Connect = Trace,
			//! Trace: generated when a traced object is lost by the device managed by the input service 
			//! (i.e head tracking, or a mocap system rigid body).
			Untrace = 8,
			//! Alternate name for Untrace events
			Disconnect = Untrace,

			//! Click: generated on a down followed by an immediate up event.
			//! parameters: position
			Click = 9,
			//! Zoom: zoom event.
			Zoom = 15,
			//! Split: generated during a split/zoom gesture. 
			//! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions), value[0] (delta distance) value[1] (delta ratio) .
			Split = 18,
			//! Rotate: generated when an event source is stationary while a second source is rotating around the first.
			//! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions), rotation[0] (degrees).
			Rotate = 21,
			//! Null: generic null value for event type.
			Null = 666
		};
		//! #PYAPI Defines some generic input event flags
		enum Flags
		{
			//! Used for right mouse button or equivalent events.
			Left = 1 << 0,
			//! Generic name for left / main button
			Button1 = 1 <<0,

			//! Used for right mouse button or equivalent events.
			Right = 1 << 1,
			//! Generic name for right / secondary button
			Button2 = 1 << 1,

			//! Used for middle mouse button or equivalent events.
			Middle = 1 << 2,
			//! Generic name for middle / tertiary button
			Button3 = 1 << 2,

			//! Used for ctrl key presses or equivalent events.
			Ctrl = 1 << 3,
			//! Generic name for control key / primary modifier button
			SpecialButton1 = 1 << 3,

			//! Used for ctrl key presses or equivalent events.
			Alt = 1 << 4,
			//! Generic name for alt key / secondary modifier button
			SpecialButton2 = 1 << 4,

			//! Used for ctrl key presses or equivalent events.
			Shift = 1 << 5,
			//! Generic name for shift key / tertiary modifier button
			SpecialButton3 = 1 << 5,

			//! Generic name for additional button 4
			Button4 = 1 << 6,
			//! Generic name for additional button 5
			Button5 = 1 << 7,
			//! Generic name for additional button 6
			Button6 = 1 << 8,
			//! Generic name for additional button 7
			Button7 = 1 << 9,

			//! Generic name for digital up button
			ButtonUp = 1 << 10,
			//! Generic name for digital down button
			ButtonDown = 1 << 11,
			//! Generic name for digital left button
			ButtonLeft = 1 << 12,
			//! Generic name for digital right button
			ButtonRight = 1 << 13,

			//! INTERNAL: Used to mark events that have been processed
			Processed = 1 << 14,
			//! User flags should offset this value: 16 user flags available (USER to USER << 16)
			User = 1 << 15
		};

		enum ExtraDataType
		{
			ExtraDataNull,
			ExtraDataFloatArray,
			ExtraDataIntArray,
			ExtraDataVector3Array,
			ExtraDataString
		};

		//! Joint enumerations for Kinect (Uses OpenNI's enumerations with additional Kinect for Windows values)
		//! See MSKinectService.h, XnTypes.h (OpenNI), or NuiSensor.h (Kinect for Windows)
		enum OmicronSkeletonJoint{
			OMICRON_SKEL_HIP_CENTER, // MSKinect only - midpoint of left & right hips for OpenNI?
			OMICRON_SKEL_HEAD,
			OMICRON_SKEL_NECK, // OpenNI Only
			OMICRON_SKEL_TORSO, // OpenNI Only
			OMICRON_SKEL_WAIST, // OpenNI Only

			OMICRON_SKEL_LEFT_COLLAR, // OpenNI Only
			OMICRON_SKEL_LEFT_SHOULDER,
			OMICRON_SKEL_LEFT_ELBOW,
			OMICRON_SKEL_LEFT_WRIST,
			OMICRON_SKEL_LEFT_HAND,
			OMICRON_SKEL_LEFT_FINGERTIP, // OpenNI Only

			OMICRON_SKEL_LEFT_HIP,
			OMICRON_SKEL_LEFT_KNEE,
			OMICRON_SKEL_LEFT_ANKLE,
			OMICRON_SKEL_LEFT_FOOT,

			OMICRON_SKEL_RIGHT_COLLAR, // OpenNI Only
			OMICRON_SKEL_RIGHT_SHOULDER,
			OMICRON_SKEL_RIGHT_ELBOW,
			OMICRON_SKEL_RIGHT_WRIST,
			OMICRON_SKEL_RIGHT_HAND,
			OMICRON_SKEL_RIGHT_FINGERTIP, // OpenNI Only

			OMICRON_SKEL_RIGHT_HIP,
			OMICRON_SKEL_RIGHT_KNEE,
			OMICRON_SKEL_RIGHT_ANKLE,
			OMICRON_SKEL_RIGHT_FOOT,

			OMICRON_SKEL_SPINE,  // MSKinect only - merge with torso?
			OMICRON_SKEL_SHOULDER_CENTER,  // MSKinect only - midpoint of left & right shoulders for OpenNI?

			OMICRON_SKEL_COUNT
		};
	};
}
#endif

namespace omicronConnector
{
#ifndef OMICRON_EVENTDATA_DEFINED
#define OMICRON_EVENTDATA_DEFINED

	#define OFLOAT_PTR(x) *((float*)&x)
	#define OINT_PTR(x) *((int*)&x)

	//////////////////////////////////////////////////////////////////////////////////////////////////
	struct EventData: public omicron::EventBase
	{
		unsigned int timestamp;
		unsigned int sourceId;
		int serviceId;
		unsigned int serviceType;
		unsigned int type;
		unsigned int flags;
		float posx;
		float posy;
		float posz;
		float orx;
		float ory;
		float orz;
		float orw;

		static const int ExtraDataSize = 1024;
		unsigned int extraDataType;
		unsigned int extraDataItems;
		unsigned int extraDataMask;
		unsigned char extraData[ExtraDataSize];

		bool getExtraDataVector3(int index, float* data) const
		{
			if(extraDataType != ExtraDataVector3Array) return false;
			if(index >= extraDataItems) return false;

			int offset = index * 3 * 4;
			data[0] = OFLOAT_PTR(extraData[offset]);
			data[1] = OFLOAT_PTR(extraData[offset + 4]);
			data[2] = OFLOAT_PTR(extraData[offset + 8]);

			return true;
		}
		///////////////////////////////////////////////////////////////////////////////////////////////
		inline float getExtraDataFloat(int index) const
		{
			if(extraDataType != ExtraDataFloatArray) return false;
			if(index >= extraDataItems) return false;

			return OFLOAT_PTR(extraData[index * 4]);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		inline int getExtraDataInt(int index) const
		{
			if(extraDataType != ExtraDataIntArray) return false;
			if(index >= extraDataItems) return false;
			return OINT_PTR(extraData[index * 4]);
		}
	};
#endif

// if OMICRON_CONNECTOR_LEAN_AND_MEAN, only define the omicron::EventBase and omicronConnector::EventData classes.
// Skip the OmicronConnectorClient class and all socket functionality.
#ifndef OMICRON_CONNECTOR_LEAN_AND_MEAN
#ifndef OMICRON_CONNECTORCLIENT_DEFINED
#define OMICRON_CONNECTORCLIENT_DEFINED
	//////////////////////////////////////////////////////////////////////////////////////////////////
	class IOmicronConnectorClientListener
	{
	public:
		virtual void onEvent(const EventData& e) = 0;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//template<typename ListenerType>
	class OmicronConnectorClient
	{
	public:
		OmicronConnectorClient(IOmicronConnectorClientListener* clistener): listener(clistener)
		{}

		void connect(const char* server, int port = 27000, int dataPort = 7000);
		void poll();
		void dispose();
		void setDataport(int);

	private:
		void initHandshake();
		void parseDGram(int);

	private:
		//typedef ListenerType Listener;

	#ifdef OMICRON_OS_WIN	
		WSADATA wsaData;
	#endif

		SOCKET ConnectSocket;
		SOCKET RecvSocket;
		struct timeval timeout;
		sockaddr_in SenderAddr;

		const char* serverAddress;
		int serverPort;
		int dataPort;

		#define DEFAULT_BUFLEN 512
		char recvbuf[DEFAULT_BUFLEN];
		int iResult, iSendResult;

		int SenderAddrSize;
		int recvbuflen;
		bool readyToReceive;

		IOmicronConnectorClientListener* listener;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//template<typename ListenerType>
	inline void OmicronConnectorClient::connect(const char* server, int port, int pdataPort) 
	{
		serverAddress = server;
		serverPort = port;
		dataPort = pdataPort;

		char srvPortChr[256];
		sprintf(srvPortChr, "%d", serverPort);

		SOCKET_INIT();

		struct addrinfo hints, *res;

		// First, load up address structs with getaddrinfo():
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		// Get the server address
		getaddrinfo(serverAddress, srvPortChr, &hints, &res);
		if(res == NULL)
		{
			printf("omicronConnectorClient: Unable to connect to server '%s' on port '%d'", serverAddress, serverPort);
			PRINT_SOCKET_ERROR("");
			SOCKET_CLEANUP();
			return;
		}

		// Generate the socket
		ConnectSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		// Connect to server
		int result = ::connect(ConnectSocket, res->ai_addr, res->ai_addrlen);

		if (result == -1) 
		{
			printf("omicronConnectorClient: Unable to connect to server '%s' on port '%d'", serverAddress, serverPort);
			PRINT_SOCKET_ERROR("");
			SOCKET_CLEANUP();
			return;
		}
		else
		{
			printf("NetService: Connected to server '%s' on port '%d'!\n", serverAddress, serverPort);
		}
		initHandshake();

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//template<typename ListenerType>
	inline void OmicronConnectorClient::initHandshake() 
	{
		char sendbuf[50];
		sprintf(sendbuf, "data_on,%d", dataPort);
		printf("NetService: Sending handshake: '%s'\n", sendbuf);

		iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);

		if (iResult == -1) 
		{
			PRINT_SOCKET_ERROR("NetService: Send failed");
			SOCKET_CLOSE(ConnectSocket);
			SOCKET_CLEANUP()
			return;
		}

		sockaddr_in RecvAddr;
		SenderAddrSize = sizeof(SenderAddr);
		RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		// Bind the socket to any address and the specified port.
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(dataPort);
		RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		::bind(RecvSocket, (const sockaddr*) &RecvAddr, sizeof(RecvAddr));
		readyToReceive = true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//template<typename ListenerType>
	inline void OmicronConnectorClient::poll()
	{
		if(readyToReceive)
		{
			int result;

			// Create a set of fd_set to store sockets
			fd_set ReadFDs, WriteFDs, ExceptFDs;

			// Set collections to null
			FD_ZERO(&ReadFDs);
			FD_ZERO(&WriteFDs);
			FD_ZERO(&ExceptFDs);

			FD_SET(RecvSocket, &ReadFDs);
			FD_SET(RecvSocket, &ExceptFDs);

			timeout.tv_sec  = 0;
			timeout.tv_usec = 0;

			do
			{
				// Check if UDP socket has data waiting to be read before socket blocks to attempt to read.
				result = select(RecvSocket+1, &ReadFDs, &WriteFDs, &ExceptFDs, &timeout);
				if( result > 0 ) parseDGram(result);
			} while(result > 0);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//template<typename ListenerType>
	inline void OmicronConnectorClient::dispose() 
	{
		// Close the socket when finished receiving datagrams
		printf("NetService: Finished receiving. Closing socket.\n");
		iResult = SOCKET_CLOSE(RecvSocket);
		if (iResult == -1) 
		{
			PRINT_SOCKET_ERROR("NetService: Closesocket failed");
			return;
		}
		SOCKET_CLEANUP();
		printf("NetService: Shutting down.");
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//template<typename ListenerType>
	inline void OmicronConnectorClient::parseDGram(int result)
	{
		result = recvfrom(RecvSocket, 
			recvbuf,
			DEFAULT_BUFLEN-1,
			0,
			(sockaddr *)&SenderAddr, 
			(socklen_t*)&SenderAddrSize);
		if(result > 0)
		{
			int offset = 0;
			int msgLen = result - 1;
			char* eventPacket = recvbuf;

			EventData ed;

			OI_READBUF(unsigned int, eventPacket, offset, ed.timestamp); 
			OI_READBUF(unsigned int, eventPacket, offset, ed.sourceId); 
			OI_READBUF(int, eventPacket, offset, ed.serviceId); 
			OI_READBUF(unsigned int, eventPacket, offset, ed.serviceType); 
			OI_READBUF(unsigned int, eventPacket, offset, ed.type); 
			OI_READBUF(unsigned int, eventPacket, offset, ed.flags); 
			OI_READBUF(float, eventPacket, offset, ed.posx); 
			OI_READBUF(float, eventPacket, offset, ed.posy); 
			OI_READBUF(float, eventPacket, offset, ed.posz); 
			OI_READBUF(float, eventPacket, offset, ed.orw); 
			OI_READBUF(float, eventPacket, offset, ed.orx); 
			OI_READBUF(float, eventPacket, offset, ed.ory); 
			OI_READBUF(float, eventPacket, offset, ed.orz); 
		
			OI_READBUF(unsigned int, eventPacket, offset, ed.extraDataType); 
			OI_READBUF(unsigned int, eventPacket, offset, ed.extraDataItems); 
			OI_READBUF(unsigned int, eventPacket, offset, ed.extraDataMask); 
			memcpy(ed.extraData, &eventPacket[offset], EventData::ExtraDataSize);

			listener->onEvent(ed);
		} 
		else 
		{
			PRINT_SOCKET_ERROR("recvfrom failed");
		}
	}
#endif
#endif
};
