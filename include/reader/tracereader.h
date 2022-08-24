/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef TRACEREADER_H
#define TRACEREADER_H

#include "all_data.h"

#include <stdint.h>
#include <memory>

class TraceReader {
   public:
    virtual ~TraceReader() {}

    virtual bool initialize(AllData& alldata)      = 0;
    virtual void close()                           = 0;
    virtual bool readDefinitions(AllData& alldata) = 0;
    virtual bool readEvents(AllData& alldata)      = 0;
    virtual bool readStatistics(AllData& alldata)  = 0;
};

std::unique_ptr<TraceReader> getTraceReader(AllData& alldata);

// data stack for function data -> enter/leave callbacks etc.
struct StackData {
    tree_node* node_p;

    uint64_t time;
    uint64_t child_incl;
};

#endif /* TRACEREADER_H */
