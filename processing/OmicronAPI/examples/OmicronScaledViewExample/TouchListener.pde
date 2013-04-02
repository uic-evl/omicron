class TouchListener implements OmicronTouchListener{
 
  // Called on a touch down event
  // mousePressed events also call this with an ID of -1 and an xWidth and yWidth of 10.
  public void touchDown(int ID, float xPos, float yPos, float xWidth, float yWidth){
    fill(255,0,0);
    noStroke();
    ellipse( xPos, yPos, xWidth, yWidth );
    
    // This is an optional call if you want the function call in the main applet class.
    // 'OmicronExample' should be replaced with the sketch name i.e. ((SketchName)applet).touchDown( ID, xPos, yPos, xWidth, yWidth );
    // Make sure applet is defined as PApplet and that 'applet = this;' is in setup().
    ((OmicronScaledViewExample)applet).touchDown( ID, xPos, yPos, xWidth, yWidth );
  }// touchDown
  
  // Called on a touch move event
  // mouseDragged events also call this with an ID of -1 and an xWidth and yWidth of 10.
  public void touchMove(int ID, float xPos, float yPos, float xWidth, float yWidth){
    fill(0,255,0);
    noStroke();
    ellipse( xPos, yPos, xWidth, yWidth );
    
    ((OmicronScaledViewExample)applet).touchMove( ID, xPos, yPos, xWidth, yWidth );
  }// touchMove
  
  // Called on a touch up event
  // mouseReleased events also call this with an ID of -1 and an xWidth and yWidth of 10.
  public void touchUp(int ID, float xPos, float yPos, float xWidth, float yWidth){
    fill(0,0,255);
    noStroke();
    ellipse( xPos, yPos, xWidth, yWidth );
    
    ((OmicronScaledViewExample)applet).touchUp( ID, xPos, yPos, xWidth, yWidth );
  }// touchUp
  
}// TouchListener
