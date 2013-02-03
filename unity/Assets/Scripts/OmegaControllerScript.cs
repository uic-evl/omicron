/**
 * ---------------------------------------------
 * OmegaControllerScript.cs
 * Description: Manages InputServiceScript controller data.
 * 		For First Person Controller prefab: Can be used to replace FPSInputController.js and MouseLook.js.
 * 
 * Copyright 2010, Electronic Visualization Laboratory, University of Illinois at Chicago.
 * Author(s): Arthur Nishimoto
 * ---------------------------------------------
 */
 
using UnityEngine;
using System.Collections;

public abstract class OmegaControllerScript : MonoBehaviour {
	public int controllerID = -1; // Controller ID this object accepts
	
	public float analogStick0Sensitivity = 1.0f;
	public bool analogStick0InvertY = false;
	public float analogStick1Sensitivity = 1.0f;
	public bool analogStick1InvertY = false;
	
	public int controllerType = -1;
	private static int XBOX360 = 0;
	private static int PS3 = 1;
	private static int WIIMOTE = 2;
	private static int WII_NUNCHUK = 3;
	private static int WII_MOTIONPLUS = 4;
	
	private Vector3 analogStick0 = new Vector3();
	public Vector3 analogStick0Deadzone = new Vector3(300,300,300);
	private Vector3 analogStick1 = new Vector3();
	public Vector3 analogStick1Deadzone = new Vector3(300,300,300);
	
	private float triggers = 0;
	public float triggerSensitivity = 1.0f;
	
	private int[] buttons = new int[16]; // Raw button data: 0(not pressed), 1(pressed)
	private int[] buttonStates = new int[16]; // Button state: 0(not pressed), 1(pressed), 2(held), 3(released)
	
	private int DPadUp = 0;
	private int DPadLeft = 0;
	private int DPadRight = 0;
	private int DPadDown = 0;
	
	public enum Button : int{ A, B, X, Y, LB, RB, Back, Start, LA, RA, Up, Down, Left, Right, Cross, Circle, Square, Triangle, L1, R1, Select, L3, R3, PS, Plus, Home, Minus, One, Two, C, Z };
	
	private Vector3 accelerometer0 = new Vector3();
	private Vector3 accelerometer1 = new Vector3();
	private Vector3 accelerometer2 = new Vector3();
	
	// Wiimote Specific Parameters
	private int batteryPercent = -1;
	private int IRMode = -1;
	private Vector2 IRDot0 = new Vector2();
	private Vector2 IRDot1 = new Vector2();
	private Vector2 IRDot2 = new Vector2();
	private Vector2 IRDot3 = new Vector2();
	private int IRDot0Valid = 0;
	private int IRDot1Valid = 0;
	private int IRDot2Valid = 0;
	private int IRDot3Valid = 0;
	
	void Start () {
		// Reserved for derived classes
	}

	void Update () {
		// Reserved for derived classes
	}
	
