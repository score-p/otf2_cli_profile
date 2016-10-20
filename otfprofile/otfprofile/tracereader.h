//
//  tracereader.h
//  
//
//  Created by Jens Doleschal on 18/06/14.
//
//

#ifndef TRACEREADER_H
#define TRACEREADER_H

#include "datastructs.h"
#include "otfprofile.h"

class TraceReader {
    
protected:

    TraceReader() {}
    
public:

    virtual ~TraceReader() {}
    
    virtual bool open( AllData& alldata ) = 0;
    virtual void close() = 0;
    virtual bool assignProcs( AllData& alldata )     = 0;
    virtual bool readDefinitions( AllData& alldata ) = 0;
    virtual bool readEvents( AllData& alldata )      = 0;
    virtual bool readStatistics( AllData& alldata )  = 0;
    
protected:

    void prepare_progress( AllData& alldata, uint64_t max_bytes );
    void update_progress( AllData& alldata, uint64_t delta_bytes, bool wait= false );
    void finish_progress( AllData& alldata );
    void share_definitions( AllData& alldata );

};

#endif /* TRACEREADER_H */
