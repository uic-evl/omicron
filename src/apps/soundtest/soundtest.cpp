/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2014, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include <omicron.h>
#include <vector>

#include <time.h>
using namespace omicron;

#include "omicron/SoundManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SoundTest
{
private:
	Vector3f audioListener;

	SoundManager* soundManager;
	SoundEnvironment* env;

	Sound* stereoTest;
	Sound* monoTest;
	Sound* monoTestShort;

	Ref<SoundInstance> si_stereoTest;
	Ref<SoundInstance> si_monoTest;
	Ref<SoundInstance> si_monoTestShort;

	String soundServerIP;
	int soundServerPort;
	int soundServerCheckDelay;

	String stereoTestSoundPath;
	String monoTestSoundPath;
	String monoTestShortSoundPath;

	int initTime;
	int nextTime;
	int currentState;
	float azimuth;
	
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SoundTest(Config* cfg)
	{
		if(!cfg->isLoaded()) cfg->load();

		Setting& stRoot = cfg->getRootSetting()["config"];
		if( stRoot.exists("sound") )
			setup( stRoot["sound"] );
		else
		{
			soundServerIP = "127.0.0.1";
			soundServerPort = 57120;
		}

		//soundManager = new SoundManager();
		//soundManager->connectToServer(soundServerIP,soundServerPort);
		
		// More concise method of above two lines
		soundManager = new SoundManager(soundServerIP,soundServerPort);

		// Delay for loadSoundFromFile()
		// Necessary if creating a new SoundInstance immediatly after calling loadSoundFromFile()
		// Default is 500, although may need to be adjusted depending on the sound server
		soundManager->setSoundLoadWaitTime(500);

		//soundManager->showDebugInfo(true);

		// Start the sound server (if not already started)
		soundManager->startSoundServer();
		
		// Get default sound environment
		env = soundManager->getSoundEnvironment();

		
		ofmsg("SoundTest: Checking if sound server is ready at %1% on port %2%... (Waiting for %3% seconds)", %soundServerIP %soundServerPort %(soundServerCheckDelay/1000));

		bool serverReady = true;
		timeb tb;
		ftime( &tb );
		int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;
		int lastSoundServerCheck = curTime;

		while( !soundManager->isSoundServerRunning() )
		{
			timeb tb;
			ftime( &tb );
			curTime = tb.millitm + (tb.time & 0xfffff) * 1000;
			int timeSinceLastCheck = curTime-lastSoundServerCheck;

			if( timeSinceLastCheck > soundServerCheckDelay )
			{
				omsg("SoundTest: Failed to start sound server. Sound disabled.");
				serverReady = false;
				break;
			}
		}
		omsg("SoundTest: SoundServer reports ready.");
		
		// Load sound assets
		env->setAssetDirectory("soundTest");
		
		monoTest = env->loadSoundFromFile("mono",monoTestSoundPath);
		si_monoTest = new SoundInstance(monoTest);
		si_monoTest->setLoop(true);

		monoTestShort = env->loadSoundFromFile("monoS",monoTestShortSoundPath);
		//si_monoTestShort = new SoundInstance(monoTestShort);
		//si_monoTestShort->setLoop(false);

		stereoTest = env->loadSoundFromFile("mus",stereoTestSoundPath);

		si_stereoTest = new SoundInstance(stereoTest);
		si_stereoTest->setLoop(true);

		si_stereoTest->playStereo();
		
		initTime = tb.millitm + (tb.time & 0xfffff) * 1000;
		currentState = 0;
		azimuth = 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void setup(Setting& syscfg)
	{
		soundServerIP = Config::getStringValue("soundServerIP", syscfg, "localhost");
        soundServerPort = Config::getIntValue("soundServerPort", syscfg, 57120);

        // Config in seconds, function below in milliseconds
        soundServerCheckDelay = Config::getFloatValue("soundServerReconnectDelay", syscfg, 5) * 1000;

		stereoTestSoundPath = Config::getStringValue("stereoTestSound", syscfg, "stereoTestSound.wav");
		monoTestSoundPath = Config::getStringValue("monoTestSoundLoop", syscfg, "monoTestSound.wav");
		monoTestShortSoundPath = Config::getStringValue("monoTestSoundShort", syscfg, "monoTestSoundShort.wav");
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Checks the type of event. If a valid event, creates an event packet and returns true. Else return false.
	virtual bool handleEvent(const Event& evt)
	{
		
		float leftRightAnalog;
		float upDownAnalog;
		float zeroTolerence = 0.008f;
		//float xPos;
		//float yPos;
		//float zPos;

		switch(evt.getServiceType())
		{
			case Service::Wand:
				leftRightAnalog = evt.getExtraDataFloat(0);
				upDownAnalog = evt.getExtraDataFloat(1);

				volume = evt.getExtraDataFloat(4);


				if( evt.getType() == Event::Down ){

					if( evt.getFlags() & Event::Button3){ // Cross
	
					}
					if( evt.getFlags() & Event::Button2){ // Circle
						si_stereoTest->stop();
					}
					
					if( evt.getFlags() & Event::Button5){ // L1
				
					}

					if( evt.getFlags() & Event::ButtonRight){

					}
					if( evt.getFlags() & Event::ButtonLeft){

					}
					if( evt.getFlags() & Event::ButtonUp){

					}
					if( evt.getFlags() & Event::ButtonDown){

					}
					//printf("%d \n", evt.getFlags() );
				}
				
				if( (leftRightAnalog > zeroTolerence || leftRightAnalog < -zeroTolerence) &&
					(upDownAnalog > zeroTolerence || upDownAnalog < -zeroTolerence)
					){
					position[0] = leftRightAnalog;
					position[1] = upDownAnalog;
					printf("Analog Stick: %f %f\n", position[0], position[1]);
				}
				if( volume > 0 )
					printf("Analog Trigger (L2): %f\n", volume);
				return true;
				break;

			case Service::Mocap:
				//if( evt.getSourceId() == 0 )
				//	soundManager->setListenerPosition( evt.getPosition() );
				//env->setListenerPosition( Vector3f(0,0,0) );
				//else if( instanceCreated && evt.getSourceId() == 1 )
				//	soundInstance->setPosition( evt.getPosition() );
				//printf("ID: %d Pos: %f %f %f\n", evt.getSourceId(), evt.getPosition(0), evt.getPosition(1), evt.getPosition(2) );
				break;
		}
		
		return false;
	}

	void update()
	{
		soundManager->poll();
		env->setListenerPosition( Vector3f(0,0,0) );

		timeb tb;
		ftime( &tb );
		int curTime = ((tb.millitm + (tb.time & 0xfffff) * 1000) - initTime) / 1000.0f;

		//printf("[%d%] :\n",curTime);
		///////////////////////////////////////////////////////////////////
		// Stereo sound test
		///////////////////////////////////////////////////////////////////
		if( currentState == 0 )
		{
			printf("[%d%] : Starting sound test...\n",currentState);
			printf("[%d%] : Stereo sound loop playing\n",currentState);
			printf("[%d%] : If playing speed is off, file is not 48000 Hz!\n",currentState);

			currentState = 1;
			nextTime = curTime + 5;
		}
		else if( currentState == 1 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setVolume(0.1)\n",currentState);
			si_stereoTest->setVolume(0.1f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 2 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setVolume(0.7)\n",currentState);
			si_stereoTest->setVolume(0.7f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 3 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setPitch(0.5) - one octave down\n",currentState);
			si_stereoTest->setPitch(0.5f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 4 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setPitch(2) - one octave up\n",currentState);
			si_stereoTest->setPitch(2);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 5 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setPitch(-1) - backward\n",currentState);
			si_stereoTest->setPitch(-1);

			currentState++;
			nextTime += 10;
		}
		else if( currentState == 6 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setPitch(1) - normal\n",currentState);
			si_stereoTest->setPitch(1);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 7 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - fade(0,2) - fade to volume 0 over 2 seconds\n",currentState);
			si_stereoTest->fade(0,2);
			//si_stereoTest->setVolume(0);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 8 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - fade(0.8,5) - fade to volume 0.8 over 5 seconds\n",currentState);
			si_stereoTest->fade(0.8f,5);
			//si_stereoTest->setVolume(0.8);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 9 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setVolume(0.1) - post fade() check\n",currentState);
			si_stereoTest->setVolume(0.1f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 10 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setVolume(0.7) - post fade() check\n",currentState);
			si_stereoTest->setVolume(0.7f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 11 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setCurrentFrame(15000)\n",currentState);
			si_stereoTest->setCurrentFrame(15000);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 12 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound - setCurrentFrame(0)\n",currentState);
			si_stereoTest->setCurrentFrame(0);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 13 && curTime >= nextTime)
		{
			//printf("[%d%] : setServerVolume(-20)\n",currentState);
			//env->setServerVolume(-20);

			currentState++;
			nextTime += 5;

			currentState = 100;
		}

		///////////////////////////////////////////////////////////////////
		// Mono sound test
		///////////////////////////////////////////////////////////////////
		if( currentState == 100 && curTime >= nextTime)
		{
			printf("[%d%] : Stereo sound stop()\n",currentState);
			si_stereoTest->stop();
			printf("[%d%] : Mono sound loop playing\n",currentState);
			printf("[%d%] : If playing speed is off, file is not 48000 Hz!\n",currentState);
			si_monoTest->play();

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 101 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setVolume(0.1)\n",currentState);
			si_monoTest->setVolume(0.1f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 102 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setVolume(0.7)\n",currentState);
			si_monoTest->setVolume(0.7f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 103 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setPitch(0.5) - one octave down\n",currentState);
			si_monoTest->setPitch(0.5f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 104 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setPitch(2) - one octave up\n",currentState);
			si_monoTest->setPitch(2);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 105 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setPitch(-1) - backward\n",currentState);
			si_monoTest->setPitch(-1);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 106 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setPitch(1) - normal\n",currentState);
			si_monoTest->setPitch(1);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 107 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - fade(0,5) - fade to volume 0 over 5 seconds\n",currentState);
			si_monoTest->fade(0,5);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 108 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - fade(0.8,1) - fade to volume 0.8 over 1 second\n",currentState);
			si_monoTest->fade(0.8f,1);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 109 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setVolume(0.1) - post fade() check \n",currentState);
			si_monoTest->setVolume(0.1f);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 110 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setVolume(1) - post fade() check \n",currentState);
			si_monoTest->setVolume(1);

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 111 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setReverb( 0.7, 0.7 )\n",currentState);
			si_monoTest->setReverb( 0.7f, 0.7f );

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 112 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - setReverb( 0.0, 0.0 ) - normal\n",currentState);
			si_monoTest->setReverb( 0.0f, 0.0f );

			currentState++;
			nextTime += 5;
		}
		else if( currentState == 113 && curTime >= nextTime)
		{
			printf("[%d%] : Mono sound - stop()\n",currentState);
			si_monoTest->stop();
			
			//printf("[%d%] : Sound cleanup\n",currentState);
			
			//env->getSoundManager()->stopAllSounds();
			//env->getSoundManager()->cleanupAllSounds();
			
			currentState++;
			nextTime += 5;
		}
		else if( currentState == 114 && curTime >= nextTime)
		{
			printf("[%d%] : Mono short sound - play()\n",currentState);
			si_monoTestShort = new SoundInstance(monoTestShort);
			si_monoTestShort->play();
			//printf("[%d%] : Sound cleanup\n",currentState);
			
			//env->getSoundManager()->stopAllSounds();
			//env->getSoundManager()->cleanupAllSounds();
			
			currentState++;
			nextTime += 1;
		}
		else if( currentState >= 115 && currentState < 125 && curTime >= nextTime)
		{
			//printf("%s", x ? "true" : "false");
			printf("[%d%] : Mono short sound - isPlaying() - %s%\n",currentState, si_monoTestShort->isPlaying() ? "true" : "false");
			printf("[%d%] : Mono short sound - isDone() - %s%\n",currentState, si_monoTestShort->isDone() ? "true" : "false");
			
			currentState++;
			nextTime += 1;
		}
		else if( currentState == 125 && curTime >= nextTime)
		{
			printf("[%d%] : Mono short sound - play() - round 2\n",currentState);
			si_monoTestShort = new SoundInstance(monoTestShort);
			si_monoTestShort->play();

			currentState++;
			nextTime += 1;
		}
		else if( currentState >= 125 && currentState < 135 && curTime >= nextTime)
		{
			//printf("%s", x ? "true" : "false");
			printf("[%d%] : Mono short sound - isPlaying() - %s%\n",currentState, si_monoTestShort->isPlaying() ? "true" : "false");
			printf("[%d%] : Mono short sound - isDone() - %s%\n",currentState, si_monoTestShort->isDone() ? "true" : "false");
			
			currentState++;
			nextTime += 1;
		}
		else if( currentState == 135 && curTime >= nextTime)
		{
			printf("[%d%] : Sound cleanup\n",currentState);
			env->getSoundManager()->stopAllSounds();
			env->getSoundManager()->cleanupAllSounds();
			
			currentState++;
			nextTime += 1;
		}

		if( currentState > 100 && currentState < 113 )
		{
			si_monoTest->setWidth(1);
			
			azimuth += 0.001f;
			float radius = 2;
			float inclination = 0;
			
			float xPos = radius * cos(azimuth) * cos(inclination);
			float zPos = radius * sin(azimuth);
			float yPos = radius * sin(inclination) * cos(azimuth);
			
			si_monoTest->setPosition( Vector3f(xPos, yPos, zPos) );
		}
	}
private:
	Vector3f position;
	float volume;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	// Read config file name from command line or use default one.
	const char* cfgName = "soundTest.cfg";
	if(argc == 2) cfgName = argv[1];

	// Load the config file
	Config* cfg = new Config(cfgName);

	DataManager* dm = DataManager::getInstance();
	// Add a default filesystem data source using current work dir.
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	ServiceManager* sm = new ServiceManager();
	sm->setupAndStart(cfg);

	// Configure app
	if( !cfg->exists("config/sound") )
	{
		printf("Config/Sound section missing from config file: Aborting.\n");
		return 0;
	}

	SoundTest app = SoundTest(cfg);

	float delay = -0.01f; // Seconds to delay sending events (<= 0 disables delay)
#ifdef _DEBUG
	bool printOutput = true;
#else
	bool printOutput = false;
#endif

	while(true)
	{
		sm->poll();

		// Get events
		int av = sm->getAvailableEvents();
		if(av != 0)
		{
			// TODO: Instead of copying the event list, we can lock the main one.
			Event evts[OMICRON_MAX_EVENTS];
			sm->getEvents(evts, OMICRON_MAX_EVENTS);
			for( int evtNum = 0; evtNum < av; evtNum++)
			{
				app.handleEvent(evts[evtNum]);
			}
			
			//if( printOutput )
			//	printf("------------------------------------------------------------------------------\n");
		}
		app.update();
	}

	sm->stop();
	delete sm;
	delete cfg;
	delete dm;
	
	return 0;
}
