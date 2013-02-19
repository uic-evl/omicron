using UnityEngine;
using System.Collections;

public class Xbox360ControllerScript : OmegaControllerScript {
	public int A = 0;
	public int B = 0;
	public int X = 0;
	public int Y = 0;
	public int LB = 0;
	public int RB = 0;
	public int Back = 0;
	public int StartB = 0;
	public int LA = 0;
	public int RA = 0;
	public int Up = 0;
	public int Down = 0;
	public int Left = 0;
	public int Right = 0;
	
	// Use this for initialization
	void Start () {
		analogStick0Sensitivity = 0.05f;
		analogStick1Sensitivity = 1.0f;
		triggerSensitivity = 0.5f;
	}
	
	// Update is called once per frame
	void Update () {
		Vector3 leftAnalog = getLeftAnalogStick();
		Vector3 rightAnalog = getRightAnalogStick();
		
		// Testing
		transform.Translate(leftAnalog.x, 0, -leftAnalog.y);
		transform.Rotate(0,rightAnalog.x, 0);
		
		transform.Translate(0, -getTrigger(), 0);
		
		A = getButton( Button.A );
		B = getButton( Button.B );
		X = getButton( Button.X );
		Y = getButton( Button.Y );
		LB = getButton( Button.LB );
		RB = getButton( Button.RB );
		Back = getButton( Button.Back );
		StartB = getButton( Button.Start );
		LA = getButton( Button.LA );
		RA = getButton( Button.RA );
		Up = getButton( Button.Up );
		Down = getButton( Button.Down );
		Left = getButton( Button.Left );
		Right = getButton( Button.Right );
	}
}
