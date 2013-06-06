/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Victor Mateevitsi		mvictoras@gmail.com
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
#ifndef __OPENNI_SERVICE_H__
#define __OPENNI_SERVICE_H__

#include "omicron/osystem.h"
#include "omicron/Color.h"
#include "omicron/ServiceManager.h"
//#include "natnet/NatNetTypes.h"

#include <XnCppWrapper.h>

#ifdef WIN32
#include "winsock2.h"
#endif
//#define KINECT_CONFIG "../../data/openni/conf/KinectConfig.xml"
#define KINECT_CONFIG "./../../data/openni/conf/KinectConfig.xml"
#define OMICRON_OPENNI_MAX_USERS 15

#define OMICRON_SKEL_HEAD				XN_SKEL_HEAD
#define OMICRON_SKEL_NECK				XN_SKEL_NECK
#define	OMICRON_SKEL_TORSO			XN_SKEL_TORSO
#define OMICRON_SKEL_WAIST			XN_SKEL_WAIST

#define OMICRON_SKEL_LEFT_COLLAR		XN_SKEL_LEFT_COLLAR
#define OMICRON_SKEL_LEFT_SHOULDER	XN_SKEL_LEFT_SHOULDER
#define OMICRON_SKEL_LEFT_ELBOW		XN_SKEL_LEFT_ELBOW
#define OMICRON_SKEL_LEFT_WRIST		XN_SKEL_LEFT_WRIST
#define OMICRON_SKEL_LEFT_HAND		XN_SKEL_LEFT_HAND
#define OMICRON_SKEL_LEFT_FINGERTIP	XN_SKEL_LEFT_FINGERTIP

#define OMICRON_SKEL_LEFT_HIP			XN_SKEL_LEFT_HIP
#define OMICRON_SKEL_LEFT_KNEE		XN_SKEL_LEFT_KNEE
#define OMICRON_SKEL_LEFT_ANKLE		XN_SKEL_LEFT_ANKLE
#define OMICRON_SKEL_LEFT_FOOT		XN_SKEL_LEFT_FOOT

#define OMICRON_SKEL_RIGHT_COLLAR		XN_SKEL_RIGHT_COLLAR
#define OMICRON_SKEL_RIGHT_SHOULDER	XN_SKEL_RIGHT_SHOULDER
#define OMICRON_SKEL_RIGHT_ELBOW		XN_SKEL_RIGHT_ELBOW
#define OMICRON_SKEL_RIGHT_WRIST		XN_SKEL_RIGHT_WRIST
#define OMICRON_SKEL_RIGHT_HAND		XN_SKEL_RIGHT_HAND
#define OMICRON_SKEL_RIGHT_FINGERTIP	XN_SKEL_RIGHT_FINGERTIP

#define OMICRON_SKEL_RIGHT_HIP		XN_SKEL_RIGHT_HIP
#define OMICRON_SKEL_RIGHT_KNEE		XN_SKEL_RIGHT_KNEE
#define OMICRON_SKEL_RIGHT_ANKLE		XN_SKEL_RIGHT_ANKLE
#define OMICRON_SKEL_RIGHT_FOOT		XN_SKEL_RIGHT_FOOT


#define OMICRON_OPENNI_MAX_DEPTH		10000

namespace omicron
{
	// Typedefs for the OpenNIService - omega integration
	typedef xn::DepthMetaData DepthMetaData;
	typedef xn::SceneMetaData RGBMetaData;
	typedef XnSkeletonJoint OmegaSkeletonJoint;

	class OMICRON_API OpenNIService : public Service
	{
	public:
		// Allocator function
		static OpenNIService* New() { return new OpenNIService(); }

	public:
		OpenNIService();
		~OpenNIService();
		void setup( Setting& settings);
		virtual void initialize();
		virtual void start();//initialize and start service here
		virtual void stop();//destroy service instance to stop
		virtual void dispose();
		virtual void poll();
		void* getDepthImageData();
		int getImageDataWidth();
		int getImageDataHeight();

		//! Returns the color used to identify the specific user id on the depth image.
		Color getUserColor(int userId);
		//may want to support the option to choose whether to have unicast or multicast networking
		//for now it is hard coded to multicast
	
	private:
		static OpenNIService* myOpenNI;

		// Used for motion capture trackable emulation.
		struct Trackable
		{
			int userId;
			int jointId;
			int trackableId;
		};

	public:
		// For the openni interaction
		static XnChar omg_strPose[20]; // XXX - What is this ???
		static XnBool omg_bNeedPose;
		static xn::Context omg_Context;
		
		static Vector<xn::DepthGenerator> *omg_DepthGenerator_v;
		static Vector<xn::UserGenerator> *omg_UserGenerator_v;
		
		static xn::SceneMetaData omg_sceneMD;

	private:
		float g_pDepthHist[OMICRON_OPENNI_MAX_DEPTH];
		static const int nColors = 10;

		unsigned char* pDepthTexBuf;

		float Colors[11][3];
		static bool loadCalibrationFromFile;
		static const char* calibrationFile;

		// Reference frame transform
		AffineTransform3 *myTransform;
		bool myUseTrackables;
		bool streamAll;
		int nmbKinects;
		int *serviceId;

		Vector<Trackable> myTrackables;

		Vector3f trackClosest;
		int trackClosestUser;
		bool trackClosestEnabled;

	private:

		static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
		static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);

		static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
		static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);

		static void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie);

		// XXX Do we need all those ????
		bool getJointPosition(XnUserID player, XnSkeletonJoint joint, Vector3f &pos);
		bool getJointPosition(XnUserID player, XnSkeletonJoint joint, Vector3f &pos, int kinectID);
		void joint2eventPointSet(XnUserID player, XnSkeletonJoint joint, Event* theEvent, int kinectID);
		void joint2eventPointSet(XnUserID player, XnSkeletonJoint joint, Event* theEvent);

		void getTexture(xn::DepthMetaData& dmd, xn::SceneMetaData& smd);

	};//class KinectService
};//namespace omicron


#endif
