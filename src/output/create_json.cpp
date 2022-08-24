/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal, Bill Williams
*/
#include <fstream>
#include <iostream>
#include <string>
#include "all_data.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
//#include "rapidjson/writer.h"

#include <rapidjson/encodings.h>
#include <rapidjson/rapidjson.h>
#include <stddef.h>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <new>
#include <set>
#include <utility>
#include "data_tree.h"
#include "definitions.h"
#include "main_structs.h"
#include "utils.h"

using namespace rapidjson;
using std::cout;
using std::string;

template <typename os_t>
class PlainWriter {
    os_t& m_stream;

   public:
    PlainWriter(os_t& output) : m_stream(output) {}
    template <typename T>
    void Key(T k) const {
        m_stream << k << ": ";
    }
    void Uint64(uint64_t u) const { m_stream << u << "\n"; }
    void Uint(uint32_t u) const { m_stream << u << "\n"; }
    void StartObject() const {}
    void EndObject() const {}
    void StartArray() const {}
    void EndArray() const {}
};

struct ProfileEntry {
    std::map<std::string, uint64_t> entries;
    void                            add_data(const std::string& key, uint64_t value) {
        if (value == 0 || value == (uint64_t)(-1))
            return;
        entries[key] += value;
    }
    template <typename Writer>
    void WriteProfile(Writer& w) const {
        w.StartObject();
        for (const auto& kv : entries) {
            w.Key(StringRef(kv.first.c_str()));
            w.Uint64(kv.second);
        }
        w.EndObject();
    }
};

using definitions::Definitions;
using definitions::IoHandle;
struct FileInfo {
    FileInfo() : parentfile(NULL) {}
    FileInfo(const Definitions& defs, uint64_t id) {
        parentfile           = NULL;
        const IoHandle* self = defs.iohandles.get(id);
        if (!self)
            return;
        const IoHandle* parent = defs.iohandles.get(self->parent);
        if (parent)
            parentfile = new FileInfo(defs, self->parent);
        filename = self->name;
        paradigm.insert(defs.io_paradigms.get(self->io_paradigm)->name);
        modes = self->modes;
    }
    void operator+=(const FileInfo& rhs) {
        filename = rhs.filename;
        if (rhs.parentfile && !parentfile)
            parentfile = rhs.parentfile;
        std::copy(rhs.paradigm.begin(), rhs.paradigm.end(), std::inserter(paradigm, paradigm.begin()));
        std::copy(rhs.modes.begin(), rhs.modes.end(), std::inserter(modes, modes.begin()));
    }
    std::string           filename;
    std::set<std::string> paradigm, modes;
    FileInfo*             parentfile;
    template <typename Writer>
    void WriteFileInfo(Writer& w) const {
        w.StartObject();
        w.Key("FileName");
        w.String(filename.c_str());

        w.Key("IoParadigm");
        w.StartArray();
        for (auto pstr : paradigm) {
            w.String(pstr.c_str());
        }
        w.EndArray();
        w.Key("AccessModes");
        std::string merged_modes;
        for (auto modestr : modes) {
            merged_modes += modestr;
        }
        w.String(merged_modes.c_str());
        w.Key("ParentFile");
        if (parentfile && parentfile->filename != filename) {
            parentfile->WriteFileInfo(w);
        } else {
            w.Null();
        }
        w.EndObject();
    }
};

struct WorkflowProfile {
    uint64_t                            job_id;
    uint32_t                            node_count;
    uint32_t                            process_count;
    uint32_t                            thread_count;
    uint64_t                            timer_resolution;
    std::map<std::string, uint64_t>     counters;
    std::map<std::string, ProfileEntry> functions_by_paradigm;
    std::map<std::string, ProfileEntry> messages_by_paradigm;
    std::map<std::string, ProfileEntry> collops_by_paradigm;
    std::map<std::string, ProfileEntry> io_ops_by_paradigm;
    std::map<std::string, FileInfo>     file_data;
    uint64_t                            parallel_region_time;
    uint64_t                            serial_time;
    uint64_t                            num_functions;
    uint64_t                            num_invocations;
    std::string                         filename;
    uint64_t                            traceID;
    template <typename Writer>
    void WriteProfile(Writer& w) const;
    WorkflowProfile()
        : job_id(0),
          node_count(0),
          process_count(0),
          thread_count(0),
          parallel_region_time(0),
          serial_time(0),
          num_functions(0),
          num_invocations(0) {}
};

template <typename Map, typename Writer>
void WriteMapUnderKey(std::string key, const Map& m, Writer& w) {
    if (m.empty())
        return;
    w.Key(StringRef(key.c_str()));
    w.StartObject();
    for (const auto& kv : m) {
        w.Key(StringRef(kv.first.c_str()));
        kv.second.WriteProfile(w);
    }
    w.EndObject();
}

