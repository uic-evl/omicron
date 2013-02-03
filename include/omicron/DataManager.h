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
#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include "osystem.h"

namespace omicron
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	class DataSource;
	class FilesystemDataSource;

	///////////////////////////////////////////////////////////////////////////////////////////////
	struct DataInfo
	{
		//! Default constructor. Creates a NULL data info (isNull() returns true)
		DataInfo():
			source(NULL),
			size(-1),
			local(false) {}

		//! The data stream name
		String name;
		//! The data stream path (relative to the DataSource)
		String path;
		//! Pointer to the data source providing this info.
		DataSource* source;
		//! The size of the data.
		int64 size;
		//! True if this data is local (that is, it can be accessed using standard streams or c 
		//! files using the DataInfo path)
		bool local;
		
		//! True if this object contains information about an existing data object.
		bool isNull() { return (size == -1); }
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API DataStream
	{
	public:
		enum Mode { Read, Write, ReadWrite };

	public:
		DataStream(const DataInfo& info): myInfo(info) {}
		virtual ~DataStream() {}

		const DataInfo& getInfo() { return myInfo; }

		virtual bool isOpen() { return false; }
		virtual int bytesAvailable() { return 0; }
		virtual bool isCFile() { return false; }
		virtual FILE* getCFile() { return NULL; }
		virtual void open(Mode mode) = 0;
		virtual void close() = 0;
		virtual void read(void* data, uint64 size) = 0;
		virtual void write(void* data, uint64 size) = 0;


	protected:
		DataInfo myInfo;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API DataSource: public ReferenceType
	{
	public:
		DataSource(const String& name): myName(name) {}

		String getName();

		//! Data stream management
		//@{
		virtual bool exists(const String& name) = 0;
		virtual DataInfo getInfo(const String& path) = 0;
		virtual DataStream* newStream(const String& name) = 0;
		virtual void deleteStream(DataStream* stream) = 0;
		//@}

	private:
		String myName;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API DataManager: public ReferenceType
	{
	public:
		typedef Dictionary<String, Ref<DataSource> > SourceDictionary;

		static DataManager* getInstance();

		//! Utility method for finding a file in one of the data sources.
		static bool findFile(const String& name, String& outPath);

		//! Utility method for reading a text file
		static String readTextFile(const String& name);

	public:
		//! Data sources
		//@{
		void addSource(DataSource* source);
		void removeSource(DataSource* source);
		//! Removes all currently registered data sources.
		void removeAllSources();
		String getDataSourceNames();
		//@}

		//! Data streams
		//@{
		DataInfo getInfo(const String& path);
		DataStream* createStream(const String& path);
		DataStream* openStream(const String& path, DataStream::Mode mode);
		void deleteStream(DataStream* stream);
		//@}

		void setCurrentPath(const String& path);
		String getCurrentPath();

	private:
		DataManager();
		static DataManager* mysInstance;

		FilesystemDataSource* myCurrentPath;
		List<DataSource*> mySources;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	inline String DataSource::getName()
	{
		return myName;
	}
}; // namespace omicron

#endif