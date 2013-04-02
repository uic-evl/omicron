/**
 * ---------------------------------------------
 * Skeleton.pde
 * Description: A simple skeleton class to store and display Kinect joint data
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

class Skeleton {
  int skeletonID = -1;

  PVector[] joints;
  int[] jointsState;
  
   /* There are 20 joints (using Kinect for Windows SDK joint IDs)
   HipCenter = 0,
   Spine = 1,
   ShoulderCenter = 2,
   Head = 3,
   ShoulderLeft = 4,
   ElbowLeft = 5,
   WristLeft = 6,
   HandLeft = 7,
   ShoulderRight = 8,
   ElbowRight = 9,
   WristRight = 10,
   HandRight = 11,
   HipLeft = 12,
   KneeLeft = 13,
   AnkleLeft = 14,
   FootLeft = 15,
   HipRight = 16,
   KneeRight = 17,
   AnkleRight = 18,
   FootRight = 19,
   */

  float lastUpdateTime;
  float timeSinceLastUpdate;

  float headAngle;

  // Joint states
  final int NO_DATA = -1; // No joint data was received
  final int ACTIVE = 0; // Joint has been recently updated
  final int INACTIVE = 1;  // Joint has not been updated recently

  public Skeleton() {
    skeletonID = -1;
    init();
  }

  public Skeleton( int ID ) {
    skeletonID = ID;
    init();
  }

  void init() {
    joints = new PVector[20];
    jointsState = new int[20];

    for ( int i = 0; i < 20; i++ ) {
      joints[i] = new PVector();
      jointsState[i] = -1;
    }
  }

  PVector transform = new PVector( width/2, height/2 );
  float movementScale = 1000;
  void draw() {
    timeSinceLastUpdate = programTimer - lastUpdateTime;

    color idColor = getColor(skeletonID);
    float fadeAmount = (5.0 - timeSinceLastUpdate) / 5.0;

    for ( int i = 0; i < 20; i++ ) {
      if ( jointsState[i] != NO_DATA ) {

        float xPos = joints[i].x * movementScale + transform.x;
        float yPos = height - (joints[i].y * movementScale + transform.y);

        //println( i + ": " + xPos + " " + yPos );


        fill( red(idColor), green(idColor), blue(idColor), 255.0 * fadeAmount);
        noStroke();
        ellipse( xPos, yPos, 30, 30 );

        textAlign(CENTER);
        fill(0);
        text( i, xPos, yPos );
      }
    }
  }

  void update( Event e ) {
    lastUpdateTime = programTimer;

    int jointID = e.getSourceID();
    float xPos = e.getXPos();
    float yPos = e.getYPos();
    float zPos = e.getZPos();

    PVector pos = new PVector(xPos, yPos, zPos);
    joints[jointID] = pos;
    jointsState[jointID] = ACTIVE;
  }
}

