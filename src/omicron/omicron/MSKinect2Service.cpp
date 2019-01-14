/**************************************************************************************************
* THE OMICRON PROJECT
*-------------------------------------------------------------------------------------------------
* Copyright 2010-2016		Electronic Visualization Laboratory, University of Illinois at Chicago
* Authors:										
*  Arthur Nishimoto		anishimoto42@gmail.com
*-------------------------------------------------------------------------------------------------
* Copyright (c) 2010-2016, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omicron/MSKinect2Service.h"
using namespace omicron;

enum TRACKED_SKELETONS
{
	SV_TRACKED_SKELETONS_DEFAULT = 0,
	SV_TRACKED_SKELETONS_NEAREST1,
	SV_TRACKED_SKELETONS_NEAREST2,
	SV_TRACKED_SKELETONS_STICKY1,
	SV_TRACKED_SKELETONS_STICKY2
} TRACKED_SKELETONS;

enum TRACKING_MODE
{
	TRACKING_MODE_DEFAULT = 0,
	TRACKING_MODE_SEATED
} TRACKING_MODE;

enum RANGE
{
	RANGE_DEFAULT = 0,
	RANGE_NEAR,
} RANGE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MSKinectService::MSKinectService(){
	kinectSensor = NULL;
	bodyFrameReader = NULL;
	m_instanceId = NULL;
	m_TrackedSkeletons = 0;
	m_hNextSkeletonEvent = NULL;
	skeletonEngineKinectID = -1;
	//m_SkeletonTrackingFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE;

#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	m_p16BitAudioStream = NULL;
	m_pSpeechStream = NULL;
	m_pSpeechRecognizer = NULL;
	m_pSpeechContext = NULL;
	m_pSpeechGrammar = NULL;
	m_hSpeechEvent = NULL;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::setup(Setting& settings)
{
	myUpdateInterval = Config::getFloatValue("updateInterval", settings, 0.01f);
	myCheckKinectInterval = Config::getFloatValue("checkInterval", settings, 2.00f);

	m_bSeatedMode = Config::getBoolValue("seatedMode", settings, false);

	debugInfo = Config::getBoolValue("debug", settings, false);

	enableKinectBody = Config::getBoolValue("enableKinectBody", settings, true);
	enableKinectColor = Config::getBoolValue("enableKinectColor", settings, false);
	enableKinectAudio = Config::getBoolValue("enableKinectSpeech", settings, false);
	enableKinectSpeechGrammar = Config::getBoolValue("useGrammar", settings, true);
	enableKinectSpeechDictation = Config::getBoolValue("useDictation", settings, false);

	caveSimulator = Config::getBoolValue("caveSimulator", settings, false);
	caveSimulatorHeadID = Config::getIntValue("caveSimulatorHeadID", settings, 0);
	caveSimulatorWandID = Config::getIntValue("caveSimulatorWandID", settings, 1);

	kinectOriginOffset = Config::getVector3fValue("kinectOriginOffset", settings, Vector3f(0, 0, 0));

	if( caveSimulator )
	{
		omsg("MSKinectService: CAVE2 tracker simulation mode active!");
		ofmsg("   Closest Kinect head will be mapped to mocap ID %1%", %caveSimulatorHeadID);
		ofmsg("   Closest Kinect hand (wand) will be mapped to mocap ID %1%", %caveSimulatorWandID);
		ofmsg("   Kinect origin offset: %1%", %kinectOriginOffset);
	}
#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	speechGrammerFilePath = Config::getStringValue("speechGrammerFilePath", settings, "kinectSpeech.grxml");

	confidenceThreshold = Config::getFloatValue("confidenceThreshold", settings, 0.3f);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring MSKinectService::StringToWString(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::initialize()
{
	mysInstance = this;

	InitializeDefaultKinect();

#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	if (enableKinectAudio)
	{
		InitializeAudioStream();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::poll()
{
	if (enableKinectBody)
	{
		pollBody();
	}
	if (enableKinectColor)
	{
		pollColor();
	}

#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	if( enableKinectAudio )
	{
		pollSpeech();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::pollBody()
{
	if ( bodyFrameReader == NULL )
	{
		return;
	}

	IBodyFrame* pBodyFrame = NULL;

	HRESULT hr = bodyFrameReader->AcquireLatestFrame(&pBodyFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;

		hr = pBodyFrame->get_RelativeTime(&nTime);

		IBody* ppBodies[BODY_COUNT] = {0};

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}
		
		
		if (SUCCEEDED(hr))
		{
			Vector4* floorClipPlane = new Vector4();
			pBodyFrame->get_FloorClipPlane(floorClipPlane);

			ProcessBody(nTime, BODY_COUNT, ppBodies, floorClipPlane);
		}

		
		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}

	SafeRelease(pBodyFrame);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::pollSpeech() 
{
#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	if (enableKinectAudio)
	{
		if (enableKinectSpeechGrammar)
			ProcessSpeech();
		if (enableKinectSpeechDictation)
			ProcessSpeechDictation();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::pollColor()
{
	if (!m_pColorFrameReader)
	{
		return;
	}

	IColorFrame* pColorFrame = NULL;

	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;

		hr = pColorFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			BYTE* pImage = reinterpret_cast<BYTE*>(pBuffer);
			unsigned long pImageSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);

			int dataPacketSize = pImageSize / 8640; // Color image: 8294400 / 200 = 41472
						
			for (int i = 0; i < 8640; i++)
			{
				memcpy(imageBuffer, &pImage[i * dataPacketSize], dataPacketSize);
				Event* evt = mysInstance->writeHead();
				evt->reset(Event::Update, Service::Generic, 0);
				evt->setFlags(i);

				evt->setExtraData(EventBase::ExtraDataString, dataPacketSize, 1, imageBuffer);
				mysInstance->unlockEvents();
			}
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::dispose() 
{
	omsg("MSKinectService: Shutting down.");

	if (kinectSensor)
	{
		// done with color frame reader
		SafeRelease(m_pColorFrameReader);

		kinectSensor->Close();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK MSKinectService::Nui_StatusProcThunk( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void * pUserData )
{
	reinterpret_cast<MSKinectService *>(pUserData)->KinectStatusCallback( hrStatus, instanceName, uniqueDeviceName );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK MSKinectService::KinectStatusCallback( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName )
{
	ofmsg("%1%: not currently implemented/supported", %__FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT MSKinectService::InitializeDefaultKinect()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&kinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (kinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IBodyFrameSource* pBodyFrameSource = NULL;

		hr = kinectSensor->Open();

		if (enableKinectBody)
		{
			if (SUCCEEDED(hr))
			{
				hr = kinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
			}

			if (SUCCEEDED(hr))
			{
				hr = kinectSensor->get_BodyFrameSource(&pBodyFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrameSource->OpenReader(&bodyFrameReader);
			}
			SafeRelease(pBodyFrameSource);
			omsg("MSKinect2Service: Body tracking started.");
		}

		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;

		if (enableKinectColor)
		{
			if (SUCCEEDED(hr))
			{
				hr = kinectSensor->get_ColorFrameSource(&pColorFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
			}

			SafeRelease(pColorFrameSource);

			// create heap storage for color pixel data in RGBX format
			m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];

			omsg("MSKinect2Service: Color camera started.");
		}
	}

	if (!kinectSensor || FAILED(hr))
	{
		omsg("MSKinectService: InitializeKinect() - No ready Kinect found!");
		return E_FAIL;
	}
	else
	{
		omsg("MSKinectService: InitializeKinect() - Default Kinect ready!");
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOT USED IN V2.0
/*HRESULT MSKinectService::InitializeKinect()
{
	int sensorID = -1;
	HRESULT  hr;

	InitializeDefaultKinect();

#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	InitializeAudioStream();
#endif
	return hr;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::UnInitializeKinect( const OLECHAR *instanceName )
{
	std::wstring deviceWStr = instanceName;
	std::string deviceName( deviceWStr.begin(), deviceWStr.end() );

	if ( sensorList.count(deviceName) == 1 )
	{
		IKinectSensor* sensor = sensorList[deviceName];

		//if ( HasSkeletalEngine(sensor) && m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
		//{
		//	printf("MSKinectService: Kinect %d skeleton engine shutting down. \n", getKinectID(deviceName) );
		//	CloseHandle( m_hNextSkeletonEvent );
		//	m_hNextSkeletonEvent = NULL;
		//}

		sensorList.erase(deviceName);
		sensorIndexList.erase(deviceName);
		sensor->Close();
		printf("MSKinectService: Kinect %d disconnected. \n", getKinectID(deviceName) );
	}
	else
	{
		printf("MSKinectService: Attempted to disconnect non-connected sensor: %s. \n", deviceName.c_str() );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies, Vector4* floorClipPlane)
{
	HRESULT hr;

	UINT64 closestBodyIndex = 0;
	float closestSkeletonDistance = 10000;

	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];
		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);
			
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count]; 
				hr = pBody->GetJoints(_countof(joints), joints);

				if (SUCCEEDED(hr))
				{
					UINT64 skeletonID;
					pBody->get_TrackingId(&skeletonID);

					Vector3f headPos;
					float headDistance = joints[JointType_Head].Position.Z;
					if (headDistance < closestSkeletonDistance)
					{
						closestSkeletonDistance = headDistance;
						closestBodyIndex = i;
					}
					GenerateMocapEvent( pBody, joints, floorClipPlane );
				}
			}
		}
	}

	if (caveSimulator)
	{
		Joint joints[JointType_Count];
		hr = ppBodies[closestBodyIndex]->GetJoints(_countof(joints), joints);

		if (SUCCEEDED(hr))
		{

			Event* evt = mysInstance->writeHead();
			evt->reset(Event::Update, Service::Mocap, caveSimulatorHeadID);

			Joint curJoint = joints[JointType_Head];
			Vector3f jointPos = Vector3f(curJoint.Position.X, curJoint.Position.Y, curJoint.Position.Z) + kinectOriginOffset;
			Vector3f pos;

			// When facing the Kinect: +X to the right, +Y up, +Z toward the player
			pos[0] = jointPos.x();
			pos[1] = jointPos.y();
			pos[2] = jointPos.z();
			evt->setPosition(pos);
			evt->setOrientation(Quaternion::Identity());

			mysInstance->unlockEvents();

			Event* evt2 = mysInstance->writeHead();
			evt2->reset(Event::Update, Service::Mocap, caveSimulatorWandID);

			Joint jointL = joints[JointType_HandLeft];
			Joint jointR = joints[JointType_HandRight];

			Vector3f jointLPos = Vector3f(jointL.Position.X, jointL.Position.Y, jointL.Position.Z) + kinectOriginOffset;
			Vector3f jointRPos = Vector3f(jointR.Position.X, jointR.Position.Y, jointR.Position.Z) + kinectOriginOffset;

			Vector3f pos2;
			// Check for the closest hand
			if (jointLPos.z() < jointRPos.z())
			{
				pos2[0] = jointLPos.x();
				pos2[1] = jointLPos.y();
				pos2[2] = jointLPos.z();
			}
			else
			{
				pos2[0] = jointRPos.x();
				pos2[1] = jointRPos.y();
				pos2[2] = jointRPos.z();
			}

			evt2->setPosition(pos2);
			evt2->setOrientation(Quaternion::Identity());

			mysInstance->unlockEvents();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::GenerateMocapEvent( IBody* body, Joint* joints, Vector4* floorClipPlane )
{      
	UINT64 skeletonID;
	body->get_TrackingId(&skeletonID);

	Vector3f headPos;

	Joint head = joints[JointType_Head];
	headPos = Vector3f( head.Position.X, head.Position.Y, head.Position.Z );

	if (debugInfo)
	{
		ofmsg("Kinect Head %1% (%2%,%3%,%4%)", %skeletonID % (headPos.x()) % (headPos.y()) % (headPos.z()));
		ofmsg("Floor Clip Plane (%1%,%2%,%3%,%4%)", % (floorClipPlane->x) % (floorClipPlane->y) % (floorClipPlane->z) % (floorClipPlane->w));
	}
	HandState leftHandState = HandState_Unknown;
	HandState rightHandState = HandState_Unknown;
	body->get_HandLeftState(&leftHandState);
	body->get_HandRightState(&rightHandState);

	Event* evt = mysInstance->writeHead();
	evt->reset(Event::Update, Service::Mocap, skeletonID, 0);
	
	Joint joint = joints[JointType_Head];
	Vector3f jointPos = Vector3f( head.Position.X, head.Position.Y, head.Position.Z );
	Vector3f pos;
	pos[0] = jointPos.x();
	pos[1] = jointPos.y();
	pos[2] = jointPos.z();
	evt->setPosition( pos );

	// Floor clip plane
	// x, y, z, unit vector indicating normal of plane
	// w distance from the plane to origin in meters
	evt->setOrientation(floorClipPlane->w, floorClipPlane->x, floorClipPlane->y, floorClipPlane->z);

	uint flags = 0;
	uint userFlag = 1 << 18;
	switch (leftHandState)
	{
		case(HandState_Unknown):
			flags |= userFlag << 1;
			break;
		case(HandState_NotTracked):
			flags |= userFlag << 2;
			break;
		case(HandState_Open):
			flags |= userFlag << 3;
			break;
		case(HandState_Closed):
			flags |= userFlag << 4;
			break;
		case(HandState_Lasso):
			flags |= userFlag << 5;
			break;
	}
	switch (rightHandState)
	{
	case(HandState_Unknown):
		flags |= userFlag << 6;
		break;
	case(HandState_NotTracked):
		flags |= userFlag << 7;
		break;
	case(HandState_Open):
		flags |= userFlag << 8;
		break;
	case(HandState_Closed):
		flags |= userFlag << 9;
		break;
	case(HandState_Lasso):
		flags |= userFlag << 10;
		break;
	}
	evt->setFlags(flags);

	evt->setExtraDataType(Event::ExtraDataVector3Array);

	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_HIP_CENTER, JointType_SpineBase );
	
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_HEAD, JointType_Head );

	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_SHOULDER, JointType_ShoulderLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_ELBOW, JointType_ElbowLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_WRIST, JointType_WristLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_HAND, JointType_HandLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_FINGERTIP, JointType_HandTipLeft );
	
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_HIP, JointType_HipLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_KNEE, JointType_KneeLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_ANKLE, JointType_AnkleLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_FOOT, JointType_FootLeft );

	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_SHOULDER, JointType_ShoulderRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_ELBOW, JointType_ElbowRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_WRIST, JointType_WristRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_HAND, JointType_HandRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_FINGERTIP, JointType_HandTipRight );

	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_HIP, JointType_HipRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_KNEE, JointType_KneeRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_ANKLE, JointType_AnkleRight );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_FOOT, JointType_FootRight );

	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_SPINE, JointType_SpineMid );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_SHOULDER_CENTER, JointType_SpineShoulder );

	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_LEFT_THUMB, JointType_ThumbLeft );
	SkeletonPositionToEvent( joints, evt, Event::OMICRON_SKEL_RIGHT_THUMB, JointType_ThumbRight );

	mysInstance->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::SkeletonPositionToEvent( Joint* joints, Event* evt, Event::OmicronSkeletonJoint omicronIndex, JointType kinectIndex )
{
	Joint joint = joints[kinectIndex];
	Vector3f jointPos = Vector3f( joint.Position.X, joint.Position.Y, joint.Position.Z );
	Vector3f pos;
	pos[0] = jointPos.x();
	pos[1] = jointPos.y();
	pos[2] = jointPos.z();
	evt->setExtraDataVector3(omicronIndex, pos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Invoked when the user changes the selection of tracked skeletons
/// </summary>
/// <param name="mode">skelton tracking mode to switch to</param>
void MSKinectService::UpdateTrackedSkeletonSelection( int mode )
{
	m_TrackedSkeletons = mode;
	ofmsg("%1%: not currently implemented/supported", %__FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Invoked when the user changes the tracking mode
/// </summary>
/// <param name="mode">tracking mode to switch to</param>
void MSKinectService::UpdateTrackingMode( int mode )
{
	ofmsg("%1%: not currently implemented/supported", %__FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Invoked when the user changes the range
/// </summary>
/// <param name="mode">range to switch to</param>
void MSKinectService::UpdateRange( int mode )
{
	ofmsg("%1%: not currently implemented/supported", %__FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Sets or clears the specified skeleton tracking flag
/// </summary>
/// <param name="flag">flag to set or clear</param>
/// <param name="value">true to set, false to clear</param>
void MSKinectService::UpdateSkeletonTrackingFlag( DWORD flag, bool value )
{
	ofmsg("%1%: not currently implemented/supported", %__FUNCTION__);
}

#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kinect Speech Recognition
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Initialize Kinect audio stream object.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSKinectService::InitializeAudioStream()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	hr = S_OK;

	IAudioSource* pAudioSource = NULL;
	IAudioBeamList* pAudioBeamList = NULL;

	hr = GetDefaultKinectSensor(&kinectSensor);

	if (FAILED(hr))
	{
		omsg("MSKinect2Service: Failed getting default sensor!");
		return hr;
	}

	hr = kinectSensor->Open();

	if (SUCCEEDED(hr))
	{
		hr = kinectSensor->get_AudioSource(&pAudioSource);
	}

	if (SUCCEEDED(hr))
	{
		hr = pAudioSource->get_AudioBeams(&pAudioBeamList);
	}

	if (SUCCEEDED(hr))
	{
		hr = pAudioBeamList->OpenAudioBeam(0, &m_pAudioBeam);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pAudioBeam->OpenInputStream(&m_pAudioStream);
		m_p16BitAudioStream = new KinectAudioStream(m_pAudioStream);
	}

	if (FAILED(hr))
	{
		omsg("MSKinect2Service: Failed opening an audio stream!");
		return hr;
	}

	hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);

	// Audio Format for Speech Processing
	WORD AudioFormat = WAVE_FORMAT_PCM;
	WORD AudioChannels = 1;
	DWORD AudioSamplesPerSecond = 16000;
	DWORD AudioAverageBytesPerSecond = 32000;
	WORD AudioBlockAlign = 2;
	WORD AudioBitsPerSample = 16;

	WAVEFORMATEX wfxOut = { AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0 };

	if (SUCCEEDED(hr))
	{

		m_p16BitAudioStream->SetSpeechState(true);
		hr = m_pSpeechStream->SetBaseStream(m_p16BitAudioStream, SPDFID_WaveFormatEx, &wfxOut);
	}

	if (SUCCEEDED(hr))
	{
		hr = CreateSpeechRecognizer();
	}

	if (FAILED(hr))
	{
		omsg("MSKinect2Service: Could not create speech recognizer. Please ensure that Microsoft Speech SDK and other sample requirements are installed.");
		return hr;
	}

	if (enableKinectSpeechGrammar)
	{
		hr = LoadSpeechGrammar();
		hr = StartSpeechRecognition();

		if (FAILED(hr))
		{
			omsg("MSKinect2Service: Could not start recognizing speech.");
			return hr;
		}

		omsg("MSKinect2Service: Speech recognition started.");
		m_bSpeechActive = true;
	}

	if (enableKinectSpeechDictation)
	{
		hr = LoadSpeechDictation();
	}

    SafeRelease(pAudioBeamList);
    SafeRelease(pAudioSource);
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Create speech recognizer that will read Kinect audio stream data.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSKinectService::CreateSpeechRecognizer()
{
    ISpObjectToken *pEngineToken = NULL;

    HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);

    if (SUCCEEDED(hr))
    {
        m_pSpeechRecognizer->SetInput(m_pSpeechStream, TRUE);

        // If this fails here, you have not installed the acoustic models for Kinect
        hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"Language=409;Kinect=True", NULL, &pEngineToken);

        if (SUCCEEDED(hr))
        {
            m_pSpeechRecognizer->SetRecognizer(pEngineToken);

			// If this fails, make sure both the Speech 11 SDK and redist runtime is installed
            hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);

            // For long recognition sessions (a few hours or more), it may be beneficial to turn off adaptation of the acoustic model. 
            // This will prevent recognition accuracy from degrading over time.
            if (SUCCEEDED(hr))
            {
                hr = m_pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);                
            }
        }
    }
    SafeRelease(pEngineToken);
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Load speech recognition grammar into recognizer.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSKinectService::LoadSpeechGrammar()
{
    HRESULT hr = m_pSpeechContext->CreateGrammar(1, &m_pSpeechGrammar);

    if (SUCCEEDED(hr))
    {
		std::wstring wstr = StringToWString(speechGrammerFilePath);
		LPCWSTR grammarStr = wstr.c_str();

		ofmsg("MSKinect2Service: Loading grammar file '%1%'", %speechGrammerFilePath);
        // Populate recognition grammar from file
        hr = m_pSpeechGrammar->LoadCmdFromFile(grammarStr, SPLO_STATIC);
    }

	if (FAILED(hr))
	{
		omsg("MSKinect2Service: Could not load speech grammar. Please ensure that grammar configuration file was properly deployed.");
	}
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Load speech recognition dictation into recognizer.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSKinectService::LoadSpeechDictation()
{
	HRESULT hr = S_OK;
	CComPtr<ISpRecognizer> cpRecoEngine;
	hr = cpRecoEngine.CoCreateInstance(CLSID_SpInprocRecognizer);

	if (SUCCEEDED(hr))
	{
		hr = cpRecoEngine->CreateRecoContext(&m_pSpeechDictationContext);
	}

	if (SUCCEEDED(hr))
	{
		// This specifies which of the recognition events are going to trigger notifications.
		// Here, all we are interested in is the beginning and ends of sounds, as well as
		// when the engine has recognized something
		const ULONGLONG ullInterest = SPFEI(SPEI_RECOGNITION);
		m_pSpeechDictationContext->SetInterest(ullInterest, ullInterest);
	}

	// create default audio object
	CComPtr<ISpAudio> cpAudio;
	SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &cpAudio);

	// set the input for the engine
	cpRecoEngine->SetInput(cpAudio, TRUE);
	hr = cpRecoEngine->SetRecoState(SPRST_ACTIVE);

	if (SUCCEEDED(hr))
	{
		// Specifies that the grammar we want is a dictation grammar.
		// Initializes the grammar (m_cpDictationGrammar)
		hr = m_pSpeechDictationContext->CreateGrammar(0, &m_cpDictationGrammar);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_cpDictationGrammar->LoadDictation(NULL, SPLO_STATIC);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_cpDictationGrammar->SetDictationState(SPRS_ACTIVE);
	}
	if (FAILED(hr))
	{
		omsg("MSKinect2Service: Could not load dictation grammar.");
		m_cpDictationGrammar->Release();
	}
	else if (SUCCEEDED(hr))
	{
		omsg("MSKinect2Service: Dictation grammar loaded.");
	}

	
	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Start recognizing speech asynchronously.
/// </summary>
/// <returns>
/// <para>S_OK on success, otherwise failure code.</para>
/// </returns>
HRESULT MSKinectService::StartSpeechRecognition()
{
    HRESULT hr = S_OK;

    // Specify that all top level rules in grammar are now active
    hr = m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
    if (FAILED(hr))
    {
        return hr;
    }

    // Specify that engine should always be reading audio
    hr = m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
    if (FAILED(hr))
    {
        return hr;
    }

    // Specify that we're only interested in receiving recognition events
    hr = m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
    if (FAILED(hr))
    {
        return hr;
    }

    // Ensure that engine is recognizing speech and not in paused state
    hr = m_pSpeechContext->Resume(0);
    if (FAILED(hr))
    {
        return hr;
    }

    m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Process recently triggered speech recognition events.
/// </summary>
void MSKinectService::ProcessSpeech()
{
    SPEVENT curEvent = {SPEI_UNDEFINED, SPET_LPARAM_IS_UNDEFINED, 0, 0, 0, 0};
    ULONG fetched = 0;
    HRESULT hr = S_OK;

	if (m_pSpeechContext == NULL)
		return;
    m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
    while (fetched > 0)
    {
        switch (curEvent.eEventId)
        {
        case SPEI_RECOGNITION:
            if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
            {
                // this is an ISpRecoResult
                ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
                SPPHRASE* pPhrase = NULL;

                hr = result->GetPhrase(&pPhrase);
                if (SUCCEEDED(hr))
                {
					if ((pPhrase->pProperties != NULL))
					{
						LPCWSTR speechWString;
						const SPPHRASEPROPERTY* pSemanticTag;
						float speechStringConfidence = 0;
						String speechString = "";

						if (pPhrase->pProperties->pFirstChild != NULL)
						{
							pSemanticTag = pPhrase->pProperties->pFirstChild;
							speechWString = pSemanticTag->pszValue;
							speechStringConfidence = pSemanticTag->SREngineConfidence;

							speechString += WStringToString(speechWString);

							while (pSemanticTag->pNextSibling != NULL)
							{
								pSemanticTag = pSemanticTag->pNextSibling;

								speechWString = pSemanticTag->pszValue;
								speechStringConfidence = pSemanticTag->SREngineConfidence;

								speechString += " " + WStringToString(speechWString);
							}
						}
						else
						{
							speechWString = pPhrase->pProperties->pszValue;
							speechStringConfidence = pPhrase->pProperties->SREngineConfidence;

							speechString += WStringToString(speechWString);
						}

						ofmsg("MSKinect2Service: Speech recognized '%1%' confidence: %2%", %speechString %speechStringConfidence);

						GenerateSpeechEvent( speechString, speechStringConfidence );
                    }
                    ::CoTaskMemFree(pPhrase);
                }
            }
            break;
        }

        m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Convert from LPCWSTR to String.
/// </summary>
String MSKinectService::WStringToString(LPCWSTR speechWString)
{
	String speechString = "";
	// Convert from LPCWSTR to String
	std::wstring wstr = speechWString;

	speechString.resize(wstr.size()); //make enough room in copy for the string 
	std::copy(wstr.begin(), wstr.end(), speechString.begin()); //copy it

	return speechString;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Process recently triggered speech recognition events.
/// </summary>
void MSKinectService::ProcessSpeechDictation()
{
	SPEVENT curEvent = { SPEI_UNDEFINED, SPET_LPARAM_IS_UNDEFINED, 0, 0, 0, 0 };
	ULONG fetched = 0;
	HRESULT hr = S_OK;

	m_pSpeechDictationContext->GetEvents(1, &curEvent, &fetched);
	while (fetched > 0)
	{
		switch (curEvent.eEventId)
		{
		case SPEI_RECOGNITION:
			if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
			{
				// this is an ISpRecoResult
				ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
				SPPHRASE* pPhrase = NULL;

				LPWSTR speechWString = L"";

				hr = result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &speechWString, NULL);
				if (SUCCEEDED(hr))
				{
					std::wstring wstr = speechWString;
					String speechString;
					speechString.resize(wstr.size()); //make enough room in copy for the string 
					std::copy(wstr.begin(), wstr.end(), speechString.begin()); //copy it

					ofmsg("MSKinect2Service: Dictation speech recognized '%1%'", %speechString);
					GenerateSpeechEvent(speechString, 1.0f);
				}
			}
			break;
		}

		m_pSpeechDictationContext->GetEvents(1, &curEvent, &fetched);
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::GenerateSpeechEvent( String speechString, float speechConfidence )
{
	Event* evt = mysInstance->writeHead();
	evt->reset(Event::Update, Service::Speech, 0, 0);

	evt->setPosition( speechConfidence, 0 );
	evt->setExtraDataType(Event::ExtraDataString);

	evt->setExtraDataString(speechString);

	mysInstance->unlockEvents();
}

#endif