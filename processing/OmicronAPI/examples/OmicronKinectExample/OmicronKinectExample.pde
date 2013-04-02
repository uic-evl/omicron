/**
 * ---------------------------------------------
 * OmicronKinectExample.pde
 * Description: Omicron Processing Kinect example.
 *
 * Class: 
 * System: Processing 2.0a5, SUSE 12.1, Windows 7 x64
 * Author: Arthur Nishimoto
 * Version: 0.1 (alpha)
 *
 * Version Notes:
 * 8/3/12      - Initial version
 * ---------------------------------------------
 */

import processing.net.*;
import omicronAPI.*;

OmicronAPI omicronManager;

EventListener eventListener;
Hashtable userSkeletons;

PFont font;
float programTimer;
float startTime;

// Override of PApplet init() which is called before setup()
public void init() {
  super.init();

  // Creates the OmicronAPI object. This is placed in init() since we want to use fullscreen
  omicronManager = new OmicronAPI(this);

  // Removes the title bar for full screen mode (present mode will not work on Cyber-commons wall)
  omicronManager.setFullscreen(true);
}

// Program initializations
void setup() {
  startTime = millis() / 1000.0;
  
  // Depending on which version of Processing you're using:
  //size( screen.width, screen.height, OPENGL ); // 1.5.1
  //size( screenWidth, screenHeight, P3D ); // 2.0a1 - 2.0a5
  //size( displayWidth, displayHeight, P3D ); // 2.0a6+ - this is broken in 2.0a8 - recommend manually entering in resolution
  size( 8160, 2304, P3D ); // Cyber-Commons wall

  // Make the connection to the tracker machine
  omicronManager.connectToTracker(7000, 7340, "localhost");
  
  // Create a listener to get events
  eventListener = new EventListener();
  
  omicronManager.setEventListener( eventListener );
  
  font = loadFont("ArialMT-48.vlw");
  textFont( font, 16 );
  
  userSkeletons = new Hashtable();
}// setup

void draw() {
  programTimer = millis() / 1000.0;
  
  // Sets the background color
  background(24);
  
  Enumeration e = userSkeletons.elements();
  while( e.hasMoreElements() ){
    Skeleton s = (Skeleton)e.nextElement();
    s.draw();
  }
    
  // For event and fullscreen processing, this must be called in draw()
  omicronManager.process();
}// draw

color getColor(int finger){
  int colorNum = finger % 20;
  color shapeColor = #000000;
  
  switch (colorNum){
  case 0: 
    shapeColor = #D2691E; break;  //chocolate
  case 1: 
    shapeColor = #FC0FC0; break;  //Shocking pink
  case 2:
    shapeColor = #014421; break;  //Forest green (traditional)
  case 3: 
    shapeColor = #FF4500; break;  //Orange Red
  case 4: 
    shapeColor = #2E8B57; break;  //Sea Green        
  case 5: 
    shapeColor = #B8860B; break;  //Dark Golden Rod
  case 6: 
    shapeColor = #696969; break;  //Dim Gray
  case 7: 
    shapeColor = #7CFC00; break;  //Lawngreen
  case 8: 
    shapeColor = #4B0082; break;  //Indigo
  case 9: 
    shapeColor = #6B8E23; break;  //Olive Drab
  case 10: 
    shapeColor = #5D8AA8; break;  //Air Force Blue
  case 11: 
    shapeColor = #F8F8FF; break;  //Ghost White
  case 12: 
    shapeColor = #0000FF; break;  //Ao
  case 13: 
    shapeColor = #00FFFF; break;  //Aqua
  case 14: 
    shapeColor = #7B1113; break;  //UP Maroon
  case 15: 
    shapeColor = #6D351A; break;  //Auburn
  case 16: 
    shapeColor = #FDEE00; break;  //Aureolin
  case 17: 
    shapeColor = #FF0000; break;  //Red
  case 18:
    shapeColor = #0F4D92; break; //Yale Blue
  case 19:
    shapeColor = #C5B358; break; //Vegas gold
  }

  return shapeColor;
}
