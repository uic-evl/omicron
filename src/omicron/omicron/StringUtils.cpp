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
 *********************************************************************************************************************/
#include "omicron/StringUtils.h"

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const String StringUtils::BLANK;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StringUtils::trim(String& str, bool left, bool right)
{
    /*
    size_t lspaces, rspaces, len = length(), i;

    lspaces = rspaces = 0;

    if( left )
    {
        // Find spaces / tabs on the left
        for( i = 0;
            i < len && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
            ++lspaces, ++i );
    }
        
    if( right && lspaces < len )
    {
        // Find spaces / tabs on the right
        for( i = len - 1;
            i >= 0 && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
            rspaces++, i-- );
    }

    *this = substr(lspaces, len-lspaces-rspaces);
    */
    static const String delims = " \t\r";
    if(right)
        str.erase(str.find_last_not_of(delims)+1); // trim right
    if(left)
        str.erase(0, str.find_first_not_of(delims)); // trim left
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector<String> StringUtils::split( const String& str, const String& delims, unsigned int maxSplits)
{
    Vector<String> ret;
    // Pre-allocate some space for performance
    ret.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

    unsigned int numSplits = 0;

    // Use STL methods 
    size_t start, pos;
    start = 0;
    do 
    {
        pos = str.find_first_of(delims, start);
        if (pos == start)
        {
            // Do nothing
            start = pos + 1;
        }
        else if (pos == String::npos || (maxSplits && numSplits == maxSplits))
        {
            // Copy the rest of the string
            ret.push_back( str.substr(start) );
            break;
        }
        else
        {
            // Copy up to delimiter
            ret.push_back( str.substr(start, pos - start) );
            start = pos + 1;
        }
        // parse up to next real data
        start = str.find_first_not_of(delims, start);
        ++numSplits;

    } while (pos != String::npos);



    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector<String> StringUtils::tokenise( const String& str, const String& singleDelims, const String& doubleDelims, unsigned int maxSplits)
{
    Vector<String> ret;
    // Pre-allocate some space for performance
    ret.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

    unsigned int numSplits = 0;
	String delims = singleDelims + doubleDelims;

	// Use STL methods 
    size_t start, pos;
	char curDoubleDelim = 0;
    start = 0;
    do 
    {
		if (curDoubleDelim != 0)
		{
			pos = str.find(curDoubleDelim, start);
		}
		else
		{
			pos = str.find_first_of(delims, start);
		}

        if (pos == start)
        {
			char curDelim = str.at(pos);
			if (doubleDelims.find_first_of(curDelim) != String::npos)
			{
				curDoubleDelim = curDelim;
			}
            // Do nothing
            start = pos + 1;
        }
        else if (pos == String::npos || (maxSplits && numSplits == maxSplits))
        {
			if (curDoubleDelim != 0)
			{
				//Missing closer. Warn or throw exception?
			}
            // Copy the rest of the string
            ret.push_back( str.substr(start) );
            break;
        }
        else
        {
			if (curDoubleDelim != 0)
			{
				curDoubleDelim = 0;
			}

			// Copy up to delimiter
			ret.push_back( str.substr(start, pos - start) );
			start = pos + 1;
        }
		if (curDoubleDelim == 0)
		{
			// parse up to next real data
			start = str.find_first_not_of(singleDelims, start);
		}
            
        ++numSplits;

    } while (pos != String::npos);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StringUtils::toLowerCase(String& str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
		tolower);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StringUtils::toUpperCase(String& str) 
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
		toupper);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool StringUtils::startsWith(const String& str, const String& pattern, bool lowerCase)
{
    size_t thisLen = str.length();
    size_t patternLen = pattern.length();
    if (thisLen < patternLen || patternLen == 0)
        return false;

    String startOfThis = str.substr(0, patternLen);
    if (lowerCase)
        StringUtils::toLowerCase(startOfThis);

    return (startOfThis == pattern);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool StringUtils::endsWith(const String& str, const String& pattern, bool lowerCase)
{
    size_t thisLen = str.length();
    size_t patternLen = pattern.length();
    if (thisLen < patternLen || patternLen == 0)
        return false;

    String endOfThis = str.substr(thisLen - patternLen, patternLen);
    if (lowerCase)
        StringUtils::toLowerCase(endOfThis);

    return (endOfThis == pattern);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String StringUtils::standardisePath(const String& init)
{
    String path = init;

    std::replace( path.begin(), path.end(), '\\', '/' );
    if( path[path.length() - 1] != '/' )
        path += '/';

    return path;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StringUtils::splitFilename(const String& qualifiedName, 
    String& outBasename, String& outPath)
{
    String path = qualifiedName;
    // Replace \ with / first
    std::replace( path.begin(), path.end(), '\\', '/' );
    // split based on final /
    size_t i = path.find_last_of('/');

    if (i == String::npos)
    {
        outPath.clear();
		outBasename = qualifiedName;
    }
    else
    {
        outBasename = path.substr(i+1, path.size() - i - 1);
        outPath = path.substr(0, i+1);
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StringUtils::splitBaseFilename(const String& fullName, 
	String& outBasename, String& outExtention)
{
	size_t i = fullName.find_last_of(".");
	if (i == String::npos)
	{
		outExtention.clear();
		outBasename = fullName;
	}
	else
	{
		outExtention = fullName.substr(i+1);
		outBasename = fullName.substr(0, i);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StringUtils::splitFullFilename(	const String& qualifiedName, 
	String& outBasename, String& outExtention, String& outPath )
{
	String fullName;
	splitFilename( qualifiedName, fullName, outPath );
	splitBaseFilename( fullName, outBasename, outExtention );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool StringUtils::match(const String& str, const String& pattern, bool caseSensitive)
{
    String tmpStr = str;
	String tmpPattern = pattern;
    if (!caseSensitive)
    {
        StringUtils::toLowerCase(tmpStr);
        StringUtils::toLowerCase(tmpPattern);
    }

    String::const_iterator strIt = tmpStr.begin();
    String::const_iterator patIt = tmpPattern.begin();
	String::const_iterator lastWildCardIt = tmpPattern.end();
    while (strIt != tmpStr.end() && patIt != tmpPattern.end())
    {
        if (*patIt == '*')
        {
			lastWildCardIt = patIt;
            // Skip over looking for next character
            ++patIt;
            if (patIt == tmpPattern.end())
			{
				// Skip right to the end since * matches the entire rest of the string
				strIt = tmpStr.end();
			}
			else
            {
				// scan until we find next pattern character
                while(strIt != tmpStr.end() && *strIt != *patIt)
                    ++strIt;
            }
        }
        else
        {
            if (*patIt != *strIt)
            {
				if (lastWildCardIt != tmpPattern.end())
				{
					// The last wildcard can match this incorrect sequence
					// rewind pattern to wildcard and keep searching
					patIt = lastWildCardIt;
					lastWildCardIt = tmpPattern.end();
				}
				else
				{
					// no wildwards left
					return false;
				}
            }
            else
            {
                ++patIt;
                ++strIt;
            }
        }

    }
	// If we reached the end of both the pattern and the string, we succeeded
	if (patIt == tmpPattern.end() && strIt == tmpStr.end())
	{
        return true;
	}
	else
	{
		return false;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const String StringUtils::replaceAll(const String& source, const String& replaceWhat, const String& replaceWithWhat)
{
	String result = source;
    String::size_type pos = 0;
	while(1)
	{
		pos = result.find(replaceWhat,pos);
		if (pos == String::npos) break;
		result.replace(pos,replaceWhat.size(),replaceWithWhat);
        pos += replaceWithWhat.size();
	}
	return result;
}

