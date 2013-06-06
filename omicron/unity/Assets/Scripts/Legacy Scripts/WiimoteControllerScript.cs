using UnityEngine;
using System.Collections;

public class WiimoteControllerScript : OmegaControllerScript {
	public int battery;
	public Vector3 analogStick;
	
	public Vector3 accelerometerVal0;
	public float accelerometerMagnitude;
	
	public Vector3 accelerometerVal1;
	
	public bool motionPlusCalibrationSet = false;
	public Vector3 motionPlusCalibration;
	public Vector3 motionPlus;
	public float motionPlusDifference;
	public float movementThreshold = 0.2f;
	
	public int A = 0;
	public int B = 0;
	public int Plus = 0;
	public int Home = 0;
	public int Minus = 0;
	public int One = 0;
	public int Two = 0;
	public int Up = 0;
	public int Down = 0;
	public int Left = 0;
	public int Right = 0;
	public int C = 0;
	public int Z = 0;
	
	public bool IR0_valid = false;
	public bool IR1_valid = false;
	public bool IR2_valid = false;
	public bool IR3_valid = false;
	
	public Vector2 IR0;
	public Vector2 IR1;
	public Vector2 IR2;
	public Vector2 IR3;
	
	public GameObject IR_DOT0;
	public GameObject IR_DOT1;
	public GameObject IR_DOT2;
	public GameObject IR_DOT3;
	// Use this for initialization
	void Start () {
		analogStick0Deadzone = new Vector3(10,10,10);
		
	}
	
	// Update is called once per frame
	void Update () {
		battery = getBatteryPercent();
		
		analogStick = getAnalogStick0();
		accelerometerVal0 = getAccelerometer0();
		accelerometerVal1 = getAccelerometer1();
		
		accelerometerMagnitude = accelerometerVal0.magnitude;
		
		if( getAccelerometer2() != Vector3.zero && !motionPlusCalibrationSet ){
			motionPlusCalibration = getAccelerometer2();
			motionPlusCalibrationSet = true;
		}
		motionPlus = getAccelerometer2() - motionPlusCalibration;
		
		motionPlusDifference = Vector3.Magnitude(getAccelerometer2() - motionPlusCalibration);
		
		if( motionPlusDifference >= movementThreshold ){
			transform.Rotate(motionPlus.y, -motionPlus.x, motionPlus.z);
			//transform.Translate(motionPlus.z, 0, 0);
		}
		
		// Testing
		transform.Translate(0, 0, analogStick.y * Time.deltaTime);
		transform.Rotate(0, analogStick.x * Time.deltaTime * 50, 0);
		
		A = getButton( Button.A );
		B = getButton( Button.B );
		Plus = getButton( Button.Plus );
		Minus = getButton( Button.Minus );
		Home = getButton( Button.Home );
		One = getButton( Button.One );
		Two = getButton( Button.Two );
		C = getButton( Button.C );
		Z = getButton( Button.Z );
		Up = getButton( Button.Up );
		Down = getButton( Button.Down );
		Left = getButton( Button.Left );
		Right = getButton( Button.Right );
		
		IR0_valid = isIRDot0Valid();
		IR1_valid = isIRDot1Valid();
		IR2_valid = isIRDot2Valid();
		IR3_valid = isIRDot3Valid();
		
		IR0 = getIRDot0() * 10;
		IR1 = getIRDot1() * 10;
		IR2 = getIRDot2() * 10;
		IR3 = getIRDot3() * 10;
		
		if( IR0_valid && IR_DOT0 ){
			IR_DOT0.transform.position = new Vector3(IR0.x, IR0.y, 0);
		} else if( IR_DOT0 ){
			IR_DOT0.transform.position = new Vector3(0, -10, 0);
		}
		
		if( IR1_valid && IR_DOT1 ){
			IR_DOT1.transform.position = IR1;
		} else if( IR_DOT1 ){
			IR_DOT1.transform.position = new Vector3(0, -10, 0);
		}
		
		if( IR2_valid && IR_DOT2 ){
			IR_DOT2.transform.position = IR2;
		} else if( IR_DOT2 ){
			IR_DOT2.transform.position = new Vector3(0, -10, 0);
		}
		
		if( IR3_valid && IR_DOT3 ){
			IR_DOT3.transform.position = IR3;
		} else if( IR_DOT3 ){
			IR_DOT3.transform.position = new Vector3(0, -10, 0);
		}
	}
}