	public void UpdateControllerData( string[] words ){
		//Debug.Log(words);
		int ID = int.Parse(words[1]); // Controller ID
		
		if( ID != controllerID )
			return;
		
		controllerType = int.Parse(words[2]); // What kind of controller (360,PS3,Wiimote,etc.)
		
		if( controllerType == XBOX360 || controllerType == PS3 ){
			// Analog sticks
			analogStick0 = ProcessAnalogStick( int.Parse(words[3]), int.Parse(words[4]), analogStick0Deadzone, analogStick0InvertY, analogStick0Sensitivity );
			analogStick1 = ProcessAnalogStick( int.Parse(words[5]), int.Parse(words[6]), analogStick1Deadzone, analogStick1InvertY, analogStick1Sensitivity );
			triggers = int.Parse(words[7]) / 1000.0f * triggerSensitivity;
			
			// Buttons
			for( int i = 0; i < 13; i++ ){
				buttons[i] = int.Parse(words[8+i]);
				
				// Set button state
				if( buttonStates[i] == 0 && buttons[i] == 1 ){
					buttonStates[i] = 1; // Not Pressed->Pressed
				} else if( buttonStates[i] == 1 && buttons[i] == 1 ){
					buttonStates[i] = 2; // Pressed->Held
				} else if( buttonStates[i] == 2 && buttons[i] == 0 ){
					buttonStates[i] = 3; // Held->Released
				} else if( buttonStates[i] == 1 && buttons[i] == 0 ){
					buttonStates[i] = 3; // Pressed->Released
				} else if( buttonStates[i] == 3 && buttons[i] == 0 ){
					buttonStates[i] = 0; // Released->Not Pressed
				}
			}
			
			// DPad
			int DPad = int.Parse(words[21]);
			if( controllerType == XBOX360 ){
				DPadUp = 0;
				DPadDown = 0;
				DPadLeft = 0;
				DPadRight = 0;
				if( DPad == 0 || DPad == 4500 || DPad == 31500 )
					DPadUp = 1;
				if( DPad == 13500 || DPad == 18000 || DPad == 22500 )
					DPadDown = 1;
				if( DPad == 4500 || DPad == 9000 || DPad == 13500 )
					DPadRight = 1;
				if( DPad == 22500 || DPad == 27000 || DPad == 31500 )
					DPadLeft = 1;
			} else if( controllerType == PS3 ){
				DPadUp = 0;
				DPadDown = 0;
				DPadLeft = 0;
				DPadRight = 0;
				if( DPad == 0 || DPad == 31500 )
					DPadUp = 1;
				if( DPad == 9000 || DPad == 13500 || DPad == 22500 )
					DPadDown = 1;
				if( DPad == 27000 || DPad == 31500 )
					DPadRight = 1;
				if( DPad == 13500 || DPad == 18000 )
					DPadLeft = 1;
				
				float roll = int.Parse(words[22]) / 1000.0f;
				float pitch = int.Parse(words[23]) / 1000.0f;
				
				accelerometer0 = new Vector3( pitch, 0, roll );
			}
		}else if( controllerType == WIIMOTE || controllerType == WII_NUNCHUK || controllerType == WII_MOTIONPLUS ){
			if( controllerType == WIIMOTE ){
				batteryPercent = int.Parse(words[3]);
				
				// Accelerometer
				float rawXAccel = int.Parse(words[4]) / 1000.0f;
				float rawYAccel = int.Parse(words[5]) / 1000.0f;
				float rawZAccel = int.Parse(words[6]) / 1000.0f;
				
				//int extension = int.Parse(words[17]);
				
				accelerometer0 = new Vector3( rawXAccel, rawYAccel, rawZAccel );
				
				// MotionPlus
				//float rawXAccelPlus = int.Parse(words[20]) / 1000.0f;
				//float rawYAccelPlus = int.Parse(words[21]) / 1000.0f;
				//float rawZAccelPlus = int.Parse(words[22]) / 1000.0f;
								
				//accelerometer2 = new Vector3( rawXAccelPlus, rawYAccelPlus, rawZAccelPlus );
				
				// Buttons
				for( int i = 0; i < 11; i++ ){
					buttons[i] = int.Parse(words[7+i]);
					
					// Set button state
					if( buttonStates[i] == 0 && buttons[i] == 1 ){
						buttonStates[i] = 1; // Not Pressed->Pressed
					} else if( buttonStates[i] == 1 && buttons[i] == 1 ){
						buttonStates[i] = 2; // Pressed->Held
					} else if( buttonStates[i] == 2 && buttons[i] == 0 ){
						buttonStates[i] = 3; // Held->Released
					} else if( buttonStates[i] == 1 && buttons[i] == 0 ){
						buttonStates[i] = 3; // Pressed->Released
					} else if( buttonStates[i] == 3 && buttons[i] == 0 ){
						buttonStates[i] = 0; // Released->Not Pressed
					}
				}
				
				IRMode = int.Parse(words[18]);
				IRDot0Valid = int.Parse(words[19]);
				IRDot0.x = int.Parse(words[20]) / 1000.0f;
				IRDot0.y = int.Parse(words[21]) / 1000.0f;
				
				IRDot1Valid = int.Parse(words[22]);
				IRDot1.x = int.Parse(words[23]) / 1000.0f;
				IRDot1.y = int.Parse(words[24]) / 1000.0f;
				
				IRDot2Valid = int.Parse(words[25]);
				IRDot2.x = int.Parse(words[26]) / 1000.0f;
				IRDot2.y = int.Parse(words[27]) / 1000.0f;
				
				IRDot3Valid = int.Parse(words[28]);
				IRDot3.x = int.Parse(words[29]) / 1000.0f;
				IRDot3.y = int.Parse(words[30]) / 1000.0f;
			} else if( controllerType == WII_NUNCHUK ){
				analogStick0 = ProcessAnalogStick( int.Parse(words[3]), int.Parse(words[4]), analogStick0Deadzone, analogStick0InvertY, analogStick0Sensitivity );
				
				// Accelerometer
				float rawXAccel = int.Parse(words[5]) / 1000.0f;
				float rawYAccel = int.Parse(words[6]) / 1000.0f;
				float rawZAccel = int.Parse(words[7]) / 1000.0f;
				
				accelerometer1 = new Vector3( rawXAccel, rawYAccel, rawZAccel );
				
				// Buttons
				// Note nunchuk buttons are placed in unused Wiimote buttons[] index 11 and 12.
				for( int i = 0; i < 2; i++ ){
					buttons[11+i] = int.Parse(words[8+i]);
					
					// Set button state
					if( buttonStates[i] == 0 && buttons[i] == 1 ){
						buttonStates[i] = 1; // Not Pressed->Pressed
					} else if( buttonStates[i] == 1 && buttons[i] == 1 ){
						buttonStates[i] = 2; // Pressed->Held
					} else if( buttonStates[i] == 2 && buttons[i] == 0 ){
						buttonStates[i] = 3; // Held->Released
					} else if( buttonStates[i] == 1 && buttons[i] == 0 ){
						buttonStates[i] = 3; // Pressed->Released
					} else if( buttonStates[i] == 3 && buttons[i] == 0 ){
						buttonStates[i] = 0; // Released->Not Pressed
					}
				}
			}// Wiimote/Nunchuk
			 else if( controllerType == WII_MOTIONPLUS ){
				// MotionPlus
				float rawXAccelPlus = int.Parse(words[3]) / 1000.0f;
				float rawYAccelPlus = int.Parse(words[4]) / 1000.0f;
				float rawZAccelPlus = int.Parse(words[5]) / 1000.0f;
								
				accelerometer2 = new Vector3( rawXAccelPlus, rawYAccelPlus, rawZAccelPlus );
			}// Wiimote/Nunchuk
		}// controllerType
	}
	
