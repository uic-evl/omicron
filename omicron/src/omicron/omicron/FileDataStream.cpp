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
#include "omicron/FileDataStream.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
FileDataStream::FileDataStream(const DataInfo& info): 
	DataStream(info),
	myFile(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool FileDataStream::isCFile()
{
	return true; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FILE* FileDataStream::getCFile() 
{ 
	return myFile; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool FileDataStream::isOpen() 
{
	return myFile != NULL ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FileDataStream::open(Mode mode)
{
	char* smode = NULL;
	if(mode == DataStream::Read) smode = "rb";
	else if(mode == DataStream::Write) smode = "wb";

	myFile = fopen(myInfo.path.c_str(), smode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FileDataStream::close()
{
	if(myFile != NULL)
	{
		fclose(myFile);
		myFile = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FileDataStream::read(void* data, uint64 size)
{
	oassert(myFile != NULL);
	fread(data, 1, size, myFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FileDataStream::write(void* data, uint64 size) 
{
	oassert(myFile != NULL);
	fwrite(data, 1, size, myFile);
}
