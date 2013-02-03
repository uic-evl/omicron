/********************************************************************************************************************** 
* THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2012							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto								anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
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
int PQService::serverX = 0; // Resolution of the machine running PQLabs
int PQService::serverY = 0; 
int PQService::screenX = 0; // If set to 1,1 PQService will send events as a normalized coordinates.
int PQService::screenY = 0;
int PQService::screenOffsetX = 0; 
int PQService::screenOffsetY = 0;
int PQService::move_threshold = 1; // pixels
bool PQService::useGestureManager = false;

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
	if(settings.exists("serverX"))
	{
		serverX =  settings["serverX"];
		printf("PQService: ServerX set to %d\n", serverX);
	}
	if(settings.exists("serverY"))
	{
		serverY =  settings["serverY"];
		printf("PQService: ServerY set to %d\n", serverY);
	}
	if(settings.exists("screenX"))
	{
		screenX =  settings["screenX"];
		printf("PQService: ScreenX set to %d\n", screenX);
	}
	if(settings.exists("screenY"))
	{
		screenY =  settings["screenY"];
		printf("PQService: ScreenY set to %d\n", screenY);
	}
	if(settings.exists("screenOffsetX"))
	{
		screenOffsetX =  settings["screenOffsetX"];
		printf("PQService: ScreenOffsetX set to %d\n", screenOffsetX);
	}
	if(settings.exists("screenOffsetY"))
	{
		screenOffsetY =  settings["screenOffsetY"];
		printf("PQService: ScreenOffsetY set to %d\n", screenOffsetY);
	}
	if(settings.exists("moveThreshold"))
	{
		move_threshold =  settings["moveThreshold"];
		printf("PQService: move threshold set to %d\n", move_threshold);
	}
	if(settings.exists("useGestureManager"))
	{
		useGestureManager = settings["useGestureManager"];
		if( useGestureManager )
			printf("PQService: Gesture Manager Enabled\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::initialize( ) 
{
	mysInstance = this;
	
	memset(m_pf_on_tges,0, sizeof(m_pf_on_tges));
	init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::initialize( char* local_ip ) 
{
	mysInstance = this;
	
	memset(m_pf_on_tges,0, sizeof(m_pf_on_tges));
	server_ip = local_ip;
	init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::poll() 
{
	if( useGestureManager )
		touchGestureManager->poll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PQService::init()
{
	nextID = 0;
	for(int i = 0; i < maxTouches; i++){
		touchID[i] = 0;
	}

	int err_code = PQMTE_SUCESS;

	// initialize the handle functions of gestures;
	//InitFuncOnTG();
	if( useGestureManager ){
		touchGestureManager = new TouchGestureManager();
		touchGestureManager->registerPQService(mysInstance);
		touchGestureManager->setMaxTouchIDs(maxTouches);
	}

	// set the functions on server callback
	SetFuncsOnReceiveProc();

	// connect server
	printf("PQService: connecting to server on %s... \n", server_ip);
	if((err_code = ConnectServer(server_ip)) != PQMTE_SUCESS){
		printf("PQService: connect to server failed, socket error code: %d\n", err_code);
		return err_code;
	}

	// send request to server
	printf("PQService: connected to server, sending request...\n");
	TouchClientRequest tcq = {0};
	tcq.app_id = GetTrialAppID();
	tcq.type = RQST_RAWDATA_ALL;
	if((err_code = SendRequest(tcq)) != PQMTE_SUCESS){
		printf("PQService: send request failed, error code: %d\n", err_code);
		return err_code;
	}
	//////////////you can set the move_threshold when the tcq.getType() is RQST_RAWDATA_INSIDE;
	//send threshold
	if((err_code = SendThreshold(move_threshold)) != PQMTE_SUCESS){
		printf(" send threadhold fail, error code:" , err_code );
		return err_code;
	}
	
	////////////////////////
	//get server resolution
	if((err_code = GetServerResolution(OnGetServerResolution, NULL)) != PQMTE_SUCESS){
		printf("PQService: get server resolution failed,error code: %d\n", err_code);
		return err_code;
	};
	//
	// start receiving
	printf("PQService: send request succeeded, start recv.\n");
	printf("PQService: Maximum blob size set to %i pixels \n", maxBlobSize);
	return err_code;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: InitFuncOnTG()
{
	// initialize the call back functions of toucha gestures;
	m_pf_on_tges[TG_TOUCH_START] = &PQService::OnTG_TouchStart;
	m_pf_on_tges[TG_DOWN] = &PQService::OnTG_Down;
	m_pf_on_tges[TG_MOVE] = &PQService::OnTG_Move;
	m_pf_on_tges[TG_UP] = &PQService::OnTG_Up;

	m_pf_on_tges[TG_SECOND_DOWN] = &PQService::OnTG_SecondDown;
	m_pf_on_tges[TG_SECOND_UP] = &PQService::OnTG_SecondUp;

	m_pf_on_tges[TG_SPLIT_START] = &PQService::OnTG_SplitStart;
	m_pf_on_tges[TG_SPLIT_APART] = &PQService::OnTG_SplitApart;
	m_pf_on_tges[TG_SPLIT_CLOSE] = &PQService::OnTG_SplitClose;
	m_pf_on_tges[TG_SPLIT_END] = &PQService::OnTG_SplitEnd;

	m_pf_on_tges[TG_TOUCH_END] = &PQService::OnTG_TouchEnd;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_TouchStart(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_TOUCH_START);
	//printf("  here, the touch start, initialize something.\n");;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: DefaultOnTG(const TouchGesture & tg,void * call_object) // just show the gesture
{
	//cout <<"ges,name:"<< GetGestureName(tg) << " type:" << tg.type << ",param size:" << tg.param_size << " ";
	//for(int i = 0; i < tg.param_size; ++ i)
	//	cout << tg.params[i] << " ";
	//cout << endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_Down(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_DOWN && tg.param_size >= 2);
	//printf("  the single finger touching at :( " 
	//	<< tg.params[0] << "," << tg.params[1] << " )\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_Move(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_MOVE && tg.param_size >= 2);
	//printf("  the single finger moving on the screen at :( " 
	//	<< tg.params[0] << "," << tg.params[1] << " )\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_Up(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_UP && tg.param_size >= 2);
	//printf(" the single finger is leaving the screen at :( " 
	//	<< tg.params[0] << "," << tg.params[1] << " )\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_SecondDown(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SECOND_DOWN && tg.param_size >= 4);
	//printf("  the second finger touching at :( " 
	//	<< tg.params[0] << "," << tg.params[1] << " ),"
	//	<< " after the first finger touched at :( "
	//	<< tg.params[2] << "," << tg.params[3] << " )\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_SecondUp(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SECOND_UP && tg.param_size >= 4);
	//printf("  the second finger is leaving at :( " 
	//	<< tg.params[0] << "," << tg.params[1] << " ),"
	//	<< " while the first finger still anchored around :( "
	//	<< tg.params[2] << "," << tg.params[3] << " )\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_SplitStart(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SPLIT_START && tg.param_size >= 4);
	//printf("  the two fingers is splitting with one finger at: ( " 
	//	<< tg.params[0] << "," << tg.params[1] << " ),"
	//	<< " , the other at :( "
	//	<< tg.params[2] << "," << tg.params[3] << " )\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_SplitApart(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SPLIT_APART && tg.param_size >= 1);
	//printf("  the two fingers is splitting apart with there distance incresed by " 
	//	<< tg.params[0]
	//	<< " with a ratio :" << tg.params[1]
	//	<< endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_SplitClose(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SPLIT_CLOSE && tg.param_size >= 1);
	//printf("  the two fingers is splitting close with there distance decresed by " 
	//	<< tg.params[0]
	//	<< " with a ratio :" << tg.params[1]
	//	<< endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTG_SplitEnd(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SPLIT_END);
	//printf("  the two splitting fingers with one finger at: ( " 
	//	<< tg.params[0] << "," << tg.params[1] << " ),"
	//	<< " , the other at :( "
	//	<< tg.params[2] << "," << tg.params[3] << " )" 
	//	<< " will end\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnTG_TouchEnd: to clear what need to clear
void PQService:: OnTG_TouchEnd(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_TOUCH_END);
	//printf("  all the fingers is leaving and there is no fingers on the screen.\n");;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::SetFuncsOnReceiveProc()
{
	PFuncOnReceivePointFrame old_rf_func = SetOnReceivePointFrame(&PQService::onReceivePointFrame,this);
	//PFuncOnReceiveGesture old_rg_func = SetOnReceiveGesture(&PQService::onReceiveGesture,this);
	PFuncOnServerBreak old_svr_break = SetOnServerBreak(&PQService::onServerBreak,NULL);
	PFuncOnReceiveError old_rcv_err_func = SetOnReceiveError(&PQService::onReceiveError,NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: onReceivePointFrame(int frame_id, int time_stamp, int moving_point_count, const TouchPoint * moving_point_array, void * call_back_object)
{
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
void PQService:: onReceiveGesture(const TouchGesture & ges, void * call_back_object)
{
	PQService * omegaDesk = static_cast<PQService*>(call_back_object);
	assert(omegaDesk != NULL);
	omegaDesk->OnTouchGesture(ges);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: onServerBreak(void * param, void * call_back_object)
{
	// when the server break, disconenct server;
	printf("PQService: server break, disconnect here\n");;
	DisconnectServer();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::onReceiveError(int err_code, void * call_back_object)
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
void PQService:: OnGetServerResolution(int x, int y, void * call_back_object)
{
	serverX = x;
	serverY = y;
	
	printf("PQService: server resolution: %d , %d \n",serverX ,serverY );
	if( screenX == 0 && screenY == 0 ){
		screenX = x;
		screenY = y;
		printf("PQService: screen resolution not specified. using server resolution. \n");
	} else if( screenX == 1 && screenY == 1 ){
		printf("PQService: using normalized coordinates.\n");
	} else {
		printf("PQService: using screen resolution: %d , %d \n",screenX ,screenY );
		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService:: OnTouchGesture(const TouchGesture & tg)
{
	if(TG_NO_ACTION == tg.type)
		return ;
	
	assert(tg.type <= TG_TOUCH_END);
	DefaultOnTG(tg,this);
	PFuncOnTouchGesture pf = m_pf_on_tges[tg.type];
	if(NULL != pf){
		pf(tg,this);
	}

	mysInstance->lockEvents();

	// Magic numbers for split and rotate gestures yay! Not needed for start/end events.
    float specialDividerX = 4096.0f;
	float specialDividerY = 4096.0f;
	
	bool debugText = false;

	//Event* evt;

	float x = 0;
	float y = 0;
	
	//bool validEvent = false;
	//switch(tg.type)
	//{
		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_DOWN:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Down, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch down at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y );
		//	}
		//	break;

		//case TG_MOVE:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Move, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch move at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;
		//case TG_UP:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Up, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch up at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;
		//case TG_CLICK:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Click, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch click at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;
		//case TG_DB_CLICK:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::DoubleClick, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch db click at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_MOVE_RIGHT:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::MoveRight, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch move right at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;
		//case TG_MOVE_UP:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::MoveUp, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch move up at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;
		//case TG_MOVE_LEFT:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::MoveLeft, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch move left at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;
		//case TG_MOVE_DOWN:
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::MoveDown, Service::Pointer, -1);
		//	x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	y = tg.params[1] * screenY / serverY + screenOffsetY;
		//	evt->setPosition(x, y);
		//	if( debugText ){
		//		printf(" Touch move down at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], x, y);
		//	}
		//	break;

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_SPLIT_START:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::SplitStart, Service::Pointer, -1);
		//	evt->setExtraDataType(Event::ExtraDataFloatArray);
		//	Vector3f pt1(
		//		tg.params[0] * screenX / specialDividerX + screenOffsetX,
		//		tg.params[1] * screenY / specialDividerY + screenOffsetY,
		//		0);
		//	Vector3f pt2(
		//		tg.params[2] * screenX / specialDividerX + screenOffsetX,
		//		tg.params[3] * screenY / specialDividerY + screenOffsetY,
		//		0);
		//	evt->setExtraDataFloat(0, pt1[0]); // x2
		//	evt->setExtraDataFloat(1, pt1[1]); // y2
		//	evt->setExtraDataFloat(2, pt2[0]); // x1
		//	evt->setExtraDataFloat(3, pt2[1]); // y1
		//	evt->setPosition((pt1 + pt2) / 2);
		//	//if( debugText ){
		//	//	printf(" Touch Split start at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], evt->pointSet[0][0], evt->pointSet[0][1] );
		//	//	printf("                      %f,%f (%f, %f)\n", tg.params[2], tg.params[3], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_SPLIT_END:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::SplitEnd, Service::Pointer, -1);
		//	evt->setExtraDataType(Event::ExtraDataFloatArray);
		//	Vector3f pt1(
		//		tg.params[0] * screenX / specialDividerX + screenOffsetX,
		//		tg.params[1] * screenY / specialDividerY + screenOffsetY,
		//		0);
		//	Vector3f pt2(
		//		tg.params[2] * screenX / specialDividerX + screenOffsetX,
		//		tg.params[3] * screenY / specialDividerY + screenOffsetY,
		//		0);
		//	evt->setExtraDataFloat(0, pt1[0]); // x2
		//	evt->setExtraDataFloat(1, pt1[1]); // y2
		//	evt->setExtraDataFloat(2, pt2[0]); // x1
		//	evt->setExtraDataFloat(3, pt2[1]); // y1
		//	evt->setPosition((pt1 + pt2) / 2);
		//	//if( debugText ){
		//	//	printf(" Touch Split end at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], evt->pointSet[0][0], evt->pointSet[1][0] );
		//	//	printf("                    %f,%f (%f, %f)\n", tg.params[2], tg.params[3], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_SPLIT_APART:
		//case TG_SPLIT_CLOSE:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Split, Service::Pointer, -1);
		//	evt->setExtraDataType(Event::ExtraDataFloatArray);
		//	Vector3f pt1(
		//		tg.params[2] * screenX / specialDividerX + screenOffsetX,
		//		tg.params[3] * screenY / specialDividerY + screenOffsetY,
		//		0);
		//	Vector3f pt2(
		//		tg.params[4] * screenX / specialDividerX + screenOffsetX,
		//		tg.params[5] * screenY / specialDividerY + screenOffsetY,
		//		0);
		//	evt->setExtraDataFloat(0, pt1[0]); // x2
		//	evt->setExtraDataFloat(1, pt1[1]); // y2
		//	evt->setExtraDataFloat(2, pt2[0]); // x1
		//	evt->setExtraDataFloat(3, pt2[1]); // y1
		//	evt->setPosition((pt1 + pt2) / 2);
		//	evt->setExtraDataFloat(4, tg.params[0]); // delta distance
		//	evt->setExtraDataFloat(5, tg.params[1]); // delta ratio
		//	//if( debugText ){
		//	//	printf(" Touch Split at %f,%f (%f, %f)\n", tg.params[2], tg.params[3], evt->pointSet[0][0], evt->pointSet[1][0] );
		//	//	printf("                %f,%f (%f, %f)\n", tg.params[4], tg.params[5], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_ROTATE_START:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::RotateStart, Service::Pointer, -1);
		//	evt->setExtraDataType(Event::ExtraDataFloatArray);
		//	float x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	float y = tg.params[0] * screenY / serverY + screenOffsetY;
		//	evt->setExtraDataFloat(0, x); // anchorX
		//	evt->setExtraDataFloat(1, y); // anchorY
		//	evt->setExtraDataFloat(2, tg.params[2] * screenX / serverX + screenOffsetX); // rotFingerX
		//	evt->setExtraDataFloat(3, tg.params[3] * screenY / serverY + screenOffsetY); // rotFingerY
		//	evt->setPosition(x, y);	// Point 0 is the center of rotation.
		//	//if( debugText ){
		//	//	printf(" Touch rotate start at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], evt->pointSet[0][0], evt->pointSet[1][0] );
		//	//	printf("                       %f,%f (%f, %f)\n", tg.params[2], tg.params[3], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_ROTATE_END:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::RotateEnd, Service::Pointer, -1);
		//	evt->setExtraDataType(Event::ExtraDataFloatArray);
		//	float x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	float y = tg.params[0] * screenY / serverY + screenOffsetY;
		//	evt->setExtraDataFloat(0, x); // anchorX
		//	evt->setExtraDataFloat(1, y); // anchorY
		//	evt->setExtraDataFloat(2, tg.params[2] * screenX / serverX + screenOffsetX); // rotFingerX
		//	evt->setExtraDataFloat(3, tg.params[3] * screenY / serverY + screenOffsetY); // rotFingerY
		//	evt->setPosition(x, y);	// Point 0 is the center of rotation.
		//	//if( debugText ){
		//	//	printf(" Touch rotate end at %f,%f (%f, %f)\n", tg.params[0], tg.params[1], evt->pointSet[0][0], evt->pointSet[1][0] );
		//	//	printf("                     %f,%f (%f, %f)\n", tg.params[2], tg.params[3], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_ROTATE_ANTICLOCK:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Rotate, Service::Pointer, -1);
		//	float x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	float y = tg.params[0] * screenY / serverY + screenOffsetY;
		//	evt->setExtraDataFloat(0, x); // anchorX
		//	evt->setExtraDataFloat(1, y); // anchorY
		//	evt->setExtraDataFloat(2, tg.params[2] * screenX / serverX + screenOffsetX); // rotFingerX
		//	evt->setExtraDataFloat(3, tg.params[3] * screenY / serverY + screenOffsetY); // rotFingerY
		//	evt->setExtraDataFloat(4, -tg.params[0]); // angle
		//	evt->setPosition(x, y);	// Point 0 is the center of rotation.
		//	//if( debugText ){
		//	//	printf(" Touch Rotate anti-clock at %f,%f (%f, %f)\n", tg.params[1], tg.params[2], evt->pointSet[0][0], evt->pointSet[1][0] );
		//	//	printf("                            %f,%f (%f, %f)\n", tg.params[3], tg.params[4], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}

		// BROKEN FOR NOW IF YOU UNCOMMENT FIX
		//case TG_ROTATE_CLOCK:
		//{
		//	evt = mysInstance->writeHead();
		//	evt->reset(Event::Rotate, Service::Pointer, -1);
		//	evt->reset(Event::Rotate, Service::Pointer, -1);
		//	float x = tg.params[0] * screenX / serverX + screenOffsetX;
		//	float y = tg.params[0] * screenY / serverY + screenOffsetY;
		//	evt->setExtraDataFloat(0, x); // anchorX
		//	evt->setExtraDataFloat(1, y); // anchorY
		//	evt->setExtraDataFloat(2, tg.params[2] * screenX / serverX + screenOffsetX); // rotFingerX
		//	evt->setExtraDataFloat(3, tg.params[3] * screenY / serverY + screenOffsetY); // rotFingerY
		//	evt->setExtraDataFloat(4, -tg.params[0]); // angle
		//	evt->setPosition(x, y);	// Point 0 is the center of rotation.
		//	//if( debugText ){
		//	//	printf(" Touch Rotate clock at %f,%f (%f, %f)\n", tg.params[1], tg.params[2], evt->pointSet[0][0], evt->pointSet[1][0] );
		//	//	printf("                       %f,%f (%f, %f)\n", tg.params[3], tg.params[4], evt->pointSet[1][0], evt->pointSet[1][1] );
		//	//}
		//	break;
		//}
		//default:
		//	break;
	//}// switch		
	//if( validEvent ){
	//	evt->serviceType = Service::Pointer;
	//	evt->sourceId = -1; // Gestures have no id
	//}
	mysInstance->unlockEvents();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// here, just record the position of point,
//	you can do mouse map like "OnTG_Down" etc;
void PQService:: OnTouchPoint(const TouchPoint & tp)
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
		
		// GestureManager needs PQ ID, otherwise PQService manages the ID
		// Basically PQ IDs recycle after the ID is done. OmegaLib increments IDs
		// until max ID is reached.
		if( useGestureManager )
			touch.ID = tp.id;
		else
			touch.ID = touchID[tp.id];

		touch.xPos = tp.x * (float)screenX / (float)serverX + screenOffsetX;
		touch.yPos = tp.y * (float)screenY / (float)serverY + screenOffsetY;
		touch.xWidth = xWidth * (float)screenX / (float)serverX;
		touch.yWidth = yWidth * (float)screenY / (float)serverY;

		touch.timestamp = timestamp;

		// Process touch gestures (this is done outside above event creation
		// during case touchGestureManager needs to create an event)
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
		} else {
			mysInstance->lockEvents();

			Event* evt = mysInstance->writeHead();
			switch(tp.point_event)
			{
				case TP_DOWN:
					evt->reset(Event::Down, Service::Pointer, nextID);
					touchID[tp.id] = nextID;
					if( nextID < maxTouches - 100 ){
						nextID++;
					} else {
						nextID = 0;
					}
					break;
				case TP_MOVE:
					evt->reset(Event::Move, Service::Pointer, touch.ID);
					touchlist[touch.ID] = touch;
					break;
				case TP_UP:
					evt->reset(Event::Up, Service::Pointer, touch.ID);
					touchlist.erase( touch.ID );
					break;
			}		
			if( serverX != 0 && serverY != 0 ){
				evt->setPosition(touch.xPos,touch.yPos);

				evt->setExtraDataType(Event::ExtraDataFloatArray);
				evt->setExtraDataFloat(0, touch.xWidth);
				evt->setExtraDataFloat(1,touch.yWidth);
			} else {
				evt->setPosition(tp.x, tp.y);

				evt->setExtraDataType(Event::ExtraDataFloatArray);
				evt->setExtraDataFloat(0, xWidth);
				evt->setExtraDataFloat(1, yWidth);
			}

			//printf(" Server %d,%d Screen %d, %d\n", serverX, serverY, screenX, screenY );
			//printf("      at %d,%d\n", tp.x, tp.y);
			mysInstance->unlockEvents();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PQService::dispose() 
{
	mysInstance = NULL;
	DisconnectServer();
}
