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
 *-------------------------------------------------------------------------------------------------
 * Contains classes to handle server and client TCP communication
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
	//! Contains information about a single connection.
	struct ConnectionInfo
	{
		ConnectionInfo(asio::io_service& io, int id = 0):
			ioService(io), id(id) {}

		asio::io_service& ioService;
		int id;
	};

	//! Represents a connection error.
	typedef asio::error_code ConnectionError;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! A TCP Connection. Can be used to establish a connection to a TCP server 
	//! (using the open method). It is also used by the TcpSerer class to represent each client 
	//! connection. User code can derive this class and reimplement the handleConnected, 
	//! handleData, handleClose and handleError methods.
	class OMICRON_API TcpConnection: public ReferenceType
	{
	friend class TcpServer;
	public:
		enum ConnectionState { 
			//! The connection is waiting for the other side to respond.
			ConnectionListening, 
			//! The connection is open. Read / write methods can be used.
			ConnectionOpen, 
			//! The connection is closed.
			ConnectionClosed };

	public:
		TcpConnection(const ConnectionInfo& ci);
		virtual ~TcpConnection() {}

		//! Connection properties
		//@{
		//! Gets the internal ASIO socket object.
		tcp::socket& getSocket() { return mySocket; }
		//! Gets the connection state.
		ConnectionState getState() { return myState; }
		//! Gets the connection info object.
		const ConnectionInfo& getConnectionInfo() { return myConnectionInfo; }
		//@}

		//! Connection management
		//@{
		//! Polls the connection. Calls handleData when new data is available. 
		//! Calls handleClosed when the connection has been closed from the other end.
		bool poll();
		//! Forces a connection close. To gracefully close connections, one side should call waitClose 
		//! (usually the client), while the other calls close. waitClose will return once the connection
		//! has been closed correctly. It is the user's responsibility to signal each end when a connection
		//! should be closed, and call close and waitClose appropriately.
		void close();
		//! Waits for the other end to close the connection.
		void waitClose();
		//! Opens a connection to a server.
		void open(const String& host, int port);
		//@}

		//! Data IO
		//! Note: all the write methods are blocking: they return only when the data has been fully written to 
		//! the internal buffers. Therefore, buffers passed to write methods can be safely modified to after a
		//! write call.
		//@{
		//! Writes a string to the connection stream. The string will NOT be NULL terminated.
		void write(const String& data);
		//! Writes a buffer to the connection stream.
		void write(void* data, size_t size);
		//! Synchronously read byte data until the specified delimiter is found or the buffer fills up.
		size_t readUntil(void* buffer, size_t size, char delimiter = '\0');
		//! Synchronously read thre specified number of bytes from the stream.
		size_t read(void* buffer, size_t size);
		//! Returns the number of bytes available to be read.
		size_t availableBytes() { return mySocket.available(); }
		//@}
		
		//! Connection events
		//@{
		virtual void handleConnected();
		virtual void handleClosed();
		virtual void handleError(const ConnectionError& err);
		virtual void handleData();
		//@}

		//! @internal
		void handle_connect(const asio::error_code& error);
	private:
		TcpConnection(const TcpConnection&);
		
	protected:
		String myHost;
		int myPort;
		ConnectionInfo myConnectionInfo;
		ConnectionState myState;
		tcp::socket mySocket;
		asio::streambuf myInputBuffer;

	protected:
		void doHandleConnected();
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	//! Implements a Tcp server. The server listens to client connections and handles 
	//! communication with each client. To implement custom servers, users derive the TcpServer 
	//! class and reimplement the createConnection method, to create custom TcpConnection instances
	//! That contains the user-defined communication logic (see TcpConnection).
	//! The TcpServer is implemented as an omicron service, so it will run automatically in the 
	//! background when users register it with ServiceManager. TcpServer can also be used without
	//! ServiceManager, by calling the intialize(), start(), stop() and poll() methods directly.
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
		//! Called when a new client connected. Creates a TcpConnection instance to handle communication
		//! User code should reimplement this method.
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