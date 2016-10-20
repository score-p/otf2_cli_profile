//
//  tracereader.cpp
//  
//
//  Created by Jens Doleschal on 18/06/14.
//
//

#include <iostream>

#include <stdio.h>
#include <string.h>

#include "tracereader.h"


void
TraceReader::prepare_progress( AllData& alldata, uint64_t max_bytes ) {
    
    Progress& progress= alldata.progress;
    
    progress.cur_bytes= 0;
    progress.max_bytes= max_bytes;
    
#ifdef OTFPROFILE_MPI
    progress.ranks_left= alldata.numRanks -1;
    
    if ( 1 < alldata.numRanks ) {
        
        /* reduce max. bytes to rank 0 */
        uint64_t sum_max_bytes;
        MPI_Reduce( &max_bytes, &sum_max_bytes, 1, MPI_LONG_LONG_INT, MPI_SUM,
                   0, MPI_COMM_WORLD );
        
        if ( 0 == alldata.myRank ) {
            
            progress.max_bytes= sum_max_bytes;
            
            progress.recv_buffers= new uint64_t[alldata.numRanks-1];
            assert( progress.recv_buffers );
            progress.recv_requests= new MPI_Request[alldata.numRanks-1];
            assert( progress.recv_requests );
            progress.recv_statuses= new MPI_Status[alldata.numRanks-1];
            assert( progress.recv_statuses );
            progress.recv_indices= new int[alldata.numRanks-1];
            assert( progress.recv_indices );
            progress.rank_cur_bytes= new uint64_t[alldata.numRanks-1];
            assert( progress.rank_cur_bytes );
            
            /* initialize array of current bytes read and start
             persistent communication */
            
            for ( uint32_t i= 0; i < alldata.numRanks -1; i++ ) {
                
                progress.rank_cur_bytes[i]= 0;
                
                /* create persistent request handle */
                MPI_Recv_init( &(progress.recv_buffers[i]), 1,
                              MPI_LONG_LONG_INT, i+1, Progress::MSG_TAG,
                              MPI_COMM_WORLD,
                              &(progress.recv_requests[i]) );
                
                /* start persistent communication */
                MPI_Start( &(progress.recv_requests[i]) );
                
            }
            
        } else { /* 0 != alldata.myRank */
            
            /* initialize request handle for sending progress to rank 0 */
            progress.send_request = MPI_REQUEST_NULL;
            
        }
        
        /* block until all worker ranks have reached this point to avoid that the
         progress does a big jump at beginning */
        MPI_Barrier( MPI_COMM_WORLD );
        
    }

#endif /* OTFPROFILE_MPI */
    
    if ( 0 == alldata.myRank ) {
        
        /* show initial progress */
        printf( "%7.2f %%\r", 0.0 );
        fflush( stdout );
    }
    
}

void
TraceReader::update_progress( AllData& alldata, uint64_t delta_bytes,
                            bool wait ) {
    
    Progress& progress= alldata.progress;
    
    progress.cur_bytes += delta_bytes;
    
    uint64_t sum_cur_bytes= progress.cur_bytes;
    
#ifdef OTFPROFILE_MPI
    if ( 1 < alldata.numRanks ) {
        
        if ( 0 == alldata.myRank ) {
            
            /* get current bytes read from all worker ranks */
            
            int out_count;
            uint32_t i;
            
            /* either wait or test for one or more updates from worker ranks */
            
            if ( wait )
            {
                
                MPI_Waitsome( alldata.numRanks - 1, progress.recv_requests,
                             &out_count, progress.recv_indices,
                             progress.recv_statuses );
                
            } else {
                
                MPI_Testsome( alldata.numRanks - 1, progress.recv_requests,
                             &out_count, progress.recv_indices,
                             progress.recv_statuses );
                
            }
            
            if ( MPI_UNDEFINED != out_count ) {
                
                for ( i= 0; i < (uint32_t) out_count; i++ ) {
                    
                    int index= progress.recv_indices[i];
                    
                    /* worker rank (index+1) is finished? */
                    if ( (uint64_t)-1 == progress.recv_buffers[index] ) {
                        
                        /* this rank is finished */
                        progress.ranks_left--;
                        
                    } else {
                        
                        /* update rank's current bytes read and restart
                         persistent communication */
                        
                        progress.rank_cur_bytes[index]= progress.recv_buffers[index];
                        
                        MPI_Start( &(progress.recv_requests[progress.recv_indices[i]]) );
                        
                    }
                }
                
            }
            
            /* recompute sum of current bytes read */
            for( i= 0; i < alldata.numRanks -1; i++ ) {
                
                sum_cur_bytes += progress.rank_cur_bytes[i];

            }
            
        } else { /* 0 != alldata.myRank */
            
            int do_send = 1;
            MPI_Status status;
            
            /* send only if it's the first send or the request handle isn't
             currently in use */
            
            if ( MPI_REQUEST_NULL != progress.send_request ) {
                
                MPI_Test( &(progress.send_request), &do_send, &status );
                
            }
            
            if ( do_send ) {
                
                MPI_Issend( &(progress.cur_bytes), 1, MPI_LONG_LONG_INT, 0,
                           Progress::MSG_TAG, MPI_COMM_WORLD,
                           &progress.send_request );

            }
            
        }
        
    }
#endif /* OTFPROFILE_MPI */
    
    if ( 0 == alldata.myRank ) {
        
        /* show progress */
        
        double percent =
        100.0 * (double) sum_cur_bytes / (double) progress.max_bytes;
        
        static const char signs[2]= { '.',' ' };
        static int signi= 0;
        
        printf( "%7.2f %% %c\r", percent, signs[signi] );
        fflush( stdout );
        
        signi^= 1;
        
    }
}

