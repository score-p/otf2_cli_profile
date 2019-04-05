#include "jsonreader.h"
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
// using namespace rapidjson;
void JSONReader::close(){

};

bool JSONReader::initialize(AllData& alldata){
    auto fname = alldata.params.input_file_name;

    FILE* file = fopen(fname.c_str(), "r");

    // TODO
    // determine wether file is open or not
    // if(!file->is_open()){
    //     std::cerr << "Unable to open file";
    //     return false;

    // }
    char readBuffer[65536];
    rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

    this->document.ParseStream(is);
    fclose(file);

    return true;
};

bool JSONReader::readDefinitions(AllData& alldata){

    const rapidjson::Value& regions = document["Definitions"]["regions"];
    assert(regions.IsArray());
    for(rapidjson::SizeType i = 0; i < regions.Size(); ++i){
        definitions::Region region{
            regions[i]["name"].GetString(),
            regions[i]["paradigm_id"].GetUint(),
            regions[i]["source_line"].GetUint(),
            regions[i]["file_name"].GetString()
        };
        alldata.definitions.regions.add(i, region);
    }

    return true;

};

void read_node(rapidjson::Value::ConstMemberIterator node, AllData& alldata, std::shared_ptr<tree_node> parent){

    uint64_t name = std::stoi(node->name.GetString());
    auto tmp_node = std::make_shared<tree_node>(name);

    tmp_node->parent = parent.get();

    // node_data

    for(auto& data : node->value["node_data"].GetArray()){

        uint64_t location_id = std::stoi(data.MemberBegin()->name.GetString());

        NodeData& node_data = tmp_node->node_data[location_id];

        const rapidjson::Value& f_data = data.MemberBegin()->value["f_data"];
        node_data.f_data.count = f_data["count"].GetUint64();
        node_data.f_data.incl_time = f_data["incl_time"].GetUint64();
        node_data.f_data.excl_time = f_data["excl_time"].GetUint64();

        const rapidjson::Value& m_data = data.MemberBegin()->value["m_data"];
        node_data.m_data.count_send = m_data["count_send"].GetUint64();
        node_data.m_data.count_recv = m_data["count_recv"].GetUint64();
        node_data.m_data.bytes_send = m_data["bytes_send"].GetUint64();
        node_data.m_data.bytes_recv = m_data["bytes_recv"].GetUint64();

        const rapidjson::Value& c_data = data.MemberBegin()->value["c_data"];
        node_data.c_data.count_send = c_data["count_send"].GetUint64();
        node_data.c_data.count_recv = c_data["count_recv"].GetUint64();
        node_data.c_data.bytes_send = c_data["bytes_send"].GetUint64();
        node_data.c_data.bytes_recv = c_data["bytes_recv"].GetUint64();
    }
    // TODO have message
    // TODO have collop



    // Children
    const rapidjson::Value& children = node->value["children"];
    for(auto& child : children.GetArray()){
        read_node(child.MemberBegin(), alldata, tmp_node);
    }

    alldata.call_path_tree.insert_node(tmp_node);
}

bool JSONReader::readEvents(AllData& alldata){

    const rapidjson::Value& root_nodes = document["data_tree"]["root_nodes"];

    assert(root_nodes.IsArray());
    for(auto& node : root_nodes.GetArray()){
        assert(node.IsObject());
        // element/node in the array is wrapped in an object
        // they are the only element in the object
        // therefore there is no need to iterate through the object -> object.Memberbegin()
        read_node(node.MemberBegin(), alldata, nullptr);
    }

    return true;

};

// read Metadata
bool JSONReader::readStatistics(AllData& alldata){
    // read communicator
    const rapidjson::Value& comm = document["meta_data"]["communicators"];
    assert(comm.IsArray());

    std::map<uint64_t, uint64_t>& communicators = alldata.metaData.communicators;

    for(rapidjson::SizeType i = 0; i < comm.Size(); ++i){
        uint64_t key = comm[i]["key"].GetUint();
        uint64_t value = comm[i]["value"].GetUint();
        communicators[key] = value;
    }

    // read processIdToName
    const rapidjson::Value& procs = document["meta_data"]["processIdToName"];
    assert(procs.IsArray());

    std::map<uint64_t, std::string>& processId_data = alldata.metaData.processIdToName;

    for(rapidjson::SizeType i = 0; i < procs.Size(); ++i){
        uint64_t processID = procs[i]["processId"].GetUint();
        std::string name = procs[i]["name"].GetString();
        processId_data[processID] = name;
    }

    // read metricIdToName

    const rapidjson::Value& metric = document["meta_data"]["metricIdToName"];
    assert(metric.IsArray());

    std::map<uint64_t, std::string>& metricId_data = alldata.metaData.metricIdToName;

    for(rapidjson::SizeType i = 0; i < metric.Size(); ++i){
        uint64_t metricID = metric[i]["metricId"].GetUint();
        std::string name = metric[i]["name"].GetString();
        metricId_data[metricID] = name;
    }

    // TODO metricclass

    // read rest of meta_data
    alldata.metaData.timerResolution = document["meta_data"]["timerResolution"].GetUint();
    alldata.metaData.myRank = document["meta_data"]["myRank"].GetUint();
    alldata.metaData.numRanks = document["meta_data"]["numRanks"].GetUint();

    return true;

};