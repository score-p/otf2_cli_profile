
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "otf.h"
#include "replayHandler.h"


#define HELPTEXT "" \
"                                                             \n" \
" ./replay_example - writes a synthetic example for the       \n" \
"         MPI replay mechanism including key-value pairs      \n" \
"                                                             \n" \


int main( int argc, const char** argv ) {


	const char *filename= "example_trace.otf";

	OTF_FileManager* manager;
	OTF_Writer* writer;

	/* test with # streams < or > # processes */
	uint32_t streams= 2;

	/* file handles available */
	uint32_t files= 100;

	/* per-stream budder size */
	uint32_t buffersize= 10*1024;

	manager= OTF_FileManager_open( files );
	assert( NULL != manager );

	writer = OTF_Writer_open( filename, streams, manager );
	OTF_Writer_setBufferSizes( writer, buffersize );
	OTF_Writer_setCompression( writer, 0 );
	
	OTF_KeyValueList* list = OTF_KeyValueList_new();



/* *** actual events for rank 0 (left) and rank 1 (right) *************************** */

        /* enter main 100 */
        OTF_Writer_writeEnter( writer, 100, 1, 1, 0 );

       		 		/* enter main 100 */
				OTF_Writer_writeEnter( writer, 100, 1, 2, 0 );

				/* kv 1:8 MPI_Irecv */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 8 );
				/* kv 2:1 receiver rank */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_SENDER, 1 );
				/* kv 3:9 communicator ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM1, 9 );
				/* kv 4:10 tag */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_TAG, 10 );
				/* kv 5:1 request ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
				/* kv 6:4 length*/
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_LENGTH, 4 );
				/* enter MPI Irecv 1010 */
				OTF_Writer_writeEnterKV( writer, 1010, 4, 2, 0, list );

        /* enter do_someting 2100 */
        OTF_Writer_writeEnter( writer, 2100, 2, 1, 0 );

				/* leave MPI Irecv 13010 */
				OTF_Writer_writeLeave( writer, 13010, 4, 2, 0 );
				/* kv 1:9 MPI_Test */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 9 );
				/* kv 5:1 request ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
				/* enter MPI_Test 13200 */
				OTF_Writer_writeEnterKV( writer, 13200, 5, 2, 0, list );
				/* leave MPI_Test 13300*/
				OTF_Writer_writeLeave( writer, 13300, 5, 2, 0 );
			
				/* enter do_someting 013400 */
				OTF_Writer_writeEnter( writer, 13400, 2, 2, 0 );


        /* leave do_someting 102630 */
        OTF_Writer_writeLeave( writer, 102630, 2, 1, 0 );


        /* enter MPI Isend 105000 */
        OTF_Writer_writeEnter( writer, 105000, 3, 1, 0 );

        /* kv 1:6 MPI_Isend */
		OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 6 );
        /* kv 5:1 request ID */
		OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
		/* kv: MPI communicator */
		OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM1, 9 );
        /* sendmsg 105000 */
        OTF_Writer_writeSendMsgKV( writer, 105000, 1, 2, 9 /* comm */, 10 /* tag */, 4, 0, list );

        /* leave MPI Isend 106000 */
        OTF_Writer_writeLeave( writer, 106000, 3, 1, 0 );

        /* enter do_someting 107000 */
        OTF_Writer_writeEnter( writer, 107000, 2, 1, 0 );

				/* leave do_someting 108400*/
				OTF_Writer_writeLeave( writer, 108400, 2, 2, 0 );
			
				/* kv 1:9 MPI_Test */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 9 );
				/* kv 5:1 request ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
				/* enter MPI_Test 108500 */
				OTF_Writer_writeEnterKV( writer, 108500, 5, 2, 0, list );
			
				/* leave MPI_Test 108600 */
				OTF_Writer_writeLeave( writer, 108600, 5, 2, 0 );

        /* leave do_someting 109630 */
        OTF_Writer_writeLeave( writer, 109630, 2, 1, 0 );

		/* kv 1:7 MPI_Wait */
		OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 7 );
		/* kv 5:1 request ID */
		OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
		/* enter MPI_Wait    109660 */
		OTF_Writer_writeEnterKV( writer, 109660, 6, 1, 0, list );
			
				/* enter do_someting 110000 */
				OTF_Writer_writeEnter( writer, 110000, 2, 2, 0 );
				/* leave do_someting 202700 */
				OTF_Writer_writeLeave( writer, 202700, 2, 2, 0 );
			
				/* kv 1:9 MPI_Test */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 9 );
				/* kv 5:1 request ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
				/* enter MPI_Test 202800 */
				OTF_Writer_writeEnterKV( writer, 202800, 5, 2, 0, list );

        /* kv 1:10 MPI_Test success */	
		OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 10 );
        /* kv 5:1  request ID */
        OTF_KeyValueList_appendUint32( list, REPLAY_KEY_REQUEST_ID, 1 );
        /* recvmsg        203000 */
        OTF_Writer_writeRecvMsgKV( writer, 203000, 2, 1, 9 /* comm */, 10 /* tag */, 4, 0, list );
				/* leave MPI_Test 203000 */
				OTF_Writer_writeLeave( writer, 203000, 5, 2, 0 );

        /* leave MPI_Wait 203500 */
        OTF_Writer_writeLeave( writer, 203500, 6, 1, 0 );

        /* leave main 205000*/
        OTF_Writer_writeLeave( writer, 205000, 1, 1, 0 );

				/* leave main 205050 */
				OTF_Writer_writeLeave( writer, 205050, 1, 2, 0 );
				
