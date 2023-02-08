/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal, Bill Williams
*/
#include <fstream>
#include <map>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "all_data.h"
#include "main_structs.h"

struct SummaryObject {
    MinMaxSum<uint64_t> count;
    MinMaxSum<uint64_t> excl_time;
    MinMaxSum<uint64_t> incl_time;
    MinMaxSum<uint64_t> count_send;
    MinMaxSum<uint64_t> count_recv;
    MinMaxSum<uint64_t> bytes_send;
    MinMaxSum<uint64_t> bytes_recv;

    SummaryObject& operator+=(const FunctionDataStats& rhs) {
        count += rhs.count;
        incl_time += rhs.incl_time;
        excl_time += rhs.excl_time;

        return *this;
    }

    SummaryObject& operator+=(const MessageData& rhs) {
        count_send += rhs.count_send;
        count_recv += rhs.count_recv;
        bytes_send += rhs.bytes_send;
        bytes_recv += rhs.bytes_recv;

        return *this;
    }

    SummaryObject& operator+=(const SummaryObject& rhs) {
        count += rhs.count;
        incl_time += rhs.incl_time;
        excl_time += rhs.excl_time;
        count_send += rhs.count_send;
        count_recv += rhs.count_recv;
        bytes_send += rhs.bytes_send;
        bytes_recv += rhs.bytes_recv;

        return *this;
    }
};

std::map<uint64_t, SummaryObject> summary_para;
std::map<uint64_t, SummaryObject> summary_reg;
uint64_t depth = 0;
uint64_t size = 0;

template<typename T>
rapidjson::Value get_minMaxSum(const MinMaxSum<T>& data, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("min", data.min, alloc);
    obj.AddMember("max", data.max, alloc);
    obj.AddMember("sum", data.sum, alloc);

    return obj;
}


template<typename T>
rapidjson::Value get_minMaxSumSeconds(const MinMaxSum<T>& data, uint64_t timerResolution, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("min", static_cast<double>(data.min) / timerResolution, alloc);
    obj.AddMember("max", static_cast<double>(data.max) / timerResolution, alloc);
    obj.AddMember("sum", static_cast<double>(data.sum) / timerResolution, alloc);

    return obj;
}

rapidjson::Value get_minMaxSumtodo(const uint64_t data, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("min", data, alloc);
    obj.AddMember("max", data, alloc);
    obj.AddMember("sum", data, alloc);

    return obj;
}

rapidjson::Value get_metaData(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {
    const auto& metaData = alldata.metaData;

    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("numberOfLocations", metaData.number_locations, alloc);
    obj.AddMember("runtime", static_cast<double>(metaData.max_time_stamp - metaData.min_time_stamp) / metaData.timerResolution, alloc);
    obj.AddMember("inputFileName", rapidjson::Value(alldata.params.input_file_name.c_str(), alloc), alloc);

    return obj;
}

rapidjson::Value get_definitions(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value definitions(rapidjson::kObjectType);
    rapidjson::Value paradigms(rapidjson::kArrayType);
    for (const auto& paradigm : alldata.definitions.paradigms.get_all()) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("paradigmId", paradigm.first, alloc);
        obj.AddMember("name",  rapidjson::Value(paradigm.second.name.c_str(), alloc), alloc);
        paradigms.PushBack(obj, alloc);
    }
    rapidjson::Value regions(rapidjson::kArrayType);
    for(const auto& region : alldata.definitions.regions.get_all()) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("regionId", region.first, alloc);
        obj.AddMember("name", rapidjson::Value(region.second.name.c_str(), alloc), alloc);
        obj.AddMember("paradigmId", region.second.paradigm_id, alloc);
        obj.AddMember("sourceLine", region.second.source_line, alloc);
        obj.AddMember("fileName", rapidjson::Value(region.second.file_name.c_str(), alloc), alloc);
        regions.PushBack(obj, alloc);
    }
    rapidjson::Value metrics(rapidjson::kArrayType);
    for(const auto& metric : alldata.definitions.metrics.get_all()) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("metricId", metric.first, alloc);
        obj.AddMember("name", rapidjson::Value(metric.second.name.c_str(), alloc), alloc);
        obj.AddMember("description", rapidjson::Value(metric.second.description.c_str(), alloc), alloc);
        obj.AddMember("metricType", static_cast<uint>(metric.second.metricType), alloc);
        obj.AddMember("metricMode", static_cast<uint>(metric.second.metricMode), alloc);
        obj.AddMember("type", static_cast<uint>(metric.second.type), alloc);
        obj.AddMember("base", static_cast<uint>(metric.second.base), alloc);
        obj.AddMember("exponent", metric.second.exponent, alloc);
        obj.AddMember("unit", rapidjson::Value(metric.second.unit.c_str(), alloc), alloc);
        obj.AddMember("allowed", metric.second.allowed, alloc);
        metrics.PushBack(obj, alloc);
    }
    definitions.AddMember("paradigms", paradigms, alloc);
    definitions.AddMember("regions", regions, alloc);
    definitions.AddMember("metrics", metrics, alloc);

    return definitions;
}

