#include <cassert>
#include <iostream>
#include <sstream>
#include <stack>

#include "OTFReader.h"

#include <otfaux.h>

#ifdef OTFPROFILE_MPI
#include <mpi.h>
#endif
using namespace std;

static uint64_t systemTreeNodeId = -1;
static std::vector<uint64_t> myProcessesList;
static map<uint64_t, vector<MetricData>> tmp_metric;
static std::vector<uint64_t> locationList;
static std::map<uint64_t, std::deque<StackData>> global_node_stack;

bool OTFReader::initialize(AllData &alldata) {
    alldata.verbosePrint(1, true, "OTF: reader initalization");
    _manager = OTF_FileManager_open(alldata.params.max_file_handles);

    if (nullptr == _manager) {
        cerr << "Failed to open OTF-FileManager" << endl;
        return false;
    }

    _reader = OTF_Reader_open(alldata.params.input_file_prefix.c_str(), _manager);
    if (nullptr == _reader) {
        cerr << "Failed to open OTF-Reader" << endl;
        return false;
    }

    OTF_FileManager *  manager = nullptr;
    OTF_MasterControl *master  = nullptr;

    manager = OTF_FileManager_open(1);
    assert(manager);

    master = OTF_MasterControl_new(manager);
    assert(master);

    int test_read = OTF_MasterControl_read(master, alldata.params.input_file_prefix.c_str());

    /* that's the first access to the input trace file; show tidy error
       message if failed */
    if (0 == test_read) {
        cerr << "ERROR: Unable to open file '" << alldata.params.input_file_prefix
             << ".otf' for reading." << endl;
        OTF_MasterControl_close(master);
        OTF_FileManager_close(manager);
        return false;
    }

    /* fill the global array of processes */

    locationList.reserve(master->n);
    for (auto i = 0; i < master->n; ++i)
        locationList.push_back(master->map[i].argument);

    /* close OTF master control and file manager */
    OTF_MasterControl_close(master);
    OTF_FileManager_close(manager);

    return true;
}

void OTFReader::close() {
    /* close OTF file manager and reader */
    if (nullptr != _reader)
        OTF_Reader_close(_reader);

    if (nullptr != _manager)
        OTF_FileManager_close(_manager);
}

/* ****************************************************************** */
/*                                                                    */
/*                            DEFINITIONS                             */
/*                                                                    */
/* ****************************************************************** */
/* TODO nicht verwendet
int OTFReader::handle_def_creator(void *fha, uint32_t stream, const char *creator,
                                  OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}
*/
/* TODO nicht verwendet
int OTFReader::handle_def_version(void *fha, uint32_t stream, uint8_t major, uint8_t minor,
                                  uint8_t sub, const char *suffix, OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}
*/
/* TODO nicht verwendet
int OTFReader::handle_def_comment(void *fha, uint32_t stream, const char *comment,
                                  OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}
*/

int OTFReader::handle_def_timerres(void *fha, uint32_t stream, uint64_t ticksPerSecond,
                                   OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    alldata->metaData.timerResolution = ticksPerSecond;

    return OTF_RETURN_OK;
}

int OTFReader::handle_def_process(void *fha, uint32_t stream, uint32_t process, const char *name,
                                  uint32_t parent, OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    if((alldata->definitions.system_tree).size() == 0){
        alldata->definitions.system_tree.insert_node("machine", 0, definitions::SystemClass::MACHINE, (uint32_t) -1);
    }

    if( parent == 0 ) {
        ++systemTreeNodeId;

        alldata->definitions.system_tree.insert_node(name, systemTreeNodeId, definitions::SystemClass::LOCATION_GROUP, 0);
        alldata->definitions.system_tree.insert_node(name, process, definitions::SystemClass::LOCATION, systemTreeNodeId);
    } else {
        alldata->definitions.system_tree.insert_node(name, process, definitions::SystemClass::LOCATION, systemTreeNodeId);
    }

    return OTF_RETURN_OK;
}
/*TODO Reimplementation mit Anpassung auf OTF2
int OTFReader::handle_def_processgroup( void* fha, uint32_t stream, uint32_t procGroup,
                                   const char*	name, uint32_t numberOfProcs, const uint32_t*
procs, OTF_KeyValueList* list ) {



    return OTF_RETURN_OK;

}*/

int OTFReader::handle_def_functiongroup(void *fha, uint32_t stream, uint32_t funcGroup,
                                        const char *name, OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    alldata->definitions.groups.add(funcGroup, {name, 0, 0, {}});

    return OTF_RETURN_OK;
}

