/******************************************************************************
 * THE OMICRON PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Matthew Rycraft		mattrycraft@comcast.net
 *  Thomas Marrinan     thomas.j.marrinan@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2014, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file:
 *  A ray to pointer mapper using a cylinder as the 2D projection surface.
 ******************************************************************************/
#ifndef RECTANGULAR_RAY_POINT_MAPPER
#define RECTANGULAR_RAY_POINT_MAPPER
#include "omicron/RayPointMapper.h"
#include "omicron/Config.h"

namespace omicron
{
    class RectangularRayPointMapper: public RayPointMapper
    {
    public:
        ///////////////////////////////////////////////////////////////////////
        RectangularRayPointMapper()
        {
            InvalidValue = Vector2f(-1.0f, -1.0f);
			
			myTopLeft = Vector3f(0.0f, 1.0f, 0.0f);
			myBottomLeft = Vector3f(0.0f, 0.0f, 0.0f);
			myTopRight = Vector3f(1.0f, 1.0f, 0.0f);
			myBottomRight = Vector3f(1.0f, 0.0f, 0.0f);

        }

        ///////////////////////////////////////////////////////////////////////
        virtual void setup(const Setting& s) 
        {
            myTopLeft = Config::getVector3fValue("topLeft", s, myTopLeft);
			myBottomLeft = Config::getVector3fValue("bottomLeft", s, myBottomLeft);
			myTopRight = Config::getVector3fValue("topRight", s, myTopRight);
			myBottomRight = Config::getVector3fValue("bottomRight", s, myBottomRight);
			
			myX = myBottomRight - myBottomLeft;
			myY = myTopLeft - myBottomLeft;
			myXMagnitude = sqrt(myX.x() * myX.x() + myX.y() * myX.y() + myX.z() * myX.z());
			myYMagnitude = sqrt(myY.x() * myY.x() + myY.y() * myY.y() + myY.z() * myY.z());
			myX.normalize();
			myY.normalize();

			myNormal = myX.cross(myY);
			myNormal.normalize();

			myPlane = Plane(myNormal, myBottomLeft);
        }
        
        ///////////////////////////////////////////////////////////////////////
        virtual Vector2f getPointFromRay(const Ray& ray)
        {
            Vector3f origin = ray.getOrigin();
			Vector3f forward = ray.getDirection();
			forward.normalize();

			std::pair<bool, real> ipair = Math::intersects(ray, myPlane);
			if (!ipair.first)
			{
				return InvalidValue;
			}

			Vector3f intersection = origin + (forward * ipair.second);

			//double s = myNormal.dot(intersection - myBottomLeft);
			double t1 = myX.dot(intersection - myBottomLeft) / myXMagnitude;
			double t2 = myY.dot(intersection - myBottomLeft) / myYMagnitude;

			if (t1 < 0.0 || t1 > 1.0 || t2 < 0.0 || t2 > 1.0)
			{
				return InvalidValue;
			}

			return Vector2f(t1, t2);
        }
        
    private:
        Vector2f InvalidValue;

		Vector3f myTopLeft;
		Vector3f myBottomLeft;
		Vector3f myTopRight;
		Vector3f myBottomRight;
		Vector3f myNormal;
		Vector3f myX;
		Vector3f myY;
		double myXMagnitude;
		double myYMagnitude;
		Plane myPlane;
    };
};
#endif
