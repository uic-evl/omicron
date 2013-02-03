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
#ifndef __AXIS_ALIGNED_BOX_H_
#define __AXIS_ALIGNED_BOX_H_

#include "Sphere.h"
//#include "EigenWrappers.h"

namespace omicron { 
	/** A 3D box aligned with the x/y/z axes.
	@remarks
	This class represents a simple box which is aligned with the
	axes. Internally it only stores 2 points as the extremeties of
	the box, one which is the minima of all 3 axes, and the other
	which is the maxima of all 3 axes. This class is typically used
	for an axis-aligned bounding box (AABB) for collision and
	visibility determination.
	*/
	class AlignedBox3
	{
	public:
		enum Extent
		{
			EXTENT_NULL,
			EXTENT_FINITE,
			EXTENT_INFINITE
		};
	protected:

		Vector3f mMinimum;
		Vector3f mMaximum;
		Extent mExtent;
		mutable Vector3f* mpCorners;

	public:
		/*
		1-----2
		/|    /|
		/ |   / |
		5-----4  |
		|  0--|--3
		| /   | /
		|/    |/
		6-----7
		*/
		typedef enum {
			FAR_LEFT_BOTTOM = 0,
			FAR_LEFT_TOP = 1,
			FAR_RIGHT_TOP = 2,
			FAR_RIGHT_BOTTOM = 3,
			NEAR_RIGHT_BOTTOM = 7,
			NEAR_LEFT_BOTTOM = 6,
			NEAR_LEFT_TOP = 5,
			NEAR_RIGHT_TOP = 4
		} CornerEnum;
		inline AlignedBox3() : mMinimum(Vector3f::Zero()), mMaximum(Vector3f::Ones()), mpCorners(0)
		{
			// Default to a null box 
			setMinimum( -0.5, -0.5, -0.5 );
			setMaximum( 0.5, 0.5, 0.5 );
			mExtent = EXTENT_NULL;
		}
		inline AlignedBox3(Extent e) : mMinimum(Vector3f::Zero()), mMaximum(Vector3f::Ones()), mpCorners(0)
		{
			setMinimum( -0.5, -0.5, -0.5 );
			setMaximum( 0.5, 0.5, 0.5 );
			mExtent = e;
		}

		inline AlignedBox3(const AlignedBox3 & rkBox) : mMinimum(Vector3f::Zero()), mMaximum(Vector3f::Ones()), mpCorners(0)

		{
			if (rkBox.isNull())
				setNull();
			else if (rkBox.isInfinite())
				setInfinite();
			else
				setExtents( rkBox.mMinimum, rkBox.mMaximum );
		}

		inline AlignedBox3( const Vector3f& min, const Vector3f& max ) : mMinimum(Vector3f::Zero()), mMaximum(Vector3f::Ones()), mpCorners(0)
		{
			setExtents( min, max );
		}

		inline AlignedBox3(
			float mx, float my, float mz,
			float Mx, float My, float Mz ) : mMinimum(Vector3f::Zero()), mMaximum(Vector3f::Ones()), mpCorners(0)
		{
			setExtents( mx, my, mz, Mx, My, Mz );
		}

		AlignedBox3& operator=(const AlignedBox3& rhs)
		{
			// Specifically override to avoid copying mpCorners
			if (rhs.isNull())
				setNull();
			else if (rhs.isInfinite())
				setInfinite();
			else
				setExtents(rhs.mMinimum, rhs.mMaximum);

			return *this;
		}

		~AlignedBox3()
		{
			if (mpCorners) 
			{
				delete[] mpCorners;
				mpCorners = NULL;
			}
		}


		/** Gets the minimum corner of the box.
		*/
		inline const Vector3f& getMinimum(void) const
		{ 
			return mMinimum; 
		}

		/** Gets a modifiable version of the minimum
		corner of the box.
		*/
		inline Vector3f& getMinimum(void)
		{ 
			return mMinimum; 
		}

		/** Gets the maximum corner of the box.
		*/
		inline const Vector3f& getMaximum(void) const
		{ 
			return mMaximum;
		}

		/** Gets a modifiable version of the maximum
		corner of the box.
		*/
		inline Vector3f& getMaximum(void)
		{ 
			return mMaximum;
		}


		/** Sets the minimum corner of the box.
		*/
		inline void setMinimum( const Vector3f& vec )
		{
			mExtent = EXTENT_FINITE;
			mMinimum = vec;
		}

		inline void setMinimum( real x, real y, real z )
		{
			mExtent = EXTENT_FINITE;
			mMinimum.x() = x;
			mMinimum.y() = y;
			mMinimum.z() = z;
		}

