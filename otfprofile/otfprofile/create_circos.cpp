#include <iostream>
#include <string.h>
#include <errno.h>

#include "otfprofile.h"

#include "create_circos.h"

#include "CircosLayouts/layouts/P2PCommLayout.hpp"

#include "circos_fix.h"

using namespace std;
        
void CreateCircos(AllData& alldata)
{

    StartMeasurement(alldata, 1, false, "produce circos output");

    VerbosePrint(alldata, 1, true, "producing circos output\n");

    ILayout *layout = NULL;
    
    layout = new P2PCommLayout(alldata);
    layout->createLayout();

    delete layout;

    /* create output pictures */
    
    int rc, es;

    rc = system( "circos -conf P2PComm_circos.conf" );

    clean_up();

        /* evaluate exit status */
    es = ( -1 != rc ) ? WEXITSTATUS( rc ) : 0;

    if ( -1 == rc || 127 == es ) {
        /* command could not be executed; print warning message */

        ostringstream warn_msg;
        warn_msg << "Warning: Could not execute command '"
        << "circos -conf P2PComm_circos.conf" << "'";

        if ( -1 == rc ) {
            warn_msg << " (" << strerror( errno ) << ")";
        }

        warn_msg << "." << endl;
        cerr << warn_msg.str() << endl;

        StopMeasurement(alldata, false, "produce circos output");

        return;

    } else if ( 0 != es ) {
        /* command executed, but failed; abort */

        cerr << "ERROR: Could not create circos output, "
        "circos returned with exit code " << es << "." << endl;

        StopMeasurement(alldata, false, "produce circos output");

        return;
    }

    StopMeasurement(alldata, false, "produce circos output");

    return;

}

