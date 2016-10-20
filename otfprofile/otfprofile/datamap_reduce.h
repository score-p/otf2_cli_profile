

#ifndef DATAMAP_REDUCE_H
#define DATAMAP_REDUCE_H



#include "datastructs.h"

    void mergeTree( AllData& alldata, CallPathTree rhs_tree );

    void mergeData( AllData& alldata, map< pair< uint64_t, uint64_t >, FunctionData > fData, CallPathTree rhs_tree );

    void mergeMData( AllData& alldata, map< pair< uint64_t, uint64_t >, MessageData > mData, CallPathTree rhs_tree );

    void mergeCData( AllData& alldata, map< pair< uint64_t, uint64_t >, MessageData > cData, CallPathTree rhs_tree );

    void mergeRmaData( AllData& alldata, map< pair< uint64_t, uint64_t >, RmaData > rma_Data, CallPathTree rhs_tree );

#endif /* DATAMAP_REDUCE_H */


