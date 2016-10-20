/*
 This is part of the OTF library. Copyright by ZIH, TU Dresden 2005-2013.
 Authors: Andreas Knuepfer, Robert Dietrich, Matthias Jurenz
*/

#include <cassert>
#include <iostream>
#include <stdio.h>

#include "otfprofile.h"
#include "reduce_data.h"

#include "datamap_reduce.h"


using namespace std;


/* fence between statistics parts within the buffer for consistency checking */
enum { FENCE= 0xDEADBEEF };

enum {
   PACK_TOTAL_SIZE                   = 0,
   PACK_FUNCTION                     = 1,
   PACK_FUNCTION_DURATION            = 2,
   PACK_COUNTER                      = 3,
   PACK_MSG_GROUP_PAIR               = 4,
   PACK_MSG_GROUP                    = 5,
   PACK_MSG_SPEED                    = 6,
   PACK_COLLECTIVE_GROUP             = 7,
   PACK_FUNCTION_RANK                = 8,
   PACK_FUNCTION_MIN_MAX             = 9,
   PACK_FUNCTION_DURATION_CALLPATH   = 10,
   PACK_FUNCTION_CALLPATH            = 11,
   PACK_MSG_SIZE                     = 12,
   PACK_FUNCTION_TREENODE            = 13,
   PACK_COLLECTIVE_STAT          	 = 14,
   PACK_P2P_STAT       				 = 15,
   PACK_CALL_PATH_TREE               = 16,
   PACK_CALL_PATH_MAPPING			 = 17,
   PACK_CALL_PATH_FDATA_CPTNODE      = 18,
   PACK_CALL_PATH_P2P_DATA           = 19,
   PACK_CALL_PATH_COLLOP_DATA        = 20,
   PACK_CALL_PATH_RMA_DATA           = 21,
   PACK_CIRCOS_MSG_SYS_NODE          = 22,
   PACK_NUM_PACKS					 = 23
    
};