		/** Changes one of the components of the minimum corner of the box
		used to resize only one dimension of the box
		*/
		inline void setMinimumX(real x)
		{
			mMinimum.x() = x;
		}

		inline void setMinimumY(real y)
		{
			mMinimum.y() = y;
		}

		inline void setMinimumZ(real z)
		{
			mMinimum.z() = z;
		}

		/** Sets the maximum corner of the box.
		*/
		inline void setMaximum( const Vector3f& vec )
		{
			mExtent = EXTENT_FINITE;
			mMaximum = vec;
		}

		inline void setMaximum( real x, real y, real z )
		{
			mExtent = EXTENT_FINITE;
			mMaximum.x() = x;
			mMaximum.y() = y;
			mMaximum.z() = z;
		}

		/** Changes one of the components of the maximum corner of the box
		used to resize only one dimension of the box
		*/
		inline void setMaximumX( real x )
		{
			mMaximum.x() = x;
		}

		inline void setMaximumY( real y )
		{
			mMaximum.y() = y;
		}

		inline void setMaximumZ( real z )
		{
			mMaximum.z() = z;
		}

		/** Sets both minimum and maximum extents at once.
		*/
		inline void setExtents( const Vector3f& min, const Vector3f& max )
		{
            assert( (min.x() <= max.x() && min.y() <= max.y() && min.z() <= max.z()) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );

			mExtent = EXTENT_FINITE;
			mMinimum = min;
			mMaximum = max;
		}

		inline void setExtents(
			real mx, real my, real mz,
			real Mx, real My, real Mz )
		{
            assert( (mx <= Mx && my <= My && mz <= Mz) &&
                "The minimum corner of the box must be less than or equal to maximum corner" );

			mExtent = EXTENT_FINITE;

			mMinimum.x() = mx;
			mMinimum.y() = my;
			mMinimum.z() = mz;

			mMaximum.x() = Mx;
			mMaximum.y() = My;
			mMaximum.z() = Mz;

		}

		/** Returns a pointer to an array of 8 corner points, useful for
		collision vs. non-aligned objects.
		@remarks
		If the order of these corners is important, they are as
		follows: The 4 points of the minimum Z face (note that
		because Ogre uses right-handed coordinates, the minimum Z is
		at the 'back' of the box) starting with the minimum point of
		all, then anticlockwise around this face (if you are looking
		onto the face from outside the box). Then the 4 points of the
		maximum Z face, starting with maximum point of all, then
		anticlockwise around this face (looking onto the face from
		outside the box). Like this:
		<pre>
		1-----2
		/|    /|
		/ |   / |
		5-----4  |
		|  0--|--3
		| /   | /
		|/    |/
		6-----7
		</pre>
		@remarks as this implementation uses a static member, make sure to use your own copy !
		*/
		inline const Vector3f* getAllCorners(void) const
		{
			assert( (mExtent == EXTENT_FINITE) && "Can't get corners of a null or infinite AAB" );

			// The order of these items is, using right-handed co-ordinates:
			// Minimum Z face, starting with Min(all), then anticlockwise
			//   around face (looking onto the face)
			// Maximum Z face, starting with Max(all), then anticlockwise
			//   around face (looking onto the face)
			// Only for optimization/compatibility.
			if (!mpCorners)
				mpCorners = new Vector3f[8];

			mpCorners[0] = mMinimum;
			mpCorners[1].x() = mMinimum.x(); mpCorners[1].y() = mMaximum.y(); mpCorners[1].z() = mMinimum.z();
			mpCorners[2].x() = mMaximum.x(); mpCorners[2].y() = mMaximum.y(); mpCorners[2].z() = mMinimum.z();
			mpCorners[3].x() = mMaximum.x(); mpCorners[3].y() = mMinimum.y(); mpCorners[3].z() = mMinimum.z();            

			mpCorners[4] = mMaximum;
			mpCorners[5].x() = mMinimum.x(); mpCorners[5].y() = mMaximum.y(); mpCorners[5].z() = mMaximum.z();
			mpCorners[6].x() = mMinimum.x(); mpCorners[6].y() = mMinimum.y(); mpCorners[6].z() = mMaximum.z();
			mpCorners[7].x() = mMaximum.x(); mpCorners[7].y() = mMinimum.y(); mpCorners[7].z() = mMaximum.z();

			return mpCorners;
		}

