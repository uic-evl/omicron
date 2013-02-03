/**
 * ---------------------------------------------
 * OmicronFullscreenExample.pde
 * Description: Omicron Processing example for running a fullscreen application on the Cyber-Commons wall.
 *
 * Class: 
 * System: Processing 2.0b3, SUSE 12.1, Windows 7
 * Author: Arthur Nishimoto
 * Version: 1.1
 *
 * Version Notes:
 * 6/14/12      - Initial version
 * 6/19/12      - Added example for fullscreen, scaling, and touch
 * 6/20/12      - Cleaned up example
 * 9/20/12      - Updated for 2.0b3
 * ---------------------------------------------
 */

import processing.net.*;
import omicronAPI.*;

OmicronAPI omicronManager;

boolean usingWall = false;

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
    size( 8160, 2304, P3D ); // Cyber-Commons wall (P3D renderer is recommended for running on the wall)
  else
    size( displayWidth, displayHeight, P3D );

}// setup

void draw() {
  // Sets the background color
  background(24);

  // For event and fullscreen processing, this must be called in draw()
  omicronManager.process();
}// draw
