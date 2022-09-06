/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal, Bill Williams
*/
#include <fstream>
#include <map>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "all_data.h"

struct SummaryObject {
    MinMaxSum<uint64_t> count;
    MinMaxSum<uint64_t> excl_time;
    MinMaxSum<uint64_t> incl_time;

    SummaryObject& operator+=(const FunctionDataStats& rhs) {
        addData(rhs);

        return *this;
    }

    SummaryObject& operator+=(const SummaryObject& rhs) {
        addData(rhs);

        return *this;
    }

private:
    template<typename T>
    void addData(const T& rhs) {
        count += rhs.count;
        incl_time += rhs.incl_time;
        excl_time += rhs.excl_time;
    }
};

std::map<uint64_t, SummaryObject> summary_para;
std::map<uint64_t, SummaryObject> summary_reg;

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

rapidjson::Value get_metaData(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("numberOfLocations", 200, alloc);
    obj.AddMember("inputFileName", rapidjson::Value(alldata.params.input_file_name.c_str(), alloc), alloc);
    obj.AddMember("runtime", 10, alloc);

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
        obj.AddMember("regionID", region.first, alloc);
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

template<typename ResT>
rapidjson::Value get_summaryObject(std::pair<uint64_t, SummaryObject> obj,rapidjson::Document::AllocatorType& alloc, ResT timerResolution) {
    using value_t = typename decltype(obj.second.count)::value_t;
    rapidjson::Value summaryObject(rapidjson::kObjectType);
    summaryObject.AddMember("id", obj.first, alloc);
    summaryObject.AddMember("count", get_minMaxSum(obj.second.count, alloc), alloc);
    summaryObject.AddMember("exclusiveTime", get_minMaxSumSeconds(obj.second.excl_time, timerResolution, alloc), alloc);
    summaryObject.AddMember("inclusiveTime", get_minMaxSumSeconds(obj.second.incl_time, timerResolution, alloc), alloc);

    return summaryObject;
}

rapidjson::Value get_summary(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value summary(rapidjson::kObjectType);
    summary.AddMember("runtime", 10, alloc);

    rapidjson::Value paradigms(rapidjson::kArrayType);
    create_paradigm_summary(alldata);
    for(const auto& paradigm : summary_para) {
        paradigms.PushBack(get_summaryObject(paradigm, alloc, alldata.metaData.timerResolution), alloc);
    }
    summary.AddMember("paradigms", paradigms, alloc);

    rapidjson::Value regions(rapidjson::kArrayType);
    for(const auto& region : summary_reg) {
        regions.PushBack(get_summaryObject(region, alloc, alldata.metaData.timerResolution), alloc);
    }
    summary.AddMember("regions", regions, alloc);

    return summary;
}

rapidjson::Value get_regionData(const FunctionDataStats& reg_data, uint64_t timerResolution, rapidjson::Document::AllocatorType& alloc) {
    using value_t = decltype(reg_data.count);
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("count", reg_data.count, alloc);
    obj.AddMember("inclusiveTime", get_minMaxSumSeconds(reg_data.incl_time, timerResolution, alloc), alloc);
    obj.AddMember("exclusiveTime", get_minMaxSumSeconds(reg_data.excl_time, timerResolution, alloc), alloc);

    return obj;
}

rapidjson::Value get_tree_node(const std::shared_ptr<tree_node>& node, uint64_t timerResolution, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value node_obj(rapidjson::kObjectType);
    rapidjson::Value children(rapidjson::kArrayType);

    node_obj.AddMember("regionId", node->function_id, alloc);

    auto& sum_reg_values = summary_reg[node->function_id];

    rapidjson::Value locations(rapidjson::kArrayType);
    for(const auto& data : node->node_data) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("locationId", data.first, alloc);
        obj.AddMember("regionData", get_regionData(data.second.f_data, timerResolution, alloc), alloc);

        locations.PushBack(obj, alloc);

        sum_reg_values += data.second.f_data;
    }
    node_obj.AddMember("nodeData", locations, alloc);


    for(const auto& child : node->children) {
        children.PushBack(get_tree_node(child.second, timerResolution, alloc), alloc);
    }
    node_obj.AddMember("children", children, alloc);

    return node_obj;
}

rapidjson::Value get_data_trees(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {

    rapidjson::Value trees(rapidjson::kArrayType);

    for(const auto& root_node : alldata.call_path_tree.root_nodes) {
        rapidjson::Value root_obj(rapidjson::kObjectType);
        root_obj.AddMember("rootNode", get_tree_node(root_node.second, alldata.metaData.timerResolution, alloc), alloc);
        root_obj.AddMember("depth", 1, alloc);
        root_obj.AddMember("size", 1, alloc);
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
    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
    document.Accept(writer);
    std::string fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname);
    outfile << strbuf.GetString() << std::endl;

    return true;
}
