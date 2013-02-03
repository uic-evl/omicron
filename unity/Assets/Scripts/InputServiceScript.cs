/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2011							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto								anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/
 
/*
 * InputServiceScript handles all network connentions to the input server and sends events out to objects tagged as listeners.
 * Currently supported input servers:
 * 		- OmegaLib (touch and controllers)
 *		- TouchAPI connector
 *
 * Scripts that handle events: These scripts should be attached to the receiving game object. These scripts handle messages with the input data.
 * You should write your own scripts that inherit from these.
 *		- MocapScript.cs (GameObjects using these are tagged as 'OmegaListener')
 *		- TouchScript.cs (GameObjects using these are tagged as 'TouchListener')
 *		- OmegaControllerScript.cs (GameObjects using these are tagged as 'OmegaListener')
 *
 * To run make sure Enable Input Service is checked and the correct Server and Msg Ports are set.
 */
using UnityEngine;
using System.Collections;

using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public class Touches{
		
	public const int GESTURE_DOWN = 0;
	public const int GESTURE_MOVE = 1;
	public const int GESTURE_UP = 2;
	
	Vector3 position;
	int ID;
	int gesture;
	Ray touchRay = new Ray();
	RaycastHit touchHit;
	long timeStamp;
	GameObject objectTouched;
	
	public Touches(Vector2 pos, int id){
		position = pos;
		ID = id;
		touchRay = Camera.main.ScreenPointToRay(position);
		DateTime baseTime = new DateTime(1970, 1, 1, 0, 0, 0);
		timeStamp = (DateTime.UtcNow - baseTime).Ticks / 10000;
		gesture = -1;
	}
	
	public Vector3 GetPosition(){
		return position;
	}
	
	public Ray GetRay(){
		return touchRay;
	}
	
	public int GetID(){
		return ID;
	}
	
	public long GetTimeStamp(){
		return timeStamp;
	}
	
	public int GetGesture(){
		return gesture;
	}
	
	public RaycastHit GetRaycastHit(){
		 return touchHit;
	}
	
	public GameObject GetObjectTouched(){
		 return objectTouched;
	}
	
	public void SetGesture(int value){
		 gesture = value;
	}
	
	public void SetRaycastHit(RaycastHit value){
		 touchHit = value;
	}
	
	public void SetObjectTouched(GameObject value){
		 objectTouched = value;
	}
	
	public override String ToString(){
		if( gesture == GESTURE_DOWN )
			return " Down Touch " + ID + " at " + position.x + "," + position.y;
		else if( gesture == GESTURE_MOVE )
			return " Move Touch " + ID + " at " + position.x + "," + position.y;
		else if( gesture == GESTURE_UP )
			return " Up Touch " + ID + " at " + position.x + "," + position.y;
		else
			return " Touch " + ID + " at " + position.x + "," + position.y;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public class InputServiceScript : MonoBehaviour {
	enum ServiceType { Pointer, Mocap, Keyboard, Controller, UI, Generic, NeuroSky }; 
	
	// TCP Connection
	TcpClient client;
	NetworkStream streamToServer;
		
	// UDP Connection
    private static UdpClient udpClient;
    private static Thread listenerThread;
	
	public String InputServer = "euclid.evl.uic.edu";
    public Int32 dataPort = 7000;
    public Int32 msgPort = 27000;
	
	public bool EnableInputService = true;
	
	public bool NEC_Wall_LEFT = false;
	public bool NEC_Wall_RIGHT = false;
    private static int screenWidth;
    private static int screenHeight;
	
	static Touches MouseTouch;
	private static ArrayList dgrams;
	
	private Hashtable touchList;
	
	public int touchTimeOut = 250; // Milliseconds before an idle touch is auto-removed and an up event is sent.
	
	public bool ping;
	
	// Use this for initialization
	void Start () {
        screenWidth = Screen.width;
        screenHeight = Screen.height;
        Debug.Log("InputService: Screen resolution " + screenWidth + "," + screenHeight);

		touchList = new Hashtable();
		dgrams = new ArrayList();
		
		if( EnableInputService ){
			Debug.Log("InputService: Initializing... ");
			try
			{
				// Create a TcpClient.
				client = new TcpClient(InputServer, msgPort);

				// Translate the passed message into ASCII and store it as a Byte array.
				String message = "data_on,"+dataPort;
				Byte[] data = System.Text.Encoding.ASCII.GetBytes(message);

				streamToServer = client.GetStream();

				// Create the UDP socket data will be received on
				udpClient = new UdpClient(dataPort);

				// Send the handshake message to the server. 
				streamToServer.Write(data, 0, data.Length);

				//Console.WriteLine("Handshake Sent: {0}", message);
				Debug.Log("InputService: Connected to "+InputServer);
				
				// Creates a separate thread to listen for incoming data
				listenerThread = new Thread(Listen);
				listenerThread.Start();				
			}
			catch (ArgumentNullException e)
			{
				Debug.Log("ArgumentNullException: " + e);
			}
			catch (SocketException e)
			{
				Debug.Log("SocketException: " + e);
			}
		}
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
			lock(dgrams.SyncRoot){
				dgrams.Add( Encoding.ASCII.GetString(receiveBytes) );
			}
        }
    }


    void ParseDGram(string dGram)
    {
        char[] delimiterChars = { ' ', ',', ':', '\t' };

        //dGram = "1294867779788:d:42,0.6105469,0.186875,0.5 "; // TacTile sample
        //dGram = "1294867779788:q:42,0.0,0.0,0.009375,0.02125,1,0.5 "; // PQLabs example
        //Debug.Log("InputService: Receiving '"+dGram+"'");
		
		bool omegalib = false;
		if( dGram.IndexOf(':') == 1 )
		{
			omegalib = true;
		}
		
        string[] words = dGram.Split(delimiterChars);
        System.Console.WriteLine("{0} words in text:", words.Length);

        //foreach (string s in words)
        //{
        //    System.Console.WriteLine(s);
        //}

        // Check dgram flag
        if (words.Length >= 2 && (
            // 'timestamp:flag:fingerID,x,y,intensity '
            words[1].Contains("d") || // TacTile/OmegaTracker data
            words[1].Contains("g") || // TacTile/OmegaTracker gesture
            words[1].Contains("p") || // TacTile/OmegaTracker pan
            words[1].Contains("z")    // TacTile/OmegaTracker zoom
        ))
        {
            System.Console.WriteLine("Dgram uses TacTile format");
            // 0 = timestamp
            // 1 = flag
            // 2 = finger ID
            // 3 = x
            // 4 = y
            // 5 = intensity
            int ID = int.Parse(words[2]);
			
			float xPos;
            float yPos;
			
			if( NEC_Wall_LEFT ){
				xPos = float.Parse(words[3]) * 8160;
				yPos = float.Parse(words[4]) * 2304;
			} else if( NEC_Wall_RIGHT ){
				xPos = float.Parse(words[3]) * 8160 - 4080;
				yPos = float.Parse(words[4]) * 2304;
			} else {
				xPos = float.Parse(words[3]) * screenWidth;
				yPos = float.Parse(words[4]) * screenHeight;
			}
			
			// Non-NEC wall 
			//float xPos = float.Parse(words[3]) * screenWidth;
            //float yPos = float.Parse(words[4]) * screenHeight;
            //Debug.Log("Touch ID " + ID + " at " + xPos + ", " + yPos);
			ProcessTouchEvent( new Touches( new Vector2(xPos, yPos), ID ) );
        }
        else if (words.Length >= 2 && (
            // 'timestamp:flag:fingerID,x,y,xW,yW,intensity '
            words[1].Contains("q") || // PQLabs/TouchAPI Connector data
            words[1].Contains("l")    // PQLabs/TouchAPI Connector gesture
        ))
        {
            System.Console.WriteLine("Dgram uses PQLabs format");
            // 0 = timestamp
            // 1 = flag
            // 2 = finger ID
            // 3 = x
            // 4 = y
            // 5 = x width
            // 6 = y width
            // 7 = gesture
            // 8 = intensity
            int ID = int.Parse(words[2]);
            float xPos = float.Parse(words[3]) * screenWidth;
            float yPos = float.Parse(words[4]) * screenHeight;
            //float xW = float.Parse(words[5]) * screenWidth;
            //float yW = float.Parse(words[6]) * screenHeight;
            //Debug.Log("Touch ID " + ID + " at " + xPos + ", " + yPos + " Size: " + xW + ", " + yW);
            //Ray ray = camera.ScreenPointToRay(new Vector3(0, 0, 0));
			//Ray ray2 = camera.ScreenPointToRay(new Vector3(screenWidth, 0, 0));
			//Ray ray3 = camera.ScreenPointToRay(new Vector3(0, screenHeight, 0));
			//Ray ray4 = camera.ScreenPointToRay(new Vector3(screenWidth, screenHeight, 0));
			//Debug.DrawRay(ray.origin, ray.direction * 10, Color.yellow);
			//Debug.DrawRay(ray2.origin, ray2.direction * 10, Color.red);
			//Debug.DrawRay(ray3.origin, ray3.direction * 10, Color.blue);
			//Debug.DrawRay(ray4.origin, ray4.direction * 10, Color.green);
		
			ProcessTouchEvent( new Touches( new Vector2(xPos, yPos), ID ) );
        }
        else if( omegalib )
        {
            System.Console.WriteLine("Dgram uses OmegaLib format");
			int inputType = int.Parse(words[0]);
			// Words[] parameters
            // 0 = Input type ( See ServiceType enum above )
			// 1 - n = type dependent
			
			if( inputType == (int)ServiceType.Pointer ){ // Touch
				// 1 = gesture
				// 2 = ID
				// 3 = x
				// 4 = y
				// 5 = x width (if ID > -1 )
				// 6 = y width (if ID > -1 )
				int ID = int.Parse(words[2]);
				float xPos = float.Parse(words[3]) * screenWidth;
				float yPos = screenHeight - float.Parse(words[4]) * screenHeight;
				//float xW = float.Parse(words[5]) * screenWidth;
				//float yW = float.Parse(words[6]) * screenHeight;
				ProcessTouchEvent( new Touches( new Vector2(xPos, yPos), ID ) );
			} else if( inputType == (int)ServiceType.Controller ){ // Controller
				//Debug.Log("Controller: " + dGram);
				
				GameObject[] eventObjects = GameObject.FindGameObjectsWithTag("OmegaListener");
				foreach (GameObject eventObj in eventObjects) {
					eventObj.BroadcastMessage("UpdateControllerData",words,SendMessageOptions.DontRequireReceiver);
				}
			} else if( inputType == (int)ServiceType.Mocap ){
				//Debug.Log("Mocap: " + dGram);

				GameObject[] touchObjects = GameObject.FindGameObjectsWithTag("OmegaListener");
				foreach (GameObject touchObj in touchObjects) {
					touchObj.BroadcastMessage("UpdateMocapPosition",words,SendMessageOptions.DontRequireReceiver);
				}
				//GameObject.FindGameObjectWithTag("OmegaListener").GetComponent<MocapScript>().UpdateMocapPosition(ID,xPos,yPos,zPos);
				//ProcessMocapEvent(xPos,yPos,zPos);
			} else if( inputType == (int)ServiceType.NeuroSky ){
				//Debug.Log("NeuroSky: " + dGram);

				GameObject[] touchObjects = GameObject.FindGameObjectsWithTag("OmegaListener");
				foreach (GameObject touchObj in touchObjects) {
					touchObj.BroadcastMessage("UpdateNeuroSky",words,SendMessageOptions.DontRequireReceiver);
				}
			}
        }
		else
        {
            Debug.Log("Unknown Dgram format: " + dGram);
        }
    }

    void OnApplicationQuit()
    {
		if( EnableInputService ){
			// Close UDP
			udpClient.Close();
			listenerThread.Abort();
			
			// Close TCP connection.
			streamToServer.Close();
			client.Close();
			
			Debug.Log("InputService: Disconnected");
		}
    }
	
	void ProcessControllerEvent( int ID,  float[] analogLeft, float[] analogRight, int[] buttons, float slider, float roll, float pitch ){
		/*
		GameObject[] touchObjects = GameObject.FindGameObjectsWithTag("TouchListener");
		foreach (GameObject touchObj in touchObjects) {
			touchObj.BroadcastMessage("OnTouch",touch,SendMessageOptions.DontRequireReceiver);
		}
		if( ID == 0 ){
			GameObject.FindGameObjectWithTag("Player").GetComponent<OmegaControllerScript>().MoveVector(analogLeft);
			GameObject.FindGameObjectWithTag("Player").GetComponent<OmegaControllerScript>().RotateVector(analogRight);
			GameObject.FindGameObjectWithTag("Player").GetComponent<OmegaControllerScript>().TiltVector(roll, pitch);
			GameObject.FindGameObjectWithTag("Player").GetComponent<OmegaControllerScript>().Buttons(buttons);
		} else {
			//GameObject.FindGameObjectWithTag("Player2").GetComponent<OmegaControllerScript>().MoveVector(analogLeft);
			//GameObject.FindGameObjectWithTag("Player2").GetComponent<OmegaControllerScript>().RotateVector(analogRight);
			//GameObject.FindGameObjectWithTag("Player2").GetComponent<OmegaControllerScript>().TiltVector(roll, pitch);
		}
		*/
	}
	
	void ProcessTouchEvent( Touches touch ){
		int ID = touch.GetID();
		
		if( touchList.ContainsKey( ID ) ){
			touch.SetGesture(Touches.GESTURE_MOVE);
			touchList.Remove( ID );
		} else {
			touch.SetGesture(Touches.GESTURE_DOWN);
		}
		
		touchList.Add( ID, touch );
		SendTouchEvent( touch );
	}

	void SendTouchEvent( Touches touch ){
		// Draw touch ray
		Ray ray = touch.GetRay();
		Debug.DrawRay(ray.origin, ray.direction * 10, Color.white);
		
		GameObject[] touchObjects = GameObject.FindGameObjectsWithTag("OmegaListener");
		foreach (GameObject touchObj in touchObjects) {
			touchObj.BroadcastMessage("OnTouch",touch,SendMessageOptions.DontRequireReceiver);
		}
		
		// Send a RayCast from source along ray looking for colliders
		//RaycastHit hit;
		//if( Physics.Raycast(ray.origin, ray.direction, out hit) ){ 
			
			// If a visible collider is hit, tell object it has been hit.
			//Debug.Log(hit.collider.gameObject.name);
			//hit.collider.gameObject.BroadcastMessage("OnTouch",touch);
		//}
	}
	
	// Update is called once per frame
	void Update () {
		lock(dgrams.SyncRoot)
		{
			foreach (String dgram in dgrams)
			{
				ParseDGram(dgram);
			}
			dgrams.Clear();
		}
		
		if( ping ){
			// Translate the passed message into ASCII and store it as a Byte array.
			client = new TcpClient(InputServer, msgPort);
			String message = "client_in,101,one ping only";
			Byte[] data = System.Text.Encoding.ASCII.GetBytes(message);
			
			streamToServer = client.GetStream();
			
			// Send the message to the server. 
			streamToServer.Write(data, 0, data.Length);
			
			Debug.Log("Ping sent");
			ping = false;
		}
		
		Hashtable tempList = new Hashtable(touchList);
		foreach( DictionaryEntry elem in tempList )
        {
            //Console.WriteLine("Key = {0}, Value = {1}", elem.Key, elem.Value);
			Touches t = (Touches)elem.Value;
			long timeStamp = t.GetTimeStamp();
			
			DateTime baseTime = new DateTime(1970, 1, 1, 0, 0, 0);
			long curTime = (DateTime.UtcNow - baseTime).Ticks / 10000;
			if( timeStamp + touchTimeOut < curTime ){
				t.SetGesture(Touches.GESTURE_UP);
				SendTouchEvent( t );
				touchList.Remove( t.GetID() );
			}
        }
		
		// Emulates mouse input as touch events
		if( Input.GetMouseButtonDown(0) )
		{
			MouseTouch = new Touches( Input.mousePosition, -1 );
			MouseTouch.SetGesture(Touches.GESTURE_DOWN);
			SendTouchEvent(MouseTouch);
		}
		else if( Input.GetMouseButtonUp(0) )
		{
			MouseTouch = new Touches( Input.mousePosition, -1 );
			MouseTouch.SetGesture(Touches.GESTURE_UP);
			SendTouchEvent(MouseTouch);
		}
		else if( Input.GetMouseButton(0) )
		{
			MouseTouch = new Touches( Input.mousePosition, -1 );
			MouseTouch.SetGesture(Touches.GESTURE_MOVE);
			SendTouchEvent(MouseTouch);
		}
		else
		{
			MouseTouch = null;
		}
			
	}
}
