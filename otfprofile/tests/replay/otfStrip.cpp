#include "otf.h"

#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <sys/stat.h>
#include <stdlib.h>

//#include "replayHandler.h"

#define DATA_FACTOR 5
#define TIME_FACTOR 1

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



using namespace std;

typedef struct{
	string src;
	string dest;
} filenames_t;

typedef struct structStats{
	uint32_t num_dropped_functions;
	uint32_t num_copied_functions;
	structStats(): num_dropped_functions(0), num_copied_functions(0) { }
} statistics_t;

typedef struct structModifications{
	float dataFact;
	float timeFact;
} modifications_t;

/* set of functions to copy */
set<uint32_t> copy_functions;
uint32_t num_dropped_functions = 0;
uint32_t num_copied_functions = 0;
modifications_t mods;

static string getFilenames( int argc, char** argv, filenames_t& files )
{
	if (argc >= 2)
	{
		files.src = argv[1];
	} else {
		cerr << "Usage: " << argv[0] << " <otf-file>" << endl;
		exit( 1 );
	}
	if (argc == 3)
	{
		files.dest = argv[2];
	} else {
		files.dest = files.src;
		files.dest.insert( files.dest.find_last_of( '.' ) , "_strip" );
	}
	return string(argv[1]);
}

static void rewriteKVList(OTF_KeyValueList* list)
{
	OTF_KeyValuePairList *plist;
	uint i = 0;
	for (plist = list->kvBegin; plist != NULL && i++ < list->count; plist = plist->kvNext)
	{
		OTF_KeyValuePair *pair = &(plist->kvPair);
		if (	pair->key == REPLAY_KEY_SEND_SIZE ||
			pair->key == REPLAY_KEY_RECV_SIZE )
		{
			pair->value.otf_uint32 *= mods.dataFact;
		}
	}
	// displacement and counts array
	for (i = 0, plist = list->kvBegin; plist != NULL && i++ < list->count; plist = plist->kvNext)
	{
		OTF_KeyValuePair *pair = &(plist->kvPair);
		if (pair->key == REPLAY_KEY_COL_RECVDISPLS
		   || pair->key == REPLAY_KEY_COL_SENDDISPLS
		   || pair->key == REPLAY_KEY_COL_RECVCOUNTS
		   || pair->key == REPLAY_KEY_COL_SENDCOUNTS)
		{
			uint32_t *elem = (uint32_t*)pair->value.otf_byte_array.array;
			for (i = 0; i < pair->value.otf_byte_array.len; i += 4, elem++)
			{
				*elem *= mods.dataFact;
			}
		}
	}
}


/* handler for function definitions */
static int handle_DefFunction (void *userData, uint32_t stream, uint32_t func, const char *name,
			uint32_t funcGroup, uint32_t source)
{
	int ret = OTF_RETURN_OK;
	string function_name = name;
	OTF_Writer* writer = (OTF_Writer*) userData;
	if ( function_name.substr( 0, 4 ) == "MPI_" || function_name == "main" )
	{
		copy_functions.insert( func );
		ret = (0 == OTF_Writer_writeDefFunction( writer, stream, func, name, funcGroup, source ) ? 
				OTF_RETURN_ABORT : OTF_RETURN_OK );
	}
	return ret;
	
}


/* handler functions for events */
static int handle_Enter (void *userData, uint64_t time, uint32_t function, uint32_t process,
		  uint32_t source, OTF_KeyValueList *list )
{
	OTF_Writer* writer = (OTF_Writer*) userData;
	int ret = OTF_RETURN_OK;
	if ( copy_functions.find( function ) != copy_functions.end() )
	{
		rewriteKVList(list);
		ret = ( 0 == OTF_Writer_writeEnterKV( writer, time, function, process, source, list ) ? 
				OTF_RETURN_ABORT : OTF_RETURN_OK );
		++num_copied_functions;
	} else {
		++num_dropped_functions;
	}
	return ret;
}

static int handle_Leave (void *userData, uint64_t time, uint32_t function, uint32_t process,
		  uint32_t source, OTF_KeyValueList *list )
{
	OTF_Writer* writer = (OTF_Writer*) userData;
	int ret = OTF_RETURN_OK;
	if ( copy_functions.find( function ) != copy_functions.end() )
	{
		rewriteKVList(list);
		ret = ( 0 == OTF_Writer_writeLeaveKV( writer, time, function, process, source, list ) ? 
				OTF_RETURN_ABORT : OTF_RETURN_OK );
	}
	
	return ret;
}

