#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

void do_something( MPI_Group group )
{
	MPI_Comm new_comm;
	int rank;
	int size;
	MPI_Comm_create(MPI_COMM_WORLD, group, &new_comm);
	if (new_comm != MPI_COMM_NULL)
	{
		MPI_Group new_group;
		MPI_Comm_group(new_comm, &new_group);
		int comp_res;
		MPI_Group_compare(group, new_group, &comp_res);
		if (comp_res == MPI_IDENT || comp_res == MPI_SIMILAR)
		{
			printf("group equals new_group\n");
		}
		int data[10];
		int new_rank;
		int new_size;
		printf("MPI_Comm_rank\n");
		MPI_Comm_rank(new_comm, &new_rank);
		printf("MPI_Comm_size\n");
		MPI_Comm_size(new_comm, &new_size);
		//printf("[%i] Process %i of %i doing Bcast\n", rank, new_rank, new_size);
		MPI_Bcast(data, sizeof(data) / sizeof(int), MPI_INT, 0, new_comm);
		printf("MPI_Comm_free\n");
		MPI_Comm_free(&new_comm);
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	printf("MPI_Group_free\n");
	MPI_Group_free(&group);
}

void test_waitall()
{
	int sdata[10];
	int rdata[10];
	int rank, size;
	int dest, src;
	MPI_Request sreq, rreq;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	dest = (rank + 1) % size;
	src = (rank + size -1) % size;
	MPI_Isend(sdata, sizeof(sdata), MPI_BYTE, dest, 0, MPI_COMM_WORLD, &sreq);
	MPI_Irecv(rdata, sizeof(rdata), MPI_BYTE, src, 0, MPI_COMM_WORLD, &rreq);
	
	MPI_Request reqs[2];
	reqs[0] = sreq;
	reqs[1] = rreq;
	printf("Doing Waitall\n");
	MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
}


int main( int argc, char** argv )
{
	int rank;
	int size;
	int result;
	MPI_Group old_group;
	MPI_Group new_group1;
	MPI_Group new_group2;
	MPI_Group new_group3;
	MPI_Comm new_comm;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (size < 4)
	{
		printf("Not enough processes. Use at least 4!\n");
		exit(1);
	}
	MPI_Comm_group(MPI_COMM_WORLD, &old_group);
	
	int ranks[] = {0,1,2};
	int ranks1[] = {3};
	printf("Creating communicator with %li ranks\n", sizeof(ranks) / sizeof(int));
	// inclusive 
	MPI_Group_incl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group1);
	do_something( new_group1 );
	
	
	// exclusive
	MPI_Group_excl(old_group, sizeof(ranks1) / sizeof(int), ranks1, &new_group2);
	do_something( new_group2 );
	/*
	MPI_Group_compare(new_group1, new_group2, &result);
	switch(result)
	{
	case MPI_IDENT:
		printf("Groups are identical\n");
		break;
	case MPI_SIMILAR:
		printf("Groups are similar\n");
		break;
	case MPI_UNEQUAL:
		printf("Groups are unequal\n");
		break;
	default:
		printf("Nonesense!\n");
	}
	*/
	// union
	MPI_Group_incl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group1);
	MPI_Group_excl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group2);
	MPI_Group_union(new_group1, new_group2, &new_group3);
	do_something(new_group3);
	//MPI_Group_free(&new_group1);
	//MPI_Group_free(&new_group2);
	
	// intersect
	/*MPI_Group_incl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group1);
	MPI_Group_incl(old_group, sizeof(ranks1) / sizeof(int), ranks1, &new_group2);
	MPI_Group_intersection(new_group1, new_group2, &new_group3);
	do_something(new_group3);
	MPI_Group_free(&new_group1);
	MPI_Group_free(&new_group2);
	*/
	// difference
	MPI_Group_incl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group1);
	MPI_Group_excl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group2);
	MPI_Group_difference(new_group1, new_group2, &new_group3);
	do_something(new_group3);
	/*
	// compare
	MPI_Group_compare(new_group1, new_group2, &result);
	if (result == MPI_UNEQUAL)
		printf("Groups are unequal!\n");
	else if (result == MPI_SIMILAR)
		printf("Groups are similar!\n");
	else
		printf("Groups are equal!\n");
	*/
	//MPI_Group_free(&new_group1);
	//MPI_Group_free(&new_group2);
	
	// range include
	int ranges[1][3] = {{0,2,1}};
	MPI_Group_range_incl(old_group, 1, ranges, &new_group2);
	printf("Done with MPI_Group_range_incl\n");
	do_something(new_group1);
	
	MPI_Group_incl(old_group, sizeof(ranks) / sizeof(int), ranks, &new_group1);
	
	MPI_Comm comm1, comm2;
	MPI_Comm_create(MPI_COMM_WORLD, new_group1, &comm1);
	MPI_Comm_create(MPI_COMM_WORLD, new_group2, &comm2);
	if (MPI_COMM_NULL != comm1 && MPI_COMM_NULL != comm2)
	{
		MPI_Comm_compare(comm1, comm2, &result);
		if (result == MPI_IDENT)
			printf("Comms are identical\n");
		else
		{
			MPI_Group_compare(new_group1, new_group2, &result);
			if (result == MPI_IDENT)
				printf("Comms are not identical but groups are!\n");
			printf("Comms are not identical\n");
		}
	}
	
	/*MPI_Comm_group(MPI_COMM_SELF, &new_group2);
	MPI_Group_compare(new_group1, new_group2, &result);
	if (result == MPI_IDENT)
		printf("Group is identical to MPI_COMM_SELF\n");
	else
		printf("Group is not identical to MPI_COMM_SELF\n");
	*/
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Group_free(&old_group);	
	
	//test_waitall();
	
	MPI_Finalize();

	return 0;
}
