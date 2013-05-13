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
 *************************************************************************************************/
#include <omicron.h>

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	if(argc < 3)
	{
		printf("usage: ocachesync host:port cacheName [file]+");
		return 0;
	}

	Vector<String> hp = StringUtils::split(argv[1], ":");
	String host = hp[0];
	int port = boost::lexical_cast<int>(hp[1]);

	String cacheName = argv[2];
	
	int numFiles = argc - 3;

	// Add a default filesystem data sources (used to retrieve configuration files and other resources)
	DataManager* dm = DataManager::getInstance();
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	AssetCacheManager* acm = new AssetCacheManager();
	acm->addCacheHost(host);
	acm->setCachePort(port);
	acm->setCacheName(cacheName);

	for(int i = 0; i < numFiles; i++) acm->addFileToCacheList(argv[i + 3]);

	acm->setVerbose(true);
	acm->sync();

	return 0;
}


