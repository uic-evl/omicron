/********************************************************************************************************************** 
* THE OMICRON PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010, Electronic Visualization Laboratory, University of Illinois at Chicago
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
 *---------------------------------------------------------------------------------------------------------------------
 * Original code taken from OGRE
 * Copyright (c) 2000-2009 Torus Knot Software Ltd
 *  For the latest info, see http://www.ogre3d.org/
 *---------------------------------------------------------------------------------------------------------------------
 * This file is based on material originally from:
 * Geometric Tools, LLC
 * Copyright (c) 1998-2010
 * Distributed under the Boost Software License, Version 1.0.
 * http://www.boost.org/LICENSE_1_0.txt
 * http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
 *********************************************************************************************************************/
#ifndef __PLANE_H__
#define __PLANE_H__

#include "EigenWrappers.h"

namespace omicron { 
       /** The "positive side" of the plane is the half space to which the
            plane normal points. The "negative side" is the other half
            space. The flag "no side" indicates the plane itself.
        */
        enum PlaneSide
        {
            NO_SIDE,
            POSITIVE_SIDE,
            NEGATIVE_SIDE,
            BOTH_SIDE
        };
	/** Defines a plane in 3D space.
        @remarks
            A plane is defined in 3D space by the equation
            Ax + By + Cz + D = 0
        @par
            This equates to a vector (the normal of the plane, whose x, y
            and z components equate to the coefficients A, B and C
            respectively), and a constant (D) which is the distance along
            the normal you have to go to move the plane back to the origin.
     */
    class Plane
    {
    public:
        /** Default constructor - sets everything to 0.
        */
        inline Plane ();
        inline Plane (const Plane& rhs);
        /** Construct a plane through a normal, and a distance to move the plane along the normal.*/
        inline Plane (const Vector3f& rkNormal, float fConstant);
		/** Construct a plane using the 4 constants directly **/
		inline Plane (float a, float b, float c, float d);
        inline Plane (const Vector3f& rkNormal, const Vector3f& rkPoint);
        inline Plane (const Vector3f& rkPoint0, const Vector3f& rkPoint1,
            const Vector3f& rkPoint2);

        inline PlaneSide getSide (const Vector3f& rkPoint) const;

        /**
        returns the side where the aligneBox is. the flag BOTH_SIDE indicates an intersecting box.
        one corner ON the plane is sufficient to consider the box and the plane intersecting.
        */
        //inline side getSide (const AlignedBox3& rkBox) const;

        /** Returns which side of the plane that the given box lies on.
            The box is defined as centre/half-size pairs for effectively.
        @param centre The centre of the box.
        @param halfSize The half-size of the box.
        @returns
            POSITIVE_SIDE if the box complete lies on the "positive side" of the plane,
            NEGATIVE_SIDE if the box complete lies on the "negative side" of the plane,
            and BOTH_SIDE if the box intersects the plane.
        */
        inline PlaneSide getSide (const Vector3f& centre, const Vector3f& halfSize) const;

        /** This is a pseudodistance. The sign of the return value is
            positive if the point is on the positive side of the plane,
            negative if the point is on the negative side, and zero if the
            point is on the plane.
            @par
            The absolute value of the return value is the true distance only
            when the plane normal is a unit length vector.
        */
        inline float getDistance (const Vector3f& rkPoint) const;

        /** Redefine this plane based on 3 points. */
        inline void redefine(const Vector3f& rkPoint0, const Vector3f& rkPoint1,
            const Vector3f& rkPoint2);

		/** Redefine this plane based on a normal and a point. */
		inline void redefine(const Vector3f& rkNormal, const Vector3f& rkPoint);

		/** Project a vector onto the plane. 
		@remarks This gives you the element of the input vector that is perpendicular 
			to the normal of the plane. You can get the element which is parallel
			to the normal of the plane by subtracting the result of this method
			from the original vector, since parallel + perpendicular = original.
		@param v The input vector
		*/
		inline Vector3f projectVector(const Vector3f& v) const;

        /** Normalises the plane.
            @remarks
                This method normalises the plane's normal and the length scale of d
                is as well.
            @note
                This function will not crash for zero-sized vectors, but there
                will be no changes made to their components.
            @returns The previous length of the plane's normal.
        */
        inline float normalise(void);

		Vector3f normal;
        float d;

        /// Comparison operator
        bool operator==(const Plane& rhs) const
        {
            return (rhs.d == d && rhs.normal == normal);
        }
        bool operator!=(const Plane& rhs) const
        {
            return (rhs.d != d && rhs.normal != normal);
        }
    };

