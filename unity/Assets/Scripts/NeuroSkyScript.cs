using UnityEngine;
using System.Collections;

public class NeuroSkyScript : MonoBehaviour {
	public int sourceID;
	
	public int signal;
	public int attention;
	public int meditation;
	public int delta;
	public int theta;
	public int lowAlpha;
	public int highAlpha;
	public int lowBeta;
	public int highBeta;
	public int lowGamma;
	public int highGamma;
	public int blink;
	
	void Start () {
		// Reserved for derived classes
	}

	void Update () {
		// Reserved for derived classes
	}
		
	public void UpdateNeuroSky( string[] words ){
		int ID = int.Parse(words[1]);
		if( ID == sourceID ){
			signal = int.Parse(words[2]);
			attention = int.Parse(words[3]);
			meditation = int.Parse(words[4]);
			delta = int.Parse(words[5]);
			theta = int.Parse(words[6]);
			lowAlpha = int.Parse(words[7]);
			highAlpha = int.Parse(words[8]);
			lowBeta = int.Parse(words[9]);
			highBeta = int.Parse(words[10]);
			lowGamma = int.Parse(words[11]);
			highGamma = int.Parse(words[12]);
			blink = int.Parse(words[13]);
		}
	}
	
	// Getters/Setters ------------------------------------------------------------

}
