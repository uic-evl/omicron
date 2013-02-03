/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Victor Mateevitsi		mvictoras@gmail.com
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

#include "omicron/OpenNIService.h"
#include "omicron/StringUtils.h"

using namespace omicron;
using namespace xn;

OpenNIService* OpenNIService::myOpenNI = NULL;
XnChar OpenNIService::omg_strPose[20] = ""; // XXX - What is this ???
XnBool OpenNIService::omg_bNeedPose = FALSE;
xn::Context OpenNIService::omg_Context = NULL;
//xn::DepthGenerator OpenNIService::omg_DepthGenerator = NULL;
//xn::UserGenerator OpenNIService::omg_UserGenerator = NULL;
Vector<xn::DepthGenerator> *OpenNIService::omg_DepthGenerator_v;
Vector<xn::UserGenerator> *OpenNIService::omg_UserGenerator_v;
//xn::SceneMetaData OpenNIService::omg_sceneMD = NULL;
bool OpenNIService::loadCalibrationFromFile = false;
const char* OpenNIService::calibrationFile = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
OpenNIService::OpenNIService()
{
	Colors[0][0] = 0; Colors[0][1] = 1; Colors[0][2] = 1;
	Colors[1][0] = 0; Colors[1][1] = 0; Colors[1][2] = 1;
	Colors[2][0] = 0; Colors[2][1] = 1; Colors[2][2] = 0;
	Colors[3][0] = 1; Colors[3][1] = 1; Colors[3][2] = 0;
	Colors[4][0] = 1; Colors[4][1] = 0; Colors[4][2] = 0;
	Colors[5][0] = 1; Colors[5][1] = .5; Colors[5][2] = 0;
	Colors[6][0] = .5; Colors[6][1] = 1; Colors[6][2] = 0;
	Colors[7][0] = 0; Colors[7][1] = .5; Colors[7][2] = 1;
	Colors[8][0] = .5; Colors[8][1] = 0; Colors[8][2] = 1;
	Colors[9][0] = 1; Colors[9][1] = 1; Colors[9][2] = .5;
	Colors[10][0] = 1; Colors[10][1] = 1; Colors[10][2] = 1;

	pDepthTexBuf = new unsigned char[640 * 480 * 4];

	omg_DepthGenerator_v = new Vector<DepthGenerator>();
	omg_UserGenerator_v = new Vector<UserGenerator>();

	trackClosestEnabled = false;
	trackClosestUser = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OpenNIService::~OpenNIService()
{
	omg_Context.Shutdown();

	delete pDepthTexBuf;
	delete omg_DepthGenerator_v;
	delete omg_UserGenerator_v;
	delete [] myTransform;
	delete [] serviceId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Color OpenNIService::getUserColor(int userId)
{
	if(userId < 11)
	{
		return Color(Colors[userId][0], Colors[userId][1], Colors[userId][2], 1.0f);
	}
	else return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// XXX HACKED TO WORK FOR 1 OR 2 KINECTS.
// BUG IN OPENNI
void OpenNIService::initialize()
{
	myOpenNI = this;
	
	omg_Context.Init();
	omg_Context.SetGlobalMirror(true);
	// Enumerate devices
	// enumerate devices 
	static xn::NodeInfoList node_info_list; 
	static xn::NodeInfoList depth_nodes; 
	static xn::NodeInfoList user_nodes;

	if (omg_Context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, node_info_list) != XN_STATUS_OK && node_info_list.Begin () !=  node_info_list.End ()) 
		omsg("OpenNIService: enumerating devices failed. Reason: "); 

	omg_Context.EnumerateProductionTrees (XN_NODE_TYPE_DEPTH, NULL, depth_nodes, NULL);
	omg_Context.EnumerateProductionTrees (XN_NODE_TYPE_USER, NULL, user_nodes, NULL);


	Vector<xn::NodeInfo> device_info;

	//for (xn::NodeInfoList::Iterator nodeIt = node_info_list.Begin (); nodeIt != node_info_list.End (); ++nodeIt) 
	// DEPTH GENERATOR
	int j = 0;
	for (xn::NodeInfoList::Iterator nodeIt = depth_nodes.Begin (); nodeIt != depth_nodes.End() && j < nmbKinects; ++nodeIt, j++) 
	{ 
		xn::NodeInfo info = *nodeIt;  
        omg_Context.CreateProductionTree (info); 
		const XnProductionNodeDescription& description = info.GetDescription(); 
		omsg("OpenNIService: image: vendor" + String(description.strVendor) + " name " + String(description.strName) + " instance " + info.GetInstanceName());

		DepthGenerator omg_DepthGenerator;

		info.GetInstance( omg_DepthGenerator ); 
		omg_DepthGenerator.StartGenerating();
		omg_DepthGenerator_v->push_back(omg_DepthGenerator);
	}

	// USER GENERATOR
	int i = 1;
	int nmbKinects_fake = 1;
	j = 0;
	if( nmbKinects == 2 ) nmbKinects_fake = 4;
	for (xn::NodeInfoList::Iterator nodeIt = user_nodes.Begin(); nodeIt != user_nodes.End () && j < nmbKinects_fake; ++nodeIt, i++, j++) 
	{ 
		if(i == 1 || i == 4) {
			xn::NodeInfo info = *nodeIt;  
			omg_Context.CreateProductionTree (info); 
			const XnProductionNodeDescription& description = info.GetDescription(); 
			omsg("image: vendor" + String(description.strVendor) + " name " + String(description.strName) + " instance " + info.GetInstanceName());

			UserGenerator omg_UserGenerator;
	 
			info.GetInstance( omg_UserGenerator ); 
			omg_UserGenerator.StartGenerating();
			omg_UserGenerator_v->push_back(omg_UserGenerator);
		

		

			// Callbacks
			// XXX - HACKED TO WORK FOR 1 OR 2 KINECTS
			if( i == 1 ) {
				XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;

				if (!omg_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
				{
					printf("OpenNIService: Supplied user generator doesn't support skeleton\n");
					
				}
				omg_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
				omg_UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, serviceId + 0, hCalibrationCallbacks); // device 0

				if (omg_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
				{
					OpenNIService::omg_bNeedPose = TRUE;

					if (!omg_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
					{
						printf("OpenNIService: Pose required, but not supported\n");
						//return 1;
					}

					omg_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, serviceId + 0, hPoseCallbacks);
					omg_UserGenerator.GetSkeletonCap().GetCalibrationPose(omg_strPose);
				}

				omg_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
			}

			if( i == 4 ) {
				XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;

				if (!omg_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
				{
					printf("OpenNIService: Supplied user generator doesn't support skeleton\n");
					
				}
				omg_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
				omg_UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, serviceId + 1, hCalibrationCallbacks); // device 2

				if (omg_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
				{
					OpenNIService::omg_bNeedPose = TRUE;

					if (!omg_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
					{
						printf("OpenNIService: Pose required, but not supported\n");
						//return 1;
					}
					omg_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, serviceId + 1, hPoseCallbacks);
					omg_UserGenerator.GetSkeletonCap().GetCalibrationPose(omg_strPose);
				}

				omg_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
			}
		}
		
	}
	
	
	//omg_Context.SetGlobalMirror(true);
	//omg_Context.StartGeneratingAll();	

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OpenNIService::setup(Setting& settings)
{
	/* Find the number of kinects */
	nmbKinects = 1;
	if(settings.exists("nmbKinects"))
	{
		nmbKinects = settings["nmbKinects"];
	}
	myTransform = new AffineTransform3[nmbKinects]();
	serviceId = new int[nmbKinects]();

	for(int i = 0; i < nmbKinects; i++ ) 
	{	
		myTransform[i] = AffineTransform3::Identity();
		serviceId[i] = i;
	}

	if(settings.exists("referenceTransform"))
	{
		Setting& srt = settings["referenceTransform"];
		for(int i = 0; i < nmbKinects; i++ ) 
		{
			Vector3f refTranslation = Vector3f::Zero();
			Matrix3f refLinear = Matrix3f::Identity();

			char buffer[50];
			sprintf(buffer, "referenceTranslation%d", i);

			if(srt.exists(buffer))
			{
				Setting& st = srt[buffer];
				refTranslation.x() = (float)st[0];
				refTranslation.y() = (float)st[1];
				refTranslation.z() = (float)st[2];
			}

			sprintf(buffer, "referenceLinear%d", i);
			if(srt.exists(buffer))
			{
				Setting& st = srt[buffer];
				for(int i = 0; i < 9; i++)
				{
					refLinear(i) = st[i];
				}
			}

			/* For compatibility with previous versions of omegalib */
			if(srt.exists("referenceTranslation"))
			{
				Setting& st = srt["referenceTranslation"];
				refTranslation.x() = (float)st[0];
				refTranslation.y() = (float)st[1];
				refTranslation.z() = (float)st[2];
			}
			if(srt.exists("referenceLinear" + i))
			{
				Setting& st = srt["referenceLinear"];
				for(int i = 0; i < 9; i++)
				{
					refLinear(i) = st[i];
				}
			}

			myTransform[i].linear() = refLinear;
			myTransform[i].translation() = refTranslation;
		}
	}

	myUseTrackables = false;
	if(settings.exists("useTrackables"))
	{
		myUseTrackables = settings["useTrackables"];
	}
	if(myUseTrackables)
	{
		Setting& strs = settings["trackables"];
		for(int i = 0; i < strs.getLength(); i++)
		{
			Setting& str = strs[i];
			Trackable trackable;
			trackable.userId = str["userId"];
			trackable.jointId = str["jointId"];
			trackable.trackableId = str["trackableId"];
			myTrackables.push_back(trackable);
		}
	}

	streamAll = false;
	if(settings.exists("streamAll"))
	{
		streamAll = settings["streamAll"];
	}

	loadCalibrationFromFile = false;
	if(settings.exists("loadCalibrationFromFile"))
	{
		if( settings["loadCalibrationFromFile"].getType() == Setting::TypeBoolean )
			loadCalibrationFromFile = settings["loadCalibrationFromFile"];
		else
		{
			loadCalibrationFromFile = true;
			calibrationFile = (const char*) settings["loadCalibrationFromFile"];
		}
	}

	if(settings.exists("trackClosest"))
	{
		Setting& st = settings["trackClosest"];
		for(int i = 0; i < 3; i++)
		{
			trackClosest[i] = st[i];
		}
		trackClosestEnabled = true;
		omsg("OpenNIService: Tracking closest user");
	}
}

void OpenNIService::start()
{
}

void OpenNIService::stop()
{
}

void OpenNIService::dispose()
{
	delete myOpenNI;
}


// Have to poll from both devices !
void OpenNIService::poll(void)
{
	if( myOpenNI )
	{
		xn::DepthGenerator omg_DepthGenerator = omg_DepthGenerator_v->at(0);
		xn::UserGenerator omg_UserGenerator = omg_UserGenerator_v->at(0);
		
		omg_Context.WaitAndUpdateAll();

		xn::SceneMetaData sceneMD;
		xn::DepthMetaData depthMD;
		omg_DepthGenerator.GetMetaData(depthMD);
		omg_UserGenerator.GetUserPixels(0, sceneMD);

		// Read next available data
		

		//process the frame data and store it in events, one event per rigid body
		XnUserID aUsers[OMICRON_OPENNI_MAX_USERS];
		XnUInt16 nUsers = OMICRON_OPENNI_MAX_USERS;

		int counter = 0;
		do
		{
			omg_DepthGenerator = omg_DepthGenerator_v->at(counter);
			omg_UserGenerator = omg_UserGenerator_v->at(counter);
			omg_UserGenerator.GetUsers(aUsers, nUsers);
			counter++;
		} while( aUsers[counter] == 0 && counter < nmbKinects );


		// Store the texture
		getTexture(depthMD, sceneMD);

		myOpenNI->lockEvents();

		float currentMinimum = 1000000;
		// Find the minimum
		for (int i = 0; i < nUsers; ++i)
		{
			XnPoint3D com;
			omg_UserGenerator.GetCoM(aUsers[i], com);
			omg_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);

			if( trackClosestEnabled ) 
			{
				Vector3f headPosition;
				if( getJointPosition(aUsers[i], OMICRON_SKEL_HEAD, headPosition, 0) ) {
					headPosition = myTransform[0] * headPosition;
				}

				float dist = headPosition[2] - trackClosest[2];
				if( dist < currentMinimum ) 
				{
					trackClosestUser = i;
					currentMinimum = dist;
				}

			}
		}

		for (int i = 0; i < nUsers; ++i)
		{
			if( trackClosestEnabled && i != trackClosestUser) continue;

			XnPoint3D com;
			omg_UserGenerator.GetCoM(aUsers[i], com);
			omg_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);

			//if ( omg_UserGenerator_v->at(0).GetSkeletonCap().IsTracking(aUsers[i]) || omg_UserGenerator_v->at(1).GetSkeletonCap().IsTracking(aUsers[i]) )
			//{
				if(streamAll)
				{
					for(int j = 0; j < nmbKinects; j++) 
					{
						omg_UserGenerator = omg_UserGenerator_v->at(j);
						if( omg_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]) ) {
							Event* theEvent = myOpenNI->writeHead();
							// j is the kinect index, used as service / device id
							theEvent->reset(Event::Update, Service::Mocap, aUsers[i], j);

                            theEvent->setExtraDataType(Event::ExtraDataVector3Array);

							joint2eventPointSet(aUsers[i], OMICRON_SKEL_HEAD, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_NECK, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_TORSO, theEvent, j);

							// Left side
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_SHOULDER, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_ELBOW, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_HAND, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_HIP, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_KNEE, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_FOOT, theEvent, j);

							// Right side
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_SHOULDER, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_ELBOW, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_HAND, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_HIP, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_KNEE, theEvent, j);
							joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_FOOT, theEvent, j);
						}
					}
				}
				else if(myUseTrackables)
				{
					for(int j = 0; j < myTrackables.size(); j++)
					{
						Trackable& t = myTrackables[j];
						if(t.userId == -1 || t.userId == aUsers[i])
						{
							// Write a trackable event for the specified joint.
							Vector3f pos;
							if( getJointPosition(aUsers[i], (OmegaSkeletonJoint)t.jointId, pos) ) 
							{
								Event* theEvent = this->writeHead();
								theEvent->reset(Event::Update, Service::Mocap, t.trackableId);
								theEvent->setPosition(pos);
								theEvent->setOrientation(Quaternion::Identity());						
							}
						}
					}
				}
				else
				{
					Event* theEvent = myOpenNI->writeHead();
					theEvent->reset(Event::Update, Service::Mocap, aUsers[i]);

                    theEvent->setExtraDataType(Event::ExtraDataVector3Array);

					joint2eventPointSet(aUsers[i], OMICRON_SKEL_HEAD, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_NECK, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_TORSO, theEvent );

					// Left side
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_SHOULDER, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_ELBOW, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_HAND, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_HIP, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_KNEE, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_LEFT_FOOT, theEvent);

					// Right side
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_SHOULDER, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_ELBOW, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_HAND, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_HIP, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_KNEE, theEvent);
					joint2eventPointSet(aUsers[i], OMICRON_SKEL_RIGHT_FOOT, theEvent);
				}
			//}
		}
		myOpenNI->unlockEvents();
	}
}

