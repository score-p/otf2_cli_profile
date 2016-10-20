//
//  tracereaderfactory.cpp
//  
//
//  Created by Jens Doleschal on 18/06/14.
//
//

#include <string>
#include "tracereaderfactory.h"

#ifdef HAVE_OPEN_TRACE_FORMAT
#include "OTFReader.h"
#endif

#ifdef HAVE_OTF2
#include "OTF2Reader.h"
#endif

TraceReader* TraceReaderFactory::getInstance ( AllData& alldata)
{
    string filename = alldata.params.input_file_name;
    string filetype;
    TraceReader* p_reader;
    
    std::string::size_type n;
    
    n = filename.rfind(".");
    filetype = filename.substr(n+1);
    alldata.params.input_file_prefix=filename.substr(0,n);
    
    if ( 0 == filetype.compare("otf") ) {

        alldata.params.is_otf2 = false;

#ifdef HAVE_OPEN_TRACE_FORMAT

        p_reader = new OTFReader();
        p_reader->open( alldata );

#else

        cerr << "ERROR: Can't process OTF files!" << endl
             << "otfprofile was not installed with access to the OTF header files/library." << endl;

#endif

    } else if ( 0 == filetype.compare("otf2") ) {

#ifdef HAVE_OTF2

        alldata.params.is_otf2 = true;

        if( alldata.params.disp ) {

            cerr << "ERROR: Dispersion in connection with OTF2 is not supported!" << endl;

            return p_reader = NULL;

        }



        p_reader = new OTF2Reader();
        p_reader->open( alldata );

#else

        p_reader = NULL;

        cerr << "ERROR: Can't process OTF2 files!" << endl
             << "otfprofile was not installed with access to the OTF2 header files/library." << endl;

#endif

    } else {

        p_reader = NULL;
        cerr << "ERROR: Unknown file type!" << endl;

    }
    
    return p_reader;

}
