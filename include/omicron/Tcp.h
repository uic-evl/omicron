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
#ifndef __TCP_H__
#define __TCP_H__

#include "omicron/osystem.h"
#include "Service.h"

#ifdef OMICRON_OS_WIN
	#define NOMINMAX
	#define BOOST_DATE_TIME_NO_LIB
	#define BOOST_REGEX_NO_LIB
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
#include <asio.hpp>


using asio::ip::tcp;


namespace omicron {

	///////////////////////////////////////////////////////////////////////////////////////////////
	struct ConnectionInfo
	{
		ConnectionInfo(asio::io_service& io, int id = 0):
			ioService(io), id(id) {}

		asio::io_service& ioService;
		int id;
	};

	typedef asio::error_code ConnectionError;

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API TcpConnection: public ReferenceType
	{
	friend class TcpServer;
	public:
		enum ConnectionState { ConnectionListening, ConnectionOpen, ConnectionClosed };

	public:
		TcpConnection(const ConnectionInfo& ci);

		//! Connection properties
		//@{
		tcp::socket& getSocket() { return mySocket; }
		ConnectionState getState() { return myState; }
		const ConnectionInfo& getConnectionInfo() { return myConnectionInfo; }
		//@}

		//! Connection management
		//@{
		bool poll();
		void close();
		//@}

		//! Data IO
		//@{
		void write(const String& data);
		void write(void* data, size_t size);
		//! Synchronously read byte data until the specified delimiter is found or the buffer fills up.
		size_t readUntil(char* buffer, size_t size, char delimiter = '\0');
		//! Synchronously read thre specified number of bytes from the stream.
		size_t read(byte* buffer, size_t size);
		size_t availableBytes() { return mySocket.available(); }
		//@}

		//! Connection events
		//@{
		virtual void handleConnected();
		virtual void handleClosed();
		virtual void handleError(const ConnectionError& err);
		virtual void handleData();
		//@}

	private:
		TcpConnection(const TcpConnection&);
		
	protected:
		ConnectionInfo myConnectionInfo;
		ConnectionState myState;
		tcp::socket mySocket;
		asio::streambuf myInputBuffer;

	protected:
		void doHandleConnected();
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API TcpClientConnection: public TcpConnection
	{
	public:
		TcpClientConnection(const ConnectionInfo& ci): TcpConnection(ci) {}
		void open(const String& host, int port);

	private:
		TcpClientConnection(const TcpClientConnection&);
	private:
		String myHost;
		int myPort;

	public:
		void handle_connect(const asio::error_code& error);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMICRON_API TcpServer: public Service
	{
	public:
		TcpServer();
		~TcpServer();

		void setPort(int value) { myPort = value; }
		int getPort() { return myPort; }

		virtual void initialize();
		virtual void start();
		virtual void stop();
		virtual void poll();
		TcpConnection* getConnection(int id);

	protected:
		virtual TcpConnection* createConnection(const ConnectionInfo& ci);
		virtual void accept();
		virtual void handleAccept(TcpConnection* newConnection, const asio::error_code& error);

	private:
		TcpConnection* doCreateConnection();

	private:
		int myConnectionCounter;
		int myPort;
		bool myInitialized;
		bool myRunning;
		tcp::acceptor* myAcceptor;
		asio::io_service myIOService;
		List< Ref<TcpConnection> > myClients;
	};
}; // namespace omicron

#endif