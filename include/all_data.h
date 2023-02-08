/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef ALLDATA_H
#define ALLDATA_H

#include "data_tree.h"
#include "definitions.h"
#include "utils.h"

/* *** management and statistics data structures, needed on all ranks *** */

struct AllData {
    /* data_tree */
    data_tree call_path_tree;
    /*meta meta*/
    meta_data metaData;

    std::map<uint64_t, std::map<uint64_t, MData>> p2p_comm_send;
    std::map<uint64_t, std::map<uint64_t, MData>> p2p_comm_recv;

    definitions::Definitions definitions;
    uint64_t                 traceID;

    /* program parameters */
    Params params;

    /* runtime measurement */
    TimeMeasurement tm;

    /* I/O summary */
    std::map<uint64_t, IoData> io_data;
    AllData(uint32_t my_rank = 0, uint32_t num_ranks = 1) {
        metaData.myRank   = my_rank;
        metaData.numRanks = num_ranks;
    }

    void verbosePrint(uint8_t vlevel, bool master_only, std::string msg) {
        if (params.verbose_level < vlevel)
            return;

        if (master_only) {
            if (metaData.myRank == 0)
                std::cout << msg << std::endl;
            return;
        }

        std::cout << "[" << metaData.myRank << "] " << msg << std::endl;
    }
};

#endif /* DATASTRUCTS_H */
