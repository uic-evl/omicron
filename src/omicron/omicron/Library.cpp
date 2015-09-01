/******************************************************************************
* THE OMICRON SDK
*-----------------------------------------------------------------------------
* Copyright 2010-2015		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2015, Electronic Visualization Laboratory,
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
*	A cross-platform object for accessyng dynamically-linked libraries
******************************************************************************/
#include "omicron/Library.h"

#include "omicron/StringUtils.h"

#ifdef OMICRON_OS_WIN
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

using namespace omicron;

namespace omicron {
///////////////////////////////////////////////////////////////////////////////
class LibraryImpl
{
public:
    LibraryImpl() : lp(0) {}
#ifdef OMICRON_OS_WIN
    HMODULE lp;
#else
    void* lp;
#endif
};
};

///////////////////////////////////////////////////////////////////////////////
Library::Library():
    myImpl(new LibraryImpl)
{}

///////////////////////////////////////////////////////////////////////////////
Library::~Library()
{
    delete myImpl;
    myImpl = 0;
}

///////////////////////////////////////////////////////////////////////////////
bool Library::open(const String& fileName)
{
    if(myImpl->lp)
    {
        owarn("[Library::open] library already open");
        return false;
    }

    if(fileName.empty())
    {
#ifdef OMICRON_OS_WIN
        myImpl->lp = GetModuleHandle(0);
        oassert(myImpl->lp);
#else
        myImpl->lp = RTLD_DEFAULT;
#endif
    }
    else
    {
#ifdef OMICRON_OS_WIN
        myImpl->lp = LoadLibrary(fileName.c_str());
#elif defined( RTLD_LOCAL )
        myImpl->lp = dlopen(fileName.c_str(), RTLD_LAZY | RTLD_LOCAL);
#else
        myImpl->lp = dlopen(fileName.c_str(), RTLD_LAZY);
#endif
        if(!myImpl->lp)
        {
            ofwarn("[Library::open] can't open library %1%", %fileName);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
void Library::close()
{
    if(!myImpl->lp)
        return;

#ifdef OMICRON_OS_WIN
    if(myImpl->lp != GetModuleHandle(0))
        FreeLibrary(myImpl->lp);
#else
    if(myImpl->lp != RTLD_DEFAULT)
        dlclose(myImpl->lp);
#endif

    myImpl->lp = 0;
}

///////////////////////////////////////////////////////////////////////////////
void* Library::getFunctionPointer(const std::string& name)
{
#ifdef OMICRON_OS_WIN
    return (void*)GetProcAddress(myImpl->lp, name.c_str());
#else
    return dlsym(myImpl->lp, name.c_str());
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool Library::isOpen() const
{
    return myImpl->lp != 0;
}

