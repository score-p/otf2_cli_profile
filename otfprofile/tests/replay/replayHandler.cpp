#include <sys/time.h>
#include "replayHandler.h"

#define VTRACE 1
#include <iostream>
#include <vt_user.h>
#include <otf.h>
#include <arpa/inet.h>
#include "otfMap.h"
#include "modificationManager.h"
#include "replayLogger.h"

using namespace std;

#define cout\
	cout << "[" << this->_rank << "] "

/* privat members */



void ReplayHandler::printKeys( OTF_KeyValueList *list ){
	
	uint32_t value;
	for ( uint32_t k= 1; k <= 6; k++ ) {
		if ( 0 == OTF_KeyValueList_getUint32( list, k, &value ) ) {
			cout<< "    key " << k << " : value " << value << endl;
		}
	}
}

void ReplayHandler::finishRequest( uint32_t req_id )
{
	//free( _state.replay_databuf[req_id] );
	delete[] _state.replay_databuf[req_id];
	_state.replay_databuf.erase( req_id );
	_state.replay_flags[ req_id ]= REPLAY_REQUEST_FINISHED;
}

uint8_t* ReplayHandler::ntohs_array( uint8_t* array, uint32_t len )
{
	uint32_t i;
	uint16_t* marray = (uint16_t*) array;
	for (i = 0; i < len / 2; ++i)
	{
		marray[i] = ntohs(marray[i]);
	}
	if (i * 2 < len)
		cerr << "Missed some bytes during byte order conversion..." << endl;
	return array;
}

int ReplayHandler::readByteArray( OTF_KeyValueList* list, uint32_t key, uint8_t* buf, uint32_t len )
{
	OTF_KeyValueList_getByteArray(list, key, buf, &len);
	this->ntohs_array(buf, len);
	return len;
	
}

void ReplayHandler::handleMPIIrecv(OTF_KeyValueList *list)
{
	uint8_t ret;
	uint32_t sender;
	uint32_t tag;
	uint32_t requestID;
	uint32_t length;
	uint32_t recvsz;

	//cout<< " # replay MPI_Irecv" << endl;

	ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_SOURCE, &sender );
	assert( 0 == ret );

	ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECVTAG, &tag );
	assert( 0 == ret );

	ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_ID, &requestID );
	assert( 0 == ret );

	ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECV_SIZE, &length );
	assert( 0 == ret );
	recvsz = length;

	MPI_Comm comm = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;
	
	//_state.replay_databuf[requestID]= (uint8_t*) malloc( recvsz );
	_state.replay_databuf[requestID] = new uint8_t[recvsz];
	assert( NULL != _state.replay_databuf[requestID] );
	_state.replay_flags[ requestID ]= REPLAY_REQUEST_ACTIVE;
	
	//cout << "MPI_Irecv from " << sender << " with tag " << tag << " and Request ID " << requestID << endl;
	
	if( MPI_Irecv( _state.replay_databuf[requestID], recvsz, MPI_BYTE, sender, 
		   tag, comm, &(_state.replay_requests[ requestID ]) ) != MPI_SUCCESS )
	{
		cerr << "Warning: MPI_Irecv returned bad status!" << endl;
	}
}

void ReplayHandler::handleMPISendrecv( OTF_KeyValueList* list )
{
	uint32_t src;
	uint32_t dest;
	uint32_t sendtag;
	uint32_t recvtag;
	uint32_t sendsize;
	uint32_t recvsize;
	uint8_t* recvbuf;
	uint8_t* sendbuf;
	int ret;
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SEND_SIZE, &sendsize );
	assert( ret == 0 );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECV_SIZE, &recvsize );
	assert( ret == 0 );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SENDTAG, &sendtag );
	assert( ret == 0 );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECVTAG, &recvtag );
	assert( ret == 0 );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SOURCE, &src );
	assert( ret == 0 );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_DEST, &dest );
	assert( ret == 0 );

	MPI_Comm comm  = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;
	
	recvbuf = (uint8_t*)_buffer_handler.getRecvBuf(recvsize * sizeof(uint8_t));
	sendbuf = (uint8_t*)_buffer_handler.getSendBuf(sendsize * sizeof(uint8_t));
	
	MPI_Sendrecv( sendbuf, sendsize, MPI_BYTE, dest, sendtag,
		recvbuf, recvsize, MPI_BYTE, src, recvtag,
		comm, MPI_STATUS_IGNORE );
}

void ReplayHandler::handleMPIWait( OTF_KeyValueList *list )
{
	uint8_t ret;
	uint32_t requestID;

	//cout<< " # replay MPI_Wait" << endl;

	ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_ID, &requestID );
	assert( 0 == ret );
	
	MPI_Wait( &(_state.replay_requests[ requestID ]), MPI_STATUS_IGNORE );
	this->finishRequest( requestID );
}

void ReplayHandler::handleMPIWaitall( OTF_KeyValueList* list )
{
	int ret;
	uint32_t i = 0;
	uint32_t count = 0;
	uint32_t total_len = 0;
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_COUNT, &count );
	assert( ret == 0 );
	
	uint32_t* req_ids = (uint32_t*)_buffer_handler.getRequestIDBuf( sizeof(uint32_t) * count );
	
	/*while (0 == OTF_KeyValueList_getByteArray( list, REPLAY_KEY_REQUEST_IDS + i, ((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, &len ))
	{
		this->ntohs_array(((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, len);
		i++;
		total_len += len;
	}*/
	/*
	OTF_KeyValueList_getArrayLength(list, REPLAY_KEY_REQUEST_IDS, &total_len);
	cout << "Expecting " << total_len << " bytes to come..." << endl; */
	total_len = readByteArray(list, REPLAY_KEY_REQUEST_IDS, (uint8_t*)req_ids, sizeof(uint32_t) * count);
	if (total_len != count * sizeof(uint32_t))
		cerr << "Did not read enough Byte in MPI_Waitall: " << total_len << " vs " << count * sizeof(uint32_t) << endl;

	//cout << "MPI_Waitall with " << count << " requests" << endl;
	MPI_Request* reqs = (MPI_Request*)_buffer_handler.getRequestBuf( sizeof(MPI_Request) * count );
	for( i = 0; i < count; ++i )
	{
		if (_state.replay_requests.find( req_ids[ i ] ) == _state.replay_requests.end())
		{
			cerr << "Request" << req_ids[i] << "not found!" << endl;
			exit(1);
		}
		reqs[ i ] = _state.replay_requests[req_ids[ i ]];
	}

	MPI_Waitall( count, reqs, MPI_STATUSES_IGNORE );

	for( i = 0; i < count; ++i )
	{
		this->finishRequest( req_ids[ i ] );
	}

}

