/*
 This is part of the OTF library. Copyright by ZIH, TU Dresden 2005-2013.
 Authors: Andreas Knuepfer, Robert Dietrich, Matthias Jurenz
*/

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "collect_data.h"
#include "otfprofile.h"
#include "tracereaderfactory.h"


using namespace std;

bool CollectData( AllData& alldata ) {

    bool error= false;

    /* start runtime measurement for collecting data */
    StartMeasurement( alldata, 1, true, "collect data" );

    /* get instance to trace file via factory class */
    TraceReader* reader = TraceReaderFactory::getInstance( alldata );

    if ( reader == NULL && !( alldata.params.disp && alldata.params.is_otf2 ) ) {

        error = true;
        cerr << "ERROR: Got no instance to handle trace format." << endl;
        return !error;

    } else if( alldata.params.disp && alldata.params.is_otf2 ) {

        error = true;
        return !error;

    }
    
    do {
        
        /* assign work to worker */
        
        VerbosePrint( alldata, 1, true, "assigning processes\n" );
        
        error= !reader->assignProcs( alldata );

        /* read definitions */

        VerbosePrint( alldata, 1, true, "reading definitions\n" );

        error= !reader->readDefinitions( alldata );

        if ( error ) {
            
            if ( 0 == alldata.myRank ) {
                
                cerr << "ERROR: Could not read definitions. " << endl;
            }
            
            break;
        }

        /* either read data from events or statistics */

        if ( !alldata.params.read_from_stats ) {

            VerbosePrint( alldata, 1, true, "reading events\n" );

            error= !reader->readEvents ( alldata );
            if ( error ) {

                if ( 0 == alldata.myRank ) {

                    cerr << "ERROR: Could not read events." << endl
                         << "If the input trace has only summarized "
                         << "information (statistics) and no events, "
                         << "please try again with option '--stat'." << endl;

                }

                break;

            }

        } else {

            VerbosePrint( alldata, 1, true, "reading statistics\n" );

            error= !reader->readStatistics( alldata );
            if ( error ) {

                if ( 0 == alldata.myRank ) {

                    cerr << "ERROR: Could not read statistics." << endl;

                }

                break;

            }

        }



    } while( false );

    delete reader;
    
    if ( !error ) {

        /* stop runtime measurement for collecting data */
        StopMeasurement( alldata, true, "collect data" );

    }
    return !error;
}
