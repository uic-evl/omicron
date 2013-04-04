/********************************************************************************************************************** 
 * THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2013								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto								anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/
#ifndef __SOUND__MANAGER_H__
#define __SOUND__MANAGER_H__

#include "omicron/osystem.h"
#include "omicron/StringUtils.h"
#include "osc/oscpkt.h"
#include "osc/udp.h"
#include "Sound.h"

using namespace oscpkt;
using namespace std;

namespace omicron
{

class SoundManager;
class Sound;
class SoundInstance;

class OMICRON_API SoundEnvironment: public ReferenceType
{
public:
	SoundEnvironment(SoundManager*);
	~SoundEnvironment();

	void stopAllSounds();
	void cleanupAllSounds();
	void showDebugInfo(bool);

	SoundManager* getSoundManager();

	Sound* createSound(const String& name);
	Sound* loadSoundFromFile(const String& soundName, const String& fileName);
	Sound* getSound(const String& name);
	SoundInstance* createInstance(Sound*);
	SoundInstance* getSoundInstance(int);

	// Listener in the world/virtual space
	Vector3f getListenerPosition();
	Quaternion getListenerOrientation();

	void setListenerPosition(Vector3f);
	void setListenerOrientation(Quaternion);
	void setListener(Vector3f, Quaternion);

	// 'User' listener position relative to the
	// sound system in user tracked applications
	Vector3f getUserPosition();
	Quaternion getUserOrientation();

	void setUserPosition(Vector3f);
	void setUserOrientation(Quaternion);

	// Environment settings
	void setAssetDirectory(const String&);
	String& getAssetDirectory();

	void setVolumeScale(float);
	float getVolumeScale();
	void setRoomSize(float);
	float getRoomSize();
	void setWetness(float);
	float getWetness();

	void addInstanceID(int);
	void addBufferID(int);
	void addInstance( Ref<SoundInstance> );

	Vector3f worldToLocalPosition(Vector3f position);
	Vector3f localToWorldPosition(Vector3f position);
private:
	void updateInstancePositions();
private:
	SoundManager* soundManager;
	float environmentVolumeScale;
	float environmentRoomSize;
	float environmentWetness;

	// This is assumed to be the navigative position
	// of the listener in world coordinates
	Vector3f listenerPosition;
	Quaternion listenerOrientation;

	// This is the user/listener position reletive to the speakers
	// Used for user tracked applications
	Vector3f userPosition;
	Quaternion userOrientation;

	String assetDirectory;
	bool assetDirectorySet;

	map<int, Ref<Sound> > soundList;
	map<int, Ref<SoundInstance> > instanceList;
	map<String, int> soundBufferIDList;

	Vector<int> instanceNodeIDList;
	Vector<int> bufferIDList;
};// SoundEnvironment

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OMICRON_API SoundManager: public ReferenceType
{
public:
	SoundManager();
	~SoundManager();

	SoundManager(const String& host, int port);
	void connectToServer(const String& host, int port);
	bool isSoundServerRunning();
	SoundEnvironment* getSoundEnvironment();
	void setEnvironment(SoundEnvironment*);

	void startSoundServer();
	void stopSoundServer();

	bool sendOSCMessage(Message);
	void showDebugInfo(bool);
	bool isDebugEnabled();
private:
	SoundEnvironment* environment;
	static UdpSocket serverSocket;
	static bool showDebug;
};// SoundManager

}; // namespace omicron

#endif
