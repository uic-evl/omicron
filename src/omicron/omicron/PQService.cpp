/********************************************************************************************************************** 
* THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2019							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto								anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2019, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omicron/PQService.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PQService* PQService::mysInstance = NULL;
int PQService::maxBlobSize = 1000;
int PQService::maxTouches = 1000; // Number of IDs assigned before resetting. Should match touchID array initialization
int PQService::move_threshold = 1; // pixels
bool PQService::normalizeData = true;
bool PQService::useGestureManager = false;
bool PQService::showStreamSpeed = false;
int PQService::lastIncomingEventTime = 0;
int PQService::eventCount = 0;
Vector2i PQService::serverResolution = Vector2i(1920,1080);
Vector2i PQService::touchOffset = Vector2i(0, 0);
Vector2i PQService::rawDataResolution = Vector2i(1920,1080);
bool PQService::hasCustomRawDataResolution = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::setup(Setting& settings)
{
	if(settings.exists("serverIP"))
	{
		server_ip =  (const char*)settings["serverIP"];
	}
	if(settings.exists("maxBlobSize"))
	{
		maxBlobSize =  settings["maxBlobSize"];
	}

	if(settings.exists("moveThreshold"))
	{
		move_threshold =  settings["moveThreshold"];
		ofmsg("PQService: move threshold set to %1%", %move_threshold);
	}
	if(settings.exists("useGestureManager"))
	{
		useGestureManager = settings["useGestureManager"];
		if( useGestureManager )
		{
			omsg("PQService: Gesture Manager Enabled");
			touchGestureManager = new TouchGestureManager();
			touchGestureManager->setup(settings);
		}
	}

	if(settings.exists("normalizeData"))
	{
		normalizeData = settings["normalizeData"];
		if( normalizeData )
			omsg("PQService: Normalizing data");
	}

	debugRawPQInfo = Config::getBoolValue("debugRawPQInfo", settings, false);
	showStreamSpeed = Config::getBoolValue("showStreamSpeed", settings, false);

	touchOffset = Config::getVector2iValue("touchOffset", settings, Vector2i(0, 0));

	if(settings.exists("rawDataResolution"))
	{
		hasCustomRawDataResolution = true;
		rawDataResolution = Config::getVector2iValue("rawDataResolution", settings, Vector2i(1920,1080) );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::initialize( ) 
{
	mysInstance = this;
	init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::initialize( char* local_ip ) 
{
	mysInstance = this;
	server_ip = local_ip;
	init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::poll() 
{
	if( useGestureManager )
	{
		touchGestureManager->poll();
	}
	
#ifndef OMICRON_OS_WIN
	// Check for response to query
	if (tuioMsgSocket.receiveNextPacket(1)) {// Paramater is timeout in milliseconds. -1 (default) will wait indefinatly 
		PacketReader pr(tuioMsgSocket.packetData(), tuioMsgSocket.packetSize());
		Message *incoming_msg;

		while (pr.isOk() && (incoming_msg = pr.popMessage()) != 0) {
			// ofmsg("OSC In: %1%", %incoming_msg->addressPattern());
			if (incoming_msg->addressPattern() == "/tuio/2Dcur")
			{
				bool MSG_SET = false;
				bool MSG_ALIVE = false;

				int curSetArg = 0;
				int id = 0;
				float x = 0;
				float y = 0;
				
				std::vector<int> aliveIDs;
				
				Message::ArgReader arg(incoming_msg->arg());
				while (arg.nbArgRemaining())
				{
					if (arg.isBlob()) {
						std::vector<char> b; arg.popBlob(b);
						//omsg("  received Blob");
					}
					else if (arg.isBool()) {
						bool b; arg.popBool(b);
						//ofmsg( "  received Bool %1%", %b );
					}
					else if (arg.isInt32()) {
						int i; arg.popInt32(i);

						if (MSG_SET)
						{
							id = i;
						}
						else if (MSG_ALIVE)
						{
							if (aliveState[i] == 0)
							{
								aliveState[i] = 1;
							}
							aliveIDs.push_back(i);
						}
						//ofmsg( "  received Int32 %1%", %i );
					}
					else if (arg.isInt64()) {
						int64_t h; arg.popInt64(h);
						//ofmsg( "  received Int64 %1%", %h );
					}
					else if (arg.isFloat()) {
						float f; arg.popFloat(f);
						switch (curSetArg)
						{
							case(2): x = f; break;
							case(3): y = f; break;
						}
						//ofmsg( "  received Float %1%", %f );
					}
					else if (arg.isDouble()) {
						double d; arg.popDouble(d);
						//ofmsg( "  received Double %1%", %d );
					}
					else if (arg.isStr()) {
						std::string s; arg.popStr(s);
						//ofmsg( "  received Str %1%", %s );
						if (s == "set")
						{
							MSG_SET = true;
						}
						else if (s == "alive")
						{
							MSG_ALIVE = true;
							aliveIDs.clear();
						}
					}
					curSetArg++;
				}

				if (MSG_SET)
				{
					TouchPoint tp;
					tp.point_event = 0;
					tp.id = id;
					tp.x = x * serverResolution[0];
					tp.y = y * serverResolution[1];
					tp.dx = 10;
					tp.dy = 10;

					if (aliveState[id] == 1)
					{
						tp.point_event = 0;
						aliveState[id] = 2;
					}
					else if (aliveState[id] == 2)
					{
						tp.point_event = 1;
					}
					
					OnTouchPoint(tp);
				}
				else if (MSG_ALIVE)
				{
					map<int, int>::iterator it;

					for ( it = aliveState.begin(); it != aliveState.end(); it++ )
					{
						int id = it->first;
						int state = it->second;
						bool stillAlive = false;
						if (state == 2)
						{
							for (int i = 0; i < aliveIDs.size(); i++)
							{
								if (aliveIDs.at(i) == id)
								{
									stillAlive = true;
									break;
								}
							}
							
							if( !stillAlive)
							{
								aliveState[id] = 0;
								
								Touch touch = touchlist[touchID[id]];
								
								TouchPoint tp;
								tp.point_event = 2;
								tp.id = id;
								tp.x = touch.xPos * serverResolution[0];
								tp.y = touch.yPos * serverResolution[1];
								tp.dx = 10;
								tp.dy = 10;
								
								OnTouchPoint(tp);
							}
						}
					}
				}
			}
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PQService::init()
{
	nextID = 0;
	lastIncomingEventTime = 0;
	eventCount = 0;

	for(int i = 0; i < maxTouches; i++){
		touchID[i] = 0;
	}

	int err_code = PQMTE_SUCCESS;

	// initialize the handle functions of gestures;
	if( useGestureManager ){
		touchGestureManager->registerPQService(mysInstance);
	}

#ifdef OMICRON_OS_WIN
	// set the functions on server callback
	SetFuncsOnReceiveProc();

	// connect server
	ofmsg("PQService: connecting to server on %1%...", %server_ip);
	if((err_code = ConnectServer(server_ip)) != PQMTE_SUCCESS){
		ofmsg("PQService: connect to server failed, socket error code: %1%", %err_code);
		return err_code;
	}

	// send request to server
	TouchClientRequest tcq = {0};
	tcq.type = RQST_RAWDATA_ALL | RQST_GESTURE_ALL;
	if((err_code = SendRequest(tcq)) != PQMTE_SUCCESS){
		ofmsg("PQService: request to server failed, socket error code: %1%", %err_code);
		return err_code;
	}

	//////////////you can set the move_threshold when the tcq.getType() is RQST_RAWDATA_INSIDE;
	//send threshold
	if((err_code = SendThreshold(move_threshold)) != PQMTE_SUCCESS){
		ofmsg("PQService: set threshold failed, socket error code: %1%", %err_code);
		return err_code;
	}
	
	////////////////////////
	//set raw data resolution
	if( hasCustomRawDataResolution )
		if((err_code = SetRawDataResolution(rawDataResolution[0], rawDataResolution[1])) != PQMTE_SUCCESS){
			ofmsg("PQService: set raw data resolution, socket error code: %1%", %err_code);
			return err_code;
		};

	////////////////////////
	//get server resolution
	if((err_code = GetServerResolution(OnGetServerResolution, NULL)) != PQMTE_SUCCESS){
		printf("PQService: get server resolution failed,error code: %d\n", err_code);
		return err_code;
	};
	//
	// start receiving
	printf("PQService: send request succeeded, start recv.\n");;
#else
	int TUIO_Port = 3333;

	

	tuioMsgSocket.connectTo(server_ip, TUIO_Port);
	tuioMsgSocket.bindTo(TUIO_Port);
	
	if (!tuioMsgSocket.isOk()) {
		ofmsg( "PQService: connect to server failed: %1%", %tuioMsgSocket.errorMessage() );
	} else {
		ofmsg("PQService: connected to server on %1% using TUIO port %2%...", %server_ip %TUIO_Port);
	}
#endif

	printf("PQService: Maximum blob size set to %i pixels \n", maxBlobSize);
	return err_code;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::SetFuncsOnReceiveProc()
{
#ifdef OMICRON_OS_WIN
	PFuncOnReceivePointFrame old_rf_func = SetOnReceivePointFrame(&PQService::OnReceivePointFrame,this);
	//PFuncOnReceiveGesture old_rg_func = SetOnReceiveGesture(&PQService::OnReceiveGesture,this);
	PFuncOnServerBreak old_svr_break = SetOnServerBreak(&PQService::OnServerBreak,NULL);
	PFuncOnReceiveError old_rcv_err_func = SetOnReceiveError(&PQService::OnReceiveError,NULL);
	PFuncOnGetDeviceInfo old_gdi_func = SetOnGetDeviceInfo(&PQService::OnGetDeviceInfo,NULL);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::OnGetDeviceInfo(const TouchDeviceInfo & deviceinfo,void *call_back_object)
{
	ofmsg("PQService: Touch screen Serial Number: %1%,(%2%,%3%)", %deviceinfo.serial_number %deviceinfo.screen_width %deviceinfo.screen_height );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::OnReceivePointFrame(int frame_id, int time_stamp, int moving_point_count, const TouchPoint * moving_point_array, void * call_back_object)
{
	if( showStreamSpeed )
	{
		if( (time_stamp - lastIncomingEventTime) >= 100 )
		{
			lastIncomingEventTime = time_stamp;
			ofmsg("PQService: Incoming event stream %1% event(s)/sec", %eventCount );
			eventCount = 0;
		}
		else
		{
			eventCount++;
		}
	}

	PQService * pqService = static_cast<PQService*>(call_back_object);
	
	// Hack: For some reason the registered callback is now losing the pointer to PQService after being set
	// by SetFuncsOnReceiveProc(). Broken sometime before April 2012 - Arthur.
	pqService = mysInstance;

	assert(pqService != NULL);
	const char * tp_event[] = 
	{
		"down",
		"move",
		"up",
	};

	//printf(" frame_id:" << frame_id << " time:"  << time_stamp << " ms" << " moving point count:" << moving_point_count << endl;
	for(int i = 0; i < moving_point_count; ++ i){
		TouchPoint tp = moving_point_array[i];
		pqService->OnTouchPoint(tp);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::OnServerBreak(void * param, void * call_back_object)
{
	// when the server break, disconenct server;
	omsg("PQService: server disconnected");
#ifdef OMICRON_OS_WIN
	DisconnectServer();
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::OnReceiveError(int err_code, void * call_back_object)
{
	switch(err_code)
	{
	case PQMTE_RCV_INVALIDATE_DATA:
		printf("PQService: error: receive invalidate data.\n");;
		break;
	case PQMTE_SERVER_VERSION_OLD:
		printf("PQService: error: the multi-touch server is old for this client, please update the multi-touch server.\n");;
		break;
	default:
		printf("PQService: socket error, socket error code:%d\n", err_code);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::OnGetServerResolution(int x, int y, void * call_back_object)
{
	serverResolution[0] = x;
	serverResolution[1] = y;

	ofmsg("PQService: server resolution: %1%,%2%", %x %y );
	if( rawDataResolution[0] != 1920 && rawDataResolution[1] != 1080 )
	{
		ofmsg("PQService: raw data resolution set at: %1%,%2%", %rawDataResolution[0] %rawDataResolution[1] );
		serverResolution[0] = rawDataResolution[0];
		serverResolution[1] = rawDataResolution[1];
	}
	if( normalizeData )
	{
		omsg("PQService: normalizing data");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// here, just record the position of point,
//	you can do mouse map like "OnTG_Down" etc;
void PQService::OnTouchPoint(const TouchPoint & tp)
{
	timeb tb;
	ftime( &tb );
	int timestamp = tb.millitm + (tb.time & 0xfffff) * 1000;

	int tEvent = tp.point_event;
	int xWidth = tp.dx;
	int yWidth = tp.dy;

	

	if(mysInstance && xWidth <= maxBlobSize && yWidth <= maxBlobSize)
	{		
		Touch touch;
		
		// PQService management of IDs
		// Basically PQ IDs recycle after the ID is done. OmegaLib increments IDs
		// until max ID is reached.
		touch.ID = touchID[tp.id];
		touch.groupID = touch.ID;
		if( debugRawPQInfo )
		{
			ofmsg("PQService: Incoming touch point ID: %1% type: %6% at (%2%,%3%) size: (%4%,%5%)", %touch.ID %tp.x %tp.y %tp.dx %tp.dy %tp.point_event);
		}

		touch.xPos = (tp.x + touchOffset[0]) / (float)serverResolution[0];
		touch.yPos = (tp.y + touchOffset[1]) / (float)serverResolution[1];
		touch.xWidth = tp.dx / (float)serverResolution[0];
		touch.yWidth = tp.dy / (float)serverResolution[1];

		touch.timestamp = timestamp;

		if (tp.point_event == TP_DOWN)
		{
			touch.ID = nextID;
			touch.groupID = touch.ID;
			touchID[tp.id] = nextID;
			if (nextID < maxTouches - 100) {
				nextID++;
			}
			else {
				nextID = 0;
			}
		}

		// Process touch gestures (this is done outside event creation
		// for the case touchGestureManager needs to create an event)
		if( useGestureManager ){
			switch(tp.point_event)
			{
				case TP_DOWN:
					touchGestureManager->addTouch( Event::Down, touch );
					break;
				case TP_MOVE:
					touchGestureManager->addTouch( Event::Move, touch );
					break;
				case TP_UP:
					touchGestureManager->addTouch( Event::Up, touch );
					break;
			}
			
		}
		mysInstance->lockEvents();

		Event* evt = mysInstance->writeHead();
		switch(tp.point_event)
		{
			case TP_DOWN:
				evt->reset(Event::Down, Service::Pointer, touch.ID);
				if (isDebugEnabled())
				{
					ofmsg("PQService: Touch ID: %1% as DOWN event at (%2%,%3%) size: (%4%,%5%)", %touch.ID %touch.xPos %touch.yPos %touch.xWidth %touch.yWidth);
				}
				break;
			case TP_MOVE:
				evt->reset(Event::Move, Service::Pointer, touch.ID);
				touchlist[touch.ID] = touch;
				if (isDebugEnabled())
				{
					ofmsg("PQService: Touch ID: %1% as MOVE event at (%2%,%3%) size: (%4%,%5%)", %touch.ID %touch.xPos %touch.yPos %touch.xWidth %touch.yWidth);
				}
				break;
			case TP_UP:
				evt->reset(Event::Up, Service::Pointer, touch.ID);
				touchlist.erase( touch.ID );
				if (isDebugEnabled())
				{
					ofmsg("PQService: Touch ID: %1% as UP event at (%2%,%3%) size: (%4%,%5%)", %touch.ID %touch.xPos %touch.yPos %touch.xWidth %touch.yWidth);
				}
				break;
		}

		evt->setPosition(touch.xPos, touch.yPos);

		evt->setExtraDataType(Event::ExtraDataFloatArray);
		evt->setExtraDataFloat(0, touch.xWidth);
		evt->setExtraDataFloat(1, touch.yWidth);

		//printf(" Server %d,%d Screen %d, %d\n", serverX, serverY, screenX, screenY );
		//printf("      at %d,%d\n", tp.x, tp.y);
		mysInstance->unlockEvents();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::dispose() 
{
	mysInstance = NULL;
#ifdef OMICRON_OS_WIN
	DisconnectServer();
#endif
}

