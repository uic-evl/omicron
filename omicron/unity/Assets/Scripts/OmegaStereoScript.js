/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010-2011							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Dennis Chau
 *  Arthur Nishimoto								anishimoto42@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************************/

/*
	Based on Stereoskopix 3D v027 by Perry Hoberman <hoberman (at) bway.net
	http://forum.unity3d.com/threads/63874
*/

@script RequireComponent (Camera)

//var enableStereo : boolean = true;
var eyePosition : Transform;
var focusPosition: Transform;

private var leftCamRT;	   
private var rightCamRT;
private var leftCam;
private var rightCam;

public var stereoMaterial : Material;

public var interaxialDistance : float = 0.03175;

enum stereoModes { None, Anaglyph, SideBySide, Interlace };
public var stereoMode = stereoModes.Interlace;
public var interlaceRows = 1526;

// Single display dimentions (including bezels)
public var displayWidth : float = 1.26;
public var displayHeight : float = 0.58;

public var topBezelWidth : float = 0.007;
public var bottomBezelWidth : float = 0.005;
public var leftBezelWidth : float = 0.006;
public var rightBezelWidth : float = 0.004;

// Manual tweaking for aspect ratio
public var manualVertStretch : float = -0.26;
public var manualHorzStretch : float = 0;

function Start () {
	//check stereo material 
	if (!stereoMaterial) {
		Debug.LogError("No Stereo Material Found. Please drag 'stereoMat' into the Stereo Material Field");
		this.enabled = false;
		return;
	}

	//init lt and rt camera properties
	leftCam = new GameObject ("leftCam", Camera);
	rightCam = new GameObject ("rightCam", Camera);

	leftCam.camera.CopyFrom (camera);
	rightCam.camera.CopyFrom (camera);
	
	leftCam.camera.renderingPath = camera.renderingPath;
	rightCam.camera.renderingPath = camera.renderingPath;
	
	//fieldOfView = camera.fieldOfView;

	leftCamRT = new RenderTexture (Screen.width, Screen.height, 24);
	rightCamRT = new RenderTexture (Screen.width, Screen.height, 24);
	
	leftCam.camera.targetTexture = leftCamRT;
	rightCam.camera.targetTexture = rightCamRT;
	  
	stereoMaterial.SetTexture ("_LeftTex", leftCamRT);
	stereoMaterial.SetTexture ("_RightTex", rightCamRT);
	
	SetWeave(0);

	leftCam.transform.parent = transform;
	rightCam.transform.parent = transform;
	
	camera.cullingMask = 0;
	camera.backgroundColor = Color (0,0,0,0);
	camera.clearFlags = CameraClearFlags.Nothing;
}

function Update () 
{
	//overridden function : Where user interaction from the GUI would update the scene
}

function LateUpdate() {
   UpdateView();
   SetWeave(0);
}

function UpdateView() {
	transform.localPosition = new Vector3( 0, 0, 0 );
	leftCam.camera.projectionMatrix = projectionMatrix(true);
	rightCam.camera.projectionMatrix = projectionMatrix(false);
	
	transform.localRotation = eyePosition.localRotation;
}

function OnRenderImage (source:RenderTexture, destination:RenderTexture) 
{
	RenderTexture.active = destination;
	GL.PushMatrix();
	GL.LoadOrtho();
	
	switch( stereoMode ){
		case( stereoModes.Anaglyph ):
			stereoMaterial.SetPass(0);
    		DrawQuad(0);
			break;
		case( stereoModes.Interlace ):
			stereoMaterial.SetPass(3);
			DrawQuad(3);
			break;
		case( stereoModes.SideBySide ):
			for(var i:int = 1; i <= 2; i++) {
				stereoMaterial.SetPass(i);
				DrawQuad(i);
			}
			break;
		default:
			stereoMaterial.SetPass(3);
			DrawQuad(3);
			break;
	}
	
	GL.PopMatrix();
}

function OnGUI () 
{
   	//overridden function : Add a GUI if you want one
}

