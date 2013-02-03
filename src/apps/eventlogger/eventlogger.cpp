/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2011		Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include <omicron.h>

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
void logEvent(const Event& e)
{
	ofmsg("EVENT pos(%1%)", %e.getPosition());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	// Add a default filesystem data sources (used to retrieve configuration files and other resources)
	DataManager* dm = DataManager::getInstance();
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	// Load a configuration file for this application and setup the system manager.
	// Read config file name from command line or use default one.
	const char* cfgName = "eventlogger.cfg";
	if(argc == 2) cfgName = argv[1];
	Config* cfg = new Config(cfgName);

	// Start running services and listening to events.
	ServiceManager* sm = new ServiceManager();
	sm->setupAndStart(cfg);

	omsg("eventlogger start logging events...");
	while(true)
	{
		// Poll services for new events.
		sm->poll(); 

		// Get available events
		Event evts[OMICRON_MAX_EVENTS];
		int av;
		if(0 != (av = sm->getEvents(evts, ServiceManager::MaxEvents)))
		{
			for( int evtNum = 0; evtNum < av; evtNum++)
			{
				logEvent(evts[evtNum]);
			}
		}// if

	}// while
	
	delete cfg;
}


