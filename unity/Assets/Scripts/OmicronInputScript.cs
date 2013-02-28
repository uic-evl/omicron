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
 
/*
 * OmicronInputScript handles all network connentions to the input server and sends events out to objects tagged as 'OmicronListener'.
 * Currently supported input servers:
 * 		- Omicron oinputserver
 *		- OmicronInputConnector
 *
 * Scripts that handle events: These scripts should be attached to the receiving game object. These scripts handle messages with the input data.
 * You should write your own scripts that inherit from these.
 *		- OmicronKinectScript.cs
 *		- OmicronMindwaveScript.cs
 *
 * To run make sure ConnectToServer is checked and the correct Server and Msg Ports are set.
 */
using UnityEngine;
using System.Collections;

using omicronConnector;
using omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public class TouchPoint{

	Vector3 position;
	int ID;
	EventBase.Type gesture;
	Ray touchRay = new Ray();
	RaycastHit touchHit;
	long timeStamp;
	GameObject objectTouched;
	
	public TouchPoint(Vector2 pos, int id){
		position = pos;
		ID = id;
		touchRay = Camera.main.ScreenPointToRay(position);
		gesture = EventBase.Type.Null;
		timeStamp = (long)Time.time;
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
	
	public EventBase.Type GetGesture(){
		return gesture;
	}
	
	public RaycastHit GetRaycastHit(){
		 return touchHit;
	}
	
	public GameObject GetObjectTouched(){
		 return objectTouched;
	}
	
	public void SetGesture(EventBase.Type value){
		 gesture = value;
	}
	
	public void SetRaycastHit(RaycastHit value){
		 touchHit = value;
	}
	
	public void SetObjectTouched(GameObject value){
		 objectTouched = value;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class EventListener : IOmicronConnectorClientListener
{
	OmicronInputScript parent;
	
	public EventListener( OmicronInputScript p )
	{
		parent = p;
	}
	
	public override void onEvent(EventData e)
	{
		parent.AddEvent(e);
	}// onEvent
	
}// EventListener

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OmicronInputScript : MonoBehaviour
{
	EventListener omicronListener;
	OmicronConnectorClient omicronManager;
	public bool connectToServer = false;
	public string serverIP = "localhost";
	public int serverMsgPort = 27000;
	public int dataPort = 7013;
	
		
	// Use mouse clicks to emulate touches
	public bool mouseTouchEmulation = true;
	
	// List storing events since we have multiple threads
	private ArrayList eventList;
	
	// Initializations
	public void Start()
	{
		omicronListener = new EventListener(this);
		omicronManager = new OmicronConnectorClient(omicronListener);
		
		if( connectToServer )
		{
			omicronManager.Connect( serverIP, serverMsgPort, dataPort );
		}
		
		eventList = new ArrayList();
	}// start
	
	public void AddEvent( EventData e )
	{
		lock(eventList.SyncRoot)
		{
			eventList.Add(e);
		}
	}
	
	public void Update()
	{
		if( mouseTouchEmulation )
		{
			Vector2 position = new Vector3( Input.mousePosition.x, Input.mousePosition.y );
					
			// Ray extending from main camera into screen from touch point
			Ray touchRay = Camera.main.ScreenPointToRay(position);
			Debug.DrawRay(touchRay.origin, touchRay.direction * 10, Color.white);
					
			TouchPoint touch = new TouchPoint(position, -1);
			
			if( Input.GetMouseButtonDown(0) )
				touch.SetGesture( EventBase.Type.Down );
			else if( Input.GetMouseButtonUp(0) )
				touch.SetGesture( EventBase.Type.Up );
			else if( Input.GetMouseButton(0) )
				touch.SetGesture( EventBase.Type.Move );
			
			GameObject[] touchObjects = GameObject.FindGameObjectsWithTag("OmicronListener");
			foreach (GameObject touchObj in touchObjects) {
				touchObj.BroadcastMessage("OnTouch",touch,SendMessageOptions.DontRequireReceiver);
			}
		}
		
		lock(eventList.SyncRoot)
		{
			foreach( EventData e in eventList )
			{
				if( (EventBase.ServiceType)e.serviceType == EventBase.ServiceType.ServiceTypePointer )
				{
					// 2D position of the touch, flipping y-coordinates
					Vector2 position = new Vector3( e.posx * Screen.width, Screen.height - e.posy * Screen.height );
					
					// Ray extending from main camera into screen from touch point
					Ray touchRay = Camera.main.ScreenPointToRay(position);
					Debug.DrawRay(touchRay.origin, touchRay.direction * 10, Color.white);
					
					TouchPoint touch = new TouchPoint(position, (int)e.sourceId);
					touch.SetGesture( (EventBase.Type)e.type ); 
					
					GameObject[] touchObjects = GameObject.FindGameObjectsWithTag("OmicronListener");
					foreach (GameObject touchObj in touchObjects) {
						touchObj.BroadcastMessage("OnTouch",touch,SendMessageOptions.DontRequireReceiver);
					}
				}
				
				else
				{
					GameObject[] omicronObjects = GameObject.FindGameObjectsWithTag("OmicronListener");
					foreach (GameObject obj in omicronObjects) {
						obj.BroadcastMessage("OnEvent",e,SendMessageOptions.DontRequireReceiver);
					}
				}
			}
			
			// Clear the list (TODO: probably should set the Processed flag instead and cleanup elsewhere)
			eventList.Clear();
		}
	}
	
	void OnApplicationQuit()
    {
		if( connectToServer ){
			omicronManager.Dispose();
			
			Debug.Log("InputService: Disconnected");
		}
    }
}// class