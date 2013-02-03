/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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
#ifndef __OMICRON_MATH__H__
#define __OMICRON_MATH__H__

#include <cmath>

#include "omicronConfig.h"
#include "Plane.h"
#include "EigenWrappers.h"

namespace omicron { 
	class Ray;
	class Plane;
	class Sphere;
	class AlignedBox3;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Stores a rectangular region in integer units. Useful for storing viewports and other 
	//! pixel-space regions.
    struct Rect
    {
		Vector2i min;
		Vector2i max;

		Rect(): min(0, 0), max(0, 0) {}

		Rect(const Vector2i& vmin, const Vector2i& vmax):
		min(vmin), max(vmax) {}

		Rect(int x, int y, int width, int height):
		min(x, y), max(x + width, y + height) {}

		int width() const { return max(0) - min(0); }
		int height() const { return max(1) - min(1); }

		int x() const { return min(0); }
		int y() const { return min(1); }
    };

	///////////////////////////////////////////////////////////////////////////////////////////////
    /** Class to provide access to common mathematical functions.
        @remarks
            Most of the maths functions are aliased versions of the C runtime
            library functions. They are aliased here to provide future
            optimisation opportunities, either from faster RTLs or custom
            math approximations.
        @note
            <br>This is based on MgcMath.h from
            <a href="http://www.geometrictools.com/">Wild Magic</a>.
    */
    class OMICRON_API Math 
    {
    protected:
        /// Size of the trig tables as determined by constructor.
        static int mTrigTableSize;

        /// Radian -> index factor value ( mTrigTableSize / 2 * Pi )
        static real mTrigTableFactor;
        static real* mSinTable;
        static real* mTanTable;

        /** Private function to build trig tables.
        */
        void buildTrigTables();

		static real SinTable (real fValue);
		static real TanTable (real fValue);
    public:
        /** Default constructor.
            @param
                trigTableSize Optional parameter to set the size of the
                tables used to implement Sin, Cos, Tan
        */
        Math(unsigned int trigTableSize = 4096);

        /** Default destructor.
        */
        ~Math();

		static int iabs (int iValue) { return ( iValue >= 0 ? iValue : -iValue ); }
		static int iceil (real fValue) { return int(std::ceil(fValue)); }
		static int ifloor (real fValue) { return int(std::floor(fValue)); }
        static int isign (int iValue);

		static real abs (real fValue) { return real(fabs(fValue)); }
		static real acos (real fValue);
		static real asin (real fValue);
		static real atan (real fValue) { return std::atan(fValue); }
		static real atan2 (real fY, real fX) { return std::atan2(fY,fX); }
		static real ceil (real fValue) { return real(std::ceil(fValue)); }
		static bool isNaN(real f)
		{
			// std::isnan() is C99, not supported by all compilers
			// However NaN always fails this next test, no other number does.
			return f != f;
		}

        /** Cosine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static real cos (real fValue, bool useTables = false) {
			return (!useTables) ? real(std::cos(fValue)) : SinTable(fValue + HalfPi);
		}

		static real exp (real fValue) { return real(std::exp(fValue)); }

		static real floor (real fValue) { return real(std::floor(fValue)); }

		static real log (real fValue) { return real(std::log(fValue)); }

		/// Stored value of log(2) for frequent use
		static const real Log2Base;

		static real log2 (real fValue) { return real(log(fValue)/Log2Base); }

		static real logN (real base, real fValue) { return real(log(fValue)/log(base)); }

		static real pow (real fBase, real fExponent) { return real(std::pow(fBase,fExponent)); }

        static real sign (real fValue);

        /** Sine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static real sin (real fValue, bool useTables = false) {
			return (!useTables) ? real(std::sin(fValue)) : SinTable(fValue);
		}

		static real sqr (real fValue) { return fValue*fValue; }

		static real sqrt (real fValue) { return real(std::sqrt(fValue)); }

        static real unitRandom ();  // in [0,1]

        static real rangeRandom (real fLow, real fHigh);  // in [fLow,fHigh]

        static real symmetricRandom ();  // in [-1,1]

        /** Tangent function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
		static real tan (real fValue, bool useTables = false) {
			return (!useTables) ? real(std::tan(fValue)) : TanTable(fValue);
		}

		static real degreesToRadians(real degrees) { return degrees * DegToRad; }
        static real radiansToDegrees(real radians) { return radians * RadToDeg; }

       /** Checks whether a given point is inside a triangle, in a
            2-dimensional (Cartesian) space.
            @remarks
                The vertices of the triangle must be given in either
                trigonometrical (anticlockwise) or inverse trigonometrical
                (clockwise) order.
            @param
                p The point.
            @param
                a The triangle's first vertex.
            @param
                b The triangle's second vertex.
            @param
                c The triangle's third vertex.
            @returns
                If the point resides in the triangle, <b>true</b> is
                returned.
            @par
                If the point is outside the triangle, <b>false</b> is
                returned.
        */
        static bool pointInTri2D(const Vector2f& p, const Vector2f& a, 
			const Vector2f& b, const Vector2f& c);