	// ProcessAnalogStick ------------------------------------------------------
	private Vector3 ProcessAnalogStick( int analogX, int analogY, Vector3 deadzone, bool invert, float sensitivity ){
		Vector3 analogStick = new Vector3();
		// Analog Stick 0 ------------------------------------------------------
		if( Mathf.Abs(analogX) <= deadzone.x ){
			analogX = 0;
		} else {
			analogX -= (int)deadzone.x;
		}
		if( Mathf.Abs(analogY) <= deadzone.y ){
			analogY = 0;
		} else {
			analogY -= (int)deadzone.y;
		}
		
		if( invert )
			analogY *= -1;

		analogStick = new Vector3( analogX, analogY, 0 );
		analogStick = analogStick / 1000.0f * sensitivity;
		return analogStick;
	}
	
	// Getters/Setters ------------------------------------------------------
	public bool isXbox360(){
		if( controllerType == XBOX360 )
			return true;
		else
			return false;
	}
	
	public bool isPS3(){
		if( controllerType == PS3 )
			return true;
		else
			return false;
	}
	
	public bool isWiimote(){
		if( controllerType == WIIMOTE || controllerType == WII_NUNCHUK || controllerType == WII_MOTIONPLUS )
			return true;
		else
			return false;
	}
	
	public bool isWiiNunchuk(){
		if( controllerType == WII_NUNCHUK )
			return true;
		else
			return false;
	}
	