void ReplayHandler::handleMPIWaitany( OTF_KeyValueList* list )
{
	int ret;
	uint32_t i = 0;
	uint32_t count;
	uint32_t total_len = 0;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_COUNT, &count );
	assert( ret == 0 );
	
	uint32_t* req_ids = (uint32_t*)_buffer_handler.getRequestIDBuf( sizeof(uint32_t) * count );
	
	/*while (0 == OTF_KeyValueList_getByteArray( list, REPLAY_KEY_REQUEST_IDS + i, ((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, &len ))
	{
		this->ntohs_array(((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, len);
		i++;
		total_len += len;
	}*/
	total_len = readByteArray(list, REPLAY_KEY_REQUEST_IDS, (uint8_t*)req_ids, sizeof(uint32_t) * count);
	if (total_len != count * sizeof(uint32_t))
		cerr << "Did not read enough Byte in MPI_Waitany: " << total_len << " vs " << count * sizeof(uint32_t) << endl;
	
	MPI_Request* reqs = (MPI_Request*)_buffer_handler.getRequestBuf( sizeof(MPI_Request) * count );
	for( i = 0; i < count; ++i )
	{
		if( _state.replay_requests.find( req_ids[ i ] ) == _state.replay_requests.end() )
		{
			cerr << "Request "<< req_ids << " not found!" << endl;
			exit(1);
		}
		reqs[ i ] = _state.replay_requests[ req_ids[ i ] ];
	}

	int idx;
	MPI_Waitany( count, reqs, &idx, MPI_STATUS_IGNORE );

	this->finishRequest( req_ids[ idx ] );
	
}

void ReplayHandler::handleMPITestany( OTF_KeyValueList* list )
{
	int ret;
	uint32_t i = 0;
	uint8_t len = 0;
	int flag;
	int index;
	uint32_t count;
	uint32_t total_len = 0;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_COUNT, &count );
	assert( 0 == ret );
	assert( count > 0 );
	
	uint32_t* req_ids = (uint32_t*)_buffer_handler.getRequestIDBuf( sizeof(uint32_t) * count );
	
	/*while (0 == OTF_KeyValueList_getByteArray( list, REPLAY_KEY_REQUEST_IDS + i, ((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, &len ))
	{
		this->ntohs_array(((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, len);
		i++;
		total_len += len;
	}*/
	total_len = readByteArray(list, REPLAY_KEY_REQUEST_IDS, (uint8_t*)req_ids, sizeof(uint32_t) * count);
	if (total_len != count * sizeof(uint32_t))
		cerr << "Did not read enough Byte in MPI_Testany: " << total_len << " vs " << count * sizeof(uint32_t) << endl;

	if( count != (len / sizeof( uint32_t )) )
	{
		cerr << "Inconsistent data encountered in MPI_Testall" << endl;
		exit(1);
	}

	MPI_Request requests[count];
	for( i = 0; i < count; ++i )
	{
		if( _state.replay_requests.find( req_ids[i] ) == _state.replay_requests.end() )
		{
			cerr << "Unknown request in MPI_Testall" << endl;
			exit(1);
		}
		requests[i] = _state.replay_requests[ req_ids[i] ];
	}

	MPI_Testany( count, requests, &index, &flag, MPI_STATUSES_IGNORE );

	if( flag )
	{
		uint32_t req_id = req_ids[index];
		//cout<< "### MPI_Testany: Successfully tested request " << req_id << endl;
		_state.replay_flags[ req_id ] = REPLAY_REQUEST_RECEIVED;
	}

}

void ReplayHandler::handleMPITestall( OTF_KeyValueList* list )
{
	int ret;
	uint32_t i = 0;
	int flag;
	uint32_t count;
	uint32_t total_len = 0;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_COUNT, &count );
	assert( 0 == ret );
	assert( count > 0 );
	
	uint32_t* req_ids = (uint32_t*)_buffer_handler.getRequestIDBuf( sizeof(uint32_t) * count );
	
	/*while (0 == OTF_KeyValueList_getByteArray( list, REPLAY_KEY_REQUEST_IDS + i, ((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, &len ))
	{
		this->ntohs_array(((uint8_t*)req_ids) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, len);
		i++;
		total_len += len;
	}*/
	total_len = readByteArray(list, REPLAY_KEY_REQUEST_IDS, (uint8_t*)req_ids, sizeof(uint32_t) * count);
	if (total_len != count * sizeof(uint32_t))
		cerr << "Did not read enough Byte in MPI_Testall: " << total_len << " vs " << count * sizeof(uint32_t) << endl;

	MPI_Request requests[count];
	for( i = 0; i < count; ++i )
	{
		if( _state.replay_requests.find( req_ids[i] ) == _state.replay_requests.end() )
		{
			cerr << "Unknown request in MPI_Testall" << endl;
			exit(1);
		}
		requests[i] = _state.replay_requests[ req_ids[i] ];
	}

	MPI_Testall( count, requests, &flag, MPI_STATUSES_IGNORE );

	if( flag )
	{
		for( i = 0; i < count; ++i )
		{
			uint32_t req_id = req_ids[i];
			_state.replay_flags[ req_id ] = REPLAY_REQUEST_RECEIVED;
		}
	}

}


void ReplayHandler::handleMPITest( OTF_KeyValueList *list )
{
	uint8_t ret;
	int flag;
	uint32_t requestID;

	//cout<< " # replay MPI_Test" << endl;
	
	ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_ID, &requestID );
	assert( 0 == ret );

	if ( REPLAY_REQUEST_ACTIVE == _state.replay_flags[ requestID ] ) {

		//cout<< " ## MPI_Test replay_flags[i]= 1: active --> call MPI_Test" << endl;

		MPI_Test( &(_state.replay_requests[ requestID ]), &flag, MPI_STATUS_IGNORE );

		if ( flag ) {

			/* sucessfully received */
			//cout<< " ### MPI_Test successful" << endl;
			_state.replay_flags[ requestID ]= REPLAY_REQUEST_RECEIVED;
		} else {
		
			/* still incomplete, go on testing later */
			//cout<< " ### MPI_Test still pending" << endl;
		}


	} else if ( REPLAY_REQUEST_RECEIVED == _state.replay_flags[ requestID ] ) {

		/* already received by former call to Test, ignore */

		//cout<< " ## MPI_Test replay_flags[i]= 3: done --> ignore" << endl;

	} else {
	
		//cout<< " ## MPI_Test replay_flags[i]= " << _state.replay_flags[ requestID ] << " --> error" << endl;
		assert( false );
	}
}