int OTFReader::handle_def_function(void *fha, uint32_t stream, uint32_t function, const char *name,
                                   uint32_t funcGroup, uint32_t source, OTF_KeyValueList *kvlist) {

    auto* alldata = static_cast<AllData*>(fha);
    // TODO better solution necessary -> funcGroup is used as pradigm here
    alldata->definitions.regions.add(function, {name, funcGroup, source, ""});

    return OTF_RETURN_OK;
}
/*TODO nicht verwendet
int OTFReader::handle_def_collop(void *fha, uint32_t stream, uint32_t collOp, const char *name,
                                 uint32_t type, OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}
*/
int OTFReader::handle_def_counter(void *fha, uint32_t stream, uint32_t counter, const char *name,
                                  uint32_t properties, uint32_t counterGroup, const char *unit,
                                  OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    if ((properties & OTF_COUNTER_TYPE_ABS) != OTF_COUNTER_TYPE_ABS) {
        if ((properties & OTF_COUNTER_SCOPE_START) == OTF_COUNTER_SCOPE_START) {
            MetricDataType a_type;

            if (properties < 256) {
                if ((properties & 32) == 32) {
                    a_type = MetricDataType::INT64;
                } else {
                    a_type = MetricDataType::UINT64;
                }
            } else {
                a_type = MetricDataType::DOUBLE;
            }

            alldata->definitions.metrics.add(counter, {name, "" /*description*/, "" /*unit*/,
                a_type, true});
        }
    }

    return OTF_RETURN_OK;
}
/*TODO nicht verwendet
int OTFReader::handle_def_keyvalue(void *fha, uint32_t stream, uint32_t key, OTF_Type type,
                                   const char *name, const char *description,
                                   OTF_KeyValueList *kvlist) {
    // AllData* alldata= (AllData*) fha;

    return OTF_RETURN_OK;
}
*/
/* ****************************************************************** */
/*                                                                    */
/*                               EVENTS                               */
/*                                                                    */
/* ****************************************************************** */

int OTFReader::handle_enter(void *fha, uint64_t time, uint32_t function, uint32_t process,
                            uint32_t source, OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    tree_node* tmp_node;
    //explezit kein find benutzt -> subscript operator null-initialisiert wenn nichts vorhanden ist -> sonst gleiches verhalten
    auto& local_stack = global_node_stack[ process ];

    if (!local_stack.empty()) {
        auto tmp = local_stack.front().node_p;
        auto tmp_child = tmp->children.find(function);

        if (tmp_child == tmp->children.end()) {
            tmp_node = alldata->call_path_tree.insert_node((uint64_t) function, tmp);
        } else {
            tmp_node = tmp_child->second.get();
        }
    } else {
        auto root_node = alldata->call_path_tree.root_nodes.find(function);

        if (root_node == alldata->call_path_tree.root_nodes.end()) {
            tmp_node = alldata->call_path_tree.insert_node((uint64_t) function, nullptr);
        } else {
            tmp_node = root_node->second.get();
        }
    }

    tmp_node->add_data(process, FunctionData{0, 0, 0});
    local_stack.push_front({tmp_node, time, 0});

    return OTF_RETURN_OK;
}

int OTFReader::handle_leave(void *fha, uint64_t time, uint32_t function, uint32_t process,
                            uint32_t source, OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    //implizite annahme das sich beim leaver immer min. ein element im stack befindet -> sonst seg. fault
    auto& local_stack = global_node_stack.find(process)->second;
    auto& tmp = local_stack.front();
    uint64_t incl_time = time - tmp.time;
    tmp.node_p->add_data(process, FunctionData{1, incl_time, incl_time - tmp.child_incl});

    // metric-counter stuff
    auto tmp_node(tmp.node_p);
    auto &node_metrics = tmp_node->last_data->metrics;

    for (auto it = tmp_metric.begin(); it != tmp_metric.end(); ++it) {
        // TODO produziert segfaults wenn die metriken nicht strikt synchron sind
        MetricData incl_metric(it->second.back());
        incl_metric -= it->second.front();
        auto metric_ref = node_metrics.find(it->first);

        if (metric_ref == node_metrics.end()) {
            metric_ref =
                node_metrics.insert(make_pair(it->first, MetricData{incl_metric.type})).first;
        }

        metric_ref->second += incl_metric;
        if (tmp_node->parent != nullptr) {
            auto parent_metric_ref = tmp_node->parent->last_data->metrics.find(it->first);
            if (parent_metric_ref == tmp_node->parent->last_data->metrics.end()) {
                parent_metric_ref = tmp_node->parent->last_data->metrics
                                        .insert(make_pair(it->first, MetricData{incl_metric.type}))
                                        .first;
            }
            parent_metric_ref->second.add_incl(incl_metric);
        }
        it->second.clear();
    }
    // metric-counter stuff end
    local_stack.pop_front();

    if (!local_stack.empty()) {
        local_stack.front().child_incl += incl_time;
    }

    return OTF_RETURN_OK;
}

