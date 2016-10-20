
#include <iostream>
#include <cassert>

#include "replayBufferHandler.h"


#define BUF_INITIAL_SIZE 100

/* private methods */
void BufferHandler::allocate( buffer_t& buf, size_t size )
{
	if ( buf.size < size )
	{
		if (buf.data != NULL)
			delete[] buf.data;
		buf.data = new uint8_t[size];
		assert( buf.data != NULL );
		buf.size = size;
	}
}

void BufferHandler::initBuffer( buffer_t& buf )
{
	buf.size = 0;
	buf.data = NULL;
	allocate( buf, BUF_INITIAL_SIZE );
}

/* public memebers */

BufferHandler::BufferHandler(void)
{
	initBuffer( send_buf );
	initBuffer( recv_buf );
	initBuffer( displs );
	initBuffer( counts );
	initBuffer( icounts );
	initBuffer( idispls );
	initBuffer( requests );
	initBuffer( request_ids );
}

BufferHandler::BufferHandler(const BufferHandler& handler):
		send_buf(handler.send_buf),
		recv_buf(handler.recv_buf),
		displs(handler.displs),
		counts(handler.counts),
		idispls(handler.idispls),
		icounts(handler.icounts),
		requests(handler.requests),
		request_ids(handler.request_ids)
{
}

BufferHandler& BufferHandler::operator=(const BufferHandler& handler)
{
	if (this == &handler) return *this;
	delete[] send_buf.data;
	delete[] recv_buf.data;
	delete[] displs.data;
	delete[] counts.data;
	delete[] idispls.data;
	delete[] icounts.data;
	delete[] requests.data;
	delete[] request_ids.data;
	
	send_buf = handler.send_buf;
	
	recv_buf = handler.recv_buf;
	displs = handler.displs;
	counts = handler.counts;
	icounts = handler.icounts;
	idispls = handler.idispls;
	requests = handler.requests;
	request_ids = handler.request_ids;
	
	return *this;
}

BufferHandler::~BufferHandler(void)
{
	delete[] send_buf.data;
	delete[] recv_buf.data;
	delete[] displs.data;
	delete[] counts.data;
	delete[] idispls.data;
	delete[] icounts.data;
	delete[] requests.data;
	delete[] request_ids.data;
}

void* BufferHandler::getSendBuf( size_t size )
{
	this->allocate( send_buf, size );
	return send_buf.data;
}

void* BufferHandler::getRecvBuf( size_t size )
{
	this->allocate( recv_buf, size );
	return recv_buf.data;
}

void* BufferHandler::getCountsBuf( size_t size )
{
	this->allocate( counts, size );
	return counts.data;
}

void* BufferHandler::getDisplsBuf( size_t size )
{
	this->allocate( displs, size );
	return displs.data;
}

void* BufferHandler::getIcountsBuf( size_t size )
{
	this->allocate( icounts, size );
	return icounts.data;
}

void* BufferHandler::getIdisplsBuf( size_t size )
{
	this->allocate( idispls, size );
	return idispls.data;
}

void* BufferHandler::getRequestBuf( size_t size )
{
	this->allocate( requests, size );
	return requests.data;
}

void* BufferHandler::getRequestIDBuf( size_t size )
{
	this->allocate( request_ids, size );
	return request_ids.data;
}
