#ifndef HANDLER_H
#define HANDLER_H


#include <otf.h>


/* handler definitions */

int handle_DefFunction (void *userData, uint32_t stream, uint32_t func, const char *name,
			uint32_t funcGroup, uint32_t source);

int handle_DefTimerResolution (void *userData, uint32_t stream, uint64_t ticksPerSecond);

int handle_DefProcessGroup( void *userData, uint32_t stream, uint32_t procGroup, const char *name, uint32_t numberOfProcs, const uint32_t *procs );


int handle_DefKeyValue ( void *userData, uint32_t stream, 
			 uint32_t token, const char *name, OTF_KeyValueList *list );

int handle_Enter (void *userData, uint64_t time, uint32_t function, uint32_t process,
		  uint32_t source, OTF_KeyValueList *list );

int handle_Leave (void *userData, uint64_t time, uint32_t function, uint32_t process,
		  uint32_t source, OTF_KeyValueList *list );

int handle_SendMsg (void *userData, uint64_t time, uint32_t sender, uint32_t receiver,
		    uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list );

int handle_RecvMsg (void *userData, uint64_t time, uint32_t recvProc, uint32_t sendProc,
		    uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list );

/*
int handle_CollectiveOperation (void *userData, uint64_t time, uint32_t process,
				uint32_t collective, uint32_t procGroup, uint32_t rootProc, uint32_t sent, 
    uint32_t received, uint64_t duration, uint32_t source, OTF_KeyValueList *list );
*/

int handle_BeginCollectiveOperation( void* userData, uint64_t time, uint32_t process, uint32_t collOp,
										uint64_t matchingId, uint32_t procGroup, uint32_t rootProc,
										uint64_t sent, uint64_t received, uint32_t scltoken,
										OTF_KeyValueList *list );

int handle_EndCollectiveOperation( void* userData, uint64_t time, uint32_t process, uint64_t matchingId,
									OTF_KeyValueList *list );


#endif /* HANDLER_H */
