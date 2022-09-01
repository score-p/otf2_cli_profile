/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal, Bill Williams
*/
#include <fstream>
#include "all_data.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

template<typename T>
rapidjson::Value get_minMaxSum(const MinMaxSum<T>& data, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("min", data.min, alloc);
    obj.AddMember("max", data.max, alloc);
    obj.AddMember("sum", data.sum, alloc);

    return obj;
}

auto get_regionData(const FunctionDataStats& reg_data, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("count", reg_data.count, alloc);
    obj.AddMember("inclusiveTime", get_minMaxSum(reg_data.incl_time, alloc), alloc);
    obj.AddMember("exclusiveTime", get_minMaxSum(reg_data.excl_time, alloc), alloc);
    //obj.AddMember("inclusiveTime", get_minMaxSum(obj, alloc), alloc);
    //obj.AddMember("exclusiveTime", get_minMaxSum(obj, alloc), alloc);

    return obj;
}


rapidjson::Value get_tree_node(const std::shared_ptr<tree_node>& node, rapidjson::Document::AllocatorType& alloc) {
    rapidjson::Value node_obj(rapidjson::kObjectType);
    rapidjson::Value children(rapidjson::kArrayType);

    node_obj.AddMember("regionId", node->function_id, alloc);

    rapidjson::Value locations(rapidjson::kArrayType);
    for(const auto& data : node->node_data) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("locationId", data.first, alloc);
        obj.AddMember("regionData", get_regionData(data.second.f_data, alloc), alloc);

        locations.PushBack(obj, alloc);
    }
    node_obj.AddMember("nodeData", locations, alloc);


    for(const auto& child : node->children) {
        rapidjson::Value obj(rapidjson::kObjectType);

        children.PushBack(get_tree_node(child.second, alloc), alloc);
    }
    node_obj.AddMember("children", children, alloc);

    return node_obj;
}

rapidjson::Value get_data_trees(AllData& alldata, rapidjson::Document::AllocatorType& alloc) {

    rapidjson::Value trees(rapidjson::kArrayType);

    for(const auto& root_node : alldata.call_path_tree.root_nodes) {
        rapidjson::Value root_obj(rapidjson::kObjectType);
        root_obj.AddMember("rootNode", get_tree_node(root_node.second, alloc), alloc);
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
    document.AddMember("callTrees", get_data_trees(alldata, alloc), alloc);
    //document.AddMember("callTrees", 1, alloc);
    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
    document.Accept(writer);
    std::string fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname);
    outfile << strbuf.GetString() << std::endl;

    return true;
}
