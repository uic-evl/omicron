using UnityEngine;
using System.Collections;

public class MindwaveGUITextScript : MonoBehaviour {
	public OmicronMindwaveScript mindwaveScript;
	
	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		guiText.text = " Signal: "+ mindwaveScript.signalStrength +
			"\n Attention: "+mindwaveScript.attention + 
			"\n Meditation: "+mindwaveScript.meditation + 
			"\n Delta: "+mindwaveScript.delta + 
			"\n Theta: "+mindwaveScript.theta + 
			"\n Low Alpha: "+mindwaveScript.lowAlpha + 
			"\n High Alpha: "+mindwaveScript.highAlpha + 
			"\n Low Beta: "+mindwaveScript.lowBeta + 
			"\n High Beta: "+mindwaveScript.highBeta + 
			"\n Low Gamma: "+mindwaveScript.lowGamma + 
			"\n High Gamma: "+mindwaveScript.highGamma + 
			"\n Blink Strength: "+mindwaveScript.blinkStrength;
	}
}
