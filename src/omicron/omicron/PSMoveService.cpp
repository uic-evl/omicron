//
//  PSMoveService.cpp
//  OmicronSDK
//
//  Created by koosha mir on 11/7/15.
//
//

#include "omicron/PSMoveService.h"

using namespace omicron;

struct PSMoveController{
    PSMove *move;
    uint myButtonState;
    int controllerID;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PSMoveService* PSMoveService::mysInstance = NULL;
int PSMoveService::controllerCount = 0;

std::map<std::string,PSMoveController*> PSMoveInfo;
typedef std::map<std::string,PSMoveController*>::iterator PSMoveIter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PSMoveService::setup(Setting& settings)
{
    myUpdateInterval = Config::getFloatValue("updateInterval", settings, 0.01f);
    myCheckControllerInterval = Config::getFloatValue("checkControllerInterval", settings, 2.00f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PSMoveService::initialize()
{
    omsg("PSMoveService: Initialize");
    if (!psmove_init(PSMOVE_CURRENT_VERSION)) {
        omsg("PSMoveService: PS Move API init failed (wrong version?)");
        return;
    }
    mysInstance = this;
    checkForNewControllers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PSMoveService::checkForNewControllers()
{
    int PSMoveCount = psmove_count_connected();

    for( int i = 0; i < PSMoveCount; i++ )
    {
        PSMove * move = psmove_connect_by_id(i);
        if (move == NULL)
            continue;
        char *serial = psmove_get_serial(move);
        std::string serialString(serial);
        free(serial);
        enum PSMove_Connection_Type cType;
        cType = psmove_connection_type(move);
        if( cType != Conn_Bluetooth){// Gyroscope and other features of PSMove devices connected by USB cannot be accessed with current API
            psmove_disconnect(move);
            continue;
        }
        if(PSMoveInfo.count(serialString)){
            psmove_disconnect(move);
            continue;
        }
        //psmove_set_rate_limiting(move);
        psmove_set_leds(move, 0, 0, 0);
        psmove_set_rumble(move, 0);
        psmove_update_leds(move);
        psmove_enable_orientation(move,PSMove_True);
        auto hasOrientation = psmove_has_orientation(move);
        std::cout<<hasOrientation<<std::endl;
        psmove_reset_orientation(move);
        auto *psMoveController = new PSMoveController;
        psMoveController->move = move;
        psMoveController->myButtonState = 0;
        psMoveController->controllerID= controllerCount++;
        PSMoveInfo[serialString] = psMoveController;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PSMoveService::poll()
{
    static float lastt;
    static float checkControllerLastt;
    float curt = (float)((double)clock() / CLOCKS_PER_SEC);
    if(curt - lastt <= myUpdateInterval)
    {
        return;
    }
    
    if(curt - checkControllerLastt > myCheckControllerInterval)
    {
        checkForNewControllers();
        checkControllerLastt = curt;
    }
    lastt = curt;
    
    for(auto PSMovePair:PSMoveInfo) {
        
        PSMoveController *controller = PSMovePair.second;
        auto move = controller->move;
        
        int res = psmove_poll(move);
        if (!res)
            continue;
        lockEvents();
        
        Event* evt = writeHead();
        
        uint curButtonState = 0;
        // Triangle Button
        if(psmove_get_buttons(move) & Btn_TRIANGLE) curButtonState |= Event::Button1;
        // Circle Button
        if(psmove_get_buttons(move) & Btn_CIRCLE) curButtonState |= Event::Button2;
        // Cross Button
        if(psmove_get_buttons(move) & Btn_CROSS) curButtonState |= Event::Button3;
        // Square Button
        if(psmove_get_buttons(move) & Btn_SQUARE) curButtonState |= Event::Button4;
        // Shoulder Button
        if(psmove_get_buttons(move) & Btn_T) curButtonState |= Event::Button7;
        // PSMOVE Button
        if(psmove_get_buttons(move) & Btn_START) curButtonState |= Event::Button5;
        // Select Button
        if(psmove_get_buttons(move) & Btn_SELECT) curButtonState |= Event::SpecialButton1;
        // Start Button
        if(psmove_get_buttons(move) & Btn_START) curButtonState |= Event::SpecialButton2;
        // PS Button
        if(psmove_get_buttons(move) & Btn_PS) curButtonState |= Event::SpecialButton3;
        
        float analog = psmove_get_trigger(move) / 255.0f;
        
        if(curButtonState != controller->myButtonState)
        {
            // If button state is bigger than previous state, it means one additional bit has been
            // set - so send a down event.
            
            if(curButtonState > controller->myButtonState)
            {
                evt->reset(Event::Down, Service::Wand, controller->controllerID);
                // Update the button state for the new down event
                controller->myButtonState = curButtonState;
                
				//if(isDebugEnabled()) ofmsg("Controller %1% flags %2% down event", %controller->controllerID %controller->myButtonState);
            }
            else
            {
                evt->reset(Event::Up, Service::Wand, controller->controllerID);
                
                // Temporarly remap controller button state to reflect recently released buttons
                controller->myButtonState = controller->myButtonState - curButtonState;
                
                //if(isDebugEnabled()) ofmsg("Controller %1% flags %2% up event", %controller->controllerID );
            }
            
        }
        else
        {
            // Button state has not changed, just send an update event.
            evt->reset(Event::Update, Service::Wand, controller->controllerID);
        }
        
        evt->setFlags(controller->myButtonState);
        
        // Re-map back to its current button state (if re-mapped for an up event)
        controller->myButtonState = curButtonState;
        float qx,qy,qz,qw;
        psmove_get_orientation(move,&qw,&qx,&qy,&qz);
        evt->setOrientation(qw,qx,qy,qz);
        evt->setExtraDataType(Event::ExtraDataFloatArray);
        evt->setExtraDataFloat(0, analog);  // analog trigger
        evt->setExtraDataFloat(1, psmove_get_temperature_in_celsius(move));// temprature
        int x,y,z;
        psmove_get_accelerometer(move, &x, &y, &z);
        evt->setExtraDataFloat(2,x); //acceleration in x axixs
        evt->setExtraDataFloat(3,y); //acceleration in y axixs
        evt->setExtraDataFloat(4,z); //acceleration in z axixs
        psmove_get_gyroscope(move, &x, &y, &z);
        evt->setExtraDataFloat(5,x); //rotation around x axixs
        evt->setExtraDataFloat(6,y); //rotation around y axixs
        evt->setExtraDataFloat(7,z); //rotation around z axixs
        psmove_get_magnetometer(move, &x, &y, &z);
        evt->setExtraDataFloat(8,x); //magnetometer in x axis
        evt->setExtraDataFloat(9,y); //magnetometer in y axis
        evt->setExtraDataFloat(10,z); //magnetometer in z axis
        
        unlockEvents();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PSMoveService::dispose()
{
    for(auto moveController:PSMoveInfo){
        psmove_disconnect(moveController.second->move);
        delete moveController.second;
    }
    PSMoveInfo.clear();
    mysInstance = NULL;
}