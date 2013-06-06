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

using UnityEngine;
using System.Collections;
using omicron;

public abstract class OmicronTouchScript : MonoBehaviour {
	public bool isTouched = false; // Valid for single touch - fix later
	public Hashtable touchlist;
	public int touchlistSize = 0;
	
	// Use this for initialization
	void Start () {
		touchlist = new Hashtable();
		if( gameObject.tag != "OmicronListener" ){
			gameObject.tag = "OmicronListener";
		}
		StartDerived();
	}

	// Update is called once per frame
	void Update () {
		touchlistSize = touchlist.Count;
		UpdateDerived();
	}
	
	public void OnTouch(TouchPoint touch){
		int fingerID = touch.GetID();
		EventBase.Type gesture = touch.GetGesture();
		Ray touchRay = touch.GetRay();
		
		// Check if the ray from the main camera touch intersects with a collider or rigidbody
		// and determine if the ray intersects this object
		RaycastHit rayCast;
		if( Physics.Raycast(touchRay.origin, touchRay.direction, out rayCast) && rayCast.transform.gameObject == gameObject ){
			 isTouched = true;
		} else {
			isTouched = false;
		}
		
		switch(gesture){
			case(EventBase.Type.Down):
				// Add finger to list
				if( isTouched ){
					if( touchlist.Contains(fingerID) )
						Debug.Log("Warning: Touch down detected for existing touch - should not happen");
					touchlist[fingerID] = touch;
					OnTouchDown(touch);
				}
				break;
			case(EventBase.Type.Move):
				if( isTouched ){
					touchlist[fingerID] = touch;
					OnTouchMove(touch);
				}
				else if( touchlist.Contains(fingerID) )
				{
					touchlist.Remove(fingerID);
					OnTouchUp(touch);
				}
				break;
			case(EventBase.Type.Up):
				if( touchlist.Contains(fingerID) ){
					touchlist.Remove(fingerID);
					OnTouchUp(touch);
				}
				isTouched = false;
				break;
		}
	}
	
	// These functions are used by derived classes 
	public abstract void StartDerived();
	public abstract void UpdateDerived();
	public abstract void OnTouchDown(TouchPoint t);
	public abstract void OnTouchMove(TouchPoint t);
	public abstract void OnTouchUp(TouchPoint t);
}

