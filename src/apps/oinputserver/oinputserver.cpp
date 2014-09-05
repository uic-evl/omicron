/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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
#include <omicron.h>
#include "omicron/InputServer.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    omsg("OmicronSDK - oinputserver");
    omsg("Copyright (C) 2010-2014 Electronic Visualization Laboratory\nUniversity of Illinois at Chicago");
    omsg("======================================================");
    omsg("");

    InputServer app;
    
    // Read config file name from command line or use default one.
    const char* cfgName = "oinputserver.cfg";
    if(argc == 2) cfgName = argv[1];

    Config* cfg = new Config(cfgName);

    DataManager* dm = DataManager::getInstance();
    // Add a default filesystem data source using current work dir.
    dm->addSource(new FilesystemDataSource("./"));
    dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

    ServiceManager* sm = new ServiceManager();
    sm->setupAndStart(cfg);

    app.startConnection(cfg);

    omsg("oinputserver: Starting to listen for clients...");
    int i = 0;
    while(true)
    {
        sm->poll();
        app.loop();

        // Start listening for clients (non-blocking)
        app.startListening();

        // Get events
        int av = sm->getAvailableEvents();
        //ofmsg("------------------------loop %1%  av %2%", %i++ %av);
        if(av != 0)
        {
            // TODO: Instead of copying the event list, we can lock the main one.
            Event evts[OMICRON_MAX_EVENTS];
            sm->getEvents(evts, OMICRON_MAX_EVENTS);
            for( int evtNum = 0; evtNum < av; evtNum++)
            {
                app.handleEvent(evts[evtNum]);
            }
        }
#ifdef WIN32
        Sleep(1);
#else
        usleep(1000);
#endif	
    }

    sm->stop();
    delete sm;
    delete cfg;
    delete dm;
}
