#include <iostream>
#include <sstream>

#include "OTF2Reader.h"
#include "otf2/OTF2_Definitions.h"
#include "otf2/OTF2_GeneralDefinitions.h"

#ifdef OTFPROFILE_MPI
#include <otf2/OTF2_MPI_Collectives.h>
#endif

#if MPI_VERSION < 3
#define OTF2_MPI_UINT64_T MPI_UNSIGNED_LONG
#define OTF2_MPI_INT64_T MPI_LONG
#endif

using namespace std;

/** OTF2 reader handle */
// metric id (real), data
static map<uint64_t, MetricData>        tmp_metric;
static std::vector<uint64_t>            locationList;
static StringIdentifier<OTF2_StringRef> string_id;
static StringIdentifier<OTF2_IoFileRef> filesystem_entries;
// TODO remove
// static std::map<OTF2_StringRef, string> stringIdToString;
static uint64_t              systemTreeNodeId;
static std::deque<StackData> node_stack;

string OTF2ParadigmToString(OTF2_Paradigm paradigm) {
    switch (paradigm) {
        case OTF2_PARADIGM_UNKNOWN:
            return "UNKNOWN";

        case OTF2_PARADIGM_USER:
            return "USER";

        case OTF2_PARADIGM_COMPILER:
            return "COMPILER";

        case OTF2_PARADIGM_OPENMP:
            return "OPENMP";

        case OTF2_PARADIGM_MPI:
            return "MPI";

        case OTF2_PARADIGM_CUDA:
            return "CUDA";

        case OTF2_PARADIGM_MEASUREMENT_SYSTEM:
            return "MEASUREMENT_SYSTEM";

        case OTF2_PARADIGM_PTHREAD:
            return "PTHREAD";

        case OTF2_PARADIGM_HMPP:
            return "HMPP";

        case OTF2_PARADIGM_OMPSS:
            return "OMPSS";

        case OTF2_PARADIGM_HARDWARE:
            return "HARDWARE";

        case OTF2_PARADIGM_GASPI:
            return "GASPI";

        case OTF2_PARADIGM_UPC:
            return "UPC";

        case OTF2_PARADIGM_SHMEM:
            return "SHMEM";

        default:
            return "UNKNOWN";
    }
}

MetricMode mappingOTF2MetricMode( OTF2_MetricMode metricMode ){
    switch (metricMode){
        case OTF2_METRIC_ACCUMULATED_START:
            return MetricMode::ACCUMULATED_START;

        case OTF2_METRIC_ACCUMULATED_POINT:
            return MetricMode::ACCUMULATED_POINT;

        case OTF2_METRIC_ACCUMULATED_LAST:
            return MetricMode::ACCUMULATED_LAST;

        case OTF2_METRIC_ACCUMULATED_NEXT:
            return MetricMode::ACCUMULATED_NEXT;

        case OTF2_METRIC_ABSOLUTE_POINT:
            return MetricMode::ABSOLUTE_POINT;

        case OTF2_METRIC_ABSOLUTE_LAST:
            return MetricMode::ABSOLUTE_LAST;

        case OTF2_METRIC_ABSOLUTE_NEXT:
            return MetricMode::ABSOLUTE_NEXT;

        case OTF2_METRIC_RELATIVE_POINT:
            return MetricMode::RELATIVE_POINT;

        case OTF2_METRIC_RELATIVE_LAST:
            return MetricMode::RELATIVE_LAST;

        case OTF2_METRIC_RELATIVE_NEXT:
            return MetricMode::RELATIVE_NEXT;
    }
}

MetricType mappingOTF2MetricType( OTF2_MetricType metricType ){
    switch( metricType ) {

        case OTF2_METRIC_TYPE_PAPI:
            return MetricType::PAPI;

        case OTF2_METRIC_TYPE_RUSAGE:
            return MetricType::RUSAGE;

        case OTF2_METRIC_TYPE_USER:
            return MetricType::USER;

        // OTF2_METRIC_TYPE_OTHER
        default:
            return MetricType::OTHER;
    }
}

RecorderKind mappingOTF2MetricRecorderType( OTF2_RecorderKind recorderKind ){
    switch( recorderKind ) {
        case OTF2_RECORDER_KIND_ABSTRACT:
            return RecorderKind::ABSTRACT;

        case OTF2_RECORDER_KIND_CPU:
            return RecorderKind::CPU;

        case OTF2_RECORDER_KIND_GPU:
            return RecorderKind::GPU;

        // OTF2_RECORDER_KIND_UNKNOWN
        default:
            return RecorderKind::UNKNOWN;
    }
}

MetricOccurrence mappingOTF2MetricOccurrence( OTF2_MetricOccurrence metricOccurrence ){
    switch( metricOccurrence ){
        case OTF2_METRIC_SYNCHRONOUS_STRICT:
            return MetricOccurrence::SYNCHRONOUS_STRICT;

        case OTF2_METRIC_SYNCHRONOUS:
            return MetricOccurrence::SYNCHRONOUS;

        case OTF2_METRIC_ASYNCHRONOUS:
            return MetricOccurrence::ASYNCHRONOUS;
    }
}

bool OTF2Reader::initialize(AllData& alldata) {
    alldata.verbosePrint(1, true, "OTF2: reader initalization");
    for (auto para = (int)OTF2_PARADIGM_UNKNOWN; para != (int)OTF2_PARADIGM_SHMEM; ++para) {
        alldata.definitions.paradigms.add(para, {OTF2ParadigmToString(para)});
    }
    _reader = OTF2_Reader_Open(alldata.params.input_file_name.c_str());

    if (nullptr == _reader) {
        cerr << "Failed to open OTF2-Reader" << endl;
        return false;
    }
#ifdef OTFPROFILE_MPI
    OTF2_MPI_Reader_SetCollectiveCallbacks(_reader, MPI_COMM_WORLD);
#endif

    auto& number_locations = alldata.metaData.number_locations;
    OTF2_Reader_GetNumberOfLocations(_reader, &number_locations);

    if (number_locations < alldata.metaData.numRanks) {
        cerr << "Warning: more analysis ranks than trace processes, "
             << "ranks " << alldata.metaData.numRanks << " to " << number_locations << endl;
    }

    locationList.reserve(number_locations);

    // convert and add all OTF2 Paradigms
    auto& paradigms = alldata.definitions.paradigms;
    for (auto i = 0; i < OTF2_PARADIGM_NONE; ++i) {
        paradigms.add(i, {OTF2ParadigmToString(i)});
    }

    OTF2_ErrorCode ignored = OTF2_Reader_GetTraceId(_reader, &alldata.traceID);
    return true;
}