void ReplayHandler::handleMPICommcreate( OTF_KeyValueList *list )
{
	int ret;
	uint32_t src_group;
	uint32_t ncom_id;

	MPI_Comm src_comm =  _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM2, &src_group );
	assert( 0 == ret );
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM3, &ncom_id );
	
	MPI_Group ncom_group = _state.replay_comms[src_group].group;
	
	MPI_Comm new_com;
	MPI_Comm_create( src_comm, ncom_group, &new_com );
	if (new_com != MPI_COMM_NULL)
	{
		cout << "Replay: MPI_Comm_create for id " << ncom_id << endl;;
		_state.replay_comms[ ncom_id ].comm = new_com;
		MPI_Comm_rank(new_com, &(_state.replay_comms[ ncom_id ].rank));
		MPI_Comm_size(new_com, &(_state.replay_comms[ ncom_id ].size));
	}
	
}

void ReplayHandler::handleMPICommdup( OTF_KeyValueList *list )
{
	int ret;
	uint32_t ncomm_id;
	MPI_Comm ncomm;
	
	MPI_Comm src_comm = _state.replay_comms.get_from_list(list, REPLAY_KEY_COMM1).comm;
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM2, &ncomm_id );
	assert(0 == ret);
	
	ret = MPI_Comm_dup(src_comm, &ncomm);
	assert(MPI_SUCCESS == ret);
	assert(MPI_COMM_NULL != ncomm);
	
	_state.replay_comms[ncomm_id].comm = ncomm;
	MPI_Comm_rank(ncomm, &(_state.replay_comms[ ncomm_id ].rank));
	MPI_Comm_size(ncomm, &(_state.replay_comms[ ncomm_id ].size));
	
}

void ReplayHandler::handleMPICommcompare(OTF_KeyValueList *list)
{
	int result;
	MPI_Comm comm1 = _state.replay_comms.get_from_list(list, REPLAY_KEY_COMM1).comm;
	MPI_Comm comm2 = _state.replay_comms.get_from_list(list, REPLAY_KEY_COMM2).comm;
	MPI_Comm_compare(comm1, comm2, &result);
}

void ReplayHandler::handleMPIGroupexcl( OTF_KeyValueList *list )
{
	int ret;
	uint32_t total_len;
	uint32_t num_members;
	
	MPI_Group src_group = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	MPI_Group new_group;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM_SIZE, &num_members );
	assert( 0 == ret );
	
	//int* members = (int*)malloc(num_members * sizeof(int));
	int members[num_members];
	/*i = 0;
	total_len = 0;
	while (0 == OTF_KeyValueList_getByteArray( list, REPLAY_KEY_COMM_MEMBERS + i, ((uint8_t*)members) + i * OTF_KEYVALUE_MAX_ARRAY_LEN, &len ))
	{
		this->ntohs_array(((uint8_t*)members) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, len);
		total_len += len;
		++i;
	}*/
	total_len = readByteArray(list, REPLAY_KEY_COMM_MEMBERS, (uint8_t*)members, sizeof(int) * num_members);
	
	if (total_len != (num_members * sizeof(int)))
	{
		cerr << "Not enough Bytes read in MPI_Group_excl: " << total_len << ", expected " << num_members * sizeof(uint32_t) << endl;
		exit(1);
	}

	uint32_t new_group_id;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM2, &new_group_id );
	assert( ret == 0 );
	assert( new_group_id != 0 );


	MPI_Group_excl( src_group, num_members, members, &new_group );
	_state.replay_comms[new_group_id].group = new_group;
	
	//free(members);
}

void ReplayHandler::handleMPIGrouprangeexcl( OTF_KeyValueList *list )
{
	int ret;
	uint32_t len;

	uint32_t group_size;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM_SIZE, &group_size );
	assert( 0 == ret );

	int members[group_size][3];
	len = readByteArray( list, REPLAY_KEY_COMM_MEMBERS, (uint8_t*)members, sizeof(int) * 3 * group_size);
	assert(len == (sizeof(int) * group_size * 3));

	MPI_Group src_group = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;

	uint32_t group_id;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM2, &group_id );

	MPI_Group new_group;
	MPI_Group_range_excl( src_group, group_size, members, &new_group );

	_state.replay_comms[group_id].group = new_group;
}

void ReplayHandler::handleMPIGroupincl( OTF_KeyValueList *list )
{
	int ret;
	uint32_t num_members = 0;
	uint32_t group_id = 0;
	uint32_t total_len;
	MPI_Group new_group;
	
	MPI_Group src_group = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM2, &group_id );
	assert( ret == 0 );
	//assert( group_id != 0 );
	cout << "Group-id: " << group_id << endl;
	
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM_SIZE, &num_members );
	assert( 0 == ret );
	assert(num_members > 0);
	
	//members = (int*)malloc( num_members * sizeof(int) );
	int members[num_members];
	/*i = 0;
	total_len = 0;
	while (0 == OTF_KeyValueList_getByteArray( list, REPLAY_KEY_COMM_MEMBERS + i, ((uint8_t*)members) + i * OTF_KEYVALUE_MAX_ARRAY_LEN, &len))
	{
		this->ntohs_array(((uint8_t*)members) + i*OTF_KEYVALUE_MAX_ARRAY_LEN, len);
		total_len += len;
		++i;
	}*/
	total_len = readByteArray(list, REPLAY_KEY_COMM_MEMBERS, (uint8_t*)members, num_members * sizeof(int));
	if (total_len != num_members * sizeof(int))
	{
		cerr << "Did not read enough Bytes in MPI_Group_incl: " << total_len << ", expected " << sizeof(int) * num_members << endl;
		exit(1);
	}
	
	MPI_Group_incl( src_group, num_members, members, &new_group );
	
	_state.replay_comms[group_id].group = new_group;
	//free(members);
}