rapidjson::Value get_system_tree_node(const std::shared_ptr<definitions::SystemTree::SystemNode> node, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value node_obj(rapidjson::kObjectType);
    node_obj.AddMember("nodeId", node->data.node_id, alloc);
    node_obj.AddMember("name", rapidjson::Value(node->data.name.c_str(), alloc), alloc);
    node_obj.AddMember("classId", static_cast<uint8_t>(node->data.class_id), alloc);
    node_obj.AddMember("locationId", node->data.location_id, alloc);
    node_obj.AddMember("level", node->data.level, alloc);

    rapidjson::Value children(rapidjson::kArrayType);
    for(const auto& child : node->children) {
        children.PushBack(get_system_tree_node(child.second, alloc), alloc);
    }
    node_obj.AddMember("children", children, alloc);

    return node_obj;
}

rapidjson::Value get_system_tree(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value systemTree(rapidjson::kObjectType);
    systemTree.AddMember("systemTreeNodes", get_system_tree_node(alldata.definitions.system_tree.get_root(), alloc), alloc);
    systemTree.AddMember("size", alldata.definitions.system_tree.size(), alloc);
    rapidjson::Value nodesPerLevel(rapidjson::kArrayType);
    for(const auto& num : alldata.definitions.system_tree.all_level()) {
        nodesPerLevel.PushBack(num, alloc);
    }
    systemTree.AddMember("numberOfNodesPerLevel", nodesPerLevel, alloc);

    return systemTree;
}

void create_paradigm_summary(AllData& alldata) {
    rapidjson::Value paradigms(rapidjson::kArrayType);
    for(const auto& region : summary_reg) {
        summary_para[alldata.definitions.regions.get(region.first)->paradigm_id] += region.second;
    }
}

rapidjson::Value get_summaryObject(std::pair<uint64_t, SummaryObject> obj, uint64_t timerResolution, rapidjson::Document::AllocatorType& alloc) {
    using value_t = typename decltype(obj.second.count)::value_t;
    rapidjson::Value summaryObject(rapidjson::kObjectType);
    summaryObject.AddMember("id", obj.first, alloc);
    summaryObject.AddMember("count", get_minMaxSum(obj.second.count, alloc), alloc);
    summaryObject.AddMember("exclusiveTime", get_minMaxSumSeconds(obj.second.excl_time, timerResolution, alloc), alloc);
    summaryObject.AddMember("inclusiveTime", get_minMaxSumSeconds(obj.second.incl_time, timerResolution, alloc), alloc);
    rapidjson::Value send(rapidjson::kObjectType);
    send.AddMember("count", get_minMaxSum(obj.second.count_send, alloc), alloc);
    send.AddMember("bytes", get_minMaxSum(obj.second.bytes_send, alloc), alloc);
    summaryObject.AddMember("send", send, alloc);
    rapidjson::Value recv(rapidjson::kObjectType);
    recv.AddMember("count", get_minMaxSum(obj.second.count_recv, alloc), alloc);
    recv.AddMember("bytes", get_minMaxSum(obj.second.bytes_recv, alloc), alloc);
    summaryObject.AddMember("recv", recv, alloc);

    return summaryObject;
}