void OpenNIService::joint2eventPointSet(XnUserID player, XnSkeletonJoint joint, Event* theEvent, int kinectID) {
	Vector3f ps;
	if( getJointPosition(player, joint, ps, kinectID) ) {

		// Transform position
		Vector3f pos;
		pos = myTransform[kinectID] * ps;
		theEvent->setExtraDataVector3(joint, pos);

		// Event position = Head position (simplifies compatibility with head tracking service)
		if( joint == OMICRON_SKEL_HEAD ) {
			theEvent->setPosition(pos);
		}
	}
}

void OpenNIService::joint2eventPointSet(XnUserID player, XnSkeletonJoint joint, Event* theEvent) {
	Vector3f pos;
	if( getJointPosition(player, joint, pos) ) {

		theEvent->setExtraDataVector3(joint, pos);

		// Event position = Head position (simplifies compatibility with head tracking service)
		if( joint == OMICRON_SKEL_HEAD ) {
			theEvent->setPosition(pos);
		}
	}
}

/**
  * getJointPosition: 
  * Get the joint from kinectID device
  */
bool OpenNIService::getJointPosition(XnUserID player, XnSkeletonJoint joint, Vector3f &pos, int kinectID) {
	xn::DepthGenerator omg_DepthGenerator = omg_DepthGenerator_v->at(kinectID);
	xn::UserGenerator omg_UserGenerator = omg_UserGenerator_v->at(kinectID);

	if( !omg_UserGenerator.GetSkeletonCap().IsTracking(player) )
		return false;

	XnSkeletonJointPosition jointPos;
	omg_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, joint, jointPos);

	// if the confidence from the first is low, try the second one!
	if( jointPos.fConfidence < 0.5 ) return false;

	pos[0] = jointPos.position.X;
	pos[1] = jointPos.position.Y;
	pos[2] = jointPos.position.Z;

	return true;
	
}

