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
#include <connector/omicronConnectorClient.h>

using namespace omicronConnector;

///////////////////////////////////////////////////////////////////////////////////////////////////
class ConnectorListener: public IOmicronConnectorClientListener
{
public:
virtual void onEvent(const EventData& e)
	{
		if (e.type == EventData::Trace) 
		{
			fprintf(stderr, "TRACE source %d\n", e.sourceId);
		}
		else if (e.type == EventData::Untrace) 
		{
			fprintf(stderr, "UNTRACE source %d\n", e.sourceId);
		}
		else if (e.type == EventData::Update) 
		{
			if (e.serviceType == EventData::ServiceTypeMocap)  // Mocap (head I guess
				fprintf(stderr, "MOCAP source %d type %d service %d type %d\n",
					e.sourceId, e.type, e.serviceId);
			if (e.serviceType == EventData::ServiceTypeWand)  // Wand
				fprintf(stderr, "Wand source %d type %d service %d type %d\n",
					e.sourceId, e.type, e.serviceId);
			fprintf(stderr, "      pos(%6.3f  %6.3f  %6.3f)\n", e.posx, e.posy, e.posz);
			fprintf(stderr, "      rot(%6.3f  %6.3f  %6.3f  %6.3f)\n", e.orx, e.ory, e.orz, e.orw);
			fprintf(stderr, "      flag: %d\n", e.flags);
			fprintf(stderr, "      extra type: %d\n", e.extraDataType);
			fprintf(stderr, "      extra items: %d\n", e.extraDataItems);
			fprintf(stderr, "      extra mask: %d\n", e.extraDataMask);
			if (e.extraDataItems) {
				if (e.extraDataType == EventData::ExtraDataFloatArray) 
				{ //array of floats
					float *ptr = (float*)(e.extraData);
					for (int k=0;k<e.extraDataItems;k++) 
					{
						fprintf(stderr, "      val %2d: [%6.3f]\n", k, ptr[k]);
					}
				}
				else if (e.extraDataType == EventData::ExtraDataVector3Array) 
				{ //array of floats
					float val[3];
					//float *ptr = (float*)(e.extraData);
					for (int k = 0; k < e.extraDataItems; k++) 
					{
						e.getExtraDataVector3(k, val);
						fprintf(stderr, "      val %2d: [%6.3f, %6.3f, %6.3f]\n", k, val[0], val[1], val[2]);
					}
				}
			}
			fprintf(stderr, "-----------\n");
		}
		else if (e.type == EventData::Down) {
			//fprintf(stderr, "EVENT down: %d\n", mButton(e.flags));
		}
		else if (e.type == EventData::Up) 
		{
			fprintf(stderr, "EVENT up: %d\n", e.sourceId, e.type);
			fprintf(stderr, "      flag: %d\n", e.flags);
			fprintf(stderr, "      extra: %d\n", e.extraDataType);
			fprintf(stderr, "      items: %d\n", e.extraDataItems);
			fprintf(stderr, "      mask: %d\n", e.extraDataMask);
			fprintf(stderr, "-----------\n");
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	ConnectorListener listener;
	OmicronConnectorClient client(&listener);
	client.connect("137.110.119.244", 27000);
	//client.connect("127.0.0.1", 27000);
	while(true)
	{
		client.poll(); 
	}
}


