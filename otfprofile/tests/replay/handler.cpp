
#include <mpi.h>
#include "handler.h"
#include "replayHandler.h"
#include "replayExceptions.h"
#include <stdlib.h>

#define VTRACE 1
#include <vt_user.h>
#include <otf.h>
#include <string>
#include <iostream>

static void do_exit( string msg )
{
	cerr << msg << "\nAborting!" << endl;
	MPI_Finalize();
	exit(1);
}


/* Callback functions */
int handle_DefFunction (void *userData, uint32_t stream, uint32_t func, const char *name,
			uint32_t funcGroup, uint32_t source) 
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleDefFunction(func, name, funcGroup, source);
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}

int handle_DefTimerResolution (void *userData, uint32_t stream, uint64_t ticksPerSecond)
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleDefTimerResolution(ticksPerSecond);
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}

int handle_DefProcessGroup( void *userData, uint32_t stream, uint32_t procGroup, const char *name, 
			    uint32_t numberOfProcs, const uint32_t *procs )
{	
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleDefProcessGroup(procGroup, name, numberOfProcs, procs);
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}


int handle_DefKeyValue ( void *userData, uint32_t stream, 
			 uint32_t token, const char *name, OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleDefKeyValue(token, name, list);
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}

int handle_Enter (void *userData, uint64_t time, uint32_t function, uint32_t process,
		  uint32_t source, OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		return handler->handleEnter( time, function, process, source, list );
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}

int handle_Leave (void *userData, uint64_t time, uint32_t function, uint32_t process,
		  uint32_t source, OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleLeave( time, function, process, source, list );
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}

int handle_SendMsg (void *userData, uint64_t time, uint32_t sender, uint32_t receiver,
		    uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		return handler->handleSendMsg( time, sender, receiver, group, type, length, source, list );
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}

int handle_RecvMsg (void *userData, uint64_t time, uint32_t recvProc, uint32_t sendProc,
		    uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		return handler->handleRecvMsg( time, recvProc, sendProc, group, type, length, source, list );
	} catch( OtfNotFoundException& exc ){
		do_exit( exc.what() );
	}
	return res;
}
/*
int handle_CollectiveOperation (void *userData, uint64_t time, uint32_t process, uint32_t collective, 
				uint32_t procGroup, uint32_t rootProc, uint32_t sent, uint32_t received, 
    				uint64_t duration, uint32_t source, OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleCollectiveOperation( time, process, collective, procGroup, rootProc, 
				sent, received, duration, source, list );
	} catch( OtfNotFoundException& exc ){
		exit( exc.what() );
	}
	return res;
}
*/
int handle_BeginCollectiveOperation( void* userData, uint64_t time, uint32_t process, uint32_t collOp,
										uint64_t matchingId, uint32_t procGroup, uint32_t rootProc,
										uint64_t sent, uint64_t received, uint32_t scltoken,
										OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleBeginCollectiveOperation( time, process, collOp, matchingId, procGroup, rootProc,
														sent, received, scltoken, list );
	}catch( OtfNotFoundException& exc ){
		do_exit(exc.what());
	}
	return res;
}

int handle_EndCollectiveOperation( void* userData, uint64_t time, uint32_t process, uint64_t matchingId,
									OTF_KeyValueList *list )
{
	ReplayHandler *handler = static_cast<ReplayHandler*>(userData);
	int res = 0;
	try{
		res = handler->handleEndCollectiveOperation( time, process, matchingId, list );
	}catch( OtfNotFoundException& exc){
		do_exit(exc.what());
	}
	return res;
}