/**
  * getJointPosition: 
  * Gets the  joint from kinect1. If no joint, then gets from kinect2.
  * returns false if there is a problem
  * position of joint is stored in pos
  */
bool OpenNIService::getJointPosition(XnUserID player, XnSkeletonJoint joint, Vector3f &pos) {
	//xn::DepthGenerator omg_DepthGenerator = omg_DepthGenerator_v->at(0);
	//xn::UserGenerator omg_UserGenerator = omg_UserGenerator_v->at(0);

	XnSkeletonJointPosition jointPos;
	//omg_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, joint, jointPos);

	// if the confidence from the first is low, try the second one!
	for(int i = 0; i < nmbKinects; i++ ) 
	{
		//if( !omg_UserGenerator.GetSkeletonCap().IsTracking(player) || jointPos.fConfidence < 0.5 )
		//{
			xn::DepthGenerator omg_DepthGenerator = omg_DepthGenerator_v->at(i);
			xn::UserGenerator omg_UserGenerator = omg_UserGenerator_v->at(i);
			omg_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, joint, jointPos);

			// If we find ANY of the kinects to contain the correct information
			// then save it and return true. Else return false.
			if( jointPos.fConfidence >= 0.5 )
			{
				pos[0] = jointPos.position.X;
				pos[1] = jointPos.position.Y;
				pos[2] = jointPos.position.Z;

				pos = myTransform[i] * pos;
				return true;
			}
		//}
	}

	return false;
}

