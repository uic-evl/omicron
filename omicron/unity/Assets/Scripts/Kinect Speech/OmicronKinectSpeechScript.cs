using UnityEngine;
using System.Collections;
using omicron;
using omicronConnector;

public abstract class OmicronKinectSpeechScript : MonoBehaviour {
	// This prints out the last speech event data
	public string lastSpeech;
	public float speechAccuracy;
	public float speechAngle;
	public float angleAccuracy;
	
	// This is what the incoming data is compared to
	public string targetSpeech;
	public float minimumAccuracy;
	public bool speechRecognized;
	
	public void CheckSpeech()
	{
		if( string.Equals(targetSpeech, lastSpeech) && speechAccuracy >= minimumAccuracy )
			speechRecognized = true;
	}
	
	public void ClearSpeech()
	{
		lastSpeech = "";
		speechAccuracy = 0;
		speechAngle = 0;
		angleAccuracy = 0;
		speechRecognized = false;
	}
	
	void OnEvent( EventData evt )
	{
		// If this is a Speech event...
		if( evt.serviceType == EventBase.ServiceType.ServiceTypeSpeech )
		{
			lastSpeech = evt.getExtraDataString().Trim();
			speechAccuracy = evt.posx;
			speechAngle = evt.posy;
			angleAccuracy = evt.posz;
		}
	}
}
