//
//  OTFReader.cpp
//  
//
//  Created by Jens Doleschal on 18/06/14.
//
//

#include <sstream>

#include <otf.h>
#include <otfaux.h>

#include "OTFReader.h"

static uint64_t  systemTreeNodeId;

OTFReader::OTFReader()
{
    manager = NULL;
    reader  = NULL;
    systemTreeNodeId = 0;
}

OTFReader::~OTFReader()
{
    close();
}

bool
OTFReader::open( AllData& alldata )
{
    bool error= false;
    
    manager=
    OTF_FileManager_open( alldata.params.max_file_handles );

    if ( NULL == manager ) {

        error= true;

    }

    assert( manager );
    
    reader=
    OTF_Reader_open( alldata.params.input_file_prefix.c_str(), manager );

    if ( NULL == reader ) {

        error= true;

    }

    assert( reader );
    
    return !error;

}

void
OTFReader::close()
{

    /* close OTF file manager and reader */
    if ( NULL != reader ) {

        OTF_Reader_close( reader );

    }

    if ( NULL != manager ) {

        OTF_FileManager_close( manager );

    }
    
}

/* definition record handler functions */

int OTFReader::handle_def_creator( void* fha, uint32_t stream, const char* creator,
                              OTF_KeyValueList* kvlist ) {
    

    AllData* alldata= (AllData*) fha;
   
    alldata->creator= creator;
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_version( void* fha, uint32_t stream,
                              uint8_t major, uint8_t minor, uint8_t sub, const char* suffix,
                              OTF_KeyValueList* kvlist ) {
    

    AllData* alldata= (AllData*) fha;
    
    ostringstream version;
    version << (int)major << "." << (int)minor;

    if ( sub > 0 ) {
        
        version << "." << (int)sub;
        
    }

    version << suffix;
    
    alldata->version= version.str();
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_comment( void* fha, uint32_t stream, const char* comment,
                              OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    /* add new-line between each comment record */
    if ( 0 < alldata->comments.length() ) {
        
        alldata->comments+= "\n";
        
    }
    
    /* wrap lines after 80 characters */
    
    const string::size_type LINE_WRAP= 80;
    
    string tmp= comment;
    
    do {
        
        if ( tmp.length() <= LINE_WRAP ) {
            
            alldata->comments+= tmp;
            break;
            
        } else {
            
            string::size_type next_wrap=
            tmp.find_last_of( " .!?:;,", LINE_WRAP -1 );
            next_wrap= ( string::npos == next_wrap ) ? LINE_WRAP : next_wrap +1;
            
            alldata->comments+= tmp.substr( 0, next_wrap ) + '\n';
            tmp= tmp.substr( next_wrap );
            
        }
        
    } while( 0 != tmp.length() );
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_timerres( void* fha, uint32_t stream,
                               uint64_t ticksPerSecond, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    alldata->timerResolution= ticksPerSecond;
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_process( void* fha, uint32_t stream, uint32_t process,
                              const char* name, uint32_t parent, OTF_KeyValueList* kvlist ) {

    AllData* alldata = (AllData*) fha;

    
    alldata->allProcesses.insert( Process( process, 0 ) );
    alldata->processIdNameMap[process] = name;

    if ( strncmp (name, "MPI",3) != 0 && strncmp (name, "All",3) != 0 ) {
        
        uint64_t nodeid= systemTreeNodeId++;
        

                
                map< uint64_t, string >::iterator iter_namemap;
                
                if ( (iter_namemap=alldata->processIdNameMap.find(process)) != alldata->processIdNameMap.end()) {
                    
                    string processname = iter_namemap->second;
                    
                    if ( 0 == parent ) {
                        
                        // create logical location group and location 
                        uint64_t locationgroupid= systemTreeNodeId++;
                        uint64_t locationid     = systemTreeNodeId++;

                        alldata->systemTreeIdToSystemTreeNodeMap[locationgroupid] = SystemTreeNode(processname,SYSTEMTREE_LOCATION_GROUP,0,parent);
                        alldata->systemTreeClassToNodeIdsMap[SYSTEMTREE_LOCATION_GROUP].insert(locationgroupid);
                        alldata->systemTreeIdToSystemTreeNodeMap[nodeid].children.insert(locationgroupid);
                        alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_LOCATION_GROUP, process) ] = locationgroupid;
                        
                        
                        alldata->systemTreeIdToSystemTreeNodeMap[locationid]= SystemTreeNode("Masterthread", SYSTEMTREE_LOCATION,process,locationgroupid);
                        alldata->systemTreeClassToNodeIdsMap[SYSTEMTREE_LOCATION].insert(locationid);
                        alldata->systemTreeIdToSystemTreeNodeMap[locationgroupid].children.insert(locationid);
                        
                        alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_LOCATION, process) ] = locationid;
                        
                    } else {
                        
                        // create location 
                        uint64_t locationid= systemTreeNodeId++;
                        
                        // look up for location group using the parent information
                        map< Pair, uint64_t>::iterator iter_idmapping;
                        
                        if ( (iter_idmapping=alldata->traceIdToSystemTreeId.find(Pair(SYSTEMTREE_LOCATION_GROUP,parent))) != alldata->traceIdToSystemTreeId.end() ) {
                            
                            alldata->systemTreeIdToSystemTreeNodeMap[locationid]= SystemTreeNode(processname, SYSTEMTREE_LOCATION,process,iter_idmapping->second);
                            alldata->systemTreeClassToNodeIdsMap[SYSTEMTREE_LOCATION].insert(locationid);
                            alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children.insert(locationid);
                            
                            alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_LOCATION, process) ] = locationid;

                        }else {
                            /* unknown location group */
                        }

                    }
                    
                } else {
                    /* no name available for given process id */
                }

            } else {
                /* unknown location */
            }
                


    return OTF_RETURN_OK;

}


