/**************************************************************************************************
* THE OMICRON PROJECT
*-------------------------------------------------------------------------------------------------
* Copyright 2010-2014		Electronic Visualization Laboratory, University of Illinois at Chicago
* Authors:										
*  Arthur Nishimoto		anishimoto42@gmail.com
*-------------------------------------------------------------------------------------------------
* Copyright (c) 2010-2014, Electronic Visualization Laboratory, University of Illinois at Chicago
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

// Static initializers
#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
LPCWSTR MSKinectService::GrammarFileName = L"S:/EVL/OmegaLib/omegalib/omegalib/external/omicron/data/kinectSpeech.grxml";
#endif

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
	m_pKinectAudioStream = NULL;
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

	caveSimulator = Config::getBoolValue("caveSimulator", settings, false);
	caveSimulatorHeadID = Config::getIntValue("caveSimulatorHeadID", settings, 0);
	caveSimulatorWandID = Config::getIntValue("caveSimulatorWandID", settings, 1);

	//if( caveSimulator )
	//{
	//	omsg("MSKinectService: CAVE2 tracker simulation mode active!");
	//	ofmsg("   Kinect head will be mapped to mocap ID %1%", %caveSimulatorHeadID);
	//	ofmsg("   Kinect right hand (wand) will be mapped to mocap ID %1%", %caveSimulatorWandID);
	//}


	//GrammarFileName = (LPCWSTR)Config::getStringValue("speechGrammerFileName", settings, "kinectSpeech.grxml").c_str();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::initialize() 
{
	mysInstance = this;

	InitializeDefaultKinect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::poll()
{
	omsg(" MSKinectService::poll");
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
			ProcessBody(nTime, BODY_COUNT, ppBodies);
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}

	SafeRelease(pBodyFrame);
	omsg(" MSKinectService::poll end");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::dispose() 
{
	omsg("MSKinectService: Shutting down.");

	if (kinectSensor)
	{
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

		//if (SUCCEEDED(hr))
		//{
		//    hr = kinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		//}

		if (SUCCEEDED(hr))
		{
			hr = kinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&bodyFrameReader);
		}

		SafeRelease(pBodyFrameSource);
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
HRESULT MSKinectService::InitializeKinect()
{
	int sensorID = -1;
	HRESULT  hr;

	InitializeDefaultKinect();

#ifdef OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO
	// Audio/Speech Recognition
	hr = InitializeAudioStream();
	if (FAILED(hr))
	{
		printf("MSKinectService: Kinect %d could not initialize audio stream. \n", sensorID );
		return hr;
	}

	hr = CreateSpeechRecognizer();
	if (FAILED(hr))
	{
		printf("MSKinectService: Kinect %d could not create speech recognizer. Please ensure that Microsoft Speech SDK and other sample requirements are installed. \n", sensorID );
		return hr;
	}

	hr = LoadSpeechGrammar();
	if (FAILED(hr))
	{
		printf("MSKinectService: Kinect %d could not load speech grammar. Please ensure that grammar configuration file was properly deployed. \n", sensorID );
		return hr;
	}

	hr = StartSpeechRecognition();
	if (FAILED(hr))
	{
		printf("MSKinectService: Kinect %d could not start recognizing speech. \n", sensorID );
		return hr;
	}
#endif
	return hr;
}

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
void MSKinectService::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies)
{
	HRESULT hr;

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
				D2D1_POINT_2F jointPoints[JointType_Count];
				HandState leftHandState = HandState_Unknown;
				HandState rightHandState = HandState_Unknown;

				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				hr = pBody->GetJoints(_countof(joints), joints);


				if (SUCCEEDED(hr))
				{
					GenerateMocapEvent( pBody, kinectSensor );
					//for (int j = 0; j < _countof(joints); ++j)
					//{
					//jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
					//}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::GenerateMocapEvent( IBody* body, IKinectSensor* currentSensor)
{      
	HRESULT hr;

	UINT64 skeletonID;
	body->get_TrackingId(&skeletonID);

	Joint joints[JointType_Count]; 

	Vector3f headPos;

	hr = body->GetJoints(_countof(joints), joints);
	if (SUCCEEDED(hr))
	{
		Joint head = joints[JointType_Head];
		headPos = Vector3f( head.Position.X, head.Position.Y, head.Position.Z );

		if( debugInfo )
			ofmsg( "Kinect Head %1% (%2%,%3%,%4%)",  %skeletonID %headPos.x() %headPos.y() %headPos.z() );
	}
	/*
	if( caveSimulator )
	{
	Event* evt = mysInstance->writeHead();
	evt->reset(Event::Update, Service::Mocap, caveSimulatorHeadID);

	Vector4 jointPos = skel.SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
	Vector3f pos;
	pos[0] = jointPos.x;
	pos[1] = jointPos.y + 1.8f;
	pos[2] = jointPos.z;
	evt->setPosition( pos );
	evt->setOrientation( Quaternion::Identity() );

	mysInstance->unlockEvents();

	Event* evt2 = mysInstance->writeHead();
	evt2->reset(Event::Update, Service::Mocap, caveSimulatorWandID);

	Vector4 jointLPos = skel.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT];
	Vector4 jointRPos = skel.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT];

	Vector3f pos2;
	// Check for the closest hand
	if( jointLPos.z < jointRPos.z )
	{
	pos2[0] = jointLPos.x;
	pos2[1] = jointLPos.y + 1.8f;
	pos2[2] = jointLPos.z;
	}
	else
	{
	pos2[0] = jointRPos.x;
	pos2[1] = jointRPos.y + 1.8f;
	pos2[2] = jointRPos.z;
	}

	evt2->setPosition( pos2 );
	evt->setOrientation( Quaternion::Identity() );

	mysInstance->unlockEvents();

	}

	Event* evt = mysInstance->writeHead();
	evt->reset(Event::Update, Service::Mocap, skeletonID, kinectID);

	Vector4 jointPos = skel.SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
	Vector3f pos;
	pos[0] = jointPos.x;
	pos[1] = jointPos.y;
	pos[2] = jointPos.z;
	evt->setPosition( pos );

	evt->setExtraDataType(Event::ExtraDataVector3Array);

	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_SPINE, NUI_SKELETON_POSITION_SPINE );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_CENTER );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_HEAD, NUI_SKELETON_POSITION_HEAD );

	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_SHOULDER, NUI_SKELETON_POSITION_SHOULDER_LEFT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_ELBOW, NUI_SKELETON_POSITION_ELBOW_LEFT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_WRIST, NUI_SKELETON_POSITION_WRIST_LEFT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_HAND, NUI_SKELETON_POSITION_HAND_LEFT );

	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_SHOULDER, NUI_SKELETON_POSITION_SHOULDER_RIGHT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_ELBOW, NUI_SKELETON_POSITION_ELBOW_RIGHT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_WRIST, NUI_SKELETON_POSITION_WRIST_RIGHT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_HAND, NUI_SKELETON_POSITION_HAND_RIGHT );

	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_HIP, NUI_SKELETON_POSITION_HIP_LEFT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_KNEE, NUI_SKELETON_POSITION_KNEE_LEFT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_ANKLE, NUI_SKELETON_POSITION_ANKLE_LEFT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_LEFT_FOOT, NUI_SKELETON_POSITION_FOOT_LEFT );

	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_HIP, NUI_SKELETON_POSITION_HIP_RIGHT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_KNEE, NUI_SKELETON_POSITION_KNEE_RIGHT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_ANKLE, NUI_SKELETON_POSITION_ANKLE_RIGHT );
	SkeletonPositionToEvent( skel, evt, Event::OMICRON_SKEL_RIGHT_FOOT, NUI_SKELETON_POSITION_FOOT_RIGHT );

	mysInstance->unlockEvents();
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::SkeletonPositionToEvent( const Joint & skel, Event* evt, Event::OmicronSkeletonJoint joint, JointType posIndex )
{
	//Vector4 jointPos = skel.SkeletonPositions[posIndex];
	//Vector3f pos;
	//pos[0] = jointPos.x;
	//pos[1] = jointPos.y;
	//pos[2] = jointPos.z;
	//evt->setExtraDataVector3(joint, pos);
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
	INuiAudioBeam*      pNuiAudioSource = NULL;
	IMediaObject*       pDMO = NULL;
	IPropertyStore*     pPropertyStore = NULL;
	IStream*            pStream = NULL;

	// Get the audio source
	HRESULT hr = m_pNuiSensor->NuiGetAudioSource(&pNuiAudioSource);
	if (SUCCEEDED(hr))
	{
		hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&pDMO);

		if (SUCCEEDED(hr))
		{
			hr = pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore);

			// Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
			// Possible values are:
			//   SINGLE_CHANNEL_AEC = 0
			//   OPTIBEAM_ARRAY_ONLY = 2
			//   OPTIBEAM_ARRAY_AND_AEC = 4
			//   SINGLE_CHANNEL_NSAGC = 5
			PROPVARIANT pvSysMode;
			PropVariantInit(&pvSysMode);
			pvSysMode.vt = VT_I4;
			pvSysMode.lVal = (LONG)(2); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
			pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
			PropVariantClear(&pvSysMode);

			// Set DMO output format
			WAVEFORMATEX wfxOut = {AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0};
			DMO_MEDIA_TYPE mt = {0};
			MoInitMediaType(&mt, sizeof(WAVEFORMATEX));

			mt.majortype = MEDIATYPE_Audio;
			mt.subtype = MEDIASUBTYPE_PCM;
			mt.lSampleSize = 0;
			mt.bFixedSizeSamples = TRUE;
			mt.bTemporalCompression = FALSE;
			mt.formattype = FORMAT_WaveFormatEx;	
			memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

			hr = pDMO->SetOutputType(0, &mt, 0);

			if (SUCCEEDED(hr))
			{
				m_pKinectAudioStream = new KinectAudioStream(pDMO);

				hr = m_pKinectAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

				if (SUCCEEDED(hr))
				{
					if (FAILED(::CoInitialize(NULL)))
						return FALSE;

					hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);

					if (SUCCEEDED(hr))
					{
						hr = m_pSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &wfxOut);
					}
				}
			}

			MoFreeMediaType(&mt);
		}
	}

	SafeRelease(pStream);
	SafeRelease(pPropertyStore);
	SafeRelease(pDMO);
	SafeRelease(pNuiAudioSource);

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
		m_pSpeechRecognizer->SetInput(m_pSpeechStream, FALSE);
		hr = SpFindBestToken(SPCAT_RECOGNIZERS,L"Language=409;Kinect=True",NULL,&pEngineToken);

		if (SUCCEEDED(hr))
		{
			m_pSpeechRecognizer->SetRecognizer(pEngineToken);
			hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
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
		// Populate recognition grammar from file
		hr = m_pSpeechGrammar->LoadCmdFromFile(GrammarFileName, SPLO_STATIC);
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
	HRESULT hr = m_pKinectAudioStream->StartCapture();

	if (SUCCEEDED(hr))
	{
		// Specify that all top level rules in grammar are now active
		m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

		// Specify that engine should always be reading audio
		m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

		// Specify that we're only interested in receiving recognition events
		m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

		// Ensure that engine is recognizing speech and not in paused state
		hr = m_pSpeechContext->Resume(0);
		if (SUCCEEDED(hr))
		{
			m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
		}
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Process recently triggered speech recognition events.
/// </summary>
void MSKinectService::ProcessSpeech()
{
	const float ConfidenceThreshold = 0.3f;

	SPEVENT curEvent;
	ULONG fetched = 0;
	HRESULT hr = S_OK;

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
					if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
					{
						const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
						if (pSemanticTag->SREngineConfidence > ConfidenceThreshold)
						{
							//TurtleAction action = MapSpeechTagToAction(pSemanticTag->pszValue);
							//m_pTurtleController->DoAction(action);
						}
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

#endif