void OTF2Reader::close() {
    if (nullptr != _reader) {
        OTF2_Reader_Close(_reader);
        _reader = nullptr;
    }
}

/* ****************************************************************** */
/*                                                                    */
/*                            DEFINITIONS                             */
/*                                                                    */
/* ****************************************************************** */

OTF2_CallbackCode OTF2Reader::handle_def_io_handle(void* userData, OTF2_IoHandleRef self, OTF2_StringRef name,
                                                   OTF2_IoFileRef file, OTF2_IoParadigmRef ioParadigm,
                                                   OTF2_IoHandleFlag ioHandleFlags, OTF2_CommRef comm,
                                                   OTF2_IoHandleRef parent) {
    auto* alldata = static_cast<AllData*>(userData);
    if (file != OTF2_UNDEFINED_IO_FILE) {
        auto strings = filesystem_entries.get(file);
        if (strings.second != OTF2_CALLBACK_SUCCESS)
            return strings.second;
        alldata->definitions.iohandles.add(self,
                                           {*strings.first[0], ioParadigm, file, parent, std::set<std::string>()});
        return OTF2_CALLBACK_SUCCESS;
    } else {
        auto strings = string_id.get(name);
        if (strings.second != OTF2_CALLBACK_SUCCESS)
            return strings.second;
        alldata->definitions.iohandles.add(self,
                                           {*strings.first[0], ioParadigm, file, parent, std::set<std::string>()});
    }
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_io_fs_entry(void* userData, OTF2_IoFileRef self, OTF2_StringRef name,
                                                     OTF2_SystemTreeNodeRef scope) {
    auto strings = string_id.get(name);
    if (strings.second != OTF2_CALLBACK_SUCCESS)
        return strings.second;
    filesystem_entries.add(self, strings.first[0]->c_str());
    return OTF2_CALLBACK_SUCCESS;
}

#if VERSION_OTF2_MAJOR >= 3
    OTF2_CallbackCode OTF2Reader::handle_def_clock_properties(void* userData, uint64_t timerResolution,
                                                              uint64_t globalOffset, uint64_t traceLength, uint64_t realtimeTimestamp) {        //OTF2 3.x
#else
    OTF2_CallbackCode OTF2Reader::handle_def_clock_properties(void* userData, uint64_t timerResolution,
                                                              uint64_t globalOffset, uint64_t traceLength) {                                    //OTF2 2.x
#endif
    auto* alldata = static_cast<AllData*>(userData);

    alldata->metaData.timerResolution = timerResolution;

    return OTF2_CALLBACK_SUCCESS;
}

/* TODO nicht verwendet
OTF2_CallbackCode OTF2Reader::handle_def_attribute(void* userData, OTF2_AttributeRef self,
                                                   OTF2_StringRef name, OTF2_StringRef description,
                                                   OTF2_Type type) {
    return OTF2_CALLBACK_SUCCESS;
}
*/
    // OTF2_GlobalDefReaderCallback_MetricMember
OTF2_CallbackCode OTF2Reader::handle_def_metrics(   void*                   userData,
                                        OTF2_MetricMemberRef    self,
                                        OTF2_StringRef          name,
                                        OTF2_StringRef          description,
                                        OTF2_MetricType         metricType,
                                        OTF2_MetricMode         metricMode,
                                        OTF2_Type               valueType,
                                        OTF2_Base               base,
                                        int64_t                 exponent,
                                        OTF2_StringRef          unit
                                    ){

    auto* alldata = static_cast<AllData*>(userData);
    auto  strings = string_id.get(name, description, unit);

    if (strings.second != OTF2_CALLBACK_SUCCESS)
        return strings.second;

    MetricDataType a_type;

    switch (valueType) {
        case OTF2_TYPE_UINT64:
            a_type = MetricDataType::UINT64;
            break;
        case OTF2_TYPE_INT64:
            a_type = MetricDataType::INT64;
            break;
        case OTF2_TYPE_DOUBLE:
            a_type = MetricDataType::DOUBLE;
            break;
        default:
            return OTF2_CALLBACK_INTERRUPT;
    }

    // if (metricMode == OTF2_METRIC_ACCUMULATED_START) {
    //     alldata->definitions.metrics.add(self,
    //                                      {*strings.first[0], *strings.first[1], *strings.first[2], a_type, false});
    // }

    definitions::Metric metric{
        *strings.first[0],                  // name
        *strings.first[1],                  // description
        mappingOTF2MetricType(metricType),  //PAPI, etc.
        mappingOTF2MetricMode(metricMode),  //accumulative, relative, etc.
        a_type,                             // type of the value: OTF2_TYPE_INT64, etc.
        base == OTF2_BASE_BINARY ? MetricBase::BINARY : MetricBase::DECIMAL,
        exponent,
        *strings.first[2],                  // unit
        false
    };

    alldata->definitions.metrics.add(self, metric);

    return OTF2_CALLBACK_SUCCESS;
}

// for metrics -> narrowing the metrics to synchronous
OTF2_CallbackCode OTF2Reader::handle_def_metric_class(  void*                       userData,
                                                        OTF2_MetricRef              self,
                                                        uint8_t                     numberOfMetrics,
                                                        const OTF2_MetricMemberRef* metricMembers,
                                                        OTF2_MetricOccurrence       metricOccurrence,
                                                        OTF2_RecorderKind           recorderKind) {



    auto* alldata = static_cast<AllData*>(userData);

    if( recorderKind != OTF2_RECORDER_KIND_ABSTRACT) {
        if(metricOccurrence == OTF2_METRIC_SYNCHRONOUS_STRICT){

            definitions::Metric_Class metric_class {
                numberOfMetrics,
                std::map<uint8_t, uint32_t>(),
                mappingOTF2MetricOccurrence(metricOccurrence),
                mappingOTF2MetricRecorderType(recorderKind)
            };

            for (int i = 0; i < numberOfMetrics; ++i) {
                auto* def_ref = alldata->definitions.metrics.get(metricMembers[i]);
                if (def_ref != nullptr){
                    metric_class.metric_member[i] = metricMembers[i];
                        const_cast<definitions::Metric*>(def_ref)->allowed = true;
                }
            }
            alldata->definitions.metric_classes.add(self, metric_class);
        }
    }


    return OTF2_CALLBACK_SUCCESS;
}

#if VERSION_OTF2_MAJOR >= 3
    OTF2_CallbackCode OTF2Reader::handle_def_location_group(void* userData, OTF2_LocationGroupRef groupIdentifier,
                                                            OTF2_StringRef name, OTF2_LocationGroupType locationGroupType,
                                                            OTF2_SystemTreeNodeRef systemTreeParent, OTF2_LocationGroupRef creatingLocationGroup) {     //OTF2 3.x
#else
    OTF2_CallbackCode OTF2Reader::handle_def_location_group(void* userData, OTF2_LocationGroupRef groupIdentifier,
                                                            OTF2_StringRef name, OTF2_LocationGroupType locationGroupType,
                                                            OTF2_SystemTreeNodeRef systemTreeParent) {                                                  //OTF2 2.x
#endif
    auto* alldata = static_cast<AllData*>(userData);

    auto strings = string_id.get(name);
    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }

    alldata->definitions.system_tree.insert_node(*strings.first[0], groupIdentifier,
                                                 definitions::SystemClass::LOCATION_GROUP, systemTreeParent);

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_location(void* userData, OTF2_LocationRef locationIdentifier,
                                                  OTF2_StringRef name, OTF2_LocationType locationType,
                                                  uint64_t numberOfEvents, OTF2_LocationGroupRef locationGroup) {
    auto* alldata = static_cast<AllData*>(userData);

    auto strings = string_id.get(name);
    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }
    auto& location_name = *strings.first[0];

    ostringstream os;
    if (location_name.length() == 0) {
        // fix for semi broken traces with no location name
        switch (locationType) {
            case OTF2_LOCATION_TYPE_CPU_THREAD:
                os << "Thread " << locationIdentifier;
                break;

            case OTF2_LOCATION_TYPE_GPU:
                os << "GPU_Thread " << locationIdentifier;
                break;

            default:
                return OTF2_CALLBACK_SUCCESS;  // nicht bekannt oder metric -> wird nicht in sysTree übernommen
        }

    } else {
        os << location_name;
    }

    alldata->definitions.system_tree.insert_node(os.str(), locationIdentifier, definitions::SystemClass::LOCATION,
                                                 locationGroup);

    if (locationType == OTF2_LOCATION_TYPE_CPU_THREAD || locationType == OTF2_LOCATION_TYPE_GPU) {
        locationList.push_back(locationIdentifier);
    }

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_group(void* userData, OTF2_GroupRef groupIdentifier, OTF2_StringRef name,
                                               OTF2_GroupType groupType, OTF2_Paradigm paradigm,
                                               OTF2_GroupFlag groupFlags, uint32_t numberOfMembers,
                                               const uint64_t* members) {
    auto* alldata = static_cast<AllData*>(userData);

    auto strings = string_id.get(name);
    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }

    vector<uint64_t> members_vec(numberOfMembers);

    for (uint32_t i = 0; i < numberOfMembers; ++i)
        members_vec[i] = members[i];

    alldata->definitions.groups.add(groupIdentifier, {*strings.first[0], groupType, paradigm, std::move(members_vec)});

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_region(void* userData, OTF2_RegionRef regionIdentifier, OTF2_StringRef name,
                                                OTF2_StringRef canonicalName, OTF2_StringRef description,
                                                OTF2_RegionRole regionRole, OTF2_Paradigm paradigm,
                                                OTF2_RegionFlag regionFlags, OTF2_StringRef sourceFile,
                                                uint32_t beginLineNumber, uint32_t endLineNumber) {
    auto* alldata = static_cast<AllData*>(userData);

    auto strings = string_id.get(name, sourceFile);
    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }

    alldata->definitions.regions.add(regionIdentifier,
                                     {*strings.first[0], paradigm, beginLineNumber, *strings.first[1]});

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_system_tree_node(void* userData, OTF2_SystemTreeNodeRef systemTreeIdentifier,
                                                          OTF2_StringRef name, OTF2_StringRef className,
                                                          OTF2_SystemTreeNodeRef parent) {
    auto* alldata = static_cast<AllData*>(userData);

    auto strings = string_id.get(name, className);
    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }

    auto& nameclass = *strings.first[1];

    definitions::SystemClass classtype;

    if (0 == nameclass.compare("machine")) {
        classtype = definitions::SystemClass::MACHINE;

    } else if (0 == nameclass.compare("cabinet row")) {
        classtype = definitions::SystemClass::CABINET_ROW;

    } else if (0 == nameclass.compare("cabinet") || 0 == nameclass.compare("rack")) {
        classtype = definitions::SystemClass::CABINET;

    } else if (0 == nameclass.compare("cage") || 0 == nameclass.compare("midplane")) {
        classtype = definitions::SystemClass::CAGE;

    } else if (0 == nameclass.compare("blade") || 0 == nameclass.compare("nodeboard")) {
        classtype = definitions::SystemClass::BLADE;

    } else if (0 == nameclass.compare("node") || 0 == nameclass.compare("nodecard")) {
        classtype = definitions::SystemClass::NODE;

    } else {
        classtype = definitions::SystemClass::OTHER;
    }

    nameclass.append(" " + *strings.first[0]);
    alldata->definitions.system_tree.insert_node(nameclass, systemTreeIdentifier, classtype, parent);

    return OTF2_CALLBACK_SUCCESS;
}

#if VERSION_OTF2_MAJOR >= 3
    OTF2_CallbackCode OTF2Reader::handle_def_comm(void* userData, OTF2_CommRef self, OTF2_StringRef name,
                                                  OTF2_GroupRef group, OTF2_CommRef parent, OTF2_CommFlag flag) {   //OTF2 3.x
#else
    OTF2_CallbackCode OTF2Reader::handle_def_comm(void* userData, OTF2_CommRef self, OTF2_StringRef name,
                                                  OTF2_GroupRef group, OTF2_CommRef parent) {                       //OTF2 2.x
#endif
    auto* alldata                         = static_cast<AllData*>(userData);
    alldata->metaData.communicators[self] = group;

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_string(void* userData, OTF2_StringRef stringIdentifier, const char* string) {
    string_id.add(stringIdentifier, string);

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_paradigm(void* userData, OTF2_Paradigm paradigm, OTF2_StringRef name,
                                                  OTF2_ParadigmClass paradigmClass) {
    auto* alldata = static_cast<AllData*>(userData);
    auto  strings = string_id.get(name);

    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }

    alldata->definitions.paradigms.add(paradigm, {*strings.first[0]});

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_io_paradigm(void* userData, OTF2_IoParadigmRef paradigm, OTF2_StringRef id,
                                                     OTF2_StringRef name, OTF2_IoParadigmClass paradigmClass,
                                                     OTF2_IoParadigmFlag flags, uint8_t numProperties,
                                                     const OTF2_IoParadigmProperty* properties, const OTF2_Type* types,
                                                     const OTF2_AttributeValue* values) {
    auto* alldata = static_cast<AllData*>(userData);
    auto  strings = string_id.get(name);

    if (strings.second != OTF2_CALLBACK_SUCCESS) {
        return strings.second;
    }

    alldata->definitions.io_paradigms.add(paradigm, {*strings.first[0]});

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_def_io_precreated_handle(void* userData, OTF2_IoHandleRef handle,
                                                              OTF2_IoAccessMode mode, OTF2_IoStatusFlag statusFlags) {
    auto* alldata = static_cast<AllData*>(userData);
    auto* ioh     = alldata->definitions.iohandles.get(handle);
    if (!ioh)
        return OTF2_CALLBACK_ERROR;
    switch (mode) {
        case OTF2_IO_ACCESS_MODE_READ_ONLY:
            ioh->modes.insert("R");
            break;
        case OTF2_IO_ACCESS_MODE_WRITE_ONLY:
            ioh->modes.insert("W");
            break;
        case OTF2_IO_ACCESS_MODE_READ_WRITE:
            ioh->modes.insert("R");
            ioh->modes.insert("W");
            break;
        case OTF2_IO_ACCESS_MODE_EXECUTE_ONLY:
            ioh->modes.insert("X");
            break;
        case OTF2_IO_ACCESS_MODE_SEARCH_ONLY:
            ioh->modes.insert("S");
            break;
        default:
            return OTF2_CALLBACK_ERROR;
    }
    return OTF2_CALLBACK_SUCCESS;
}

/* ****************************************************************** */
/*                                                                    */
/*                               EVENTS                               */
/*                                                                    */
/* ****************************************************************** */

struct PendingIoEvt {
    OTF2_TimeStamp begin_time;
    uint64_t       bytes_request;
};

static std::map<uint64_t, PendingIoEvt> open_io_events;

OTF2_CallbackCode OTF2Reader::handle_io_begin(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                              void* userData, OTF2_AttributeList* attributeList,
                                              OTF2_IoHandleRef handle, OTF2_IoOperationMode mode,
                                              OTF2_IoOperationFlag flag, uint64_t bytesRequest, uint64_t matchingId) {
    open_io_events[matchingId] = {time, bytesRequest};
    auto* alldata              = static_cast<AllData*>(userData);
    auto* h                    = alldata->definitions.iohandles.get(handle);
    if (!h)
        return OTF2_CALLBACK_ERROR;
    switch (mode) {
        case OTF2_IO_OPERATION_MODE_READ:
            h->modes.insert("R");
            break;
        case OTF2_IO_OPERATION_MODE_WRITE:
            h->modes.insert("W");
            break;
        case OTF2_IO_OPERATION_MODE_FLUSH:
        default:
            break;
    }

    return OTF2_CALLBACK_SUCCESS;
}
OTF2_CallbackCode OTF2Reader::handle_io_end(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                            void* userData, OTF2_AttributeList* attributeList, OTF2_IoHandleRef handle,
                                            uint64_t bytesResult, uint64_t matchingId) {
    auto* alldata     = static_cast<AllData*>(userData);
    auto  found_start = open_io_events.find(matchingId);
    if (found_start != open_io_events.end()) {
        auto duration  = time - found_start->second.begin_time;
        auto bytes_req = found_start->second.bytes_request;
        open_io_events.erase(found_start);
        auto h = alldata->definitions.iohandles.get(handle);
        if (!h)
            return OTF2_CALLBACK_ERROR;  // event on undefined IO handle
        uint64_t p = h->io_paradigm;
        alldata->io_data[p].num_operations++;
        if (bytesResult != OTF2_UNDEFINED_UINT64) {
            alldata->io_data[p].num_bytes += bytesResult;
            alldata->io_data[p].transfer_time += duration;
        } else {
            alldata->io_data[p].nontransfer_time += duration;
        }
    }
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_io_create_handle(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                      uint64_t eventPosition, void* userData,
                                                      OTF2_AttributeList* attributeList, OTF2_IoHandleRef handle,
                                                      OTF2_IoAccessMode mode, OTF2_IoCreationFlag creationFlags,
                                                      OTF2_IoStatusFlag statusFlags) {
    auto* alldata = static_cast<AllData*>(userData);
    auto* ioh     = alldata->definitions.iohandles.get(handle);
    switch (mode) {
        case OTF2_IO_ACCESS_MODE_READ_ONLY:
            ioh->modes.insert("R");
            break;
        case OTF2_IO_ACCESS_MODE_WRITE_ONLY:
            ioh->modes.insert("W");
            break;
        case OTF2_IO_ACCESS_MODE_READ_WRITE:
            ioh->modes.insert("R");
            ioh->modes.insert("W");
            break;
        case OTF2_IO_ACCESS_MODE_EXECUTE_ONLY:
            ioh->modes.insert("X");
            break;
        case OTF2_IO_ACCESS_MODE_SEARCH_ONLY:
            ioh->modes.insert("S");
            break;
        default:
            return OTF2_CALLBACK_ERROR;
    }
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_metric(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                            void* userData, OTF2_AttributeList* attributeList, OTF2_MetricRef metric,
                                            uint8_t numberOfMetrics, const OTF2_Type* typeIDs,
                                            const OTF2_MetricValue* metricValues) {
    auto* alldata = static_cast<AllData*>(userData);

    auto class_mapping = alldata->definitions.metric_classes.get(metric);
    if (class_mapping != nullptr) {
        auto* metric_class_def = alldata->definitions.metric_classes.get(metric);

        if(metric_class_def->metric_occurrence == MetricOccurrence::SYNCHRONOUS_STRICT){

            MetricDataType a_type;

            for (uint8_t i = 0; i < numberOfMetrics; i++) {
                auto  metric_ref = class_mapping->metric_member.find(i);
                auto* metric_def = alldata->definitions.metrics.get(metric_ref->second);
                if (metric_ref != class_mapping->metric_member.end() && metric_def != nullptr && metric_def->allowed) {
                    MetricData md;

                    if (typeIDs[i] == OTF2_TYPE_UINT64) {
                        md = {MetricDataType::UINT64, metricValues[i].unsigned_int, metricValues[i].unsigned_int};
                    } else if (typeIDs[i] == OTF2_TYPE_INT64) {
                        md = {MetricDataType::INT64, metricValues[i].signed_int, metricValues[i].signed_int};
                    } else if (typeIDs[i] == OTF2_TYPE_DOUBLE) {
                        md = {MetricDataType::DOUBLE, metricValues[i].floating_point, metricValues[i].floating_point};
                    }

                    tmp_metric.insert(make_pair(metric_ref->second, md));
                }
            }
        }
    }

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_enter(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                           void* userData, OTF2_AttributeList* attributeList, OTF2_RegionRef region)

{
    auto*      alldata = static_cast<AllData*>(userData);

    alldata->metaData.min_time_stamp = std::min(alldata->metaData.min_time_stamp, time);
    tree_node* tmp_node;

    if (!node_stack.empty()) {
        auto tmp = node_stack.front().node_p;

        auto tmp_child = tmp->children.find(region);
        if (tmp_child == tmp->children.end()) {
            tmp_node = alldata->call_path_tree.insert_node(region, tmp);
        } else {
            tmp_node = tmp_child->second.get();
        }

    } else {
        auto root_node = alldata->call_path_tree.root_nodes.find(region);

        if (root_node == alldata->call_path_tree.root_nodes.end()) {
            tmp_node = alldata->call_path_tree.insert_node(region, nullptr);
        } else {
            tmp_node = root_node->second.get();
        }
    }

    tmp_node->add_data(locationID);
    auto& node_metrics = tmp_node->last_data->metrics;

    if (!tmp_metric.empty()) {
        for (auto it : tmp_metric) {
            auto metric_ref = node_metrics.find(it.first);

            if (metric_ref == node_metrics.end()) {
                metric_ref = node_metrics.insert(make_pair(it.first, MetricData{it.second.type})).first;
            }

            metric_ref->second -= it.second;

            if (tmp_node->parent != nullptr) {
                auto parent_metric_ref = tmp_node->parent->last_data->metrics.find(it.first);
                parent_metric_ref->second.add_incl(it.second);
            }
        }

        tmp_metric.clear();
    }

    node_stack.push_front({tmp_node, time, 0});

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_leave(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                           void* userData, OTF2_AttributeList* attributeList, OTF2_RegionRef region) {
    auto* alldata = static_cast<AllData*>(userData);

    alldata->metaData.max_time_stamp = std::max(alldata->metaData.max_time_stamp, time);

    auto&    tmp       = node_stack.front();
    uint64_t incl_time = time - tmp.time;
    tmp.node_p->add_data(locationID, FunctionData{1, incl_time, incl_time - tmp.child_incl});

    // ugly metric stuff
    auto* tmp_node(tmp.node_p);
    auto& node_metrics = tmp_node->last_data->metrics;
    if (!tmp_metric.empty()) {
        for (auto it = tmp_metric.begin(); it != tmp_metric.end(); it++) {
            auto metric_ref = node_metrics.find(it->first);

            if (metric_ref == node_metrics.end()) {
                metric_ref = node_metrics.insert(make_pair(it->first, MetricData{it->second.type})).first;
            }

            metric_ref->second += it->second;
            if (tmp_node->parent != nullptr) {
                auto parent_metric_ref = tmp_node->parent->last_data->metrics.find(it->first);
                parent_metric_ref->second.sub_incl(it->second);
            }
        }

        tmp_metric.clear();
    }
    node_stack.pop_front();
    if (!node_stack.empty()) {
        node_stack.front().child_incl += incl_time;
    }

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_mpi_send(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                              void* userData, OTF2_AttributeList* attributeList, uint32_t receiver,
                                              OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength) {
    auto* alldata = static_cast<AllData*>(userData);

    auto& tmp = node_stack.front();
    tmp.node_p->add_data(locationID, MessageData{1, 0, msgLength, 0});
    // TODO workaround
    tmp.node_p->has_p2p = true;

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_mpi_recv(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                              void* userData, OTF2_AttributeList* attributeList, uint32_t sender,
                                              OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength) {
    auto* alldata = static_cast<AllData*>(userData);

    auto& tmp = node_stack.front();
    tmp.node_p->add_data(locationID, MessageData{0, 1, 0, msgLength});
    // TODO workaround
    tmp.node_p->has_p2p = true;

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_mpi_isend(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                               void* userData, OTF2_AttributeList* attributeList, uint32_t receiver,
                                               OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength,
                                               uint64_t requestID) {
    auto* alldata = static_cast<AllData*>(userData);

    auto& tmp = node_stack.front();
    tmp.node_p->add_data(locationID, MessageData{1, 0, msgLength, 0});
    // TODO workaround
    tmp.node_p->has_p2p = true;

    return OTF2_CALLBACK_SUCCESS;
}
/* TODO nicht verwendet
OTF2_CallbackCode OTF2Reader::handle_mpi_isend_complete(OTF2_LocationRef locationID,
                                                        OTF2_TimeStamp time, uint64_t eventPosition,
                                                        void*               userData,
                                                        OTF2_AttributeList* attributeList,
                                                        uint64_t            requestID) {
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_mpi_irecv_request(OTF2_LocationRef locationID,
                                                       OTF2_TimeStamp time, uint64_t eventPosition,
                                                       void*               userData,
                                                       OTF2_AttributeList* attributeList,
                                                       uint64_t            requestID) {
    return OTF2_CALLBACK_SUCCESS;
}
*/
OTF2_CallbackCode OTF2Reader::handle_mpi_irecv(OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition,
                                               void* userData, OTF2_AttributeList* attributeList, uint32_t sender,
                                               OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength,
                                               uint64_t requestID) {
    auto* alldata = static_cast<AllData*>(userData);

    auto& tmp = node_stack.front();
    tmp.node_p->add_data(locationID, MessageData{0, 1, 0, msgLength});
    // TODO workaround
    tmp.node_p->has_p2p = true;

    return OTF2_CALLBACK_SUCCESS;
}
// TODO evtl nützlich aber nicht verwendet
/*
OTF2_CallbackCode OTF2Reader::handle_buffer_flush(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                  void* userData, OTF2_AttributeList* attributeList,
                                                  OTF2_TimeStamp stopTime) {
    return OTF2_CALLBACK_SUCCESS;
}
*/
/* TODO nicht verwendet
OTF2_CallbackCode OTF2Reader::handle_mpi_request_test(OTF2_LocationRef locationID,
                                                      OTF2_TimeStamp time, uint64_t eventPosition,
                                                      void*               userData,
                                                      OTF2_AttributeList* attributeList,
                                                      uint64_t            requestID) {
    return OTF2_CALLBACK_SUCCESS;
}
*/
/*
OTF2_CallbackCode OTF2Reader::handle_mpi_collective_begin(OTF2_LocationRef locationID,
                                                          OTF2_TimeStamp   time,
                                                          uint64_t eventPosition, void* userData,
                                                          OTF2_AttributeList* attributeList) {
    return OTF2_CALLBACK_SUCCESS;
}
*/
OTF2_CallbackCode OTF2Reader::handle_mpi_collective_end(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                        uint64_t eventPosition, void* userData,
                                                        OTF2_AttributeList* attributeList, OTF2_CollectiveOp type,
                                                        OTF2_CommRef communicator, uint32_t root, uint64_t sizeSent,
                                                        uint64_t sizeReceived) {
    if (type == OTF2_COLLECTIVE_OP_BARRIER)
        return OTF2_CALLBACK_SUCCESS;

    auto* alldata = static_cast<AllData*>(userData);

    auto& tmp = node_stack.front();

    if (sizeSent > 0) {
        tmp.node_p->add_data(locationID, CollopData{1, 0, sizeSent, 0});
    }

    if (sizeReceived > 0) {
        tmp.node_p->add_data(locationID, CollopData{0, 1, 0, sizeReceived});
    }
    // TODO workaround
    tmp.node_p->has_collop = true;

    return OTF2_CALLBACK_SUCCESS;
}

// TODO callbacks raus für rma - nicht verwendet
/*
OTF2_CallbackCode OTF2Reader::handle_rma_put(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                             void* userData, OTF2_AttributeList* attributeList,
                                             OTF2_RmaWinRef win, uint32_t remote, uint64_t bytes,
                                             uint64_t matchingId) {
        alldata* alldata = ( alldata* )userData;

        auto it = alldata->call_path_tree.rma_map.find( locationID );

        if( it != alldata->call_path_tree.rma_map.end() ) {

            it->second.add( 1, 0, bytes, 0 );

        } else {

            alldata->call_path_tree.rma_map.insert( make_pair( locationID, rma_data( 1, 0, bytes, 0
       ) ) );

        }
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_rma_get(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                             void* userData, OTF2_AttributeList* attributeList,
                                             OTF2_RmaWinRef win, uint32_t remote, uint64_t bytes,
                                             uint64_t matchingId) {

        alldata* alldata = ( alldata* )userData;

        auto it = alldata->call_path_tree.rma_map.find( locationID );

        if( it != alldata->call_path_tree.rma_map.end() ) {

            it->second.add( 1, 0, bytes, 0 );

        } else {

            alldata->call_path_tree.rma_map.insert( make_pair( locationID, rma_data( 0, 1, 0, bytes
       ) ) );

        }

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode OTF2Reader::handle_rma_n_block_complete(OTF2_LocationRef locationID,
                                                          OTF2_TimeStamp time, void* userData,
                                                          OTF2_AttributeList* attributeList,
                                                          OTF2_RmaWinRef win, uint64_t matchingId) {
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Reader::handle_rma_block_complete(OTF2_LocationRef locationID,
                                                        OTF2_TimeStamp time, void* userData,
                                                        OTF2_AttributeList* attributeList,
                                                        OTF2_RmaWinRef win, uint64_t matchingId) {
    return OTF2_CALLBACK_SUCCESS;
}
*/
/*TODO nicht verwendet
OTF2_CallbackCode OTF2Reader::handle_unknown(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                             void* userData, OTF2_AttributeList* attributeList) {
    return OTF2_CALLBACK_SUCCESS;
}

*/

bool OTF2Reader::readDefinitions(AllData& alldata) {
    alldata.verbosePrint(1, true, "OTF2: read definitions");

    OTF2_ErrorCode status;

    OTF2_GlobalDefReader* glob_def_reader = OTF2_Reader_GetGlobalDefReader(_reader);

    if (NULL == glob_def_reader)
        return false;

    OTF2_GlobalDefReaderCallbacks* glob_def_callbacks = OTF2_GlobalDefReaderCallbacks_New();

    if (NULL == glob_def_callbacks)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetClockPropertiesCallback(glob_def_callbacks, handle_def_clock_properties);

    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetLocationGroupCallback(glob_def_callbacks, handle_def_location_group);

    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetLocationCallback(glob_def_callbacks, handle_def_location);

    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetGroupCallback(glob_def_callbacks, handle_def_group);

    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetRegionCallback(glob_def_callbacks, handle_def_region);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetIoRegularFileCallback(glob_def_callbacks, handle_def_io_fs_entry);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetIoDirectoryCallback(glob_def_callbacks, handle_def_io_fs_entry);

    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetIoParadigmCallback(glob_def_callbacks, handle_def_io_paradigm);

    if (OTF2_SUCCESS != status)
        return false;
    /* TODO nicht verwendet
        status = OTF2_GlobalDefReaderCallbacks_SetAttributeCallback(glob_def_callbacks,
                                                                    handle_def_attribute);

        if (OTF2_SUCCESS != status)
            return false;
    */
    if (alldata.params.read_metrics) {
        status = OTF2_GlobalDefReaderCallbacks_SetMetricMemberCallback(glob_def_callbacks, handle_def_metrics);
        if (OTF2_SUCCESS != status)
            return false;

        status = OTF2_GlobalDefReaderCallbacks_SetMetricClassCallback(glob_def_callbacks, handle_def_metric_class);
        if (OTF2_SUCCESS != status)
            return false;
    }

    status = OTF2_GlobalDefReaderCallbacks_SetSystemTreeNodeCallback(glob_def_callbacks, handle_def_system_tree_node);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetCommCallback(glob_def_callbacks, handle_def_comm);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetStringCallback(glob_def_callbacks, handle_def_string);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetParadigmCallback(glob_def_callbacks, handle_def_paradigm);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_GlobalDefReaderCallbacks_SetIoHandleCallback(glob_def_callbacks, handle_def_io_handle);
    if (OTF2_SUCCESS != status)
        return false;
    status = OTF2_GlobalDefReaderCallbacks_SetIoPreCreatedHandleStateCallback(glob_def_callbacks,
                                                                              handle_def_io_precreated_handle);
    if (OTF2_SUCCESS != status)
        return false;

    status = OTF2_Reader_RegisterGlobalDefCallbacks(_reader, glob_def_reader, glob_def_callbacks, &alldata);
    if (OTF2_SUCCESS != status)
        return false;
    uint64_t definitions_read = 0;
    status                    = OTF2_Reader_ReadAllGlobalDefinitions(_reader, glob_def_reader, &definitions_read);
    if (OTF2_SUCCESS != status) {
        std::cerr << "ERROR: Could not read definitions from OTF2 trace." << std::endl;
        return false;
    }

    OTF2_GlobalDefReaderCallbacks_Delete(glob_def_callbacks);

    OTF2_Reader_CloseDefFiles(_reader);

    return true;
}

bool OTF2Reader::readEvents(AllData& alldata) {
    alldata.verbosePrint(1, true, "OTF2: read events");

    uint64_t otf2_STEP = OTF2_UNDEFINED_UINT64;
    uint64_t events_read;

    OTF2_ErrorCode  status;
    OTF2_EvtReader* local_evt_reader;

    OTF2_EvtReaderCallbacks* evt_callbacks = OTF2_EvtReaderCallbacks_New();

    if (NULL == evt_callbacks)
        return false;

    OTF2_EvtReaderCallbacks_SetEnterCallback(evt_callbacks, handle_enter);
    OTF2_EvtReaderCallbacks_SetLeaveCallback(evt_callbacks, handle_leave);

    OTF2_EvtReaderCallbacks_SetMpiSendCallback(evt_callbacks, handle_mpi_send);
    OTF2_EvtReaderCallbacks_SetMpiIsendCallback(evt_callbacks, handle_mpi_isend);
    // OTF2_EvtReaderCallbacks_SetMpiIsendCompleteCallback(evt_callbacks, handle_mpi_isend_complete); TODO nicht
    // verwendet

    // OTF2_EvtReaderCallbacks_SetMpiIrecvRequestCallback(evt_callbacks, handle_mpi_irecv_request); TODO nicht verwendet
    OTF2_EvtReaderCallbacks_SetMpiRecvCallback(evt_callbacks, handle_mpi_recv);
    OTF2_EvtReaderCallbacks_SetMpiIrecvCallback(evt_callbacks, handle_mpi_irecv);

    // OTF2_EvtReaderCallbacks_SetMpiRequestTestCallback(evt_callbacks, handle_mpi_request_test); TODO nicht verwendet

    /*TODO nicht verwendet
    OTF2_EvtReaderCallbacks_SetMpiCollectiveBeginCallback(evt_callbacks,
                                                          handle_mpi_collective_begin);
    */
    OTF2_EvtReaderCallbacks_SetMpiCollectiveEndCallback(evt_callbacks, handle_mpi_collective_end);

    OTF2_EvtReaderCallbacks_SetMetricCallback(evt_callbacks, handle_metric);
    OTF2_EvtReaderCallbacks_SetIoOperationBeginCallback(evt_callbacks, handle_io_begin);
    OTF2_EvtReaderCallbacks_SetIoOperationCompleteCallback(evt_callbacks, handle_io_end);
    OTF2_EvtReaderCallbacks_SetIoCreateHandleCallback(evt_callbacks, handle_io_create_handle);
#ifndef OTFPROFILE_MPI

    OTF2_DefReader* local_def_reader;

    for (const auto location : locationList) {
        local_def_reader = OTF2_Reader_GetDefReader(_reader, location);
        uint64_t definitions_read;
        status = OTF2_Reader_ReadAllLocalDefinitions(_reader, local_def_reader, &definitions_read);
        if (OTF2_SUCCESS != status) {
            std::cerr << "ERROR: Could not read local definitions from OTF2 trace." << std::endl;
            return false;
        }

        local_evt_reader = OTF2_Reader_GetEvtReader(_reader, location);

        if (NULL == local_evt_reader)
            return false;

        status = OTF2_Reader_RegisterEvtCallbacks(_reader, local_evt_reader, evt_callbacks, &alldata);
        status = OTF2_Reader_ReadLocalEvents(_reader, local_evt_reader, otf2_STEP, &events_read);
        node_stack.clear();

        if (OTF2_SUCCESS != status)
            std::cerr << "Error while reading events from OTF2 trace." << std::endl;
    }
    /* Clean up */
    OTF2_EvtReaderCallbacks_Delete(evt_callbacks);

#else

    MPI_Win shared_space;

    uint64_t* val_p;

    uint64_t to_read = 0;
    uint64_t initial = -1;
    uint64_t result;

    if (alldata.metaData.myRank == 0) {
        MPI_Win_allocate(sizeof(uint64_t), sizeof(uint64_t), MPI_INFO_NULL, MPI_COMM_WORLD, &val_p, &shared_space);
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
            /*
             * read local definitions of that rank before reading local events
             * reading local definition enables the internal mapping of OTF2 between local and global definitions
             * ->necessary and !NOT! mandatory
             */
            OTF2_DefReader* local_def_reader;
            local_def_reader = OTF2_Reader_GetDefReader(_reader, locationList[to_read]);
            uint64_t definitions_read;
            status = OTF2_Reader_ReadAllLocalDefinitions(_reader, local_def_reader, &definitions_read);
            if (OTF2_SUCCESS != status) {
                std::cerr << "ERROR: Could not read local definitions from OTF2 trace." << std::endl;
                return false;
            }

            status = OTF2_Reader_CloseDefReader(_reader, local_def_reader);
            if (OTF2_SUCCESS != status) {
                return false;
            }
            // end local def reading

            local_evt_reader = OTF2_Reader_GetEvtReader(_reader, locationList[to_read]);
            if (NULL == local_evt_reader)
                return false;

            status = OTF2_Reader_RegisterEvtCallbacks(_reader, local_evt_reader, evt_callbacks, &alldata);
            status = OTF2_Reader_ReadLocalEvents(_reader, local_evt_reader, otf2_STEP, &events_read);

            if (OTF2_SUCCESS != status) {
                std::cerr << "Error while reading events from OTF2 trace." << std::endl;
            }

            node_stack.clear();
            initial = to_read;
            ++to_read;

        } else {
            MPI_Get(&initial, 1, MPI_LONG_LONG_INT, 0, 0, 1, MPI_LONG_LONG_INT, shared_space);
            to_read = initial + 1;
        }
    }

    /* Clean up */
    MPI_Win_unlock_all(shared_space);
    MPI_Win_free(&shared_space);
    OTF2_EvtReaderCallbacks_Delete(evt_callbacks);

#endif

    /*
     * Callbacks not implemented yet
     *
     OTF2_GlobalEvtReaderCallbacks_SetBufferFlushCallback( evt_callbacks,
     c3g::Otf2Reader::handleBufferFlush );
     OTF2_GlobalEvtReaderCallbacks_SetOmpForkCallback( evt_callbacks, c3g::Otf2Reader::handleOmpFork
     );
     OTF2_GlobalEvtReaderCallbacks_SetOmpTaskCreateCallback( evt_callbacks,
     c3g::Otf2Reader::handleOmpTaskCreate );
     OTF2_GlobalEvtReaderCallbacks_SetOmpTaskSwitchCallback( evt_callbacks,
     c3g::Otf2Reader::handleOmpTaskSwitch );
     OTF2_GlobalEvtReaderCallbacks_SetOmpTaskCompleteCallback( evt_callbacks,
     c3g::Otf2Reader::handleOmpTaskComplete );
     OTF2_GlobalEvtReaderCallbacks_SetOmpJoinCallback( evt_callbacks, c3g::Otf2Reader::handleOmpJoin
     );
     OTF2_GlobalEvtReaderCallbacks_SetOmpAcquireLockCallback( evt_callbacks,
     c3g::Otf2Reader::handleOmpAcquireLock );
     OTF2_GlobalEvtReaderCallbacks_SetOmpReleaseLockCallback( evt_callbacks,
     c3g::Otf2Reader::handleOmpReleaseLock );
     OTF2_GlobalEvtReaderCallbacks_SetUnknownCallback( evt_callbacks, c3g::Otf2Reader::handleUnknown
     );
     */

    return true;
}
// TODO nicht verwendet im moment
bool OTF2Reader::readStatistics(AllData& alldata) { return true; }
