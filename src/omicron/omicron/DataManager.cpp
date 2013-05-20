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
 *-------------------------------------------------------------------------------------------------
 * Utility classes to deal with data loading, files and directories
 *************************************************************************************************/
#include "omicron/DataManager.h"
#include "omicron/FilesystemDataSource.h"
#include "omicron/StringUtils.h"

#include <fstream>

#ifdef WIN32
#include "direct.h"
#else
#include "sys/stat.h"
#endif

using namespace omicron;

DataManager* DataManager::mysInstance = NULL;
	
///////////////////////////////////////////////////////////////////////////////////////////////////
DataManager* DataManager::getInstance() 
{
	if(mysInstance == NULL)
	{
		mysInstance = new DataManager();
	}
	return mysInstance; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DataManager::findFile(const String& name, String& outPath)
{
	DataInfo di = mysInstance->getInfo(name);
	if(!di.isNull())
	{
		outPath = di.path;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DataManager::createPath(const String& path)
{
	Vector<String> args = StringUtils::split(path, "/");
	String curPath = "";
	// Keep initial slash for absolute paths.
	if(path[0] == '/') curPath = "/";
	foreach(String dir, args)
	{
		curPath += dir + "/";
#ifdef WIN32
		_mkdir(curPath.c_str());
#else
		mkdir(curPath.c_str(), S_IRUSR | S_IWUSR | S_I_ROTH);
#endif
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String DataManager::readTextFile(const String& name)
{
	std::ifstream t(name.c_str());
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataManager::DataManager()
{
	if(mysInstance != NULL)
	{
		oerror("DataManager: creating multiple instances not allowed.");
	}
	myCurrentPath = new FilesystemDataSource("[cwd]");
	addSource(myCurrentPath);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataManager::setCurrentPath(const String& path)
{
	myCurrentPath->setPath(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String DataManager::getCurrentPath()
{
	return myCurrentPath->getPath();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String DataManager::getDataSourceNames()
{
	String res = "";
	typedef std::pair<String, DataSource*> DataSourceItem;
	foreach(DataSource* src, mySources)
	{
		res = res + src->getName() + "\n";
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataManager::addSource(DataSource* source)
{
	mySources.push_back(source);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataManager::removeSource(DataSource* source)
{
	mySources.remove(source);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataManager::removeAllSources()
{
	mySources.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//DataSource* DataManager::getSource(const String& name)
//{
//	return mySources[name].get();
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataInfo DataManager::getInfo(const String& path)
{
	foreach(DataSource* ds, mySources)
	{
		if(ds->exists(path))
		{
			return ds->getInfo(path);
		}
	}
	// Data stream not found, return a null data info.
	return DataInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
DataStream* DataManager::createStream(const String& path)
{
	DataStream* stream = NULL;
	foreach(DataSource* ds, mySources)
	{
		stream = ds->newStream(path);
		if(stream != NULL) break;
	}
	return stream;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataStream* DataManager::openStream(const String& path, DataStream::Mode mode)
{
	DataStream* stream = createStream(path);
	if(stream != NULL)
	{
		stream->open(mode);
	}
	return stream;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataManager::deleteStream(DataStream* stream)
{
	if(stream->isOpen()) stream->close();
	delete stream;
}