	public bool isWiiMotionPlus(){
		if( controllerType == WII_MOTIONPLUS )
			return true;
		else
			return false;
	}
	
	public Vector3 getAnalogStick0(){
		return analogStick0;
	}
	
	public Vector3 getAnalogStick1(){
		return analogStick1;
	}
	
	public Vector3 getAccelerometer0(){
		return accelerometer0;
	}
	
	public Vector3 getAccelerometer1(){
		return accelerometer1;
	}
	
	public Vector3 getAccelerometer2(){
		return accelerometer2;
	}
	
	public float getTrigger(){
		return triggers;
	}
	
	public int getButton( Button button ){
		if( isXbox360() ){
			switch((int)button){
				case( (int)Button.A ): return buttons[0];
				case( (int)Button.B ): return buttons[1];
				case( (int)Button.X ): return buttons[2];
				case( (int)Button.Y ): return buttons[3];
				case( (int)Button.LB ): return buttons[4];
				case( (int)Button.RB ): return buttons[5];
				case( (int)Button.Back ): return buttons[6];
				case( (int)Button.Start ): return buttons[7];
				case( (int)Button.LA ): return buttons[8];
				case( (int)Button.RA ): return buttons[9];
				case( (int)Button.Up ): return DPadUp;
				case( (int)Button.Down ): return DPadDown;
				case( (int)Button.Left ): return DPadLeft;
				case( (int)Button.Right ): return DPadRight;
				default: return -1;
			}// switch
		} else if( isPS3() ){
			switch((int)button){
				case( (int)Button.Cross ):
				case( (int)Button.X ):
					return buttons[0];
				case( (int)Button.Circle ): return buttons[1];
				case( (int)Button.Square ): return buttons[2];
				case( (int)Button.Triangle ): return buttons[3];
				case( (int)Button.L1 ): return buttons[4];
				case( (int)Button.R1 ): return buttons[5];
				case( (int)Button.Select ): return buttons[6];
				case( (int)Button.Start ): return buttons[7];
				case( (int)Button.L3 ): return buttons[8];
				case( (int)Button.R3 ): return buttons[9];
				case( (int)Button.PS ): return buttons[10];
				case( (int)Button.Up ): return DPadUp;
				case( (int)Button.Down ): return DPadDown;
				case( (int)Button.Left ): return DPadLeft;
				case( (int)Button.Right ): return DPadRight;
				default: return -1;
			}// switch
		} else if( isWiimote() ){
			switch((int)button){
				case( (int)Button.A ):return buttons[0];
				case( (int)Button.B ): return buttons[1];
				case( (int)Button.Plus ): return buttons[2];
				case( (int)Button.Home ): return buttons[3];
				case( (int)Button.Minus ): return buttons[4];
				case( (int)Button.One ): return buttons[5];
				case( (int)Button.Two ): return buttons[6];
				case( (int)Button.Up ): return buttons[7];
				case( (int)Button.Down ): return buttons[8];
				case( (int)Button.Left ): return buttons[9];
				case( (int)Button.Right ): return buttons[10];
				case( (int)Button.C ): return buttons[11];
				case( (int)Button.Z ): return buttons[12];
				default: return -1;
			}// switch
		}
		
		return -1;
	}
	
	// TODO: Not fully implemented
	public int getButtonState( int button ){
		if( isXbox360() ){
			switch(button){
				case( (int)Button.A ): return buttonStates[0];
				case( (int)Button.B ): return buttonStates[1];
				case( (int)Button.X ): return buttonStates[2];
				case( (int)Button.Y ): return buttonStates[3];
				case( (int)Button.LB ): return buttonStates[4];
				case( (int)Button.RB ): return buttonStates[5];
				case( (int)Button.Back ): return buttonStates[6];
				case( (int)Button.Start ): return buttonStates[7];
				case( (int)Button.LA ): return buttonStates[8];
				case( (int)Button.RA ): return buttonStates[9];
				default: return -1;
			}// switch
		} else if( isPS3() ){
			switch(button){
				case( (int)Button.Cross ):
				case( (int)Button.X ):
					return buttonStates[0];
				case( (int)Button.Circle ): return buttonStates[1];
				case( (int)Button.Square ): return buttonStates[2];
				case( (int)Button.Triangle ): return buttonStates[3];
				case( (int)Button.L1 ): return buttonStates[4];
				case( (int)Button.R1 ): return buttonStates[5];
				case( (int)Button.Select ): return buttonStates[6];
				case( (int)Button.Start ): return buttonStates[7];
				case( (int)Button.L3 ): return buttonStates[8];
				case( (int)Button.R3 ): return buttonStates[9];
				case( (int)Button.PS ): return buttonStates[10];
				default: return -1;
			}// switch
		}
		
		return -1;
	}
	
