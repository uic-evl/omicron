/********************************************************************************************************************** 
* THE OMICRON PROJECT
*---------------------------------------------------------------------------------------------------------------------
* Copyright 2010-2013							Electronic Visualization Laboratory, University of Illinois at Chicago
* Authors:										
*  Arthur Nishimoto							anishimoto42@gmail.com
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

#include "omicron/SoundManager.h"

using namespace omicron;
using namespace oscpkt;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UdpSocket SoundManager::serverSocket;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::SoundManager()
{
	environment = new SoundEnvironment(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::~SoundManager()
{
	//stopSoundServer(); // Do not use - penalty by catapult
	//stopAllSounds(); // SoundEnvironment handles cleanup
	//cleanupAllSounds();
	delete environment;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::SoundManager(const String& serverIP, int serverPort)
{
	environment = new SoundEnvironment(this);
	connectToServer(serverIP, serverPort);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::connectToServer(const String& serverIP, int serverPort)
{
	// Create socket and connect to OSC server
	serverSocket.connectTo(serverIP, serverPort);
	if (!serverSocket.isOk()) {
		ofmsg( "SoundManager: Error connection to port %1%: %2%", %serverPort %serverSocket.errorMessage() );
	} else {
		ofmsg( "SoundManager: Connected to server, will send messages to %1% on port %2%", %serverIP %serverPort );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::startSoundServer()
{
	Message msg("/startServer");
	sendOSCMessage(msg);

	Message msg2("/loadSynth");
	sendOSCMessage(msg2);

	Message msg3("/loadStereoSynth");
	sendOSCMessage(msg3);

	//Message msg4("/startup");
	//sendOSCMessage(msg4);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::stopSoundServer()
{
	Message msg("/killServer");
	sendOSCMessage(msg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SoundManager::isSoundServerRunning()
{
	printf( "%s: Not implemented yet\n", __FUNCTION__);

	// Message the server and inquire if the sound server is running
	serverSocket.receiveNextPacket(1000); // Paramater is timeout in milliseconds. -1 (default) will wait indefinatly 

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundEnvironment* SoundManager::getSoundEnvironment()
{
	return environment;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setEnvironment(SoundEnvironment* newEnv)
{
	environment = newEnv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SoundManager::sendOSCMessage(Message msg)
{
	PacketWriter pw;
	pw.startBundle().addMessage(msg).endBundle();
	return serverSocket.sendPacket(pw.packetData(), pw.packetSize());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundEnvironment::SoundEnvironment(SoundManager* soundManager)
{
	this->soundManager = soundManager;

	listenerPosition = Vector3f::Zero();
	listenerOrientation = Quaternion::Identity();

	userPosition = Vector3f::Zero();
	userOrientation = Quaternion::Identity();

	assetDirectory = "";
	assetDirectorySet = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundEnvironment::~SoundEnvironment()
{
	stopAllSounds();
	cleanupAllSounds();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::stopAllSounds()
{
	for( int i = 0; i < instanceNodeIDList.size(); i++ )
	{
		Message msg("/freeNode");
		msg.pushInt32(instanceNodeIDList[i]);
		soundManager->sendOSCMessage(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::cleanupAllSounds()
{
	for( int i = 0; i < bufferIDList.size(); i++ )
	{
		Message msg("/freeBuf");
		msg.pushInt32(bufferIDList[i]);
		soundManager->sendOSCMessage(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager* SoundEnvironment::getSoundManager()
{
	return soundManager;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundEnvironment::createSound(const String& soundName)
{
	Sound* newSound = new Sound(soundName);
	newSound->setSoundEnvironment(this);

	soundList[newSound->getBufferID()] = newSound;
	soundBufferIDList[soundName] = newSound->getBufferID();

	return newSound;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundEnvironment::getSound(const String& soundName)
{
	Sound* newSound = soundList[soundBufferIDList[soundName]];
	if( newSound == NULL )
		ofmsg("SoundEnvironment:getSound() - '%1%' does not exist", %soundName);
	return newSound;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundEnvironment::loadSoundFromFile(const String& soundName, const String& fileName)
{
	String soundFullPath = assetDirectory + "/" + fileName;
	if( !assetDirectorySet )
		soundFullPath = fileName;

	Sound* sound = createSound(soundFullPath);
	if(sound != NULL)
	{
		sound->loadFromFile(soundFullPath);
	}
	return sound;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundInstance* SoundEnvironment::createInstance(Sound* sound)
{
	SoundInstance* newInstance = new SoundInstance(sound);
	return newInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::getListenerPosition()
{
	return listenerPosition;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion SoundEnvironment::getListenerOrientation()
{
	return listenerOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListenerPosition(Vector3f newPos)
{
	listenerPosition = newPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListenerOrientation(Quaternion newOrientation)
{
	listenerOrientation = newOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListener(Vector3f newPos, Quaternion newOrientation)
{
	listenerPosition = newPos;
	listenerOrientation = newOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::getUserPosition()
{
	//return userPosition;
	return Vector3f(0,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion SoundEnvironment::getUserOrientation()
{
	return userOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setUserPosition(Vector3f newPos)
{
	userPosition = newPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setUserOrientation(Quaternion newOrientation)
{
	userOrientation = newOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setAssetDirectory(const String& directory)
{
	assetDirectory = directory;
	assetDirectorySet = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String& SoundEnvironment::getAssetDirectory()
{
	return assetDirectory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::addInstanceID(int newID)
{
	instanceNodeIDList.push_back(newID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::addBufferID(int newID)
{
	bufferIDList.push_back(newID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::worldToLocal( Vector3f& position )
{
	Vector3f res = listenerOrientation.inverse() * (position - listenerPosition);
	return res;
};