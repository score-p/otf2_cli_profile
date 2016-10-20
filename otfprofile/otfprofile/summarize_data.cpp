/*
 This is part of the OTF library. Copyright by ZIH, TU Dresden 2005-2013.
 Authors: Andreas Knuepfer, Robert Dietrich, Matthias Jurenz
*/

#include <iostream>

#include "otfprofile.h"
#include "summarize_data.h"


using namespace std;


static void get_grouping( AllData& alldata ) {

    uint32_t r_processes= alldata.allProcesses.size();
    uint32_t r_groups= alldata.params.max_groups;

    set< Process, ltProcess >::iterator pos= alldata.allProcesses.begin();

    for ( uint32_t c= 0;
          c < Grouping::MAX_GROUPS && 0 < r_processes; c++ ) {

        uint32_t n=
            ( ( r_processes / r_groups ) * r_groups < r_processes ) ?
            ( r_processes / r_groups + 1 ) : ( r_processes / r_groups );

        for ( uint32_t i= 0; i < n; i++ ) {

            alldata.grouping.insert( c+1, pos->process );

            pos++;
            r_processes--;

        }

        r_groups--;

    }
}

static uint64_t systemTreeParentIdOfClassType( AllData& alldata, uint64_t node_id, SystemTreeClass parent_class_id ) {
    
    map<uint64_t,SystemTreeNode>::const_iterator it= alldata.systemTreeIdToSystemTreeNodeMap.find(node_id);
    
    assert(it!= alldata.systemTreeIdToSystemTreeNodeMap.end());

    SystemTreeNode node= it->second;
    
    uint64_t parent_node_id= node.parent;
    uint64_t last_parent_node_id;
    
    while ( parent_class_id != node.class_id ) {
        it = alldata.systemTreeIdToSystemTreeNodeMap.find(parent_node_id);
        assert(it!=alldata.systemTreeIdToSystemTreeNodeMap.end());
        
        node= it->second;
        last_parent_node_id = parent_node_id;
        parent_node_id = it->first;
        
        if (last_parent_node_id == parent_node_id)
          break;
    }
    
    return parent_node_id;
}