	// Generic Setters ------------------------------------------------------------
	public void setAnalogStick0Sensitivity( float value ){
		analogStick0Sensitivity = value;
	}
	
	public void setAnalogStick1Sensitivity( float value ){
		analogStick1Sensitivity = value;
	}
	
	public void setAnalogStick0Deadzone( Vector3 value ){
		analogStick0Deadzone = value;
	}
	
	public void setAnalogStick1Deadzone( Vector3 value ){
		analogStick1Deadzone = value;
	}
	
	public void setTriggerSensitivity( float value ){
		triggerSensitivity = value;
	}
	
	// Controller Specific Convenience Setters ------------------------------------
	public void setLeftAnalogStickSensitivity( float value ){
		setAnalogStick0Sensitivity(value);
	}
	
	public void setRightAnalogStickSensitivity( float value ){
		setAnalogStick1Sensitivity(value);
	}
	
	public void setNunchukAnalogStickSensitivity( float value ){
		setAnalogStick0Sensitivity(value);
	}
	
	public void setLeftAnalogStickDeadzone( Vector3 value ){
		setAnalogStick0Deadzone(value);
	}
	
	public void setRightAnalogStickDeadzone( Vector3 value ){
		setAnalogStick1Deadzone(value);
	}
	
	public void setNunchukAnalogStickDeadzone( Vector3 value ){
		setAnalogStick0Deadzone(value);
	}
	
	// Controller Specific Convenience Getters ------------------------------------
	// PS3/360
	public Vector3 getLeftAnalogStick(){
		return getAnalogStick0();
	}
	
	// PS3/360
	public Vector3 getRightAnalogStick(){
		return getAnalogStick1();
	}
	
	// PS3
	public float getSixaxisPitch(){
		return getAccelerometer0().x;
	}
	
	// PS3
	public float getSixaxisRoll(){
		return getAccelerometer0().z;
	}
	
	// Wii
	public int getBatteryPercent(){
		return batteryPercent;
	}
	
	// Wii
	public Vector3 getNunchukAnalogStick(){
		return getAnalogStick0();
	}
	
	// Wii
	public Vector3 getWiimoteAcceleration(){
		return getAccelerometer0();
	}
	
	// Wii
	public Vector3 getWiiMotionPlusAcceleration(){
		return getAccelerometer1();
	}
	
	// Wii
	public int getIRMode(){
		return IRMode;
	}
	
	// Wii
	public bool isIRDot0Valid(){
		if( IRDot0Valid == 1 )
			return true;
		else
			return false;
	}
	
	// Wii
	public Vector2 getIRDot0(){
		return IRDot0;
	}
	
	// Wii
	public bool isIRDot1Valid(){
		if( IRDot1Valid == 1 )
			return true;
		else
			return false;
	}
	
	// Wii
	public Vector2 getIRDot1(){
		return IRDot1;
	}
	
	// Wii
	public bool isIRDot2Valid(){
		if( IRDot2Valid == 1 )
			return true;
		else
			return false;
	}
	
	// Wii
	public Vector2 getIRDot2(){
		return IRDot2;
	}
	
	// Wii
	public bool isIRDot3Valid(){
		if( IRDot3Valid == 1 )
			return true;
		else
			return false;
	}
	
	// Wii
	public Vector2 getIRDot3(){
		return IRDot3;
	}
}