	//-----------------------------------------------------------------------
	inline
	
	Plane::Plane ()
	{
		normal = Vector3f::Zero();
		d = 0.0;
	}
	//-----------------------------------------------------------------------
	inline
	
	Plane::Plane (const Plane& rhs)
	{
		normal = rhs.normal;
		d = rhs.d;
	}
	//-----------------------------------------------------------------------
	inline
	
	Plane::Plane (const Vector3f& rkNormal, float fConstant)
	{
		normal = rkNormal;
		d = -fConstant;
	}
	//---------------------------------------------------------------------
	inline
	
	Plane::Plane (float a, float b, float c, float _d)
		: normal(a, b, c), d(_d)
	{
	}
	//-----------------------------------------------------------------------
	inline
	
	Plane::Plane (const Vector3f& rkNormal, const Vector3f& rkPoint)
	{
		redefine(rkNormal, rkPoint);
	}
	//-----------------------------------------------------------------------
	inline
	
	Plane::Plane (const Vector3f& rkPoint0, const Vector3f& rkPoint1,
		const Vector3f& rkPoint2)
	{
		redefine(rkPoint0, rkPoint1, rkPoint2);
	}
	//-----------------------------------------------------------------------
	inline
	
	float Plane::getDistance (const Vector3f& rkPoint) const
	{
		return normal.dot(rkPoint) + d;
	}
	//-----------------------------------------------------------------------
	inline
	
	PlaneSide Plane::getSide (const Vector3f& rkPoint) const
	{
		float fDistance = getDistance(rkPoint);

		if ( fDistance < 0.0 )
			return NEGATIVE_SIDE;

		if ( fDistance > 0.0 )
			return POSITIVE_SIDE;

		return NO_SIDE;
	}


	//-----------------------------------------------------------------------
/*	
	typename Plane::side Plane::getSide (const AlignedBox3& box) const
	{
		if (box.isNull()) 
			return NO_SIDE;
		if (box.isInfinite())
			return BOTH_SIDE;

        return getSide(box.getCenter(), box.getHalfSize());
	}*/
    //-----------------------------------------------------------------------
	inline
	
    PlaneSide Plane::getSide (const Vector3f& centre, const Vector3f& halfSize) const
    {
        // Calculate the distance between box centre and the plane
        float dist = getDistance(centre);

        // Calculate the maximise allows absolute distance for
        // the distance between box centre and plane
        float maxAbsDist = abs(normal.dot(halfSize));

        if (dist < -maxAbsDist)
            return NEGATIVE_SIDE;

        if (dist > +maxAbsDist)
            return POSITIVE_SIDE;

        return BOTH_SIDE;
    }
	//-----------------------------------------------------------------------
	inline
	
	void Plane::redefine(const Vector3f& rkPoint0, const Vector3f& rkPoint1,
		const Vector3f& rkPoint2)
	{
		Vector3f kEdge1 = rkPoint1 - rkPoint0;
		Vector3f kEdge2 = rkPoint2 - rkPoint0;
		normal = kEdge1.cross(kEdge2);
		normal.normalize();
		d = -normal.dot(rkPoint0);
	}
	//-----------------------------------------------------------------------
	inline
	
	void Plane::redefine(const Vector3f& rkNormal, const Vector3f& rkPoint)
	{
		normal = rkNormal;
		d = -rkNormal.dot(rkPoint);
	}
	//-----------------------------------------------------------------------
	//Vector3f Plane::projectVector(const Vector3f& p) const
	//{
	//	// We know plane normal is unit length, so use simple method
	//	Matrix3f xform;
	//	xform[0][0] = 1.0f - normal.x() * normal.x();
	//	xform[0][1] = -normal.x() * normal.y();
	//	xform[0][2] = -normal.x() * normal.z();
	//	xform[1][0] = -normal.y() * normal.x();
	//	xform[1][1] = 1.0f - normal.y() * normal.y();
	//	xform[1][2] = -normal.y() * normal.z();
	//	xform[2][0] = -normal.z() * normal.x();
	//	xform[2][1] = -normal.z() * normal.y();
	//	xform[2][2] = 1.0f - normal.z() * normal.z();
	//	return xform * p;

	//}

	//-----------------------------------------------------------------------
	inline
    float Plane::normalise(void)
    {
		float fLength = normal.norm();

        // Will also work for zero-sized vectors, but will change nothing
        if (fLength > 1e-08f)
        {
            float fInvLength = 1.0f / fLength;
            normal *= fInvLength;
            d *= fInvLength;
        }

        return fLength;
    }
}; //namespace omicron

#endif