int OTFReader::handle_def_functiongroup( void * fha, uint32_t stream, uint32_t funcGroup,
                                    const char * name, OTF_KeyValueList * kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    alldata->functionGroupIdNameMap[funcGroup]= name;

    return OTF_RETURN_OK;

}


int OTFReader::handle_def_function( void * fha, uint32_t stream, uint32_t function,
                               const char * name, uint32_t funcGroup, uint32_t source,
                               OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    alldata->functionIdNameMap[function]= name;
    alldata->functionIdToGroupIdMap[function]=funcGroup;

	alldata->functionDetails.insert(pair<uint64_t, functionDetail >( (uint64_t)function, functionDetail(source, name) ) );
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_collop( void* fha, uint32_t stream, uint32_t collOp,
                             const char* name, uint32_t type, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    alldata->collectiveOperationsToClasses[collOp] = type;
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_counter( void* fha, uint32_t stream, uint32_t counter,
                              const char* name, uint32_t properties, uint32_t counterGroup,
                              const char* unit, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    if ( OTF_COUNTER_TYPE_ACC == ( properties & OTF_COUNTER_TYPE_BITS ) ) {
        
        alldata->countersOfInterest.insert( counter );
        
        alldata->counterIdNameMap[counter]= name;
        
    }
    
    return OTF_RETURN_OK;

}


int OTFReader::handle_def_keyvalue( void* fha, uint32_t stream, uint32_t key,
                               OTF_Type type, const char* name, const char* description,
                               OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    if ( 0 == strcmp( name, OTFAUX_KEYVALUE_TUD_P2P_RECEIVED_TIME_NAME ) ) {
        
        alldata->recvTimeKey= key;
        
    }
    
    return OTF_RETURN_OK;

}

/* event record handler functions */

int OTFReader::handle_enter( void* fha, uint64_t time, uint32_t function,
                        uint32_t process, uint32_t source, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    list< StackType >& stack = alldata->stackPerProcess[ process ];
    
    uint64_t res;

    if( stack.empty() ){

        res = alldata->callPathTree.insertNode( CallPathTreeNode( function, 0 ), process );

    } else {

	    res = alldata->callPathTree.insertNode( CallPathTreeNode( function, stack.back().cpt_nid ), process );

    }


    stack.push_back( StackType( function, time, res ) );

    return OTF_RETURN_OK;

}

int OTFReader::handle_leave( void* fha, uint64_t time, uint32_t function,
                        uint32_t process, uint32_t source, OTF_KeyValueList* kvlist ) {

    AllData* alldata= (AllData*) fha;
    
    list<StackType>& stack= alldata->stackPerProcess[ process ];
    assert( !stack.empty() );
    
    StackType& top= stack.back();
    list<StackType>::reverse_iterator parent_it= ++stack.rbegin();
    
    uint64_t func= top.fid;
    uint64_t incl= time - top.timestamp;
    uint64_t excl= incl - top.childDuration;

    uint64_t cpt_nid = top.cpt_nid;
    
    if ( parent_it != stack.rend() ) {
        
        parent_it->childDuration += incl;
        
    }
    
    map< uint64_t, StackType::CounterData >::const_iterator it = top.counterIdDataMap.begin( );
    for ( ; it != top.counterIdDataMap.end( ); it++ ) {
        
        const uint64_t& counter= it->first;
        const uint64_t& firstvalue= it->second.firstValue;
        const uint64_t& lastvalue= it->second.lastValue;
        const uint64_t& lasttime= it->second.lastTime;
        
        if ( lasttime == time && firstvalue != (uint64_t)-1 &&
            lastvalue != (uint64_t)-1 ) {
            
            uint64_t counter_incl= lastvalue - firstvalue;
            uint64_t counter_excl= counter_incl - it->second.childDelta;
            
            alldata->counterMapPerFunctionRank[ Triple( process, func, counter ) ]
            .add( 1, counter_excl, counter_incl );
            
            if ( parent_it != stack.rend() ) {
                
                parent_it->counterIdDataMap[ counter ].childDelta+= counter_incl;
                
            }
            
        }
        
    }
    
    stack.pop_back();
    
    alldata->functionMapPerRank[ Pair( process, func ) ].add( 1, excl, incl );

    map< pair< uint64_t, uint64_t >, FunctionData>::iterator it_func;
    it_func = alldata->functionDataPerCPTNode.find( pair< uint64_t, uint64_t >( cpt_nid, process ) ) ;

    if( it_func == alldata->functionDataPerCPTNode.end() ) {

        pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >(cpt_nid, process );

        FunctionData insertMeToo = FunctionData(1, ( (double) excl / (double) alldata->timerResolution ) , ( (double) incl / (double) alldata->timerResolution ), cpt_nid );

        alldata->functionDataPerCPTNode.insert( pair< pair< uint64_t, uint64_t >, FunctionData >(insertMe, insertMeToo) );

    } else {

        it_func->second.add(1, ( (double) excl / (double) alldata->timerResolution ), ( (double) incl / (double) alldata->timerResolution ) );

    }

    return OTF_RETURN_OK;

}

int OTFReader::handle_counter( void* fha, uint64_t time, uint32_t process,
                          uint32_t counter, uint64_t value, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    list<StackType>& stack= alldata->stackPerProcess[ process ];
    
    if ( stack.empty( ) ) {
        
        return OTF_RETURN_OK;
        
    }
    
    if ( alldata->countersOfInterest.find( counter ) ==
        alldata->countersOfInterest.end( ) ) {
        
        return OTF_RETURN_OK;
        
    }
    
    StackType& top= stack.back( );
    
    if ( time == top.timestamp ) {
        
        top.counterIdDataMap[ counter ].firstValue= value;
        
    } else {
        
        map< uint64_t, StackType::CounterData >::iterator it=
        top.counterIdDataMap.find( counter );
        
        if ( it != top.counterIdDataMap.end() ) {
            
            StackType::CounterData& top_counter= it->second;
            top_counter.lastValue= value;
            top_counter.lastTime= time;
            
        }
    }
    
    return OTF_RETURN_OK;

}

int OTFReader::handle_rma_put( void* fha, uint64_t time, uint32_t process,
                               uint32_t origin, uint32_t target, uint32_t communicator,
                               uint32_t tag, uint64_t bytes, uint32_t source,
                               OTF_KeyValueList* list ) {

    AllData* alldata = (AllData*) fha;

    std::list< StackType >& stack = alldata->stackPerProcess[ process ];

    uint64_t cid;

    if( stack.empty() ) {

        cid = -1;

    } else {

        cid = stack.front().cpt_nid;

    }

    map< pair< uint64_t, uint64_t>, RmaData >::iterator it_rma = alldata->RmaMsgData.find( pair< uint64_t, uint64_t >( cid, process ) );

    if(  it_rma == alldata->RmaMsgData.end() ) {

        RmaData insertMe;
        insertMe.rma_put_cnt = 1;
        insertMe.rma_get_cnt = 0;

        insertMe.rma_get_bytes = 0;
        insertMe.rma_put_bytes = bytes;

        alldata->RmaMsgData.insert( pair< pair< uint64_t, uint64_t>, RmaData >( pair< uint64_t, uint64_t >( cid, process), insertMe) );

    } else {

        it_rma->second.rma_put_cnt += 1;
        it_rma->second.rma_put_bytes += bytes;

    }

    return OTF_RETURN_OK;

}

int OTFReader::handle_rma_get( void* fha, uint64_t time, uint32_t process,
                               uint32_t origin, uint32_t target, uint32_t communicator,
                               uint32_t tag, uint64_t bytes, uint32_t source,
                               OTF_KeyValueList* list ) {

    AllData* alldata = (AllData*) fha;

    std::list< StackType >& stack = alldata->stackPerProcess[ process ];

    uint64_t cid;

    if( stack.empty() ) {

        cid = -1;

    } else {

        cid = stack.front().cpt_nid;

    }

    map< pair< uint64_t, uint64_t>, RmaData >::iterator it_rma = alldata->RmaMsgData.find( pair< uint64_t, uint64_t >( cid, process ) );

    if(  it_rma == alldata->RmaMsgData.end() ) {

        RmaData insertMe;
        insertMe.rma_get_cnt = 1;
        insertMe.rma_put_cnt = 0;

        insertMe.rma_put_bytes = 0;
        insertMe.rma_get_bytes = bytes;

        alldata->RmaMsgData.insert( pair< pair< uint64_t, uint64_t>, RmaData >( pair< uint64_t, uint64_t >( cid, process), insertMe) );

    } else {

        it_rma->second.rma_get_cnt += 1;
        it_rma->second.rma_get_bytes += bytes;

    }

    return OTF_RETURN_OK;

}

int OTFReader::handle_send( void* fha, uint64_t time, uint32_t sender,
                       uint32_t receiver, uint32_t group, uint32_t type,
                       uint32_t length, uint32_t source, OTF_KeyValueList* kvlist ) {

    AllData* alldata= (AllData*) fha;
    
    list<StackType>& stack= alldata->stackPerProcess[ sender ];
    
    if(alldata->P2PFuncIds.end() == alldata->P2PFuncIds.find(stack.back().fid))
    {
        alldata->P2PFuncIds.insert(stack.back().fid);
    }
    
    
    double duration= 0.0;
    
    /* get matching receive time from key-values, if available */
    
    if ( 0 != alldata->recvTimeKey ) {
        
        uint64_t recv_time;
        if ( 0 == OTF_KeyValueList_getUint64( kvlist, alldata->recvTimeKey,
                                             &recv_time ) ) {
            
            /* ignore "backward-running" messages */
            if( recv_time > time ) {
                
                duration= (double) ( recv_time - time );

            }

        }

    }
    
    alldata->messageMapPerRankPair[ Pair(sender, receiver) ]
    .add_send( 1, length, duration );
    alldata->messageMapPerRank[ sender ].add_send( 1, length, duration );
    
    /* get message speed */
    
    if ( length > 0 && duration > 0.0 ) {
        
        uint64_t speed_bin=
        Logi( (uint64_t)(
                         ( (double)length * (double)alldata->timerResolution ) /
                         duration ), MessageSpeedData::BIN_LOG_BASE );
        
        uint64_t length_bin= Logi( length, MessageSpeedData::BIN_LOG_BASE );
        
        alldata->messageSpeedMapPerLength[ Pair( speed_bin, length_bin ) ]
        .add( 1 );
        
    }
    
    if (alldata->params.write_csv_msg_sizes)
    {

        alldata->messageMapPerSize[ length ].count++;
        if ( duration > 0.0 )
        {

            alldata->messageMapPerSize[ length ].time.append((uint64_t)duration);

        }

    }
    

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it;  
    it = alldata->MessageDataPerNode.find( pair< uint64_t , uint64_t >( stack.back().cpt_nid, sender ) );
    

    if( it != alldata->MessageDataPerNode.end()) {

        it->second.add_send(1, length, 0); //0 == time -> if needed enter value 

    } else {

        alldata->MessageDataPerNode[ pair<uint64_t, uint64_t >( stack.back().cpt_nid, sender ) ].add_send( 1, length, 0 );

    }

    return OTF_RETURN_OK;

}

int OTFReader::handle_recv( void* fha, uint64_t time, uint32_t receiver,
                       uint32_t sender, uint32_t group, uint32_t type, uint32_t length,
                       uint32_t source, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    /* necessary for cube output - create map which stores recv P2PCommunication functions
     including the received bytes */
    
    list<StackType>& stack = alldata->stackPerProcess[ receiver ];
    
    if(alldata->P2PFuncIds.end() == alldata->P2PFuncIds.find(stack.back().fid)) {

        alldata->P2PFuncIds.insert(stack.back().fid);

    }
    
    
    /* duration will never be available at receive event */
    double duration= 0.0;
    
    alldata->messageMapPerRankPair[ Pair(receiver, sender) ]
    .add_recv( 1, length, duration );
    alldata->messageMapPerRank[ receiver ].add_recv( 1, length, duration );

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it;  
    it = alldata->MessageDataPerNode.find( pair< uint64_t , uint64_t >( stack.back().cpt_nid, receiver ) );
        

    if( it != alldata->MessageDataPerNode.end()) {

        it->second.add_recv(1, length, 0); //0 == time -> if needed enter value 

    } else {

        alldata->MessageDataPerNode[ pair<uint64_t, uint64_t >( stack.back().cpt_nid, receiver ) ].add_recv( 1, length, 0 );

    }

    return OTF_RETURN_OK;

}

int OTFReader::handle_collop( void* fha, uint64_t time, uint32_t process, uint32_t collOp,
                             uint32_t procGroup, uint32_t rootProc, uint32_t sent, uint32_t received,
                             uint64_t duration, uint32_t source, OTF_KeyValueList* kvlist) {
    
    AllData* alldata= (AllData*) fha;

    return OTF_RETURN_OK;

}

int OTFReader::handle_begin_collop( void* fha, uint64_t time, uint32_t process,
                                   uint32_t collOp, uint64_t matchingId, uint32_t procGroup,
                                   uint32_t rootProc, uint64_t sent, uint64_t received,
                                   uint32_t scltoken, OTF_KeyValueList* kvlist ) {

    AllData* alldata= (AllData*) fha;
    
    list< StackType >& stack = alldata->stackPerProcess[ process];
    
    if(alldata->CollFuncIds.end() == alldata->CollFuncIds.find(stack.back().fid)) {

        alldata->CollFuncIds.insert(stack.back().fid);

    }
    
    alldata->pendingCollectives[ Pair( matchingId, process ) ]=
    PendingCollective( collOp, sent, received, time );

 
    map< pair< uint64_t, uint64_t>, MessageData >::iterator it_msg = alldata->CollopDataPerNode.find( pair< uint64_t, uint64_t >( stack.back().cpt_nid, process ) );

    if(  it_msg == alldata->CollopDataPerNode.end() ) {
 
        MessageData insertMe;

        if( sent > 0 ) {

            insertMe.add_send( 1, sent, 0);
 
        }

        if( received > 0 ) {
 
            insertMe.add_recv( 1, received, 0);

        }
 
        alldata->CollopDataPerNode.insert(pair< pair< uint64_t, uint64_t>, MessageData >( pair< uint64_t, uint64_t >( stack.back().cpt_nid, process ), insertMe ) );

    } else {

        if( sent > 0 ) {

            it_msg->second.add_send( 1, sent, 0);

        }

        if( received > 0 ) {

            it_msg->second.add_recv( 1, received, 0);

        }

    }

    return OTF_RETURN_OK;

}

int OTFReader::handle_end_collop( void* fha, uint64_t time, uint32_t process,
                             uint64_t matchingId, OTF_KeyValueList* kvlist ) {

    AllData* alldata= (AllData*) fha;
    
    /* get corresponding pending collective operation */
    
    map< Pair, PendingCollective, ltPair >::iterator pending_it=
    alldata->pendingCollectives.find( Pair( matchingId, process ) );
    assert( pending_it != alldata->pendingCollectives.end() );
    
    const PendingCollective& pending= pending_it->second;
    
    /* get class of collective operation */
    
    map< uint64_t, uint64_t >::const_iterator op_class_it=
    alldata->collectiveOperationsToClasses.find( pending.collop );
    assert( op_class_it != alldata->collectiveOperationsToClasses.end() );
    
    const uint64_t& op_class= op_class_it->second;
    
    /* calculate duration */
    double duration= (double) ( time - pending.begin_time );
    
    /* add collective operation to statistics */
    
    if ( OTF_COLLECTIVE_TYPE_BARRIER == op_class ) {
        
        alldata->collectiveMapPerRank[ Pair( process, op_class ) ]
        .add_send( 1, 0, duration );
        alldata->collectiveMapPerRank[ Pair( process, op_class ) ]
        .add_recv( 1, 0, duration );
        
    } else {
        
        if ( 0 < pending.bytes_send ) {
            
            alldata->collectiveMapPerRank[ Pair( process, op_class ) ]
            .add_send( 1, pending.bytes_send, duration );
            
        }

        if ( 0 < pending.bytes_recv ) {
            
            alldata->collectiveMapPerRank[ Pair( process, op_class ) ]
            .add_recv( 1, pending.bytes_recv, duration );
            
        }
        
    }
    
    /* erase processed pending collective operation from map */
    alldata->pendingCollectives.erase( pending_it );

    return OTF_RETURN_OK;

}

int OTFReader::handle_function_summary( void* fha, uint64_t time, uint32_t func,
                                   uint32_t process, uint64_t count, uint64_t exclTime,
                                   uint64_t inclTime, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    /* add/overwrite function statistics */
    
    FunctionData tmp;
    
    tmp.count.cnt = tmp.count.sum = count;
    tmp.count.min = tmp.count.max = 0;
    
    tmp.excl_time.cnt = count;
    tmp.excl_time.sum = exclTime;
    tmp.excl_time.min = tmp.excl_time.max = 0;
    
    tmp.incl_time.cnt = count;
    tmp.incl_time.sum = inclTime;
    tmp.incl_time.min = tmp.incl_time.max = 0;
    
    alldata->functionMapPerRank[ Pair( process, func ) ]= tmp;
    
    return OTF_RETURN_OK;

}

int OTFReader::handle_message_summary( void* fha, uint64_t time, uint32_t process,
                                  uint32_t peer, uint32_t comm, uint32_t type, uint64_t sentNumber,
                                  uint64_t receivedNumber, uint64_t sentBytes,
                                  uint64_t receivedBytes, OTF_KeyValueList* kvlist ) {
    
    
    AllData* alldata= (AllData*) fha;
    
    /* do handle this record only if there is a peer and no
     communicator and tag (default behavior of VampirTrace) */
    if ( 0 != peer && 0 == comm && 0 == type ) {
        
        /* add/overwrite message statistics */
        
        MessageData tmp;
        
        if ( 0 < sentNumber ) {
            
            tmp.count_send.cnt= tmp.count_send.sum= sentNumber;
            tmp.count_send.min= tmp.count_send.max= 0;
            
            tmp.bytes_send.cnt= sentNumber;
            tmp.bytes_send.sum= sentBytes;
            tmp.bytes_send.min= tmp.bytes_send.max= 0;
            
        }
        if ( 0 < receivedNumber ) {
            
            tmp.count_recv.cnt= tmp.count_recv.sum= receivedNumber;
            tmp.count_recv.min= tmp.count_recv.max= 0;
            
            tmp.bytes_recv.cnt= receivedNumber;
            tmp.bytes_recv.sum= receivedBytes;
            tmp.bytes_recv.min= tmp.bytes_recv.max= 0;
            
        }
        
        alldata->messageMapPerRankPair[ Pair(process, peer) ]= tmp;
        alldata->messageMapPerRank[ process ]= tmp;
    }
    
    return OTF_RETURN_OK;

}

int OTFReader::handle_collop_summary( void* fha, uint64_t time, uint32_t process,
                                 uint32_t comm, uint32_t collOp, uint64_t sentNumber,
                                 uint64_t receivedNumber, uint64_t sentBytes,
                                 uint64_t receivedBytes, OTF_KeyValueList* kvlist ) {
    
    AllData* alldata= (AllData*) fha;
    
    /* do handle this record only if there is a coll.-op and no communicator
     (default behavior of VampirTrace) */
    if ( 0 != collOp && 0 == comm ) {
        
        /* get class of collective operation */
        
        map< uint64_t, uint64_t >::const_iterator op_class_it=
        alldata->collectiveOperationsToClasses.find( collOp );
        assert( op_class_it != alldata->collectiveOperationsToClasses.end() );
        
        const uint64_t& op_class= op_class_it->second;
        
        /* add/overwrite collective operation statistics */
        
        CollectiveData tmp;
        
        if ( 0 < sentNumber ) {
            
            tmp.count_send.cnt= tmp.count_send.sum= sentNumber;
            tmp.count_send.min= tmp.count_send.max= 0;
            
            tmp.bytes_send.cnt= sentNumber;
            tmp.bytes_send.sum= sentBytes;
            tmp.bytes_send.min= tmp.bytes_send.max= 0;
            
        }
        if ( 0 < receivedNumber ) {
            
            tmp.count_recv.cnt= tmp.count_recv.sum= receivedNumber;
            tmp.count_recv.min= tmp.count_recv.max= 0;
            
            tmp.bytes_recv.cnt= receivedNumber;
            tmp.bytes_recv.sum= receivedBytes;
            tmp.bytes_recv.min= tmp.bytes_recv.max= 0;
            
        }
        
        alldata->collectiveMapPerRank[ Pair( process, op_class ) ]= tmp;
        
    }
    
    return OTF_RETURN_OK;

}




/* assign trace processes to analysis processes explicitly in order to allow
 sophisticated grouping of MPI ranks/processes/threads/GPU threads/etc.
 in the future, return true if succeeded  */

bool
OTFReader::assignProcs(AllData& alldata) {
    
    bool error = false;
    
    OTF_FileManager* manager = NULL;
    OTF_MasterControl* master = NULL;
    
    if (0 == alldata.myRank) {
        
        /* the master reads OTF master control of input trace file */
        
        manager = OTF_FileManager_open(1);
        assert( manager );
        
        master = OTF_MasterControl_new(manager);
        assert( master );
        
        int master_read_ret = OTF_MasterControl_read(master,
                                                     alldata.params.input_file_prefix.c_str());
        
        /* that's the first access to the input trace file; show tidy error
         message if failed */
        if (0 == master_read_ret) {
            
            cerr << "ERROR: Unable to open file '"
            << alldata.params.input_file_prefix << ".otf' for reading."
            << endl;
            OTF_MasterControl_close(master);
            OTF_FileManager_close(manager);
            error = true;

        }

    }
    
#ifdef OTFPROFILE_MPI
    /* broadcast error indicator to workers because Open MPI had all
     ranks except rank 0 waiting endlessly in the MPI_Recv, when the '.otf' file
     was absent. */
    SyncError( alldata, error, 0 );
#endif /* OTFPROFILE_MPI */
    if (error) {
        
        return false;

    }
    
    if (0 == alldata.myRank) {
        
        /* fill the global array of processes */
        
        alldata.myProcessesNum = OTF_MasterControl_getrCount(master);
        alldata.myProcessesList = (uint64_t*) malloc(alldata.myProcessesNum
                                                     * sizeof(uint64_t));
        assert( alldata.myProcessesList );
        
        uint32_t i = 0;
        uint32_t j = 0;
        
        while (true) {
            
            OTF_MapEntry* entry = OTF_MasterControl_getEntryByIndex(master, i);
            
            if (NULL == entry)
                break;
            
            for (uint32_t k = 0; k < entry->n; k++) {
                
                alldata.myProcessesList[j] = entry->values[k];
                j++;
            }
            
            i++;

        }
        assert( alldata.myProcessesNum == j );
        
        /* close OTF master control and file manager */
        OTF_MasterControl_close(master);
        OTF_FileManager_close(manager);
        
        /* DEBUG */
        /*cerr << "processes in trace: ";
         for ( uint32_t k= 0; k < alldata.myProcessesNum; k++ ) {
         
         cerr << alldata.myProcessesList[k] << " ";
         }
         cerr << endl;*/
    }
    
    /* now we may re-arrange the process list for a better layout
     - note that this layout is optimal to re-use OTF streams
     if there are multiple processes per stream
     - one may read the OTF definitions to know how to re-arrange */
    
#ifdef OTFPROFILE_MPI
    if ( 0 == alldata.myRank ) {

        /* get number of ranks per worker, send to workers */
        
        /* remaining ranks and remaining workers */
        uint64_t r_ranks= alldata.myProcessesNum;
        uint64_t r_workers= alldata.numRanks;
        
        uint64_t pos= 0;
        bool warn_for_empty= true;
        for ( int w= 0; w < (int)alldata.numRanks; w++ ) {
            
            uint64_t n= ( ( r_ranks / r_workers ) * r_workers < r_ranks) ?
            ( r_ranks / r_workers +1 ) : ( r_ranks / r_workers );

            if ( ( 0 == n ) && warn_for_empty ) {
                
                cerr << "Warning: more analysis ranks than trace processes, "
                << "ranks " << w << " to " << alldata.numRanks -1
                << " are unemployed" << endl;
                
                warn_for_empty= false;

            }
            
            if ( 0 == w ) {
                
                /* for master itself simply truncate processesList,
                 don't send and receive */
                alldata.myProcessesNum= n;
                
            } else {
                
                MPI_Send( &n, 1, MPI_LONG_LONG_INT, w, 2, MPI_COMM_WORLD );
                
                MPI_Send( alldata.myProcessesList + pos, n, MPI_LONG_LONG_INT,
                         w, 3, MPI_COMM_WORLD );
                
            }

            pos += n;
            r_ranks -= n;
            r_workers -= 1;

        }
        
    } else { /* 0 != alldata.myRank */
        
        /* workers receive number and sub-list of their ranks to process */
        
        alldata.myProcessesNum= 0;
        
        MPI_Status status;
        
        MPI_Recv( &alldata.myProcessesNum, 1, MPI_LONG_LONG_INT, 0, 2, MPI_COMM_WORLD,
                 &status );

        alldata.myProcessesList= (uint64_t*)malloc(
                                                   alldata.myProcessesNum * sizeof(uint64_t) );
        assert( alldata.myProcessesList );
        
        MPI_Recv( alldata.myProcessesList, alldata.myProcessesNum, MPI_LONG_LONG_INT, 0,
                 3, MPI_COMM_WORLD, &status );
        
    }
    
    /* DEBUG */
    /*cerr << " worker " << alldata.myRank << " handles: ";
     for ( uint64_t k= 0; k < alldata.myProcessesNum; k++ ) {
     
     cerr << alldata.myProcessesList[k] << " ";
     }
     cerr << endl;*/
#endif /* OTFPROFILE_MPI */
    
    return !error;

}

bool
OTFReader::readDefinitions( AllData& alldata ) {
    
    bool error= false;
    
    /* open OTF handler array */
    OTF_HandlerArray* handlers= OTF_HandlerArray_open( );
    assert( handlers );
    
    /* set record handler functions */
    
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_creator,
                                OTF_DEFCREATOR_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_version,
                                OTF_DEFVERSION_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_comment,
                                OTF_DEFINITIONCOMMENT_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_timerres,
                                OTF_DEFTIMERRESOLUTION_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_process,
                                OTF_DEFPROCESS_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_function,
                                OTF_DEFFUNCTION_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_functiongroup,
                                OTF_DEFFUNCTIONGROUP_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_collop,
                                OTF_DEFCOLLOP_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_counter,
                                OTF_DEFCOUNTER_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_def_keyvalue,
                                OTF_DEFKEYVALUE_RECORD );
    
    /* set record handler's first arguments */
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFCREATOR_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFVERSION_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFINITIONCOMMENT_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFTIMERRESOLUTION_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFPROCESS_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFPROCESSGROUP_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFFUNCTION_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFFUNCTIONGROUP_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFCOLLOP_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_DEFCOUNTER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                           OTF_DEFKEYVALUE_RECORD );

    /* read definitions */
    uint64_t read_ret= OTF_Reader_readDefinitions( reader, handlers );

    if ( OTF_READ_ERROR == read_ret ) {
            
        cerr << "ERROR: Could not read definitions." << endl;
        error= true;
            
    }
        
    /* close OTF handler array */
    OTF_HandlerArray_close( handlers );
    
#ifdef OTFPROFILE_MPI
    /* broadcast error indicator to workers */
    if ( SyncError( alldata, error, 0 ) ) {
        
        return error;
        
    }
    
    /* share definitions needed for reading events to workers */
    
    if ( 1 < alldata.numRanks ) {
        
        share_definitions( alldata );
        
    }

#endif /* OTFPROFILE_MPI */
    
    return !error;
}

