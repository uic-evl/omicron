package omicronAPI;

import hypermedia.net.UDP;

/**************************************************************************************************
 * THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto            arthur.nishimoto@gmail.com
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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import processing.core.PApplet;
import processing.core.PVector;
import processing.net.Client;

import java.awt.event.MouseEvent;
import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;

public class OmicronAPI
{
	public enum ServiceType
	{
		Pointer, Mocap, Keyboard, Controller, UI, Generic, Brain, Wand, Audio, Speech, Kinect
	};

	public enum Type
	{
		Select, Toggle, ChangeValue, Update, Move, Down, Up, Trace, Untrace, Click, DoubleClick, MoveLeft, MoveRight, MoveUp, MoveDown, Zoom, SplitStart, SplitEnd, Split, RotateStart, RotateEnd, Rotate, Null
	};
	
	public enum ExtraDataType
	{
		ExtraDataNull, ExtraDataFloatArray, ExtraDataIntArray, ExtraDataVector3Array, ExtraDataString
	};
	
	final static ServiceType[] serviceType = ServiceType.values();

	// Service Types (should be synchronized with omicronConnectorClient.h)
	// public final static int POINTER = 0;
	// public final static int MOCAP = 1;
	// public final static int KEYBOARD = 2;
	// public final static int CONTROLLER = 3;
	// public final static int UI = 4;
	// public final static int GENERIC = 5;
	// public final static int BRAIN = 6;
	// public final static int WAND = 7;
	// public final static int AUDIO = 8;

	// Service IDs for Pointer
	final static int DYNALLAX = -1;
	final static int MOUSE = 0;
	final static int TOUCH = 1;

	// Source IDs for Mocap
	final static int HIP_CENTER = 0;
	final static int HEAD = 3;
	final static int L_ELBOW = 5;
	final static int L_HAND = 7;
	final static int R_ELBOW = 9;
	final static int R_HAND = 11;
	final static int L_KNEE = 13;
	final static int L_FOOT = 15;
	final static int R_KNEE = 17;
	final static int R_FOOT = 19;


	// Event Types (Planned for Omicron)
	final static int MultiHold = 22;
	final static int MultiSwipe = 23;
	final static int BigTouch = 24;

	PApplet applet;
	OmicronListener eventListener;
	OmicronTouchListener touchListener;

	private boolean trackerOn = false;
	private boolean legacyMode = false;
	
	private boolean mouseTouchEmulation = true;
	
	// Fullscreen
	private boolean fullscreenEnabled = false;
	private float frameSetDelay = 3; // Delay in seconds before java frame is set

	// Screen scaling
	private boolean scaleScreen = false;
	private float screenScale = 1;
	private float screenOffsetX = 0;
	private float screenOffsetY = 0;
	private float targetWidth;
	private float targetHeight;
	
	// Viewing mode
	private boolean scaledViewMode = false;	
	
	/**
	 * Constructor used to setup fullscreen or scaling functions. This must be
	 * called in init() to work properly.
	 * 
	 * @param parent
	 *            the parent object so the class knows how to get back to the
	 *            Processing Application
	 */
	public OmicronAPI(PApplet parent)
	{
		this.applet = parent;
		trackerOn = false;
		
		applet.addMouseWheelListener(new java.awt.event.MouseWheelListener()
		{
			public void mouseWheelMoved(java.awt.event.MouseWheelEvent evt)
			{
				omicronMouseWheel(evt.getWheelRotation());
			}
		});
	}// constructor

	/**
	 * Constructor used to create a connection between an Omicron server and a
	 * Processing application. This can be called in setup() if
	 * fullscreen/scaling functions will not be used.
	 * 
	 * @param parent
	 *            the parent object so the class knows how to get back to the
	 *            Processing Application
	 * @param dataPort
	 *            the client port where data should be sent
	 * @param msgPort
	 *            the server port where the message to begin data transfer is
	 *            set
	 * @param trackerIP
	 *            the IP address of the Omicron server
	 */
	public OmicronAPI(PApplet parent, int dataPort, int msgPort, String trackerIP)
	{
		connectToTracker(dataPort, msgPort, trackerIP);
		eventList = new ArrayList<Event>();
		this.applet = parent; // This needs to match sketch name i.e.
								// '(SketchName)parent'
		trackerOn = true;
		
		applet.addMouseWheelListener(new java.awt.event.MouseWheelListener()
		{
			public void mouseWheelMoved(java.awt.event.MouseWheelEvent evt)
			{
				omicronMouseWheel(evt.getWheelRotation());
			}
		});
	}// constructor

	/**
	 * Sets the OmicronListener callback function used by the Processing
	 * application to receive all event data.
	 * 
	 * @param listener
	 *            OmicronListener class implemented by the Processing
	 *            application
	 */
	public void setEventListener(OmicronListener listener)
	{
		eventListener = listener;
	}// setEventListener

	/**
	 * Sets the OmicronTouchListener callback function used by the Processing
	 * application to receive touch data.
	 * 
	 * @param listener
	 *            TouchListener class implemented by the Processing application
	 */
	public void setTouchListener(OmicronTouchListener listener)
	{
		touchListener = listener;
	}// setTouchListener

	/**
	 * Fullscreen mode will remove the Java frame and set the window location at
	 * 0,0. This is useful for systems where Processing's Present mode won't
	 * work - such as a tiled wall using multiple graphics cards in xinerama
	 * (linux)
	 * 
	 * This must be called in init() not setup() to work properly.
	 * 
	 * @param value
	 *            Boolean to toggle fullscreen mode
	 */
	public void setFullscreen(boolean value)
	{
		if (applet.frame != null && value)
		{
			applet.frame.removeNotify();
			applet.frame.setUndecorated(true);
			applet.frame.addNotify();
			fullscreenEnabled = true;
		}
		else
		{
			applet.frame.removeNotify();
			applet.frame.setUndecorated(false);
			applet.frame.addNotify();
			fullscreenEnabled = false;
		}
	}// setFullscreen

	/**
	 * Process() should be called in draw() in order to process fullscreen and
	 * input data.
	 */
	public void process()
	{

		// Sets the window location to 0,0.
		// This is set during draw because the window may not be ready to be
		// moved when setup() is called
		if (fullscreenEnabled)
		{
			applet.frame.setLocation(0, 0);
		}
		
		if( mouseTouchEmulation )
			processMouseEvent(); // Maps Processing mouse to pointer event

		// Only get events if an event type is enabled
		if (trackerOn)
		{
			ArrayList<Event> localEventList = getEventList();

			for (int i = 0; i < localEventList.size(); i++)
			{
				Event e = localEventList.get(i);
				processEvent(e);
			}
		}
	}// getEvents

	private void processEvent(Event e)
	{
		try
		{
			processSpeechEvent(e);
			processMocapEvent(e);
			processPointerEvent(e);

			if (eventListener != null)
				eventListener.onEvent(e);
		}
		catch( NullPointerException exc )
		{
			System.err.println("Exception: " + exc.toString());
		}
	}

	private void processSpeechEvent(Event e)
	{
		if (e.serviceType != OmicronAPI.ServiceType.Speech)
			return;

		int commandID = e.getIntData(0);
		int systemID = e.getIntData(1);
	}// processSpeechEvent

	private void processMocapEvent(Event e)
	{
		if (e.serviceType != OmicronAPI.ServiceType.Mocap)
			return;

		float xPos = e.getXPos();
		float yPos = e.getYPos();
		float zPos = e.getZPos();
	}// processMocapEvent

	private void processPointerEvent(Event e)
	{
		if (e.serviceType != OmicronAPI.ServiceType.Pointer || touchListener == null)
			return;

		// Get normalized touch coordinates and expand to screen size
		float xPos = e.getXPos() * applet.width;
		float yPos = e.getYPos() * applet.height;
		float xWidth = e.getFloatData(0) * applet.width;
		float yWidth = e.getFloatData(1) * applet.height;
		int ID = e.getSourceID();
		OmicronAPI.Type gesture = e.getEventType();

		// This selects a function to call in the main sketch based on
		// the touch gesture
		switch (gesture)
		{
		case Down:
			// This calls the touchDown() function in the main sketch
			touchListener.touchDown(ID, xPos, yPos, xWidth, yWidth);
			break;
		case Move:
			// This calls the touchMove() function in the main sketch
			touchListener.touchMove(ID, xPos, yPos, xWidth, yWidth);
			break;
		case Up:
			// This calls the touchUp() function in the main sketch
			touchListener.touchUp(ID, xPos, yPos, xWidth, yWidth);
			break;
		}
	}// processPointerEvent

	private boolean mouseDown = false;
	private boolean secondMouseDown = false;
	private float secondMouseX, secondMouseY;
	private int mouseTouchHoldKeycode = 17;
	private int mouseID = -1;
	private int secondMouseID = -100;
	PVector lastMovePosition;
	
	/**
	 * Disables the creation of touch events from mouse events
	 */
	public void disableMouseTouchEmulation()
	{
		mouseTouchEmulation = false;
	}
	
	/**
	 * Enables the creation of touch events from mouse events
	 */
	public void enableMouseTouchEmulation()
	{
		mouseTouchEmulation = true;
	}
	
	private void processMouseEvent()
	{
		if (touchListener == null)
			return;
		
		float mouseOffsetX = 0;
		float mouseOffsetY = 0;
		float mouseScale = 1;
		if( scaleScreen )
		{
			mouseOffsetX = screenOffsetX;
			mouseOffsetY = screenOffsetY;
			mouseScale = screenScale;
		}
		if (applet.mousePressed)
		{
			if (!mouseDown)
			{
				touchListener.touchDown(mouseID, (applet.mouseX - mouseOffsetX) / mouseScale, (applet.mouseY - mouseOffsetY) / mouseScale, 10, 10);
				mouseDown = true;
				
				if(scaledViewMode)
					lastMovePosition = new PVector(applet.mouseX, applet.mouseY); // Set the initial position to prevent jumping
			}
			else
			{
				touchListener.touchMove(mouseID, (applet.mouseX - mouseOffsetX) / mouseScale, (applet.mouseY - mouseOffsetY) / mouseScale, 10, 10);
				
				if(scaledViewMode){
					if (lastMovePosition != null)
					{
						float newX, newY;
						newX = getScreenOffsetX() + (applet.mouseX - lastMovePosition.x) * screenScale;
						newY = getScreenOffsetY() + (applet.mouseY - lastMovePosition.y) * screenScale;
						
						setScreenTransformation(getScreenScale(), newX, newY);
					}
					lastMovePosition = new PVector(applet.mouseX, applet.mouseY);
				}
			}
		}
		else
		{
			if (mouseDown)
			{
				touchListener.touchUp(mouseID, (applet.mouseX - mouseOffsetX) / mouseScale, (applet.mouseY - mouseOffsetY) / mouseScale, 10, 10);
				mouseDown = false;
				mouseID--;
			}
		}
		
		if( mouseID < -90 )
			mouseID = -1;
		
		if (applet.keyPressed && applet.keyCode == applet.CONTROL)
		{
			if (!secondMouseDown)
			{
				touchListener.touchDown(secondMouseID, (applet.mouseX - mouseOffsetX) / mouseScale, (applet.mouseY - mouseOffsetY) / mouseScale, 10, 10);
				secondMouseDown = true;
				secondMouseX = (applet.mouseX - mouseOffsetX) / mouseScale;
				secondMouseY = (applet.mouseY - mouseOffsetY) / mouseScale;
			}
			else
			{
				touchListener.touchMove(secondMouseID, secondMouseX, secondMouseY, 10, 10);
			}
		}
		else if (!applet.keyPressed)
		{
			if (secondMouseDown)
			{
				touchListener.touchUp(secondMouseID, secondMouseX, secondMouseY, 10, 10);
				secondMouseDown = false;
				secondMouseID--;
			}
		}

		if( secondMouseID < -200 )
			secondMouseID = -100;
	}// processMouseEvent

	public void setTouchHoldKey(int keyCode)
	{

	}// setSecondTouchMouseKey

	/**
	 * Enable/disable screen scaling
	 * 
	 * @param scale
	 */
	public void enableScreenScale(boolean scale)
	{
		scaleScreen = scale;
	}// enableScreenScale

	/**
	 * Gets the screen scale factor based on calculateScreenTransformation()
	 * 
	 * @return scale factor
	 */
	public float getScreenScale()
	{
		return screenScale;
	}// getScreenScale

	/**
	 * Gets the screen x offset factor based on calculateScreenTransformation()
	 * 
	 * @return offset factor
	 */
	public float getScreenOffsetX()
	{
		return screenOffsetX;
	}// getScreenOffsetX

	/**
	 * Gets the screen y offset factor based on calculateScreenTransformation()
	 * 
	 * @return offset factor
	 */
	public float getScreenOffsetY()
	{
		return screenOffsetY;
	}// getScreenOffsetY

	/**
	 * Used to initiate the screen scale. This should be placed in draw() before
	 * the elements to be scaled are drawn.
	 */
	public void pushScreenScale()
	{
		if (!scaleScreen)
		{
			applet.pushMatrix();
			applet.translate(0, 0);
			applet.scale(1);
		}
		else
		{
			applet.pushMatrix();
			applet.translate(screenOffsetX, screenOffsetY);
			applet.scale(screenScale);
		}
	}// pushScreenScale

	/**
	 * Used to end the screen scale. This should be placed in draw() after the
	 * elements to be scaled are drawn.
	 */
	public void popScreenScale()
	{
		applet.popMatrix();
	}// popScreenScale
	
	/**
	 * Used to toggle screen scale mode which enables standard map-like interaction
	 * with the mouse
	 */
	public void setScaledViewMode(boolean value)
	{
		scaledViewMode = value;
	}// setScaleViewMode
	
	/**
	 * Used for testing only.
	 * @param newScale
	 * @param newOffsetX
	 * @param newOffsetY
	 */
	private void setScreenTransformation(float newScale, float newOffsetX, float newOffsetY)
	{
		screenScale = newScale;
		screenOffsetX = newOffsetX;
		screenOffsetY = newOffsetY;
		//applet.println("Scaling screen by " + screenScale + " Offset: " + screenOffsetX + "," + screenOffsetY );
	}// popScreenScale
	
	/**
	 * Calculates the scale and offsets to scale the Processing application from
	 * a target resolution to the current screen resolution. Must be called
	 * after size().
	 * 
	 * This should be used in a Processing sketch as follows: setup(){
	 * size(screenWidth,screenHeight); omicronAPI.calculateScreenScale(
	 * targetWidth, targetHeight ); }
	 * 
	 * draw(){ pushMatrix(); translate( omicronAPI.getScreenOffsetX(),
	 * omicronAPI.getScreenOffsetY() ); scale( omicronAPI.getScreenScale() );
	 * 
	 * // draw stuff...
	 * 
	 * popMatrix(); }
	 * 
	 * @param newWidth
	 *            target width
	 * @param newHeight
	 *            target height
	 */
	public void calculateScreenTransformation(float newWidth, float newHeight)
	{
		targetWidth = newWidth;
		targetHeight = newHeight;

		if (!scaleScreen)
		{
			screenScale = 1.0f;
			screenOffsetX = 0.0f;
			screenOffsetY = 0.0f;
			return;
		}
		if (applet.width / applet.height >= targetWidth / targetHeight)
		{
			screenScale = applet.height / targetHeight;
			screenOffsetX = (applet.width - targetWidth * screenScale) * 0.5f;
			screenOffsetY = 0;
		}
		else
		{
			screenScale = applet.width / targetWidth;
			screenOffsetX = 0;
			screenOffsetY = (applet.height - targetHeight * screenScale) * 0.5f;
		}

		// Cluster support later
		// if (cluster) {
		// screenScale = 1.0;
		// screenOffsetX = -(scaledWidth / N_NODES) * (NODE_ID / N_NODES);
		// screenOffsetY = 0;
		// }
		//applet.println("OmicronAPI: Scaling screen by " + screenScale + " Offset: " + screenOffsetX + "," + screenOffsetY );
	}// calculateScreenTransformation
	
	private void omicronMouseWheel(int delta)
	{
		if(scaledViewMode){
			float sc = 1.0f;
			if (delta < 0)
			{
				sc = 1.05f;
			}
			else if (delta > 0)
			{
				sc = 1.0f / 1.05f;
			}
			float mx = applet.mouseX * getScreenScale() - 8160/2.0f;
		    float my = applet.mouseY * getScreenScale() - 2304/2.0f;
		    
		    screenOffsetX = screenOffsetX - mx*screenScale;
		    screenOffsetY = screenOffsetY - my*screenScale;
		    screenScale = screenScale * sc;
		    screenOffsetX = screenOffsetX + mx*screenScale;
		    screenOffsetY = screenOffsetY + my*screenScale;
		}
	}
	
	// Server communication -------------------------------------------------
	private Object owner = null;

	private String serverName;
	private int port_tcp = 7000;
	private int port_udp = 3740;

	private UDP socketForData;
	private Client clientForServer;

	private boolean server = false;
	private boolean connected = false;
	private boolean dataOn = false;
	private boolean dataLoggingOn = false;

	ArrayList<Event> eventList = new ArrayList<Event>();
	private final Lock listLock = new ReentrantLock(); // Lock to ensure that

	private float startTime;

	private PrintStream out;

	// threads do not cause
	// read/write issues

	// The "receive handler" methods name. This is the method name
	// that the UDP will call when the socket receives data.
	String modHandler = "__omicron_Polling__";

	/**
	 * The TCP message to initiate data transfer
	 */
	private static final String MSG_TCP_SEND_DATA = "omicron_data_on,";
	private static final String MSG_TCP_SEND_LEGACY_DATA = "omicron_legacy_data_on,";

	/**
	 * The milliseconds required for the server to realize there are no events
	 */
	private static final int TIMEOUT_TIME = 30;

	/**
	 * Used to create a connection between an Omicron server and a Processing
	 * application. This can be called in setup() if fullscreen/scaling
	 * functions will not be used.
	 * 
	 * @param clientPort
	 *            the client port where data should be sent
	 * @param serverPort
	 *            the server port where the message to begin data transfer is
	 *            set
	 * @param trackerIP
	 *            the IP address of the Omicron server
	 */
	public void connectToTracker(int clientPort, int serverPort, String trackerIP)
	{
		this.owner = applet;

		// Register this object to the PApplet
		try
		{
			if (owner instanceof PApplet)
			{
				((PApplet) owner).registerDispose(this);
			}
		}
		catch (NoClassDefFoundError e)
		{
		}

		// Open UDP Socket
		if (clientPort != 0)
		{
			this.port_udp = clientPort;
			socketForData = new UDP(this, clientPort);
			socketForData.setReceiveHandler(modHandler);
		}

		// Open connection to server
		if (serverPort != 0 && trackerIP == null)
		{
			try
			{
				InetAddress address = InetAddress.getLocalHost();

				byte[] ip = address.getAddress();

				int i = 4;
				String ipAddress = "";
				for (byte b : ip)
				{
					ipAddress += (b & 0xFF);
					if (--i > 0)
					{
						ipAddress += ".";
					}
				}
				this.serverName = ipAddress;
				this.port_tcp = serverPort;
				this.server = true;
				// Initialize connection with server
				clientForServer = new Client((PApplet) owner, serverName, port_tcp);
				this.connected = true;
			}
			catch (UnknownHostException e)
			{
				e.printStackTrace();
			}
		}
		else if (serverPort != 0 && trackerIP != null)
		{
			this.serverName = trackerIP;
			this.port_tcp = serverPort;
			this.server = true;
			// Initialize connection with server
			clientForServer = new Client((PApplet) owner, serverName, port_tcp);
			this.connected = true;
			trackerOn = true;
		}
		else
		{
			this.server = false;
			this.connected = false;
		}

		// Initiate data transfer
		this.dataOn = false;
		initHandShake();

		// Tell port to start polling
		udpListen(true);

		startTime = (float) (System.currentTimeMillis() / 1000.0);
	}// CTOR
	
	/**
	 * Used to create a connection between an Omicron legacy server and a Processing
	 * application. This can be called in setup() if fullscreen/scaling
	 * functions will not be used.
	 * 
	 * @param clientPort
	 *            the client port where data should be sent
	 * @param serverPort
	 *            the server port where the message to begin data transfer is
	 *            set
	 * @param trackerIP
	 *            the IP address of the Omicron server
	 */
	public void connectToLegacyTracker(int clientPort, int serverPort, String trackerIP)
	{
		legacyMode = true;
		connectToTracker( clientPort, serverPort, trackerIP );
	}// CTOR
	
	
	/**
	 * Tells the server to start sending msgs to the UDP socket
	 */
	private void initHandShake()
	{
		if (server == true)
		{
			if (connected == true)
			{
				String MsgTCPInit = MSG_TCP_SEND_DATA;
				
				if( legacyMode )
					MsgTCPInit = MSG_TCP_SEND_LEGACY_DATA;
				
				MsgTCPInit = MsgTCPInit + port_udp;
				// MsgTCPInit = padMsg(MsgTCPInit);
				clientForServer.write(MsgTCPInit);
				this.dataOn = true;
			}
			else
			{
				// error("no connection to server");
				System.exit(1);
			}
		}
		else
		{
			// error("no server info");
			System.exit(1);
		}
	}

	/**
	 * Process data that is transfered into the socket. Creates a touch out of
	 * it and stores it in the dataTouchList and managedTouchList variable.
	 * 
	 * @param data
	 *            the data byte array of data
	 */
	private void processData(byte[] data)
	{
		String dGram = new String(data);
		
		Event newEvent = new Event();
		
		if( legacyMode )
		{
			try
			{
				newEvent = parseDGram(dGram);
			}
			catch( Exception e)
			{
				System.err.println("Exception on dgram '" + dGram + "':");
				e.printStackTrace();
			}
			
		}
		else
			newEvent = parseBinaryPacket(data);
		
		if (newEvent != null)
		{
			listLock.lock(); // block until condition holds
			try
			{
				addEvent(newEvent);
			}
			finally
			{
				listLock.unlock();
			}
		}
	}

	/**
	 * Parses the messages that are sent to the socket.
	 * 
	 * @param dGram
	 *            The datagram that was sent to the socket
	 * @return
	 */
	private Event parseDGram(String dGram)
	{
		String delims = "[:, ]+";
		String[] params = dGram.split(delims);

		// System.out.println( dGram );

		if (dataLoggingOn)
		{
			out.println(dGram);
		}

		OmicronAPI.ServiceType serviceType = OmicronAPI.ServiceType.values()[Integer.valueOf(params[0])];

		Event event = new Event();

		int sourceID = -1;
		float xPos, yPos, xWidth, yWidth;
		float zPos, xRot, yRot, zRot, wRot;
		event.timestamp = (float) (System.currentTimeMillis() / 1000.0 - startTime);
		event.serviceType = serviceType;

		switch (event.serviceType)
		{
		case Pointer:
			int eventType = Integer.valueOf(params[1]);
			sourceID = Integer.valueOf(params[2]);
			xPos = Float.valueOf(params[3]);
			yPos = Float.valueOf(params[4]);
			xWidth = Float.valueOf(params[5]);
			yWidth = Float.valueOf(params[6]);
			
			if( Integer.valueOf(params[1]) != -1 )
				event.eventType = OmicronAPI.Type.values()[Integer.valueOf(params[1])];
			else // If -1, then likely this is a TouchAPI/TacTile dgram
				event.eventType = OmicronAPI.Type.Null;
			
			event.sourceID = sourceID;
			event.position = new float[]
			{ xPos, yPos, 0 };

			float[] touchArray =
			{ xWidth, yWidth };
			event.dataArray = touchArray;
			event.dataArraySize = event.dataArray.length;
			break;
		case Mocap:
			sourceID = Integer.valueOf(params[1]); // TrackableID or JointID
													// (Kinect)
			xPos = Float.valueOf(params[2]);
			yPos = Float.valueOf(params[3]);
			zPos = Float.valueOf(params[4]);
			xRot = Float.valueOf(params[5]);
			yRot = Float.valueOf(params[6]);
			zRot = Float.valueOf(params[7]);
			wRot = Float.valueOf(params[8]);

			int mocapUserID = 0;
			if (params.length == 10) // Has additional userID data (Kinect)
				mocapUserID = Integer.valueOf(params[9].trim());

			float[] mocapExtraData =
			{ mocapUserID, sourceID };
			event.dataArray = mocapExtraData;
			event.dataArraySize = event.dataArray.length;

			event.sourceID = sourceID;
			event.position = new float[]
			{ xPos, yPos, zPos };
			event.orientation = new float[]
			{ xRot, yRot, zRot, wRot };
			break;
		case Kinect:
			int userID = Integer.valueOf(params[1]); // User ID
			int jointID = Integer.valueOf(params[2]);
			xPos = Float.valueOf(params[3]);
			yPos = Float.valueOf(params[4]);
			zPos = Float.valueOf(params[5]);
			xRot = Float.valueOf(params[6]);
			yRot = Float.valueOf(params[7]);
			zRot = Float.valueOf(params[8]);
			wRot = Float.valueOf(params[9]);

			event.sourceID = jointID;
			event.position = new float[]
			{ xPos, yPos, zPos };
			event.orientation = new float[]
			{ xRot, yRot, zRot, wRot };

			float[] kinectArray =
			{ userID, jointID };
			event.dataArray = kinectArray;
			event.dataArraySize = event.dataArray.length;
			break;
		case Brain:
			sourceID = Integer.valueOf(params[1]);
			int signal = Integer.valueOf(params[2]);
			int attention = Integer.valueOf(params[3]);
			int meditation = Integer.valueOf(params[4]);
			int delta = Integer.valueOf(params[5]);
			int theta = Integer.valueOf(params[6]);
			int lowAlpha = Integer.valueOf(params[7]);
			int highAlpha = Integer.valueOf(params[8]);
			int lowBeta = Integer.valueOf(params[9]);
			int highBeta = Integer.valueOf(params[10]);
			int lowGamma = Integer.valueOf(params[11]);
			int highGamma = Integer.valueOf(params[12]);
			int blink = Integer.valueOf(params[13]);

			float[] floatArray =
			{ signal, attention, meditation, delta, theta, lowAlpha, highAlpha, lowBeta, highBeta, lowGamma, highGamma, blink };
			event.sourceID = sourceID;
			event.dataArray = floatArray;
			event.dataArraySize = event.dataArray.length;
			break;
		case Speech:
			sourceID = Integer.valueOf(params[1]);

			int commandID = -1;
			int systemID = -1;
			float confidence = -1;
			float sourceAngle = -1;
			float angleConfidence = -1;

			if (sourceID == -1)
			{
				event.extraDataType = OmicronAPI.ExtraDataType.ExtraDataString;
				event.dataArraySize = 1;
				event.stringArray = new String[] { params[2] };
				confidence = Float.valueOf(params[3]);
				sourceAngle = Float.valueOf(params[4]);
				angleConfidence = Float.valueOf(params[5]);
			}
			else
			{
				commandID = Integer.valueOf(params[2]);
				systemID = Integer.valueOf(params[3]);
				sourceAngle = Float.valueOf(params[4]);
			}

			float[] speechArray =
			{ commandID, systemID, confidence, sourceAngle, angleConfidence };
			event.sourceID = sourceID;
			event.dataArray = speechArray;
			event.dataArraySize = event.dataArray.length;
			break;
		case Wand:
			// for( int i = 0; i < params.length; i++ )
			// System.out.println( "["+i+"] " + params[i] );

			event.eventType = OmicronAPI.Type.values()[Integer.valueOf(params[1])]; // EventID:
																					// update/down/up
			event.sourceID = Integer.valueOf(params[2]); // SourceID: Wand ID
			event.flags = Integer.valueOf(params[3]); // Flag: Button ID on
														// down/up events
			/*
			 * xPos = Float.valueOf(params[4]); yPos = Float.valueOf(params[5]);
			 * zPos = Float.valueOf(params[6]); xRot = Float.valueOf(params[7]);
			 * yRot = Float.valueOf(params[8]); zRot = Float.valueOf(params[9]);
			 * wRot = Float.valueOf(params[10]);
			 */
			float wandAnalog_1 = Float.valueOf(params[4]); // Left analog
															// (-left, +right)
			float wandAnalog_2 = Float.valueOf(params[5]); // Left analog (-up,
															// +down)
			float wandAnalog_3 = Float.valueOf(params[6]); // Right analog
															// (-left, +right)
			float wandAnalog_4 = Float.valueOf(params[7]); // Right analog (-up,
															// +down)
			float wandAnalog_5 = Float.valueOf(params[8]); // Trigger 2 (+left,
															// -right)
			float[] wandArray =
			{ wandAnalog_1, wandAnalog_2, wandAnalog_3, wandAnalog_4, wandAnalog_5 };
			event.dataArray = wandArray;
			event.dataArraySize = event.dataArray.length;

			break;
		default:
			break;
		}

		return event;
	}
	
	private Event parseBinaryPacket( byte[] data )
	{
		Event evt = new Event();
		int length = data.length;

		InputStream input = new ByteArrayInputStream(data);

		ByteBuffer byteBuffer = ByteBuffer.allocate(length);

		byteBuffer.put(data); // Load byte data into buffer
		byteBuffer.position(0); // Start at beginning of buffer
		
		// Bytes are in reverse order
		evt.timestamp = Integer.reverseBytes(byteBuffer.getInt());
		evt.sourceID = Integer.reverseBytes(byteBuffer.getInt());
		evt.serviceID = Integer.reverseBytes(byteBuffer.getInt());
		
		byteBuffer.position(12); // Fix offset due to C++/Java conflict
		try
		{
			evt.serviceType = OmicronAPI.ServiceType.values()[Integer.valueOf( Integer.reverseBytes(byteBuffer.getInt()) ) ];
		}
		catch(ArrayIndexOutOfBoundsException e)
		{
			System.err.println("Exception: " + e.toString());
			System.err.println(" If this index is really large, the tracker you are connecting to may only support legacy Omicron connections.");
			System.err.println(" You may have to use connectToLegacyTracker() instead of connectToTracker()");
			return evt;
		}
		byteBuffer.position(16); // Fix offset due to C++/Java conflict
		evt.eventType = OmicronAPI.Type.values()[Integer.valueOf( Integer.reverseBytes(byteBuffer.getInt()) ) ];
		evt.flags = Integer.reverseBytes(byteBuffer.getInt());
		
		// Again, reverse bytes and then convert to float correcting precision
		float x = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		float y = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		float z = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		float rw = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		float rx = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		float ry = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		float rz = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
		
		evt.position = new float[] { x, y, z };
		evt.orientation = new float[] { rx, ry, rz, rw };
		
		evt.extraDataType = OmicronAPI.ExtraDataType.values()[Integer.valueOf( Integer.reverseBytes(byteBuffer.getInt()) )];
		evt.extraDataItems = Integer.valueOf( Integer.reverseBytes(byteBuffer.getInt()) );
		
		evt.dataArray = new float[evt.extraDataItems];
		evt.dataArraySize = evt.extraDataItems;
		
		byteBuffer.position(64); // Fix offset due to C++/Java conflict
		if( evt.extraDataType == OmicronAPI.ExtraDataType.ExtraDataFloatArray )
		{
			for( int i = 0; i < evt.extraDataItems; i++ )
			{
				evt.dataArray[i] = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
			}
			//applet.println(evt.dataArray);
		}
		else if( evt.extraDataType == OmicronAPI.ExtraDataType.ExtraDataIntArray )
		{
			for( int i = 0; i < evt.extraDataItems; i++ )
			{
				evt.dataArray[i] = Integer.reverseBytes(byteBuffer.getInt());
			}
		}
		else if( evt.extraDataType == OmicronAPI.ExtraDataType.ExtraDataVector3Array )
		{
			evt.vectorArray = new PVector[evt.extraDataItems];
			
			for( int i = 0; i < evt.extraDataItems; i++ )
			{
				PVector vec = new PVector();
				vec.x = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
				vec.y = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
				vec.z = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
				
				evt.vectorArray[i] = vec;
			}
		}
		else if( evt.extraDataType == OmicronAPI.ExtraDataType.ExtraDataString )
		{
			evt.vectorArray = new PVector[evt.extraDataItems];
			
			for( int i = 0; i < evt.extraDataItems; i++ )
			{
				PVector vec = new PVector();
				vec.x = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
				vec.y = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
				vec.z = Float.intBitsToFloat( Integer.reverseBytes(byteBuffer.getInt()) );
				
				evt.vectorArray[i] = vec;
			}
		}
		return evt;
	}
	
	/**
	 * Adds a new event to the eventList.
	 * 
	 * @param newEvent
	 *            An Event object to be added to eventList.
	 */
	private void addEvent(Event newEvent)
	{

		listLock.lock(); // block until condition holds
		try
		{
			eventList.add(newEvent);
		}
		finally
		{
			listLock.unlock();
		}
	}

	/**
	 * Returns a copy of the eventList.
	 * 
	 * @return ArrayList <Event>
	 */
	public ArrayList<Event> getEventList()
	{
		ArrayList<Event> temp;

		listLock.lock(); // block until condition holds
		try
		{
			temp = new ArrayList<Event>(eventList);
		}
		finally
		{
			listLock.unlock();
		}
		eventList.clear();
		return temp;
	}

	/**
	 * Goes through the event list and removes old events
	 */
	private ArrayList<Event> updateList(ArrayList<Event> eventList)
	{
		ArrayList<Event> temp = new ArrayList<Event>();

		for (int i = 0; i < eventList.size(); i++)
		{
			float eventTimeout = 0.25f; // seconds

			Event e = eventList.get(i);
			float timestamp = e.timestamp;
			float currentTime = System.currentTimeMillis() / 1000.0f;
			if (currentTime - timestamp > eventTimeout)
			{
				// Event is too old
			}
			else
			{
				temp.add(e);
			}

			// if( e.serviceType == POINTER ){

			// }
		}
		return temp;
	}

	public void enableDataLogging(boolean bool)
	{
		if (!dataLoggingOn)
		{
			try
			{
				out = new PrintStream(new FileOutputStream("dataLog.txt"));
				dataLoggingOn = true;
			}
			catch (FileNotFoundException e)
			{
				e.printStackTrace();
			}
		}
	}

	/**
	 * Tell UDP socket to Start/stop waiting constantly for incoming data.
	 * 
	 * @param on
	 *            the required listening status.
	 * 
	 */
	private void udpListen(boolean on)
	{
		socketForData.listen(TIMEOUT_TIME);
	}

	/**
	 * Close the connections that are made to touch server and the Processing
	 * Application. This method is automatically called by Processing when the
	 * PApplet shuts down.<br>
	 * You <i>DO NOT</i> need to call this method.
	 */
	public void dispose()
	{
		udpClose();
		tcpClose();

		if (dataLoggingOn)
			out.close();
	}

	/**
	 * Closes the server connection.
	 */
	private void tcpClose()
	{
		if (connected())
		{
			int port = tcp_port();
			String ip = serverAddy();

			// close the connection
			clientForServer.stop();
			clientForServer = null;
			// log("close socket < port:" + port + ", address:" + ip + " >\n");
		}
		else
		{
			// error("Tried to close a connection to a server that was not there!");
		}
	}

	/**
	 * Close the socket and all associate resources.
	 */
	private void udpClose()
	{
		socketForData.close();
	}

	/**
	 * Return true if socket is closed
	 * 
	 * @return boolean
	 */
	private boolean socketIsClosed()
	{
		return socketForData.isClosed();
	}

	/**
	 * Return true if connected to server
	 * 
	 * @return boolean
	 */
	private boolean connected()
	{
		return connected;
	}

	/**
	 * Return the socket on the server where msgs are passed, or -1 if not
	 * connected to a server.
	 * 
	 * @return int
	 */
	protected int tcp_port()
	{
		if (server == false)
			return -1;
		return port_tcp;
	}

	/**
	 * Return the actual socket's local port, or -1 if the socket is closed.
	 * 
	 * @return int
	 */
	protected int udp_port()
	{
		return socketForData.port();
	}

	/**
	 * Return the actual socket's local address
	 * 
	 * @return String
	 */
	protected String localAddy()
	{
		if (socketIsClosed())
			return null;
		return socketForData.address();
	}

	/**
	 * Return the server's address, or <code>null</code> if not connected to a
	 * server
	 * 
	 * @return String
	 */
	protected String serverAddy()
	{
		if (server == false)
			return null;
		return serverName;
	}

	/**
	 * In order to perform parsing of the incoming data, this method is needed.<br>
	 * You <i>DO NOT</i> need to implement/use this method.
	 * 
	 * @param data
	 *            the data that is transmitted
	 * @param IP
	 *            the IP of the data's origin
	 * @param port
	 *            the port the data was sent from
	 * @see OmegaLibClient#processData(byte[] data)
	 */
	// This implementation of the handler is needed in your code. This method
	// will
	// be
	// automatically called by the UDP object each time he receive a non-null
	// message. By default, this method have just one argument (the received
	// message
	// as byte[ ] array), but in addition, two arguments (representing in order
	// the sender IP address and his port can be set like below.<br>
	// Current implementation is: once data is passed in, it hands it over to
	// processData() for parsing
	// void __omicron_Polling__( byte[] data ) { // <-- default handler
	public void __omicron_Polling__(byte[] data, String ip, int port)
	{ // <--
		// extended
		// handler
		processData(data);
	}
}// class
