/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2011		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 * Contributors:
 *  Victor Mateevitsi		mvictoras@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#ifndef __POINTSET_ID_H__
#define __POINTSET_ID_H__

#ifdef OMEGA_USE_OPENNI
#ifdef OMEGA_OS_WIN
#include <XnCppWrapper.h>
#else
#include <ni/XnCppWrapper.h>
#endif
#endif

namespace omicron
{
#ifdef OMEGA_USE_OPENNI
	enum PointSetId
	{
		Head = XN_SKEL_HEAD,
		Neck = XN_SKEL_NECK,
		Torso = XN_SKEL_TORSO,
		LeftShoulder = XN_SKEL_LEFT_SHOULDER,
		LeftElbow = XN_SKEL_LEFT_ELBOW,
		LeftHand = XN_SKEL_LEFT_HAND,
		LeftHip = XN_SKEL_LEFT_HIP,
		LeftKnee = XN_SKEL_LEFT_KNEE,
		LeftFoot = XN_SKEL_LEFT_FOOT,
		RightShoulder = XN_SKEL_RIGHT_SHOULDER,
		RightElbow = XN_SKEL_RIGHT_ELBOW,
		RightHand = XN_SKEL_RIGHT_HAND,
		RightHip = XN_SKEL_RIGHT_HIP,
		RightKnee = XN_SKEL_RIGHT_KNEE,
		RightFoot = XN_SKEL_RIGHT_FOOT
	};
#else
	enum PointSetId
	{
		Head,
		Neck,
		Torso,
		LeftShoulder,
		LeftElbow,
		LeftHand,
		LeftHip,
		LeftKnee,
		LeftFoot,
		RightShoulder,
		RightElbow,
		RightHand,
		RightHip,
		RightKnee,
		RightFoot
	};
#endif
}; // namespace omega

#endif