void ReplayHandler::handleMPIGrouprangeincl( OTF_KeyValueList *list )
{
	int ret;
	uint32_t len;
	uint32_t group_id;
	uint32_t group_size;
	MPI_Group new_group;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM_SIZE, &group_size );
	assert( 0 == ret );

	int members[group_size][3];
	len = readByteArray( list, REPLAY_KEY_COMM_MEMBERS, (uint8_t*)members, sizeof(int) * group_size * 3 );
	assert(len == (sizeof(int) * group_size * 3));

	MPI_Group src_group = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM2, &group_id );
	assert(0 == ret);

	MPI_Group_range_incl( src_group, group_size, members, &new_group );

	_state.replay_comms[group_id].group = new_group;
}

void ReplayHandler::handleMPIGroupcompare( OTF_KeyValueList *list )
{
	MPI_Group group1 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;

	MPI_Group group2 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM2 ).group;

	int result;
	MPI_Group_compare( group1, group2, &result );
}

void ReplayHandler::handleMPIGroupintersection( OTF_KeyValueList *list )
{
	int ret;
	MPI_Group group1 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	MPI_Group group2 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM2 ).group;

	MPI_Group new_group;
	uint32_t group_id;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM3, &group_id );
	assert( 0 == ret );

	MPI_Group_intersection( group1, group2, &new_group );

	_state.replay_comms[ group_id ].group = new_group;
}

void ReplayHandler::handleMPIGroupdifference( OTF_KeyValueList *list )
{
	int ret;
	MPI_Group group1 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	MPI_Group group2 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM2 ).group;

	MPI_Group new_group;
	uint32_t group_id;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM3, &group_id );
	assert( 0 == ret );

	MPI_Group_difference( group1, group2, &new_group );

	_state.replay_comms[ group_id ].group = new_group;
}

void ReplayHandler::handleMPIGroupunion( OTF_KeyValueList *list )
{
	int ret;
	MPI_Group group1 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	MPI_Group group2 = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM2 ).group;

	MPI_Group new_group;
	uint32_t group_id;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM3, &group_id );
	assert( 0 == ret );

	MPI_Group_union( group1, group2, &new_group );

	_state.replay_comms[ group_id ].group = new_group;
}


void ReplayHandler::handleMPICommsize( OTF_KeyValueList* list )
{
	MPI_Comm comm = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;
	int tmp;
	MPI_Comm_size( comm, &tmp );
}

void ReplayHandler::handleMPICommrank( OTF_KeyValueList* list )
{
	uint32_t comm_id;
	int ret;
	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_COMM1, &comm_id );
	assert(0 == ret);
	//cout << "MPI_Comm_rank: Comm: " << comm_id << endl;
	MPI_Comm comm = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;
	
	int tmp;
	MPI_Comm_rank( comm, &tmp );
}

void ReplayHandler::handleMPICommfree( OTF_KeyValueList* list )
{
	uint32_t com_id;
	OTF_KeyValueList_getUint32(list, REPLAY_KEY_COMM1, &com_id);
	//cout << "Freeing com " << com_id << endl;
	MPI_Comm comm = (_state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm);
	MPI_Comm_free( &comm );
}

void ReplayHandler::handleMPICommgroup( OTF_KeyValueList* list )
{
	MPI_Comm comm = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;
	MPI_Group group;
	MPI_Comm_group( comm, &group );
}

void ReplayHandler::handleMPIGroupsize( OTF_KeyValueList* list )
{
	MPI_Group group = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	int tmp;
	MPI_Group_size( group, &tmp );
}

void ReplayHandler::handleMPIGrouprank( OTF_KeyValueList* list )
{
	MPI_Group group = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group;
	int tmp;
	MPI_Group_rank( group, &tmp );
}

void ReplayHandler::handleMPIGroupfree( OTF_KeyValueList* list )
{
	MPI_Group* group = &(_state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).group);
	MPI_Group_free( group );
}

void ReplayHandler::handleMPIIprobe( OTF_KeyValueList* list )
{
	uint32_t tag;
	uint32_t source;
	int flag;
	int ret;
	
	MPI_Comm comm = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SOURCE, &source );
	assert( 0 == ret );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECVTAG, &tag );
	assert( 0 == ret );

	MPI_Iprobe( source, tag, comm, &flag, MPI_STATUS_IGNORE );
}

void ReplayHandler::handleMPIProbe( OTF_KeyValueList* list )
{
	uint32_t tag;
	uint32_t source;
	int ret;
	
	MPI_Comm comm = _state.replay_comms.get_from_list( list, REPLAY_KEY_COMM1 ).comm;

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SOURCE, &source );
	assert( 0 == ret );

	ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECVTAG, &tag );
	assert( 0 == ret );

	MPI_Probe( source, tag, comm, MPI_STATUS_IGNORE );
}

/* public members */

ReplayHandler& ReplayHandler::operator=(const ReplayHandler& handler)
{
	if (this == &handler) return *this;
	mpi_map = handler.mpi_map;
	key_map = handler.key_map;
	_state = handler._state;
	_function_map = handler._function_map;
	_function_stack = handler._function_stack;
	_buffer_handler = handler._buffer_handler;
	timer = handler.timer;
	logger = handler.logger;
	
	return *this;
}

ReplayHandler::ReplayHandler(const ReplayHandler& handler) : 
		_rank(handler._rank), 
		_size(handler._size)
{
	logger.set_timer(&timer);
	logger.set_rank(_rank);
}

ReplayHandler::ReplayHandler(){
	MPI_Comm_rank( MPI_COMM_WORLD, &_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &_size );
	logger.set_timer(&timer);
	logger.set_rank(_rank);
}

ReplayHandler::ReplayHandler(const int rank ){
	this->_rank = rank;
	MPI_Comm_size( MPI_COMM_WORLD, &_size );
	logger.set_timer(&timer);
	logger.set_rank(_rank);
	
}

ReplayHandler::ReplayHandler(const int rank, const int size ){
	this->_rank = rank;
	this->_size = size;
	logger.set_timer(&timer);
	logger.set_rank(_rank);
}

ReplayHandler::~ReplayHandler()
{
	set<uint32_t>::iterator it;
	if ((it = mpi_map.begin()) != mpi_map.end())
	{
		cout << "List of unhandled MPI functions: " << endl;
		for (; it != mpi_map.end(); it++)
			cout << "\t" << _function_map[*it];
	}
}

int ReplayHandler::handleDefFunction(const uint32_t func, const char *name,
				     const uint32_t funcGroup, const uint32_t source) {
	//cout<< "DefFunction " << func << ": " << name << endl;
	this->_function_map[func] = name;
	return OTF_RETURN_OK;
}

int ReplayHandler::handleDefTimerResolution(const uint64_t ticksPerSecond) {

	this->timer.set_timer_resolution(ticksPerSecond);

	return OTF_RETURN_OK;
}
 
