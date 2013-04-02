package omicronUI;

import processing.core.*;

public class Button extends PApplet
{
	PVector position;
	PVector buttonSize;
	
	int buttonColor = color(25);
	int buttonStroke = color(50);
	int buttonPressedColor = color(10, 228, 228);
	int buttonPressedStroke = color(50);
	
	boolean pressed = false;

	float timeSinceLastTouch;
	float pressedTimeout = 250; // milliseconds
	
	PApplet applet;

	public Button(PApplet parent, int xPos, int yPos, int width, int height)
	{
		position = new PVector(xPos, yPos);
		buttonSize = new PVector(width, height);

		applet = parent;
	}// CTOR

	public void draw()
	{
		if (pressed)
		{
			applet.fill(buttonPressedColor);
			applet.stroke(buttonPressedStroke);
		}
		else
		{
			applet.fill(buttonColor);
			applet.stroke(buttonStroke);
		}

		applet.rect(position.x, position.y, buttonSize.x, buttonSize.y);

		timeSinceLastTouch = applet.millis();

		if (pressed && timeSinceLastTouch > pressedTimeout)
			pressed = false;
	}// draw

	public boolean isPressed(float xPos, float yPos)
	{
		if (xPos >= position.x && xPos <= position.x + buttonSize.x && yPos >= position.y && yPos <= position.y + buttonSize.y)
		{
			timeSinceLastTouch = 0;
			pressed = true;
			return true;
		}

		pressed = false;
		return false;
	}// isPressed

}
