package omicronTester;

import omicronAPI.*;
import omicronUI.Button;
import processing.core.*;

public class OmicronTester extends PApplet
{
	OmicronAPI omicronManager;

	TouchListener touchListener;
	EventListener eventListener;

	PApplet applet;

	Button testButton;

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
		omicronManager.setFullscreen(false);
	}

	public void setup()
	{
		size(100, 100, PApplet.P2D); // 2.0a6 - 2.0b3 -
														// recommend manually
														// entering in
														// resolution

		// Make the connection to the tracker machine (uncomment this if testing
		// without touch)
		omicronManager.connectToTracker(7804, 28000, "131.193.77.211");

		// Create a listener to get events
		touchListener = new TouchListener();
		// eventListener = new EventListener();

		// Register listener with OmicronAPI
		omicronManager.setTouchListener(touchListener);
		// omicronManager.setEventListener(eventListener);

		// Scaling the application display:
		// Use this if you want to preview what you application would look like
		// on a different resolution.
		// Calculate the screen transformation *This must be called after
		// size()*
		omicronManager.calculateScreenTransformation(8160, 2304); // EVL Cybercommons wall

		// Enabled/disables the screen transformation.
		// This can be toggled during runtime
		omicronManager.enableScreenScale(true);

		// Sets applet to this sketch
		applet = this;

		testButton = new Button(this, width / 2, height / 2, 200, 100);
	}
	
	public void draw()
	{
		// Sets the background color
		background(24);

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

	// TouchListener
	// ------------------------------------------------------------------------------
	class TouchListener implements OmicronTouchListener
	{

		// Called on a touch down event
		// mousePressed events also call this with an ID of -1 and an xWidth and
		// yWidth of 10.
		public void touchDown(int ID, float xPos, float yPos, float xWidth, float yWidth)
		{
			fill(255, 0, 0);
			noStroke();
			ellipse(xPos, yPos, xWidth, yWidth);

			// This is an optional call if you want the function call in the
			// main applet class.
			// 'OmicronExample' should be replaced with the sketch name i.e.
			// ((SketchName)applet).touchDown( ID, xPos, yPos, xWidth, yWidth );
			// Make sure applet is defined as PApplet and that 'applet = this;'
			// is in setup().
			((OmicronTester) applet).touchDown(ID, xPos, yPos, xWidth, yWidth);
		}// touchDown

		// Called on a touch move event
		// mouseDragged events also call this with an ID of -1 and an xWidth and
		// yWidth of 10.
		public void touchMove(int ID, float xPos, float yPos, float xWidth, float yWidth)
		{
			fill(0, 255, 0);
			noStroke();
			ellipse(xPos, yPos, xWidth, yWidth);

			((OmicronTester) applet).touchMove(ID, xPos, yPos, xWidth, yWidth);
		}// touchMove

		// Called on a touch up event
		// mouseReleased events also call this with an ID of -1 and an xWidth
		// and yWidth of 10.
		public void touchUp(int ID, float xPos, float yPos, float xWidth, float yWidth)
		{
			fill(0, 0, 255);
			noStroke();
			ellipse(xPos, yPos, xWidth, yWidth);

			((OmicronTester) applet).touchUp(ID, xPos, yPos, xWidth, yWidth);
		}// touchUp

	}// TouchListener

	// EventListener
	// ------------------------------------------------------------------------------
	class EventListener implements OmicronListener
	{

		@Override
		public void onEvent(Event e)
		{
			// TODO Auto-generated method stub
			println("-----");
			println(e.getServiceType());
			println(e.getStringData(0));
		}

	}// EventListener

	/**
	 * @param args
	 */
	public static void main(String[] args)
	{
		// TODO Auto-generated method stub
		PApplet.main(new String[]
		{ OmicronTester.class.getName() });
	}

}
