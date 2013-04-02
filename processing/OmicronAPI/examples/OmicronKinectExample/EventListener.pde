class EventListener implements OmicronListener{
 
  // This is called on every Omicron event
  public void onEvent( Event e ){
    
    // Here we process mocap data
    if( e.getServiceType() == OmicronAPI.ServiceType.MOCAP ){
      
      int userID = (int)e.getFloatData(0);
      int jointID = (int)e.getFloatData(1);
      
      Skeleton s = (Skeleton)userSkeletons.get(userID);
      
      if( s != null ){ //Existing user
        s.update(e);
        userSkeletons.put(userID, s);
      } else { // New user
        s= new Skeleton(userID);
        s.update(e);
        userSkeletons.put(userID, s);
        println("Added new user: " + userID);
      }
    } else if( e.getServiceType() == OmicronAPI.ServiceType.SPEECH ){ // Kinect speech recognition
      String speechText = e.getDataString();
      float speechConfidence = e.getFloatData(2);
      float speechAngle = e.getFloatData(3);
      float angleConfidence = e.getFloatData(4);
      
      println("Kinect speech event: " +speechText + " at angle " + speechAngle + " with speech confidence " + speechConfidence);
    } else {
      println("Unknown service type: " + e.getServiceType() );
    }
  }// OnEvent
  
}// EventListener
