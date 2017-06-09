/******************************************************************************
 * THE OMICRON SDK
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2014, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and * the following disclaimer in the documentation and/or other materials provided
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file:
 *	The header-only omicron connector client, used to connect to an omicron 
 *  input server to receive event data
 ******************************************************************************/
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
        KEY_UNKNOWN  = -1,
        KEY_SPACE   =32,
        KEY_APOSTROPHE =39, /* ' */
        KEY_COMMA  =44, /* , */ 
        KEY_MINUS  =45 ,/* - */
        KEY_PERIOD  =46 ,/* . */
        KEY_SLASH  =47 ,/* / */
        KEY_0  =48,
        KEY_1  =49,
        KEY_2  =50,
        KEY_3  =51,
        KEY_4  =52,
        KEY_5  =53,
        KEY_6  =54,
        KEY_7  =55,
        KEY_8  =56,
        KEY_9  =57,
        KEY_SEMICOLON  =59,/* ; */
        KEY_EQUAL  =61,/* = */
        KEY_A  =65,
        KEY_B  =66,
        KEY_C  =67,
        KEY_D  =68,
        KEY_E  =69,
        KEY_F  =70,
        KEY_G  =71,
        KEY_H  =72,
        KEY_I  =73,
        KEY_J  =74,
        KEY_K  =75,
        KEY_L  =76,
        KEY_M  =77,
        KEY_N  =78,
        KEY_O  =79,
        KEY_P  =80,
        KEY_Q  =81,
        KEY_R  =82,
        KEY_S  =83,
        KEY_T  =84,
        KEY_U  =85,
        KEY_V  =86,
        KEY_W  =87,
        KEY_X  =88,
        KEY_Y  =89,
        KEY_Z  =90,
        KEY_LEFT_BRACKET  =91, /* [ */
        KEY_BACKSLASH  =92, /* \ */
        KEY_RIGHT_BRACKET  =93, /* ] */
        KEY_GRAVE_ACCENT  =96, /* ` */
        KEY_WORLD_1  =161, /* non-US #1 */
        KEY_WORLD_2  =162, /* non-US #2 */
        KEY_ESCAPE  =256,
        KEY_ENTER  =257,
        KEY_TAB  =258,
        KEY_BACKSPACE  =259,
        KEY_INSERT  =260,
        KEY_DELETE  =261,
        KEY_RIGHT  =262,
        KEY_LEFT  =263,
        KEY_DOWN  =264,
        KEY_UP  =265,
        KEY_PAGE_UP  =266,
        KEY_PAGE_DOWN  =267,
        KEY_HOME  =268,
        KEY_END  =269,
        KEY_CAPS_LOCK  =280,
        KEY_SCROLL_LOCK  =281,
        KEY_NUM_LOCK  =282,
        KEY_PRINT_SCREEN  =283,
        KEY_PAUSE  =284,
        KEY_F1  =290,
        KEY_F2  =291,
        KEY_F3  =292,
        KEY_F4  =293,
        KEY_F5  =294,
        KEY_F6  =295,
        KEY_F7  =296,
        KEY_F8  =297,
        KEY_F9  =298,
        KEY_F10  =299,
        KEY_F11  =300,
        KEY_F12  =301,
        KEY_F13  =302,
        KEY_F14  =303,
        KEY_F15  =304,
        KEY_F16  =305,
        KEY_F17  =306,
        KEY_F18  =307,
        KEY_F19  =308,
        KEY_F20  =309,
        KEY_F21  =310,
        KEY_F22  =311,
        KEY_F23  =312,
        KEY_F24  =313,
        KEY_F25  =314,
        KEY_KP_0  =320,
        KEY_KP_1  =321,
        KEY_KP_2  =322,
        KEY_KP_3  =323,
        KEY_KP_4  =324,
        KEY_KP_5  =325,
        KEY_KP_6  =326,
        KEY_KP_7  =327,
        KEY_KP_8  =328,
        KEY_KP_9  =329,
        KEY_KP_DECIMAL  =330,
        KEY_KP_DIVIDE  =331,
        KEY_KP_MULTIPLY  =332,
        KEY_KP_SUBTRACT  =333,
        KEY_KP_ADD  =334,
        KEY_KP_ENTER  =335,
        KEY_KP_EQUAL  =336,
        KEY_LEFT_SHIFT  =340,
        KEY_LEFT_CONTROL  =341,
        KEY_LEFT_ALT  =342,
        KEY_LEFT_SUPER  =343,
        KEY_RIGHT_SHIFT  =344,
        KEY_RIGHT_CONTROL  =345,
        KEY_RIGHT_ALT  =346,
        KEY_RIGHT_SUPER  =347,
        KEY_MENU  =348
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
            ServiceTypeSpeech }; 

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
            //! Used for confirm button prsses or equivalent events
            Enter = 1 << 6,

            //! Generic name for additional button 5
            Button5 = 1 << 7,
            //! Used for backspace button presses or equivalent events
            Backspace = 1 << 7,

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

            //! Generic name for additional button 8
            Button8 = 1 << 15,
            //! Generic name for additional button 9
            Button9 = 1 << 16,

            //! INTERNAL: Used to mark events that have been processed
            Processed = 1 << 14,
            //! INTERNAL: Used to mark events that are sent to a single endpoint
            //! instead of being broadcast
            Exclusive = 1 << 17,

            //! User flags should offset this value: 14 user flags available (USER to USER << 18)
            User = 1 << 18
        };

        enum ExtraDataType
        {
            ExtraDataNull,
            ExtraDataFloatArray,
            ExtraDataIntArray,
            ExtraDataVector3Array,
            ExtraDataString,
            ExtraDataKinectSpeech
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

            OMICRON_SKEL_LEFT_THUMB,
            OMICRON_SKEL_RIGHT_THUMB,

            OMICRON_SKEL_COUNT
        };

        //! Bit masks used to read the service id and user id from the event
        //! device tag.
        enum DeviceTagMask
        {
            DTServiceIdMask = 0x0000ffff,
            DTUserIdMask = 0xffff0000,
            DTUserIdOffset = 16,
            DTServiceIdOffset = 0
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
        unsigned int deviceTag;
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
#if !defined(__GNUC__)
        unsigned int extraDataItems;
#else
        int extraDataItems; // GCC complains of signed-unsigned comparison
#endif
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

        bool connect(const char* server, int port = 27000, int dataPort = 7000, int mode = 0);
        void poll();
        void dispose();
        void setDataport(int);

    private:
        bool initHandshake(int);
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

        #define DEFAULT_BUFLEN 1024
        char recvbuf[DEFAULT_BUFLEN];
        int iResult, iSendResult;

        int SenderAddrSize;
        int recvbuflen;
        bool readyToReceive;

        IOmicronConnectorClientListener* listener;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //template<typename ListenerType>
    inline bool OmicronConnectorClient::connect(const char* server, int port, int pdataPort, int mode) 
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
            return false;
        }

        // Generate the socket
        ConnectSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        // Connect to server
        int result = ::connect(ConnectSocket, res->ai_addr, (int)res->ai_addrlen);

        if (result == -1) 
        {
            printf("omicronConnectorClient: Unable to connect to server '%s' on port '%d'", serverAddress, serverPort);
            PRINT_SOCKET_ERROR("");
            SOCKET_CLEANUP();
            return false;
        }
        else
        {
            printf("NetService: Connected to server '%s' on port '%d'!\n", serverAddress, serverPort);
        }
        return initHandshake(mode);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //template<typename ListenerType>
    inline bool OmicronConnectorClient::initHandshake(int mode) 
    {
        char sendbuf[50];
		if (mode == 1)
		{
			sprintf(sendbuf, "omicron_data_in,%d", dataPort);
		}
		else
		{
			sprintf(sendbuf, "omicron_data_on,%d", dataPort);
		}
        printf("NetService: Sending handshake: '%s'\n", sendbuf);

        iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);

        if (iResult == -1) 
        {
            PRINT_SOCKET_ERROR("NetService: Send failed");
            SOCKET_CLOSE(ConnectSocket);
            SOCKET_CLEANUP()
            return false;
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
		return true;
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
		char sendbuf[50];
        sprintf(sendbuf, "data_off");
        printf("NetService: Sending disconnect signal: '%s'\n", sendbuf);
        iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);

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
#if !defined (__GNUC__) // gcc with Werror does not like unused variables
            int msgLen = result - 1;
#endif
            char* eventPacket = recvbuf;

            EventData ed;

            OI_READBUF(unsigned int, eventPacket, offset, ed.timestamp); 
            OI_READBUF(unsigned int, eventPacket, offset, ed.sourceId); 
            OI_READBUF(unsigned int, eventPacket, offset, ed.deviceTag); 
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