bool
OTFReader::readEvents( AllData& alldata ) {

    bool error= false;
    
    /* open OTF handler array */
    OTF_HandlerArray* handlers= OTF_HandlerArray_open( );
    assert( handlers );
    
    /* set record handler functions */
    
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_enter,
                                OTF_ENTER_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_leave,
                                OTF_LEAVE_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_counter,
                                OTF_COUNTER_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_send,
                                OTF_SEND_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_recv,
                                OTF_RECEIVE_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_begin_collop,
                                OTF_BEGINCOLLOP_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_end_collop,
                                OTF_ENDCOLLOP_RECORD );


    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_rma_put,
                                OTF_RMAPUT_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_rma_get,
                                OTF_RMAGET_RECORD );
    
    /* set record handler's first arguments */
    
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_ENTER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_LEAVE_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_COUNTER_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_SEND_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_RECEIVE_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_BEGINCOLLOP_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_ENDCOLLOP_RECORD );

    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_RMAPUT_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_RMAGET_RECORD );
    
    /* select processes to read */
    OTF_Reader_setProcessStatusAll( reader, 0 );

    for ( uint32_t i= 0; i < alldata.myProcessesNum; i++ ) {
        
        OTF_Reader_enableProcess( reader, alldata.myProcessesList[ i ] );
    }
    
    /* prepare progress */
    if ( alldata.params.progress ) {
        
        OTF_Reader_setRecordLimit( reader, 0 );
        error= ( OTF_READ_ERROR == OTF_Reader_readEvents( reader, handlers ) );
        
#ifdef OTFPROFILE_MPI
        /* synchronize error indicator to workers */
        SyncError( alldata, error );

#endif /* OTFPROFILE_MPI */
        
        if ( !error ) {
            
            uint64_t min, cur, max;
            
            OTF_Reader_eventBytesProgress( reader, &min, &cur, &max );
            prepare_progress( alldata, max );
            
            OTF_Reader_setRecordLimit( reader, Progress::EVENTS_RECORD_LIMIT );
            
        }
        
    }
    
    /* read events */
    
    uint64_t records_read= 0;
    
    while ( !error && OTF_READ_ERROR !=
           ( records_read= OTF_Reader_readEvents( reader, handlers ) ) ) {
        
        /* update progress */
        if ( alldata.params.progress ) {
            
            uint64_t min, cur, max;
            static uint64_t last_cur= 0;
            
            OTF_Reader_eventBytesProgress( reader, &min, &cur, &max );
            update_progress( alldata, cur - last_cur );
            
            last_cur = cur;
            
        }
        
        /* stop reading if done */
        if ( 0 == records_read ) {

            break;
    
        }

    }

    error= ( error || OTF_READ_ERROR == records_read );
    