		/** gets the position of one of the corners
		*/
		Vector3f getCorner(CornerEnum cornerToGet) const
		{
			switch(cornerToGet)
			{
			case FAR_LEFT_BOTTOM:
				return mMinimum;
			case FAR_LEFT_TOP:
				return Vector3f(mMinimum.x(), mMaximum.y(), mMinimum.z());
			case FAR_RIGHT_TOP:
				return Vector3f(mMaximum.x(), mMaximum.y(), mMinimum.z());
			case FAR_RIGHT_BOTTOM:
				return Vector3f(mMaximum.x(), mMinimum.y(), mMinimum.z());
			case NEAR_RIGHT_BOTTOM:
				return Vector3f(mMaximum.x(), mMinimum.y(), mMaximum.z());
			case NEAR_LEFT_BOTTOM:
				return Vector3f(mMinimum.x(), mMinimum.y(), mMaximum.z());
			case NEAR_LEFT_TOP:
				return Vector3f(mMinimum.x(), mMaximum.y(), mMaximum.z());
			case NEAR_RIGHT_TOP:
				return mMaximum;
			default:
				return Vector3f();
			}
		}

		/** Merges the passed in box into the current box. The result is the
		box which encompasses both.
		*/
		void merge( const AlignedBox3& rhs )
		{
			// Do nothing if rhs null, or this is infinite
			if ((rhs.mExtent == EXTENT_NULL) || (mExtent == EXTENT_INFINITE))
			{
				return;
			}
			// Otherwise if rhs is infinite, make this infinite, too
			else if (rhs.mExtent == EXTENT_INFINITE)
			{
				mExtent = EXTENT_INFINITE;
			}
			// Otherwise if current null, just take rhs
			else if (mExtent == EXTENT_NULL)
			{
				setExtents(rhs.mMinimum, rhs.mMaximum);
			}
			// Otherwise merge
			else
			{
				Vector3f min = mMinimum.cwiseMin(rhs.mMinimum);
				Vector3f max = mMaximum.cwiseMax(rhs.mMaximum);
				setExtents(min, max);
			}

		}

		/** Extends the box to encompass the specified point (if needed).
		*/
		inline void merge( const Vector3f& point )
		{
			switch (mExtent)
			{
			case EXTENT_NULL: // if null, use this point
				setExtents(point, point);
				return;

			case EXTENT_FINITE:
				mMaximum = mMaximum.cwiseMax(point);
				mMinimum = mMinimum.cwiseMin(point);
				return;

			case EXTENT_INFINITE: // if infinite, makes no difference
				return;
			}

			assert( false && "Never reached" );
		}

		/** Transforms the box according to the matrix supplied.
		@remarks
		By calling this method you get the axis-aligned box which
		surrounds the transformed version of this box. Therefore each
		corner of the box is transformed by the matrix, then the
		extents are mapped back onto the axes to produce another
		AABB. Useful when you have a local AABB for an object which
		is then transformed.
		*/
		//inline void transform( const Matrix4f& matrix )
		//{
		//	// Do nothing if current null or infinite
		//	if( mExtent != EXTENT_FINITE )
		//		return;

		//	Vector3f oldMin, oldMax, currentCorner;

		//	// Getting the old values so that we can use the existing merge method.
		//	oldMin = mMinimum;
		//	oldMax = mMaximum;

		//	// reset
		//	setNull();

		//	// We sequentially compute the corners in the following order :
		//	// 0, 6, 5, 1, 2, 4 ,7 , 3
		//	// This sequence allows us to only change one member at a time to get at all corners.

		//	// For each one, we transform it using the matrix
		//	// Which gives the resulting point and merge the resulting point.

		//	// First corner 
		//	// min min min
		//	currentCorner = oldMin;
		//	merge( matrix * currentCorner );

		//	// min,min,max
		//	currentCorner.z() = oldMax.z();
		//	merge( matrix * currentCorner );

		//	// min max max
		//	currentCorner.y() = oldMax.y();
		//	merge( matrix * currentCorner );

		//	// min max min
		//	currentCorner.z() = oldMin.z();
		//	merge( matrix * currentCorner );

		//	// max max min
		//	currentCorner.x() = oldMax.x();
		//	merge( matrix * currentCorner );

		//	// max max max
		//	currentCorner.z() = oldMax.z();
		//	merge( matrix * currentCorner );

		//	// max min max
		//	currentCorner.y() = oldMin.y();
		//	merge( matrix * currentCorner );

		//	// max min min
		//	currentCorner.z() = oldMin.z();
		//	merge( matrix * currentCorner ); 
		//}