int OTFReader::handle_counter(void *fha, uint64_t time, uint32_t process, uint32_t counter,
                              uint64_t value, OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    auto* counter_ref = alldata->definitions.metrics.get(counter);

    if (counter_ref != nullptr) {
        MetricData md{counter_ref->type, value, value};
        auto stack_ref = tmp_metric.find(counter);

        if (stack_ref == tmp_metric.end()) {
            stack_ref = tmp_metric.insert(make_pair(counter, vector<MetricData>())).first;
        }
        stack_ref->second.push_back(md);
    }

    return OTF_RETURN_OK;
}
/*TODO nicht verwendet
int OTFReader::handle_rma_put(void *fha, uint64_t time, uint32_t process, uint32_t origin,
                              uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes,
                              uint32_t source, OTF_KeyValueList *list) {
     AllData* alldata = ( AllData* )fha;

     auto it = alldata->call_path_tree.rma_map.find( process );

     if( it != alldata->call_path_tree.rma_map.end() ) {

         it->second.add( 1, 0, bytes, 0 );

     } else {

         alldata->call_path_tree.rma_map.insert( make_pair( process, rma_data( 1, 0, bytes, 0 ) ) );

     }

    return OTF_RETURN_OK;
}

int OTFReader::handle_rma_get(void *fha, uint64_t time, uint32_t process, uint32_t origin,
                              uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes,
                              uint32_t source, OTF_KeyValueList *list) {
    /*AllData* alldata = ( AllData* )fha;

    auto it = alldata->call_path_tree.rma_map.find( process );

    if( it != alldata->call_path_tree.rma_map.end() ) {

        it->second.add( 1, 0, bytes, 0 );

    } else {

        alldata->call_path_tree.rma_map.insert( make_pair( process, rma_data( 0, 1, 0, bytes ) ) );

    }

    return OTF_RETURN_OK;
}
*/
int OTFReader::handle_send(void *fha, uint64_t time, uint32_t sender, uint32_t receiver,
                           uint32_t group, uint32_t type, uint32_t length, uint32_t source,
                           OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    auto& tmp = global_node_stack.find(sender)->second.front();
    tmp.node_p->add_data(sender, MessageData{1, 0, length, 0});

//TODO workaround
    tmp.node_p->has_p2p = true;

    return OTF_RETURN_OK;
}

int OTFReader::handle_recv(void *fha, uint64_t time, uint32_t receiver, uint32_t sender,
                           uint32_t group, uint32_t type, uint32_t length, uint32_t source,
                           OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    auto& tmp = global_node_stack.find(receiver)->second.front();
    tmp.node_p->add_data(receiver, MessageData{0, 1, 0, length});

//TODO workaround
    tmp.node_p->has_p2p = true;

    return OTF_RETURN_OK;
}

int OTFReader::handle_collop(void *fha, uint64_t time, uint32_t process, uint32_t collOp,
                             uint32_t procGroup, uint32_t rootProc, uint32_t sent,
                             uint32_t received, uint64_t duration, uint32_t source,
                             OTF_KeyValueList *kvlist) {
    auto* alldata = static_cast<AllData*>(fha);

    auto& tmp = global_node_stack.find(process)->second.front();

    if (sent > 0) {
        tmp.node_p->add_data(process, CollopData{1, 0, sent, 0});
    }

    if (received > 0) {
        tmp.node_p->add_data(process, CollopData{0, 1, 0, received});
    }

//TODO workaround
    tmp.node_p->has_collop = true;

    return OTF_RETURN_OK;
}