#ifdef OTFPROFILE_MPI
    /* synchronize error indicator to workers */
    SyncError( alldata, error );

#endif /* OTFPROFILE_MPI */
    
    /* finish progress */
    if ( alldata.params.progress ) {
        
        finish_progress( alldata );
        
    }
    
    /* close OTF handler array */
    OTF_HandlerArray_close( handlers );
    
    uint64_t tmp = 0; 

    alldata.systemTreeIdToSystemTreeNodeMap[ tmp ] = SystemTreeNode( "machine", SYSTEMTREE_MACHINE, 0, 0);
    alldata.systemTreeClassToNodeIdsMap[SYSTEMTREE_MACHINE].insert( tmp );

    map< uint64_t, SystemTreeNode>::iterator it_mach = alldata.systemTreeIdToSystemTreeNodeMap.find( tmp );

    map< SystemTreeClass, set < uint64_t > >::iterator it_map = alldata.systemTreeClassToNodeIdsMap.find( SYSTEMTREE_LOCATION_GROUP );
    set< uint64_t>::iterator it_set = it_map->second.begin();

    map< uint64_t, SystemTreeNode>::iterator it_sys;

    for( ; it_set != it_map->second.end(); it_set++) {

        if( (it_sys = alldata.systemTreeIdToSystemTreeNodeMap.find( *it_set )) != alldata.systemTreeIdToSystemTreeNodeMap.end() ) {

            it_sys->second.parent = tmp;
            it_mach->second.children.insert( it_sys->first );
            it_mach->second.child_counter++;

        }

    }

    return !error;

}