int ReplayHandler::handleDefProcessGroup(const uint32_t procGroup, 
					 const char *name, const uint32_t numberOfProcs, const uint32_t *procs ) {

	std::string comm_world= "MPI_COMM_WORLD";
	std::string comm_self = "MPI_COMM_SELF";
	//uint32_t cid = procGroup - ((int)1E9);
	uint32_t cid = procGroup;
	//cout << "Communicator definition for comm " << name << "(" << procGroup << ") with " << numberOfProcs << " processes" << endl;
	if ( comm_world == name ) {
		
		//cerr << "            MPI_COMM_WORLD == " << cid << endl;
		//_state.replay_comms[ cid ] = comm_t(MPI_COMM_WORLD);
		_state.replay_comms[cid].comm = MPI_COMM_WORLD;
		MPI_Comm_group(MPI_COMM_WORLD, &(_state.replay_comms[cid].group));
		MPI_Comm_rank(MPI_COMM_WORLD, &(_state.replay_comms[cid].rank));
		MPI_Comm_size(MPI_COMM_WORLD, &(_state.replay_comms[cid].size));
	} else if (comm_self == string(name).substr(0,13))
	{
		_state.replay_comms[cid].comm = MPI_COMM_SELF;
		MPI_Comm_group(MPI_COMM_SELF, &(_state.replay_comms[cid].group));
		MPI_Comm_rank(MPI_COMM_SELF, &(_state.replay_comms[cid].rank));
		MPI_Comm_size(MPI_COMM_SELF, &(_state.replay_comms[cid].size));
	}

	return OTF_RETURN_OK;
}


int ReplayHandler::handleDefKeyValue(const uint32_t key, const char *name, const OTF_KeyValueList *list ) {
	
	//cout<< "    key value def: " << key << " : \"" << name << "\"" << endl;

	this->key_map[key] = name;

	return OTF_RETURN_OK;
}


int ReplayHandler::handleEnter (const uint64_t time, const uint32_t function, const uint32_t process,
				const uint32_t source, OTF_KeyValueList *list )
{
	string function_name;
	//uint32_t fid;

	/* if stack is not empty, get previous function id from stack and resolve the function name */
	/*if( ! this->_function_stack.empty() ) {
		fid = this->_function_stack.top();
		function_name = this->_function_map[fid];
		
	} else {
		function_name = "MPI_";
	}*/

	
	timer.user_function( time );

	/* add the current function id to the top of the stack */
	this->_function_stack.push(function);

	/* save the current timestamp */
	//this->_sim_timestamp = time;

	/* get the current function name */
	function_name = this->_function_map[function];

	/* decide what to do next depending on the function name */
	if( function_name.substr(0,4) == "MPI_" ) {
		
		//cout<< " Enter " << "\"" << function_name << "\"" << endl;
	
		if( function_name == "MPI_Send" ) {
			/* all right, handled in send hanlder below */
		} else if( function_name == "MPI_Isend" ) {
			/* all right, handled in send hanlder below */
		} else if ( function_name == "MPI_Recv" ) {
			/* all right, handled below in receive handler  */
		} else if ( function_name == "MPI_Irecv" ) {
			this->handleMPIIrecv( list );
		} else if ( function_name == "MPI_Sendrecv") {
			this->handleMPISendrecv( list );
		} else if ( function_name == "MPI_Gather" ) {
			/* all right */
		} else if ( function_name == "MPI_Allgather" ) {
			/* all right */
		} else if ( function_name == "MPI_Allgatherv" ) {
			/* all right */
		} else if ( function_name == "MPI_Gatherv" ) {
			/* all right */
		} else if ( function_name == "MPI_Scatter" ) {
			/* all right */
		} else if ( function_name == "MPI_Scatterv" ) {
			/* all right */
		} else if ( function_name == "MPI_Reduce" ) {
			/* all right */
		} else if ( function_name == "MPI_Allreduce" ) {
			/* all right */
		} else if ( function_name == "MPI_Barrier" ) {
			/* all right */
		} else if ( function_name == "MPI_Bcast" ) {
			/* all right */
		} else if ( function_name == "MPI_Init" ) {
			/* not done yet, but is there anything to do? */
		} else if ( function_name == "MPI_Finalize" ) {
			/* not done yet, but is there anything to do? */
		} else if ( function_name == "MPI_Type_hvector" ) {
			/* not done yet, but is there anything to do? */
		} else if ( function_name == "MPI_Type_struct" ) {
			/* not done yet, but is there anything to do? */
		} else if ( function_name == "MPI_Type_free" ) {
			/* not done yet, but is there anything to do? */
		} else if ( function_name == "MPI_Type_commit" ) {
			/* not done yet, but is there anything to do? */
		} else if ( function_name == "MPI_Wait" ) {
			this->handleMPIWait( list );
		} else if ( function_name == "MPI_Waitall" ) {
			this->handleMPIWaitall( list );
		} else if ( function_name == "MPI_Test" ) {
			this->handleMPITest( list );
		} else if ( function_name == "MPI_Testall" ){
			this->handleMPITestall( list );
		} else if ( function_name == "MPI_Testany" ){
			this->handleMPITestany( list );
		} else if ( function_name == "MPI_Probe" ){
			this->handleMPIProbe( list );
		} else if ( function_name == "MPI_Iprobe" ){

			this->handleMPIIprobe( list );

		} else if ( function_name == "MPI_Wtime" ) {
			
			MPI_Wtime();
			
		} else if ( function_name.substr( 0, 8 ) == "MPI_Comm" ){
			string comm_op = function_name.substr( 9, function_name.length() - 9 );
			
			if ( comm_op == "rank") {

				this->handleMPICommrank( list );
	
			} else if ( comm_op == "size" ) {
	
				this->handleMPICommsize( list );
			}
			else if ( comm_op == "create" ){
				// handled at leave
				//this->handleMPICommcreate( list );
	
			} else if ( comm_op == "free" ){
				
				this->handleMPICommfree( list );
				
			} else if ( comm_op == "group" ){
				
				this->handleMPICommgroup( list );
				
			} else if ( comm_op == "compare" ){ 
			 	this->handleMPICommcompare(list);
			} else {
				//cerr << "Unknown MPI_Comm operation: " << comm_op << endl;
				logger.error("Unknown MPI_Comm operation: " + comm_op);
				exit( 1 );
			}
			
		} else if ( function_name.substr(0, 9) == "MPI_Group" ){
			string group_op = function_name.substr( 10, function_name.length() - 10 );
			cout << "Group OP: " << group_op << "(" << function << ")" << endl;
			if ( group_op == "rank" ){
				
				this->handleMPIGrouprank( list ); 
				
			} else if ( group_op == "size" ){
				
				this->handleMPIGroupsize( list );
				
			} else if ( group_op == "free" ){
				
				this->handleMPIGroupfree( list );
				
			} else if ( group_op == "intersection" ){
				// handled at leave
				//this->handleMPIGroupintersection( list );
				
			} else if ( group_op == "difference" ){
				// handled at leave
				//this->handleMPIGroupdifference( list );
				
			} else if ( group_op == "union" ){
				// handled at leave
				//this->handleMPIGroupunion( list );
				
			} else if ( group_op == "range_incl" ){
				// handled at leave
				//this->handleMPIGrouprangeincl( list );
				
			} else if ( group_op == "range_excl" ){
				// handled at leave
				//this->handleMPIGrouprangeexcl( list );
				
			} else if ( group_op == "incl" ){
				// handled at leave
				//this->handleMPIGroupincl( list );
				
			} else if ( group_op == "excl" ){
				// handled at leave
				//this->handleMPIGroupexcl( list );
	
			} else if ( group_op == "compare" ){
	
				this->handleMPIGroupcompare( list );
			} else {
				cerr << "Unknown MPI_Group operation " << group_op << "!" << endl;
				exit( 1 );
			}
		} else {

			if( this->mpi_map.end() == this->mpi_map.find(function) ) {
				this->mpi_map.insert(function);
				cerr << this->_rank << ": " << function_name << " is not processed yet" << endl;
			}
		}
		//cout << "Leaving " << function_name << endl;
	} else if ( function_name != "sync time" )
	{
		VT_USER_START( function_name.c_str() );
	}

	return OTF_RETURN_OK;
}