       /** Checks whether a given 3D point is inside a triangle.
       @remarks
            The vertices of the triangle must be given in either
            trigonometrical (anticlockwise) or inverse trigonometrical
            (clockwise) order, and the point must be guaranteed to be in the
			same plane as the triangle
        @param
            p The point.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
		@param 
			normal The triangle plane's normal (passed in rather than calculated
				on demand since the caller may already have it)
        @returns
            If the point resides in the triangle, <b>true</b> is
            returned.
        @par
            If the point is outside the triangle, <b>false</b> is
            returned.
        */
        static bool pointInTri3D(const Vector3f& p, const Vector3f& a, 
			const Vector3f& b, const Vector3f& c, const Vector3f& normal);
        /** Ray / plane intersection, returns boolean result and distance. */
        static std::pair<bool, real> intersects(const Ray& ray, const Plane& plane);

        /** Ray / sphere intersection, returns boolean result and distance. */
        static std::pair<bool, real> intersects(const Ray& ray, const Sphere& sphere, 
            bool discardInside = true);
        
        /** Ray / box intersection, returns boolean result and distance. */
        static std::pair<bool, real> intersects(const Ray& ray, const AlignedBox3& box);

        /** Ray / box intersection, returns boolean result and two intersection distance.
        @param
            ray The ray.
        @param
            box The box.
        @param
            d1 A real pointer to retrieve the near intersection distance
                from the ray origin, maybe <b>null</b> which means don't care
                about the near intersection distance.
        @param
            d2 A real pointer to retrieve the far intersection distance
                from the ray origin, maybe <b>null</b> which means don't care
                about the far intersection distance.
        @returns
            If the ray is intersects the box, <b>true</b> is returned, and
            the near intersection distance is return by <i>d1</i>, the
            far intersection distance is return by <i>d2</i>. Guarantee
            <b>0</b> <= <i>d1</i> <= <i>d2</i>.
        @par
            If the ray isn't intersects the box, <b>false</b> is returned, and
            <i>d1</i> and <i>d2</i> is unmodified.
        */
        static bool intersects(const Ray& ray, const AlignedBox3& box,
            real* d1, real* d2);

        /** Ray / triangle intersection, returns boolean result and distance.
        @param
            ray The ray.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
		@param 
			normal The triangle plane's normal (passed in rather than calculated
				on demand since the caller may already have it), doesn't need
                normalised since we don't care.
        @param
            positiveSide Intersect with "positive side" of the triangle
        @param
            negativeSide Intersect with "negative side" of the triangle
        @returns
            If the ray is intersects the triangle, a pair of <b>true</b> and the
            distance between intersection point and ray origin returned.
        @par
            If the ray isn't intersects the triangle, a pair of <b>false</b> and
            <b>0</b> returned.
        */
        static std::pair<bool, real> intersects(const Ray& ray, const Vector3f& a,
            const Vector3f& b, const Vector3f& c, const Vector3f& normal,
            bool positiveSide = true, bool negativeSide = true);

        /** Ray / triangle intersection, returns boolean result and distance.
        @param
            ray The ray.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
        @param
            positiveSide Intersect with "positive side" of the triangle
        @param
            negativeSide Intersect with "negative side" of the triangle
        @returns
            If the ray is intersects the triangle, a pair of <b>true</b> and the
            distance between intersection point and ray origin returned.
        @par
            If the ray isn't intersects the triangle, a pair of <b>false</b> and
            <b>0</b> returned.
        */
        static std::pair<bool, real> intersects(const Ray& ray, const Vector3f& a,
            const Vector3f& b, const Vector3f& c,
            bool positiveSide = true, bool negativeSide = true);