/*TODO nicht verwendet
int OTFReader::handle_begin_collop(void *fha, uint64_t time, uint32_t process, uint32_t collOp,
                                   uint64_t matchingId, uint32_t procGroup, uint32_t rootProc,
                                   uint64_t sent, uint64_t received, uint32_t scltoken,
                                   OTF_KeyValueList *kvlist) {
//TODO bringt irgendwie nichts -> handle_collop bringt daten....
    auto mapping =
        alldata->metaData.traceIdToSystemTreeId.find(make_pair(definitions::SystemClass::LOCATION, process));

    return OTF_RETURN_OK;
}

int OTFReader::handle_end_collop(void *fha, uint64_t time, uint32_t process, uint64_t matchingId,
                                 OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}

int OTFReader::handle_function_summary(void *fha, uint64_t time, uint32_t func, uint32_t process,
                                       uint64_t count, uint64_t exclTime, uint64_t inclTime,
                                       OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}

int OTFReader::handle_message_summary(void *fha, uint64_t time, uint32_t process, uint32_t peer,
                                      uint32_t comm, uint32_t type, uint64_t sentNumber,
                                      uint64_t receivedNumber, uint64_t sentBytes,
                                      uint64_t receivedBytes, OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}

int OTFReader::handle_collop_summary(void *fha, uint64_t time, uint32_t process, uint32_t comm,
                                     uint32_t collOp, uint64_t sentNumber, uint64_t receivedNumber,
                                     uint64_t sentBytes, uint64_t receivedBytes,
                                     OTF_KeyValueList *kvlist) {
    return OTF_RETURN_OK;
}
*/
bool OTFReader::readDefinitions(AllData &alldata) {
    alldata.verbosePrint(1, true, "OTF: read definitions");

    /* open OTF handler array */
    OTF_HandlerArray *handlers = OTF_HandlerArray_open();
    assert(handlers);

    /* set record handler functions */
    /*TODO nicht verwendet
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_creator,
                                OTF_DEFCREATOR_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_version,
                                OTF_DEFVERSION_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_comment,
                                OTF_DEFINITIONCOMMENT_RECORD);
    */
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_timerres,
                                OTF_DEFTIMERRESOLUTION_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_process,
                                OTF_DEFPROCESS_RECORD);

    /*TODO    OTF_HandlerArray_setHandler( handlers,
                                    (OTF_FunctionPointer*) handle_def_processgroup,
                                    OTF_DEFPROCESSGROUP_RECORD );
    */
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_function,
                                OTF_DEFFUNCTION_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_functiongroup,
                                OTF_DEFFUNCTIONGROUP_RECORD);
    /*TODO nicht verwendet
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_collop,
                                OTF_DEFCOLLOP_RECORD);
    */
    if( alldata.params.read_metrics ) {

        OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_counter,
                                    OTF_DEFCOUNTER_RECORD);

    }

    /* TODO nicht verwendet
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_def_keyvalue,
                                OTF_DEFKEYVALUE_RECORD);
    */
    /* set record handler's first arguments */
    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFCREATOR_RECORD); TODO nicht verwendet
    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFVERSION_RECORD); TODO nicht verwendet
    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFINITIONCOMMENT_RECORD); TODO nicht verwendet
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFTIMERRESOLUTION_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFPROCESS_RECORD);
    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFPROCESSGROUP_RECORD); TODO nicht verwendet
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFFUNCTION_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFFUNCTIONGROUP_RECORD);
    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFCOLLOP_RECORD); TODO nicht verwendet
    if( alldata.params.read_metrics ) {

        OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFCOUNTER_RECORD);

    }

    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_DEFKEYVALUE_RECORD);

    /* read definitions */
    uint64_t read_ret = OTF_Reader_readDefinitions(_reader, handlers);

    if (OTF_READ_ERROR == read_ret) {
        cerr << "ERROR: Could not read definitions." << endl;
        return false;
    }

    /* close OTF handler array */
    OTF_HandlerArray_close(handlers);

    return true;
}

bool OTFReader::readEvents(AllData &alldata) {
    alldata.verbosePrint(1, true, "OTF: read events");

    /* open OTF handler array */
    OTF_HandlerArray *handlers = OTF_HandlerArray_open();
    assert(handlers);

    /* set record handler functions */

    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_enter, OTF_ENTER_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_leave, OTF_LEAVE_RECORD);

    if( alldata.params.read_metrics ) {
        OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_counter,
                                    OTF_COUNTER_RECORD);
    }

    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_send, OTF_SEND_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_recv, OTF_RECEIVE_RECORD);
//    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_begin_collop,
//                                OTF_BEGINCOLLOP_RECORD);
//    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_end_collop,
//                                OTF_ENDCOLLOP_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_collop,
                                OTF_COLLOP_RECORD);
    /*TODO nicht verwendet
    //OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_rma_put, OTF_RMAPUT_RECORD); TODO nicht verwendet
    //OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_rma_get, OTF_RMAGET_RECORD); TODO nicht verwendet

    /* set record handler's first arguments */

    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_ENTER_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_LEAVE_RECORD);

    if( alldata.params.read_metrics ) {
        OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_COUNTER_RECORD);

    }

    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_SEND_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_RECEIVE_RECORD);
