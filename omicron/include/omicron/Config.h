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
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "osystem.h"

namespace omicron
{
///////////////////////////////////////////////////////////////////////////////////////////////////
class OMICRON_API Config: public ReferenceType
{
public:
	static String getTypeName(Setting::Type type);
	static bool getBoolValue(const String& name, const Setting& s, bool defaultValue = false);
	static float getFloatValue(const String& name, const Setting& s, float defaultValue = 0.0f);
	static float getFloatValue(int index, const Setting& s, float defaultValue = 0.0f);
	static int getIntValue(const String& name, const Setting& s, int defaultValue = 0);
	static int getIntValue(int index, const Setting& s, int defaultValue = 0);
	static Vector4f getVector4fValue(const String& name, const Setting& s, const Vector4f& defaultValue = Vector4f::Zero());
	static Vector3f getVector3fValue(const String& name, const Setting& s, const Vector3f& defaultValue = Vector3f::Zero());
	static Vector2f getVector2fValue(const String& name, const Setting& s, const Vector2f& defaultValue = Vector2f::Zero());
	static Vector2i getVector2iValue(const String& name, const Setting& s, const Vector2i& defaultValue = Vector2i::Zero());
	static String getStringValue(const String& name, const Setting& s, const String& defaultValue = "");

public:
	Config(const String& filename);
	virtual ~Config();

	const String& getFilename() { return myCfgFilename; }

	bool load();

	bool isLoaded() { return myIsLoaded; }

	Setting& getRootSetting();
	bool exists(const String& path);
	Setting& lookup(const String& path);

	bool getBoolValue(const String& name, bool defaultValue = false);

private:
	bool myIsLoaded;

	String myCfgFilename;
	libconfig::Config* myCfgFile;
};

}; // namespace omicron

#endif