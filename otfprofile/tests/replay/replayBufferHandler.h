
#ifndef REPLAY_BUFFER_HANDLER_H
#define REPLAY_BUFFER_HANDLER_H

//#include <cstdlib>

typedef struct structBuffer{
	uint8_t* data;
	size_t size;
	structBuffer(): data(NULL),size(0){ }
} buffer_t;

class BufferHandler{
private:
	buffer_t send_buf;
	buffer_t recv_buf;
	buffer_t displs;
	buffer_t counts;
	buffer_t idispls;
	buffer_t icounts;
	buffer_t requests;
	buffer_t request_ids;

	void allocate(buffer_t& buf, size_t size );
	void initBuffer( buffer_t& buf );
	
public:
	BufferHandler(void);
	BufferHandler(const BufferHandler& handler);
	BufferHandler& operator=(const BufferHandler& handler);
	~BufferHandler(void);
	void* getSendBuf( size_t size );
	void* getRecvBuf( size_t size );
	void* getDisplsBuf( size_t size );
	void* getCountsBuf( size_t size );
	void* getIdisplsBuf( size_t size );
	void* getIcountsBuf( size_t size );
	void* getRequestBuf( size_t size );
	void* getRequestIDBuf( size_t size );
};

#endif /* REPLAY_BUFFER_HANDLER_H */

