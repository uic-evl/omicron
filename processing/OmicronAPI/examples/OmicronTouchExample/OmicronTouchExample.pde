/**
 * ---------------------------------------------
 * OmicronTouchExample.pde
 * Description: Omicron Processing example for running a fullscreen touch application on the Cyber-Commons wall.
 *       You should not use mousePressed() or any other mouse functions (mouseX, mouseY) for developing your application.
 *       Mouse events are sent to touchDown (mousePressed), touchMove (moudeDragged), and touchUp (mouseReleased)
 *
 * Class: 
 * System: Processing 2.1, SUSE 12.1, Windows 7
 * Author: Arthur Nishimoto
 * Version: 1.2
 *
 * Version Notes:
 * 6/14/12      - Initial version
 * 6/19/12      - Added example for fullscreen, scaling, and touch
 * 6/20/12      - Cleaned up example
 * 9/20/12      - Updated for 2.0b3
 * 11/8/13      - Cleaned up example for Processing 2.1
 * ---------------------------------------------
 */

import processing.net.*;
import omicronAPI.*;

OmicronAPI omicronManager;
TouchListener touchListener;

// Link to this Processing applet - used for touchDown() callback example
// applet should always be of type of the sketchname i.e. if this file is called MySketch: 'MySketch applet;'
OmicronTouchExample applet;

boolean usingWall = false;
String touchServerIP = "127.0.0.1";
int dataPort = 7001;
int msgPort = 7340;

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
  // For almost any Processing application size() should be called before anything else in setup()
  if( usingWall )
  {
    // Cyber-Commons wall
    //   - Processing 2.0b6 or below and the P3D renderer is recommended for running on the wall
    size( 8160, 2304, P3D );
  }
  else
    size( displayWidth, displayHeight );
  
  // Make the connection to the tracker machine (Comment this out if testing with only mouse)
  println("Attempting connection to '"+touchServerIP+"' on msgport: "+ msgPort+"...");
  if( omicronManager.connectToTracker(dataPort, msgPort, touchServerIP) )
    println("Connected to '"+touchServerIP+"' receiving data on port: "+ dataPort);
  
  // Create a listener to get events
  touchListener = new TouchListener();
  
  // Register listener with OmicronAPI (This will still get mouse input without connecting to the tracker)
  omicronManager.setTouchListener(touchListener);

  // Sets applet to this sketch
  applet = this;
}// setup

void draw() {
  // Sets the background color
  background(24);

  // For event and fullscreen processing, this must be called in draw()
  // For touch display to appear, this should be called at the end of draw()
  omicronManager.process();
}// draw

// See TouchListener on how these functions are called
// NOTE: Mouse pressed, dragged, and released events will also trigger these
//       using an ID of -1 and an xWidth and yWidth value of 10.
void touchDown(int ID, float xPos, float yPos, float xWidth, float yWidth){
  noFill();
  stroke(255,0,0);
  ellipse( xPos, yPos, xWidth, yWidth );
}// touchDown

void touchMove(int ID, float xPos, float yPos, float xWidth, float yWidth){
  noFill();
  stroke(0,255,0);
  ellipse( xPos, yPos, xWidth, yWidth );
}// touchMove

void touchUp(int ID, float xPos, float yPos, float xWidth, float yWidth){
  noFill();
  stroke(0,0,255);
  ellipse( xPos, yPos, xWidth, yWidth );
}// touchUp
