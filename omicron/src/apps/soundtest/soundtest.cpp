/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Arthur Nishimoto		anishimoto42@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
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

	Sound* showMenuSound;
	Sound* hideMenuSound;
	Sound* selectMenuSound;
	Sound* scrollMenuSound;
	Sound* soundLoop;
	Ref<SoundInstance> soundLoopInstance;
	Ref<SoundInstance> rewindingSoundInstance;

public:


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SoundTest()
	{
		//soundManager = new SoundManager();
		//soundManager->connectToServer("localhost",57120);
		
		// More concise method of above two lines
		soundManager = new SoundManager("localhost",57120);

		soundManager->showDebugInfo(true);

		// Get default sound environment
		env = soundManager->getSoundEnvironment();

		
		while( !soundManager->isSoundServerRunning() )
		{
			soundManager->startSoundServer();
		}

		// Load sound assets
		//env->setAssetDirectory("menu_sounds");

		showMenuSound = env->loadSoundFromFile("showMenuSound","menu_sounds/menu_load.wav");
		hideMenuSound = env->loadSoundFromFile("hideMenuSound","menu_sounds/menu_closed.wav");
		scrollMenuSound = env->loadSoundFromFile("scrollMenuSound","menu_sounds/menu_scroll.wav");
		selectMenuSound = env->loadSoundFromFile("selectMenuSound","menu_sounds/menu_select.wav");
		soundLoop = env->loadSoundFromFile("mus","Omega4Relay.wav");

		SoundInstance* soundInstance = new SoundInstance(showMenuSound);
		soundInstance->setReverb( 1.0, 1.0 );
		soundInstance->setVolume(1.0);
		soundInstance->setPosition( Vector3f(0,1,0) );
		soundInstance->play();

		rewindingSoundInstance = new SoundInstance(showMenuSound);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Checks the type of event. If a valid event, creates an event packet and returns true. Else return false.
	virtual bool handleEvent(const Event& evt)
	{
		
		float leftRightAnalog;
		float upDownAnalog;
		float zeroTolerence = 0.008f;
		float xPos;
		float yPos;
		float zPos;

		switch(evt.getServiceType())
		{
			case Service::Wand:
				leftRightAnalog = evt.getExtraDataFloat(0);
				upDownAnalog = evt.getExtraDataFloat(1);

				volume = evt.getExtraDataFloat(4);
				
				if( soundLoopInstance != NULL )
				{
					soundLoopInstance->setVolume(volume);
				}

				if( evt.getType() == Event::Down ){

					if( evt.getFlags() & Event::Button3){ // Cross
						
						soundLoopInstance->setEnvironmentSound( true );
						soundLoopInstance->setLoop( true );
						
						soundLoopInstance->playStereo();
					}
					if( evt.getFlags() & Event::Button2){ // Circle
						soundLoopInstance->stop();
					}
					
					if( evt.getFlags() & Event::Button5){ // L1
						soundLoopInstance = new SoundInstance(soundLoop);
					}

					if( evt.getFlags() & Event::ButtonRight){
						
						rewindingSoundInstance->setPosition( Vector3f(leftRightAnalog,1,0) );
						rewindingSoundInstance->setReverb( upDownAnalog, upDownAnalog );
						rewindingSoundInstance->play();
					}
					if( evt.getFlags() & Event::ButtonLeft){
						SoundInstance* soundInstance = new SoundInstance(hideMenuSound);
						soundInstance->setPosition( Vector3f(leftRightAnalog,1,0) );
						soundInstance->setReverb( upDownAnalog, upDownAnalog );
						soundInstance->play();
					}
					if( evt.getFlags() & Event::ButtonUp){
						SoundInstance* soundInstance = new SoundInstance(selectMenuSound);
						soundInstance->setPosition( Vector3f(leftRightAnalog,1,0) );
						soundInstance->setReverb( upDownAnalog, upDownAnalog );
						soundInstance->play();
					}
					if( evt.getFlags() & Event::ButtonDown){
						SoundInstance* soundInstance = new SoundInstance(scrollMenuSound);
						soundInstance->setPosition( Vector3f(leftRightAnalog,1,0) );
						soundInstance->setReverb( upDownAnalog, upDownAnalog );
						soundInstance->play();
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
	}
private:
	Vector3f position;
	float volume;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SoundTest app;

	// Read config file name from command line or use default one.
	const char* cfgName = "soundTest.cfg";
	if(argc == 2) cfgName = argv[1];

	Config* cfg = new Config(cfgName);

	DataManager* dm = DataManager::getInstance();
	// Add a default filesystem data source using current work dir.
	dm->addSource(new FilesystemDataSource("./"));
	dm->addSource(new FilesystemDataSource(OMICRON_DATA_PATH));

	ServiceManager* sm = new ServiceManager();
	sm->setupAndStart(cfg);

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
			app.update();
			//if( printOutput )
			//	printf("------------------------------------------------------------------------------\n");
		}
	}

	sm->stop();
	delete sm;
	delete cfg;
	delete dm;
	
	return 0;
}