function DoWindow (windowID : int) 
{
}
 
 private function SetWeave(xy) {
	if (!xy) 
	{
		stereoMaterial.SetFloat("_Weave_X", 1);
		stereoMaterial.SetFloat("_Weave_Y", interlaceRows);
	}
}

 
private function DrawQuad(cam) {
	switch( stereoMode ){
		case( stereoModes.SideBySide ):
			if (cam==1) {
				GL.Begin (GL.QUADS);      
				GL.TexCoord2( 0.0, 0.0 ); GL.Vertex3( 0.0, 0.0, 0.1 );
				GL.TexCoord2( 1.0, 0.0 ); GL.Vertex3( 0.5, 0.0, 0.1 );
				GL.TexCoord2( 1.0, 1.0 ); GL.Vertex3( 0.5, 1.0, 0.1 );
				GL.TexCoord2( 0.0, 1.0 ); GL.Vertex3( 0.0, 1.0, 0.1 );
				GL.End();
			} else {
				GL.Begin (GL.QUADS);      
				GL.TexCoord2( 0.0, 0.0 ); GL.Vertex3( 0.5, 0.0, 0.1 );
				GL.TexCoord2( 1.0, 0.0 ); GL.Vertex3( 1.0, 0.0, 0.1 );
				GL.TexCoord2( 1.0, 1.0 ); GL.Vertex3( 1.0, 1.0, 0.1 );
				GL.TexCoord2( 0.0, 1.0 ); GL.Vertex3( 0.5, 1.0, 0.1 );
				GL.End();
			}
			break;
		default:
			GL.Begin (GL.QUADS);      
		   	GL.TexCoord2( 0.0, 0.0 ); GL.Vertex3( 0.0, 0.0, 0.1 );
		   	GL.TexCoord2( 1.0, 0.0 ); GL.Vertex3( 1, 0.0, 0.1 );
		  	GL.TexCoord2( 1.0, 1.0 ); GL.Vertex3( 1, 1.0, 0.1 );
		  	GL.TexCoord2( 0.0, 1.0 ); GL.Vertex3( 0.0, 1.0, 0.1 );
			GL.End();
			break;
	}
} 

