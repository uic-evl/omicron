/**************************************************************************************************
 * THE OMICRON SDK
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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
 *-------------------------------------------------------------------------------------------------
 * ocachesrv - a standalone cache server. Can be used to synchronize remote files through 
 * ocachesync or by using the AssetCacheManager API.
 *************************************************************************************************/
#include <omicron.h>

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	// Add a default filesystem data sources (used to retrieve configuration files and other resources)
	DataManager* dm = DataManager::getInstance();
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	// Start running services and listening to events.
	ServiceManager* sm = new ServiceManager();

	AssetCacheService* cacheService = new AssetCacheService();

	if(argc > 1)
	{
		cacheService->setCacheRoot(argv[1]);
	}
	else
	{
		// If we have no cache root argument, look for a config file containing it.
		String cfgPath;
		if(DataManager::findFile("ocachesrv.cfg", cfgPath))
		{
			Config cfg = Config(cfgPath);
			if(cfg.isLoaded())
			{
				if(cfg.exists("config/cacheRoot"))
				{
					String cacheRoot = cfg.lookup("config/cacheRoot");
					cacheService->setCacheRoot(cacheRoot);
					ofmsg("Cache root set to: %1%", %cacheRoot);
				}
			}
		}
	}


	sm->addService(cacheService);
	sm->initialize();
	sm->start();

	ofmsg("cache server listening on port %1%", %cacheService->getPort());

	while(true)
	{
		sm->poll(); 
		osleep(10);
	}
	sm->stop();
}

