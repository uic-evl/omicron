/**
 * ---------------------------------------------
 * OmicronScaledViewExample.pde
 * Description: Shows how to use Omicron's scaled view mode which
 *  allows for testing the sketch using a custom resolution
 *  like the Cybercommons wall on a smaller display.
 *
 *  In this example toggle view mode by pressing 'v'
 *  In view mode the screen will mode around using standard map
 *  controls:
 *    Click-drag to move the view around
 *    Mouse wheel to zoom in/out
 *
 *  Pressing 's' will toggle screen scaling on/off
 *  Pressing 'r' while in scaling is enabled will reset the view
 *
 *  Also includes a very basic button class used to test the
 *  scaled screen / interaction mapping.
 *
 * Class: 
 * System: Processing 2.0b6, SUSE 12.1, Windows 7 x64
 * Author: Arthur Nishimoto
 * Version: 0.1 (alpha)
 *
 * Version Notes:
 * 11/6/12      - Initial version
 * ---------------------------------------------
 */

import processing.net.*;
import omicronAPI.*;

OmicronAPI omicronManager;

TouchListener touchListener;
EventListener eventListener;

PApplet applet;

Button testButton;

boolean usingWall = false;

boolean scaleScreen = true;
boolean scaledViewMode = false;

// Override of PApplet init() which is called before setup()
// Typically you do not want to put anything else in init()
public void init()
{
  super.init();

  // Creates the OmicronAPI object. This is placed in init() since we want
  // to use fullscreen
  omicronManager = new OmicronAPI(this);

  // Removes the title bar for full screen mode (present mode will not
  // work on Cyber-commons wall)
  if( usingWall )
    omicronManager.setFullscreen(true);
}

public void setup()
{
  if( usingWall )
    size(8160, 2394, P2D);
  else
    size(displayWidth, displayHeight, P2D);

  // Make the connection to the tracker machine (uncomment this if testing without touch)
  // omicronManager.connectToTracker(7001, 7340, "localhost");

  // Create a listener to get events
  touchListener = new TouchListener();

  // Register listener with OmicronAPI
  omicronManager.setTouchListener(touchListener);

  // Enabled/disables the screen transformation.
  // This can be toggled during runtime
  omicronManager.enableScreenScale(scaleScreen);
  
  // Scaling the application display:
  // Use this if you want to preview what you application would look like
  // on a different resolution.
  // Calculate the screen transformation *This must be called after size() and enableScreenScale()*
  // If this is called on the wall it will scale to 1 with offset 0,0 (i.e. does nothing)
  omicronManager.calculateScreenTransformation(8160, 2304); // EVL Cybercommons wall

  // Sets applet to this sketch
  applet = this;

  testButton = new Button(width / 2, height / 2, 200, 100);
}

public void draw()
{
  // Sets the background color
  if( scaledViewMode )
    background(24,24,50); // Make the background different in view mode
  else
    background(24);
  
  // Toggle the interaction mode
  omicronManager.setScaledViewMode( scaledViewMode );
  
  // Everything after between this line and popScreenScale() will be scale
  // according to the transformation
  omicronManager.pushScreenScale();

  // For example we draw a black background representing the visible space
  // on the Cybercommons wall
  fill(0);
  noStroke();
  rect(0, 0, 8160, 2304);

  testButton.draw();

  // For event and fullscreen processing, this must be called in draw()
  omicronManager.process();

  // Everything after this will not be scaled
  omicronManager.popScreenScale();
}

void keyReleased() {
  if(key == 'v') // Toggle scale view mode
    scaledViewMode = !scaledViewMode;
  if(key == 'r') // Reset scaled view
    omicronManager.calculateScreenTransformation(8160, 2304); // EVL Cybercommons wall
  if(key == 's'){
    scaleScreen = !scaleScreen;
    omicronManager.enableScreenScale(scaleScreen);
  }
}

public void touchDown(float ID, float xPos, float yPos, float xWidth, float yWidth)
{
  testButton.isPressed(xPos, yPos);
}

public void touchMove(float ID, float xPos, float yPos, float xWidth, float yWidth)
{
  testButton.isPressed(xPos, yPos);
}

public void touchUp(float ID, float xPos, float yPos, float xWidth, float yWidth)
{
}

