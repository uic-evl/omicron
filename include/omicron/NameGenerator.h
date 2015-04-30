////////////////////////////////////////////////////////////////////////////////
//! Utility class to generate a sequentially numbered series of names
//------------------------------------------------------------------------------
// Original code taken from OGRE
//  Copyright (c) 2000-2009 Torus Knot Software Ltd
//  For the latest info, see http://www.ogre3d.org/
#ifndef __NAME__GENERATOR__
#define __NAME__GENERATOR__

#include "Thread.h"

#include<string>

namespace omicron 
{
    class NameGenerator
    {
    protected:
        std::string mPrefix;
        unsigned long long int mNext;
        Lock mMutex;
    public:
        NameGenerator(const NameGenerator& rhs)
            : mPrefix(rhs.mPrefix), mNext(rhs.mNext) {}

        NameGenerator(const String& prefix) : mPrefix(prefix), mNext(1) {}

        /// Generate a new name
        String generate()
        {
            AutoLock al(mMutex);
            std::ostringstream s;
            s << mPrefix << mNext++;
            return s.str();
        }

        /// Reset the internal counter
        void reset()
        {
            AutoLock al(mMutex);
            mNext = 1ULL;
        }

        /// Manually set the internal counter (use caution)
        void setNext(unsigned long long int val)
        {
            AutoLock al(mMutex);
            mNext = val;
        }

        /// Get the internal counter
        unsigned long long int getNext() const
        {
            AutoLock al(*(const_cast<Lock*>(&mMutex)));
            return mNext;
        }
    };
};
#endif
