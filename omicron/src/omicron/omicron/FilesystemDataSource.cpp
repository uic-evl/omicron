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
#include "omicron/FilesystemDataSource.h"
#include "omicron/FileDataStream.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
FilesystemDataSource::FilesystemDataSource(const String& name, const String& path):
	DataSource(name)
{
	setPath(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FilesystemDataSource::FilesystemDataSource(const String& path):
	DataSource(path)
{
	setPath(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilesystemDataSource::setPath(const String& value)
{ 
	myPath = value;
	// We add a trailing slash if it is missing (unless the path is completely empty, 
	// to accept absolute paths.
	if(!StringUtils::endsWith(myPath, "/") && myPath != "")
	{
		myPath = value + "/";
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool FilesystemDataSource::exists(const String& name)
{
	String fullname = myPath + name;
	FILE* f = fopen(fullname.c_str(), "r");
	if(f != NULL)
	{
		fclose(f);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataInfo FilesystemDataSource::getInfo(const String& name)
{
	DataInfo info;
	info.name = name;
	info.path = myPath + name;
	info.local = true;
	info.source = this;
	if(exists(name))
	{
		// Get file size.
		FILE* f = fopen(info.path.c_str(), "r");
		fseek(f, 0, SEEK_END);
		info.size = ftell(f);
		fclose(f);
	}
	return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DataStream* FilesystemDataSource::newStream(const String& name)
{
	if(exists(name))
	{
		DataInfo info = getInfo(name);
		return new FileDataStream(info);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilesystemDataSource::deleteStream(DataStream* stream)
{
	oassert(stream->getInfo().source == this);
	delete stream;
}
