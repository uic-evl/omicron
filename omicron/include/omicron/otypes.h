/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2012, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __OMICRON_TYPES_H__
#define __OMICRON_TYPES_H__

#include "omicronConfig.h"

// Stdlib includes
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <time.h>

#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

// boost includes
#include <boost/foreach.hpp>
#include <boost/detail/atomic_count.hpp>

// Hack to rename intrusive_ptr to Ref.
#include "omicron/ref.hpp"
#define foreach BOOST_FOREACH

#define NOMINMAX

// Unordered map: use different implementations on linux & windows.
#ifdef __GNUC__
	#include <tr1/unordered_map>
#else
	#include <hash_map>
#endif

// Libconfig
#include "libconfig/libconfig.hh"

// make sure the min and max macros are undefined.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// eigenwrap includes
#include <omicron/math/Math.h>
#include <omicron/math/Ray.h>
#include <omicron/math/AlignedBox.h>
#include <omicron/math/Sphere.h>
#include <omicron/math/Plane.h>

namespace boost { template<class Ch, class Tr, class Alloc> class basic_format; };

namespace omicron
{
	// Basic typedefs
	typedef unsigned char byte;
	#ifndef OMICRON_OS_LINUX 
	typedef unsigned int uint;
	#endif
	typedef unsigned long long uint64;
	typedef unsigned long long int64;
	typedef std::string String;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! A key-value pair, usually stored in objects of the Dictionary class.
	template<typename K, typename T>
	class KeyValue: public std::pair<K, T>
	{
	public:
		KeyValue(const K& k, T v): std::pair<K, T>(k, v) {}
		KeyValue(std::pair<K, T> src): std::pair<K, T>(src.first, src.second) {}
		KeyValue(std::pair<const K, T> src): std::pair<K, T>(src.first, src.second) {}
		const K& getKey() { return this->first; }
		T getValue() { return this->second; }
		T operator->() { return this->second; }
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Container typedefs
	//! A Dictionary storing key-value pairs using a hashtable implementation.
	//! @remarks Dictionary is usually a lightweight wrapper around a standard library implementation
	#ifdef __GNUC__
		template<typename K, typename T> class Dictionary: public std::tr1::unordered_map<K, T> {
	#else
		template<typename K, typename T> class Dictionary: public stdext::hash_map<K, T> {
	#endif
		public:
			typedef KeyValue<K, T> Item;
			typedef std::pair<  typename Dictionary<K, T>::iterator,  typename Dictionary<K, T>::iterator> Range;
			typedef std::pair<  typename Dictionary<K, T>::const_iterator,  typename Dictionary<K, T>::const_iterator> ConstRange;
		};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Dictionary is usually a lightweight wrapper around a standard library vector implementation
	template<typename T> class Vector: public std::vector<T>
	{
	public:
		typedef std::pair<  typename Vector<T>::iterator,  typename Vector<T>::iterator> Range;
		typedef std::pair<  typename Vector<T>::const_iterator,  typename Vector<T>::const_iterator> ConstRange;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! List is usually a lightweight wrapper around a standard library list implementation
	template<typename T> class List: public std::list<T> 
	{
	public:
		typedef std::pair<  typename List<T>::iterator, typename List<T>::iterator> Range;
		typedef std::pair<  typename List<T>::const_iterator,  typename List<T>::const_iterator> ConstRange;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Queue is usually a lightweight wrapper around a standard library queue implementation
	template<typename T> class Queue: public std::queue<T>
	{
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Stores configuration settings from a section of a config file.
	//! @remarks for full reference see http://www.hyperrealm.com/libconfig/libconfig_manual.html#The-C_002b_002b-API
	typedef libconfig::Setting Setting;


	///////////////////////////////////////////////////////////////////////////////////////////////
	//! enumeration for the axes
	enum Axis
	{
		AxisX,
		AxisY,
		AxisZ
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Enumeration for orientation.
	enum Orientation 
	{
		Horizontal = 0, 
		Vertical = 1
	};


	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Implements a base class for reference-counted types. o be used in conjunction with the 
	//! Ref<> type.
	class OMICRON_API ReferenceType
	{
	public:
		static void printObjCounts();
	public:
		ReferenceType();
		virtual ~ReferenceType();


		void ref()
			{
				//oassert(s->ref_count >= 0);
				//oassert(s != 0);
				++myRefCount;
			}

		void unref()
			{
				//oassert(s->ref_count > 0);
				//oassert(s != 0);
				if (--myRefCount == 0) delete this;
			}

		long refCount() 
		{  
			return myRefCount;
		}

	private:
		mutable boost::detail::atomic_count myRefCount;
	protected:
		static List<ReferenceType*> mysObjList;
	};

	inline void intrusive_ptr_add_ref(ReferenceType* p) { p->ref(); }
	inline void intrusive_ptr_release(ReferenceType* p) { p->unref(); }

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//! Utility class to generate a sequentially numbered series of names
	//-------------------------------------------------------------------------------------------------
	// Original code taken from OGRE
	//  Copyright (c) 2000-2009 Torus Knot Software Ltd
	//  For the latest info, see http://www.ogre3d.org/
	class NameGenerator
	{
	protected:
		String mPrefix;
		unsigned long long int mNext;
		//OGRE_AUTO_MUTEX
	public:
		NameGenerator(const NameGenerator& rhs)
			: mPrefix(rhs.mPrefix), mNext(rhs.mNext) {}
		
		NameGenerator(const String& prefix) : mPrefix(prefix), mNext(1) {}

		/// Generate a new name
		String generate()
		{
			//OGRE_LOCK_AUTO_MUTEX
			std::ostringstream s;
			s << mPrefix << mNext++;
			return s.str();
		}

		/// Reset the internal counter
		void reset()
		{
			//OGRE_LOCK_AUTO_MUTEX
			mNext = 1ULL;
		}

		/// Manually set the internal counter (use caution)
		void setNext(unsigned long long int val)
		{
			//OGRE_LOCK_AUTO_MUTEX
			mNext = val;
		}

		/// Get the internal counter
		unsigned long long int getNext() const
		{
			// lock even on get because 64-bit may not be atomic read
			//OGRE_LOCK_AUTO_MUTEX
			return mNext;
		}
	};
}; // namespace omicron
#endif
