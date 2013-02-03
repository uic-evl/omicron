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
#include "omicron/Tcp.h"
#include "omicron/StringUtils.h"

#include <boost/bind.hpp>

using namespace omicron;

///////////////////////////////////////////////////////////////////////////////////////////////////
TcpServer::TcpServer():
	myRunning(false),
	myInitialized(false),
	myConnectionCounter(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpServer::initialize()
{
	if(!myInitialized)
	{
		myInitialized = true;
		myAcceptor = new tcp::acceptor(myIOService, tcp::endpoint(tcp::v4(), myPort));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TcpServer::~TcpServer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpServer::start()
{
	myRunning = true;
	accept();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpServer::stop()
{
	myRunning = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpServer::poll()
{
	if(myRunning)
	{
		Queue<TcpConnection*> closedConnections;

		myIOService.poll();
		foreach(TcpConnection* c, myClients)
		{
			if(!c->poll()) closedConnections.push(c);
		}

		while(closedConnections.size() != 0)
		{
			myClients.remove(closedConnections.front());
			closedConnections.pop();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpServer::accept()
{
	TcpConnection* conn = doCreateConnection();
	myAcceptor->async_accept(conn->getSocket(), boost::bind(&TcpServer::handleAccept, this, conn, asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpServer::handleAccept(TcpConnection* newConnection, const asio::error_code& error)
{
	if(!error)
	{
		newConnection->doHandleConnected();
		accept();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TcpConnection* TcpServer::createConnection(const ConnectionInfo& ci)
{
	return new TcpConnection(ci);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TcpConnection* TcpServer::doCreateConnection()
{
	ConnectionInfo ci(myIOService, myConnectionCounter++);
	TcpConnection* conn = createConnection(ci);
	myClients.push_back(conn);
	return conn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TcpConnection* TcpServer::getConnection(int id)
{
	foreach(TcpConnection* c, myClients)
	{
		if(c->getConnectionInfo().id == id) return c;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TcpConnection::TcpConnection(const ConnectionInfo& ci):
	myConnectionInfo(ci),
	mySocket(ci.ioService),
	myState(ConnectionListening)
{}
	
///////////////////////////////////////////////////////////////////////////////////////////////////
bool TcpConnection::poll()
{
	// If connection is in listening state (waiting for other end to connect) do nothing
	if(myState == ConnectionListening) return true;

	if(myState == ConnectionOpen)
	{
		// If connection is open but the inner socket is not, signal that the connection has 
		// been closed.
		if(!mySocket.is_open())
		{
			myState = ConnectionClosed;
			handleClosed();
			return false;
		}
		else if(mySocket.available() != 0)
		{
			while(mySocket.available() != 0) handleData();
		}
	} 
	else if(myState == ConnectionClosed)
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::close()
{
	if(myState == ConnectionOpen)
	{
		myState = ConnectionClosed;
		handleClosed();
		mySocket.close();
	}
	else 
	{
		ofwarn("TcpConnection::close (id=%1%): connection was not open", %myConnectionInfo.id);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::write(const String& data)
{
	if(mySocket.is_open())
	{
		asio::error_code error;
		asio::write(mySocket, asio::buffer(data), error);
		if(error)
		{
			mySocket.close();
			handleError(error);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::write(void* data, size_t size)
{
	if(mySocket.is_open())
	{
		asio::error_code error;
		asio::write(mySocket, asio::buffer(data, size), error);
		if(error)
		{
			mySocket.close();
			handleError(error);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t TcpConnection::readUntil(char* buffer, size_t size, char delimiter)
{
	if(mySocket.is_open())
	{
		asio::error_code error;
		size_t bufsize = asio::read_until(mySocket, myInputBuffer, delimiter, error);
		if(!error)
		{
			if(bufsize > size - 1) bufsize = size;
			myInputBuffer.sgetn((char*)buffer, bufsize);
			buffer[bufsize] = '\0';
			return bufsize;
		}
		else
		{
			mySocket.close();
			handleError(error);
		}
	}
		return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t TcpConnection::read(byte* buffer, size_t size)
{
	if(mySocket.is_open())
	{
		asio::error_code error;
		size_t bufsize = asio::read(mySocket, myInputBuffer, asio::transfer_exactly(size), error);
		if(!error)
		{
			myInputBuffer.sgetn((char*)buffer, bufsize);
			return size;
		}
		else
		{
			mySocket.close();
			handleError(error);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::doHandleConnected()
{
	omsg("TcpConnection::doHandleConnected");
	myState = ConnectionOpen;
	handleConnected();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::handleConnected()
{
	ofmsg("TcpConnection::handleConnected (id=%1%)", %myConnectionInfo.id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::handleClosed()
{
	ofmsg("TcpConnection::handleClosed (id=%1%)", %myConnectionInfo.id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::handleError(const ConnectionError& err)
{
	ofwarn("TcpConnection:handleError (id=%1%): %2%", %myConnectionInfo.id %err.message());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpConnection::handleData()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpClientConnection::open(const String& host, int port)
{
	myHost = host;
	myPort = port;
	
	asio::ip::tcp::resolver resolver(myConnectionInfo.ioService);
    asio::ip::tcp::resolver::query query(host, ostr("%1%", %port));
    asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

	mySocket.async_connect(*iterator, boost::bind(&TcpClientConnection::handle_connect, this, asio::placeholders::error));
	//asio::connect(mySocket, iterator, boost::bind(&TcpClientConnection::handle_connect, this, asio::placeholders::error));
	//asio::async_connect(mySocket, iterator, boost::bind(&TcpClientConnection::handle_connect, this, asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TcpClientConnection::handle_connect(const asio::error_code& error)
{
	if(!error)
	{
		doHandleConnected();
	}
	else
	{
		handleError(error);
	}
}