		/** Transforms the box according to the affine matrix supplied.
		@remarks
		By calling this method you get the axis-aligned box which
		surrounds the transformed version of this box. Therefore each
		corner of the box is transformed by the matrix, then the
		extents are mapped back onto the axes to produce another
		AABB. Useful when you have a local AABB for an object which
		is then transformed.
		@note
		The matrix must be an affine matrix. @see Matrix4f::isAffine.
		*/
		inline void transformAffine(const AffineTransform3& tf)
		{
			// Do nothing if current null or infinite
			if ( mExtent != EXTENT_FINITE )
				return;

			Vector3f centre = getCenter();
			Vector3f halfSize = getHalfSize();

			Matrix4f m = tf.matrix();

			Vector3f newCentre = tf * centre;
			Vector3f newHalfSize(
				Math::abs(m(0, 0)) * halfSize.x() + Math::abs(m(0, 1)) * halfSize.y() + Math::abs(m(0, 2)) * halfSize.z(), 
				Math::abs(m(1, 0)) * halfSize.x() + Math::abs(m(1, 1)) * halfSize.y() + Math::abs(m(1, 2)) * halfSize.z(),
				Math::abs(m(2, 0)) * halfSize.x() + Math::abs(m(2, 1)) * halfSize.y() + Math::abs(m(2, 2)) * halfSize.z());

			setExtents(newCentre - newHalfSize, newCentre + newHalfSize);
		}

		/** Sets the box to a 'null' value i.e. not a box.
		*/
		inline void setNull()
		{
			mExtent = EXTENT_NULL;
		}

		/** Returns true if the box is null i.e. empty.
		*/
		inline bool isNull(void) const
		{
			return (mExtent == EXTENT_NULL);
		}

		/** Returns true if the box is finite.
		*/
		bool isFinite(void) const
		{
			return (mExtent == EXTENT_FINITE);
		}

		/** Sets the box to 'infinite'
		*/
		inline void setInfinite()
		{
			mExtent = EXTENT_INFINITE;
		}

		/** Returns true if the box is infinite.
		*/
		bool isInfinite(void) const
		{
			return (mExtent == EXTENT_INFINITE);
		}

		/** Returns whether or not this box intersects another. */
		inline bool intersects(const AlignedBox3& b2) const
		{
			// Early-fail for nulls
			if (this->isNull() || b2.isNull())
				return false;

			// Early-success for infinites
			if (this->isInfinite() || b2.isInfinite())
				return true;

			// Use up to 6 separating planes
			if (mMaximum.x() < b2.mMinimum.x())
				return false;
			if (mMaximum.y() < b2.mMinimum.y())
				return false;
			if (mMaximum.z() < b2.mMinimum.z())
				return false;

			if (mMinimum.x() > b2.mMaximum.x())
				return false;
			if (mMinimum.y() > b2.mMaximum.y())
				return false;
			if (mMinimum.z() > b2.mMaximum.z())
				return false;

			// otherwise, must be intersecting
			return true;

		}

		/// Calculate the area of intersection of this box and another
		inline AlignedBox3 intersection(const AlignedBox3& b2) const
		{
            if (this->isNull() || b2.isNull())
			{
				return AlignedBox3();
			}
			else if (this->isInfinite())
			{
				return b2;
			}
			else if (b2.isInfinite())
			{
				return *this;
			}

			Vector3f intMin = mMinimum;
            Vector3f intMax = mMaximum;


			intMin = intMin.cwiseMax(b2.getMinimum());
			intMax = intMax.cwiseMax(b2.getMaximum());

            // Check intersection isn't null
            if (intMin.x() < intMax.x() &&
                intMin.y() < intMax.y() &&
                intMin.z() < intMax.z())
            {
                return AlignedBox3(intMin, intMax);
            }

            return AlignedBox3();
		}

		/// Calculate the volume of this box
		float volume(void) const
		{
			switch (mExtent)
			{
			case EXTENT_NULL:
				return 0.0f;

			case EXTENT_FINITE:
				{
					Vector3f diff = mMaximum - mMinimum;
					return diff.x() * diff.y() * diff.z();
				}

			case EXTENT_INFINITE:
				return Math::PositiveInfinity;

			default: // shut up compiler
				assert( false && "Never reached" );
				return 0.0f;
			}
		}

		/** Scales the AABB by the vector given. */
		inline void scale(const Vector3f& s)
		{
			// Do nothing if current null or infinite
			if (mExtent != EXTENT_FINITE)
				return;

			// NB assumes centered on origin
			Vector3f min = mMinimum.cwiseProduct(s);
			Vector3f max = mMaximum.cwiseProduct(s);
			setExtents(min, max);
		}

