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
#include "omicron/Config.h"
#include "omicron/DataManager.h"
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
String Config::getTypeName(Setting::Type type)
{
	switch(type)
	{
	case Setting::TypeArray: return "array";
	case Setting::TypeBoolean: return "bool";
	case Setting::TypeFloat: return "float";
	case Setting::TypeGroup: return "group";
	case Setting::TypeInt: return "int";
	case Setting::TypeInt64: return "int64";
	case Setting::TypeList: return "list";
	case Setting::TypeNone: return "null";
	case Setting::TypeString: return "string";
	}
	return "UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Config::Config(const String& filename): 
	myCfgFile(NULL), 
	myCfgFilename(filename),
	myIsLoaded(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Config::~Config()
{
	if(myCfgFile != NULL) 
	{
		delete myCfgFile;
		myCfgFile = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Config::load()
{
	if(!myIsLoaded)
	{
		myCfgFile = new libconfig::Config();
		bool useFile = true;

		DataManager* dm = DataManager::getInstance();
		DataStream* stream = NULL;
		if(myCfgFilename[0] != '@')
		{
			stream = dm->openStream(myCfgFilename, DataStream::Read);
			if(stream == NULL)
			{
				oferror("Config::Load - Opening file failed: %1%", %myCfgFilename);
				return false;
			}
		}
		else
		{
			useFile = false;
		}

		ofmsg("Opened config file: %1%", %myCfgFilename);

		try
		{
			if(useFile) 
			{
				myCfgFile->read(stream->getCFile());
			}
			else
			{
				const char* str = &myCfgFilename.c_str()[1];
				myCfgFile->readString(str);
			}
		}
		catch(libconfig::ParseException e)
		{
			oferror("Config loading: %1% at line %2%", %e.getError() %e.getLine());
			if(useFile)
			{
				dm->deleteStream(stream);
			}
			return false;
		}
		if(useFile)
		{
			dm->deleteStream(stream);
		}

		myIsLoaded = true;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Setting& Config::getRootSetting()
{
	return myCfgFile->getRoot();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Config::exists(const String& path)
{
	return myCfgFile->exists(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Setting& Config::lookup(const String& path)
{
	return myCfgFile->lookup(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Config::getBoolValue(const String& name, bool defaultValue)
{
	if(!exists(name)) return defaultValue;
	Setting& s = lookup(name);
	if(s.getType() != Setting::TypeBoolean)
	{
		ofwarn("%1%: wrong setting type. Expected 'bool', found '%2%'", %name %getTypeName(s.getType()));
		return defaultValue;
	}
	return (bool)s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Config::getBoolValue(const String& name, const Setting& s, bool defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	if(sv.getType() != Setting::TypeBoolean)
	{
		ofwarn("%1%: wrong setting type. Expected 'bool', found '%2%'", %name %getTypeName(sv.getType()));
		return defaultValue;
	}
	return (bool)sv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
float Config::getFloatValue(const String& name, const Setting& s, float defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	if(sv.getType() != Setting::TypeFloat && sv.getType() != Setting::TypeInt)
	{
		ofwarn("%1%: wrong setting type. Expected 'float', found '%2%'", %name %getTypeName(sv.getType()));
		return defaultValue;
	}
	if(sv.getType() == Setting::TypeInt) return (float)((int)sv);
	return (float)sv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
float Config::getFloatValue(int index, const Setting& s, float defaultValue)
{
	if(s.getLength() <= index) return defaultValue;
	Setting& sv = s[index];
	if(sv.getType() != Setting::TypeFloat && sv.getType() != Setting::TypeInt)
	{
		ofwarn("%1%[%2%]: wrong setting type. Expected 'float', found '%3%'", %s.getName() %index %getTypeName(sv.getType()));
		return defaultValue;
	}
	if(sv.getType() == Setting::TypeInt) return (float)((int)sv);
	return (float)sv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int Config::getIntValue(const String& name, const Setting& s, int defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	if(sv.getType() != Setting::TypeFloat && sv.getType() != Setting::TypeInt)
	{
		ofwarn("%1%: wrong setting type. Expected 'int', found '%2%'", %name %getTypeName(sv.getType()));
		return defaultValue;
	}
	if(sv.getType() == Setting::TypeFloat) return (int)((float)sv);
	return (int)sv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int Config::getIntValue(int index, const Setting& s, int defaultValue)
{
	if(s.getLength() <= index) return defaultValue;
	Setting& sv = s[index];
	if(sv.getType() != Setting::TypeFloat && sv.getType() != Setting::TypeInt)
	{
		ofwarn("%1%[%2%]: wrong setting type. Expected 'int', found '%3%'", %s.getName() %index %getTypeName(sv.getType()));
		return defaultValue;
	}
	if(sv.getType() == Setting::TypeFloat) return (int)((float)sv);
	return (int)sv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String Config::getStringValue(const String& name, const Setting& s, const String& defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	if(sv.getType() != Setting::TypeString)
	{
		ofwarn("%1%: wrong setting type. Expected 'String', found '%2%'", %name %getTypeName(sv.getType()));
		return defaultValue;
	}
	return String((const char*)sv);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector4f Config::getVector4fValue(const String& name, const Setting& s, const Vector4f& defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	Vector4f value;
	value[0] = getFloatValue(0, sv, defaultValue[0]);
	value[1] = getFloatValue(1, sv, defaultValue[1]);
	value[2] = getFloatValue(2, sv, defaultValue[2]);
	value[3] = getFloatValue(3, sv, defaultValue[3]);
	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f Config::getVector3fValue(const String& name, const Setting& s, const Vector3f& defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	Vector3f value;
	value[0] = getFloatValue(0, sv, defaultValue[0]);
	value[1] = getFloatValue(1, sv, defaultValue[1]);
	value[2] = getFloatValue(2, sv, defaultValue[2]);
	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector2f Config::getVector2fValue(const String& name, const Setting& s, const Vector2f& defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	Vector2f value;
	value[0] = getFloatValue(0, sv, defaultValue[0]);
	value[1] = getFloatValue(1, sv, defaultValue[1]);
	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector2i Config::getVector2iValue(const String& name, const Setting& s, const Vector2i& defaultValue)
{
	if(!s.exists(name)) return defaultValue;
	Setting& sv = s[name];
	Vector2i value;
	value[0] = getIntValue(0, sv, defaultValue[0]);
	value[1] = getIntValue(1, sv, defaultValue[1]);
	return value;
}
