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
 *---------------------------------------------------------------------------------------------------------------------
 * Provides a sound API to interface with a SuperCollider sound server.
*********************************************************************************************************************/
#include "omicron/SoundManager.h"
#include "omicron/AssetCacheManager.h"

using namespace omicron;
using namespace oscpkt;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UdpSocket SoundManager::serverSocket;
bool SoundManager::showDebug = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::SoundManager():
	myAssetCacheEnabled(false)
{
	environment = new SoundEnvironment(this);
	myAssetCacheManager = new AssetCacheManager();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::~SoundManager()
{
	//stopSoundServer(); // Do not use - penalty by catapult
	//stopAllSounds(); // SoundEnvironment handles cleanup
	//cleanupAllSounds();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::SoundManager(const String& serverIP, int serverPort):
	myAssetCacheEnabled(false)
{
	environment = new SoundEnvironment(this);
	myAssetCacheManager = new AssetCacheManager();
	connectToServer(serverIP, serverPort);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::connectToServer(const String& serverIP, int serverPort)
{
	// Create socket and connect to OSC server
	serverSocket.bindTo(8001); // Port to receive notify messages
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

	if( serverSocket.receiveNextPacket(1000) ) // Paramater is timeout in milliseconds. -1 (default) will wait indefinatly
	{
		// Wait a second for server to startup before continuing to load sounds
	}
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
	/*
	// Message the server and inquire if the sound server is running
	if( serverSocket.receiveNextPacket(1000) ){// Paramater is timeout in milliseconds. -1 (default) will wait indefinatly 
		PacketReader pr(serverSocket.packetData(), serverSocket.packetSize());
        Message *incoming_msg;
		
        while (pr.isOk() && (incoming_msg = pr.popMessage()) != 0) {
          ofmsg( "Client: received %1%", %incoming_msg->addressPattern() );
		  
		   Message::ArgReader arg(incoming_msg->arg());
			while (arg.nbArgRemaining()) {
			  if (arg.isBlob()) {
				std::vector<char> b; arg.popBlob(b); 
				
			  } else if (arg.isBool()) {
				bool b; arg.popBool(b);
				ofmsg( "  received %1%", %b );
			  } else if (arg.isInt32()) {
				int i; arg.popInt32(i); ofmsg( "  received %1%", %i );
			  } else if (arg.isInt64()) {
				int64_t h; arg.popInt64(h); ofmsg( "  received %1%", %h );
			  } else if (arg.isFloat()) {
				float f; arg.popFloat(f); ofmsg( "  received %1%", %f );
			  } else if (arg.isDouble()) {
				double d; arg.popDouble(d); ofmsg( "  received %1%", %d );
			  } else if (arg.isStr()) {
				std::string s; arg.popStr(s); 
				ofmsg( "  received %1%", %s );
			  }
			}
        }
	}*/
	return false;
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
void SoundManager::showDebugInfo(bool value)
{
	showDebug = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SoundManager::isDebugEnabled()
{
	return showDebug;
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

	environmentVolumeScale = 1.0;
	environmentRoomSize = 0.0;
	environmentWetness = 0.0;
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
void SoundEnvironment::showDebugInfo(bool value)
{
	soundManager->showDebugInfo(value);
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
void SoundEnvironment::setSound(const String& soundName, Ref<Sound> newSound)
{
	if( soundBufferIDList.count(soundName) > 0 )
	{
		Sound* oldSound = getSound(soundName);

		ofmsg("SoundEnvironment:setSound() - Replacing bufferID %2% with bufferID %3% for sound '%1%' ", %soundName %oldSound->getBufferID() %newSound->getBufferID() );

		soundList[newSound->getBufferID()] = newSound;
		soundBufferIDList[soundName] = newSound->getBufferID();
	}
	else
	{
		ofmsg("SoundEnvironment:setSound() - '%1%' does not exist", %soundName);
	}

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
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListenerOrientation(Quaternion newOrientation)
{
	listenerOrientation = newOrientation;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListener(Vector3f newPos, Quaternion newOrientation)
{
	listenerPosition = newPos;
	listenerOrientation = newOrientation;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::getUserPosition()
{
	//return userPosition; // Disabled until local user position is correctly accounted for sound server side
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
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setUserOrientation(Quaternion newOrientation)
{
	userOrientation = newOrientation;
	updateInstancePositions();
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
void SoundEnvironment::setVolumeScale(float value)
{
	environmentVolumeScale = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float SoundEnvironment::getVolumeScale()
{
	return environmentVolumeScale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setRoomSize(float value)
{
	environmentRoomSize = value;
	for( int i = 0; i < instanceNodeIDList.size(); i++ )
	{
		Message msg("/setReverb");
		msg.pushInt32(instanceNodeIDList[i]);
		msg.pushFloat(environmentWetness);
		msg.pushFloat(environmentRoomSize);
		soundManager->sendOSCMessage(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float SoundEnvironment::getRoomSize()
{
	return environmentRoomSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setWetness(float value)
{
	environmentWetness = value;
	for( int i = 0; i < instanceNodeIDList.size(); i++ )
	{
		Message msg("/setReverb");
		msg.pushInt32(instanceNodeIDList[i]);
		msg.pushFloat(environmentWetness);
		msg.pushFloat(environmentRoomSize);
		soundManager->sendOSCMessage(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float SoundEnvironment::getWetness()
{
	return environmentWetness;
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
void SoundEnvironment::addInstance( Ref<SoundInstance> newInstance )
{
	instanceList[newInstance->getID()] = newInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::worldToLocalPosition( Vector3f position )
{
	Vector3f res = listenerOrientation.inverse() * (position - listenerPosition);
	return res;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::localToWorldPosition( Vector3f position )
{
	Vector3f res = listenerPosition + listenerOrientation * position;
    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::updateInstancePositions()
{
	map< int, Ref<SoundInstance> >::iterator it = instanceList.begin();
	for ( it = instanceList.begin(); it != instanceList.end(); ++it )
	{
		SoundInstance* inst = it->second;
		if( getSoundManager()->isDebugEnabled() )
			ofmsg("%1%: instanceID %2%", %__FUNCTION__ %inst->getID() );

		Message msg("/setObjectLoc");
		msg.pushInt32(inst->getID());
		
		Vector3f soundLocalPosition = worldToLocalPosition( inst->getPosition() );

		msg.pushFloat( soundLocalPosition[0] );
		msg.pushFloat( soundLocalPosition[1] );
		msg.pushFloat( soundLocalPosition[2] );

		getSoundManager()->sendOSCMessage(msg);
	}
};