		/** Tests whether this box intersects a sphere. */
		bool intersects(const Sphere& s) const
		{
			return Math::intersects(s, *this); 
		}
		/** Tests whether this box intersects a plane. */
		bool intersects(const Plane& p) const
		{
			return Math::intersects(p, *this);
		}
		/** Tests whether the vector point is within this box. */
		bool intersects(const Vector3f& v) const
		{
			switch (mExtent)
			{
			case EXTENT_NULL:
				return false;

			case EXTENT_FINITE:
				return(v.x() >= mMinimum.x()  &&  v.x() <= mMaximum.x()  && 
					v.y() >= mMinimum.y()  &&  v.y() <= mMaximum.y()  && 
					v.z() >= mMinimum.z()  &&  v.z() <= mMaximum.z());

			case EXTENT_INFINITE:
				return true;

			default: // shut up compiler
				assert( false && "Never reached" );
				return false;
			}
		}
		/// Gets the centre of the box
		Vector3f getCenter(void) const
		{
			assert( (mExtent == EXTENT_FINITE) && "Can't get center of a null or infinite AAB" );

			return Vector3f(
				(mMaximum.x() + mMinimum.x()) * 0.5f,
				(mMaximum.y() + mMinimum.y()) * 0.5f,
				(mMaximum.z() + mMinimum.z()) * 0.5f);
		}
		/// Gets the size of the box
		Vector3f getSize(void) const
		{
			switch (mExtent)
			{
			case EXTENT_NULL:
				return Vector3f::Zero();

			case EXTENT_FINITE:
				return mMaximum - mMinimum;

			case EXTENT_INFINITE:
				return Vector3f(
					Math::PositiveInfinity,
					Math::PositiveInfinity,
					Math::PositiveInfinity);

			default: // shut up compiler
				assert( false && "Never reached" );
				return Vector3f::Zero();
			}
		}
		/// Gets the half-size of the box
		Vector3f getHalfSize(void) const
		{
			switch (mExtent)
			{
			case EXTENT_NULL:
				return Vector3f::Zero();

			case EXTENT_FINITE:
				return (mMaximum - mMinimum) * 0.5;

			case EXTENT_INFINITE:
				return Vector3f(
					Math::PositiveInfinity,
					Math::PositiveInfinity,
					Math::PositiveInfinity);

			default: // shut up compiler
				assert( false && "Never reached" );
				return Vector3f::Zero();
			}
		}

        /** Tests whether the given point contained by this box.
        */
        bool contains(const Vector3f& v) const
        {
            if (isNull())
                return false;
            if (isInfinite())
                return true;

            return mMinimum.x() <= v.x() && v.x() <= mMaximum.x() &&
                   mMinimum.y() <= v.y() && v.y() <= mMaximum.y() &&
                   mMinimum.z() <= v.z() && v.z() <= mMaximum.z();
        }

        /** Tests whether another box contained by this box.
        */
        bool contains(const AlignedBox3& other) const
        {
            if (other.isNull() || this->isInfinite())
                return true;

            if (this->isNull() || other.isInfinite())
                return false;

            return this->mMinimum.x() <= other.mMinimum.x() &&
                   this->mMinimum.y() <= other.mMinimum.y() &&
                   this->mMinimum.z() <= other.mMinimum.z() &&
                   other.mMaximum.x() <= this->mMaximum.x() &&
                   other.mMaximum.y() <= this->mMaximum.y() &&
                   other.mMaximum.z() <= this->mMaximum.z();
        }

        /** Tests 2 boxes for equality.
        */
        bool operator== (const AlignedBox3& rhs) const
        {
            if (this->mExtent != rhs.mExtent)
                return false;

            if (!this->isFinite())
                return true;

            return this->mMinimum == rhs.mMinimum &&
                   this->mMaximum == rhs.mMaximum;
        }

        /** Tests 2 boxes for inequality.
        */
        bool operator!= (const AlignedBox3& rhs) const
        {
            return !(*this == rhs);
        }

		// special values
		static const AlignedBox3 BOX_NULL;
		static const AlignedBox3 BOX_INFINITE;

		// This method is kinda dangerous. It sets the extent to finite, without any guarantee
		// it's actually going to be so.
		Vector3f& operator[]( const size_t i )
		{
			assert( i < 2 );
			mExtent = EXTENT_FINITE;
			if(i == 0) return mMinimum;
			return mMaximum;
		}
        
		const Vector3f& operator[]( const size_t i ) const
		{
			assert( i < 2 );
			if(i == 0) return mMinimum;
			return mMaximum;
		}

	};

}
#endif