int ReplayHandler::handleLeave(const uint64_t time, const uint32_t function, const uint32_t process,
			       const uint32_t source, OTF_KeyValueList *list) 
{
	string function_name;
	uint32_t fid;

	/* get previous function id from stack and remove it afterwards */
	fid = this->_function_stack.top();
	this->_function_stack.pop();

	/* get function name for function id */
	function_name = this->_function_map[fid];

	/* exit the function if it was a user function*/
	if( function_name.substr(0,4) != "MPI_" && function_name != "sync time" ) {
		VT_USER_END( function_name.c_str() );
	}
	
	/* handle some of the MPI_Group and MPI_Comm calls */
	if (function_name == "MPI_Group_excl")
	{
		this->handleMPIGroupexcl(list);
	} else if (function_name == "MPI_Group_incl")
	{
		this->handleMPIGroupincl(list);
	} else if (function_name == "MPI_Group_difference")
	{
		this->handleMPIGroupdifference(list);
	} else if (function_name == "MPI_Group_intersection")
	{
		this->handleMPIGroupintersection(list);
	} else if (function_name == "MPI_Group_union")
	{
		this->handleMPIGroupunion(list);
	} else if (function_name == "MPI_Group_range_incl")
	{
		this->handleMPIGrouprangeincl(list);
	} else if (function_name == "MPI_Group_range_excl")
	{
		this->handleMPIGrouprangeexcl(list);
	} else if (function_name == "MPI_Comm_create")
	{
		this->handleMPICommcreate(list);
	} else if (function_name == "MPI_Comm_dup")
	{
		this->handleMPICommdup(list);
	}
		   
	timer.update_timer( time );

	////cout<< " Leave " << "\"" << function_name << "\"" << endl;
	return OTF_RETURN_OK;
}


int ReplayHandler::handleSendMsg(const uint64_t time, const uint32_t sender, const uint32_t receiver,
				 const uint32_t group, const uint32_t tag, const uint32_t length, const uint32_t source, OTF_KeyValueList *list ) 
{
	uint32_t fid;
	string function_name;
	uint32_t cid = group;
	uint32_t sendsz;

	uint8_t ret;

	fid = this->_function_stack.top();
	function_name = this->_function_map[fid];

	//this->printKeys( list );

	MPI_Comm comm = _state.replay_comms.get_by_id( cid ).comm;
	
	sendsz = length;

	if( "MPI_Send" == function_name ) {

		//cout<< " # replay MPI_Send" << endl;

		uint8_t* buffer = (uint8_t*)_buffer_handler.getSendBuf( sendsz );

		MPI_Send( buffer, sendsz, MPI_BYTE, receiver - 1, tag, comm );
		
		//free(buffer);

	} else if ( "MPI_Isend" == function_name ) {

		uint32_t requestID;

		//cout<< " # replay MPI_Isend" << endl;
		ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_ID, &requestID );
		assert( 0 == ret );

		//_state.replay_databuf[requestID]= (uint8_t*) malloc( sendsz );
		_state.replay_databuf[requestID] = new uint8_t[sendsz];
		assert( NULL != _state.replay_databuf[requestID] );

		_state.replay_flags[ requestID ]= REPLAY_REQUEST_ACTIVE;

		MPI_Isend( _state.replay_databuf[requestID], sendsz, MPI_BYTE, receiver -1, 
			tag, comm, &(_state.replay_requests[ requestID ]) );

	} else if( "MPI_Sendrecv" == function_name ) {
		/* ingore, handled separately */
	} else {

		if( this->mpi_map.end() == this->mpi_map.find(fid) ) {
			this->mpi_map.insert(fid);
			cerr << this->_rank << ": SendMsg: " << function_name << " is not processed yet" << endl;
		}

	}

	/* update timer */
	timer.update_timer( time );
	
	return OTF_RETURN_OK;
}

