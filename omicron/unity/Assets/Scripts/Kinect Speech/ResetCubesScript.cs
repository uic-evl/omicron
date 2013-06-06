using UnityEngine;
using System.Collections;

public class ResetCubesScript : OmicronKinectSpeechScript {
	Vector3 initialPosition;
	Quaternion initialRotation;
	
	// Use this for initialization
	void Start () {
		initialPosition = transform.position;
		initialRotation = transform.rotation;
	}
	
	// Update is called once per frame
	void Update () {
		CheckSpeech();
		
		if( speechRecognized )
		{
			transform.position = initialPosition;
			transform.rotation = initialRotation;
			
			rigidbody.AddForce( -rigidbody.velocity, ForceMode.VelocityChange );
			rigidbody.AddRelativeTorque( -rigidbody.angularVelocity, ForceMode.VelocityChange );
			
			// Mark this event as processed
			ClearSpeech();
		}
	}
}
