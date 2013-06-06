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
 *-------------------------------------------------------------------------------------------------
 * Original code copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *************************************************************************************************/
#ifndef __HTTP_REQUEST__
#define __HTTP_REQUEST__

#include "omicronConfig.h"
#include "StringUtils.h"
#include <asio.hpp>
#include <boost/bind.hpp>

using asio::ip::tcp;


namespace omicron {
	///////////////////////////////////////////////////////////////////////////////////////////////
	class IHttpRequestListener
	{
	public:
		virtual void onDataReceived(asio::streambuf& myResponse) = 0;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class HttpRequest
	{
	public:
		///////////////////////////////////////////////////////////////////////////////////////////
		HttpRequest(asio::io_service& io_service,
			const String& server, IHttpRequestListener* listener)
			: myResolver(io_service),
				myServer(server),
				mySocket(io_service),
				myListener(listener)
		{
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		void sendRequest(const String& request)
		{
			// Form the request. We specify the "Connection: close" header so that the
			// server will close the socket after transmitting the response. This will
			// allow us to treat all data up until the EOF as the content.
			std::ostream myRequeststream(&myRequest);
			myRequeststream << "GET " << request << " HTTP/1.0\r\n";
			myRequeststream << "Host: " << myServer << "\r\n";
			myRequeststream << "Accept: */*\r\n";
			myRequeststream << "Connection: close\r\n\r\n";

			// Start an asynchronous resolve to translate the server and service names
			// into a list of endpoints.
			tcp::resolver::query query(myServer, "http");
			myResolver.async_resolve(query,
				boost::bind(&HttpRequest::handle_resolve, this,
					asio::placeholders::error,
					asio::placeholders::iterator));
		}

	private:
		///////////////////////////////////////////////////////////////////////////////////////////
		void handle_resolve(const asio::error_code& err, tcp::resolver::iterator endpoint_iterator)
		{
			if (!err)
			{
				// Attempt a connection to the first endpoint in the list. Each endpoint
				// will be tried until we successfully establish a connection.
				tcp::endpoint endpoint = *endpoint_iterator;
				mySocket.async_connect(endpoint,
					bind(&HttpRequest::handle_connect, this,
					asio::placeholders::error, ++endpoint_iterator));
			}
			else
			{
				ofwarn("HttpRequest error: %1%", %err.message());
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		void handle_connect(const asio::error_code& err, tcp::resolver::iterator endpoint_iterator)
		{
			if (!err)
			{
				// The connection was successful. Send the request.
				asio::async_write(mySocket, myRequest,
					bind(&HttpRequest::handle_write_request, this,
					asio::placeholders::error));
			}
			else if (endpoint_iterator != tcp::resolver::iterator())
			{
				// The connection failed. Try the next endpoint in the list.
				mySocket.close();
				tcp::endpoint endpoint = *endpoint_iterator;
				mySocket.async_connect(endpoint,
					bind(&HttpRequest::handle_connect, this,
					asio::placeholders::error, ++endpoint_iterator));
			}
			else
			{
				ofwarn("HttpRequest error: %1%", %err.message());
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		void handle_write_request(const asio::error_code& err)
		{
			if (!err)
			{
				// Read the response status line.
				asio::async_read_until(mySocket, myResponse, "\r\n",
					bind(&HttpRequest::handle_read_status_line, this,
					asio::placeholders::error));
			}
			else
			{
				ofwarn("HttpRequest error: %1%", %err.message());
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		void handle_read_status_line(const asio::error_code& err)
		{
			if (!err)
			{
				// Check that response is OK.
				std::istream myResponsestream(&myResponse);
				String http_version;
				myResponsestream >> http_version;
				unsigned int status_code;
				myResponsestream >> status_code;
				String status_message;
				std::getline(myResponsestream, status_message);
				if (!myResponsestream || http_version.substr(0, 5) != "HTTP/")
				{
					owarn("HttpRequest: Invalid response");
					return;
				}
				if (status_code != 200)
				{
					ofwarn("HttpRequest: Response returned with status code ", %status_code);
					return;
				}

				// Read the response headers, which are terminated by a blank line.
				asio::async_read_until(mySocket, myResponse, "\r\n\r\n",
					bind(&HttpRequest::handle_read_headers, this,
					asio::placeholders::error));
			}
			else
			{
				ofwarn("HttpRequest error: %1%", %err);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		void handle_read_headers(const asio::error_code& err)
		{
			if (!err)
			{
				// Process the response headers.
				std::istream myResponsestream(&myResponse);
				String header;
				while (std::getline(myResponsestream, header) && header != "\r")
				//std::cout << header << "\n";
				//std::cout << "\n";

				// Write whatever content we already have to output.
				//if (myResponse.size() > 0)
				//std::cout << &myResponse;

				// Start reading remaining data until EOF.
				asio::async_read(mySocket, myResponse,
					asio::transfer_at_least(1),
					bind(&HttpRequest::handle_read_content, this,
					asio::placeholders::error));
			}
			else
			{
				ofwarn("HttpRequest eror: %1%", %err);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		void handle_read_content(const asio::error_code& err)
		{
			if (!err)
			{
				// Write all of the data that has been read so far.
				//std::cout << &myResponse;

				// Continue reading remaining data until EOF.
				asio::async_read(mySocket, myResponse,
					asio::transfer_at_least(1),
					bind(&HttpRequest::handle_read_content, this,
					asio::placeholders::error));
			}
			else if (err != asio::error::eof)
			{
				ofwarn("HttpRequest eror: %1%", %err);
			}
			else if(err == asio::error::eof && myListener != NULL)
			{
				myListener->onDataReceived(myRequest);
			}
		}

		private:
			String myServer;
			tcp::resolver myResolver;
			tcp::socket mySocket;
			asio::streambuf myRequest;
			asio::streambuf myResponse;
			IHttpRequestListener* myListener;
	};
};