int ReplayHandler::handleRecvMsg (const uint64_t time, const uint32_t recvProc, const uint32_t sendProc,
				  const uint32_t group, const uint32_t type, const uint32_t length, const uint32_t source, OTF_KeyValueList *list)
{
	uint32_t fid;
	string function_name;
	uint32_t cid = group;
	uint32_t recvsz;

	uint8_t ret;

	fid = this->_function_stack.top();
	function_name = this->_function_map[fid];
	
	recvsz = length;
	
	if( function_name == "MPI_Recv" ) {

		//cout<< " # replay MPI_Recv" << endl;
		uint8_t* buffer = (uint8_t*) _buffer_handler.getRecvBuf( recvsz );
		MPI_Comm comm = _state.replay_comms.get_by_id( cid ).comm;

		MPI_Recv( buffer, recvsz, MPI_BYTE, sendProc - 1, type, comm, MPI_STATUS_IGNORE);
		//free(buffer);

	} else if( function_name == "MPI_Test" ) {

		//cout<< " # replay MPI_Irecv in MPI_Test" << endl;

		/* this is the point where the original completion of the non-blocking operation 
		was recorded --> do MPI_Test here in a loop 
		(although it could have been another call --> does this matter?)*/

		int flag;
		uint32_t requestID;

		ret= OTF_KeyValueList_getUint32( list, REPLAY_KEY_REQUEST_ID, &requestID );
		assert( 0 == ret );

		if ( REPLAY_REQUEST_ACTIVE == _state.replay_flags[ requestID ] ) {

			//cout<< " ## MPI_Irecv + MPI_Test replay_flags[i]= 1: active --> call MPI_Test" << endl;

			flag= 0;

			while ( 0 == flag ) {

				MPI_Test( &(_state.replay_requests[ requestID ]), &flag, MPI_STATUS_IGNORE );

				if ( flag ) {

				/* sucessfully received */
				//cout<< " ### MPI_Irecv + MPI_Test successful" << endl;
				_state.replay_flags[ requestID ]= REPLAY_REQUEST_RECEIVED;

				} else {

					/* still incomplete, go on testing later */
					//cout<< " ### MPI_Test still pending" << endl;
					sleep(1);
				}
			}

		} else if ( REPLAY_REQUEST_RECEIVED == _state.replay_flags[ requestID ] ) {

			/* already received by former call to Test, ignore */

			//cout<< " ## MPI_Test replay_flags[i]= 3: done --> ignore" << endl;

		} else {

			//cout<< " ## MPI_Test replay_flags[i]= " << _state.replay_flags[ requestID ] <<  " --> error" << endl;
			assert( false );
		}

		this->finishRequest( requestID );
		
	} else if ( function_name == "MPI_Wait" ) {
		/* nothing to do here, message was received before */
	} else if ( function_name == "MPI_Waitall" ) {
		/* nothing to do here, message was received before */
	} else if ( function_name == "MPI_Sendrecv" ) {
		/* nothing to do here, handled on enter */
	} else {
		if( this->mpi_map.end() == this->mpi_map.find(fid) ) {
		this->mpi_map.insert(fid);
		cerr << this->_rank << ": RecvMsg: " << function_name << " is not processed yet" << endl;
	}

	}
	
	/* update timer */
	timer.update_timer( time );
	
	return OTF_RETURN_OK;
}