static int handle_RecvMsg (void *userData, uint64_t time, uint32_t recvProc, uint32_t sendProc,
		    uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list )
{
	OTF_Writer* writer = (OTF_Writer*) userData;
	uint32_t nlength = length * mods.dataFact;
	int ret = (0 == OTF_Writer_writeRecvMsgKV(writer, time, recvProc, sendProc, group, type, nlength, source, list) ? 
			OTF_RETURN_ABORT : OTF_RETURN_OK);
	
	return ret;
}

static int handle_SendMsg(void *userData, uint64_t time, uint32_t sender, uint32_t receiver,
		    uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list )
{
	OTF_Writer *writer = (OTF_Writer*) userData;
	uint32_t nlength = length * mods.dataFact;
	int ret = (0 == OTF_Writer_writeSendMsgKV(writer, time, sender, receiver, group, type, nlength, source, list)? 
			OTF_RETURN_ABORT : OTF_RETURN_OK);
	return ret;
}

static int handle_BeginCollectiveOperation( void* userData, uint64_t time, uint32_t process, uint32_t collOp,
				     uint64_t matchingId, uint32_t procGroup, uint32_t rootProc,
				uint64_t sent, uint64_t received, uint32_t scltoken,
  				OTF_KeyValueList *list )
{
	OTF_Writer *writer = (OTF_Writer*)userData;
	uint32_t sentsz = sent * mods.dataFact;
	uint32_t recvsz = sent * mods.dataFact;
	rewriteKVList(list);
	int ret = (0 == OTF_Writer_writeBeginCollectiveOperationKV( writer, time, process, 
		   		collOp, matchingId, procGroup, rootProc,
		   		sentsz, recvsz, scltoken, list) ? 
			OTF_RETURN_ABORT : OTF_RETURN_OK);
	return ret;
}
int main( int argc, char** argv )
{
	OTF_FileManager* manager;
	OTF_HandlerArray* handlers;
	OTF_Reader* reader;
	OTF_Writer* writer;
	OTF_MasterControl* master;
	int num_otf_streams;
	filenames_t files;
	
	mods.dataFact = DATA_FACTOR;
	mods.timeFact = TIME_FACTOR;
	
	getFilenames( argc, argv, files );
	cout << "Writing to file " << files.dest << " using " << files.src << " as input " << endl;
	manager = OTF_FileManager_open( 100 );
	assert( manager );
	
	handlers = OTF_HandlerArray_open();
	assert( handlers );
	
	reader = OTF_Reader_open( files.src.c_str(), manager );
	if ( !reader )
	{
		cerr << "Unable to open tracefile " << files.src << endl;
	}
	
	/* read number of streams from master file */
	master = OTF_MasterControl_new( manager );
	OTF_MasterControl_read( master, files.src.c_str() );
	num_otf_streams = OTF_MasterControl_getCount( master );
	OTF_MasterControl_close( master );
	
	// open writer 
	cout << "Open writer..." << endl;
	writer = OTF_Writer_open( files.dest.c_str(), num_otf_streams, manager );
	assert( writer );
	
	// set CopyHandler
	cout << "Getting CopyHandler..." << endl;
	OTF_HandlerArray_getCopyHandler( handlers, writer );
	
	// read function definitions
	cout << "Setting handler for definitions..." << endl;
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_DefFunction, OTF_DEFFUNCTION_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, writer, OTF_DEFFUNCTION_RECORD );
	cout << "Reading definitions... " << endl;
	OTF_Reader_readDefinitions( reader, handlers);
	
	// read events
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_Enter, OTF_ENTER_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, writer, OTF_ENTER_RECORD );	

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_Leave, OTF_LEAVE_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, writer, OTF_LEAVE_RECORD );	
	
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_BeginCollectiveOperation, OTF_BEGINCOLLOP_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, writer, OTF_BEGINCOLLOP_RECORD );
	
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_RecvMsg, OTF_RECEIVE_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, writer, OTF_RECEIVE_RECORD );
	
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_SendMsg, OTF_SEND_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, writer, OTF_SEND_RECORD );
	
	cout << "Reading events... " << endl;
	OTF_Reader_readEvents( reader, handlers );
	
	cout << "Done stripping OTF file." << endl
		<< "Number of Enters:\n"
		<< "\tDropped: " << num_dropped_functions << "\n"
		<< "\tRemaining: " << num_copied_functions << endl;
	
	// clean up
	OTF_Writer_close( writer );
	OTF_Reader_close( reader );
	OTF_HandlerArray_close( handlers );
	OTF_FileManager_close( manager );
	return 0;
}
