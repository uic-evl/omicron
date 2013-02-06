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
#include "omicron/MSKinectService.h"
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
	m_pNuiSensor = NULL;
	m_instanceId = NULL;
	m_TrackedSkeletons = 0;
	m_hNextSkeletonEvent = NULL;
	skeletonEngineKinectID = -1;
	m_SkeletonTrackingFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE;

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

	//GrammarFileName = (LPCWSTR)Config::getStringValue("speechGrammerFileName", settings, "kinectSpeech.grxml").c_str();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::initialize() 
{
	mysInstance = this;
	
	NuiSetDeviceStatusCallback( &MSKinectService::Nui_StatusProcThunk, mysInstance );
	
	int iSensorCount = 0;
    HRESULT hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
        return;

	INuiSensor* sensor;

	ofmsg("MSKinectService: %1% Kinect(s) detected.", %iSensorCount );

	// Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &sensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = sensor->NuiStatus();
        if (S_OK == hr)
        {
            InitializeKinect( sensor->NuiDeviceConnectionId() );
        }
		else
		{
			// This sensor wasn't OK, so release it since we're not using it
			sensor->Release();
		}
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::poll()
{
	std::map<String,INuiSensor*>::iterator it;
	for ( it = sensorList.begin(); it != sensorList.end(); it++ )
	{
		INuiSensor* sensor = it->second;

		// Wait for 0ms, just quickly test if it is time to process a skeleton
		if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0) )
		{
			ProcessSkeleton(sensor);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::dispose() 
{
    omsg("MSKinectService: Shutting down.");

	if (m_pNuiSensor)
    {
        m_pNuiSensor->NuiShutdown();
    }

    if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
    {
        CloseHandle(m_hNextSkeletonEvent);
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
    if( SUCCEEDED(hrStatus) )
    {
        if ( S_OK == hrStatus )
        {
            InitializeKinect(instanceName);
			m_pNuiSensor = NULL;
        }
    }
    else
    {
        UnInitializeKinect(instanceName);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT MSKinectService::InitializeKinect( const OLECHAR *instanceName )
{
	// Generic creation failure
    if ( NULL == instanceName )
    {
        //MessageBoxResource( IDS_ERROR_NUICREATE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }

    HRESULT hr = NuiCreateSensorById( instanceName, &m_pNuiSensor );
    
    // Generic creation failure
    if ( FAILED(hr) )
    {
        //MessageBoxResource( IDS_ERROR_NUICREATE, MB_OK | MB_ICONHAND );
        return hr;
    }
    return InitializeKinect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT MSKinectService::InitializeKinect()
{
	int sensorID = -1;
    HRESULT  hr;
    if ( !m_pNuiSensor )
    {
        hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

        if ( FAILED(hr) )
        {
            return hr;
        }
    }
	
	SysFreeString(m_instanceId);
	m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
	std::wstring deviceWStr = m_pNuiSensor->NuiDeviceConnectionId();
	std::string deviceName( deviceWStr.begin(), deviceWStr.end() );
	
	sensorID = getKinectID(deviceName);
	printf("MSKinectService: Kinect %d (%s) connected. \n", sensorID, deviceName.c_str() );
	sensorList[deviceName] = m_pNuiSensor;
	sensorIndexList[deviceName] = sensorID;
	
    //m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    //m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( m_hNextSkeletonEvent == NULL )
		m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    // reset the tracked skeletons, range, and tracking mode
    //SendDlgItemMessage(m_hWnd, IDC_TRACKEDSKELETONS, CB_SETCURSEL, 0, 0);
    //SendDlgItemMessage(m_hWnd, IDC_TRACKINGMODE, CB_SETCURSEL, 0, 0);
    //SendDlgItemMessage(m_hWnd, IDC_RANGE, CB_SETCURSEL, 0, 0);

	/*bool result;
    m_pDrawDepth = new DrawDevice( );
    result = m_pDrawDepth->Initialize( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ), m_pD2DFactory, 320, 240, 320 * 4 );
    if ( !result )
    {
        //MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
    m_pDrawColor = new DrawDevice( );
    result = m_pDrawColor->Initialize( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
    */
    //DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_SKELETON; // | NUI_INITIALIZE_FLAG_USES_AUDIO;

    hr = m_pNuiSensor->NuiInitialize(nuiFlags);
    if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
    {
		printf("MSKinectService: Kinect %d cannot start skeleton engine - already in use by Kinect %d. \n", sensorID, skeletonEngineKinectID );
        nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
        hr = m_pNuiSensor->NuiInitialize(nuiFlags) ;
    }

    if ( FAILED( hr ) )
    {
        if ( E_NUI_DEVICE_IN_USE == hr )
        {
            printf("MSKinectService: Kinect %d cannot start - already in use. \n", sensorID );
        }
        else
        {
            //MessageBoxResource( IDS_ERROR_NUIINIT, MB_OK | MB_ICONHAND );
        }
        return hr;
    }
	
	
    if ( HasSkeletalEngine( m_pNuiSensor ) )
    {
		hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, m_bSeatedMode ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0);
        if( FAILED( hr ) )
        {
            //MessageBoxResource( IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND );
			printf("MSKinectService: Kinect %d error while enabling skeleton tracking. \n", sensorID );
            return hr;
        }
		else
		{
			skeletonEngineKinectID = sensorID;
			if( m_bSeatedMode )
				printf("MSKinectService: Kinect %d seated skeleton tracking enabled. \n", sensorID );
			else
				printf("MSKinectService: Kinect %d default skeleton tracking enabled. \n", sensorID );
		}
    }
	/*
    hr = m_pNuiSensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextColorFrameEvent,
        &m_pVideoStreamHandle );

    if ( FAILED( hr ) )
    {
        MessageBoxResource( IDS_ERROR_VIDEOSTREAM, MB_OK | MB_ICONHAND );
        return hr;
    }
	
    hr = m_pNuiSensor->NuiImageStreamOpen(
        HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_320x240,
        m_DepthStreamFlags,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );

    if ( FAILED( hr ) )
    {
        //MessageBoxResource(IDS_ERROR_DEPTHSTREAM, MB_OK | MB_ICONHAND);
        return hr;
    }

    // Start the Nui processing thread
    //m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    //m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );
	*/
	
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

	/*
    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop )
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
    }
	*/
	
	if ( sensorList.count(deviceName) == 1 )
    {
		INuiSensor* sensor = sensorList[deviceName];
		
		if ( HasSkeletalEngine(sensor) && m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
		{
			printf("MSKinectService: Kinect %d skeleton engine shutting down. \n", getKinectID(deviceName) );
			CloseHandle( m_hNextSkeletonEvent );
			m_hNextSkeletonEvent = NULL;
		}

		sensorList.erase(deviceName);
		sensorIndexList.erase(deviceName);
        sensor->NuiShutdown( );
		sensor->Release();
		printf("MSKinectService: Kinect %d disconnected. \n", getKinectID(deviceName) );
    }
	else
	{
		printf("MSKinectService: Attempted to disconnect non-connected sensor: %s. \n", deviceName.c_str() );
	}
    /*
    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if ( m_hNextColorFrameEvent && ( m_hNextColorFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextColorFrameEvent );
        m_hNextColorFrameEvent = NULL;
    }
	*/
    //SafeRelease( m_pNuiSensor );
    /*
    // clean up Direct2D graphics
    delete m_pDrawDepth;
    m_pDrawDepth = NULL;

    delete m_pDrawColor;
    m_pDrawColor = NULL;

    DiscardDirect2DResources();*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::ProcessSkeleton( INuiSensor* sensor )
{
    NUI_SKELETON_FRAME skeletonFrame = {0};

    HRESULT hr = sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
    if ( FAILED(hr) )
    {
        return;
    }

    // smooth out the skeleton data
    sensor->NuiTransformSmooth(&skeletonFrame, NULL);

    // Endure Direct2D is ready to draw
    /*hr = EnsureDirect2DResources( );
    if ( FAILED(hr) )
    {
        return;
    }

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear( );
    */
    RECT rct;
    GetClientRect( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), &rct);
    int width = rct.right;
    int height = rct.bottom;

    for (int i = 0 ; i < NUI_SKELETON_COUNT; ++i)
    {
        NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;

        if (NUI_SKELETON_TRACKED == trackingState)
        {
            // We're tracking the skeleton, draw it
            GenerateMocapEvent(skeletonFrame.SkeletonData[i], sensor );
        }
        else if (NUI_SKELETON_POSITION_ONLY == trackingState)
        {
            // we've only received the center point of the skeleton, draw that
            /*D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                SkeletonToScreen(skeletonFrame.SkeletonData[i].Position, width, height),
                g_JointThickness,
                g_JointThickness
                );

            m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);*/
        }
    }

    //hr = m_pRenderTarget->EndDraw();

    // Device lost, need to recreate the render target
    // We'll dispose it now and retry drawing
    /*if (D2DERR_RECREATE_TARGET == hr)
    {
        hr = S_OK;
        DiscardDirect2DResources();
    }*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::GenerateMocapEvent(const NUI_SKELETON_DATA & skel, INuiSensor* currentSensor)
{      
	
	//printf( " %f %f %f \n", headPos.x, headPos.y, headPos.z );

	int kinectID = currentSensor->NuiInstanceIndex();
	int skeletonID = skel.dwTrackingID;

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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MSKinectService::SkeletonPositionToEvent( const NUI_SKELETON_DATA & skel, Event* evt, Event::OmicronSkeletonJoint joint, _NUI_SKELETON_POSITION_INDEX posIndex )
{
	Vector4 jointPos = skel.SkeletonPositions[posIndex];
	Vector3f pos;
	pos[0] = jointPos.x;
	pos[1] = jointPos.y;
	pos[2] = jointPos.z;
	evt->setExtraDataVector3(joint, pos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Invoked when the user changes the selection of tracked skeletons
/// </summary>
/// <param name="mode">skelton tracking mode to switch to</param>
void MSKinectService::UpdateTrackedSkeletonSelection( int mode )
{
    m_TrackedSkeletons = mode;

    UpdateSkeletonTrackingFlag(
        NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS,
        (mode != SV_TRACKED_SKELETONS_DEFAULT));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Invoked when the user changes the tracking mode
/// </summary>
/// <param name="mode">tracking mode to switch to</param>
void MSKinectService::UpdateTrackingMode( int mode )
{
    UpdateSkeletonTrackingFlag(
        NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT,
        (mode == TRACKING_MODE_SEATED) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Invoked when the user changes the range
/// </summary>
/// <param name="mode">range to switch to</param>
void MSKinectService::UpdateRange( int mode )
{
    //UpdateDepthStreamFlag(
    //    NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE,
    //    (mode != RANGE_DEFAULT) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Sets or clears the specified skeleton tracking flag
/// </summary>
/// <param name="flag">flag to set or clear</param>
/// <param name="value">true to set, false to clear</param>
void MSKinectService::UpdateSkeletonTrackingFlag( DWORD flag, bool value )
{
    DWORD newFlags = m_SkeletonTrackingFlags;

    if (value)
    {
        newFlags |= flag;
    }
    else
    {
        newFlags &= ~flag;
    }

    if (NULL != m_pNuiSensor && newFlags != m_SkeletonTrackingFlags)
    {
        if ( !HasSkeletalEngine(m_pNuiSensor) )
        {
            //MessageBoxResource(IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND);
        }

        m_SkeletonTrackingFlags = newFlags;

        HRESULT hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, m_SkeletonTrackingFlags );

        if ( FAILED( hr ) )
        {
            //MessageBoxResource(IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND);
        }
    }
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