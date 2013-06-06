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

public class OmicronKinectScript : MonoBehaviour {
	public EventBase.OmicronSkeletonJoint jointID;
	
	public int skeletonID = -1;
	public Vector3 jointLocalPosition;
	
	public bool flipXAxis = true;
	
	// Use this for initialization
	void Start () {
		if( gameObject.tag != "OmicronListener" ){
			gameObject.tag = "OmicronListener";
		}
	}
	
	// Update is called once per frame
	void Update () {
		transform.localPosition = jointLocalPosition;
	}
	
	// Kinect Event Data:
	// sourceID = skeleton ID
	// position = head position
	// extraDataVector3[n] = joint position where n is the joint ID
	// *Note* Based on provider (KinectSDK / OpenNI) not all joints will return values
	/* OmicronSkeletonJoint enum in OmicronConnectorClient.cs
	 		OMICRON_SKEL_HIP_CENTER, (MS KinectSDK only)
            OMICRON_SKEL_HEAD,	
            OMICRON_SKEL_NECK, (OpenNI only)
            OMICRON_SKEL_TORSO, (OpenNI only)
            OMICRON_SKEL_WAIST, (OpenNI only)

            OMICRON_SKEL_LEFT_COLLAR, (OpenNI only)
            OMICRON_SKEL_LEFT_SHOULDER,
            OMICRON_SKEL_LEFT_ELBOW,
            OMICRON_SKEL_LEFT_WRIST,
            OMICRON_SKEL_LEFT_HAND,
            OMICRON_SKEL_LEFT_FINGERTIP, (OpenNI only)

            OMICRON_SKEL_LEFT_HIP,
            OMICRON_SKEL_LEFT_KNEE,
            OMICRON_SKEL_LEFT_ANKLE,
            OMICRON_SKEL_LEFT_FOOT,

            OMICRON_SKEL_RIGHT_COLLAR, (OpenNI only)
            OMICRON_SKEL_RIGHT_SHOULDER,
            OMICRON_SKEL_RIGHT_ELBOW,
            OMICRON_SKEL_RIGHT_WRIST,
            OMICRON_SKEL_RIGHT_HAND,
            OMICRON_SKEL_RIGHT_FINGERTIP, (OpenNI only)

            OMICRON_SKEL_RIGHT_HIP,
            OMICRON_SKEL_RIGHT_KNEE,
            OMICRON_SKEL_RIGHT_ANKLE,
            OMICRON_SKEL_RIGHT_FOOT,

            OMICRON_SKEL_SPINE, (MS KinectSDK only)
            OMICRON_SKEL_SHOULDER_CENTER, (MS KinectSDK only)

            OMICRON_SKEL_COUNT (Not currently used)
     */
	void OnEvent( EventData evt )
	{
		skeletonID = (int)evt.sourceId;
		
		// If this is a Mocap event...
		if( evt.serviceType == EventBase.ServiceType.ServiceTypeMocap )
		{
			float[] vec = { 0, 0, 0 };
			
			// Make sure this is the correct joint
			if( evt.getExtraDataVector3( (int)jointID, vec ) )
			{
				// Account for Kinect using right-handed, Unity using left-handed coordinates
				if( flipXAxis )
					vec[0] *= -1;
				
				jointLocalPosition = new Vector3( vec[0], vec[1], vec[2] );
			}
		}
	}
}
