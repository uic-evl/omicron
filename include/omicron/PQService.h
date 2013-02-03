/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
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
#include "omicron/osystem.h"
#include "omicron/ServiceManager.h"
#include "omicron/TouchGestureManager.h"
#include "pqlabs/PQMTClient.h"

//#include "input/Touches.h"
//#include <vector>
#include <sys/timeb.h>

#ifndef __PQ_LAB_TOUCH__SERVICE_H__
#define __PQ_LAB_TOUCH__SERVICE_H__

using namespace PQ_SDK_MultiTouch;
/* // Replaced with TouchGestureManager version
struct Touch{
	int ID;
	float xPos;
	float yPos;
	float xWidth;
	float yWidth;
	int timestamp;

	// Gestures
	int gestureType;
	int ID_1; // IDs of other touches in the same gesture
	int ID_2;
};
*/
namespace omicron
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PQService: public Service
{
public:
	// Allocator function
	static PQService* New() { return new PQService(); }

public:
	virtual void initialize();
	virtual void poll();
	virtual void dispose();

	void setup(Setting& settings);
	void initialize(  char* local_ip );
	int init();
private:
	static PQService* mysInstance;	
	static int maxBlobSize;
	static int screenX, screenY, serverX, serverY, screenOffsetX, screenOffsetY;
	static bool useGestureManager;
	const char* server_ip;
	int touchID[1000]; // Max IDs assigned before resetting
	static int maxTouches; // Should be same number as touchID array init
	int nextID;
	static int move_threshold;// in pixels

	std::map<int,Touch> touchlist; // Internal touch list to generate custom gestures
	
	TouchGestureManager* touchGestureManager;

	//////////////////////call back functions///////////////////////
	// onReceivePointFrame: function to handle when recieve touch point frame
	//	the unmoving touch point won't be sent from server. The new touch point with its pointevent is TP_DOWN
	//	and the leaving touch point with its pointevent will be always sent from server;
	static void onReceivePointFrame(int frame_id,int time_stamp,int moving_point_count,const TouchPoint * moving_point_array, void * call_back_object);
	// onReceivePointFrame: function to handle when recieve touch gesture
	static void onReceiveGesture(const TouchGesture & ges, void * call_back_object);
	// onServerBreak: function to handle when server break(disconnect or network error)
	static void onServerBreak(void * param, void * call_back_object);
	// onReceiveError: function to handle when some errors occur on the process of receiving touch datas.
	static void onReceiveError(int err_code,void * call_back_object);
	//
	static void OnGetServerResolution(int x, int y, void * call_back_object);
	//////////////////////call back functions end ///////////////////////

	// functions to handle TouchGestures, attention the means of the params
	void InitFuncOnTG();
	
	// set the call back functions while reciving touch data;
	void SetFuncsOnReceiveProc();

	// OnTouchPoint: function to handle TouchPoint
	void OnTouchPoint(const TouchPoint & tp);
	// OnTouchGesture: function to handle TouchGesture
	void OnTouchGesture(const TouchGesture & tg);
	//

	//here use function pointer table to handle the different gesture type;
	typedef void (*PFuncOnTouchGesture)(const TouchGesture & tg,void * call_object);
	PFuncOnTouchGesture m_pf_on_tges[TG_TOUCH_END + 1];
	static void DefaultOnTG(const TouchGesture & tg,void * call_object); // just show the gesture
	static void OnTG_TouchStart(const TouchGesture & tg,void * call_object);
	static void OnTG_Down(const TouchGesture & tg,void * call_object);
	static void OnTG_Move(const TouchGesture & tg,void * call_object);
	static void OnTG_Up(const TouchGesture & tg,void * call_object);
	static void OnTG_SecondDown(const TouchGesture & tg,void * call_object);
	static void OnTG_SecondUp(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitStart(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitApart(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitClose(const TouchGesture & tg,void * call_object);
	static void OnTG_SplitEnd(const TouchGesture & tg,void * call_object);
	static void OnTG_TouchEnd(const TouchGesture & tg,void * call_object);

};

}; // namespace omicron

#endif