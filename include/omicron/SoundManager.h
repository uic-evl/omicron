/********************************************************************************************************************** 
 * THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2015								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto								anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory, University of Illinois at Chicago
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
 *---------------------------------------------------------------------------------------------------------------------
 * Provides a sound API to interface with a SuperCollider sound server.
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
class AssetCacheManager;

class OMICRON_API SoundEnvironment: public ReferenceType
{
public:
	SoundEnvironment(SoundManager*);
	~SoundEnvironment();

	void showDebugInfo(bool);

	SoundManager* getSoundManager();

	Sound* createSound(const String& name);
	//Sound* loadSoundFromFile(const String& fileName);
	Sound* loadSoundFromFile(const String& soundName, const String& fileName);
	Sound* getSound(const String& name);
	void setSound(const String& name, Ref<Sound> sound);
	SoundInstance* createInstance(Sound*);
	SoundInstance* getSoundInstance(int);

	// Handles to SoundManager -----------------------
	Vector3f getListenerPosition();
	Quaternion getListenerOrientation();

	void setListenerPosition(Vector3f);
	void setListenerOrientation(Quaternion);
	void setListener(Vector3f, Quaternion);
	
	Vector3f getUserPosition();
	Quaternion getUserOrientation();

	void setUserPosition(Vector3f);
	void setUserOrientation(Quaternion);

	void setAssetDirectory(const String&);
	String& getAssetDirectory();

	void setServerVolume(int);
	int getServerVolume();

	void setForceCacheOverwrite(bool value);
	bool isForceCacheOverwriteEnabled();

	void setSoundLoadWaitTime(int time);
	int getSoundLoadWaitTime();

	//void addInstanceID(int);
	//void addBufferID(int);
	//void addInstance( Ref<SoundInstance> );

	// Environment settings --------------------------
	void setVolumeScale(float);
	float getVolumeScale();
	void setRoomSize(float);
	float getRoomSize();
	void setWetness(float);
	float getWetness();
private:
	SoundManager* soundManager;
	float environmentVolumeScale;
	float environmentRoomSize;
	float environmentWetness;
};// SoundEnvironment

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OMICRON_API SoundManager: public ReferenceType
{
public:
	SoundManager();
	~SoundManager();

	SoundManager(const String& host, int port);
	void setup(Setting& settings);

	void connectToServer(const String& host, int port);
	bool isSoundServerRunning();
	SoundEnvironment* getSoundEnvironment();
	void setEnvironment(SoundEnvironment*);

	void startSoundServer();
	void stopSoundServer();

	bool sendOSCMessage(Message);
	void showDebugInfo(bool);
	bool isDebugEnabled();
	void wait(float);

	void poll();

	//! Asset cache management
	//@{
	void setAssetCacheEnabled(bool value) { myAssetCacheEnabled = value; }
	bool isAssetCacheEnabled() { return myAssetCacheEnabled; }
	AssetCacheManager* getAssetCacheManager() { return myAssetCacheManager; }
	void setForceCacheOverwrite(bool value);
	bool isForceCacheOverwriteEnabled();
	//@}

	// Getters/Setters for SoundEnvironments
	void setAssetDirectory(const String&);
	String& getAssetDirectory();
	bool isAssetDirectorySet();

	void addBuffer( Ref<Sound> );
	void addInstance( Ref<SoundInstance> );

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

	Sound* getSound(const String& name);
	void setSound(const String& name, Ref<Sound> sound);

	Vector<int> getInstanceIDList();

	Vector3f worldToLocalPosition(Vector3f position);
	Vector3f localToWorldPosition(Vector3f position);

	void stopAllSounds();
	void cleanupAllSounds();

	void setServerVolume(int); // typically -30 to 8
	int getServerVolume();

	void setSoundLoadWaitTime(int time) { soundLoadWaitTime = time; };
	int getSoundLoadWaitTime() { return soundLoadWaitTime; };
private:
	void updateInstancePositions();
	void updateAudioImage(Vector3f soundLocalPosition, Vector3f userPosition, int instID);
	void updateObjectWidth(float width, float objToUser3D, int instID);
	void removeInstanceNode(int);
private:
	Ref<SoundEnvironment> environment;
	bool myAssetCacheEnabled;
	Ref<AssetCacheManager> myAssetCacheManager;

	static UdpSocket soundServerSocket; // Socket sounds information is sent on
	static UdpSocket soundMsgSocket; // Used for /notify messages
	static bool showDebug;
	static bool startingSoundServer;
	static bool soundServerRunning;

	// This is set in .cfg. If 'radius' does not exist in .cfg,
	// a large value is used.
	float radius;

	// This is assumed to be the navigative position
	// of the listener in world coordinates
	Vector3f listenerPosition;
	Quaternion listenerOrientation;

	// This is the user/listener position relative to the speakers
	// Used for user tracked applications
	Vector3f userPosition;
	Quaternion userOrientation;

	String assetDirectory;
	bool assetDirectorySet;
	
	// List of sound/instance objects by ID
	map< int, Ref<Sound> > soundList;
	map< int, Ref<SoundInstance> > soundInstanceList;

	// List of all active IDs
	Vector<int> instanceNodeIDList;
	Vector<int> bufferIDList;

	// Mapping of sound names to their ID
	map< string, int > soundNameList;

	// Sound server status
	static int nUnitGenerators;
	static int nSynths;
	static int nGroups;
	static int nLoadedSynths;
	static float avgCPU;
	static float peakCPU;
	static double nominalSampleRate;
	static double actualSampleRate;

	// Sound server control
	static int soundServerVolume;
	static int soundLoadWaitTime;
};// SoundManager

}; // namespace omicron

#endif
