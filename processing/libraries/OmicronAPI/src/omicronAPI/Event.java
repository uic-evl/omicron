package omicronAPI;

/**************************************************************************************************
 * THE OMICRON PROJECT
 * ----------------------------------------------------------
 * --------------------------------------- Copyright 2010-2012 Electronic
 * Visualization Laboratory, University of Illinois at Chicago Authors: Arthur
 * Nishimoto arthur.nishimoto@gmail.com
 * ------------------------------------------
 * ------------------------------------------------------- Copyright (c)
 * 2010-2011, Electronic Visualization Laboratory, University of Illinois at
 * Chicago All rights reserved. Redistribution and use in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
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
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/

import processing.core.PVector;

public class Event
{
	public int sourceID; // Device IDs (touch ids, mocap joints, etc.)
	public int serviceID; // Used for pointer to denote types (Mouse, touch)
	public int flags; // Used mostly for Wand/Controller to denote button ID
						// during down/up events
	public OmicronAPI.ServiceType serviceType = OmicronAPI.ServiceType.Generic; // Pointer, mocap, voice, etc.
	public OmicronAPI.Type eventType = OmicronAPI.Type.Null; // Event type i.e. click, move, etc.
	
	public int extraDataSize = 1024;
	
	public OmicronAPI.ExtraDataType extraDataType = OmicronAPI.ExtraDataType.ExtraDataNull;
	public int extraDataItems = 0;
	public int extraDataMask;
	public byte[] extraData = new byte[extraDataSize];
	
	int dataArraySize = 0;
	
	// Data array types
	public float[] dataArray;
	public PVector[] vectorArray;
	public String[] stringArray;

	public float[] position;
	public float[] orientation;
	public float timestamp;

	Event()
	{
	}

	public float getXPos()
	{
		if (position != null)
			return position[0];
		else
			return -1;
	}

	public float getYPos()
	{
		if (position != null)
			return position[1];
		else
			return -1;
	}

	public float getZPos()
	{
		if (position != null)
			return position[2];
		else
			return -1;
	}

	public OmicronAPI.ServiceType getServiceType()
	{
		return serviceType;
	}

	public int getSourceID()
	{
		return sourceID;
	}

	public OmicronAPI.Type getEventType()
	{
		return eventType;
	}

	public void setTimeStamp(float ts)
	{
		timestamp = ts;
	}

	public long getTimeStamp()
	{
		return (long) timestamp;
	}

	public float getFloatData(int index)
	{
		if (dataArraySize > index && dataArray != null )
			return (float) dataArray[index];
		else
			return -1;
	}

	public int getIntData(int index)
	{
		if (dataArraySize > index && dataArray != null )
			return (int) dataArray[index];
		else
			return -1;
	}

	public String getStringData(int index)
	{
		if (dataArraySize > index && stringArray != null )
			return stringArray[index];
		else
			return "";
	}

	public int getFlags()
	{
		return flags;
	}
}
