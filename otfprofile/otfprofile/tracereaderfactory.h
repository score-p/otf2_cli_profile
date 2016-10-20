//
//  tracereaderfactory.h
//  
//
//  Created by Jens Doleschal on 18/06/14.
//
//

#ifndef TRACEREADERFACTORY_H
#define TRACEREADERFACTORY_H

#include "datastructs.h"
#include "tracereader.h"

class TraceReaderFactory {
    
public:

    TraceReaderFactory() {}
    ~TraceReaderFactory() {}
    
    static TraceReader* getInstance( AllData& alldata );
    
private:

};

#endif /* TRACEREADERFACTORY_H */