bool
OTFReader::readStatistics( AllData& alldata ) {
    
    bool error= false;
    
    /* open OTF handler array */
    OTF_HandlerArray* handlers= OTF_HandlerArray_open( );
    assert( handlers );
    
    /* set record handler functions */
    
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_function_summary,
                                OTF_FUNCTIONSUMMARY_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_message_summary,
                                OTF_MESSAGESUMMARY_RECORD );
    OTF_HandlerArray_setHandler( handlers,
                                (OTF_FunctionPointer*) handle_collop_summary,
                                OTF_COLLOPSUMMARY_RECORD );
    
    /* set record handler's first arguments */
    
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_FUNCTIONSUMMARY_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_MESSAGESUMMARY_RECORD );
    OTF_HandlerArray_setFirstHandlerArg( handlers, &alldata,
                                        OTF_COLLOPSUMMARY_RECORD );
    
    /* select processes to read */
    OTF_Reader_setProcessStatusAll( reader, 0 );

    for ( uint32_t i= 0; i < alldata.myProcessesNum; i++ ) {
        
        OTF_Reader_enableProcess( reader, alldata.myProcessesList[ i ] );

    }
    
    /* prepare progress */
    if ( alldata.params.progress ) {
        
        OTF_Reader_setRecordLimit( reader, 0 );
        
        if ( OTF_READ_ERROR != OTF_Reader_readStatistics( reader, handlers ) ) {
            
            uint64_t min, cur, max;
            OTF_Reader_statisticBytesProgress( reader, &min, &cur, &max );
            prepare_progress( alldata, max );
            
        }
        
        OTF_Reader_setRecordLimit( reader, Progress::STATS_RECORD_LIMIT );
        
    }
    
    /* read statistics */
    
    uint64_t records_read= 0;
    
    while ( OTF_READ_ERROR !=
           ( records_read= OTF_Reader_readStatistics( reader, handlers ) ) ) {
        
        /* update progress */
        if ( alldata.params.progress ) {
            
            uint64_t min, cur, max;
            static uint64_t last_cur= 0;
            
            OTF_Reader_statisticBytesProgress( reader, &min, &cur, &max );
            update_progress( alldata, cur - last_cur );
            
            last_cur = cur;
            
        }
        
        /* stop reading if done */
        if ( 0 == records_read ) {

            break;
        }

    }
    
    /* show error message if reading failed */
    if ( OTF_READ_ERROR == records_read ) {
        
        cerr << "ERROR: Could not read statistics." << endl;
        error= true;
        
    }
    
    /* close OTF handler array */
    OTF_HandlerArray_close( handlers );
    
    return !error;
}

