/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
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

using System.Linq;
using System.Collections;

using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.IO;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using omicron;
using UnityEngine;

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

    public class EventBase
    {
        //! Enumerates the service classes supported by omicron. Each service class generates 
        //! events with the same structure.
        public enum ServiceType
        {
            ServiceTypePointer,
            ServiceTypeMocap,
            ServiceTypeKeyboard,
            ServiceTypeController,
            ServiceTypeUi,
            ServiceTypeGeneric,
            ServiceTypeBrain,
            ServiceTypeWand,
            ServiceTypeSpeech
        };

        //! #PYAPI Supported event types.
        //! The python API exposed this enum in the EventType object.
        public enum Type
        {
            //! Select: generated when the source of the event gets selected or activated.
            //! Used primarily for use iterface controls.
            Select,
            //! Toggle: generated when some boolean state in the event source changes. Can represent
            //! state changes in physical switches and buttons, or in user interface controls like
            //! check boxes and radio buttons.
            Toggle,
            //!ChangeValue: generated when the source of an event changes it's internal value or state.
            //! Different from Update because ChangeValue is not usually fired at regular intervals,
            //! while Update events are normally sent at a constant rate.
            ChangeValue,
            //! Update: Generated when the soruce of an event gets updated (what 'update') means depends
            //! on the event source.
            Update,
            //! Move: Generated whenever the source of an event moves.
            Move,
            //! Down: generated when the source of an event goes to a logical 'down' state (i.e. touch on a surface or 
            //! a mouse button press count as Down events)
            Down,
            //! Up: generated when the source of an event goes to a logical 'up' state (i.e. remove touch from a surface or 
            //! a mouse button release count as Up events)
            Up,
            //! Trace: generated when a new object is identified by the device managed by the input service 
            //! (i.e head tracking, or a mocap system rigid body).
            Trace,
            //! Alternate name for Trace events
            Connect = Trace,
            //! Trace: generated when a traced object is lost by the device managed by the input service 
            //! (i.e head tracking, or a mocap system rigid body).
            Untrace,
            //! Alternate name for Untrace events
            Disconnect = Untrace,

            //! Click: generated on a down followed by an immediate up event.
            //! parameters: position
            Click,
            //! DoubleClick: generated by a sequence of quick down/up/down/up events.
            //! parameters: position.
            DoubleClick,
            //! MoveLeft: generated when the source of event goes toward the left of the screen.
            //! parameters: position.
            MoveLeft,
            //! MoveRight: generated when the source of event goes toward the right of the screen.
            //! parameters: position.
            MoveRight,
            //! MoveUp: generated when the source of event goes toward the top of the screen.
            //! parameters: position.
            MoveUp,
            //! MoveDown: generated when the source of event goes toward the bottom of the screen.
            //! parameters: position.
            MoveDown,
            //! Zoom: zoom event.
            Zoom,
            //! SplitStart: generated at the start of a split/zoom gesture.
            //! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions) .
            SplitStart,
            //! SplitEnd: generated at the end of a split/zoom gesture.
            //! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions) .
            SplitEnd,
            //! Split: generated during a split/zoom gesture. 
            //! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions), value[0] (delta distance) value[1] (delta ratio) .
            Split,
            //! RotateStart: generated at the start of a rotation gesture.
            //! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions) .
            RotateStart,
            //! RotateEnd: generated at the end of a rotation gesture.
            //! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions) .
            RotateEnd,
            //! Rotate: generated when an event source is stationary while a second source is rotating around the first.
            //! parameters: position (center of gesture) pointSet[0, 1] (individual finger positions), rotation[0] (degrees).
            Rotate,
            //! Null: generic null value for event type.
            Null
        };
        //! #PYAPI Defines some generic input event flags
        public enum Flags
        {
            //! Used for right mouse button or equivalent events.
            Left = 1 << 0,
            //! Generic name for left / main button
            Button1 = 1 << 0,

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

        public enum ExtraDataType
        {
            ExtraDataNull,
            ExtraDataFloatArray,
            ExtraDataIntArray,
            ExtraDataVector3Array,
            ExtraDataString
        };

        //! Joint enumerations for Kinect (Uses OpenNI's enumerations with additional Kinect for Windows values)
        //! See MSKinectService.h, XnTypes.h (OpenNI), or NuiSensor.h (Kinect for Windows)
        public enum OmicronSkeletonJoint
        {
            OMICRON_SKEL_HIP_CENTER,
            OMICRON_SKEL_HEAD,
            OMICRON_SKEL_NECK,
            OMICRON_SKEL_TORSO,
            OMICRON_SKEL_WAIST,

            OMICRON_SKEL_LEFT_COLLAR,
            OMICRON_SKEL_LEFT_SHOULDER,
            OMICRON_SKEL_LEFT_ELBOW,
            OMICRON_SKEL_LEFT_WRIST,
            OMICRON_SKEL_LEFT_HAND,
            OMICRON_SKEL_LEFT_FINGERTIP,

            OMICRON_SKEL_LEFT_HIP,
            OMICRON_SKEL_LEFT_KNEE,
            OMICRON_SKEL_LEFT_ANKLE,
            OMICRON_SKEL_LEFT_FOOT,

            OMICRON_SKEL_RIGHT_COLLAR,
            OMICRON_SKEL_RIGHT_SHOULDER,
            OMICRON_SKEL_RIGHT_ELBOW,
            OMICRON_SKEL_RIGHT_WRIST,
            OMICRON_SKEL_RIGHT_HAND,
            OMICRON_SKEL_RIGHT_FINGERTIP,

            OMICRON_SKEL_RIGHT_HIP,
            OMICRON_SKEL_RIGHT_KNEE,
            OMICRON_SKEL_RIGHT_ANKLE,
            OMICRON_SKEL_RIGHT_FOOT,

            OMICRON_SKEL_SPINE,
            OMICRON_SKEL_SHOULDER_CENTER,

            OMICRON_SKEL_COUNT
        };
    };
}