#ifdef OTFPROFILE_MPI
static void share_grouping( AllData& alldata ) {

    assert( 1 < alldata.numRanks );

    char* buffer;
    int buffer_size= 0;
    int buffer_pos= 0;

    if ( 0 == alldata.myRank ) {

        /* get size needed to send grouping information to workers */

        int size;

        /* alldata.grouping.groupsToProcesses.size() + firsts */
        MPI_Pack_size( 1 + alldata.grouping.groupsToProcesses.size(),
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD, &size );
        buffer_size+= size;

        /* alldata.grouping.groupsToProcesses.second.size() + second */
        for ( map< uint64_t, set<uint64_t> >::const_iterator it=
              alldata.grouping.groupsToProcesses.begin();
              it != alldata.grouping.groupsToProcesses.end(); it++ ) {

            MPI_Pack_size( 1 + it->second.size(), MPI_LONG_LONG_INT,
                           MPI_COMM_WORLD, &size );
            buffer_size+= size;

        }

    }

    /* broadcast buffer size */
    MPI_Bcast( &buffer_size, 1, MPI_INT, 0, MPI_COMM_WORLD );

    /* allocate buffer */
    buffer= new char[ buffer_size ];
    assert( buffer );

    /* pack grouping information to buffer */

    if ( 0 == alldata.myRank ) {

        /* alldata.grouping.groupsToProcesses.size() */
        uint64_t clust_proc_map_size=
            alldata.grouping.groupsToProcesses.size();
        MPI_Pack( &clust_proc_map_size, 1, MPI_LONG_LONG_INT, buffer,
                  buffer_size, &buffer_pos, MPI_COMM_WORLD );

        /* alldata.grouping.groupsToProcesses */
        for ( map< uint64_t, set<uint64_t> >::const_iterator it=
              alldata.grouping.groupsToProcesses.begin();
              it != alldata.grouping.groupsToProcesses.end(); it++ ) {

            /* alldata.grouping.groupsToProcesses.first */
            uint64_t group= it->first;
            MPI_Pack( &group, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                      &buffer_pos, MPI_COMM_WORLD );

            /* alldata.grouping.groupsToProcesses.second.size() */
            uint64_t processes_size= it->second.size();
            MPI_Pack( &processes_size, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                      &buffer_pos, MPI_COMM_WORLD );

            /* alldata.grouping.groupsToProcesses.second */
            for ( set<uint64_t>::const_iterator it2= it->second.begin();
                  it2 != it->second.end(); it2++ ) {

                uint64_t process= *it2;
                MPI_Pack( &process, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                          &buffer_pos, MPI_COMM_WORLD );

            }

        }

    }

    /* broadcast definitions buffer */
    MPI_Bcast( buffer, buffer_size, MPI_PACKED, 0, MPI_COMM_WORLD );

    /* unpack grouping information from buffer */

    if ( 0 != alldata.myRank ) {

        /* alldata.grouping.groupsToProcesses.size() */
        uint64_t clust_proc_map_size;
        MPI_Unpack( buffer, buffer_size, &buffer_pos, &clust_proc_map_size, 1,
                    MPI_LONG_LONG_INT, MPI_COMM_WORLD );

        /* alldata.grouping.groupsToProcesses */
        for ( uint64_t i= 0; i < clust_proc_map_size; i++ ) {

            /* alldata.grouping.groupsToProcesses.first */
            uint64_t group;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &group, 1,
                        MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            /* alldata.grouping.groupsToProcesses.second.size() */
            uint64_t processes_size;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &processes_size, 1,
                        MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            /* alldata.grouping.groupsToProcesses.second */
            for ( uint64_t j= 0; j < processes_size; j++ ) {

                uint64_t process;
                MPI_Unpack( buffer, buffer_size, &buffer_pos, &process, 1,
                            MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                alldata.grouping.insert( group, process );

            }

        }

    }

    delete[] buffer;
}
#endif /* OTFPROFILE_MPI */


bool SummarizeData( AllData& alldata ) {

    bool error= false;

    /* start runtime measurement for summarizing data */
    StartMeasurement( alldata, 1, true, "summarize data" );

    /* rank 0 gets grouping information */

    if ( 0 == alldata.myRank ) {

        get_grouping( alldata );

    }

#ifdef OTFPROFILE_MPI
    /* share grouping information to workers */

    if ( 1 < alldata.numRanks ) {

        share_grouping( alldata );

    }

#endif /* OTFPROFILE_MPI */

    /* macro to set min, max to sum before summarizing */
#   define MINMAX2SUM(v) \
    if( 0 != (v).cnt ) { \
        (v).cnt = 1; \
        (v).min= (v).max= (v).sum; \
    } else { \
        (v).cnt = 0; \
        /* (v).min= OTF_UINT64_MAX; (v).max= 0; \
           ^^^ this is set already by the constructor and never touched \
           if (v).cnt == 0. Therefore, it is ignored when computing min/max \
           further on. */ \
    }

    /* summarize function and message statistics over the system tree */
    {

        set<uint64_t> tmp_systemTreeNodeSet;
        map< SystemTreeClass, set < uint64_t > >::const_iterator it_systemTree = alldata.systemTreeClassToNodeIdsMap.find(SYSTEMTREE_LOCATION);

        /* assert if no process/location is in the system tree */
        assert ( it_systemTree != alldata.systemTreeClassToNodeIdsMap.end() );
        
        set< uint64_t >::iterator it= it_systemTree->second.begin();
        set< uint64_t >::iterator itend= it_systemTree->second.end();

        /* iterate over all leaf nodes first */
        while ( itend != it) {
            
            /* get location id via system tree node*/
            map< uint64_t, SystemTreeNode >::const_iterator it_systemNode = alldata.systemTreeIdToSystemTreeNodeMap.find(*it);
            
            assert( it_systemNode != alldata.systemTreeIdToSystemTreeNodeMap.end() );
            
            SystemTreeNode node = it_systemNode->second;
            SystemTreeNode& parent_node = alldata.systemTreeIdToSystemTreeNodeMap.find(node.parent)->second;
            
            /* add each function statistic to parent node */
            map< Pair, FunctionData, ltPair >::const_iterator it_FuncLb = alldata.functionMapPerRank.lower_bound( Pair(node.location,0) );
            map< Pair, FunctionData, ltPair >::const_iterator it_FuncUb = alldata.functionMapPerRank.upper_bound( Pair(node.location,(uint64_t)-1) );
            
            while (it_FuncLb != it_FuncUb) {
                
                const uint64_t& func= it_FuncLb->first.b;

#ifdef OTFPROFILE_MPI

                map< Pair, FunctionData, ltPair >::iterator tmp_it = alldata.functionMapPerSystemTreeNodeReduce.find( Pair( *it, func ) );

                if( tmp_it != alldata.functionMapPerSystemTreeNodeReduce.end() ) {

                    tmp_it->second.add( it_FuncLb->second );

                }else{

                    alldata.functionMapPerSystemTreeNodeReduce.insert( make_pair( Pair( *it, func ), it_FuncLb->second ) );

                }

#else

                map< Pair, FunctionData, ltPair >::iterator tmp_it = alldata.functionMapPerSystemTreeNode.find( Pair( *it, func ) );

                if( tmp_it != alldata.functionMapPerSystemTreeNode.end() ) {

                    tmp_it->second.add( it_FuncLb->second );

                }else{

                    alldata.functionMapPerSystemTreeNode.insert( make_pair( Pair( *it, func ), it_FuncLb->second ) );

                }

#endif


#ifdef OTFPROFILE_MPI
                alldata.functionMapPerSystemTreeNodeReduce[ Pair(*it,func) ].add( it_FuncLb->second );
                alldata.functionMapPerSystemTreeNodeReduce[ Pair(node.parent, func) ].add( it_FuncLb->second );
#else   //OTFPROFILE_MPI
                alldata.functionMapPerSystemTreeNode[ Pair(*it,func) ].add( it_FuncLb->second );
                alldata.functionMapPerSystemTreeNode[ Pair(node.parent, func) ].add( it_FuncLb->second );
#endif //OTFPROFILE_MPI                

                
                it_FuncLb++;
            }
            
            /* add each message statistic to parent node */
            
            map< Pair, MessageData, ltPair >::const_iterator it_MsgLb = alldata.messageMapPerRankPair.lower_bound( Pair(node.location,0) );
            map< Pair, MessageData, ltPair >::const_iterator it_MsgUb = alldata.messageMapPerRankPair.upper_bound( Pair(node.location,(uint64_t)-1) );
            
            while ( it_MsgLb != it_MsgUb ) {
                
                uint64_t group_a= it_MsgLb->first.a;
                uint64_t group_b= it_MsgLb->first.b;
                
                /* id of receiver in the system tree */
                map< Pair, uint64_t, ltPair >::const_iterator recvIter = alldata.traceIdToSystemTreeId.find( Pair(SYSTEMTREE_LOCATION,group_b) );
                assert(recvIter != alldata.traceIdToSystemTreeId.end());
                uint64_t receiver= recvIter->second;
                
                /* get copy of message data in order to keep original data
                 unchanged for CSV output */
                MessageData data= it_MsgLb->second;
                
                uint64_t receiver_parent = systemTreeParentIdOfClassType( alldata, receiver, parent_node.class_id );

#ifdef OTFPROFILE_MPI

                alldata.messageMapPerSystemTreeNodePairReduce[ Pair( *it, receiver) ].add( data );
                alldata.messageMapPerSystemTreeNodePairReduce[ Pair( node.parent, receiver_parent ) ].add( data );

#else //OTFPROFILE_MPI


                alldata.messageMapPerSystemTreeNodePair[ Pair( *it, receiver) ].add( data );
                alldata.messageMapPerSystemTreeNodePair[ Pair( node.parent, receiver_parent ) ].add( data );

#endif //OTFPROFILE_MPI
                it_MsgLb++;
            }
            
            /* add message data of location to parent node*/
            map< uint64_t, MessageData >::const_iterator it_Msg= alldata.messageMapPerRank.find(node.location);
                
            /* get copy of message data in order to keep original data
            unchanged for CSV output */

            if( it_Msg != alldata.messageMapPerRank.end() ) {

                MessageData data= it_Msg->second;
                    
                map< uint64_t, MessageData >::iterator tmp_it = alldata.messageMapPerSystemTreeNode.find( *it );

                if( tmp_it != alldata.messageMapPerSystemTreeNode.end() ) {

                    tmp_it->second.add( data );

                }else{

                    alldata.messageMapPerSystemTreeNode.insert( make_pair( *it, data ) );

                }
            }  
          
            parent_node.child_counter++;
            
            if ( parent_node.child_counter == parent_node.children.size() ) {
                
                /* add parent to set of nodes that are "flagged" to propagate their statistic information to their parents */
                tmp_systemTreeNodeSet.insert(node.parent);

            }

            it++;

        }

        /* now iterate over all nodes in the system tree that are "flagged" to propagate their values */
        while (!tmp_systemTreeNodeSet.empty()) {

            /* get first element of the set of nodes that already have their statistic information */
            set<uint64_t>::const_iterator it_set= tmp_systemTreeNodeSet.begin();
            
            /* get current system tree node*/
            map< uint64_t, SystemTreeNode >::const_iterator it_systemNode = alldata.systemTreeIdToSystemTreeNodeMap.find(*it_set);
            
            assert( it_systemNode != alldata.systemTreeIdToSystemTreeNodeMap.end() );
            
            uint64_t node_id = *it_set;
            SystemTreeNode node = it_systemNode->second;
            SystemTreeNode& parent_node = alldata.systemTreeIdToSystemTreeNodeMap.find(node.parent)->second;
            

            
            /* add each function statistic to parent node */
#ifdef OTFPROFILE_MPI                
            map< Pair, FunctionData, ltPair >::const_iterator it_FuncLb = alldata.functionMapPerSystemTreeNodeReduce.lower_bound( Pair(node_id,0) );
            map< Pair, FunctionData, ltPair >::const_iterator it_FuncUb = alldata.functionMapPerSystemTreeNodeReduce.upper_bound( Pair(node_id,(uint64_t)-1) );
#else //OTFPROFILE_MPI
            map< Pair, FunctionData, ltPair >::const_iterator it_FuncLb = alldata.functionMapPerSystemTreeNode.lower_bound( Pair(node_id,0) );
            map< Pair, FunctionData, ltPair >::const_iterator it_FuncUb = alldata.functionMapPerSystemTreeNode.upper_bound( Pair(node_id,(uint64_t)-1) );
#endif //OTFPROFILE_MPI
            while (it_FuncLb != it_FuncUb) {
                
                const uint64_t& func= it_FuncLb->first.b;
                                

#ifdef OTFPROFILE_MPI

                map< Pair, FunctionData, ltPair >::iterator tmp_it = alldata.functionMapPerSystemTreeNodeReduce.find( Pair( node.parent, func ) );

                if( tmp_it != alldata.functionMapPerSystemTreeNodeReduce.end() ) {

                    tmp_it->second.add( it_FuncLb->second );

                }else{

                    alldata.functionMapPerSystemTreeNodeReduce.insert( make_pair( Pair( node.parent, func ), it_FuncLb->second ) );

                }

#else

                map< Pair, FunctionData, ltPair >::iterator tmp_it = alldata.functionMapPerSystemTreeNode.find( Pair( node.parent, func ) );

                if( tmp_it != alldata.functionMapPerSystemTreeNode.end() ) {

                    tmp_it->second.add( it_FuncLb->second );

                }else{

                    alldata.functionMapPerSystemTreeNode.insert( make_pair( Pair( node.parent, func ), it_FuncLb->second ) );

                }

#endif

                it_FuncLb++;
            }
            
            /* add each message statistic to parent node */
#ifdef OTFPROFILE_MPI                
            map< Pair, MessageData, ltPair >::const_iterator it_MsgLb = alldata.messageMapPerSystemTreeNodePairReduce.lower_bound( Pair(node_id,0) );
            map< Pair, MessageData, ltPair >::const_iterator it_MsgUb = alldata.messageMapPerSystemTreeNodePairReduce.upper_bound( Pair(node_id,(uint64_t)-1) );
#else //OTFPROFILE_MPI
            map< Pair, MessageData, ltPair >::const_iterator it_MsgLb = alldata.messageMapPerSystemTreeNodePair.lower_bound( Pair(node_id,0) );
            map< Pair, MessageData, ltPair >::const_iterator it_MsgUb = alldata.messageMapPerSystemTreeNodePair.upper_bound( Pair(node_id,(uint64_t)-1) );
#endif //OTFPROFILE_MPIf
            
            while ( it_MsgLb != it_MsgUb ) {
                
                uint64_t group_a= it_MsgLb->first.a;
                uint64_t group_b= it_MsgLb->first.b;
                
                /* get copy of message data in order to keep original data
                 unchanged for CSV output */
                MessageData data= it_MsgLb->second;

                /*map< Pair, uint64_t, ltPair >::const_iterator recvIter = alldata.traceIdToSystemTreeId.find( Pair(SYSTEMTREE_LOCATION,group_b) );
                assert(recvIter != alldata.traceIdToSystemTreeId.end());*/
                /* \todo group_b is already a system tree id, no need to query traceIdToSystemTreeId again ?!*/
                uint64_t receiver= group_b;
                
                uint64_t receiver_parent = systemTreeParentIdOfClassType( alldata, receiver, parent_node.class_id );

#ifdef OTFPROFILE_MPI                
                alldata.messageMapPerSystemTreeNodePairReduce[ Pair( node.parent, receiver_parent ) ].add( data );
#else //OTFPROFILE_MPI                
                alldata.messageMapPerSystemTreeNodePair[ Pair( node.parent, receiver_parent ) ].add( data );
#endif //OTFPROFILE_MPI                
                it_MsgLb++;

            }
            
            /* add message data of location to parent node*/

            map< uint64_t, MessageData >::const_iterator it_Msg= alldata.messageMapPerSystemTreeNode.find(node_id);
            
            if( it_Msg != alldata.messageMapPerSystemTreeNode.end() ) {

                /* get copy of message data in order to keep original data
                 unchanged for CSV output */
                MessageData data= it_Msg->second;

                if( alldata.messageMapPerSystemTreeNode.find( node.parent ) != alldata.messageMapPerSystemTreeNode.end() ) {

                    alldata.messageMapPerSystemTreeNode[ node.parent ].add( data );

                } else {

                    alldata.messageMapPerSystemTreeNode.insert( make_pair( node.parent, data ) );

                }

            }

            parent_node.child_counter++;
            
            if ( parent_node.child_counter == parent_node.children.size() && 0 != node.parent ) {
                
                /* add parent to set of nodes that are "flagged" to propagate their statistic information to their parents */
                tmp_systemTreeNodeSet.insert(node.parent);
            }
            
            /* delete current element */
            tmp_systemTreeNodeSet.erase(it_set);

        }
    }
    
    /* summarize collective message statistics over the system tree */
    {
        set<uint64_t> tmp_systemTreeNodeSet;
        map< SystemTreeClass, set < uint64_t > >::const_iterator it_systemTree = 
                alldata.systemTreeClassToNodeIdsMap.find(SYSTEMTREE_LOCATION);
        
        /* assert if no process/location is in the system tree */
        assert ( it_systemTree != alldata.systemTreeClassToNodeIdsMap.end() );
        
        set< uint64_t >::const_iterator it= it_systemTree->second.begin();
        set< uint64_t >::const_iterator itend= it_systemTree->second.end();
        
        map< uint64_t, uint64_t > parentChildCtrMap;
        
        /* iterate over all leaf nodes first */
        while ( itend != it) {
            
            /* get location id via system tree node*/
            map< uint64_t, SystemTreeNode >::const_iterator it_systemNode = 
                    alldata.systemTreeIdToSystemTreeNodeMap.find(*it);
            
            assert( it_systemNode != alldata.systemTreeIdToSystemTreeNodeMap.end() );
            
            uint64_t node_id = *it;
            SystemTreeNode node = it_systemNode->second;
            SystemTreeNode& parent_node = alldata.systemTreeIdToSystemTreeNodeMap.find(node.parent)->second;
            
            /* add each message statistic to parent node */
            
            map< Triple, CollectiveData, ltTriple >::const_iterator it_CollLb = 
                    alldata.collectiveMapPerRankPair.lower_bound( Triple(node.location,0,0) );
            map< Triple, CollectiveData, ltTriple >::const_iterator it_CollUb = 
                    alldata.collectiveMapPerRankPair.upper_bound( Triple(node.location,(uint64_t)-1,0) );
            
            while ( it_CollLb != it_CollUb ) {
                
                uint64_t group_a= it_CollLb->first.a;
                uint64_t group_b= it_CollLb->first.b;
                uint64_t collOp = it_CollLb->first.c;
                
                /* id of partner in the system tree */
                map< Pair, uint64_t, ltPair >::const_iterator partnerIter = 
                        alldata.traceIdToSystemTreeId.find( Pair(SYSTEMTREE_LOCATION,group_b) );
                assert(partnerIter != alldata.traceIdToSystemTreeId.end());
                uint64_t partner= partnerIter->second;
                
                /* get copy of message data in order to keep original data
                 unchanged for CSV output */
                CollectiveData data= it_CollLb->second;
                
                uint64_t partner_parent = systemTreeParentIdOfClassType( alldata, partner, parent_node.class_id );
                
#ifdef OTFPROFILE_MPI
                alldata.collectiveMapPerSystemTreeNodePairReduce[ Triple( node_id, partner, collOp) ].add( data );
                alldata.collectiveMapPerSystemTreeNodePairReduce[ Triple( node.parent, partner_parent, collOp ) ].add( data );
#else //OTFPROFILE_MPI
                alldata.collectiveMapPerSystemTreeNodePair[ Triple( node_id, partner, collOp) ].add( data );
                alldata.collectiveMapPerSystemTreeNodePair[ Triple( node.parent, partner_parent, collOp ) ].add( data );
#endif //OTFPROFILE_MPI
                it_CollLb++;
            }
            
            /* add message data of location to parent node*/
            for (map< Pair, CollectiveData >::const_iterator it_Coll= alldata.collectiveMapPerRank.begin();
                  it_Coll != alldata.collectiveMapPerRank.end(); ++it_Coll)
            {
                if (it_Coll->first.a != node.location)
                    continue;

                /* get copy of message data in order to keep original data
                unchanged for CSV output */
                CollectiveData data= it_Coll->second;

                alldata.collectiveMapPerSystemTreeNode[ Pair( node_id, it_Coll->first.b ) ].add( data );
                alldata.collectiveMapPerSystemTreeNode[ Pair( node.parent, it_Coll->first.b ) ].add( data );
            }
            
            parentChildCtrMap[node.parent]++;
            
            if ( parentChildCtrMap[node.parent] == parent_node.children.size() ) {
                
                /* add parent to set of nodes that are "flagged" to propagate their statistic information to their parents */
                tmp_systemTreeNodeSet.insert(node.parent);
            }
            
            it++;

        }
        
        /* now iterate over all nodes in the system tree that are "flagged" to propagate their values */
        while (!tmp_systemTreeNodeSet.empty()) {
            
            /* get first element of the set of nodes that already have their statistic information */
            set<uint64_t>::const_iterator it_set= tmp_systemTreeNodeSet.begin();
            
            /* get current system tree node*/
            map< uint64_t, SystemTreeNode >::const_iterator it_systemNode = alldata.systemTreeIdToSystemTreeNodeMap.find(*it_set);
            
            assert( it_systemNode != alldata.systemTreeIdToSystemTreeNodeMap.end() );
            
            uint64_t node_id = *it_set;
            SystemTreeNode node = it_systemNode->second;
            SystemTreeNode& parent_node = alldata.systemTreeIdToSystemTreeNodeMap.find(node.parent)->second;
            
            /* add each message statistic to parent node */
            
            

#ifdef OTFPROFILE_MPI
                map< Triple, CollectiveData, ltTriple >::const_iterator it_CollLb = 
                    alldata.collectiveMapPerSystemTreeNodePairReduce.lower_bound( Triple(node_id,0,0) );
                map< Triple, CollectiveData, ltTriple >::const_iterator it_CollUb = 
                    alldata.collectiveMapPerSystemTreeNodePairReduce.upper_bound( Triple(node_id,(uint64_t)-1,(uint64_t)-1) );
#else //OTFPROFILE_MPI
                map< Triple, CollectiveData, ltTriple >::const_iterator it_CollLb = 
                    alldata.collectiveMapPerSystemTreeNodePair.lower_bound( Triple(node_id,0,0) );
                map< Triple, CollectiveData, ltTriple >::const_iterator it_CollUb = 
                    alldata.collectiveMapPerSystemTreeNodePair.upper_bound( Triple(node_id,(uint64_t)-1,(uint64_t)-1) );
#endif //OTFPROFILE_MPI

            
            while ( it_CollLb != it_CollUb ) {
                
                uint64_t group_a= it_CollLb->first.a;
                uint64_t group_b= it_CollLb->first.b;
                uint64_t collOp = it_CollLb->first.c;
                
                /* get copy of message data in order to keep original data
                 unchanged for CSV output */
                CollectiveData data= it_CollLb->second;

                uint64_t partner= group_b;
                
                uint64_t partner_parent = systemTreeParentIdOfClassType( alldata, partner, parent_node.class_id );
#ifdef OTFPROFILE_MPI
                alldata.collectiveMapPerSystemTreeNodePairReduce[ Triple( node.parent, partner_parent, collOp ) ].add( data );
#else //OTFPROFILE_MPI
                alldata.collectiveMapPerSystemTreeNodePair[ Triple( node.parent, partner_parent, collOp ) ].add( data );
#endif //OTFPROFILE_MPI
                it_CollLb++;
            }
            
            /* add message data of location to parent node*/
            for (map< Pair, CollectiveData >::const_iterator it_Coll= alldata.collectiveMapPerSystemTreeNode.begin();
                  it_Coll != alldata.collectiveMapPerSystemTreeNode.end(); ++it_Coll)
            {
                if (it_Coll->first.a != node_id)
                    continue;
            
                /* get copy of message data in order to keep original data
                 unchanged for CSV output */
                CollectiveData data= it_Coll->second;

                alldata.collectiveMapPerSystemTreeNode[ Pair(node.parent, it_Coll->first.b) ].add( data );

            }
            
            parentChildCtrMap[node.parent]++;
            
            if ( parentChildCtrMap[node.parent] == parent_node.children.size() && 0 != node.parent ) {
                
                /* add parent to set of nodes that are "flagged" to propagate their statistic information to their parents */
                tmp_systemTreeNodeSet.insert(node.parent);

            }
            
            /* delete current element */
            tmp_systemTreeNodeSet.erase(it_set);

        }
    }
    
    /* summarize map ( rank x func ) to map ( func ) */
    {
        map< Pair, FunctionData, ltPair >::const_iterator it= alldata.functionMapPerRank.begin();
        map< Pair, FunctionData, ltPair >::const_iterator itend= alldata.functionMapPerRank.end();
        while ( itend != it ) {

            const uint64_t& func= it->first.b;

            alldata.functionMapGlobal[ func ].add( it->second );
            it++;

        }

        /* in case of additional clustering or producing CSV output do not
        clear map ( rank x func ) because it is needed later */
        if ( !alldata.params.clustering.enabled &&
             !alldata.params.create_csv  && 
             !alldata.params.create_cube &&
             !alldata.params.create_circos) {
                
            alldata.functionMapPerRank.clear();

        }

        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH)
        {
            map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator it= alldata.functionCallpathMapPerRank.begin();
            map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator itend= alldata.functionCallpathMapPerRank.end();
            while ( itend != it ) {

                const uint64_t& func= it->first.c;
                const string callpath= it->first.b;

                alldata.functionCallpathMapGlobal[ PairCallpath(func,callpath) ].add( it->second );
                it++;

            }

            /* in case of additional clustering or producing CSV output do not
            clear map ( rank x func ) because it is needed later */
            if ( !alldata.params.clustering.enabled &&
                 !alldata.params.create_csv) {

                alldata.functionCallpathMapPerRank.clear();

            }
        }
    }
  
    /* summarize map ( rank x func x counter ) to map ( counter x func ) */
    {
        map< Triple, CounterData, ltTriple >::const_iterator it= alldata.counterMapPerFunctionRank.begin();
        map< Triple, CounterData, ltTriple >::const_iterator itend= alldata.counterMapPerFunctionRank.end();
        while ( itend != it ) {

            const uint64_t& func= it->first.b;
            const uint64_t& counter= it->first.c;

            alldata.counterMapGlobal[ Pair( counter, func ) ].add( it->second );
            it++;

        }

        /* in case of producing CSV output do not clear
        map ( rank x func x counter ) because it is needed later */
        if ( !alldata.params.create_csv  && 
             !alldata.params.create_cube &&
             !alldata.params.create_circos) {

            alldata.counterMapPerFunctionRank.clear();

        }
    }

    /* summarize map ( rank x rank ) to map ( group x group ) */
    {
        map< Pair, MessageData, ltPair >::const_iterator it= alldata.messageMapPerRankPair.begin();
        map< Pair, MessageData, ltPair >::const_iterator itend= alldata.messageMapPerRankPair.end();
        while ( itend != it ) {

            uint64_t group_a= it->first.a;
            uint64_t group_b= it->first.b;

            /* get copy of message data in order to keep original data
            unchanged for CSV output */
            MessageData data= it->second;

            if ( alldata.grouping.enabled ) {

                /* convert process IDs to group IDs */

                group_a= alldata.grouping.process2group( group_a );
                assert( 0 != group_a );
                group_b= alldata.grouping.process2group( group_b );
                assert( 0 != group_b );

            }

            alldata.messageMapPerGroupPair[ Pair( group_a, group_b ) ].add( data );
            it++;

        }

        if ( !alldata.params.create_csv  && 
             !alldata.params.create_html &&
             !alldata.params.create_cube &&
             !alldata.params.create_circos) {

            alldata.messageMapPerRankPair.clear();

        }
    }

    /* summarize map ( rank ) to map ( group ) */
    {
        map< uint64_t, MessageData >::const_iterator it= alldata.messageMapPerRank.begin();
        map< uint64_t, MessageData >::const_iterator itend= alldata.messageMapPerRank.end();
        while ( itend != it ) {

            uint64_t group= it->first;

            /* get copy of message data in order to keep original data
            unchanged for CSV output */
            MessageData data= it->second;

            if ( alldata.grouping.enabled ) {

                /* convert process ID to group ID */
                group= alldata.grouping.process2group( group );
                assert( 0 != group );

            }

            alldata.messageMapPerGroup[ group ].add( data );
            it++;

        }

        /* in case of producing CSV output do not clear map ( rank )
        because it is needed later */
        if ( !alldata.params.create_csv  &&
             !alldata.params.create_html &&
             !alldata.params.create_cube &&
             !alldata.params.create_circos) {

            alldata.messageMapPerRank.clear();

        }

    }
    
    /* summarize map ( rank x class ) to map ( class x group ) */
    {
        map< Pair, CollectiveData, ltPair >::iterator it= alldata.collectiveMapPerRank.begin();
        map< Pair, CollectiveData, ltPair >::iterator itend= alldata.collectiveMapPerRank.end();
        while ( itend != it ) {

            uint64_t group= it->first.a;
            const uint64_t& op_class= it->first.b;

            /* get copy of collective op. data in order to keep original data
            unchanged for CSV output */
            CollectiveData data= it->second;

            if ( alldata.grouping.enabled ) {

                /* convert process ID to group ID */
                group= alldata.grouping.process2group( group );
                assert( 0 != group );

            }

            MINMAX2SUM( data.count_send );
            MINMAX2SUM( data.count_recv );
            MINMAX2SUM( data.bytes_send );
            MINMAX2SUM( data.bytes_recv );
            MINMAX2SUM( data.duration_send );
            MINMAX2SUM( data.duration_recv );

            alldata.collectiveMapPerGroup[ Pair( op_class, group ) ].add( data );
            it++;

        }

        /* in case of producing CSV output do not clear map ( class x rank )
        because it is needed later */
        if ( !alldata.params.create_csv  && 
             !alldata.params.create_html &&
             !alldata.params.create_cube &&
             !alldata.params.create_circos) {

            alldata.collectiveMapPerRank.clear();

        }

    }

#ifdef OTFPROFILE_MPI
    /* synchronize error indicator with workers */
    /*SyncError( alldata, error );*/
#endif /* OTFPROFILE_MPI */

    if ( !error ) {

        /* stop runtime measurement for summarizing data */
        StopMeasurement( alldata, true, "summarize data" );

    }

    return !error;
}

