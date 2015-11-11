//
//  PSMoveService.hpp
//  OmicronSDK
//
//  Created by koosha mir on 11/7/15.
//
//

#ifndef PSMoveService_h
#define PSMoveService_h
#include "omicron/osystem.h"
#include "omicron/ServiceManager.h"
#include <psmove.h>

namespace omicron {
    ///////////////////////////////////////////////////////////////////////////////////////////////
    //! HearthbeatService implements a very simple event service, that sends out update events
    //! at a predefined rate.
    //! HearthbeatService main function is to test the omegalib event distribution system. It can
    //! also be used as an example and a starting point to develop custom event services.
    class PSMoveService: public Service
    {
        

    public:
        //! Allocator function (will be used to register the service inside SystemManager)
        static PSMoveService* New() { return new PSMoveService(); }
        
    public:
        
        virtual void setup(Setting& settings);
        virtual void poll();
        virtual void initialize();
        virtual void dispose();
        void checkForNewControllers();
        
    private:
        
        static PSMoveService* mysInstance;
        static int controllerCount;
        float myUpdateInterval;
        float myCheckControllerInterval;
        //Controllers

    };
}; // namespace omicron
#endif /* PSMoveService_hpp */
