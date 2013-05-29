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
UdpSocket SoundManager::soundServerSocket;
UdpSocket SoundManager::soundMsgSocket;
bool SoundManager::showDebug = false;
bool SoundManager::soundServerRunning = false;

int SoundManager::nUnitGenerators = -1;
int SoundManager::nSynths = -1;
int SoundManager::nGroups = -1;
int SoundManager::nLoadedSynths = -1;
float SoundManager::avgCPU = -1;
float SoundManager::peakCPU = -1;
double SoundManager::nominalSampleRate = -1;
double SoundManager::actualSampleRate = -1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::SoundManager():
	myAssetCacheEnabled(false)
{
	environment = new SoundEnvironment(this);
	myAssetCacheManager = new AssetCacheManager();

	listenerPosition = Vector3f::Zero();
	listenerOrientation = Quaternion::Identity();

	userPosition = Vector3f::Zero();
	userOrientation = Quaternion::Identity();

	assetDirectory = "";
	assetDirectorySet = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::~SoundManager()
{
	stopSoundServer();
	stopAllSounds();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundManager::SoundManager(const String& serverIP, int serverPort):
	myAssetCacheEnabled(false)
{
	environment = new SoundEnvironment(this);
	myAssetCacheManager = new AssetCacheManager();
	connectToServer(serverIP, serverPort);

	listenerPosition = Vector3f::Zero();
	listenerOrientation = Quaternion::Identity();

	userPosition = Vector3f::Zero();
	userOrientation = Quaternion::Identity();

	assetDirectory = "";
	assetDirectorySet = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::connectToServer(const String& serverIP, int serverPort)
{
	soundMsgSocket.connectTo(serverIP, 57110);
	soundMsgSocket.bindTo(8001); // Port to receive notify messages

	Message msg5("/notify");
	msg5.pushInt32(1);

	PacketWriter pw;
	pw.startBundle().addMessage(msg5).endBundle();
	soundMsgSocket.sendPacket(pw.packetData(), pw.packetSize());

	// Create socket and connect to OSC server
	soundServerSocket.connectTo(serverIP, serverPort);
	if (!soundServerSocket.isOk()) {
		ofmsg( "SoundManager: Error connection to port %1%: %2%", %serverPort %soundServerSocket.errorMessage() );
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

	Message msg4("/startup");
	sendOSCMessage(msg4);

	wait(1000); // Give the server a second to startup
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
	// Query sound server
	Message msg5("/status");

	PacketWriter pw;
	pw.startBundle().addMessage(msg5).endBundle();
	soundMsgSocket.sendPacket(pw.packetData(), pw.packetSize());

	// Check for response to query
	if( soundMsgSocket.receiveNextPacket(1000) ){// Paramater is timeout in milliseconds. -1 (default) will wait indefinatly 
		PacketReader pr(soundMsgSocket.packetData(), soundMsgSocket.packetSize());
        Message *incoming_msg;
		
        while (pr.isOk() && (incoming_msg = pr.popMessage()) != 0) {			
			//ofmsg( "Client: received %1%", %incoming_msg->addressPattern() );
			Message::ArgReader arg(incoming_msg->arg());
			while (arg.nbArgRemaining()) {
				if (arg.isBlob()) {
					std::vector<char> b; arg.popBlob(b); 
				} else if (arg.isBool()) {
					bool b; arg.popBool(b);
					//ofmsg( "  received %1%", %b );
				} else if (arg.isInt32()) {
					int i; arg.popInt32(i); //ofmsg( "  received %1%", %i );
					if( nUnitGenerators == -1 )
						nUnitGenerators = i;
					else if( nSynths == -1 )
						nSynths = i;
					else if( nGroups == -1 )
						nGroups = i;
					else if( nLoadedSynths == -1 )
						nLoadedSynths = i;
				} else if (arg.isInt64()) {
					int64_t h; arg.popInt64(h); //ofmsg( "  received %1%", %h );
				} else if (arg.isFloat()) {
					float f; arg.popFloat(f); //ofmsg( "  received %1%", %f );
					if( avgCPU == -1 )
						avgCPU = f;
					else if( peakCPU == -1 )
						peakCPU = f;
				} else if (arg.isDouble()) {
					double d; arg.popDouble(d); //ofmsg( "  received %1%", %d );
					if( nominalSampleRate == -1 )
						nominalSampleRate = d;
					else if( actualSampleRate == -1 )
						actualSampleRate = d;
				} else if (arg.isStr()) {
					std::string s; arg.popStr(s); 
					//ofmsg( "  received %1%", %s );
			  }
			}
			
			if( nLoadedSynths > 0 )
			{
				omsg( "  Sound Server Status: Running");
				if( isDebugEnabled() )
				{
					omsg( "  Sound Server Status: Running");
					ofmsg( "    Unit Generators: %1%", %nUnitGenerators );
					ofmsg( "    Synths: %1%", %nSynths );
					ofmsg( "    Groups: %1%", %nGroups );
					ofmsg( "    Loaded Synths: %1%", %nLoadedSynths );
					ofmsg( "    Avg. Signal Processing CPU: %1%", %avgCPU );
					ofmsg( "    Peak Signal Processing CPU: %1%", %peakCPU );
					ofmsg( "    Nominal Sample Rate: %1%", %nominalSampleRate );
					ofmsg( "    Actual Sample Rate: %1%", %actualSampleRate );
				}
				soundServerRunning = true;

				Message msg5("/notify");
				msg5.pushInt32(1);

				PacketWriter pw;
				pw.startBundle().addMessage(msg5).endBundle();
				soundMsgSocket.sendPacket(pw.packetData(), pw.packetSize());
			}
        }
	}
	return soundServerRunning;
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
	return soundServerSocket.sendPacket(pw.packetData(), pw.packetSize());
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
void SoundManager::wait(float millis)
{
	if( soundServerSocket.receiveNextPacket(millis) ) // Paramater is timeout in milliseconds. -1 (default) will wait indefinatly
	{
		// Wait a second for server to startup before continuing to load sounds
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::poll()
{
	if( soundMsgSocket.receiveNextPacket(1) ){// Paramater is timeout in milliseconds. -1 (default) will wait indefinatly 
		PacketReader pr(soundMsgSocket.packetData(), soundMsgSocket.packetSize());
        Message *incoming_msg;
		
		bool nodeEndEvent = false;
		int nodeID = -1;

        while (pr.isOk() && (incoming_msg = pr.popMessage()) != 0) {
			if( isDebugEnabled() )
				ofmsg( "Client: received %1%", %incoming_msg->addressPattern() );
			if( incoming_msg->addressPattern() == "/n_end" )
				nodeEndEvent = true;

			Message::ArgReader arg(incoming_msg->arg());
			while (arg.nbArgRemaining()) {
				if (arg.isBlob()) {
					std::vector<char> b; arg.popBlob(b); 
				} else if (arg.isBool()) {
					bool b; arg.popBool(b);
					if( isDebugEnabled() ) ofmsg( "  received %1%", %b );
				} else if (arg.isInt32()) {
					int i; arg.popInt32(i);
					if( isDebugEnabled() ) ofmsg( "  received %1%", %i );

					if( nodeID == -1 )
						nodeID = i;
				} else if (arg.isInt64()) {
					int64_t h; arg.popInt64(h);
					if( isDebugEnabled() ) ofmsg( "  received %1%", %h );
				} else if (arg.isFloat()) {
					float f; arg.popFloat(f);
					if( isDebugEnabled() ) ofmsg( "  received %1%", %f );
				} else if (arg.isDouble()) {
					double d; arg.popDouble(d);
					if( isDebugEnabled() ) ofmsg( "  received %1%", %d );
				} else if (arg.isStr()) {
					std::string s; arg.popStr(s); 
					if( isDebugEnabled() ) ofmsg( "  received %1%", %s );
			  }
			}
        }
		if( nodeEndEvent )
		{
			removeInstanceNode(nodeID);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setAssetDirectory(const String& directory)
{
	assetDirectory = "/"+directory;
	assetDirectorySet = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String& SoundManager::getAssetDirectory()
{
	return assetDirectory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SoundManager::isAssetDirectorySet()
{
	return assetDirectorySet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::addInstance( Ref<SoundInstance> newInstance )
{
	soundInstanceList[newInstance->getID()] = newInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::addBuffer( Ref<Sound> newSound )
{
	soundList[newSound->getBufferID()] = newSound;
	soundNameList[newSound->getName()] = newSound->getBufferID();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundManager::getListenerPosition()
{
	return listenerPosition;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion SoundManager::getListenerOrientation()
{
	return listenerOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setListenerPosition(Vector3f newPos)
{
	listenerPosition = newPos;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setListenerOrientation(Quaternion newOrientation)
{
	listenerOrientation = newOrientation;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setListener(Vector3f newPos, Quaternion newOrientation)
{
	listenerPosition = newPos;
	listenerOrientation = newOrientation;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundManager::getUserPosition()
{
	//return userPosition; // Disabled until local user position is correctly accounted for sound server side
	return Vector3f(0,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion SoundManager::getUserOrientation()
{
	return userOrientation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setUserPosition(Vector3f newPos)
{
	userPosition = newPos;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setUserOrientation(Quaternion newOrientation)
{
	userOrientation = newOrientation;
	updateInstancePositions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundManager::worldToLocalPosition( Vector3f position )
{
	Vector3f res = listenerOrientation.inverse() * (position - listenerPosition);
	return res;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundManager::localToWorldPosition( Vector3f position )
{
	Vector3f res = listenerPosition + listenerOrientation * position;
    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundManager::getSound(const String& soundName)
{
	Sound* newSound = soundList[soundNameList[soundName]];
	if( newSound == NULL )
		ofmsg("SoundEnvironment:getSound() - '%1%' does not exist", %soundName);
	return newSound;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::setSound(const String& soundName, Ref<Sound> newSound)
{
	if( soundNameList.count(soundName) > 0 )
	{
		Sound* oldSound = getSound(soundName);

		ofmsg("SoundEnvironment:setSound() - Replacing bufferID %2% with bufferID %3% for sound '%1%' ", %soundName %oldSound->getBufferID() %newSound->getBufferID() );

		soundList[newSound->getBufferID()] = newSound;
		soundNameList[soundName] = newSound->getBufferID();
	}
	else
	{
		ofmsg("SoundEnvironment:setSound() - '%1%' does not exist", %soundName);
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector<int> SoundManager::getInstanceIDList()
{
	return instanceNodeIDList;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::stopAllSounds()
{
	for( int i = 0; i < instanceNodeIDList.size(); i++ )
	{
		Message msg("/freeNode");
		msg.pushInt32(instanceNodeIDList[i]);
		sendOSCMessage(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::cleanupAllSounds()
{
	for( int i = 0; i < bufferIDList.size(); i++ )
	{
		Message msg("/freeBuf");
		msg.pushInt32(bufferIDList[i]);
		sendOSCMessage(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::updateInstancePositions()
{
	map< int, Ref<SoundInstance> >::iterator it = soundInstanceList.begin();
	for ( it = soundInstanceList.begin(); it != soundInstanceList.end(); ++it )
	{
		SoundInstance* inst = it->second;

		if( inst->isPlaying() )
		{
			if( isDebugEnabled() )
				ofmsg("%1%: instanceID %2%", %__FUNCTION__ %inst->getID() );

			Message msg("/setObjectLoc");
			msg.pushInt32(inst->getID());
		
			Vector3f soundLocalPosition = worldToLocalPosition( inst->getPosition() );

			msg.pushFloat( soundLocalPosition[0] );
			msg.pushFloat( soundLocalPosition[1] );
			msg.pushFloat( soundLocalPosition[2] );

			sendOSCMessage(msg);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundManager::removeInstanceNode(int id)
{
	soundInstanceList.erase(id);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundEnvironment::SoundEnvironment(SoundManager* soundManager)
{
	this->soundManager = soundManager;

	environmentVolumeScale = 0.5;
	environmentRoomSize = 0.0;
	environmentWetness = 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoundEnvironment::~SoundEnvironment()
{

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

	soundManager->addBuffer( newSound );

	return newSound;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundEnvironment::getSound(const String& soundName)
{
	return soundManager->getSound(soundName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setSound(const String& soundName, Ref<Sound> newSound)
{
	soundManager->setSound( soundName, newSound );

}
/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundEnvironment::loadSoundFromFile(const String& fileName)
{
	String soundFullPath = soundManager->getAssetDirectory() + "/" + fileName;
	if( !soundManager->isAssetDirectorySet() )
		soundFullPath = fileName;

	Sound* sound = createSound(soundFullPath);
	if(sound != NULL)
	{
		sound->loadFromFile(soundFullPath);
	}
	return sound;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Sound* SoundEnvironment::loadSoundFromFile(const String& soundName, const String& filePath)
{
	String soundFullPath = soundManager->getAssetDirectory() + "/" + filePath;
	if( !soundManager->isAssetDirectorySet() )
		soundFullPath = filePath;

	// If the asset cache is enabled, copy local sound assets to the sound server
	 if(soundManager->isAssetCacheEnabled())
	 {
		AssetCacheManager* acm = soundManager->getAssetCacheManager();
		acm->setCacheName(soundManager->getAssetDirectory());
		acm->clearCacheFileList();
		acm->addFileToCacheList(filePath);
		acm->sync();
	}

	Sound* sound = createSound(soundName);
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
	return soundManager->getListenerPosition();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion SoundEnvironment::getListenerOrientation()
{
	return soundManager->getListenerOrientation();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListenerPosition(Vector3f newPos)
{
	soundManager->setListenerPosition(newPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListenerOrientation(Quaternion newOrientation)
{
	soundManager->setListenerOrientation(newOrientation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setListener(Vector3f newPos, Quaternion newOrientation)
{
	soundManager->setListenerPosition(newPos);
	soundManager->setListenerOrientation(newOrientation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f SoundEnvironment::getUserPosition()
{
	return soundManager->getUserPosition(); // Disabled until local user position is correctly accounted for sound server side
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Quaternion SoundEnvironment::getUserOrientation()
{
	return soundManager->getUserOrientation();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setUserPosition(Vector3f newPos)
{
	soundManager->setUserPosition(newPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setUserOrientation(Quaternion newOrientation)
{
	soundManager->setUserOrientation(newOrientation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundEnvironment::setAssetDirectory(const String& directory)
{
	soundManager->setAssetDirectory(directory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String& SoundEnvironment::getAssetDirectory()
{
	return soundManager->getAssetDirectory();
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
	Vector<int> instanceNodeIDList = soundManager->getInstanceIDList();

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

	Vector<int> instanceNodeIDList = soundManager->getInstanceIDList();
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