function projectionMatrix(isLeftCam : boolean) : Matrix4x4 
{
	// 2732x1536 (Windowed) - Does not fill screen
	var windowUpperOffset : float = 0.539;
	var windowLowerOffset : float = -0.552;
	var windowLeftOffset : float = -0.965;
	var windowRightOffset : float =  0.973;
	
	var manualOffsetR : float = -0.34;
	var manualOffsetU : float = 0.09;
	
	var manualOffsetH : float = 0.17;
	var manualOffsetV : float = -0.05;
	
	// 2731x1525 (Windowed) - Nearly fills screen
	//windowUpperOffset = 0.539 + manualOffsetU + manualOffsetV;
	//windowLowerOffset = -0.535 + manualOffsetV;
	//windowLeftOffset = -1.16 + manualOffsetH;
	//windowRightOffset =  1.17 + manualOffsetR + manualOffsetH;

	var menuBarHeight : float = 0.015;
	
	// Distance from the visible screen center (in meters) to the edges of the visible screen
	windowUpperOffset = displayHeight - topBezelWidth - menuBarHeight + manualHorzStretch;
	windowLowerOffset = -(displayHeight - bottomBezelWidth + manualHorzStretch);
	windowLeftOffset = -(displayWidth - leftBezelWidth + manualVertStretch);
	windowRightOffset =  displayWidth - rightBezelWidth + manualVertStretch;
	
	var Scr_LL_wc : Vector3 = Vector3( windowLeftOffset , windowLowerOffset , 0 ) + focusPosition.localPosition;   //Lower Left Corner 
    var Scr_LR_wc : Vector3 = Vector3( windowRightOffset , windowLowerOffset , 0 ) + focusPosition.localPosition;   //Lower Right Corner 
    var Scr_UL_wc : Vector3 = Vector3( windowLeftOffset , windowUpperOffset , 0 ) + focusPosition.localPosition;   //Upper Left Corner 

    //Coord of the eye
    var eye : Vector3;
	var eyeOffset : float = interaxialDistance;
	if( stereoMode == stereoModes.None )
		eyeOffset = 0;
	
	if( isLeftCam )
	{
		if( Application.unityVersion.Contains("3.1.0") ) // Flips eye y translation for older versions of Unity
			eye  = Vector3( eyePosition.localPosition.x - eyeOffset , -eyePosition.localPosition.y, eyePosition.localPosition.z);
		else // Tested for versions 3.3.0 and 3.4.0
			eye  = Vector3( eyePosition.localPosition.x - eyeOffset , eyePosition.localPosition.y, eyePosition.localPosition.z);
	}
	else 
	{	
		if( Application.unityVersion.Contains("3.1.0") ) // Flips eye y translation for older versions of Unity
			eye  = Vector3( eyePosition.localPosition.x + eyeOffset , -eyePosition.localPosition.y, eyePosition.localPosition.z);
		else // Tested for versions 3.3.0 and 3.4.0
			eye  = Vector3( eyePosition.localPosition.x + eyeOffset , eyePosition.localPosition.y, eyePosition.localPosition.z);	
	}
	
	//temp
    var vec3_temp : Vector3;
    
    //Orthonormal Basis of 3D space
    var v_right : Vector3;  //vector right
    var v_up    : Vector3;  //vector up
    var v_normal: Vector3;  //vector normal
    
    vec3_temp = Scr_LR_wc - Scr_LL_wc;
    v_right = (vec3_temp) / vec3_temp.magnitude;
    
    vec3_temp = Scr_UL_wc - Scr_LL_wc;
    v_up = (vec3_temp) / vec3_temp.magnitude;

    vec3_temp = Vector3.Cross(v_right, v_up);
    v_normal = -(vec3_temp) / vec3_temp.magnitude; 
	
	//Calculate the vectors from the eye to the screen corners
	var vec_e_LL = Scr_LL_wc - eye;
    var vec_e_LR = Scr_LR_wc - eye;
    var vec_e_UL = Scr_UL_wc - eye;
	
    //dist from eye to screen space origin
    var dist = -Vector3.Dot(v_normal, vec_e_LL );

    //setup tracker-space distance from screen_space origin
    var l = Vector3.Dot( v_right, vec_e_LL ) * camera.nearClipPlane / dist;
    var r = Vector3.Dot( v_right, vec_e_LR ) * camera.nearClipPlane / dist;
    var b = Vector3.Dot( v_up, vec_e_LL ) * camera.nearClipPlane / dist;
    var t = Vector3.Dot( v_up, vec_e_UL ) * camera.nearClipPlane / dist;            
    
    var x =  (2.0 * camera.nearClipPlane) / (r - l);
    var y =  (2.0 * camera.nearClipPlane) / (t - b);
    var f =  (r + l) / (r - l);
    var g =  (t + b) / (t - b);
    var h = (camera.farClipPlane + camera.nearClipPlane) / (camera.farClipPlane - camera.nearClipPlane);
    var j = (2.0 * camera.farClipPlane * camera.nearClipPlane) / (camera.farClipPlane - camera.nearClipPlane);
    var k = 1.0;

	var P : Matrix4x4;
	P[0,0] = x;  P[0,1] = 0;  P[0,2] = f;  P[0,3] = 0;
    P[1,0] = 0; P[1,1] = y;  P[1,2] = g;  P[1,3] = 0;
    P[2,0] = 0;  P[2,1] =0;  P[2,2] = -h;  P[2,3] = -j;
    P[3,0] = 0;  P[3,1] = 0;  P[3,2] = -k;  P[3,3] = 0;

	// Map screen coordinate space ---> right handed 
    var M : Matrix4x4;
	M[0,0] = v_right[0];  		M[0,1] = v_right[1];  		M[0,2] = v_right[2]; 		M[0,3] = 0;
    M[1,0] = v_up[0];     		M[1,1] = v_up[1];     		M[1,2] = v_up[2];     		M[1,3] = 0;
    M[2,0] = -v_normal[0]; 	M[2,1] = -v_normal[1]; 	M[2,2] = -v_normal[2]; 	M[2,3] = 0;
    M[3,0] = 0;  						M[3,1] = 0;  						M[3,2] = 0;  						M[3,3] = 1;
	
	var T : Matrix4x4;
	T[0,0] = 1;  T[0,1] = 0;  T[0,2] = 0;  T[0,3] = -eye[0];
    T[1,0] = 0;  T[1,1] = 1;  T[1,2] = 0;  T[1,3] = -eye[1];
    T[2,0] = 0;  T[2,1] = 0;  T[2,2] = 1;  T[2,3] =  eye[2];
    T[3,0] = 0;  T[3,1] = 0;  T[3,2] = 0;  T[3,3] = 1;

	//Debug.Log( "eye: " + eye);

    var P_final : Matrix4x4;
	P_final = P * M * T;
	return P_final;
} 