template <typename Writer>
void WorkflowProfile::WriteProfile(Writer& w) const {
    w.StartObject();
    w.Key("Trace");
    w.StartObject();
    w.Key("FileName");
    w.String(filename.c_str());
    w.Key("Id");
    w.Uint64(traceID);
    w.EndObject();
    w.Key("JobId");
    w.Uint64(job_id);
    w.Key("NodeCount");
    w.Uint(node_count);
    w.Key("ProcessCount");
    w.Uint(process_count);
    w.Key("ThreadCount");
    w.Uint(thread_count);
    w.Key("TimerResolution");
    w.Uint64(timer_resolution);
    w.Key("HardwareCounters");
    w.StartArray();
    for (const auto& c : counters) {
        w.Key(StringRef(c.first.c_str()));
        w.Uint64(c.second);
    }
    w.EndArray();
    WriteMapUnderKey("Functions", functions_by_paradigm, w);
    WriteMapUnderKey("Messages", messages_by_paradigm, w);
    WriteMapUnderKey("CollectiveOperations", collops_by_paradigm, w);
    WriteMapUnderKey("IOOperations", io_ops_by_paradigm, w);
    w.Key("Files");
    w.StartArray();
    for (auto f : file_data) {
        f.second.WriteFileInfo(w);
    }
    w.EndArray();
    w.Key("ParallelRegionTime");
    w.Uint64(parallel_region_time);
    w.Key("SerialRegionTime");
    w.Uint64(serial_time);
    w.Key("TotalFunctions");
    w.Uint64(num_functions);
    w.Key("TotalCalls");
    w.Uint64(num_invocations);
    w.EndObject();
}

bool CreateJSON(AllData& alldata) {
    cout << "Creating JSON profile" << std::endl;
    WorkflowProfile            profile;
    StringBuffer               b;
    PrettyWriter<StringBuffer> w(b);
    for (const auto& n : alldata.definitions.system_tree) {
        switch (n.data.class_id) {
            case definitions::SystemClass::LOCATION:
                profile.thread_count++;
                break;
            case definitions::SystemClass::LOCATION_GROUP:
                profile.process_count++;
                break;
            case definitions::SystemClass::NODE:
                profile.node_count++;
                break;
            default:
                break;
        }
    }
    static std::string bytestr("Bytes");
    static std::string timestr("Time");
    static std::string countstr("Count");
    profile.timer_resolution = alldata.metaData.timerResolution;

    for (const auto& call_node : alldata.call_path_tree) {
        const auto& r = alldata.definitions.regions.get(call_node.function_id);
        if (!r) {
            continue;
        }

        const auto& p        = alldata.definitions.paradigms.get(r->paradigm_id);
        std::string paradigm = "COMPUTE";
        if (p) {
            paradigm = p->name;
        }

        profile.num_functions++;
        uint64_t excl_time = 0;

        for (const auto& one_node_data : call_node.node_data) {
            profile.num_invocations += one_node_data.second.f_data.count;
            auto& entries_map = profile.functions_by_paradigm[paradigm].entries;
            entries_map[countstr] += one_node_data.second.f_data.count;
            if (excl_time < one_node_data.second.f_data.excl_time) {
                excl_time = one_node_data.second.f_data.excl_time;
            }
            auto&       message_entry = profile.messages_by_paradigm[paradigm];
            auto&       collop_entry  = profile.collops_by_paradigm[paradigm];
            const auto& message_data  = one_node_data.second.m_data;
            const auto& collop_data   = one_node_data.second.c_data;
            message_entry.add_data(bytestr, message_data.bytes_send);
            message_entry.add_data(bytestr, message_data.bytes_recv);
            message_entry.add_data(countstr, message_data.count_send);
            message_entry.add_data(countstr, message_data.count_recv);
            if (one_node_data.second.c_data.bytes_send)
                profile.collops_by_paradigm[paradigm].entries[bytestr] += one_node_data.second.c_data.bytes_send;
            if (one_node_data.second.c_data.bytes_recv)
                profile.collops_by_paradigm[paradigm].entries[bytestr] += one_node_data.second.c_data.bytes_recv;
            if (one_node_data.second.c_data.count_send)
                profile.collops_by_paradigm[paradigm].entries[countstr] += one_node_data.second.c_data.count_send;
            if (one_node_data.second.c_data.count_recv)
                profile.collops_by_paradigm[paradigm].entries[countstr] += one_node_data.second.c_data.count_recv;
            for (const auto& metric : one_node_data.second.metrics) {
                if (metric.second.type == MetricDataType::UINT64) {
                    auto m = alldata.definitions.metrics.get(metric.first);
                    if (m)
                        profile.counters[m->name] += (uint64_t)metric.second.data_excl;
                }
            }
        }
        if (call_node.node_data.size() == 1) {
            profile.serial_time += excl_time;
        } else {
            profile.parallel_region_time += excl_time;
        }
        profile.functions_by_paradigm[paradigm].entries[timestr] += excl_time;
    }
    static std::string meta_time     = "MetaOperationTime";
    static std::string transfer_time = "TransferOperationTime";
    for (auto io_entry : alldata.io_data) {
        std::string paradigm_name = alldata.definitions.io_paradigms.get(io_entry.first)->name;
        cout << "Summarizing io for " << paradigm_name << std::endl;
        profile.io_ops_by_paradigm[paradigm_name].entries[bytestr] += io_entry.second.num_bytes;
        profile.io_ops_by_paradigm[paradigm_name].entries[countstr] += io_entry.second.num_operations;
        profile.io_ops_by_paradigm[paradigm_name].entries[transfer_time] += io_entry.second.transfer_time;
        profile.io_ops_by_paradigm[paradigm_name].entries[meta_time] += io_entry.second.nontransfer_time;
    }
    for (auto file_entry : alldata.definitions.iohandles.get_all()) {
        auto file_handle = file_entry.second;
        profile.file_data[file_handle.name] += FileInfo(alldata.definitions, file_entry.first);
    }
    profile.filename = alldata.params.input_file_name;
    profile.traceID  = alldata.traceID;
    profile.WriteProfile(w);
    string        fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname.c_str());
    outfile << b.GetString() << std::endl;
    return true;
}