// Callback: New user was detected
void XN_CALLBACK_TYPE OpenNIService::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	ofmsg("OpenNIService: New User %1%", %(int)nId);

	if(loadCalibrationFromFile)
	{
		XnStatus status = generator.GetSkeletonCap().LoadCalibrationDataFromFile(nId, calibrationFile);
		ofmsg("OpenNIService: User %1% tracked on %2%", %(int)nId %(bool)generator.GetSkeletonCap().IsCalibrated(nId) );
        generator.GetSkeletonCap().StartTracking(nId);
	}
    else 
    {

	    // New user found
	    if ( OpenNIService::omg_bNeedPose)
	    {
		    generator.GetPoseDetectionCap().StartPoseDetection(omg_strPose, nId);
        }	
	    else
	    {
		    generator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE OpenNIService::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	ofmsg("Lost user %1%\n", %(int)nId);
	myOpenNI->lockEvents();
	Event* theEvent = myOpenNI->writeHead();
	theEvent->reset(Event::Untrace, Service::Mocap, nId);
	theEvent->setPosition(Vector3f::Zero());
	theEvent->setOrientation(Quaternion::Identity());
	myOpenNI->unlockEvents();
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE OpenNIService::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	int device = ((int*)(pCookie))[0];
	xn::DepthGenerator omg_DepthGenerator = omg_DepthGenerator_v->at(device);
	xn::UserGenerator omg_UserGenerator = omg_UserGenerator_v->at(device);

	ofmsg("OpenNIService: Pose %1% detected for user %2% on Device%3%", %strPose %(int)nId %device);
	omg_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	omg_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE OpenNIService::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	int device = ((int*)(pCookie))[0];
	ofmsg("OpenNIService: Calibration started for user %1% on Device%2%", %(int)nId %device);
}

// Callback: Finished calibration
void XN_CALLBACK_TYPE OpenNIService::UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	int device = ((int*)(pCookie))[0];
	xn::DepthGenerator omg_DepthGenerator = omg_DepthGenerator_v->at(device);
	xn::UserGenerator omg_UserGenerator = omg_UserGenerator_v->at(device);
	if (bSuccess)
	{
		// Calibration succeeded
		ofmsg("OpenNIService: Calibration complete, start tracking user %1% on Device%2%", %(int)nId %device);
		omg_UserGenerator.GetSkeletonCap().StartTracking(nId);

        omg_UserGenerator.GetSkeletonCap().SaveCalibrationData(nId, 0);
		myOpenNI->lockEvents();
		Event* theEvent = myOpenNI->writeHead();
		theEvent->reset(Event::Trace, Service::Mocap, nId);
		theEvent->setPosition(Vector3f::Zero());
		theEvent->setOrientation(Quaternion::Identity());
		myOpenNI->unlockEvents();
	}
	else
	{
		// Calibration failed
		ofmsg("OpenNIService: Calibration failed for user %1% on Device%2%", %(int)nId %device);
		if ( OpenNIService::omg_bNeedPose )
		{
			omg_UserGenerator.GetPoseDetectionCap().StartPoseDetection(omg_strPose, nId);
		}
		else
		{
			omg_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}

void OpenNIService::getTexture(xn::DepthMetaData& dmd, xn::SceneMetaData& smd) 
{
	static bool bInitialized = false;	
	static uint depthTexID;
	static int texWidth, texHeight;
	float topLeftX = dmd.XRes();
	float topLeftY = 0;
	float bottomRightY = dmd.YRes();
	float bottomRightX = 0;
	float texXpos = (float)dmd.XRes()/texWidth;
	float texYpos = (float)dmd.YRes()/texHeight; 

	//memset(texcoords, 0, 8*sizeof(float));
	//texcoords[0] = texXpos, texcoords[1] = texYpos, texcoords[2] = texXpos, texcoords[7] = texYpos;

	unsigned int nValue = 0;
	unsigned int nHistValue = 0;
	unsigned int nIndex = 0;
	unsigned int nX = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	XnUInt16 g_nXRes = dmd.XRes();
	XnUInt16 g_nYRes = dmd.YRes();

	//pDepthTexBuf = new unsigned char[g_nXRes * g_nYRes * 4];

	unsigned char* pDestImage = pDepthTexBuf;

	const XnDepthPixel* pDepth = dmd.Data();
	const XnLabel* pLabels = smd.Data();

	// Calculate the accumulative histogram
	memset(g_pDepthHist, 0, OMICRON_OPENNI_MAX_DEPTH*sizeof(float));
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX<g_nXRes; nX++)
		{
			nValue = *pDepth;

			if (nValue != 0)
			{
				g_pDepthHist[nValue]++;
				nNumberOfPoints++;
			}

			pDepth++;
		}
	}

	for (nIndex=1; nIndex<OMICRON_OPENNI_MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<OMICRON_OPENNI_MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	pDepth = dmd.Data();
	nIndex = 0;
	texWidth = 640;
	// Prepare the texture map
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX < g_nXRes; nX++, nIndex++)
		{

			pDestImage[0] = 0;
			pDestImage[1] = 0;
			pDestImage[2] = 0;
			pDestImage[3] = 255;
			//if ( *pLabels != 0 )
			//{
				nValue = *pDepth;
				XnLabel label = *pLabels;
				XnUInt32 nColorID = label % nColors;
				if (label == 0)
				{
					nColorID = nColors;
				}

				if (nValue != 0)
				{
					nHistValue = g_pDepthHist[nValue];

					pDestImage[0] = nHistValue * Colors[nColorID][0]; 
					pDestImage[1] = nHistValue * Colors[nColorID][1];
					pDestImage[2] = nHistValue * Colors[nColorID][2];
				}
			//}

			pDepth++;
			pLabels++;
			pDestImage+=4;
		}

		pDestImage += (texWidth - g_nXRes) *4;
	}
}

void* OpenNIService::getDepthImageData()
{
	return pDepthTexBuf;
}

int OpenNIService::getImageDataWidth()
{
	return 640;
}

int OpenNIService::getImageDataHeight()
{
	return 480;
}