//    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_BEGINCOLLOP_RECORD); TODO nicht verwendet
//    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_ENDCOLLOP_RECORD); TODO nicht verwendet
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_COLLOP_RECORD);

    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_RMAPUT_RECORD); TODO nicht verwendet
    //OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_RMAGET_RECORD); TODO nicht verwendet

    /* select processes to read */
    uint64_t records_read = 0;

#ifdef OTFPROFILE_MPI

    MPI_Win shared_space;

    uint64_t *val_p;

    uint64_t to_read = 0;
    uint64_t initial = -1;
    uint64_t result;

    if (alldata.metaData.myRank == 0) {
        MPI_Win_allocate(sizeof(uint64_t), sizeof(uint64_t), MPI_INFO_NULL, MPI_COMM_WORLD, &val_p,
                         &shared_space);
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, shared_space);
        MPI_Put(&initial, 1, MPI_LONG_LONG_INT, 0, 0, 1, MPI_LONG_LONG_INT, shared_space);
        MPI_Win_unlock(0, shared_space);
    } else {
        MPI_Win_allocate(0, sizeof(uint64_t), MPI_INFO_NULL, MPI_COMM_WORLD, &val_p, &shared_space);
    }

    MPI_Win_fence(0, shared_space);
    MPI_Win_lock_all(0, shared_space);

    while (to_read < locationList.size()) {
        MPI_Compare_and_swap(&to_read, &initial, &result, MPI_LONG_LONG_INT, 0, 0, shared_space);

        if (result == initial) {
            auto areader = OTF_RStream_open(alldata.params.input_file_prefix.c_str(),
                                            locationList[to_read], _manager);

            if (OTF_RStream_readEvents(areader, handlers) == 0) {
                return false;
            }

            initial = to_read;
            ++to_read;
            global_node_stack.clear();

        } else {
            MPI_Get(&initial, 1, MPI_LONG_LONG_INT, 0, 0, 1, MPI_LONG_LONG_INT, shared_space);
            to_read = initial + 1;
        }
    }

    MPI_Win_unlock_all(shared_space);
    MPI_Win_free(&shared_space);

#else

    // processes (locations in OTF2) start with 1 rather then 0
    for (uint32_t i = 0; i < locationList.size(); ++i) {
        auto areader =
            OTF_RStream_open(alldata.params.input_file_prefix.c_str(), locationList[i], _manager);

        if (OTF_RStream_readEvents(areader, handlers) == 0) {
            return false;
        }

        OTF_RStream_close(areader);
        global_node_stack.clear();
    }

#endif
    /* Clean up */
    OTF_HandlerArray_close( handlers );

    return true;
}
//TODO nicht verwendet im moment
bool OTFReader::readStatistics(AllData &alldata) {
    bool error = false;

    /* open OTF handler array */
    OTF_HandlerArray *handlers = OTF_HandlerArray_open();
    assert(handlers);

    /* set record handler functions */
    /*TODO nicht verwendet
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_function_summary,
                                OTF_FUNCTIONSUMMARY_RECORD);
    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_message_summary,
                                OTF_MESSAGESUMMARY_RECORD);

    OTF_HandlerArray_setHandler(handlers, (OTF_FunctionPointer *)handle_collop_summary,
                                OTF_COLLOPSUMMARY_RECORD);
    */
    /* set record handler's first arguments */

    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_FUNCTIONSUMMARY_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_MESSAGESUMMARY_RECORD);
    OTF_HandlerArray_setFirstHandlerArg(handlers, &alldata, OTF_COLLOPSUMMARY_RECORD);

    /* select processes to read */
    OTF_Reader_setProcessStatusAll(_reader, 0);

    for (uint32_t i = 0; i < myProcessesList.size(); i++) {
        OTF_Reader_enableProcess(_reader, myProcessesList[i]);
    }
    /* read statistics */
    uint64_t records_read = 0;

    while (OTF_READ_ERROR != (records_read = OTF_Reader_readStatistics(_reader, handlers))) {
        /* stop reading if done */
        if (0 == records_read) {
            break;
        }
    }

    /* show error message if reading failed */
    if (OTF_READ_ERROR == records_read) {
        cerr << "ERROR: Could not read statistics." << endl;
        error = true;
    }

    /* close OTF handler array */
    OTF_HandlerArray_close(handlers);

    return !error;
}

