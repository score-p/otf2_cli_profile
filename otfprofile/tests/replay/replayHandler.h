#ifndef REPLAY_HANDLER_H
#define REPLAY_HANDLER_H

#include <mpi.h>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <time.h>

#include <cassert>

#include <otf.h>

#include "otfMap.h"
#include "replayBufferHandler.h"
#include "modificationManager.h"
#include "replayTimer.h"
#include "replayLogger.h"

#define REPLAY_KEY_NUM_COL 1000

/* Key definitions */
enum enum_ReplayKeys{
	REPLAY_KEY_MPI_FUNC=1,
 	REPLAY_KEY_SOURCE=2,
 	REPLAY_KEY_DEST=3,
 	REPLAY_KEY_COMM1=4,
 	REPLAY_KEY_COMM2=5,
 	REPLAY_KEY_COMM3=6,
 	REPLAY_KEY_SENDTAG=7,
 	REPLAY_KEY_RECVTAG=8,
 	REPLAY_KEY_REQUEST_ID=9,
 	REPLAY_KEY_REQUEST_COUNT=10,
 	REPLAY_KEY_SEND_SIZE=11,
 	REPLAY_KEY_RECV_SIZE=12,
 	REPLAY_KEY_COMM_SIZE=13,
 	//REPLAY_KEY_TYPE_SIZE=14,
 	REPLAY_KEY_ROOT=15,
 	REPLAY_KEY_REQUEST_IDS=16,
 	REPLAY_KEY_COMM_MEMBERS=17,
 	/* number of elements per process for collective operations */
	//REPLAY_KEY_COL_COUNTS=18,
 	REPLAY_KEY_COL_RECVCOUNTS=18,
 	REPLAY_KEY_COL_SENDCOUNTS=19,
 	/* displacements per process for collective operations */
 	REPLAY_KEY_COL_RECVDISPLS=20,
  	REPLAY_KEY_COL_SENDDISPLS=21,
  	/* size of types for send and receive data */
  	REPLAY_KEY_RECVTYPE_SIZE=22,
  	REPLAY_KEY_SENDTYPE_SIZE=23
};

enum enum_RequestFlags{
	REPLAY_REQUEST_FINISHED = 0,
	REPLAY_REQUEST_ACTIVE = 1,
	REPLAY_REQUEST_RECEIVED = 3
};


using namespace std;

typedef struct structMPIComm{
	MPI_Comm comm;
	MPI_Group group;
	//uint32_t id;
	int size;
	int rank;
	/*structMPIComm(): comm(MPI_COMM_NULL) { }
	structMPIComm(MPI_Group group ): comm(MPI_COMM_NULL), size(-1), rank(-1)
	{
		this->group = group;
	}
	structMPIComm(MPI_Comm comm)
	{
		this->comm = comm;
		// gather information about communicator
		MPI_Comm_group( comm, &(this->group) );
		MPI_Comm_rank( comm, &(this->rank) );
		MPI_Comm_size( comm, &(this->size) );
	}*/
} comm_t;

typedef struct structMPIRequest{
	MPI_Request request;
	uint8_t* buffer;
	uint8_t flag;
	uint32_t id;
} request_t;


// TODO: proper cleanup needed
struct structState{
	map<uint32_t, MPI_Request> replay_requests;
	map<uint32_t, uint8_t*> replay_databuf;
	map<uint32_t, uint8_t> replay_flags; /* 0=undef, 1=active, 2=late, 3=done */
	//map<uint32_t, MPI_Group> replay_groups;
	//OtfMap<MPI_Group> replay_groups;
	//map<uint32_t, MPI_Comm> replay_comms;
	OtfMap<comm_t> replay_comms;
};
typedef structState state_t;

/* handler class */

class ReplayHandler {
private:
	int _rank;
	int _size;

	map<uint32_t, string> _function_map;
	stack<uint32_t> _function_stack;
	state_t _state;

	/* collection of buffers to save calls to malloc */
	BufferHandler _buffer_handler;
	
	/* saves unknown MPI calls --> this is temporary */
	set<uint32_t> mpi_map;
	map<uint32_t, string> key_map;
	
	ReplayTimer timer;
	ReplayLogger logger;
	
