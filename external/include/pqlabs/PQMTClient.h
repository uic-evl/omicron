//+---------------------------------------------------------------------------
//
//  PQLabs.
//
//  Copyright (c) PQLabs.  All rights reserved.
//
//  File:       PQMTClient.h
//
//  Contents:   SDK APIs for MultiTouch Server.
//
//  Date:		2010-04-13
//
//----------------------------------------------------------------------------

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PQ_MULTITOUCH_CLIENT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PQMT_CLIENT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(WIN32) || defined(_WIN64)
#ifdef PQ_MULTITOUCH_CLIENT_EXPORTS
#define PQMT_CLIENT_API __declspec(dllexport)
#else
#define PQMT_CLIENT_API __declspec(dllimport)
#endif
#else
#define PQMT_CLIENT_API
#endif

#ifndef PQMT_CLIENT_H_
#define PQMT_CLIENT_H_
// for window ide
#ifdef _WINDOWS
#include <guiddef.h>
// others os
#else 

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef PQMT_CLIENT_API struct{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;
#endif

#endif

namespace PQ_SDK_MultiTouch
{

#ifdef  __cplusplus
extern "C"{
#endif
#define PQ_MT_CLIENT_VERSION	(0x0103)

#define PQMTE_SUCCESS			(0)
#define PQMTE_RCV_INVALIDATE_DATA				(0x31000001) // the data received is invalidate,
													 //	may be the client receive the data from other application but pqmtserver;
#define PQMTE_SERVER_VERSION_OLD				(0x31000002) // the pqmtserver is too old for this version of client.
#define PQMTE_EXCEPTION_FROM_CALLBACKFUNCTION	(0x31000003) // some exceptions thrown from the call back functions;

//
#define PQMT_DEFAULT_CLIENT_PORT	(21555)

// point_event of TouchPoint
#define TP_DOWN		(0)
#define TP_MOVE		(1)
#define TP_UP		(2)
//
//The raw touch point received from the server.
struct PQMT_CLIENT_API TouchPoint
{
	unsigned short	point_event;//Indicates current action or event of the touch point.
	unsigned short	id;         //use id to distinguish different points on the screen.
	int				x;			//the x-coordinate of the center position of the point.In pixels.
	int				y;			//the y-coordinate of the center position of the point.In pixels.
	unsigned short	dx;		    //the x-width of the touch point.In pixels.
	unsigned short	dy;			//the y-width of the touch point.In pixels.
};

// MAX_TG_PARAM_SIZE: max size of touch gesture params
#define MAX_TG_PARAM_SIZE	(6)

// type of TouchGesture

// single point
#define TG_TOUCH_START				(0x0000)
#define TG_DOWN						(0x0001)

#define TG_MOVE						(0x0006)
#define TG_UP						(0x0007)
#define TG_CLICK					(0x0008)
#define TG_DB_CLICK					(0x0009)
// single big point
#define TG_BIG_DOWN					(0x000a)
#define TG_BIG_MOVE					(0x000b)
#define TG_BIG_UP					(0x000c)

#define TG_MOVE_RIGHT				(0x0011)
#define TG_MOVE_UP					(0x0012)
#define TG_MOVE_LEFT				(0x0013)
#define TG_MOVE_DOWN				(0x0014)

// second point
#define TG_SECOND_DOWN				(0x0019)
#define TG_SECOND_UP				(0x001a) 
#define TG_SECOND_CLICK				(0x001b)
#define TG_SECOND_DB_CLICK          (0x001c)

// split
#define TG_SPLIT_START				(0x0020)
#define TG_SPLIT_APART				(0x0021)
#define TG_SPLIT_CLOSE				(0x0022)
#define TG_SPLIT_END				(0x0023)

// rotate
#define TG_ROTATE_START				(0x0024)
#define TG_ROTATE_ANTICLOCK			(0x0025)
#define TG_ROTATE_CLOCK				(0x0026)
#define TG_ROTATE_END				(0x0027)

// near parrel
#define TG_NEAR_PARREL_DOWN			(0x0028)
#define TG_NEAR_PARREL_MOVE			(0x002d)
#define TG_NEAR_PARREL_UP			(0x002e)
#define TG_NEAR_PARREL_CLICK		(0x002f)
#define TG_NEAR_PARREL_DB_CLICK		(0x0030)

#define TG_NEAR_PARREL_MOVE_RIGHT	(0x0031)
#define TG_NEAR_PARREL_MOVE_UP		(0x0032)
#define TG_NEAR_PARREL_MOVE_LEFT	(0x0033)
#define TG_NEAR_PARREL_MOVE_DOWN	(0x0034)

// multi points
#define TG_MULTI_DOWN				(0x0035)
#define TG_MULTI_MOVE				(0x003a)
#define TG_MULTI_UP					(0x003b)

#define TG_MULTI_MOVE_RIGHT			(0x003c)
#define TG_MULTI_MOVE_UP			(0x003d)
#define TG_MULTI_MOVE_LEFT			(0x003e)
#define TG_MULTI_MOVE_DOWN			(0x003f)

// 
#define TG_TOUCH_END				(0x0080)
#define TG_NO_ACTION				(0xffff)
//

struct PQMT_CLIENT_API TouchGesture{
	unsigned short	type;						//use type to distinguish different gestures.
	unsigned short	param_size;					//specifies size of the params that the gesture contains.
	double			params[MAX_TG_PARAM_SIZE];	//contains the value of the gesture params.
};

// type 0f TouchClientRequest
#define RQST_RAWDATA_INSIDE_ONLY	(0x00)
//
#define RQST_RAWDATA_INSIDE			(0x01)		// the server will send the raw touch points to this client application
												// when the first finger are touching in the client windows. 

#define RQST_RAWDATA_ALL			(0x02)		// server will send all raw touch points to this client.
#define RQST_GESTURE_INSIDE			(0x04) 
#define RQST_GESTURE_ALL			(0x08) 
#define RQST_TRANSLATOR_CONFIG	    (0x10)		// reserved


struct PQMT_CLIENT_API TouchClientRequest{
	int				type;		//specifies what service you want the server to provide.
	GUID			app_id;		//an license key id of your application.reserved.
	// param: reserved for RQST_TRANSLATOR_CONFIG, it is the name of gesture translator which will be queried from the server;
	char			param[128];	//If type is RQST_TRANSLATOR_CONFIG,it is the name of gesture translator which will be queired from the server configure tools.Otherwise it is reserved.
};

struct PQMT_CLIENT_API TouchDeviceInfo{
	int				screen_width;				//the physical touchable width of touch screen device, in millimeter
	int				screen_height;				//the physical touchabel height of touch screen device, in millimeter
	char			serial_number[128];			//the serial number of touch screen device
};

// PFuncOnReceivePointFrame
//  the action you want to take when receive the touch frame.
//	the touch points unmoving won't be sent from the server for the sake of efficency;
//	The new touch point with its pointevent being TP_DOWN
//	and the leaving touch point with its pointevent being TP_UP will be always sent from server;
typedef
PQMT_CLIENT_API
void 
(*PFuncOnReceivePointFrame)(
	int					frame_id,			// the id of this frame
	int					time_stamp,         // time stamp
	int					point_count,        // size of point_array
	const TouchPoint *	point_array,        // touch points array
	void *				call_back_object    
	);

// PFuncOnReceiveGesture
//	the action you want to take when receive the TouchGesture
typedef
PQMT_CLIENT_API
void 
(*PFuncOnReceiveGesture)(
	const TouchGesture &	gesture,
	void *					call_back_object
	);

// PFuncOnServerBreak
//	the action you want to take when server interrupt the connection.
typedef
PQMT_CLIENT_API
void 
(*PFuncOnServerBreak)(
	void * param,
	void * call_back_object
	);

// PFuncOnReceiveError
//  there may occur some network error while inter-communicate with the server, handle them here.
typedef
PQMT_CLIENT_API
void 
(*PFuncOnReceiveError)(
	int		error_code,
	void *	call_back_object
	);

// PFuncOnGetServerResolution
// call back function of getting the display resolution of the server system, attention: not the resolution of touch screen.
typedef
PQMT_CLIENT_API
void
(*PFuncOnGetServerResolution)(
	int		max_x,
	int		max_y,
	void *	call_back_object
	);

// PFuncOnGetDeviceInfo 
//	call back function of getting the information of the device.
typedef
PQMT_CLIENT_API
void
(*PFuncOnGetDeviceInfo)(
	const TouchDeviceInfo & deviceinfo,	
	void *					call_back_object
	);

// ConnectServer: 
//  Connect the multi-touch server.
PQMT_CLIENT_API 
int
ConnectServer(
	const char *	ip = "127.0.0.1",					//the ip of server, default as for local machine;
	int				port = PQMT_DEFAULT_CLIENT_PORT     //the port of server, default as PQMT_DEFAULT_CLIENT_PORT
	);

// SendRequest:
//	After connect the multi-touch server successfully, send your request to the server.
//	The request tell the server which service you'd like to enjoy.
PQMT_CLIENT_API 
int 
SendRequest(
	const TouchClientRequest & request //request information to send to the server. RQST_RAWDATA_INSIDE etc.
	);

// SendThreshold:
// only for request type RQST_RAWDATA_INSIDE(see SendRequest)
PQMT_CLIENT_API 
int 
SendThreshold(
	int move_threshold		//	it is the move threshold that will filter some points not move(the moving distance < threshold);
							//	it is in pixel(the pixel in the coordinate of server);
							//  0 for highest sensitivity in server;
	);

// SetRawDataResolution
//	set the resolution of the raw data(touch points);
PQMT_CLIENT_API
int	
SetRawDataResolution(
	int max_x,
	int max_y
	);

// GetServerResolution:
//	to get the display resolution of the server system, attention: not the resolution of touch screen.
PQMT_CLIENT_API
int
GetServerResolution(
	PFuncOnGetServerResolution	pFnCallback,
	void *						call_back_object
	);

// DisconnectServer:
//  Disconnect from the multi-touch server.
PQMT_CLIENT_API
int
DisconnectServer(
	void
	);

// SetOnReceivePointFrame:
//  Set the function that you want to execute while receiving the touch point frame.
//		pf_on_rcv_point_frame: The function pointer you want to execute while receiving the touch frame.
PQMT_CLIENT_API
PFuncOnReceivePointFrame
SetOnReceivePointFrame(
	PFuncOnReceivePointFrame	pf_on_rcv_point_frame,
	void *						call_back_object
	);

// SetOnReceiveGesture:
//  Set the function that you want to execute while receiving the touch gesture.
//		pf_on_rcv_gesture: The function pointer you want to execute while receiving the touch gesture.
PQMT_CLIENT_API
PFuncOnReceiveGesture
SetOnReceiveGesture(
	PFuncOnReceiveGesture		pf_on_rcv_gesture,
	void *						call_back_object
	);

// SetOnServerBreak:
//  Set the function that you want to execute while receive the message that the server interrupt the connection.
PQMT_CLIENT_API
PFuncOnServerBreak
SetOnServerBreak(
	PFuncOnServerBreak			pf_on_rcv_data,
	void *						call_back_object
	);

// SetOnReceiveError:
//  Set the function that you want to execute while some errors occur during the receive process.
PQMT_CLIENT_API
PFuncOnReceiveError 
SetOnReceiveError(
	PFuncOnReceiveError			pf_on_rcv_error,
	void *						call_back_object
	);

//SetOnGetDeviceInfo
// the device information will be sent after the client SendRequest to server,
// or when the device plug in/out;
// Set the function that you want to execute while the client wants the device information
PQMT_CLIENT_API
PFuncOnGetDeviceInfo
SetOnGetDeviceInfo(
	PFuncOnGetDeviceInfo		pf_on_get_device_info,
	void *						call_back_object
	);

// GetGestureName:
//	Get the touch gesture name of the touch gesture.
PQMT_CLIENT_API
const char * 
GetGestureName(
	const TouchGesture & tg
	);

//

#ifdef  __cplusplus
}
#endif

};// end of namespace

#endif // header define