namespace omicronConnector
{
    //#define OFLOAT_PTR(x) *((float*)&x)
    //#define OINT_PTR(x) *((int*)&x)

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class EventData : EventBase
    {
        public uint timestamp;
        public uint sourceId;
        public int serviceId;
        public EventBase.ServiceType serviceType;
        public uint type;
        public uint flags;
        public float posx;
        public float posy;
        public float posz;
        public float orx;
        public float ory;
        public float orz;
        public float orw;

        public const int ExtraDataSize = 1024;
        public omicron.EventBase.ExtraDataType extraDataType;
        public uint extraDataItems;
        public uint extraDataMask;
        public byte[] extraData = new byte[ExtraDataSize];

        public bool getExtraDataVector3(int index, float[] data)
        {
            if (extraDataType != omicron.EventBase.ExtraDataType.ExtraDataVector3Array) return false;
            if (index >= extraDataItems) return false;

            int offset = index * 3 * 4;
            //data[0] = OFLOAT_PTR(extraData[offset]);
            //data[1] = OFLOAT_PTR(extraData[offset + 4]);
            //data[2] = OFLOAT_PTR(extraData[offset + 8]);

            data[0] = BitConverter.ToSingle(extraData, offset);
            data[1] = BitConverter.ToSingle(extraData, offset + 4);
            data[2] = BitConverter.ToSingle(extraData, offset + 8);

            return true;
        }
		
        ///////////////////////////////////////////////////////////////////////////////////////////////
        public float getExtraDataFloat(int index)
        {
            if (extraDataType != omicron.EventBase.ExtraDataType.ExtraDataFloatArray) return 0;
            if (index >= extraDataItems) return 0;

            return BitConverter.ToSingle(extraData, index * 4);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        public int getExtraDataInt(int index)
        {
            if (extraDataType != omicron.EventBase.ExtraDataType.ExtraDataIntArray) return 0;
            if (index >= extraDataItems) return 0;
            return BitConverter.ToInt32(extraData, index * 4);
        }
		
		///////////////////////////////////////////////////////////////////////////////////////////////
        public string getExtraDataString()
        {
            if (extraDataType != omicron.EventBase.ExtraDataType.ExtraDataString) return "";
			extraData = Encoding.Convert(Encoding.GetEncoding("iso-8859-1"), Encoding.UTF8, extraData);
            string dataString = Encoding.UTF8.GetString(extraData, 0, (int)extraDataItems);
			dataString = dataString.Substring(0, ((int)extraDataItems / sizeof(char)) - 1 );
			return dataString;
        }
    };

    abstract class IOmicronConnectorClientListener
	{
		public abstract void onEvent(EventData e);
	};

    class OmicronConnectorClient 
    {
        // TCP Connection
        TcpClient client;
        NetworkStream streamToServer;

        // UDP Connection
        private static UdpClient udpClient;
        private static Thread listenerThread;

        public String InputServer = "localhost";
        public Int32 dataPort = 7000;
        public Int32 msgPort = 27000;

        public bool EnableInputService = true;

        static IOmicronConnectorClientListener listener;
		
		public OmicronConnectorClient()
        {
        }
		
        public OmicronConnectorClient(IOmicronConnectorClientListener clistener)
        {
            listener = clistener;
        }

        public void Connect(string serverIP, int msgPort, int dataPort)
        {
            if (EnableInputService)
            {
                //dgrams = new ArrayList();

                Debug.Log("OmicronConnector: Initializing... ");
                try
                {
                    // Create a TcpClient.
					Debug.Log("InputService: Connecting to to " + serverIP);
                    client = new TcpClient(serverIP, msgPort);

                    // Translate the passed message into ASCII and store it as a Byte array.
                    String message = "omicron_data_on," + dataPort;
                    Byte[] data = System.Text.Encoding.ASCII.GetBytes(message);

                    streamToServer = client.GetStream();

                    // Create the UDP socket data will be received on
                    udpClient = new UdpClient(dataPort);

                    // Send the handshake message to the server.
                    streamToServer.Write(data, 0, data.Length);

                    //Console.WriteLine("Handshake Sent: {0}", message);
                    Debug.Log("InputService: Connected to " + serverIP);

                    // Creates a separate thread to listen for incoming data
                    listenerThread = new Thread(Listen);
                    listenerThread.Start();
                }
                catch (ArgumentNullException e)
                {
                    Debug.LogError("ArgumentNullException: " + e);
                }
                catch (SocketException e)
                {
                    Debug.LogError("SocketException: " + e);
                }
            }
        }// CTOR

        public void Dispose() 
	    {
		    // Close the socket when finished receiving datagrams
            Debug.Log("OmicronConnectorClient: Finished receiving. Closing socket.\n");
            udpClient.Close();
            listenerThread.Abort();

            // Close TCP connection.
            streamToServer.Close();
            client.Close();

            Debug.Log("OmicronConnectorClient: Shutting down.");
	    }

        private static void Listen()
        {
            //Debug.Log("InputService: Ready to receive data");
            while (true)
            {
                //IPEndPoint object will allow us to read datagrams sent from any source.
                IPEndPoint RemoteIpEndPoint = new IPEndPoint(IPAddress.Any, 0);

                // Blocks until a message returns on this socket from a remote host.
                Byte[] receiveBytes = udpClient.Receive(ref RemoteIpEndPoint);
                //Console.WriteLine("receiveBytes length: " + receiveBytes.Length);

                //#define OI_READBUF(type, buf, offset, val) val = *((type*)&buf[offset]); offset += sizeof(type);

                MemoryStream ms = new MemoryStream();
                ms.Write(receiveBytes, 0, receiveBytes.Length);

                BinaryReader reader = new BinaryReader(ms);
                omicronConnector.EventData ed = new omicronConnector.EventData();
                reader.BaseStream.Position = 0;

                ed.timestamp = reader.ReadUInt32();
                ed.sourceId = reader.ReadUInt32();
                ed.serviceId = reader.ReadInt32();
                ed.serviceType = (EventBase.ServiceType)reader.ReadUInt32();
                ed.type = reader.ReadUInt32();
                ed.flags = reader.ReadUInt32();

                if (ed.type != 3)
                {
                    Console.WriteLine(ed.type);
                    Console.WriteLine(ed.flags);
                    Console.WriteLine("---");
                }
                
                ed.posx = reader.ReadSingle();
                ed.posy = reader.ReadSingle();
                ed.posz = reader.ReadSingle();
                ed.orw = reader.ReadSingle();
                ed.orx = reader.ReadSingle();
                ed.ory = reader.ReadSingle();
                ed.orz = reader.ReadSingle();

                ed.extraDataType = (omicron.EventBase.ExtraDataType)reader.ReadUInt32();
                ed.extraDataItems = reader.ReadUInt32();
                ed.extraDataMask = reader.ReadUInt32();

                ed.extraData = reader.ReadBytes(EventData.ExtraDataSize);

                listener.onEvent(ed);      
            }
        }// Listen()

        void Update()
        {

        }// Update()
    }
}