/* Test Group functionality */
	int new_group[] = {0, 1};
	
	/* kv 1:11 MPI_Group_incl */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 11 );
	/* kv 9: Number of memebers of the new group */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM_SIZE, 2 );	
	/* kv 7: Member of the new group */
	OTF_KeyValueList_appendByteArray( list, REPLAY_KEY_COMM_MEMBERS, (uint8_t*)new_group, sizeof( int ) * 2 );	
	/* kv 8: Group ID */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM1, 1 );	
	/* kv: source group */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM2, 9 );
	/* enter MPI_Group_incl 205500 */
	OTF_Writer_writeEnterKV( writer, 205500, 7, 1, 0, list );
	
	/* leave MPI_Group_incl 206000 */
	OTF_Writer_writeLeave( writer, 206000, 7, 1, 0 );
	
				/* kv 1:11 MPI_Group_incl */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 11 );
				/* kv 9: Number of memebers of the new group */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM_SIZE, 2 );	
				/* kv 7: Member of the new group */
				OTF_KeyValueList_appendByteArray( list, REPLAY_KEY_COMM_MEMBERS, (uint8_t*)new_group, sizeof( int ) * 2 );	
				/* kv 8: Group ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM1, 1 );
				/* kv: source group */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM2, 9 );
				/* enter MPI_Group_incl 205500 */
				OTF_Writer_writeEnterKV( writer, 205500, 7, 2, 0, list );
				
				/* leave MPI_Group_incl 206000 */
				OTF_Writer_writeLeave( writer, 206000, 7, 2, 0 );
				
	/* kv: Group ID */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM1, 1 );
	/* kv 1:11 MPI_Group_incl */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 12 );
	/* kv: Communicator ID */
	OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM2, 9 );
	/* enter MPI_Comm_create 206500 */
	OTF_Writer_writeEnterKV( writer, 206500, 8, 1, 0, list );
	

	/* leave MPI_Comm_create 207000*/
	OTF_Writer_writeLeave( writer, 207000, 8, 1, 0 );
	
	
				/* kv: Group ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM1, 1 );
				/* kv 1:11 MPI_Group_incl */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_MPI_FUNC, 12);
				/* kv: Communicator ID */
				OTF_KeyValueList_appendUint32( list, REPLAY_KEY_COMM2, 9 );
				/* enter MPI_Comm_create 206500 */
				OTF_Writer_writeEnterKV( writer, 206500, 8, 2, 0, list );
				

				/* leave MPI_Comm_create 207000*/
				OTF_Writer_writeLeave( writer, 207000, 8, 2, 0 );