        /** Sphere / box intersection test. */
        static bool intersects(const Sphere& sphere, const AlignedBox3& box);

        /** Plane / box intersection test. */
        static bool intersects(const Plane& plane, const AlignedBox3& box);

        /** Ray / convex plane list intersection test. 
        @param ray The ray to test with
        @param plaeList List of planes which form a convex volume
        @param normalIsOutside Does the normal point outside the volume
        */
        static std::pair<bool, real> intersects(
            const Ray& ray, const std::vector<Plane >& planeList, 
            bool normalIsOutside);
        /** Ray / convex plane list intersection test. 
        @param ray The ray to test with
        @param plaeList List of planes which form a convex volume
        @param normalIsOutside Does the normal point outside the volume
        */
        static std::pair<bool, real> intersects(
            const Ray& ray, const std::list<Plane >& planeList, 
            bool normalIsOutside);

        /** Sphere / plane intersection test. 
        @remarks NB just do a plane.getDistance(sphere.getCenter()) for more detail!
        */
        static bool intersects(const Sphere& sphere, const Plane& plane);

        /** Compare 2 reals, using tolerance for inaccuracies.
        */
        static bool floatEqual(real a, real b,
            real tolerance = std::numeric_limits<real>::epsilon());

        /** Calculates the tangent space vector for a given set of positions / texture coords. */
        static Vector3f calculateTangentSpaceVector(
            const Vector3f& position1, const Vector3f& position2, const Vector3f& position3,
            real u1, real v1, real u2, real v2, real u3, real v3);

        /** Build a reflection matrix for the passed in plane. */
        static Matrix4f buildReflectionMatrix(const Plane& p);
        /** Calculate a face normal, including the w component which is the offset from the origin. */
        static Vector4f calculateFaceNormal(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3);
        /** Calculate a face normal, no w-information. */
        static Vector3f calculateBasicFaceNormal(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3);
        /** Calculate a face normal without normalize, including the w component which is the offset from the origin. */
        static Vector4f calculateFaceNormalWithoutNormalize(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3);
        /** Calculate a face normal without normalize, no w-information. */
        static Vector3f calculateBasicFaceNormalWithoutNormalize(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3);

		/** Generates a value based on the Gaussian (normal) distribution function
			with the given offset and scale parameters.
		*/
		static real gaussianDistribution(real x, real offset = 0.0f, real scale = 1.0f);

		/** Clamp a value within an inclusive range. */
		static real Clamp(real val, real minval, real maxval)
		{
			assert (minval < maxval && "Invalid clamp range");
			return std::max(std::min(val, maxval), minval);
		}

		static AffineTransform3 makeViewMatrix(const Vector3f& position, const Quaternion& orientation);
		static Matrix4f makePerspectiveMatrix(float fov, float aspect, float nearZ, float farZ);

		/** Get a bounding radius value from a bounding box. */
		static real boundingRadiusFromAABB(const AlignedBox3& aabb);

		/** Compute a quaternion rotation transforming vector a to vector b **/
		static Quaternion buildRotation(const Vector3f& a, const Vector3f& b, const Vector3f& fallbackAxis );//= Vector3f::Zero());

		//! Converts a quaternion to euler angles (pitch, yaw, roll)
		static Vector3f quaternionToEuler(const Quaternion& quat);

		static 
		Ray unproject(const Vector2f& point, const AffineTransform3& modelview, const Transform3& projection, const Rect& viewport);
		static 
		Ray unprojectNormalized(const Vector2f& point, const AffineTransform3& modelview, const Transform3& projection);
		static 
		Vector3f project(const Vector3f& point, const AffineTransform3& modelview, const Transform3& projection, const Rect& viewport);

		static Vector3f normal(const Vector3f& aa, const Vector3f& bb, const Vector3f& cc);

		static AffineTransform3 computeMatchingPointsTransform(const Vectors3f& src, const Vectors3f& dst);

		static void swapMinMax(real& min, real& max);


        static const real PositiveInfinity;
        static const real NegativeInfinity;
        static const float Pi;
        static const real TwoPi;
        static const real HalfPi;
		static const real DegToRad;
		static const real RadToDeg;
    };

    
}
#endif

