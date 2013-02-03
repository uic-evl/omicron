/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2011		Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omicron/osystem.h"
#include "omicron/math/Math.h"
#include "omicron/math/Ray.h"
#include "omicron/math/Sphere.h"

using namespace omicron;

const real Math::PositiveInfinity = std::numeric_limits<real>::infinity();
    
const real Math::NegativeInfinity = -std::numeric_limits<real>::infinity();
    
    
const float Math::Pi = 3.1415926535f;

    
const real Math::TwoPi = real( 2.0 * Pi );
    
const real Math::HalfPi = real( 0.5 * Pi );
    
const real Math::DegToRad = Pi / real(180.0);
    
const real Math::RadToDeg = real(180.0) / Pi;
    
const real Math::Log2Base = log(real(2.0));

    
int Math::mTrigTableSize;

    
real  Math::mTrigTableFactor;
    
real *Math::mSinTable = NULL;
    
real *Math::mTanTable = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////
    
Math::Math( unsigned int trigTableSize )
{
    mTrigTableSize = trigTableSize;
    mTrigTableFactor = mTrigTableSize / Math::TwoPi;

    mSinTable = new real[mTrigTableSize];
    mTanTable = new real[mTrigTableSize];

    buildTrigTables();
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Math::~Math()
{
	delete[] mSinTable;
	delete[] mTanTable;
	mSinTable = NULL;
	mTanTable = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
void Math::buildTrigTables(void)
{
    // Build trig lookup tables
    // Could get away with building only Pi sized Sin table but simpler this 
    // way. Who cares, it'll ony use an extra 8k of memory anyway and I like 
    // simplicity.
    real angle;
    for (int i = 0; i < mTrigTableSize; ++i)
    {
        angle = Math::TwoPi * i / mTrigTableSize;
        mSinTable[i] = sin(angle);
        mTanTable[i] = tan(angle);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::SinTable (real fValue)
{
    // Convert range to index values, wrap if required
    int idx;
    if (fValue >= 0)
    {
        idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
    }
    else
    {
        idx = mTrigTableSize - (int(-fValue * mTrigTableFactor) % mTrigTableSize) - 1;
    }

    return mSinTable[idx];
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::TanTable (real fValue)
{
    // Convert range to index values, wrap if required
	int idx = int(fValue *= mTrigTableFactor) % mTrigTableSize;
	return mTanTable[idx];
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
int Math::isign (int iValue)
{
    return ( iValue > 0 ? +1 : ( iValue < 0 ? -1 : 0 ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::acos (real fValue)
{
    if ( -1.0 < fValue )
    {
        if ( fValue < 1.0 )
            return acos(fValue);
        else
            return 0.0f;
    }
    else
    {
        return Pi;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::asin (real fValue)
{
    if ( -1.0 < fValue )
    {
        if ( fValue < 1.0 )
            return asin(fValue);
        else
            return HalfPi;
    }
    else
    {
        return -HalfPi;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::sign (real fValue)
{
    if ( fValue > 0.0 )
        return 1.0;

    if ( fValue < 0.0 )
        return -1.0;

    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::unitRandom ()
{
	return ((real)rand() / RAND_MAX);
}    

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::rangeRandom (real fLow, real fHigh)
{
    return (fHigh-fLow)*unitRandom() + fLow;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::symmetricRandom ()
{
	return 2.0f * unitRandom() - 1.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// EIGEN: undefined cross for 2d vectors.    
//bool Math::pointInTri2D(const Vector2f& p, const Vector2f& a, 
//	const Vector2f& b, const Vector2f& c)
//{
//	// Winding must be consistent from all edges for point to be inside
//	Vector2f v1, v2;
//	real dot[3];
//	bool zeroDot[3];
//
//	v1 = b - a;
//	v2 = p - a;
//
//	// Note we don't care about normalisation here since sign is all we need
//	// It means we don't have to worry about magnitude of cross products either
//	dot[0] = v1.cross(v2);
//	zeroDot[0] = Math::floatEqual(dot[0], 0.0f, 1e-3f);
//
//
//	v1 = c - b;
//	v2 = p - b;
//
//	dot[1] = v1.cross(v2);
//	zeroDot[1] = Math::floatEqual(dot[1], 0.0f, 1e-3f);
//
//	// Compare signs (ignore colinear / coincident points)
//	if(!zeroDot[0] && !zeroDot[1] 
//	&& Math::sign(dot[0]) != Math::sign(dot[1]))
//	{
//		return false;
//	}
//
//	v1 = a - c;
//	v2 = p - c;
//
//	dot[2] = v1.cross(v2);
//	zeroDot[2] = Math::floatEqual(dot[2], 0.0f, 1e-3f);
//	// Compare signs (ignore colinear / coincident points)
//	if((!zeroDot[0] && !zeroDot[2] 
//		&& Math::sign(dot[0]) != Math::sign(dot[2])) ||
//		(!zeroDot[1] && !zeroDot[2] 
//		&& Math::sign(dot[1]) != Math::sign(dot[2])))
//	{
//		return false;
//	}
//
//
//	return true;
//}

///////////////////////////////////////////////////////////////////////////////////////////////
    
bool Math::pointInTri3D(const Vector3f& p, const Vector3f& a, 
	const Vector3f& b, const Vector3f& c, const Vector3f& normal)
{
    // Winding must be consistent from all edges for point to be inside
	Vector3f v1, v2;
	real dot[3];
	bool zeroDot[3];

    v1 = b - a;
    v2 = p - a;

	// Note we don't care about normalisation here since sign is all we need
	// It means we don't have to worry about magnitude of cross products either
    dot[0] = v1.cross(v2).dot(normal);
	zeroDot[0] = Math::floatEqual(dot[0], 0.0f, 1e-3f);


    v1 = c - b;
    v2 = p - b;

	dot[1] = v1.cross(v2).dot(normal);
	zeroDot[1] = Math::floatEqual(dot[1], 0.0f, 1e-3f);

	// Compare signs (ignore colinear / coincident points)
	if(!zeroDot[0] && !zeroDot[1] 
		&& Math::sign(dot[0]) != Math::sign(dot[1]))
	{
        return false;
	}

    v1 = a - c;
    v2 = p - c;

	dot[2] = v1.cross(v2).dot(normal);
	zeroDot[2] = Math::floatEqual(dot[2], 0.0f, 1e-3f);
	// Compare signs (ignore colinear / coincident points)
	if((!zeroDot[0] && !zeroDot[2] 
		&& Math::sign(dot[0]) != Math::sign(dot[2])) ||
		(!zeroDot[1] && !zeroDot[2] 
		&& Math::sign(dot[1]) != Math::sign(dot[2])))
	{
		return false;
	}


    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
bool Math::floatEqual( real a, real b, real tolerance )
{
    if (fabs(b-a) <= tolerance)
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, const Plane& plane)
{

    real denom = plane.normal.dot(ray.getDirection());
    if (Math::abs(denom) < std::numeric_limits<real>::epsilon())
    {
        // Parallel
        return std::pair<bool, real>(false, 0.0f);
    }
    else
    {
        real nom = plane.normal.dot(ray.getOrigin()) + plane.d;
        real t = -(nom/denom);
        return std::pair<bool, real>(t >= 0, t);
    }
        
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, 
    const std::vector<Plane >& planes, bool normalIsOutside)
{
	std::list<Plane > planesList;
	for (std::vector<Plane >::const_iterator i = planes.begin(); i != planes.end(); ++i)
	{
		planesList.push_back(*i);
	}
	return intersects(ray, planesList, normalIsOutside);
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, 
    const std::list<Plane >& planes, bool normalIsOutside)
{
	std::list<Plane >::const_iterator planeit, planeitend;
	planeitend = planes.end();
	bool allInside = true;
	std::pair<bool, real> ret;
	std::pair<bool, real> end;
	ret.first = false;
	ret.second = 0.0f;
	end.first = false;
	end.second = 0;


	// derive side
	// NB we don't pass directly since that would require Plane::Side in 
	// interface, which results in recursive includes since math is so fundamental
	PlaneSide outside = normalIsOutside ? POSITIVE_SIDE : NEGATIVE_SIDE;

	for (planeit = planes.begin(); planeit != planeitend; ++planeit)
	{
		const Plane& plane = *planeit;
		// is origin outside?
		if (plane.getSide(ray.getOrigin()) == outside)
		{
			allInside = false;
			// Test single plane
			std::pair<bool, real> planeRes = 
				ray.intersects(plane);
			if (planeRes.first)
			{
				// Ok, we intersected
				ret.first = true;
				// Use the most distant result since convex volume
				ret.second = std::max(ret.second, planeRes.second);
			}
			else
			{
				ret.first =false;
				ret.second=0.0f;
				return ret;
			}
		}
		else
		{
			std::pair<bool, real> planeRes = 
				ray.intersects(plane);
			if (planeRes.first)
			{
				if( !end.first )
				{
					end.first = true;
					end.second = planeRes.second;
				}
				else
				{
					end.second = std::min( planeRes.second, end.second );
				}

			}

		}
	}

	if (allInside)
	{
		// Intersecting at 0 distance since inside the volume!
		ret.first = true;
		ret.second = 0.0f;
		return ret;
	}

	if( end.first )
	{
		if( end.second < ret.second )
		{
			ret.first = false;
			return ret;
		}
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, const Sphere& sphere, 
    bool discardInside)
{
    const Vector3f& raydir = ray.getDirection();
    // Adjust ray origin relative to sphere center
    const Vector3f& rayorig = ray.getOrigin() - sphere.getCenter();
    real radius = sphere.getRadius();

    // Check origin inside first
    if (rayorig.squaredNorm() <= radius*radius && discardInside)
    {
        return std::pair<bool, real>(true, 0.0f);
    }

    // Mmm, quadratics
    // Build coeffs which can be used with std quadratic solver
    // ie t = (-b +/- sqrt(b*b + 4ac)) / 2a
    real a = raydir.dot(raydir);
    real b = 2 * rayorig.dot(raydir);
    real c = rayorig.dot(rayorig) - radius*radius;

    // Calc determinant
    real d = (b*b) - (4 * a * c);
    if (d < 0)
    {
        // No intersection
        return std::pair<bool, real>(false, 0.0f);
    }
    else
    {
        // BTW, if d=0 there is one intersection, if d > 0 there are 2
        // But we only want the closest one, so that's ok, just use the 
        // '-' version of the solver
        real t = ( -b - Math::sqrt(d) ) / (2 * a);
        if (t < 0)
            t = ( -b + Math::sqrt(d) ) / (2 * a);
        return std::pair<bool, real>(true, t);
    }


}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, const AlignedBox3& box)
{
	if (box.isNull()) return std::pair<bool, real>(false, 0);
	if (box.isInfinite()) return std::pair<bool, real>(true, 0);

	real lowt = 0.0f;
	real t;
	bool hit = false;
	Vector3f hitpoint;
	const Vector3f& min = box.getMinimum();
	const Vector3f& max = box.getMaximum();
	const Vector3f& rayorig = ray.getOrigin();
	const Vector3f& raydir = ray.getDirection();

	// Check origin inside first
	if ( (rayorig.array()  > min.array()).all() && (rayorig.array() < max.array()).all() )
	{
		return std::pair<bool, real>(true, 0);
	}

	// Check each face in turn, only check closest 3
	// Min x
	if (rayorig.x() <= min.x() && raydir.x() > 0)
	{
		t = (min.x() - rayorig.x()) / raydir.x();
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.y() >= min.y() && hitpoint.y() <= max.y() &&
				hitpoint.z() >= min.z() && hitpoint.z() <= max.z() &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max x
	if (rayorig.x() >= max.x() && raydir.x() < 0)
	{
		t = (max.x() - rayorig.x()) / raydir.x();
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.y() >= min.y() && hitpoint.y() <= max.y() &&
				hitpoint.z() >= min.z() && hitpoint.z() <= max.z() &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Min y
	if (rayorig.y() <= min.y() && raydir.y() > 0)
	{
		t = (min.y() - rayorig.y()) / raydir.y();
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x() >= min.x() && hitpoint.x() <= max.x() &&
				hitpoint.z() >= min.z() && hitpoint.z() <= max.z() &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max y
	if (rayorig.y() >= max.y() && raydir.y() < 0)
	{
		t = (max.y() - rayorig.y()) / raydir.y();
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x() >= min.x() && hitpoint.x() <= max.x() &&
				hitpoint.z() >= min.z() && hitpoint.z() <= max.z() &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Min z
	if (rayorig.z() <= min.z() && raydir.z() > 0)
	{
		t = (min.z() - rayorig.z()) / raydir.z();
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x() >= min.x() && hitpoint.x() <= max.x() &&
				hitpoint.y() >= min.y() && hitpoint.y() <= max.y() &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max z
	if (rayorig.z() >= max.z() && raydir.z() < 0)
	{
		t = (max.z() - rayorig.z()) / raydir.z();
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x() >= min.x() && hitpoint.x() <= max.x() &&
				hitpoint.y() >= min.y() && hitpoint.y() <= max.y() &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}

	return std::pair<bool, real>(hit, lowt);

} 

///////////////////////////////////////////////////////////////////////////////////////////////
    
bool Math::intersects(const Ray& ray, const AlignedBox3& box,
    real* d1, real* d2)
{
    if (box.isNull())
        return false;

    if (box.isInfinite())
    {
        if (d1) *d1 = 0;
        if (d2) *d2 = Math::PositiveInfinity;
        return true;
    }

    const Vector3f& min = box.getMinimum();
    const Vector3f& max = box.getMaximum();
    const Vector3f& rayorig = ray.getOrigin();
    const Vector3f& raydir = ray.getDirection();

    Vector3f absDir;
    absDir[0] = Math::abs(raydir[0]);
    absDir[1] = Math::abs(raydir[1]);
    absDir[2] = Math::abs(raydir[2]);

    // Sort the axis, ensure check minimise floating error axis first
    int imax = 0, imid = 1, imin = 2;
    if (absDir[0] < absDir[2])
    {
        imax = 2;
        imin = 0;
    }
    if (absDir[1] < absDir[imin])
    {
        imid = imin;
        imin = 1;
    }
    else if (absDir[1] > absDir[imax])
    {
        imid = imax;
        imax = 1;
    }

    real start = 0, end = Math::PositiveInfinity;

#define _CALC_AXIS(i)                                       \
do {                                                    \
    real denom = 1 / raydir[i];                         \
    real newstart = (min[i] - rayorig[i]) * denom;      \
    real newend = (max[i] - rayorig[i]) * denom;        \
    if (newstart > newend) std::swap(newstart, newend); \
    if (newstart > end || newend < start) return false; \
    if (newstart > start) start = newstart;             \
    if (newend < end) end = newend;                     \
} while(0)

    // Check each axis in turn

    _CALC_AXIS(imax);

    if (absDir[imid] < std::numeric_limits<real>::epsilon())
    {
        // Parallel with middle and minimise axis, check bounds only
        if (rayorig[imid] < min[imid] || rayorig[imid] > max[imid] ||
            rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
            return false;
    }
    else
    {
        _CALC_AXIS(imid);

        if (absDir[imin] < std::numeric_limits<real>::epsilon())
        {
            // Parallel with minimise axis, check bounds only
            if (rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
                return false;
        }
        else
        {
            _CALC_AXIS(imin);
        }
    }
#undef _CALC_AXIS

    if (d1) *d1 = start;
    if (d2) *d2 = end;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, const Vector3f& a,
    const Vector3f& b, const Vector3f& c, const Vector3f& normal,
    bool positiveSide, bool negativeSide)
{
    //
    // Calculate intersection with plane.
    //
    real t;
    {
        real denom = normal.dot(ray.getDirection());

        // Check intersect side
        if (denom > + std::numeric_limits<real>::epsilon())
        {
            if (!negativeSide)
                return std::pair<bool, real>(false, 0);
        }
        else if (denom < - std::numeric_limits<real>::epsilon())
        {
            if (!positiveSide)
                return std::pair<bool, real>(false, 0);
        }
        else
        {
            // Parallel or triangle area is close to zero when
            // the plane normal not normalized.
            return std::pair<bool, real>(false, 0);
        }

        t = normal.dot(a - ray.getOrigin()) / denom;

        if (t < 0)
        {
            // Intersection is behind origin
            return std::pair<bool, real>(false, 0);
        }
    }

    //
    // Calculate the largest area projection plane in X, Y or Z.
    //
    size_t i0, i1;
    {
        real n0 = Math::abs(normal[0]);
        real n1 = Math::abs(normal[1]);
        real n2 = Math::abs(normal[2]);

        i0 = 1; i1 = 2;
        if (n1 > n2)
        {
            if (n1 > n0) i0 = 0;
        }
        else
        {
            if (n2 > n0) i1 = 0;
        }
    }

    //
    // Check the intersection point is inside the triangle.
    //
    {
        real u1 = b[i0] - a[i0];
        real v1 = b[i1] - a[i1];
        real u2 = c[i0] - a[i0];
        real v2 = c[i1] - a[i1];
        real u0 = t * ray.getDirection()[i0] + ray.getOrigin()[i0] - a[i0];
        real v0 = t * ray.getDirection()[i1] + ray.getOrigin()[i1] - a[i1];

        real alpha = u0 * v2 - u2 * v0;
        real beta  = u1 * v0 - u0 * v1;
        real area  = u1 * v2 - u2 * v1;

        // epsilon to avoid real precision error
        const real EPSILON = 1e-6f;

        real tolerance = - EPSILON * area;

        if (area > 0)
        {
            if (alpha < tolerance || beta < tolerance || alpha+beta > area-tolerance)
                return std::pair<bool, real>(false, 0);
        }
        else
        {
            if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
                return std::pair<bool, real>(false, 0);
        }
    }

    return std::pair<bool, real>(true, t);
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
std::pair<bool, real> Math::intersects(const Ray& ray, const Vector3f& a,
    const Vector3f& b, const Vector3f& c,
    bool positiveSide, bool negativeSide)
{
    Vector3f normal = calculateBasicFaceNormalWithoutNormalize(a, b, c);
    return intersects(ray, a, b, c, normal, positiveSide, negativeSide);
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
bool Math::intersects(const Sphere& sphere, const AlignedBox3& box)
{
    if (box.isNull()) return false;
    if (box.isInfinite()) return true;

    // Use splitting planes
    const Vector3f& center = sphere.getCenter();
    real radius = sphere.getRadius();
    const Vector3f& min = box.getMinimum();
    const Vector3f& max = box.getMaximum();

	// Arvo's algorithm
	real s, d = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (center[i] < min[i])
		{
			s = center[i] - min[i];
			d += s * s; 
		}
		else if(center[i] > max[i])
		{
			s = center[i] - max[i];
			d += s * s; 
		}
	}
	return d <= radius * radius;

}

///////////////////////////////////////////////////////////////////////////////////////////////
    
//bool Math::intersects(const Plane& plane, const AlignedBox3& box)
//{
//    return (plane.getSide(box) == BOTH_SIDE);
//}

///////////////////////////////////////////////////////////////////////////////////////////////
    
bool Math::intersects(const Sphere& sphere, const Plane& plane)
{
    return (
        Math::abs(plane.getDistance(sphere.getCenter()))
        <= sphere.getRadius() );
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Vector3f Math::calculateTangentSpaceVector(
    const Vector3f& position1, const Vector3f& position2, const Vector3f& position3,
    real u1, real v1, real u2, real v2, real u3, real v3)
{
	//side0 is the vector along one side of the triangle of vertices passed in, 
	//and side1 is the vector along another side. Taking the cross product of these returns the normal.
	Vector3f side0 = position1 - position2;
	Vector3f side1 = position3 - position1;
	//Calculate face normal
	Vector3f normal = side1.cross(side0);
	normal.normalize();
	//Now we use a formula to calculate the tangent. 
	real deltaV0 = v1 - v2;
	real deltaV1 = v3 - v1;
	Vector3f tangent = deltaV1 * side0 - deltaV0 * side1;
	tangent.normalize();
	//Calculate binormal
	real deltaU0 = u1 - u2;
	real deltaU1 = u3 - u1;
	Vector3f binormal = deltaU1 * side0 - deltaU0 * side1;
	binormal.normalize();
	//Now, we take the cross product of the tangents to get a vector which 
	//should point in the same direction as our normal calculated above. 
	//If it points in the opposite direction (the dot product between the normals is less than zero), 
	//then we need to reverse the s and t tangents. 
	//This is because the triangle has been mirrored when going from tangent space to object space.
	//reverse tangents if necessary
	Vector3f tangentCross = tangent.cross(binormal);
	if (tangentCross.dot(normal) < 0.0f)
	{
		tangent = -tangent;
		binormal = -binormal;
	}

    return tangent;

}
//-----------------------------------------------------------------------
//Matrix4f Math::buildReflectionMatrix(const Plane& p)
//{
//    return Matrix4f(
//        -2 * p.normal.x() * p.normal.x() + 1,   -2 * p.normal.x() * p.normal.y(),       -2 * p.normal.x() * p.normal.z(),       -2 * p.normal.x() * p.d, 
//        -2 * p.normal.y() * p.normal.x(),       -2 * p.normal.y() * p.normal.y() + 1,   -2 * p.normal.y() * p.normal.z(),       -2 * p.normal.y() * p.d, 
//        -2 * p.normal.z() * p.normal.x(),       -2 * p.normal.z() * p.normal.y(),       -2 * p.normal.z() * p.normal.z() + 1,   -2 * p.normal.z() * p.d, 
//        0,                                  0,                                  0,                                  1);
//}
///////////////////////////////////////////////////////////////////////////////////////////////
    
Vector4f Math::calculateFaceNormal(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
    Vector3f normal = calculateBasicFaceNormal(v1, v2, v3);
    // Now set up the w (distance of tri from origin
    return Vector4f(normal.x(), normal.y(), normal.z(), -(normal.dot(v1)));
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Vector3f Math::calculateBasicFaceNormal(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
    Vector3f normal = (v2 - v1).cross(v3 - v1);
    normal.normalize();
    return normal;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Vector4f Math::calculateFaceNormalWithoutNormalize(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
    Vector3f normal = calculateBasicFaceNormalWithoutNormalize(v1, v2, v3);
    // Now set up the w (distance of tri from origin)
    return Vector4f(normal.x(), normal.y(), normal.z(), -(normal.dot(v1)));
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Vector3f Math::calculateBasicFaceNormalWithoutNormalize(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
    Vector3f normal = (v2 - v1).cross(v3 - v1);
    return normal;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::gaussianDistribution(real x, real offset, real scale)
{
	real nom = Math::exp(
		-Math::sqr(x - offset) / (2 * Math::sqr(scale)));
	real denom = scale * Math::sqrt(2 * Math::Pi);

	return nom / denom;

}
	
///////////////////////////////////////////////////////////////////////////////////////////////
    
AffineTransform3 Math::makeViewMatrix(const Vector3f& position, const Quaternion& orientation)
{
	AffineTransform3 viewMatrix;

	// View matrix is:
	//
	//  [ Lx  Uy  Dz  Tx  ]
	//  [ Lx  Uy  Dz  Ty  ]
	//  [ Lx  Uy  Dz  Tz  ]
	//  [ 0   0   0   1   ]
	//
	// Where real = -(Transposed(Rot) * Pos)

	// This is most efficiently done using 3x3 Matrices
	Matrix3f rot = orientation.toRotationMatrix();
		
	//orientation.get_rotation_matrix(rot);
	//rot = orientation;

	// Make the translation relative to new axes
	Matrix3f rotT;
	rotT = rot.transpose();
	Vector3f trans = -rotT * position;

	// Make final matrix
	viewMatrix = Matrix4f::Identity();
	viewMatrix = rotT; // fills upper 3x3
	viewMatrix(0, 3) = trans.x();
	viewMatrix(1, 3) = trans.y();
	viewMatrix(2, 3) = trans.z();
	viewMatrix(3, 3) = 1.0f;

	return viewMatrix;

}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Matrix4f Math::makePerspectiveMatrix(float fov, float a, float n, float f)
{
	Matrix4f m = Matrix4f::Zero();
	float e = 1.0f / tan(fov / 2);

	m(0,0) = e;
	m(1,1) = e / a;
	m(2,2) = - (f + n) / (f - n);
	m(2,3) = - (2 * f * n) / (f - n);
	m(3,2) = -1;

	return m;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
real Math::boundingRadiusFromAABB(const AlignedBox3& aabb)
{
	Vector3f max = aabb.getMaximum();
	Vector3f min = aabb.getMinimum();

	Vector3f magnitude = max;
	magnitude = magnitude.cwiseMax(-max);
	magnitude = magnitude.cwiseMax(min);
	magnitude = magnitude.cwiseMax(-min);

	return magnitude.norm();
}

	/** Gets the shortest arc quaternion to rotate this vector to the destination
	vector.
@remarks
	If you call this with a dest vector that is close to the inverse
	of this vector, we will rotate 180 degrees around the 'fallbackAxis'
	(if specified, or a generated axis if not) since in this case
	ANY axis of rotation is valid.
*/
    
Quaternion Math::buildRotation(const Vector3f& a, const Vector3f& b,
	const Vector3f& fallbackAxis)
{
	// Based on Stan Melax's article in Game Programming Gems
	Quaternion q;
	// Copy, since cannot modify local
	Vector3f v0 = a;
	Vector3f v1 = b;
	v0.normalize();
	v1.normalize();

	real d = v0.dot(v1);
	// If dot == 1, vectors are the same
	if (d >= 1.0f)
	{
		return Quaternion::Identity();
	}
	if (d < (1e-6f - 1.0f))
	{
		if (fallbackAxis != Vector3f::Zero())
		{
			// rotate 180 degrees about the fallback axis
			q = Eigen::AngleAxis<real>(Math::Pi, fallbackAxis);
		}
		else
		{
			// Generate an axis
			Vector3f axis = Vector3f::UnitX().cross(a);
			if (axis.norm() == 0) // pick another if colinear
				axis = Vector3f::UnitY().cross(a);
			axis.normalize();
			q = Eigen::AngleAxis<real>(Math::Pi, axis);
		}
	}
	else
	{
		real s = Math::sqrt( (1+d)*2 );
		real invs = 1 / s;

		Vector3f c = v0.cross(v1);

    	q.x() = c.x() * invs;
		q.y() = c.y() * invs;
		q.z() = c.z() * invs;
		q.w() = s * 0.5f;
		q.normalize();
	}
	return q;
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Ray Math::unproject(
	const Vector2f& point, const AffineTransform3& modelview, 
	const Transform3& projection, const Rect& viewport)
{
	Vector2f np;
	np[0]=(point[0] - (float)viewport.x()) / (float)(viewport.width()) * 2.0f - 1.0f;
    np[1]=((viewport.height() - point[1]) - (float)viewport.y()) / (float)(viewport.height()) * 2.0f - 1.0f;

	return unprojectNormalized(np, modelview, projection);
}

///////////////////////////////////////////////////////////////////////////////////////////////
    
Ray Math::unprojectNormalized(
	const Vector2f& point, const AffineTransform3& modelview, 
	const Transform3& projection)
{
	Vector3f origin;
	Vector3f direction;

	Matrix4f A = projection.matrix() * modelview.matrix();
	Matrix4f m;
	m = A.inverse();
	//if(!A.inverse(m)) return Ray();

	Vector4f in;
	in[0]= point[0];
    in[1]= point[1];
	// z == projection plane z.
    in[2]= -1.0f;
    in[3]=1.0f;

	Vector4f m1 = m * in;
	m1 = m1 / m1[3];

	// z == far projection plane z (I guess we could choose a different value since we are just using this
	// to compute the ray direction).
    in[2] = 1.0f;
	Vector4f m2 = m * in;
	m2 = m2 / m2[3];

	//float t = (z - m1[2]) / (m2[2] - m1[2]);
	//origin[0] = m1[0] + t * (m2[0] - m1[0]);
	//origin[1] = m1[1] + t * (m2[1] - m1[1]);
	//origin[2] = z;

	origin[0] = m1[0];
	origin[1] = m1[1];
	origin[2] = m1[2];

	direction = Vector3f((m2[0] - m1[0]), (m2[1] - m1[1]), (m2[2] - m1[2]));
	direction.normalize();
	//direction *= -1;

	return Ray(origin, direction);
}

///////////////////////////////////////////////////////////////////////////////////////////////
Vector3f Math::project(
	const Vector3f& point, const AffineTransform3& mmodelview, 
	const Transform3& mprojection, const Rect& rviewport)
{
		const float* modelview = mmodelview.data();
		const float* projection = mprojection.data();
		float viewport[4];

		viewport[0] = rviewport.x();
		viewport[1] = rviewport.y();
		viewport[2] = rviewport.width();
		viewport[3] = rviewport.height();

		float objx = point[0];
		float objy = point[1];
		float objz = point[2];

		Vector3f windowCoordinate;

		//Transformation vectors
		float fTempo[8];
		//Modelview transform
		fTempo[0]=modelview[0]*objx+modelview[4]*objy+modelview[8]*objz+modelview[12];  //w is always 1
		fTempo[1]=modelview[1]*objx+modelview[5]*objy+modelview[9]*objz+modelview[13];
		fTempo[2]=modelview[2]*objx+modelview[6]*objy+modelview[10]*objz+modelview[14];
		fTempo[3]=modelview[3]*objx+modelview[7]*objy+modelview[11]*objz+modelview[15];
		//Projection transform, the final row of projection matrix is always [0 0 -1 0]
		//so we optimize for that.
		fTempo[4]=projection[0]*fTempo[0]+projection[4]*fTempo[1]+projection[8]*fTempo[2]+projection[12]*fTempo[3];
		fTempo[5]=projection[1]*fTempo[0]+projection[5]*fTempo[1]+projection[9]*fTempo[2]+projection[13]*fTempo[3];
		fTempo[6]=projection[2]*fTempo[0]+projection[6]*fTempo[1]+projection[10]*fTempo[2]+projection[14]*fTempo[3];
		fTempo[7]=-fTempo[2];
		//The result normalizes between -1 and 1
		//if(fTempo[7]==0.0)	//The w value
		// return 0;
		fTempo[7]=1.0/fTempo[7];
		//Perspective division
		fTempo[4]*=fTempo[7];
		fTempo[5]*=fTempo[7];
		fTempo[6]*=fTempo[7];
		//Window coordinates
		//Map x, y to range 0-1
		windowCoordinate[0]=(fTempo[4]*0.5+0.5)*viewport[2]+viewport[0];
		windowCoordinate[1]=viewport[3] - (fTempo[5]*0.5+0.5)*viewport[3]+viewport[1];
		//This is only correct when glDepthRange(0.0, 1.0)
		windowCoordinate[2]=(1.0+fTempo[6])*0.5;	//Between 0 and 1
		return windowCoordinate;
}

///////////////////////////////////////////////////////////////////////////////////////////////
Vector3f Math::normal(
	const Vector3f& aa, 
	const Vector3f& bb, 
	const Vector3f& cc
	) 
{
	Vector3f u,v,tmp;
	// right hand system, CCW triangle
	u = bb - aa;
	v = cc - aa;

	tmp = u.cross(v);
	tmp.normalize();
	return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////
AffineTransform3 Math::computeMatchingPointsTransform(
	const Vectors3f& src, 
	const Vectors3f& dst)
{
	Matrix4f result = Eigen::umeyama(src, dst);
	return AffineTransform3(result);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void Math::swapMinMax(real& min, real& max)
{
	if(min > max)
	{
		float tmp = min;
		min = max;
		max = tmp;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
Vector3f Math::quaternionToEuler(const Quaternion& q)
{
	Vector3f res;
	float q0 = q.x();
	float q1 = q.y();
	float q2 = q.z();
	float q3 = q.w();
	res[0] = asin(2 * (q0 * q2 - q3 * q1));
	res[1] = atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (q2 * q2 + q3 * q3));
	res[2] = atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (q1 * q1 + q2 * q2));
	return res;
}