/* ********************************************************************************** */


	/* write definitions -- may be written at any time 
	before/during/after writing events */

	OTF_Writer_writeDefTimerResolution( writer, 
		0 /* uint32_t stream */, 
		1e5 /* uint64_t ticksPerSecond */ );

	OTF_Writer_writeDefProcess( writer, 
		0 /* uint32_t stream */, 
		1 /* uint32_t process */, 
		"Rank 0" , 
		0 /* uint32_t parent */ );
	OTF_Writer_writeDefProcess( writer, 
		0 /* uint32_t stream */, 
		2 /* uint32_t process */, 
		"Rank 1" /* const char *name */ , 
		0 /* uint32_t parent */ );

    const uint32_t group[]= {1,2};
    OTF_Writer_writeDefProcessGroup( writer, 
        0, 
        9, 
        "MPI_COMM_WORLD", 
        2, group );
    
    	OTF_Writer_writeDefProcessGroup( writer,
		0,
  		10,
    		"MPI_COMM_1",
      		2, group );

	OTF_Writer_writeDefFunctionGroup( writer, 
		0 /* uint32_t stream */, 
		16 /* uint32_t funcGroup */, 
		"MPI" /* const char *name */ );

	OTF_Writer_writeDefFunctionGroup( writer, 
		0 /* uint32_t stream */, 
		17 /* uint32_t funcGroup */, 
		"User" /* const char *name */ );

	OTF_Writer_writeDefFunction( writer, 
		0 /* uint32_t stream */, 
		1 /* uint32_t func */, 
		"main" /* const char *name */, 
		17 /* uint32_t funcGroup */, 
		0 /* uint32_t source */ );

	OTF_Writer_writeDefFunction( writer, 
		0 /* uint32_t stream */, 
		2 /* uint32_t func */, 
		"do_something" /* const char *name */, 
		17 /* uint32_t funcGroup */, 
		0 /* uint32_t source */ );

	OTF_Writer_writeDefFunction( writer, 
		0 /* uint32_t stream */, 
		3 /* uint32_t func */, 
		"MPI_Isend" /* const char *name */, 
		16 /* uint32_t funcGroup */, 
		0 /* uint32_t source */ );

	OTF_Writer_writeDefFunction( writer, 
		0 /* uint32_t stream */, 
		4 /* uint32_t func */, 
		"MPI_Irecv" /* const char *name */, 
		16 /* uint32_t funcGroup */, 
		0 /* uint32_t source */ );

	OTF_Writer_writeDefFunction( writer, 
		0 /* uint32_t stream */, 
		5 /* uint32_t func */, 
		"MPI_Test" /* const char *name */, 
		16 /* uint32_t funcGroup */, 
		0 /* uint32_t source */ );

	OTF_Writer_writeDefFunction( writer, 
		0 /* uint32_t stream */, 
		6 /* uint32_t func */, 
		"MPI_Wait" /* const char *name */, 
		16 /* uint32_t funcGroup */, 
		0 /* uint32_t source */ );
	
	OTF_Writer_writeDefFunction( writer,
		0 /* uint32_t stream */, 
	 	7 /* uint32_t func */, 
  		"MPI_Group_incl" /* const char *name */, 
  		16 /* uint32_t funcGroup */, 
  		0 /* uint32_t source */ );
	
	OTF_Writer_writeDefFunction( writer,
		0 /* uint32_t stream */, 
		8 /* uint32_t func */, 
  		"MPI_Comm_create" /* const char *name */, 
  		16 /* uint32_t funcGroup */, 
  		0 /* uint32_t source */ );

    /* key value definitions */

	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_MPI_FUNC, "REPLAY: MPI routines" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_SENDER, "REPLAY: receiver rank" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_COMM1, "REPLAY: commuicator ID" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_TAG, "REPLAY: tag" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_REQUEST_ID, "REPLAY: request ID" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_LENGTH, "REPLAY: length" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_COMM_MEMBERS, "REPLAY: members of group" );
	OTF_Writer_writeDefKeyValue( writer, 0, REPLAY_KEY_COMM_SIZE, "REPLAY: size of group" );

	OTF_Writer_close( writer );

	return 0;
}