void
TraceReader::finish_progress( AllData& alldata ) {
    
#ifdef OTFPROFILE_MPI
    Progress& progress= alldata.progress;
    
    if ( 1 < alldata.numRanks ) {
        
        if ( 0 == alldata.myRank ) {
            
            /* update progress until all worker ranks are
             finished / all bytes are read */
            
            while ( 0 < progress.ranks_left ) {
                
                update_progress( alldata, 0, true );

            }
            
            /* ensure that all requests are inactive before freeing memory */
            MPI_Waitall( alldata.numRanks - 1, progress.recv_requests,
                        progress.recv_statuses );
            
            /* free memory */
            delete [] progress.recv_buffers;
            delete [] progress.recv_requests;
            delete [] progress.recv_statuses;
            delete [] progress.recv_indices;
            delete [] progress.rank_cur_bytes;
            
        } else { /* 0 != alldata.myRank */
            
            MPI_Status status;
            MPI_Wait( &(progress.send_request), &status );
            
            /* send last current bytes read to rank 0 */
            MPI_Send( &(progress.cur_bytes), 1, MPI_LONG_LONG_INT, 0,
                     Progress::MSG_TAG, MPI_COMM_WORLD );
            
            /* send marker (-1) to rank 0 which indicates that this worker rank
             is finished */
            
            progress.cur_bytes = (uint64_t) -1;
            MPI_Send( &(progress.cur_bytes), 1, MPI_LONG_LONG_INT, 0,
                     Progress::MSG_TAG, MPI_COMM_WORLD );
            
        }
        
    }
#endif /* OTFPROFILE_MPI */
    
    if ( 0 == alldata.myRank ) {
        
        /* show final progress */
        printf( "%7.2f %% done\n", 100.0 );
        
    }
}