rapidjson::Value get_summary(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {
    const auto& metaData = alldata.metaData;

    rapidjson::Value summary(rapidjson::kObjectType);
    rapidjson::Value paradigms(rapidjson::kArrayType);
    create_paradigm_summary(alldata);
    for(const auto& paradigm : summary_para) {
        paradigms.PushBack(get_summaryObject(paradigm, metaData.timerResolution, alloc), alloc);
    }
    summary.AddMember("paradigms", paradigms, alloc);

    rapidjson::Value regions(rapidjson::kArrayType);
    for(const auto& region : summary_reg) {
        regions.PushBack(get_summaryObject(region, alldata.metaData.timerResolution, alloc), alloc);
    }
    summary.AddMember("regions", regions, alloc);

    return summary;
}

rapidjson::Value get_msgObject(uint64_t loc, MData obj, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value msgObject(rapidjson::kObjectType);
    msgObject.AddMember("id", loc, alloc);
    msgObject.AddMember("count", obj.count, alloc);
    msgObject.AddMember("bytes", obj.bytes, alloc);

    return msgObject;
}

rapidjson::Value get_messages(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {

    rapidjson::Value msg(rapidjson::kObjectType);
    rapidjson::Value send(rapidjson::kArrayType);
    for(const auto& loc : alldata.p2p_comm_send) {
        rapidjson::Value loc_send(rapidjson::kArrayType);
        for(const auto& mdata : loc.second) {
            loc_send.PushBack(get_msgObject(mdata.first, mdata.second, alloc), alloc);
        }
        rapidjson::Value tmp(rapidjson::kObjectType);
        tmp.AddMember("id", loc.first, alloc);
        tmp.AddMember("to", loc_send, alloc);
        send.PushBack(tmp, alloc);
    }
    msg.AddMember("send", send, alloc);

    rapidjson::Value recv(rapidjson::kArrayType);
    for(const auto& loc : alldata.p2p_comm_recv) {
        rapidjson::Value loc_recv(rapidjson::kArrayType);
        for(const auto& mdata : loc.second) {
            loc_recv.PushBack(get_msgObject(mdata.first, mdata.second, alloc), alloc);
        }
        rapidjson::Value tmp(rapidjson::kObjectType);
        tmp.AddMember("id", loc.first, alloc);
        tmp.AddMember("from", loc_recv, alloc);
        recv.PushBack(tmp, alloc);
    }
    msg.AddMember("recv", recv, alloc);

    return msg;
}

rapidjson::Value get_regionData(const FunctionDataStats& reg_data, uint64_t timerResolution, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("count", reg_data.count, alloc);
    obj.AddMember("inclusiveTime", get_minMaxSumSeconds(reg_data.incl_time, timerResolution, alloc), alloc);
    obj.AddMember("exclusiveTime", get_minMaxSumSeconds(reg_data.excl_time, timerResolution, alloc), alloc);

    return obj;
}

template<typename T>
rapidjson::Value get_regionCommData(const T& comm_data, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    rapidjson::Value mSent(rapidjson::kObjectType);
    mSent.AddMember("count", comm_data.count_send, alloc);
    mSent.AddMember("bytes", comm_data.bytes_send, alloc);
    obj.AddMember("send", mSent, alloc);
    rapidjson::Value mRecv(rapidjson::kObjectType);
    mRecv.AddMember("count", comm_data.count_recv, alloc);
    mRecv.AddMember("bytes", comm_data.bytes_recv, alloc);
    obj.AddMember("recv", mRecv, alloc);

    return obj;
}

rapidjson::Value get_tree_node(const std::shared_ptr<tree_node>& node, uint64_t timerResolution, rapidjson::Document::AllocatorType& alloc, uint64_t path_depth) {
    rapidjson::Value node_obj(rapidjson::kObjectType);
    rapidjson::Value children(rapidjson::kArrayType);

    node_obj.AddMember("regionId", node->function_id, alloc);

    auto& sum_reg_values = summary_reg[node->function_id];

    path_depth += 1;
    ++size;

    rapidjson::Value locations(rapidjson::kArrayType);
    for(const auto& data : node->node_data) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("locationId", data.first, alloc);
        obj.AddMember("regionData", get_regionData(data.second.f_data, timerResolution, alloc), alloc);
        obj.AddMember("messageData", get_regionCommData(data.second.m_data, alloc), alloc);
        obj.AddMember("collectiveData", get_regionCommData(data.second.c_data, alloc), alloc);

        locations.PushBack(obj, alloc);

        sum_reg_values += data.second.f_data;
        sum_reg_values += data.second.m_data;
    }
    node_obj.AddMember("nodeData", locations, alloc);

    for(const auto& child : node->children) {
        children.PushBack(get_tree_node(child.second, timerResolution, alloc, path_depth), alloc);
    }
    node_obj.AddMember("children", children, alloc);

    depth = std::max(depth, path_depth);

    return node_obj;
}

rapidjson::Value get_data_trees(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {

    rapidjson::Value trees(rapidjson::kArrayType);

    for(const auto& root_node : alldata.call_path_tree.root_nodes) {
        rapidjson::Value root_obj(rapidjson::kObjectType);
        uint64_t path_depth = 0;
        root_obj.AddMember("rootNode", get_tree_node(root_node.second, alldata.metaData.timerResolution, alloc, path_depth), alloc);
        root_obj.AddMember("depth", depth, alloc);
        root_obj.AddMember("size", size, alloc);
        trees.PushBack(root_obj , alloc);
    }

    return trees;
}

bool CreateJson(AllData& alldata) {
    rapidjson::Document document;
    document.SetObject();

    auto& alloc = document.GetAllocator();
    document.AddMember("metaData", get_metaData(alldata, alloc), alloc);
    document.AddMember("definitions", get_definitions(alldata, alloc), alloc);
    document.AddMember("systemTree", get_system_tree(alldata, alloc), alloc);
    document.AddMember("callTrees", get_data_trees(alldata, alloc), alloc);
    document.AddMember("summary", get_summary(alldata, alloc), alloc);
    document.AddMember("messages", get_messages(alldata, alloc), alloc);
    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
    document.Accept(writer);
    std::string fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname);
    outfile << strbuf.GetString() << std::endl;

    for(const auto& loc : alldata.p2p_comm_send) {
        std::cout << "Location: " << loc.first << "\n";
        for(const auto& mdata : loc.second) {
            std::cout << mdata.first << ": " << mdata.second.count << "\n";
        }
    }

    return true;
}
