/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#include <mpi.h>
#include <cmath>
#include <sstream>

#include "reduce_data.h"

using namespace std;

/* mapping and data deques for serialisation */
static map<uint64_t, pair<uint64_t, uint64_t>>                 mapping;
static deque<tuple<uint64_t, uint64_t, FunctionData*>>         f_data;
static deque<tuple<uint64_t, uint64_t, MessageData*>>          m_data;
static deque<tuple<uint64_t, uint64_t, CollopData*>>           c_data;
static deque<tuple<uint64_t, uint64_t, uint64_t, MetricData*>> met_data;

/* fence between statistics parts within the buffer for consistency checking */
enum { FENCE = 0xDEADBEEF };

enum {

    PACK_TOTAL_SIZE    = 0,
    PACK_MAPPING       = 1,
    PACK_FUNCTION_DATA = 2,
    PACK_MESSAGE_DATA  = 3,
    PACK_COLLOP_DATA   = 4,
    PACK_METRIC_DATA   = 5,
    PACK_NUM_PACKS     = 6

};

/* pack the local alldata into a buffer, return buffer */
static char* pack_worker_data(AllData& alldata, uint32_t sizes[PACK_NUM_PACKS]) {
    uint64_t fence      = FENCE;
    uint32_t num_fences = 1;

    /* get the sizes of all parts that need to be transmitted */

    for (uint32_t i = 1; i < PACK_NUM_PACKS; i++) {
        sizes[i] = 0;
    }

    sizes[PACK_MAPPING] = mapping.size();
    num_fences++;

    sizes[PACK_FUNCTION_DATA] = f_data.size();
    num_fences++;

    sizes[PACK_MESSAGE_DATA] = m_data.size();
    num_fences++;

    sizes[PACK_COLLOP_DATA] = c_data.size();
    num_fences++;

    sizes[PACK_METRIC_DATA] = met_data.size();
    num_fences++;

    /* get bytesize multiplying all pieces */
    uint32_t bytesize = 0;
    int      s1, s2;

    MPI_Pack_size(num_fences, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1);
    bytesize += s1;

    MPI_Pack_size(sizes[PACK_MAPPING] * 3, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1);
    bytesize += s1;

    MPI_Pack_size(sizes[PACK_FUNCTION_DATA] * 3, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1);
    MPI_Pack_size(sizes[PACK_FUNCTION_DATA] * 2, MPI_DOUBLE, MPI_COMM_WORLD, &s2);
    bytesize += s1 + s2;

    MPI_Pack_size(sizes[PACK_MESSAGE_DATA] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1);
    bytesize += s1;

    MPI_Pack_size(sizes[PACK_COLLOP_DATA] * 6, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1);
    bytesize += s1;

    MPI_Pack_size(sizes[PACK_METRIC_DATA] * 7, MPI_LONG_LONG_INT, MPI_COMM_WORLD, &s1);
    bytesize += s1;

    /* get the buffer */
    sizes[PACK_TOTAL_SIZE] = bytesize;
    char* buffer           = alldata.metaData.guaranteePackBuffer(bytesize);

    /* pack parts */
    int position = 0;

    /* extra check that doesn't cost too much */
    MPI_Pack((void*)&fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

    /* pack mapping (three) */
    {
        for (auto it = mapping.begin(); it != mapping.end(); it++) {
            MPI_Pack((void*)&it->first, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&it->second.first, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&it->second.second, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
        }
    }

    /* extra check that doesn't cost too much */
    MPI_Pack((void*)&fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

    /* pack function data */
    {
        for (auto it = f_data.begin(); it != f_data.end(); it++) {
            FunctionData tmp = *get<2>(*it);
            MPI_Pack((void*)&get<0>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

            MPI_Pack((void*)&get<1>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.count, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.incl_time, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.excl_time, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
        }
    }

    /* extra check that doesn't cost too much */
    MPI_Pack((void*)&fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

    /* pack message data */
    {
        for (auto it = m_data.begin(); it != m_data.end(); it++) {
            MessageData tmp = *get<2>(*it);

            MPI_Pack((void*)&get<0>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

            MPI_Pack((void*)&get<1>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.count_send, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.count_recv, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.bytes_send, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.bytes_recv, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
        }
    }

    /* extra check that doesn't cost too much */
    MPI_Pack((void*)&fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

    /* pack collop data */
    {
        for (auto it = c_data.begin(); it != c_data.end(); it++) {
            CollopData tmp = *get<2>(*it);

            MPI_Pack((void*)&get<0>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&get<1>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

            MPI_Pack((void*)&tmp.count_send, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.count_recv, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.bytes_send, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&tmp.bytes_recv, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
        }
    }

    /* extra check that doesn't cost too much */
    MPI_Pack((void*)&fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

    /* pack metrics (counter) data */
    {
        for (auto it = met_data.begin(); it != met_data.end(); it++) {
            MetricData tmp = *get<3>(*it);

            MPI_Pack((void*)&get<0>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&get<1>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
            MPI_Pack((void*)&get<2>(*it), 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

            MPI_Pack((void*)&tmp.type, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

            switch (tmp.type) {
                case MetricDataType::UINT64:
                    MPI_Pack((void*)&tmp.data_incl, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
                    MPI_Pack((void*)&tmp.data_excl, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
                    break;
                case MetricDataType::INT64:
                    MPI_Pack((void*)&tmp.data_incl, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
                    MPI_Pack((void*)&tmp.data_excl, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);
                    break;
                case MetricDataType::DOUBLE:
                    MPI_Pack((void*)&tmp.data_incl, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD);
                    MPI_Pack((void*)&tmp.data_excl, 1, MPI_DOUBLE, buffer, bytesize, &position, MPI_COMM_WORLD);
                    break;
            }
        }
    }

    /* extra check that doesn't cost too much */
    MPI_Pack((void*)&fence, 1, MPI_LONG_LONG_INT, buffer, bytesize, &position, MPI_COMM_WORLD);

    return buffer;
}

/* prepare alldata for unpack, return buffer of sufficient size */
static char* prepare_worker_data(AllData& alldata, uint32_t sizes[PACK_NUM_PACKS]) {
    uint32_t bytesize = sizes[PACK_TOTAL_SIZE];

    return alldata.metaData.guaranteePackBuffer(bytesize);
}

/* unpack the received worker data and add it to the local alldata */
static void unpack_worker_data(AllData& alldata, uint32_t sizes[PACK_NUM_PACKS]) {
    data_tree tmp_tree;
    /* <node_id, <parent_id, function_id, pointer on object> */
    map<uint64_t, tuple<uint64_t, uint64_t, shared_ptr<tree_node>>> tmp_map;

    uint64_t fence;

    /* unpack parts */
    int   position = 0;
    char* buffer   = alldata.metaData.getPackBuffer();

    /* extra check that doesn't cost too much */
    fence = 0;
    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
    assert(FENCE == fence);

    /* unpack mapping -> generate temporary tree */
    {
        for (uint64_t i = 0; i < sizes[PACK_MAPPING]; i++) {
            uint64_t id, function_id, parent_id;

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &function_id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &parent_id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            tmp_map.insert(make_pair(id, make_tuple(function_id, parent_id, shared_ptr<tree_node>(nullptr))));
        }

        // generate tree out of mapping AND add pointer to the respective node to the mapping
        tmp_tree = data_tree(tmp_map);

        /* extra check that doesn't cost too much */
        fence = 0;
        MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
        assert(FENCE == fence);
    }

    /* unpack function data -> add it to the temporary tree */
    {
        for (uint64_t i = 0; i < sizes[PACK_FUNCTION_DATA]; i++) {
            uint64_t id, rank, count, incl, excl;

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &rank, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &count, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &incl, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &excl, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            /*  IMPORTANT: tmp_map.find( id ) --- find node in mapping
             *  ->second --- get pointer to node in tree
             *  ->m_data.insert() --- insert data in node-data-container
             *  -----> insert is without checks for existence because an analysis rank is working on
             * trace ranks exclusivly
             *  that means if analysis rank 0 has trace ranks 0-3 he does not have any clue of trace
             * rank 4-n so an insert of
             *  rank 4-n is safe without testing for the existence of data
             */

            get<2>(tmp_map.find(id)->second)->node_data.insert(make_pair(rank, FunctionData{count, incl, excl}));
        }

        /* extra check that doesn't cost too much */
        fence = 0;
        MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
        assert(FENCE == fence);
    }

    /* unpack message data */
    {
        for (uint64_t i = 0; i < sizes[PACK_MESSAGE_DATA]; i++) {
            uint64_t id, rank, count_send, count_recv, bytes_send, bytes_recv;

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &rank, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &count_send, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &count_recv, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &bytes_send, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &bytes_recv, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            // analogous to unpack function data
            get<2>(tmp_map.find(id)->second)
                ->add_data(rank, MessageData{count_send, count_recv, bytes_send, bytes_recv});
        }

        /* extra check that doesn't cost too much */
        fence = 0;
        MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
        assert(FENCE == fence);
    }

    /* unpack collop data */
    {
        for (uint64_t i = 0; i < sizes[PACK_COLLOP_DATA]; i++) {
            uint64_t id, rank, count_send, count_recv, bytes_send, bytes_recv;

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &rank, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &count_send, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &count_recv, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &bytes_send, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &bytes_recv, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            // analogous to unpack function data
            get<2>(tmp_map.find(id)->second)
                ->add_data(rank, CollopData{count_send, count_recv, bytes_send, bytes_recv});
        }

        /* extra check that doesn't cost too much */
        fence = 0;
        MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
        assert(FENCE == fence);
    }

    /* unpack metrics (counter) data */

    {
        for (uint64_t i = 0; i < sizes[PACK_METRIC_DATA]; i++) {
            uint64_t       id, rank, metric_id;
            MetricDataType type;

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &rank, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &metric_id, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &type, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

            switch (type) {
                case MetricDataType::UINT64: {
                    uint64_t value_incl, value_excl;
                    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &value_incl, 1, MPI_LONG_LONG_INT,
                               MPI_COMM_WORLD);
                    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &value_excl, 1, MPI_LONG_LONG_INT,
                               MPI_COMM_WORLD);
                    get<2>(tmp_map.find(id)->second)
                        ->add_data(rank, metric_id, MetricData{type, value_incl, value_excl});
                    break;
                }
                case MetricDataType::DOUBLE: {
                    double value_incl, value_excl;
                    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &value_incl, 1, MPI_DOUBLE, MPI_COMM_WORLD);
                    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &value_excl, 1, MPI_DOUBLE, MPI_COMM_WORLD);
                    get<2>(tmp_map.find(id)->second)
                        ->add_data(rank, metric_id, MetricData{type, value_incl, value_excl});
                    break;
                }
                case MetricDataType::INT64: {
                    int64_t value_incl, value_excl;
                    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &value_incl, 1, MPI_LONG_LONG_INT,
                               MPI_COMM_WORLD);
                    MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &value_excl, 1, MPI_LONG_LONG_INT,
                               MPI_COMM_WORLD);
                    get<2>(tmp_map.find(id)->second)
                        ->add_data(rank, metric_id, MetricData{type, value_incl, value_excl});
                    break;
                }
            }
        }

        /* extra check that doesn't cost too much */
        fence = 0;
        MPI_Unpack(buffer, sizes[PACK_TOTAL_SIZE], &position, &fence, 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);
        assert(FENCE == fence);
    }

    alldata.call_path_tree.merge_tree(tmp_tree);
}

bool ReduceData(AllData& alldata) {
    bool error = false;

    assert(1 < alldata.metaData.numRanks);

    alldata.verbosePrint(1, true, "reducing data");

    /* implement reduction myself because MPI and C++ STL don't play with
    each other */

    /* how many rounds until master has all the data? */
    uint32_t num_rounds = std::log2(alldata.metaData.numRanks);
    uint32_t round_no   = 0;
    uint32_t round      = 1;
    while (round < alldata.metaData.numRanks) {
        round_no++;

        ostringstream msg;
        msg << " round " << round_no << " / " << num_rounds;
        alldata.verbosePrint(1, true, msg.str());

        uint32_t peer = alldata.metaData.myRank ^ round;

        /* if peer rank is not there, do nothing but go on */
        if (peer >= alldata.metaData.numRanks) {
            round = round << 1;
            continue;
        }

        /* send to smaller peer, receive from larger one */
        uint32_t sizes[PACK_NUM_PACKS];
        char*    buffer;

        if (alldata.metaData.myRank < peer) {
            MPI_Status status;

            MPI_Recv(sizes, PACK_NUM_PACKS, MPI_UNSIGNED, peer, 4, MPI_COMM_WORLD, &status);

            // DEBUG
            // cout << "    round " << round << " recv " << peer << "--> " <<
            // alldata.myRank << " with " <<
            // sizes[PACK_TOTAL_SIZE] << " bytes, " <<
            // sizes[PACK_FUNCTION] << ", " <<
            // sizes[PACK_FUNCTION_DURATION] << ", " <<
            // sizes[PACK_COUNTER] << ", " <<
            // sizes[PACK_MSG_GROUP_PAIR] << "" << endl << flush;

            buffer = prepare_worker_data(alldata, sizes);

            msg << ": receiving " << sizes[PACK_TOTAL_SIZE] << " bytes from rank " << peer;
            alldata.verbosePrint(2, false, msg.str());

            MPI_Recv(buffer, sizes[PACK_TOTAL_SIZE], MPI_PACKED, peer, 5, MPI_COMM_WORLD, &status);

            unpack_worker_data(alldata, sizes);

        } else {
            alldata.call_path_tree.serialize_data(mapping, f_data, m_data, c_data, met_data);

            buffer = pack_worker_data(alldata, sizes);

            // DEBUG
            // cout << "    round " << round << " send " << alldata.myRank <<
            //" --> " << peer << " with " <<
            // sizes[PACK_TOTAL_SIZE] << " bytes, " <<
            // sizes[PACK_FUNCTION] << ", " <<
            // sizes[PACK_FUNCTION_DURATION] << ", " <<
            // sizes[PACK_COUNTER] << ", " <<
            // sizes[PACK_MSG_GROUP_PAIR] << "" << endl << flush;

            msg << ": sending " << sizes[PACK_TOTAL_SIZE] << " bytes to rank " << peer;
            alldata.verbosePrint(2, false, msg.str());

            MPI_Send(sizes, PACK_NUM_PACKS, MPI_UNSIGNED, peer, 4, MPI_COMM_WORLD);

            MPI_Send(buffer, sizes[PACK_TOTAL_SIZE], MPI_PACKED, peer, 5, MPI_COMM_WORLD);

            /* every work has to send off its data at most once,
            after that, break from the collective reduction operation */
            break;
        }

        round = round << 1;
    }

    alldata.metaData.freePackBuffer();

    /* synchronize error indicator with workers */
    /*SyncError( alldata, error );*/

    return !error;
}