/* pack the local alldata into a buffer, return buffer */
static char* pack_worker_data( AllData& alldata, uint32_t sizes[PACK_NUM_PACKS] ) {

    uint64_t fence= FENCE;
    uint32_t num_fences= 1;

    /* get the sizes of all parts that need to be transmitted */

    for ( uint32_t i= 1; i < PACK_NUM_PACKS; i++ ) {

        sizes[i]= 0;

    }

    if ( alldata.params.create_tex ) {

        /* map< uint64_t, FunctionData > functionMapGlobal; */
        sizes[PACK_FUNCTION]          = alldata.functionMapGlobal.size(); 
        num_fences++;

        /* map< Pair, FunctionData > functionDurationSectionMapGlobal; */
        sizes[PACK_FUNCTION_DURATION] = alldata.functionDurationSectionMapGlobal.size(); 
        num_fences++;

        /* map< Pair, CounterData, ltPair > counterMapGlobal; */
        sizes[PACK_COUNTER]           = alldata.counterMapGlobal.size(); 
        num_fences++;

        /* map< Pair, MessageData, ltPair > messageMapPerGroupPair; */
        sizes[PACK_MSG_GROUP_PAIR]    = alldata.messageMapPerGroupPair.size(); 
        num_fences++;

        /* map< uint64_t, MessageData > messageMapPerGroup; */
        sizes[PACK_MSG_GROUP]         = alldata.messageMapPerGroup.size(); 
        num_fences++;

        /* map< Pair, MessageSpeedData, ltPair > messageSpeedMapPerLength; */
        sizes[PACK_MSG_SPEED]         = alldata.messageSpeedMapPerLength.size(); 
        num_fences++;

        /* map< Pair, CollectiveData, ltPair > collectiveMapPerGroup; */
        sizes[PACK_COLLECTIVE_GROUP]  = alldata.collectiveMapPerGroup.size(); 
        num_fences++;
		
        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH)
        {

            /* map< TripleCallpath, FunctionData, ltTripleCallpath > functionDurationSectionCallpathMapGlobal; */
            sizes[PACK_FUNCTION_DURATION_CALLPATH] = alldata.functionDurationSectionCallpathMapGlobal.size(); 
            num_fences++;

            /* map< PairCallpath, FunctionData, ltPairCallpath > */
            sizes[PACK_FUNCTION_CALLPATH]          = alldata.functionCallpathMapGlobal.size(); 
            num_fences++;

        }

    }

    if ( alldata.params.clustering.enabled ) {

        /* map< Pair, FunctionData, ltPair > */
        sizes[PACK_FUNCTION_RANK]= alldata.functionMapPerRank.size(); 
        num_fences++;

    }

    if ( alldata.params.dispersion.enabled) {
        
        /* map< uint64_t, FunctionMinMaxLocactionData > functionMinMaxLocationMap; */
        sizes[PACK_FUNCTION_MIN_MAX]= alldata.functionMinMaxLocationMap.size(); 
        num_fences++;

    }

    if ( alldata.params.write_csv_msg_sizes ) {

       /* map< uint64_t, MessageSizeData > */ 
       sizes[PACK_MSG_SIZE] = alldata.messageMapPerSize.size(); 
       num_fences++;

    }

    if ( alldata.params.create_circos || alldata.params.create_tex ) {

        /* map< Pair, FunctionData, ltPair > */
        sizes[PACK_FUNCTION_TREENODE] = alldata.functionMapPerSystemTreeNodeReduce.size(); 
        num_fences++;
        
        /* map< Triple, CollectiveData, ltTriple > */
        sizes[PACK_COLLECTIVE_STAT] = alldata.collectiveMapPerSystemTreeNodePairReduce.size(); 
        num_fences++;

        /* map< Pair, MessageData, ltPair > */
        sizes[PACK_P2P_STAT] = alldata.messageMapPerSystemTreeNodePairReduce.size(); 
        num_fences++;

    }

	if ( alldata.params.create_cube || alldata.params.create_circos ) {

        /* map< uint64_t, CallPathTreeNode> */
		sizes[PACK_CALL_PATH_TREE]	  = alldata.callPathTree.tree.size(); 
		num_fences++;

        /* map< std::pair< uint64_t, uint64_t >, uint64_t > */
		sizes[PACK_CALL_PATH_MAPPING] = alldata.callPathTree.mapping.size(); 
		num_fences++;

        /* map< std::pair< uint64_t, uint64_t >, FunctionData > */
        sizes[PACK_CALL_PATH_FDATA_CPTNODE] = alldata.functionDataPerCPTNode.size(); 
        num_fences++;

        /* map< std::pair< uint64_t, uint64_t >, MessageData > */
        sizes[PACK_CALL_PATH_P2P_DATA] = alldata.MessageDataPerNode.size(); 
        num_fences++;

        /* map< std::pair< uint64_t, uint64_t >, CollopDataPerNode > */
        sizes[PACK_CALL_PATH_COLLOP_DATA] = alldata.CollopDataPerNode.size(); 
        num_fences++;

        /* map< std::pair< uint64_t, uint64_t >, RmaData > */
        sizes[PACK_CALL_PATH_RMA_DATA] = alldata.RmaMsgData.size(); 
        num_fences++;

	}

    if ( alldata.params.create_circos ) {

        sizes[PACK_CIRCOS_MSG_SYS_NODE] = alldata.messageMapPerSystemTreeNode.size();
        num_fences++;

    }

    /* get bytesize multiplying all pieces */

    uint32_t bytesize= 0;
    int s1, s2;

    MPI_Pack_size( num_fences, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    bytesize += s1;
    
    MPI_Pack_size( sizes[PACK_FUNCTION] * 7, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_FUNCTION] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_FUNCTION_DURATION] * 8, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_FUNCTION_DURATION] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;
    
    MPI_Pack_size( sizes[PACK_COUNTER] * 8, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_COUNTER] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_MSG_GROUP_PAIR] * 20, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_MSG_GROUP_PAIR] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_MSG_GROUP] * 19, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_MSG_GROUP] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_MSG_SPEED] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    bytesize += s1;

    MPI_Pack_size( sizes[PACK_COLLECTIVE_GROUP] * 20, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_COLLECTIVE_GROUP] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_FUNCTION_RANK] * 8, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_FUNCTION_RANK] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_FUNCTION_MIN_MAX] * 7, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_FUNCTION_MIN_MAX] * 0, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    MPI_Pack_size( sizes[PACK_FUNCTION_DURATION_CALLPATH] * 9, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
    MPI_Pack_size( sizes[PACK_FUNCTION_DURATION_CALLPATH] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
    bytesize += s1 + s2;

    if( alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH ) {

        map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator it=    alldata.functionDurationSectionCallpathMapGlobal.begin();
        map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator itend= alldata.functionDurationSectionCallpathMapGlobal.end();

        for ( ; it != itend; ++it ) {

            MPI_Pack_size( it->first.b.length(), MPI_CHAR, MPI_COMM_WORLD, &s1 );
            bytesize += s1;

        }

        MPI_Pack_size( sizes[PACK_FUNCTION_CALLPATH] * 8, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        MPI_Pack_size( sizes[PACK_FUNCTION_CALLPATH] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
        bytesize += s1 + s2;

        {
            map< PairCallpath, FunctionData, ltPairCallpath >::const_iterator it=    alldata.functionCallpathMapGlobal.begin();
            map< PairCallpath, FunctionData, ltPairCallpath >::const_iterator itend= alldata.functionCallpathMapGlobal.end();
            for ( ; it != itend; ++it ) {

                MPI_Pack_size( it->second.callpath.length(), MPI_CHAR, MPI_COMM_WORLD, &s1 );
                bytesize += s1;

            }

        }

    }

    if ( alldata.params.write_csv_msg_sizes ) {

       MPI_Pack_size( sizes[PACK_MSG_SIZE] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
       bytesize += s1;

    }

    if ( alldata.params.create_circos || alldata.params.create_tex ) {
            
        MPI_Pack_size( sizes[PACK_FUNCTION_TREENODE] * 8, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        MPI_Pack_size( sizes[PACK_FUNCTION_TREENODE] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
        bytesize += s1 + s2;

        MPI_Pack_size( sizes[PACK_COLLECTIVE_STAT] * 21, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        MPI_Pack_size( sizes[PACK_COLLECTIVE_STAT] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
        bytesize += s1 + s2;
    
        MPI_Pack_size( sizes[PACK_P2P_STAT] * 30, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        MPI_Pack_size( sizes[PACK_P2P_STAT] * 6, MPI_DOUBLE, MPI_COMM_WORLD, &s2 );
        bytesize += s1 + s2;
    
    }

	if ( alldata.params.create_cube || alldata.params.create_circos ) {

		MPI_Pack_size( sizes[PACK_CALL_PATH_TREE] * 2, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
		bytesize += s1;

		MPI_Pack_size( sizes[PACK_CALL_PATH_MAPPING] * 3, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
		bytesize += s1;

        MPI_Pack_size( sizes[PACK_CALL_PATH_FDATA_CPTNODE] * 4, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        bytesize += s1;
        MPI_Pack_size( sizes[PACK_CALL_PATH_FDATA_CPTNODE] * 2, MPI_DOUBLE, MPI_COMM_WORLD, &s1 );
        bytesize += s1;

        MPI_Pack_size( sizes[PACK_CALL_PATH_P2P_DATA] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        bytesize += s1;
        
        MPI_Pack_size( sizes[PACK_CALL_PATH_COLLOP_DATA] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        bytesize += s1;

        MPI_Pack_size( sizes[PACK_CALL_PATH_RMA_DATA] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        bytesize += s1;

	}

    if ( alldata.params.create_circos ) {

        MPI_Pack_size( sizes[PACK_CIRCOS_MSG_SYS_NODE] * 35, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1 );
        bytesize += s1;

    }

    /* get the buffer */
    sizes[PACK_TOTAL_SIZE]= bytesize;
    char* buffer= alldata.guaranteePackBuffer( bytesize );

    /* pack parts */
    int position= 0;

    /* extra check that doesn't cost too much */
    MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    if ( alldata.params.create_tex && alldata.params.create_cube == false ) {

        /* pack functionMapGlobal */
        {

            map< uint64_t, FunctionData >::const_iterator it=    alldata.functionMapGlobal.begin();
            map< uint64_t, FunctionData >::const_iterator itend= alldata.functionMapGlobal.end();
            for ( ; it != itend; ++it ) {

                MPI_Pack( (void*) &it->first,                1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.functionMapGlobal.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH)
        {

                /* pack functionCallpathMapGlobal */
                {

                    map< PairCallpath, FunctionData, ltPairCallpath >::const_iterator it=   alldata.functionCallpathMapGlobal.begin();
                    map< PairCallpath, FunctionData, ltPairCallpath>::const_iterator itend= alldata.functionCallpathMapGlobal.end();
                    uint64_t len;
                    for ( ; it != itend; ++it ) {

                    	len = it->second.callpath.length();
                        MPI_Pack( (void*) &it->first.a,               1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                        MPI_Pack( (void*) &len,                       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                        MPI_Pack( (void*) &it->second.count.min,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                        MPI_Pack( (void*) &it->second.count.max,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                        MPI_Pack( (void*) &it->second.count.sum,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                        MPI_Pack( (void*) &it->second.count.cnt,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                         MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                         MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                         MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                         MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                         MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                         MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                         MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                         MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                         MPI_Pack( (void*) it->first.b.c_str(), len, MPI_CHAR, buffer, bytesize, &position, MPI_COMM_WORLD );

                     }

                    alldata.functionCallpathMapGlobal.clear();
                }

                /* extra check that doesn't cost too much */
                MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        }

        /* pack functionDurationSectionMapGlobal */
        {

            map< Pair, FunctionData, ltPair >::const_iterator it=    alldata.functionDurationSectionMapGlobal.begin();
            map< Pair, FunctionData, ltPair >::const_iterator itend= alldata.functionDurationSectionMapGlobal.end();

            for ( ; it != itend; ++it ) {
                
                MPI_Pack( (void*) &it->first.a,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.functionDurationSectionMapGlobal.clear();           
            
        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
        
        /* pack counterMapGlobal */
        {

            map< Pair, CounterData, ltPair >::const_iterator it=    alldata.counterMapGlobal.begin();
            map< Pair, CounterData, ltPair >::const_iterator itend= alldata.counterMapGlobal.end();

            for ( ; it != itend; ++it ) {

                MPI_Pack( (void*) &it->first.a,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.counterMapGlobal.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack messageMapPerGroupPair  */
        {

            map< Pair, MessageData, ltPair >::const_iterator it=    alldata.messageMapPerGroupPair.begin();
            map< Pair, MessageData, ltPair >::const_iterator itend= alldata.messageMapPerGroupPair.end();

            for ( ; it != itend; ++it ) {

                MPI_Pack( (void*) &it->first.a,                  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,                  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count_send.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count_recv.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.bytes_send.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.bytes_recv.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.duration_send.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.duration_recv.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.messageMapPerGroupPair.clear();
        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack messageMapPerGroup  */
        {

            map< uint64_t, MessageData >::const_iterator it=    alldata.messageMapPerGroup.begin();
            map< uint64_t, MessageData >::const_iterator itend= alldata.messageMapPerGroup.end();

            for ( ; it != itend; ++it ) {

                MPI_Pack( (void*) &it->first,                    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count_send.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count_recv.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.bytes_send.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.bytes_recv.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.duration_send.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.duration_recv.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.messageMapPerGroup.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack messageSpeedMapPerLength */
        {

            map< Pair, MessageSpeedData, ltPair >::const_iterator it=    alldata.messageSpeedMapPerLength.begin();
            map< Pair, MessageSpeedData, ltPair >::const_iterator itend= alldata.messageSpeedMapPerLength.end();

            for ( ; it != itend; ++it ) {

                MPI_Pack( (void*) &it->first.a,          1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,          1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count.min, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.max, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.sum, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.messageSpeedMapPerLength.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack collectiveMapPerGroup */
        {

            map< Pair, CollectiveData, ltPair >::const_iterator it=    alldata.collectiveMapPerGroup.begin();
            map< Pair, CollectiveData, ltPair >::const_iterator itend= alldata.collectiveMapPerGroup.end();

            for ( ; it != itend; ++it ) {

                MPI_Pack( (void*) &it->first.a,                  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,                  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count_send.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.count_recv.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.bytes_send.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.bytes_recv.min,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.max,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.sum,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.duration_send.min, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.max, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.sum, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.duration_recv.min, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.max, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.sum, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

            alldata.collectiveMapPerGroup.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );


        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH)
        {
            /* pack functionDurationSectionCallpathMapGlobal*/

            {

                map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator it=    alldata.functionDurationSectionCallpathMapGlobal.begin();
                map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator itend= alldata.functionDurationSectionCallpathMapGlobal.end();
                uint64_t len = 0;

                for ( ; it != itend; ++it ) {

                    len = it->second.callpath.length();
                    MPI_Pack( (void*) &it->first.a,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &len,                      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->first.c,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                    MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                    MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                    MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                    MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                    MPI_Pack( (void*) it->second.callpath.c_str(), len, MPI_CHAR, buffer, bytesize, &position, MPI_COMM_WORLD );

                }

                alldata.functionDurationSectionMapGlobal.clear();

            }

            /* extra check that doesn't cost too much */
            MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        }

    }

    if ( alldata.params.clustering.enabled ) {

        /* pack functionMapPerRank */

        map< Pair, FunctionData, ltPair >::const_iterator it= alldata.functionMapPerRank.begin();
        map< Pair, FunctionData, ltPair >::const_iterator itend= alldata.functionMapPerRank.end();

        for ( ; it != itend; ++it ) {

            MPI_Pack( (void*) &it->first.a,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->first.b,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        }

        /* in case of producing CSV output do not clear map because it is
        needed later */
        if ( !alldata.params.create_csv || 
            ( alldata.params.clustering.enabled && alldata.myRank > 0 ) ) {

            alldata.functionMapPerRank.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    }

    if ( alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH ) {

        /* pack functionCallpathMapPerRank */

        map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator it= alldata.functionCallpathMapPerRank.begin();
        map< TripleCallpath, FunctionData, ltTripleCallpath >::const_iterator itend= alldata.functionCallpathMapPerRank.end();
        uint64_t len=0;

        for ( ; it != itend; ++it ) {

        	len = it->first.b.length();

            MPI_Pack( (void*) &it->first.a,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &len,                      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->first.c,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            MPI_Pack( (void*) it->first.b.c_str(), len, MPI_CHAR, buffer, bytesize, &position, MPI_COMM_WORLD );

        }

        /* in case of producing CSV output do not clear map because it is
        needed later */
        if ( !alldata.params.create_csv ) {

            alldata.functionCallpathMapPerRank.clear();

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    }
    
    if ( alldata.params.dispersion.enabled ) {
        
        /* pack functionMinMaxLocationMap */
        
        map<uint64_t, FunctionMinMaxLocationData>::const_iterator it= alldata.functionMinMaxLocationMap.begin();
        map<uint64_t, FunctionMinMaxLocationData>::const_iterator itend= alldata.functionMinMaxLocationMap.end();
        
        for ( ; it != itend ; ++it ) {
            
            MPI_Pack( (void*) &it->first,                    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.location.min,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.location.max,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.location.loc_min,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.location.loc_max,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.location.time_max, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
            MPI_Pack( (void*) &it->second.location.time_max, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        }
        
        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    }

    if ( alldata.params.write_csv_msg_sizes ) {
        /* pack messageMapPerSize */

        for ( map<uint64_t, MessageSizeData>::const_iterator it = alldata.messageMapPerSize.begin();
              it != alldata.messageMapPerSize.end();
              it++ )
        {

           MPI_Pack( (void*) &it->first,           1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
           MPI_Pack( (void*) &it->second.count,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
           MPI_Pack( (void*) &it->second.time.min, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
           MPI_Pack( (void*) &it->second.time.max, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
           MPI_Pack( (void*) &it->second.time.sum, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
           MPI_Pack( (void*) &it->second.time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        }

        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    }

    if ( alldata.params.create_circos || alldata.params.create_tex ) {
              
        /* pack functionMapPerSystemTreeNode */
        {
            map< Pair, FunctionData, ltPair >::iterator it    = alldata.functionMapPerSystemTreeNodeReduce.begin();
            map< Pair, FunctionData, ltPair >::iterator itend = alldata.functionMapPerSystemTreeNodeReduce.end();
            
            for( ; it != itend; ++it) {

                MPI_Pack( (void*) &it->first.a,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,              1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count.min,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.max,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.sum,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count.cnt,     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.excl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.excl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it->second.incl_time.min, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.max, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.sum, 1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.incl_time.cnt, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

        }
        
        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
        
        /* pack collectiveMapPerSystemTreeNodePair */
        {
            map< Triple, CollectiveData, ltTriple >::iterator it    = alldata.collectiveMapPerSystemTreeNodePairReduce.begin();
            map< Triple, CollectiveData, ltTriple >::iterator itend = alldata.collectiveMapPerSystemTreeNodePairReduce.end();
            
            for( ; it != itend; ++it) {
                
                MPI_Pack( (void*) &it->first.a,                     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,                     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.c,                     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count_send.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count_recv.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.bytes_send.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.bytes_recv.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.duration_send.min,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.max,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.sum,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.duration_recv.min,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.max,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.sum,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

        }
        
        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
        
        /* pack collectiveMapPerSystemTreeNodePair */
        {
            map< Pair, MessageData, ltPair >::iterator it    = alldata.messageMapPerSystemTreeNodePairReduce.begin();
            map< Pair, MessageData, ltPair >::iterator itend = alldata.messageMapPerSystemTreeNodePairReduce.end();
            
            for( ; it != itend; ++it) {
                
                MPI_Pack( (void*) &it->first.a,                     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->first.b,                     1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count_send.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count_recv.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.bytes_send.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.bytes_recv.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.duration_send.min,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.max,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.sum,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.duration_recv.min,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.max,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.sum,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                for( int i = 0; i < 10; i++ ) {

                    MPI_Pack( (void*) &it->second.msgSizes.hist[i], 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                }
                
            }

        }
        
        /* extra check that doesn't cost too much */
        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    }

	if ( alldata.params.create_cube || alldata.params.create_circos ) {

		/* pack callPathTree.tree */
		{

			map< uint64_t, CallPathTreeNode>::iterator it = alldata.callPathTree.tree.begin();
			map< uint64_t, CallPathTreeNode>::iterator it_e = alldata.callPathTree.tree.end();

			for(; it != it_e; it++ ) {

				MPI_Pack( (void*) &it->second.function_id,	1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
				MPI_Pack( (void*) &it->second.parent_id, 	1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

			}

		}

		MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

		/* pack callPathTree.mapping */
		{

			map< pair< uint64_t, uint64_t>, uint64_t>::iterator itMap = alldata.callPathTree.mapping.begin();
			map< pair< uint64_t, uint64_t>, uint64_t>::iterator itMap_e = alldata.callPathTree.mapping.end();

			for(; itMap != itMap_e; itMap++ ) {

				MPI_Pack( (void*)  &itMap->first.first, 	1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
				MPI_Pack( (void*)  &itMap->first.second, 	1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
				MPI_Pack( (void*)  &itMap->second, 		    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

			}

		}

	    MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack functionDataPerCPTNode */
        {

            map< pair< uint64_t, uint64_t >, FunctionData>::iterator it_funky = alldata.functionDataPerCPTNode.begin();
            map< pair< uint64_t, uint64_t >, FunctionData>::iterator it_funky_e = alldata.functionDataPerCPTNode.end();

            for(; it_funky != it_funky_e; it_funky++ ) {

                MPI_Pack( (void*) &it_funky->first.first,           1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_funky->first.second,          1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it_funky->second.start_nid,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_funky->second.count.cnt,      1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_funky->second.excl_time.sum,  1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_funky->second.incl_time.sum,  1, MPI_DOUBLE,        buffer, bytesize, &position, MPI_COMM_WORLD );

            }

        }

	    MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack MessageDataPerNode (call path tree) */
        {

            map< pair< uint64_t, uint64_t >, MessageData >::iterator it_mdat = alldata.MessageDataPerNode.begin();
            map< pair< uint64_t, uint64_t >, MessageData >::iterator it_mdat_e = alldata.MessageDataPerNode.end();

            for(; it_mdat != it_mdat_e; it_mdat++ ) {

                MPI_Pack( (void*) &it_mdat->first.first,            1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_mdat->first.second,           1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it_mdat->second.count_send.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_mdat->second.bytes_send.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_mdat->second.count_recv.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_mdat->second.bytes_recv.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD ); 

            }

        }

	    MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack CollopDataPerNode (call path tree) */
        {

            map< pair< uint64_t, uint64_t >, MessageData >::iterator it_cold = alldata.CollopDataPerNode.begin();
            map< pair< uint64_t, uint64_t >, MessageData >::iterator it_cold_e = alldata.CollopDataPerNode.end();

            for(; it_cold != it_cold_e; it_cold++ ) {

                MPI_Pack( (void*) &it_cold->first.first,            1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_cold->first.second,           1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                MPI_Pack( (void*) &it_cold->second.count_send.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_cold->second.count_recv.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_cold->second.bytes_send.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it_cold->second.bytes_recv.sum,  1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

            }

        }

	    MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

        /* pack RmaMsgData */
		{

			map< pair< uint64_t, uint64_t>, RmaData>::iterator itRma = alldata.RmaMsgData.begin();
			map< pair< uint64_t, uint64_t>, RmaData>::iterator itRma_e = alldata.RmaMsgData.end();

			for(; itRma != itRma_e; itRma++ ) {

				MPI_Pack( (void*)  &itRma->first.first, 	        1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
				MPI_Pack( (void*)  &itRma->first.second, 	        1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

				MPI_Pack( (void*)  &itRma->second.rma_put_cnt, 		1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
				MPI_Pack( (void*)  &itRma->second.rma_get_cnt, 		1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*)  &itRma->second.rma_put_bytes,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
				MPI_Pack( (void*)  &itRma->second.rma_get_bytes,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

			}

		}

	    MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

	}

    /* messageMapPerRankPair - only ranks and send/recv sum*/
    if ( alldata.params.create_circos ) {

        /* pack collectiveMapPerSystemTreeNode */
        {

            map< uint64_t, MessageData >::iterator it    = alldata.messageMapPerSystemTreeNode.begin();
            map< uint64_t, MessageData >::iterator itend = alldata.messageMapPerSystemTreeNode.end();
            
            for( ; it != itend; ++it) {
                
                MPI_Pack( (void*) &it->first,                       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count_send.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_send.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.count_recv.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.count_recv.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.bytes_send.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_send.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.bytes_recv.min,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.max,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.sum,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.bytes_recv.cnt,       1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.duration_send.min,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.max,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.sum,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_send.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );
                
                MPI_Pack( (void*) &it->second.duration_recv.min,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.max,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.sum,    1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD );
                MPI_Pack( (void*) &it->second.duration_recv.cnt,    1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                for( int i = 0; i < 10; i++ ) { 

                    MPI_Pack( (void*) &it->second.msgSizes.hist[i], 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

                }
                
            }

        }

        MPI_Pack( (void*) &fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD );

    }

   
    return buffer;
}


/* prepare alldata for unpack, return buffer of sufficient size */
static char* prepare_worker_data( AllData& alldata, uint32_t sizes[PACK_NUM_PACKS] ) {

    uint32_t bytesize= sizes[PACK_TOTAL_SIZE];

    return alldata.guaranteePackBuffer( bytesize );

}

/* unpack the received worker data and add it to the local alldata */
static void unpack_worker_data( AllData& alldata, uint32_t sizes[PACK_NUM_PACKS] ) {

    uint64_t fence;

    /* unpack parts */
    int position= 0;
    char* buffer= alldata.getPackBuffer( );

    /* extra check that doesn't cost too much */
    fence= 0;
    MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
    assert( FENCE == fence );

    /* chararray for unpacking the callpath */
    char* callpath = (char*) malloc(alldata.maxCallpathLength * sizeof(char));


    if ( alldata.params.create_tex && alldata.params.create_cube == false ) {

        /* unpack functionMapGlobal */
        for ( uint32_t i= 0; i < sizes[PACK_FUNCTION]; i++ ) {

            uint64_t func;
            FunctionData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &func,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.functionMapGlobal[ func ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH) {

            /* unpack functionCallpathMapGlobal */
            for ( uint32_t i= 0; i < sizes[PACK_FUNCTION_CALLPATH]; i++ ) {

                uint64_t func;
                uint64_t len;
                FunctionData tmp;

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &func,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &len,               1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, callpath, len, MPI_CHAR, MPI_COMM_WORLD );

                tmp.callpath = callpath;
                tmp.callpath = tmp.callpath.substr (0,len);
                alldata.functionCallpathMapGlobal[ PairCallpath(func,tmp.callpath) ].add( tmp );

            }

            /* extra check that doesn't cost too much */
            fence= 0;
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            assert( FENCE == fence );

        }

        /* unpack functionDurationSectionMapGlobal */
        for ( uint32_t i= 0; i < sizes[PACK_FUNCTION_DURATION]; i++ ) {
            
            uint64_t func;
            uint64_t bin;
            FunctionData tmp;
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &func,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &bin,               1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            alldata.functionDurationSectionMapGlobal[ Pair( func, bin ) ].add( tmp );

        }
        
        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );
        
        /* unpack counterMapGlobal */
        for ( uint32_t i= 0; i < sizes[PACK_COUNTER]; i++ ) {

            uint64_t a;
            uint64_t b;
            CounterData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &a,                 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b,                 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.counterMapGlobal[ Pair( a, b ) ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        /* unpack messageMapPerGroupPair */
        for ( uint32_t i= 0; i < sizes[PACK_MSG_GROUP_PAIR]; i++ ) {

            uint64_t a;
            uint64_t b;
            MessageData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &a,                     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &b,                     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.messageMapPerGroupPair[ Pair(a,b) ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        /* unpack messageMapPerGroup */
        for ( uint32_t i= 0; i < sizes[PACK_MSG_GROUP]; i++ ) {

            uint64_t a;
            MessageData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &a,                     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.messageMapPerGroup[ a ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        /* unpack messageSpeedMapPerLength */
        for ( uint32_t i= 0; i < sizes[PACK_MSG_SPEED]; i++ ) {

            uint64_t a;
            uint64_t b;
            MessageSpeedData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &a,             1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &b,             1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count.min, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count.max, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count.sum, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.messageSpeedMapPerLength[ Pair(a,b) ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        /* unpack collectiveMapPerGroup */
        for ( uint32_t i= 0; i < sizes[PACK_COLLECTIVE_GROUP]; i++ ) {

            uint64_t a;
            uint64_t b;
            CollectiveData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &a,                     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &b,                     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_send.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.count_recv.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_send.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.min,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.max,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.sum,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.bytes_recv.cnt,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_send.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &tmp.duration_recv.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.collectiveMapPerGroup[ Pair(a,b) ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH) {

            /* unpack functionDurationSectionCallpathMapGlobal */
            for ( uint32_t i= 0; i < sizes[PACK_FUNCTION_DURATION_CALLPATH]; i++ ) {

                uint64_t func;
                uint64_t bin;
                uint64_t len;
                FunctionData tmp;

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &func,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &len,               1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &bin,               1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, callpath, len, MPI_CHAR, MPI_COMM_WORLD );
                tmp.callpath = callpath;
                tmp.callpath = tmp.callpath.substr (0,len);
                alldata.functionDurationSectionCallpathMapGlobal[ TripleCallpath( func, tmp.callpath,bin ) ].add( tmp );

            }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        }

    }

    if ( alldata.params.clustering.enabled ) {

        /* unpack functionMapPerRank */
        for ( uint32_t i= 0; i < sizes[PACK_FUNCTION_RANK]; i++ ) {

            uint64_t a;
            uint64_t b;
            FunctionData tmp;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &a,        1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position,  (void*) &b,        1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            alldata.functionMapPerRank[ Pair(a,b) ].add( tmp );

        }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

    }

    if ( alldata.params.dispersion.enabled) {
        
        /* unpack functionMinMaxLocationMap */
        for ( uint32_t i= 0;  i < sizes[PACK_FUNCTION_MIN_MAX]; i++) {
            
            uint64_t a;
            FunctionMinMaxLocationData tmp;
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &a,                     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &tmp.location.min,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &tmp.location.max,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &tmp.location.loc_min,  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &tmp.location.loc_max,  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &tmp.location.time_min, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &tmp.location.time_max, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            alldata.functionMinMaxLocationMap[ a ].add(tmp);

        }
        
        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );      

    }
    
    if ( alldata.params.write_csv_msg_sizes ) {

       /* unpack messageMapPerSize */
       for ( uint32_t i = 0; i < sizes[PACK_MSG_SIZE]; i++ ) {
          uint64_t size, count;
          min_max_avg<uint64_t> time;
          MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &size,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
          MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &count,        1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
          MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &time.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
          MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &time.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
          MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &time.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
          MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &time.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

          alldata.messageMapPerSize[size].count += count;
          alldata.messageMapPerSize[size].time.add(time);

       }

        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );      

       
    }
    
    if ( alldata.params.create_circos || alldata.params.create_tex ) {
        
        /* unpack functionMapPerSystemTreeNode */
        for (uint32_t i= 0; i < sizes[PACK_FUNCTION_TREENODE]; i++ ) {
            
            uint64_t a;
            uint64_t b;
            FunctionData tmp;
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &a,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &b,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.excl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.min, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.max, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.sum, 1, MPI_DOUBLE,        MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.incl_time.cnt, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        
            alldata.functionMapPerSystemTreeNodeReduce[ Pair(a,b) ].add( tmp );
            
        }  
        
        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );     
        
        /* unpack functionMapPerSystemTreeNodeReduce */

        for (uint32_t i= 0; i < sizes[PACK_COLLECTIVE_STAT]; i++ ) {
            
            uint64_t a;
            uint64_t b;
            uint64_t c;
            
            CollectiveData tmp;
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &a,                  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &b,                  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &c,                  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.min,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.max,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.sum,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.cnt,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.min,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.max,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.sum,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.cnt,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
       
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.min,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.max,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.sum,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.cnt,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.min,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.max,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.sum,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.cnt,         1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.min,      1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.max,      1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.sum,      1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.cnt,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
       
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.min,      1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.max,      1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.sum,      1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.cnt,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            alldata.collectiveMapPerSystemTreeNodePairReduce[ Triple(a,b,c) ].add(tmp);  
        
        }
        
        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );  
        
        for (uint32_t i= 0; i < sizes[PACK_P2P_STAT]; i++ ) {
            
            uint64_t a;
            uint64_t b;

            msgSizeHistogram tmp_histo;

            MessageData tmp;
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &a,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &b,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
       
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.min,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.max,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.sum,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.cnt,  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
       
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.min,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.max,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.sum,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.cnt,  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            for( int i = 0; i < 10; i++ ) {

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp_histo.hist[i], 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            }

            tmp.msgSizes = tmp_histo;
            
            alldata.messageMapPerSystemTreeNodePairReduce[ Pair(a,b) ].add(tmp);  
        
        }
        
        /* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );  

    }

	/* unpack callPathTree with mapping, functiondata, messagedata and collop-data */
	if ( alldata.params.create_cube || alldata.params.create_circos ) { 

		CallPathTree tmp_tree;
        map< pair< uint64_t, uint64_t >, FunctionData > tmp_fData;
        map< pair< uint64_t, uint64_t >, MessageData > tmp_mData;
        map< pair< uint64_t, uint64_t >, MessageData > tmp_cData;
        map< pair< uint64_t, uint64_t >, RmaData > tmp_rmaData;

        for (uint32_t i = 0; i < sizes[PACK_CALL_PATH_TREE]; i++ ) {

			CallPathTreeNode tmp_node;

			MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp_node.function_id,	1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
			MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp_node.parent_id, 	1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

			tmp_tree.tree.insert( pair< uint64_t, CallPathTreeNode >( tmp_tree.tree.size() + 1, tmp_node ) );

		}

		/* extra check that doesn't cost too much */
        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

		for ( uint32_t i = 0; i < sizes[PACK_CALL_PATH_MAPPING]; i++ ) {

			uint64_t o_nid, rank, n_nid;

			MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &o_nid, 	1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
			MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &rank, 	1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

			MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &n_nid,	1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

			tmp_tree.mapping.insert( pair< pair < uint64_t, uint64_t >, uint64_t >( pair< uint64_t, uint64_t >( o_nid, rank), n_nid ) );		

		}
	
		mergeTree( alldata, tmp_tree );

		fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        for ( uint32_t i = 0; i < sizes[PACK_CALL_PATH_FDATA_CPTNODE]; i++ ) {

            uint64_t o_nid, fid, rank, cnt, s_nid;
            double ex_time, in_time;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &o_nid,  1, MPI_LONG_LONG_INT,   MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &rank,   1, MPI_LONG_LONG_INT,   MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &s_nid,  1, MPI_LONG_LONG_INT,   MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &cnt,    1, MPI_LONG_LONG_INT,   MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &ex_time,1, MPI_DOUBLE,          MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &in_time,1, MPI_DOUBLE,          MPI_COMM_WORLD );


            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( o_nid, rank );
            FunctionData insertMeToo = FunctionData(1, ex_time, in_time, s_nid );
            insertMeToo.count.cnt = cnt;
            tmp_fData.insert( pair< pair< uint64_t, uint64_t >, FunctionData >( insertMe, insertMeToo ) );

        }

        mergeData( alldata, tmp_fData, tmp_tree );

        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        for( uint32_t i = 0; i < sizes[PACK_CALL_PATH_P2P_DATA]; i++ ) {

            uint64_t o_nid, rank, c_send_s, b_send_s, c_recv_s, b_recv_s;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &o_nid,       1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &rank,        1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &c_send_s,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b_send_s,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &c_recv_s,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b_recv_s,    1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( o_nid, rank );
            MessageData insertMeToo = MessageData( c_send_s, b_send_s, 0, c_recv_s, b_recv_s, 0 );

            tmp_mData.insert( pair< pair<uint64_t, uint64_t>, MessageData >( insertMe, insertMeToo ) );

        }

        mergeMData( alldata, tmp_mData, tmp_tree );

        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

        for( uint32_t i = 0; i < sizes[PACK_CALL_PATH_COLLOP_DATA]; i++ ) {

            uint64_t o_nid, rank, c_send_s, c_recv_s, b_send_s, b_recv_s;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &o_nid,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &rank,       1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &c_send_s,   1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &c_recv_s,   1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b_send_s,   1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b_recv_s,   1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( o_nid, rank );
            MessageData insertMeToo = MessageData( c_send_s, b_send_s, 0, c_recv_s, b_recv_s, 0 );

            tmp_cData.insert( pair< pair<uint64_t, uint64_t>, MessageData >( insertMe, insertMeToo ) );

        }

        mergeCData( alldata, tmp_cData, tmp_tree );

        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );


        for( uint32_t i = 0; i < sizes[PACK_CALL_PATH_RMA_DATA]; i++ ) {

            uint64_t o_nid, rank, c_get, c_put, b_get, b_put;
            double time_p, time_g, time_u;

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &o_nid,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &rank,       1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &c_put,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &c_get,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b_put,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &b_get,      1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( o_nid, rank );
            RmaData insertMeToo;

            insertMeToo.rma_put_cnt = c_put;
            insertMeToo.rma_get_cnt = c_get; 

            insertMeToo.rma_put_bytes = b_put;
            insertMeToo.rma_get_bytes = b_get;

            tmp_rmaData.insert( pair< pair<uint64_t, uint64_t>, RmaData >( insertMe, insertMeToo ) );

        }

        mergeRmaData( alldata, tmp_rmaData, tmp_tree );

        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

	}

    if ( alldata.params.create_circos ) {

        for (uint32_t i= 0; i < sizes[PACK_CIRCOS_MSG_SYS_NODE]; i++ ) {
            
            uint64_t a;

            msgSizeHistogram tmp_histo;

            MessageData tmp;
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, (void*) &a,              1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_send.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.count_recv.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
       
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_send.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.min,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.max,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.sum,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.bytes_recv.cnt,     1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.min,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.max,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.sum,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_send.cnt,  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
       
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.min,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.max,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.sum,  1, MPI_DOUBLE, MPI_COMM_WORLD );
            MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp.duration_recv.cnt,  1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            for( int i = 0; i < 10; i++ ) {

                MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &tmp_histo.hist[i], 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );

            }

            tmp.msgSizes = tmp_histo;
            
            alldata.messageMapPerSystemTreeNode.insert( make_pair( a, tmp ) ); 
        
        }

        fence= 0;
        MPI_Unpack( buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        assert( FENCE == fence );

    }

        /* free the callpath chararray */
        delete callpath;

}


bool ReduceData( AllData& alldata ) {

    bool error= false;

    assert( 1 < alldata.numRanks );

    /* start runtime measurement for reducing data */
    StartMeasurement( alldata, 1, true, "reduce data" );

    VerbosePrint( alldata, 1, true, "reducing data\n" );

    /* implement reduction myself because MPI and C++ STL don't play with
    each other */

    /* how many rounds until master has all the data? */
    uint32_t num_rounds= Logi( alldata.numRanks ) -1;
    uint32_t round_no= 0;
    uint32_t round= 1;
    while ( round < alldata.numRanks ) {

        round_no++;

            VerbosePrint( alldata, 1, true, " round %u / %u\n",
                              round_no, num_rounds );

        uint32_t peer= alldata.myRank ^ round;

        /* if peer rank is not there, do nothing but go on */
        if ( peer >= alldata.numRanks ) {

            round= round << 1;
            continue;
        }

        /* send to smaller peer, receive from larger one */
        uint32_t sizes[PACK_NUM_PACKS];
        char* buffer;

        if ( alldata.myRank < peer ) {
                        
            MPI_Status status;

            MPI_Recv( sizes, PACK_NUM_PACKS, MPI_UNSIGNED, peer, 4, MPI_COMM_WORLD,
                      &status );

            // DEBUG
            //cout << "    round " << round << " recv " << peer << "--> " <<
            //alldata.myRank << " with " <<
            //sizes[PACK_TOTAL_SIZE] << " bytes, " <<
            //sizes[PACK_FUNCTION] << ", " <<
            //sizes[PACK_FUNCTION_DURATION] << ", " <<
            //sizes[PACK_COUNTER] << ", " <<
            //sizes[PACK_MSG_GROUP_PAIR] << "" << endl << flush;

            buffer= prepare_worker_data( alldata, sizes );

            VerbosePrint( alldata, 2, false,
                          "round %u / %u: receiving %u bytes from rank %u\n",
                          round_no, num_rounds, sizes[PACK_TOTAL_SIZE], peer );

            MPI_Recv( buffer, sizes[PACK_TOTAL_SIZE], MPI_PACKED, peer, 5, MPI_COMM_WORLD,
                      &status );

            unpack_worker_data( alldata, sizes );

        } else {
                        
            buffer= pack_worker_data( alldata, sizes );
            
            // DEBUG
            //cout << "    round " << round << " send " << alldata.myRank <<
            //" --> " << peer << " with " <<
            //sizes[PACK_TOTAL_SIZE] << " bytes, " <<
            //sizes[PACK_FUNCTION] << ", " <<
            //sizes[PACK_FUNCTION_DURATION] << ", " <<
            //sizes[PACK_COUNTER] << ", " <<
            //sizes[PACK_MSG_GROUP_PAIR] << "" << endl << flush;

            VerbosePrint( alldata, 2, false,
                          "round %u / %u: sending %u bytes to rank %u\n",
                          round_no, num_rounds, sizes[PACK_TOTAL_SIZE], peer );

            MPI_Send( sizes, PACK_NUM_PACKS, MPI_UNSIGNED, peer, 4, MPI_COMM_WORLD );

            MPI_Send( buffer, sizes[PACK_TOTAL_SIZE], MPI_PACKED, peer, 5,
                      MPI_COMM_WORLD );

            /* every work has to send off its data at most once,
            after that, break from the collective reduction operation */
            break;
        }

        round= round << 1;
    }

    alldata.freePackBuffer();

    /* clear MessagePerSize except for root to prevent sending it again */
    if ( alldata.params.write_csv_msg_sizes && alldata.myRank != 0 )
    {
       alldata.messageMapPerSize.clear();
    }

    /* synchronize error indicator with workers */
    /*SyncError( alldata, error );*/

    if ( !error ) {

        /* stop runtime measurement for reducing data */
        StopMeasurement( alldata, true, "reduce data" );
    }

    return !error;
}


bool ReduceDataDispersion( AllData& alldata ) {

    bool error= false;

    assert( 1 < alldata.numRanks );
    
    /* start runtime measurement for reducing data */
    StartMeasurement( alldata, 1, true, "reduce data dispersion" );
    
    VerbosePrint( alldata, 1, true, "reducing data dispersion\n" );


    /* implement reduction myself because MPI and C++ STL don't play with
     each other */
    
    /* how many rounds until master has all the data? */
    uint32_t num_rounds= Logi( alldata.numRanks ) -1;
    uint32_t round_no= 0;
    uint32_t round= 1;

    while ( round < alldata.numRanks ) {
        
        round_no++;
        
        if ( 1 == alldata.params.verbose_level ) {
            
            VerbosePrint( alldata, 1, true, " round %u / %u\n",
                         round_no, num_rounds );
        }
        
        uint32_t peer= alldata.myRank ^ round;
        
        /* if peer rank is not there, do nothing but go on */
        if ( peer >= alldata.numRanks ) {
            
            round= round << 1;
            continue;
        }
        
        /* send to smaller peer, receive from larger one */
        uint32_t sizes[PACK_NUM_PACKS];
        char* buffer;
        
        if ( alldata.myRank < peer ) {
            
            
            
            MPI_Status status;
            
            MPI_Recv( sizes, PACK_NUM_PACKS, MPI_UNSIGNED, peer, 4, MPI_COMM_WORLD,
                     &status );
            
            // DEBUG
      /*      cout << "    round " << round << " recv " << peer << "--> " <<
            alldata.myRank << " with " <<
            sizes[PACK_TOTAL_SIZE] << " bytes, " <<
            sizes[PACK_FUNCTION] << ", " <<
            sizes[PACK_FUNCTION_DURATION] << ", " <<
            sizes[PACK_COUNTER] << ", " <<
            sizes[PACK_MSG_GROUP_PAIR] << "" << endl << flush;
        */
            buffer= prepare_worker_data( alldata, sizes );
            
            VerbosePrint( alldata, 2, false,
                         "round %u / %u: receiving %u bytes from rank %u\n",
                         round_no, num_rounds, sizes[PACK_TOTAL_SIZE], peer );
            
            MPI_Recv( buffer, sizes[PACK_TOTAL_SIZE], MPI_PACKED, peer, 5, MPI_COMM_WORLD,
                     &status );
            
            unpack_worker_data( alldata, sizes );
            
        } else {
            
            
            
            /* don't reduce function map global twice */ 
            alldata.functionMapGlobal.clear();
            if(alldata.params.dispersion.mode == DISPERSION_MODE_PERCALLPATH)
                alldata.functionCallpathMapGlobal.clear();
            
            buffer= pack_worker_data( alldata, sizes );
            
            // DEBUG
            //cout << "    round " << round << " send " << alldata.myRank <<
            //" --> " << peer << " with " <<
            //sizes[PACK_TOTAL_SIZE] << " bytes, " <<
            //sizes[PACK_FUNCTION] << ", " <<
            //sizes[PACK_FUNCTION_DURATION] << ", " <<
            //sizes[PACK_COUNTER] << ", " <<
            //sizes[PACK_MSG_GROUP_PAIR] << "" << endl << flush;
            
            VerbosePrint( alldata, 2, false,
                         "round %u / %u: sending %u bytes to rank %u\n",
                         round_no, num_rounds, sizes[PACK_TOTAL_SIZE], peer );
            
            MPI_Send( sizes, PACK_NUM_PACKS, MPI_UNSIGNED, peer, 4, MPI_COMM_WORLD );
            
            MPI_Send( buffer, sizes[PACK_TOTAL_SIZE], MPI_PACKED, peer, 5,
                     MPI_COMM_WORLD );
            
            /* every work has to send off its data at most once,
             after that, break from the collective reduction operation */
            break;
        }
        
        round= round << 1;
    }
    alldata.freePackBuffer();
    
    /* synchronize error indicator with workers */
    /*SyncError( alldata, error );*/
    
    if ( !error ) {
        
        /* stop runtime measurement for reducing data */
        StopMeasurement( alldata, true, "reduce data dispersion" );
    }
    
    return !error;
}