	void printKeys( OTF_KeyValueList *list );
	void finishRequest( uint32_t req_id );
	uint8_t* ntohs_array( uint8_t* array, uint32_t len );
	int readByteArray( OTF_KeyValueList* list, uint32_t key, uint8_t* buf, uint32_t len );
	/* private handle members */
	void handleMPIIrecv( OTF_KeyValueList *list );
	void handleMPISendrecv( OTF_KeyValueList* list );
	void handleMPIWait( OTF_KeyValueList *list );
	void handleMPIWaitall( OTF_KeyValueList *list );
	void handleMPIWaitany( OTF_KeyValueList *list );
	void handleMPITest( OTF_KeyValueList *list );
	void handleMPITestall( OTF_KeyValueList* list );
	void handleMPITestany( OTF_KeyValueList* list );
	void handleMPIGroupincl( OTF_KeyValueList *list );
	void handleMPIGrouprangeincl( OTF_KeyValueList *list );
	void handleMPIGroupexcl( OTF_KeyValueList *list );
	void handleMPIGroupcompare( OTF_KeyValueList *list );
	void handleMPIGrouprangeexcl( OTF_KeyValueList *list );
	void handleMPICommcreate( OTF_KeyValueList *list );
	void handleMPICommdup( OTF_KeyValueList *list );
	void handleMPICommsize( OTF_KeyValueList *list );
	void handleMPICommrank( OTF_KeyValueList *list );
	void handleMPICommfree( OTF_KeyValueList* list );
	void handleMPICommcompare(OTF_KeyValueList *list);
	void handleMPICommgroup( OTF_KeyValueList* list );
	void handleMPIGrouprank( OTF_KeyValueList* list );
	void handleMPIGroupsize( OTF_KeyValueList* list );
	void handleMPIGroupfree( OTF_KeyValueList* list );
	void handleMPIGroupintersection( OTF_KeyValueList *list );
	void handleMPIGroupdifference( OTF_KeyValueList *list );
	void handleMPIGroupunion( OTF_KeyValueList *list );
	
	void handleMPIProbe( OTF_KeyValueList* list );
	void handleMPIIprobe( OTF_KeyValueList* list );
	
public:
	ReplayHandler(const ReplayHandler& handler);
	ReplayHandler();
	ReplayHandler(const int rank );
	ReplayHandler(const int rank, const int size );
	~ReplayHandler();
	ReplayHandler& operator=(const ReplayHandler& handler);
	
	int handleDefFunction(const uint32_t func, const char *name,
			      const uint32_t funcGroup, const uint32_t source);
	int handleDefTimerResolution(const uint64_t ticksPerSecond);

	int handleDefProcessGroup(const uint32_t procGroup, const char *name, 
				  const uint32_t numberOfProcs, const uint32_t *procs );

	int handleDefKeyValue(const uint32_t token, const char *name, const OTF_KeyValueList *list );

	int handleEnter (const uint64_t time, const uint32_t function, const uint32_t process,
			 const uint32_t source, OTF_KeyValueList *list );

	int handleLeave (const uint64_t time, const uint32_t function, const uint32_t process,
			 const uint32_t source, OTF_KeyValueList *list);

	int handleSendMsg (const uint64_t time, const uint32_t sender, const uint32_t receiver,
			   const uint32_t group, const uint32_t type, const uint32_t length, 
      			   const uint32_t source, OTF_KeyValueList *list );

	int handleRecvMsg (const uint64_t time, const uint32_t recvProc, const uint32_t sendProc,
			   const uint32_t group, const uint32_t type, const uint32_t length, const uint32_t source, OTF_KeyValueList *list);
		
	/*int handleCollectiveOperation ( uint64_t time, uint32_t process,
				uint32_t collective, uint32_t procGroup, uint32_t rootProc, uint32_t sent, 
    				uint32_t received, uint64_t duration, uint32_t source, OTF_KeyValueList *list );
	*/
	int handleBeginCollectiveOperation(const uint64_t time, const uint32_t process,
					   const uint32_t collOp, const uint64_t matchingId, const uint32_t procGroup,
					   const uint32_t rootProc, const uint64_t sent, const uint64_t received,
					   const uint32_t scltoken, OTF_KeyValueList *list);

	int handleEndCollectiveOperation(const uint64_t time, const uint32_t process, const uint64_t matchingId, OTF_KeyValueList *list );
};

#endif /* REPLAY_HANDLER_H */