int ReplayHandler::handleBeginCollectiveOperation(const uint64_t time, const uint32_t process,
		const uint32_t collOp, const uint64_t matchingId, const uint32_t procGroup,
  		const uint32_t rootProc, const uint64_t sent, const uint64_t received,
  		const uint32_t scltoken, OTF_KeyValueList *list)
{
	uint8_t *send_buffer;
	uint8_t *recv_buffer;
	uint32_t sent_size = sent;
	uint32_t recv_size = received;
	int ret;
	uint32_t root;
	
	uint32_t fid = this->_function_stack.top();
	string function_name = this->_function_map[fid];
//	cout << "Entering handleBeginCollectiveOperation to handle " << function_name << endl;

	uint32_t cid = procGroup;
	MPI_Comm comm = _state.replay_comms.get_by_id( cid ).comm;
	uint32_t com_size = _state.replay_comms[ cid ].size;
	uint32_t com_rank = _state.replay_comms[ cid ].rank;

	if( function_name == "MPI_Gather" ) {
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_ROOT, &root );
		assert(0 == ret);
		if (root != com_rank)
			recv_size = 1;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );

		MPI_Gather(send_buffer, sent_size, MPI_BYTE, recv_buffer, recv_size, MPI_BYTE, root, comm);

	} else if( function_name == "MPI_Gatherv" ){
		int i = 0;
		uint32_t type_size;
		uint32_t total_len = 0;
		
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_ROOT, &root );
		assert(0 == ret);
		
		if (root != com_rank)
			recv_size = 1;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		
		uint32_t* counts = (uint32_t*)_buffer_handler.getCountsBuf( com_size * sizeof(uint32_t) );
		int* icounts = (int*)_buffer_handler.getIcountsBuf( com_size * sizeof(int) );
		
		uint32_t* displs = (uint32_t*)_buffer_handler.getDisplsBuf( com_size * sizeof(uint32_t) );
		int* idispls = (int*)_buffer_handler.getIdisplsBuf( com_size * sizeof(int) );
		
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECVTYPE_SIZE, &type_size );
		assert( 0 == ret );
		
		total_len = readByteArray(list, REPLAY_KEY_COL_RECVDISPLS, (uint8_t*)displs, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte in MPI_Gatherv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;

		i = 0;
		total_len = 0;
		total_len = readByteArray(list, REPLAY_KEY_COL_RECVCOUNTS, (uint8_t*)counts, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte in MPI_Gatherv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;

		for( i = 0; i < _size; ++i )
		{
			icounts[ i ] = type_size * counts[ i ];
			idispls[ i ] = type_size * displs[ i ];
		}

		MPI_Gatherv( send_buffer, sent_size, MPI_BYTE, recv_buffer, icounts, idispls, MPI_BYTE, root, comm );
		
			
	} else if ( function_name == "MPI_Allgather" ){
		sent_size /=  com_size;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		MPI_Allgather( send_buffer, sent_size, MPI_BYTE, recv_buffer, recv_size / com_size, MPI_BYTE, comm );
		
	} else if ( function_name == "MPI_Allgatherv" ) {
		uint32_t i;
		uint32_t type_size;
		uint32_t total_len;
		
		sent_size /= com_size;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		
		uint32_t* counts = (uint32_t*)_buffer_handler.getCountsBuf( com_size * sizeof(uint32_t) );
		int* icounts = (int*)_buffer_handler.getIcountsBuf( com_size * sizeof(int) );
		
		uint32_t* displs = (uint32_t*)_buffer_handler.getDisplsBuf( com_size * sizeof(uint32_t) );
		int* idispls = (int*)_buffer_handler.getIdisplsBuf( com_size * sizeof(int) );
		
		total_len = readByteArray(list, REPLAY_KEY_COL_RECVDISPLS, (uint8_t*)displs, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte for " << REPLAY_KEY_COL_RECVDISPLS << " in MPI_Allgatherv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;

		total_len = readByteArray(list, REPLAY_KEY_COL_RECVCOUNTS, (uint8_t*)counts, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte for " << REPLAY_KEY_COL_RECVCOUNTS << "  in MPI_Allgatherv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;

		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_RECVTYPE_SIZE, &type_size );
		assert( 0 == ret );

		for( i = 0; i < com_size; ++i )
		{
			icounts[ i ] = type_size * counts[ i ];
			idispls[ i ] = type_size * displs[ i ];
		}
		MPI_Allgatherv( send_buffer, sent_size, MPI_BYTE, recv_buffer, icounts, idispls, MPI_BYTE, comm );

	} else if ( function_name == "MPI_Scatter" ) {
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_ROOT, &root );
		assert(0 == ret);
		
		sent_size = (com_rank == root) ? (sent_size / com_size) : 0;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		
		MPI_Scatter(send_buffer, sent_size, MPI_BYTE, recv_buffer, recv_size, MPI_BYTE, root, comm);

	} else if ( function_name == "MPI_Scatterv" ){
		uint i = 0;
		uint32_t type_size;
		uint32_t total_len;
		
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_ROOT, &root );
		assert(0 == ret);
		
		sent_size = (com_rank == root) ? (sent_size / com_size) : 0;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		
		
		uint32_t* counts = (uint32_t*)_buffer_handler.getCountsBuf( com_size * sizeof(uint32_t) );
		int* icounts = (int*)_buffer_handler.getIcountsBuf( com_size * sizeof(int) );
		
		uint32_t* displs = (uint32_t*)_buffer_handler.getDisplsBuf( com_size * sizeof(uint32_t) );
		int* idispls = (int*)_buffer_handler.getIdisplsBuf( com_size * sizeof(int) );

		total_len = readByteArray(list, REPLAY_KEY_COL_SENDDISPLS, (uint8_t*)displs, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte in MPI_Scatterv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;
		
		total_len = readByteArray(list, REPLAY_KEY_COL_SENDCOUNTS, (uint8_t*)counts, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte in MPI_Scatterv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;
		
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SENDTYPE_SIZE, &type_size );
		assert( 0 == ret );
		assert( 0 < type_size );
		
		for( i = 0; i < com_size; ++i )
		{
			icounts[ i ] = type_size * counts[ i ];
			idispls[ i ] = type_size * displs[ i ];
		}
		
		MPI_Scatterv( send_buffer, icounts, idispls, MPI_BYTE, recv_buffer, recv_size, MPI_BYTE, root, comm );
		
	} else if (function_name == "MPI_Reduce_scatter")
	{ 	
		uint32_t total_len, i;
		uint32_t type_size;
		uint32_t* counts = (uint32_t*)_buffer_handler.getCountsBuf( com_size * sizeof(uint32_t) );
		int* icounts = (int*)_buffer_handler.getIcountsBuf( com_size * sizeof(int) );
		total_len = readByteArray(list, REPLAY_KEY_COL_RECVCOUNTS, (uint8_t*)counts, com_size * sizeof(uint32_t));
		if (total_len != com_size * sizeof(uint32_t))
			cerr << "Did not read enough Byte in MPI_Scatterv: " << total_len << " vs " << com_size * sizeof(uint32_t) << endl;
		
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_SENDTYPE_SIZE, &type_size );
		assert( 0 == ret );
		assert( 0 < type_size );
		
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		
		for( i = 0; i < com_size; ++i )
		{
			icounts[ i ] = type_size * counts[ i ];
		}
		MPI_Reduce_scatter(send_buffer, recv_buffer, icounts, MPI_BYTE, MPI_BOR, comm);
		
	} else if ( function_name == "MPI_Barrier" ) {
		MPI_Barrier(comm);
	} else if ( function_name == "MPI_Bcast" ) {
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_ROOT, &root );
		assert(0 == ret);
		sent_size = (root == com_rank && com_size > 1) ? sent_size / (com_size-1) : recv_size;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		//cout << "MPI_Bcast with " << sent_size << " ( " << sent << " / " << com_size << " ) Bytes from root " << root << endl;
		MPI_Bcast( send_buffer, sent_size, MPI_CHAR, root, comm );
	} else if ( function_name == "MPI_Reduce" ) {
		recv_size = 8 * sizeof(uint8_t);
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		ret = OTF_KeyValueList_getUint32( list, REPLAY_KEY_ROOT, &root );
		assert(0 == ret);
		MPI_Reduce( send_buffer, recv_buffer, sent_size, MPI_BYTE, MPI_BOR, root, comm );
	} else if ( function_name == "MPI_Allreduce" ) {
		sent_size /= com_size;
		recv_size = sizeof(uint8_t);
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		MPI_Allreduce( send_buffer, recv_buffer, sent_size, MPI_BYTE, MPI_BOR, comm );
	} else if (function_name == "MPI_Alltoall"){
		sent_size /= com_size;
		recv_size /= com_size;
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf( (sent_size) * sizeof(uint8_t) );
		recv_buffer = (uint8_t*)_buffer_handler.getRecvBuf( (recv_size) * sizeof(uint8_t) );
		MPI_Alltoall(send_buffer, sent_size, MPI_BYTE, recv_buffer, recv_size, MPI_BYTE, comm);
	} else if (function_name == "MPI_Scan"){
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf(recv_size);
		recv_buffer = (uint8_t*)_buffer_handler.getSendBuf(recv_size);
		MPI_Scan(send_buffer, recv_buffer, recv_size, MPI_BYTE, MPI_BOR, comm);	
	} else if (function_name == "MPI_Exscan"){
		send_buffer = (uint8_t*)_buffer_handler.getSendBuf(recv_size);
		recv_buffer = (uint8_t*)_buffer_handler.getSendBuf(recv_size);
		MPI_Exscan(send_buffer, recv_buffer, recv_size, MPI_BYTE, MPI_BOR, comm);	
	}  else {
		if( this->mpi_map.end() == this->mpi_map.find(fid) ) {
			this->mpi_map.insert(fid);
			cerr << this->_rank << ": Collop: " << function_name << " is not processed yet" << endl;
		}
	}
	
	/* update timer */
	timer.update_timer( time );

	return OTF_RETURN_OK;
}

int ReplayHandler::handleEndCollectiveOperation(const uint64_t time, const uint32_t process, const uint64_t matchingId, OTF_KeyValueList *list )
{
	// nothing to do here, yet
	return OTF_RETURN_OK;
}
