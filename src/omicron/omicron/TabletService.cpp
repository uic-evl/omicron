/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
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
#include "omicron/TabletService.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
TabletConnection::TabletConnection(ConnectionInfo ci, TabletService* service): 
	TcpConnection(ci), myService(service)
{
    ltClick = false;
    rtClick = false;
    anchor[0] = -1;
    anchor[1] = -1;
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletConnection::handleData()
{
    // Read message type.
    readUntil(myBuffer, BufferSize, ':');
	if(myBuffer[0] == 't') 
	{
		handleTouchEvent();
	}
	else 
	{
		handleUiEvent();
	}
    // Read until the end of the message
    readUntil(myBuffer, BufferSize, '|');
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletConnection::handleUiEvent()
{
    readUntil(myBuffer, BufferSize, ':');
	int id = atoi(myBuffer);

    readUntil(myBuffer, BufferSize, ':');
	int value = atoi(myBuffer);

    myService->lockEvents();
    Event* evt = myService->writeHead();
	evt->reset(Event::ChangeValue, Service::Ui, id);
	evt->setExtraDataType(Event::ExtraDataIntArray);
	evt->setExtraDataInt(0, value);
    myService->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletConnection::handleTouchEvent()
{
    // Read event type.
    readUntil(myBuffer, BufferSize, ':');
    int eventType = atoi(myBuffer);
            
    float pt1x , pt1y , pt2x , pt2y;
            
    if ( eventType == Event::RotateEnd )
    {
        rtClick = false;
        genSimpleEvent( Event::Up , Service::Pointer , 0.0 , 0.0 );
    }
    else if(eventType == Event::Move || eventType == Event::Up || eventType == Event::Down )
    {
        // Read x.
        readUntil(myBuffer, BufferSize, ':');
        pt1x = atof(myBuffer);
                
        // Read y.
        readUntil(myBuffer, BufferSize, ':');
        pt1y = atof(myBuffer);
                
        switch (eventType) 
        {
            case Event::Move:
                if ( ltClick == true )  //This is not the first touch down
                {
                    float tolerance = .1f;
                    if( withinAnchor( pt1x , pt1y , tolerance) ) //if ess. the same pt.
                    {
                        genSimpleEvent( Event::Move , Service::Pointer , pt1x , pt1y );
                        anchor[0] = pt1x;
                        anchor[1] = pt1y;
                    }
                }
                break;
            case Event::Up:
                ltClick = false;
                rtClick = false;
                genSimpleEvent( Event::Up , Service::Pointer , pt1x , pt1y );
                break;
            case Event::Down:
                if ( ltClick != true )  //This is not the first touch down
                {
                    ltClick = true;                        
                    genSimpleEvent( Event::Down , Service::Pointer , pt1x , pt1y );
                    anchor[0] = pt1x;
                    anchor[1] = pt1y;                        
                }
                break;
            default:
                break;
        }
    }
    else if(eventType == Event::Zoom || eventType == Event::Rotate )
    {
        // Read point 1 x.
        readUntil(myBuffer, BufferSize, ':');
        pt1x = atof(myBuffer);
                
        // Read point 1 y.
        readUntil(myBuffer, BufferSize, ':');
        pt1y = atof(myBuffer);
                
        // Read point 2 x.
        readUntil(myBuffer, BufferSize, ':');
        pt2x = atof(myBuffer);
                
        // Read point 2 y.
        readUntil(myBuffer, BufferSize, ':');
        pt2y = atof(myBuffer);
                
        readUntil(myBuffer, BufferSize, ':');
        float param = atof(myBuffer);
                
        switch (eventType)
        {
            case Event::Rotate: // param = angle
                if( rtClick == false )  //This is the first rotate call so send a down
                {
                    rtClick = true;
                    anchor[0] = pt1x;
                    anchor[1] = pt1y;                        
                    genSimpleEvent( Event::Down , Service::Pointer , (pt1x + pt2x) * 0.5 , (pt1y + pt2y) * 0.5 );
                }
                //The rest can be simple moves with average out the rotation pts
                genSimpleEvent( Event::Move , Service::Pointer , (anchor[0] + pt2x) * 0.5 , (anchor[1] + pt2y) * 0.5 );                    
                break;
                        
            case Event::Zoom:   // param = scale
                if( rtClick == false )  //This is the first rotate call so send a down
                {
                    rtClick = true;
                    anchor[0] = pt1x;
                    anchor[1] = pt1y;                        
                    genSimpleEvent( Event::Down , Service::Pointer , (pt1x + pt2x) * 0.5 , (pt1y + pt2y) * 0.5 );
                }
                //The rest can be simple moves with average out the rotation pts
                genSimpleEvent( Event::Move , Service::Pointer , (anchor[0] + pt2x) * 0.5 , (anchor[1] + pt2y) * 0.5 );                    
                break;
            default:
                break;
        }             
    }
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
bool TabletConnection::withinAnchor( float x , float y , float tolerance )
{
    if( x + tolerance > anchor[0] && x - tolerance < anchor[0] &&  
        y + tolerance > anchor[1] && y - tolerance < anchor[1]) return true;
    else return false;
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletConnection::genSimpleEvent( Event::Type evtType ,Service::ServiceType servType , float x , float y)
{
    Event::Flags myFlag = (Event::Flags)0;
            
    if( ltClick ) myFlag = Event::Left;
    if( rtClick ) myFlag = Event::Right;
            
    myService->lockEvents();
    Event* evt = myService->writeHead();
	evt->reset(evtType, servType, getConnectionInfo().id);
    evt->setPosition( x , y );
    evt->setFlags(myFlag);
            
    myService->unlockEvents();
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletConnection::handleClosed()
{
    ofmsg("Tablet connection closed (id=%1%)", %getConnectionInfo().id);
}
        
///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletConnection::handleConnected()
{
	TcpConnection::handleConnected();

    ofmsg("Tablet connection open (id=%1%)", %getConnectionInfo().id);
	// Send out event
    myService->lockEvents();
	Event* evt = myService->writeHead();
	evt->reset(Event::Connect, Service::Generic, getConnectionInfo().id);
    myService->unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TabletService::TabletService()
{
	myServer = new TabletServer(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TabletService::~TabletService()
{
	delete myServer;
	myServer = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletService::setup(Setting& settings)
{
	int port = 3490;
	if(settings.exists("port"))
	{
		port = settings["port"];
	}
	myServer->initialize(port);
	myServer->start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TabletService::poll() 
{
	myServer->poll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TabletConnection* TabletService::getConnection(int id)
{
	if(myServer != NULL)
	{
		return (TabletConnection*)myServer->getConnection(id);
	}
	return NULL;
}