bool SummarizeDataDispersion( AllData& alldata ) {
    
    bool error= false;
    
    /* start runtime measurement for summarizing dispersion data */
    StartMeasurement( alldata, 1, true, "summarize dispersion data" );
    
    VerbosePrint( alldata, 1, true, "summarize dispersion data\n" );
    
    /* summarize map ( rank x func x bin ) to map (  func x bin ) */
    {
        map< Triple, FunctionData, ltTriple>::const_iterator it= alldata.functionDurationSectionMapPerRank.begin();
        map< Triple, FunctionData, ltTriple>::const_iterator itend= alldata.functionDurationSectionMapPerRank.end();
        while ( itend != it ) {
     
            const uint64_t& func= it->first.b;
            const uint64_t& bin= it->first.c;
     
            alldata.functionDurationSectionMapGlobal[ Pair( func, bin ) ].add( it->second );
            it++;

        }

        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH)
        {

            map< Quadruple, FunctionData, ltQuadruple>::const_iterator itc= alldata.functionDurationSectionCallpathMapPerRank.begin();
            map< Quadruple, FunctionData, ltQuadruple>::const_iterator itendc= alldata.functionDurationSectionCallpathMapPerRank.end();
            while ( itendc != itc ) {

                const uint64_t& func= itc->first.b;
                const string callpath= itc->first.c;
                const uint64_t& bin= itc->first.d;

                alldata.functionDurationSectionCallpathMapGlobal[ TripleCallpath( func, callpath, bin ) ].add( itc->second );
                itc++;

            }

        }
        /* in case of producing CSV output do not clear map ( rank x func x bin )
         because it is needed later */
        if ( !alldata.params.create_csv ) {    
            
            alldata.functionDurationSectionMapPerRank.clear();
            alldata.functionDurationSectionCallpathMapPerRank.clear();

        }

    }

#ifdef OTFPROFILE_MPI
    /* synchronize error indicator with workers */
    /*SyncError( alldata, error );*/
#endif /* OTFPROFILE_MPI */
    
    //cerr << " Size of functionDurationSectionMapGlobal: " << alldata.functionDurationSectionMapGlobal.size() << endl;
    
    if ( !error ) {
        
        /* stop runtime measurement for summarizing dispersion data */
        StopMeasurement( alldata, true, "summarize dispersion data" );

    }
    
    return !error;
}

