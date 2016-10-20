#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "replayHandler.h"
#include "handler.h"
#include "vt_user.h"

using namespace std;

static void print_help(char* progname)
{
	cout << "Usage:" << endl;
	cout << "\t" << progname << " [-s <data size factor>] [-t <wait time factor>] otf_file" << endl;
}

static void help_and_exit(char* progname)
{
	print_help(progname);
	exit(1);
}


int main(int argc, char **argv) {

	int num_otf_streams;
	int com_rank;
	int com_size;
	char *tracefile = NULL;
	
	VT_OFF();
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &com_rank );
	MPI_Comm_size(MPI_COMM_WORLD, &com_size );

	ReplayHandler replay_handler(com_rank, com_size);
	
	if( argc < optind ) {
		if( com_rank == 0) {
			printf("Usage: 1th parameter = tracefile.\n");
		}
		MPI_Finalize();
		exit(1);
	}
	
	tracefile = OTF_stripFilename(argv[optind]);

	OTF_Reader *reader;
	OTF_FileManager *manager;
	OTF_HandlerArray *handlers;
	OTF_MasterControl *master;

	manager= OTF_FileManager_open( 100 );
	assert( manager );

	handlers = OTF_HandlerArray_open();
	assert( handlers );

	/* read definitions */
	reader = OTF_Reader_open( tracefile, manager );
	if( reader == NULL) {
		if( com_rank == 0) {
			cerr <<  "Error: Tracefile not found. Abort." << endl;
		}

		MPI_Finalize();
		return 1;
	}

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_DefFunction, OTF_DEFFUNCTION_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_DEFFUNCTION_RECORD );

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_DefTimerResolution, OTF_DEFTIMERRESOLUTION_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_DEFTIMERRESOLUTION_RECORD );

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_DefProcessGroup, OTF_DEFPROCESSGROUP_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_DEFPROCESSGROUP_RECORD );

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_DefKeyValue, OTF_DEFKEYVALUE_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_DEFKEYVALUE_RECORD );

	OTF_Reader_readDefinitions( reader, handlers);


	/* read master file */
	master = OTF_MasterControl_new( manager );
	OTF_MasterControl_read( master, tracefile );
	num_otf_streams = OTF_MasterControl_getCount( master );

	OTF_MasterControl_close( master );

	if( num_otf_streams != com_size ) {
		if ( com_rank == 0 ) {
			cerr << "Error: The number of otf streams ("<< num_otf_streams << ") and the number "
			<< " of mpi processes ("<< com_size << ") differs. Abort." << endl;
		}

		MPI_Finalize();
		return 1;
	}

	/* read events */

	OTF_Reader_setProcessStatusAll( reader, 0);
	OTF_Reader_setProcessStatus( reader, com_rank + 1, 1);

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_Enter, OTF_ENTER_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_ENTER_RECORD );	

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_Leave, OTF_LEAVE_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_LEAVE_RECORD );	

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_SendMsg, OTF_SEND_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_SEND_RECORD );	

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_RecvMsg, OTF_RECEIVE_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_RECEIVE_RECORD );

	/*
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_CollectiveOperation, OTF_COLLOP_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_COLLOP_RECORD );
	*/
	
	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_BeginCollectiveOperation, OTF_BEGINCOLLOP_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_BEGINCOLLOP_RECORD );

	OTF_HandlerArray_setHandler( handlers, (OTF_FunctionPointer*) handle_EndCollectiveOperation, OTF_ENDCOLLOP_RECORD );
	OTF_HandlerArray_setFirstHandlerArg( handlers, &replay_handler, OTF_ENDCOLLOP_RECORD );
	
	MPI_Barrier( MPI_COMM_WORLD );
	VT_ON();
	OTF_Reader_readEvents( reader, handlers );
	VT_OFF();
	OTF_Reader_close( reader );

	OTF_HandlerArray_close( handlers );
	OTF_FileManager_close( manager );

	MPI_Finalize();

	return 0;
}

