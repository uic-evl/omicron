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
using omicronConnector;

public class OmicronMindwaveScript : MonoBehaviour {
	public int connectionID;
	
	public float signalStrength;
	public float attention;
	public float meditation;
	public float delta;
	public float theta;
	public float lowAlpha;
	public float highAlpha;
	public float lowBeta;
	public float highBeta;
	public float lowGamma;
	public float highGamma;
	public float blinkStrength;
	
	public float prev_attention;
	public float prev_meditation;
	
	public float lastUpdateTime;
	
	// Use this for initialization
	void Start () {
		if( gameObject.tag != "OmicronListener" ){
			gameObject.tag = "OmicronListener";
		}
	}
	
	// Update is called once per frame
	void Update () {
		float movementTime = 0.5f;
		
		// If the block has the right name, move from 0.0 to 3.5 based on that value
		if( gameObject.name == "Attention Block" )
		{
			float interpolationTimer = (Time.time - lastUpdateTime) / movementTime;
			
			// Since we only get new data once a second, interpolate the box position
			Vector3 newPosition = Vector3.Lerp( new Vector3( 2.2f, (prev_attention / 100.0f) * 3.5f, 0), new Vector3( 2.2f, (attention / 100.0f) * 3.5f, 0), interpolationTimer );
			transform.localPosition = newPosition;
			
			if( interpolationTimer > 1 )
			{
				prev_attention = attention;
			}
		}
		if( gameObject.name == "Meditation Block" )
		{
			float interpolationTimer = (Time.time - lastUpdateTime) / movementTime;
			Vector3 newPosition = Vector3.Lerp( new Vector3( -2.2f, (prev_meditation / 100.0f) * 3.5f, 0), new Vector3( -2.2f, (meditation / 100.0f) * 3.5f, 0), interpolationTimer );
			
			transform.localPosition = newPosition;
			
			if( interpolationTimer > 1 )
			{
				prev_meditation = meditation;
			}
		}
	}
	
	void OnEvent( EventData evt )
	{
		if( evt.serviceType == EventBase.ServiceType.ServiceTypeBrain )
		{
			signalStrength = evt.getExtraDataFloat(0);
			attention = evt.getExtraDataFloat(1);
			meditation = evt.getExtraDataFloat(2);
			delta = evt.getExtraDataFloat(3);
			theta = evt.getExtraDataFloat(4);
			lowAlpha = evt.getExtraDataFloat(5);
			highAlpha = evt.getExtraDataFloat(6);
			lowBeta = evt.getExtraDataFloat(7);
			highBeta = evt.getExtraDataFloat(8);
			lowGamma = evt.getExtraDataFloat(9);
			highGamma = evt.getExtraDataFloat(10);
			blinkStrength = evt.getExtraDataFloat(11);
			
			lastUpdateTime = Time.time;
		}
	} 
}