#ifdef OTFPROFILE_MPI
void
TraceReader::share_definitions( AllData& alldata ) {
    
    assert( 1 < alldata.numRanks );
    
    char* buffer;
    int buffer_size= 0;
    int buffer_pos= 0;
    
    /* get size needed to send definitions to workers */
    
    if ( 0 == alldata.myRank ) {
        
        int size;
        
        MPI_Pack_size( 1 + alldata.functionIdNameMap.size() * 2 +
                      1 + alldata.counterIdNameMap.size() * 2 +
                      1 + alldata.collectiveOperationsToClasses.size() * 2 +
                      1 + alldata.countersOfInterest.size() +
                      1 /* timerResolution */ +
                      1 /* recvTimeKey */,
                      MPI_LONG_LONG_INT, MPI_COMM_WORLD, &buffer_size );
        
        /* functionIdNameMap seconds */
        for ( map< uint64_t, string >::const_iterator it =
             alldata.functionIdNameMap.begin();
             it != alldata.functionIdNameMap.end(); it++ ) {
            
            MPI_Pack_size( it->second.length() +1, MPI_CHAR,
                          MPI_COMM_WORLD, &size );
            buffer_size += size;
            
        }
        
        /* counterIdNameMap seconds */
        for ( map< uint64_t, string >::const_iterator it =
             alldata.counterIdNameMap.begin();
             it != alldata.counterIdNameMap.end(); it++ ) {
            
            MPI_Pack_size( it->second.length() +1, MPI_CHAR,
                          MPI_COMM_WORLD, &size );
            buffer_size += size;
            
        }
        
        /* get size of additional definitions needed for CSV creation */
        if ( alldata.params.create_csv ) {
            
            /* processIdNameMap.size() + firsts */
            MPI_Pack_size( 1 + alldata.processIdNameMap.size() * 2,
                          MPI_LONG_LONG_INT, MPI_COMM_WORLD, &size );
            buffer_size += size;
            
            /* processIdNameMap seconds */
            for ( map< uint64_t, string >::const_iterator it =
                 alldata.processIdNameMap.begin();
                 it != alldata.processIdNameMap.end(); it++ ) {
                
                MPI_Pack_size( it->second.length() +1, MPI_CHAR,
                              MPI_COMM_WORLD, &size );
                buffer_size += size;
                
            }
            
            /* allProcesses */
            MPI_Pack_size( 1 + alldata.allProcesses.size() * 2,
                          MPI_LONG_LONG_INT, MPI_COMM_WORLD, &size );
            buffer_size += size;
            
        }
        
    }
    
    /* broadcast buffer size */
    MPI_Bcast( &buffer_size, 1, MPI_INT, 0, MPI_COMM_WORLD );
    
    /* allocate buffer */
    buffer= new char[ buffer_size ];
    assert( buffer );
    
    /* pack definitions to buffer */
    
    if ( 0 == alldata.myRank ) {
        
        /* functionIdNameMap.size() */
        uint64_t funcid_name_map_size= alldata.functionIdNameMap.size();
        MPI_Pack( &funcid_name_map_size, 1, MPI_LONG_LONG_INT, buffer,
                 buffer_size, &buffer_pos, MPI_COMM_WORLD );
        
        /* functionIdNameMap */
        for ( map< uint64_t, string >::const_iterator it =
             alldata.functionIdNameMap.begin();
             it != alldata.functionIdNameMap.end(); it++ ) {
            
            /* functionIdNameMap.first */
            uint64_t first= it->first;
            MPI_Pack( &first, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            
            /* functionIdNameMap.second.length() */
            uint64_t second_length= it->second.length() +1;
            MPI_Pack( &second_length, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            
            /* functionIdNameMap.second */
            char* second= strdup( it->second.c_str() );
            assert( second );
            MPI_Pack( second, second_length, MPI_CHAR, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            free( second );
            
        }
        
        /* counterIdNameMap.size() */
        uint64_t cntrid_name_map_size= alldata.counterIdNameMap.size();
        MPI_Pack( &cntrid_name_map_size, 1, MPI_LONG_LONG_INT, buffer,
                 buffer_size, &buffer_pos, MPI_COMM_WORLD );
        
        /* counterIdNameMap */
        for ( map< uint64_t, string >::const_iterator it =
             alldata.counterIdNameMap.begin();
             it != alldata.counterIdNameMap.end(); it++ ) {
            
            /* counterIdNameMap.first */
            uint64_t first= it->first;
            MPI_Pack( &first, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            
            /* counterIdNameMap.second.length() */
            uint64_t second_length= it->second.length() +1;
            MPI_Pack( &second_length, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            
            /* counterIdNameMap.second */
            char* second= strdup( it->second.c_str() );
            assert( second );
            MPI_Pack( second, second_length, MPI_CHAR, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            free( second );
            
        }
        
        /* collectiveOperationsToClasses.size() */
        uint64_t collop_classes_map_size=
        alldata.collectiveOperationsToClasses.size();
        MPI_Pack( &collop_classes_map_size, 1, MPI_LONG_LONG_INT, buffer,
                 buffer_size, &buffer_pos, MPI_COMM_WORLD );
        
        /* collectiveOperationsToClasses */
        for ( map< uint64_t, uint64_t >::const_iterator it =
             alldata.collectiveOperationsToClasses.begin();
             it != alldata.collectiveOperationsToClasses.end(); it++ ) {
            
            /* collectiveOperationsToClasses.first */
            uint64_t first= it->first;
            MPI_Pack( &first, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            /* collectiveOperationsToClasses.second */
            uint64_t second= it->second;
            MPI_Pack( &second, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            
        }
        
        /* countersOfInterest.size() */
        uint64_t counters_size= alldata.countersOfInterest.size();
        MPI_Pack( &counters_size, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                 &buffer_pos, MPI_COMM_WORLD );
        
        /* countersOfInterest */
        for ( set< uint64_t >::const_iterator it=
             alldata.countersOfInterest.begin();
             it != alldata.countersOfInterest.end(); it++ ) {
            
            uint64_t counter= *it;
            MPI_Pack( &counter, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                     &buffer_pos, MPI_COMM_WORLD );
            
        }
        
        /* recvTimeKey */
        MPI_Pack( &(alldata.recvTimeKey), 1, MPI_LONG_LONG_INT, buffer,
                 buffer_size, &buffer_pos, MPI_COMM_WORLD );
        
        /* timerResolution */
        MPI_Pack( &(alldata.timerResolution), 1, MPI_LONG_LONG_INT, buffer,
                 buffer_size, &buffer_pos, MPI_COMM_WORLD );
        
        /* pack additional definitions needed for CSV creation */
        if ( alldata.params.create_csv ) {
            
            /* processIdNameMap.size() */
            uint64_t procid_name_map_size= alldata.processIdNameMap.size();
            MPI_Pack( &procid_name_map_size, 1, MPI_LONG_LONG_INT, buffer,
                     buffer_size, &buffer_pos, MPI_COMM_WORLD );
            
            /* processIdNameMap */
            for ( map< uint64_t, string >::const_iterator it =
                 alldata.processIdNameMap.begin();
                 it != alldata.processIdNameMap.end(); it++ ) {
                
                /* processIdNameMap.first */
                uint64_t first= it->first;
                MPI_Pack( &first, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                         &buffer_pos, MPI_COMM_WORLD );
                
                /* processIdNameMap.second.length() */
                uint64_t second_length= it->second.length() +1;
                MPI_Pack( &second_length, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                         &buffer_pos, MPI_COMM_WORLD );
                
                /* processIdNameMap.second */
                char* second= strdup( it->second.c_str() );
                assert( second );
                MPI_Pack( second, second_length, MPI_CHAR, buffer, buffer_size,
                         &buffer_pos, MPI_COMM_WORLD );
                free( second );
                
            }
            
            /* allProcesses.size() */
            uint64_t all_processes_map = alldata.allProcesses.size();
            MPI_Pack( &all_processes_map, 1, MPI_LONG_LONG_INT, buffer,
                     buffer_size, &buffer_pos, MPI_COMM_WORLD );
            
            /* allProcesses */
            for ( set< Process, ltProcess >::iterator it = alldata.allProcesses.begin();
                 it != alldata.allProcesses.end();
                 it++ )
            {
                uint64_t process, parent;
                process = it->process;
                parent  = it->parent;
                
                /* process */
                MPI_Pack( &process, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                         &buffer_pos, MPI_COMM_WORLD );
                /* parent */
                MPI_Pack( &parent, 1, MPI_LONG_LONG_INT, buffer, buffer_size,
                         &buffer_pos, MPI_COMM_WORLD );
            }
            
        }
        
    }
    
    /* broadcast definitions buffer */
    MPI_Bcast( buffer, buffer_size, MPI_PACKED, 0, MPI_COMM_WORLD );
    
    /* unpack definitions from buffer */
    
    if ( 0 != alldata.myRank ) {
        
        /* functionIdNameMap.size() */
        uint64_t funcid_name_map_size;
        MPI_Unpack( buffer, buffer_size, &buffer_pos,
                   &funcid_name_map_size, 1, MPI_LONG_LONG_INT,
                   MPI_COMM_WORLD );
        
        /* functionIdNameMap */
        for ( uint64_t i= 0; i < funcid_name_map_size; i++ ) {
            
            /* functionIdNameMap.first */
            uint64_t first;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &first, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            /* functionIdNameMap.second.length() */
            uint64_t second_length;
            
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &second_length, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            /* functionIdNameMap.second */
            char* second= new char[ second_length ];
            assert( second );
            MPI_Unpack( buffer, buffer_size, &buffer_pos, second,
                       second_length, MPI_CHAR, MPI_COMM_WORLD );
            
            alldata.functionIdNameMap[ first ]= second;
            
            delete [] second;
        }
        
        /* counterIdNameMap.size() */
        uint64_t cntrid_name_map_size;
        MPI_Unpack( buffer, buffer_size, &buffer_pos,
                   &cntrid_name_map_size, 1, MPI_LONG_LONG_INT,
                   MPI_COMM_WORLD );
        
        /* counterIdNameMap */
        for ( uint64_t i= 0; i < cntrid_name_map_size; i++ ) {
            
            /* counterIdNameMap.first */
            uint64_t first;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &first, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            /* counterIdNameMap.second.length() */
            uint64_t second_length;
            
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &second_length, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            /* counterIdNameMap.second */
            char* second= new char[ second_length ];
            assert( second );
            MPI_Unpack( buffer, buffer_size, &buffer_pos, second,
                       second_length, MPI_CHAR, MPI_COMM_WORLD );
            
            alldata.counterIdNameMap[ first ]= second;
            
            delete [] second;

        }
        
        /* collectiveOperationsToClasses.size() */
        uint64_t collop_classes_map_size;
        MPI_Unpack( buffer, buffer_size, &buffer_pos,
                   &collop_classes_map_size, 1, MPI_LONG_LONG_INT,
                   MPI_COMM_WORLD );
        
        /* collectiveOperationsToClasses */
        for ( uint64_t i= 0; i < collop_classes_map_size; i++ ) {
            
            /* collectiveOperationsToClasses.first */
            uint64_t first;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &first, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            /* collectiveOperationsToClasses.second */
            uint64_t second;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &second, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            alldata.collectiveOperationsToClasses[ first ]= second;
            
        }
        
        /* countersOfInterest.size() */
        uint64_t counters_size;
        MPI_Unpack( buffer, buffer_size, &buffer_pos,
                   &counters_size, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        
        /* countersOfInterest */
        for ( uint64_t i= 0; i < counters_size; i++ ) {
            
            uint64_t counter;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, &counter, 1,
                       MPI_LONG_LONG_INT, MPI_COMM_WORLD );
            
            alldata.countersOfInterest.insert( counter );
            
        }
        
        /* recvTimeKey */
        MPI_Unpack( buffer, buffer_size, &buffer_pos, &(alldata.recvTimeKey),
                   1, MPI_LONG_LONG_INT, MPI_COMM_WORLD );
        
        /* timerResolution */
        MPI_Unpack( buffer, buffer_size, &buffer_pos,
                   &(alldata.timerResolution), 1, MPI_LONG_LONG_INT,
                   MPI_COMM_WORLD );
        
        /* unpack additional definitions needed for CSV creation */
        if ( alldata.params.create_csv ) {
            
            /* processIdNameMap.size() */
            uint64_t procid_name_map_size;
            MPI_Unpack( buffer, buffer_size, &buffer_pos,
                       &procid_name_map_size, 1, MPI_LONG_LONG_INT,
                       MPI_COMM_WORLD );
            
            /* processIdNameMap */
            for ( uint64_t i= 0; i < procid_name_map_size; i++ ) {
                
                /* processIdNameMap.first */
                uint64_t first;
                MPI_Unpack( buffer, buffer_size, &buffer_pos, &first, 1,
                           MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                
                /* processIdNameMap.second.length() */
                uint64_t second_length;
                
                MPI_Unpack( buffer, buffer_size, &buffer_pos, &second_length, 1,
                           MPI_LONG_LONG_INT, MPI_COMM_WORLD );
                
                /* processIdNameMap.second */
                char* second= new char[ second_length ];
                assert( second );
                MPI_Unpack( buffer, buffer_size, &buffer_pos, second,
                           second_length, MPI_CHAR, MPI_COMM_WORLD );
                
                alldata.processIdNameMap[ first ]= second;
                
                delete [] second;

            }
            
            /* allProcesses.size() */
            uint64_t all_processes_size;
            MPI_Unpack( buffer, buffer_size, &buffer_pos, 
                       &all_processes_size, 1, MPI_LONG_LONG_INT,
                       MPI_COMM_WORLD );
            
            for ( uint64_t i = 0; i < all_processes_size; i++ )
            {
                uint64_t process, parent;
                
                /* process */
                MPI_Unpack( buffer, buffer_size, &buffer_pos, 
                           &process, 1, MPI_LONG_LONG_INT,
                           MPI_COMM_WORLD );
                /* parent */
                MPI_Unpack( buffer, buffer_size, &buffer_pos, 
                           &parent, 1, MPI_LONG_LONG_INT,
                           MPI_COMM_WORLD );
                
                alldata.allProcesses.insert( Process(process, parent) );

            }
        }
    }
    
    delete[] buffer;

}
#endif /* OTFPROFILE_MPI */
