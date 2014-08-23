/******************************************************************************
 * THE OMICRON PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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
#ifndef CYLINDRICAL_RAY_POINT_MAPPER
#define CYLINDRICAL_RAY_POINT_MAPPER
#include "omicron/RayPointMapper.h"
#include "omicron/Config.h"

namespace omicron
{
    class CylindricalRayPointMapper: public RayPointMapper
    {
    public:
        ///////////////////////////////////////////////////////////////////////
        CylindricalRayPointMapper():
            myRadius(1.0f),
            myDoorWidth(1.0f * Math::DegToRad),
            myXBias(0.0f),
            myMinY(0.0f),
            myMaxY(1.0f)
        {
            InvalidValue = Vector2f(-1.0f, -1.0f);
        }

        ///////////////////////////////////////////////////////////////////////
        virtual void setup(const Setting& s) 
        {
            myRadius = Config::getFloatValue("radius", s, myRadius);

            myDoorWidth = Config::getFloatValue("doorWidth", s, 1.0f);
            myDoorWidth *= Math::DegToRad;

            myXBias = Config::getFloatValue("xBias", s, myXBias);
            myMinY = Config::getFloatValue("minY", s, myMinY);
            myMaxY = Config::getFloatValue("maxY", s, myMaxY);
        }
        
        ///////////////////////////////////////////////////////////////////////
        virtual Vector2f getPointFromRay(const Ray& ray)
        {
            // Sanity check: if ray is pointing straight up or down, there will be
            // no intersection. return immediately.
            if(ray.getDirection().x() == 0 && ray.getDirection().z() == 0)
            {
                return InvalidValue;
            }

            // x and z coordinates of the center of the circle
            float h = 0;
            float k = 0;

            float ox = ray.getDirection().x();
            float oy = ray.getDirection().y();
            float oz = ray.getDirection().z();
            float x0 = ray.getOrigin().x();
            float y0 = ray.getOrigin().y();
            float z0 = ray.getOrigin().z();
            float r = myRadius;

            // A * t^2 + B * c + C
            float A = ox*ox + oz*oz;
            float B = 2*ox*x0 + 2*oz*z0 - 2*h*ox - 2*k*oz;
            float C = x0*x0 + z0*z0 + h*h + k*k - r*r - 2*h*x0 - 2*k*z0;

            if(A != 0  && (B*B - 4*A*C) >= 0)
            {
                float t1 = (-B + Math::sqrt(B*B - 4*A*C)) / (2*A);
                float t2 = (-B - Math::sqrt(B*B - 4*A*C)) / (2*A);
                float t = 0;
                if(t1 > 0) t = t1;
                else if(t2 > 0) t = t2;
                else return InvalidValue;

                float x = ox*t + x0;
                float y = oy*t + y0;
                float z = oz*t + z0;

                float angle = Math::atan2(x, z);
                if(angle < 0) angle += Math::TwoPi;
                angle = Math::TwoPi - angle;
                angle -= myDoorWidth / 2;
                x = angle / (Math::TwoPi - myDoorWidth);
                x += myXBias;
                y -= myMinY;
                y /= (myMaxY - myMinY);
                
                if(x < 0) x = 0;
                if(x > 1) x = 1;
                if(y < 0) y = 0;
                if(y > 1) y = 1; 

                return Vector2f(x, (1.0f - y));
            }

            return InvalidValue;
        }
        
    private:
        Vector2f InvalidValue;

        //! The cylinder radius in meters
        float myRadius;
        //! The door width in radians
        float myDoorWidth;
        //! Correction to computed x values, applied to normalized output
        float myXBias;

        float myMinY;
        float myMaxY;
    };
};
#endif
