#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>


int main( int argc, char** argv )
{
	MPI_Init( &argc, &argv );
	int com_rank, com_size;
	char buf0[100], buf1[100];
	MPI_Comm_rank( MPI_COMM_WORLD, &com_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &com_size );

	MPI_Request req0, req1;
	MPI_Status status;
	int flag = 0;
	if ( com_rank == 0 )
	{
		printf( "Send: ");
		MPI_Isend( buf0, 100, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &req0 );
		printf( "Done\n");
		sleep( 1 );
		printf( "Wait: ");
		//MPI_Wait( &req, &status );
		printf( "Done\nSend: " );
		MPI_Barrier( MPI_COMM_WORLD );
		//MPI_Send( buf1, 100, MPI_BYTE, 1, 1, MPI_COMM_WORLD );	
		/**/
		MPI_Irecv( buf1, 100, MPI_BYTE, 1, 1, MPI_COMM_WORLD, &req1 );
		//MPI_Wait( &req, &status );
		MPI_Request requests[] = { req0, req1 };
		MPI_Status stats[2];
		MPI_Waitall( 2, requests, stats );
		/**/
		MPI_Sendrecv( buf0, 100, MPI_BYTE, 1, 0,
			buf1, 100, MPI_BYTE, 1, 1,
			MPI_COMM_WORLD, &status );
		/**/
		MPI_Bcast( buf0, 100, MPI_BYTE, 0, MPI_COMM_WORLD );
		/* Scatter */
		MPI_Scatter( buf0, 50, MPI_BYTE, buf1, 50, MPI_BYTE, 0, MPI_COMM_WORLD );
		/* Gather */
		MPI_Gather( buf0, 50, MPI_BYTE, buf1, 50, MPI_BYTE, 0, MPI_COMM_WORLD );
		/* Reduce */
		MPI_Reduce( buf0, buf1, 100, MPI_BYTE, MPI_BOR, 0, MPI_COMM_WORLD );
		/* Allreduce */
		MPI_Allreduce( buf0, buf1, 100, MPI_BYTE, MPI_BOR, MPI_COMM_WORLD );
		
		printf( "Done.\n" );
	} else {
		//MPI_Irecv( buf1, 100, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &req );
		//MPI_Test( &req, &flag, &status );
		sleep(1);
		//MPI_Wait( &req, &status );
		MPI_Barrier( MPI_COMM_WORLD );
		MPI_Recv( buf0, 100, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status );
		/**/
		MPI_Send( buf1, 100, MPI_BYTE, 0, 1, MPI_COMM_WORLD );
		/**/
		//MPI_Wait( &req, &status );
		
		MPI_Sendrecv( buf0, 100, MPI_BYTE, 0, 1,
			buf1, 100, MPI_BYTE, 0, 0,
			MPI_COMM_WORLD, &status );
		/* Bcast */
		MPI_Bcast( buf0, 100, MPI_BYTE, 0, MPI_COMM_WORLD );
		/* Scatter */
		MPI_Scatter( buf0, 50, MPI_BYTE, buf1, 50, MPI_BYTE, 0, MPI_COMM_WORLD );
		/* Gather */
		MPI_Gather( buf0, 50, MPI_BYTE, buf1, 50, MPI_BYTE, 0, MPI_COMM_WORLD );
		/* Reduce */
		MPI_Reduce( buf0, buf1, 100, MPI_BYTE, MPI_BOR, 0, MPI_COMM_WORLD );
		/* Allreduce */
		MPI_Allreduce( buf0, buf1, 100, MPI_BYTE, MPI_BOR, MPI_COMM_WORLD );
	}


	MPI_Finalize();

	return 0;
}

