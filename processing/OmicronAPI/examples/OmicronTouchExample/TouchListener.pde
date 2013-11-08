class TouchListener implements OmicronTouchListener{
 
  // Called on a touch down event
  // mousePressed events also call this with an ID of -1 and an xWidth and yWidth of 10.
  public void touchDown(int ID, float xPos, float yPos, float xWidth, float yWidth){
   applet.touchDown( ID, xPos, yPos, xWidth, yWidth );
  }// touchDown
  
  // Called on a touch move event
  // mouseDragged events also call this with an ID of -1 and an xWidth and yWidth of 10.
  public void touchMove(int ID, float xPos, float yPos, float xWidth, float yWidth){
    applet.touchMove( ID, xPos, yPos, xWidth, yWidth );
  }// touchMove
  
  // Called on a touch up event
  // mouseReleased events also call this with an ID of -1 and an xWidth and yWidth of 10.
  public void touchUp(int ID, float xPos, float yPos, float xWidth, float yWidth){
    applet.touchUp( ID, xPos, yPos, xWidth, yWidth );
  }// touchUp
  
}// TouchListener
