/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#include <iostream>
#include <string>

#include "otf-profiler-config.h"
#include "tracereader.h"

#if defined HAVE_OPEN_TRACE_FORMAT
#include "OTFReader.h"
#endif

#if defined HAVE_OTF2
#include "OTF2Reader.h"
#endif

#ifdef HAVE_JSON
#include "jsonreader.h"
#endif

using namespace std;

unique_ptr<TraceReader> getTraceReader(AllData& alldata) {
    string filename                  = alldata.params.input_file_name;
    auto   n                         = filename.rfind(".");
    alldata.params.input_file_prefix = filename.substr(0, n);

    string filetype = filename.substr(n + 1);

    if (filetype == "otf") {
#ifndef HAVE_OPEN_TRACE_FORMAT
        cerr << "ERROR: Can't process OTF files!" << endl
             << "otfprofile was not installed with access to the OTF header files/library." << endl;
        return nullptr;
#else
        return unique_ptr<OTFReader>(new OTFReader);
#endif
    } else if (filetype == "otf2") {
#ifndef HAVE_OTF2
        cerr << "ERROR: Can't process OTF2 files!" << endl
             << "otfprofile was not installed with access to the OTF2 header files/library." << endl;
        return nullptr;
#else
        return unique_ptr<OTF2Reader>(new OTF2Reader);
#endif
    } else if (filetype == "json"){
#ifndef HAVE_JSON
        cerr << "ERROR: Can't process json files!" << endl
             << "rapidjson was not found." << endl;
        return nullptr;
#else
    #define HAVE_DATA_IN
        return unique_ptr<JsonReader>(new JsonReader);
#endif
    } else
        cerr << "ERROR: Unknown file type!" << endl;

    return nullptr;
}
