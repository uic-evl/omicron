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
#ifndef __SOUND_H__
#define __SOUND_H__

#include "omicron/osystem.h"
#include "omicron/StringUtils.h"


#include "omicron/SoundManager.h"

namespace omicron
{

class SoundEnvironment;
class SoundInstance;

class OMICRON_API Sound: public ReferenceType
{
public:
	Sound(const String& name);
	Sound(const String& name, float, float, float, float, bool, bool);
	
	void setDefaultParameters(float, float, float, float, bool, bool);
	
	bool loadFromFile(const String& name);
	bool loadFromMemory(const void*,size_t);
	float getDuration();
	void setVolumeScale(float);
	float getVolumeScale();
	
	int getBufferID();
	String& getFilePath();

	float getDefaultVolume();

	// Reverb
	float getDefaultRoomSize();
	float getDefaultWetness();

	float getDefaultWidth();

	bool isDefaultLooping();
	bool isEnvironmentSound();

	void resetToEnvironmentParameters();
	bool isUsingEnvironmentParameters();

	void setSoundEnvironment(SoundEnvironment*);
	SoundEnvironment* getSoundEnvironment();
private:

public:
private:
	String soundName;
	String filePath;
	int bufferID;
	float duration;
	float volumeScale;
	
	// Default sound instance parameters
	float volume;	// Amplitude (0.0 - 1.0)
	float width;	// Speaker width / nSpeakers (1-20)
	float wetness;		// Wetness of sound (0.0 - 1.0)
	float roomSize;	// Room size / reverb amount (0.0 - 1.0)
	bool loop;
	bool environmentSound; // Plays on all 20 speakers
	float maxDistance;
	float minRolloffDistance;
	bool useEnvironmentParameters;

	// Sound environment this sound belongs to
	SoundEnvironment* environment;

};// Sound

class OMICRON_API SoundInstance: public ReferenceType
{
public:
	SoundInstance(Sound*);
	~SoundInstance();

	void setLoop(bool);
	bool getLoop();
	void play();
	void playStereo();
	void play( Vector3f, float, float, float, float, bool );
	void playStereo( float, bool );
	void pause();
	void stop();
	bool isPlaying();
	
	void setPosition(Vector3f);
	const Vector3f& getPosition();
	void setLocalPosition(Vector3f);
	const Vector3f& getLocalPosition();
	bool isEnvironmentSound();
	void setEnvironmentSound(bool);

	void setVolume(float);
	float getVolume();

	void fade(float, float);

	void setWidth(float);
	float getWidth();

	// Reverb
	void setRoomSize(float);
	float getRoomSize();
	void setWetness(float);
	float getWetness();
	void setReverb(float,float);
	
	void setMaxDistance(float);
	float getMaxDistance();
	void setMinRolloffDistance(float);
	float getMinRolloffDistance();
	void setDistanceRange(float, float);

	int getID();
	void setSoundEnvironment(SoundEnvironment*);
private:

public:
private:
	Ref<Sound> sound;
	int instanceID;

	enum State {playing, paused, stopped};
	State playState;

	bool loop;
	bool environmentSound;
	float volume;	// Amplitude (0.0 - 1.0)
	float width;	// Speaker width / nSpeakers (1-20)
	float wetness;		// Wetness of sound (0.0 - 1.0)
	float roomSize;	// Room size / reverb amount (0.0 - 1.0)
	float pitch;
	Vector3f position;
	Vector3f localPosition;

	float maxDistance; // Max distance sound amplitude > 0
	float minRolloffDistance; // Min distance sound amplitude at full before rolling off

	bool useEnvironmentParameters;

	SoundEnvironment* environment;
};// SoundInstance

}; // namespace omicron

#endif

