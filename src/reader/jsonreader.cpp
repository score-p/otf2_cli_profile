#include "jsonreader.h"
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
// using namespace rapidjson;
void JsonReader::close(){
}

bool JsonReader::initialize(AllData& alldata){
    auto fname = alldata.params.input_file_name;

    FILE* file = fopen(fname.c_str(), "r"); // r - read

    char readBuffer[65536];
    rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

    this->document.ParseStream(is);
    fclose(file);

    return true;
}

bool JsonReader::readDefinitions(AllData& alldata){

    // parse definitions::regions
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

    // parse definitions::metrics
    const rapidjson::Value& metrics = document["Definitions"]["metrics"];
    for(const auto& metric : metrics.GetArray()){

        uint location = std::stoul(metric.MemberBegin()->name.GetString());
        const rapidjson::Value& value = metric.MemberBegin()->value;
        definitions::Metric new_metric {
            value["name"].GetString(),
            value["description"].GetString(),
            static_cast<MetricType> (value["metricType"].GetUint()),
            static_cast<MetricMode> (value["metricMode"].GetUint()),
            static_cast<MetricDataType> (value["type"].GetUint()),
            static_cast<MetricBase> (value["base"].GetUint()),
            value["exponent"].GetInt64(),
            value["unit"].GetString(),
            value["allowed"].GetBool()
        };

    alldata.definitions.metrics.add(location, new_metric);
    }

    // parse definitions::metric_classes
    const rapidjson::Value& metric_classes = document["Definitions"]["metric_classes"];
    for(const auto& metric_class : metric_classes.GetArray()){

        uint location = std::stoul(metric_class.MemberBegin()->name.GetString());
        const rapidjson::Value& class_data = metric_class.MemberBegin()->value;

        definitions::Metric_Class new_metric_class {
            static_cast<uint8_t>(class_data["num_of_metrics"].GetUint()),
            std::map<uint8_t, uint32_t>(),
            static_cast<MetricOccurrence> (class_data["metric_occurrence"].GetUint()),
            static_cast<RecorderKind> (class_data["recorder_kind"].GetUint())
        };

        for(const auto& member : class_data["metric_member"].GetArray()){
            uint32_t key    = std::stoul(member.MemberBegin()->name.GetString());
            uint32_t value  = member.MemberBegin()->value.GetUint();
            new_metric_class.metric_member[key] = value;

        }

        alldata.definitions.metric_classes.add(location, new_metric_class);
    }

    // parse definitions::paradigms
    const rapidjson::Value& paradigms = document["Definitions"]["paradigms"];
    for(const auto& paradigm : paradigms.GetArray()){
        uint32_t paradigm_id = std::stoul(paradigm.MemberBegin()->name.GetString());
        std::string value    = paradigm.MemberBegin()->value.GetString();

        alldata.definitions.paradigms.add(paradigm_id, definitions::Paradigm{value});
    }

    // parse definitions::io_paradigms
    const rapidjson::Value& io_paradigms = document["Definitions"]["io_paradigms"];
    for(const auto& io_paradigm : io_paradigms.GetArray()){
        uint32_t paradigm_id = std::stoul(io_paradigm.MemberBegin()->name.GetString());
        std::string value    = io_paradigm.MemberBegin()->value.GetString();

        alldata.definitions.io_paradigms.add(paradigm_id, definitions::Paradigm{value});
    }


    // parse definitions::iohandles

    const rapidjson::Value& iohandles = document["Definitions"]["iohandles"];
    for(const auto& iohandle : iohandles.GetArray()){
        uint64_t    iohandle_id = std::stoll(iohandle.MemberBegin()->name.GetString());
        std::string name           = iohandle.MemberBegin()->value["name"].GetString();
        uint32_t    io_paradigm    = iohandle.MemberBegin()->value["io_paradigm"].GetUint();
        uint64_t    file           = iohandle.MemberBegin()->value["file"].GetUint64();
        uint64_t    parent         = iohandle.MemberBegin()->value["parent"].GetUint64();

        alldata.definitions.iohandles.add(iohandle_id, definitions::IoHandle{name, io_paradigm, file, parent});
    }

    // parse definitions::groups
    const rapidjson::Value& groups = document["Definitions"]["groups"];
    for(const auto& group : groups.GetArray()){

        uint32_t location = std::stoul(group.MemberBegin()->name.GetString());

        const rapidjson::Value& group_data = group.MemberBegin()->value;
        std::vector<uint64_t> members;
        for(const auto& member : group_data["members"].GetArray()){
            members.push_back(member.GetUint64());
        }

        definitions::Group new_group{
            group_data["name"].GetString(),
            static_cast<uint8_t>(group_data["type"].GetUint()),
            group_data["paradigm_id"].GetUint(),
            members
        };

        alldata.definitions.groups.add(location, new_group);
    }

    readSystemTree(alldata);

    return true;
}

void read_node(rapidjson::Value::ConstMemberIterator node, AllData& alldata, std::shared_ptr<tree_node> parent){

    uint64_t functionId = std::stoull(node->name.GetString());
    auto tmp_node = std::make_shared<tree_node>(functionId);

    tmp_node->parent = parent.get();

    // parse node_data

    for(auto& data : node->value["node_data"].GetArray()){

        uint64_t location_id = std::stoull(data.MemberBegin()->name.GetString());
        NodeData& node_data  = tmp_node->node_data[location_id];

        const rapidjson::Value& f_data = data.MemberBegin()->value["f_data"];
        tmp_node->add_data(
            location_id,
            FunctionData{
                f_data["count"].GetUint64(),
                f_data["incl_time"].GetUint64(),
                f_data["excl_time"].GetUint64(),
            }
        );

        const rapidjson::Value& m_data = data.MemberBegin()->value["m_data"];
        tmp_node->add_data(
            location_id,
            MessageData{
                m_data["count_send"].GetUint64(),
                m_data["count_recv"].GetUint64(),
                m_data["bytes_send"].GetUint64(),
                m_data["bytes_recv"].GetUint64()
            }
        );

        const rapidjson::Value& c_data = data.MemberBegin()->value["c_data"];

        tmp_node->add_data(
            location_id,
            CollopData{
                c_data["count_send"].GetUint64(),
                c_data["count_recv"].GetUint64(),
                c_data["bytes_send"].GetUint64(),
                c_data["bytes_recv"].GetUint64()
            }
        );

        const rapidjson::Value& metrics = data.MemberBegin()->value["metrics"];
        for(const auto& metric : metrics.GetArray()){
            auto metric_id = std::stoull(metric.MemberBegin()->name.GetString());
            tmp_node->add_data(
                location_id,
                metric_id,
                {
                    static_cast<MetricDataType> (metric.MemberBegin()->value["MetricDataType"].GetUint()),
                    {metric.MemberBegin()->value["data_incl"]["u"].GetUint64()},
                    {metric.MemberBegin()->value["data_excl"]["u"].GetUint64()}
                }
            );
        }
    }


    // Children
    const rapidjson::Value& children = node->value["children"];
    for(auto& child : children.GetArray()){
        read_node(child.MemberBegin(), alldata, tmp_node);
    }

    tmp_node->has_p2p    = node->value["has_p2p"].GetBool();
    tmp_node->has_collop = node->value["has_collop"].GetBool();


    alldata.call_path_tree.insert_node(tmp_node);
}

bool JsonReader::readEvents(AllData& alldata){

    const rapidjson::Value& root_nodes = document["data_tree"]["root_nodes"];

    assert(root_nodes.IsArray());
    for(auto& node : root_nodes.GetArray()){
        assert(node.IsObject());
        read_node(node.MemberBegin(), alldata, nullptr);
    }
    return true;
}

// parse Metadata
bool JsonReader::readStatistics(AllData& alldata){

    std::map<uint64_t, uint64_t>& communicators = alldata.metaData.communicators;

    const rapidjson::Value& comm = document["meta_data"]["communicators"];
    assert(comm.IsArray());

    for(const auto& var : comm.GetArray()){
        uint64_t key       = std::stoull(var.MemberBegin()->name.GetString());
        uint64_t value     = var.MemberBegin()->value.GetUint();
        communicators[key] = value;
    }

    // parse processIdToName
    const rapidjson::Value& procs = document["meta_data"]["processIdToName"];
    assert(procs.IsArray());

    std::map<uint64_t, std::string>& processId_data = alldata.metaData.processIdToName;

    for(rapidjson::SizeType i     = 0; i < procs.Size(); ++i){
        uint64_t processID        = procs[i]["processId"].GetUint();
        std::string name          = procs[i]["name"].GetString();
        processId_data[processID] = name;
    }

    // parse metricIdToName

    const rapidjson::Value& metric = document["meta_data"]["metricIdToName"];
    assert(metric.IsArray());

    std::map<uint64_t, std::string>& metricId_data = alldata.metaData.metricIdToName;

    for(rapidjson::SizeType i = 0; i < metric.Size(); ++i){
        uint64_t metricID       = metric[i]["metricId"].GetUint();
        std::string name        = metric[i]["name"].GetString();
        metricId_data[metricID] = name;
    }

    // parse metricClassToMetric

    const rapidjson::Value& metricClassToMetric = document["meta_data"]["metricClassToMetric"];

    for(rapidjson::SizeType i = 0; i < metricClassToMetric.Size(); ++i){
        uint64_t metricClassId   = metricClassToMetric[i]["metricClassId"].GetUint64();

        const rapidjson::Value& metricMembers = metricMembers;
        for (rapidjson::SizeType j = 0; j < metricMembers.Size(); ++j) {
            uint64_t numberOfMetrics = metricMembers[j]["numberOfMetrics"].GetUint64();
            uint64_t metricMember    = metricMembers[j]["metricMember"].GetUint64();
            alldata.metaData.metricClassToMetric[metricClassId][numberOfMetrics] = metricMember;
        }
    }


    // parse rest of meta_data
    alldata.metaData.timerResolution = document["meta_data"]["timerResolution"].GetUint();
    alldata.metaData.myRank   = document["meta_data"]["myRank"].GetUint();
    alldata.metaData.numRanks = document["meta_data"]["numRanks"].GetUint();
    #ifdef OTFPROFILER_MPI
        alldata.metaData.packBufferSize = document["meta_data"]["packBufferSize"].GetUint64();
        alldata.metaData.Buffer         = document["meta_data"]["packBuffer"].GetUint64();
    #endif
    return true;
}

// parse system_nodes, locationgroups and locations
void readSystemNode(const rapidjson::Value& node, uint64_t parent_id, uint64_t parent_location_id, AllData& alldata){

    std::string name        = node["data"]["name"].GetString();
    uint64_t    node_id     = node["data"]["node_id"].GetUint64();
    uint64_t    location_id = node["data"]["location_id"].GetUint64();
    uint8_t     class_id    = node["data"]["class_id"].GetUint();
    definitions::SystemClass systemclass = static_cast<definitions::SystemClass> (class_id);

    alldata.definitions.system_tree.insert_node(name, node_id, systemclass, parent_id, location_id, parent_location_id);

    for(const auto& child : node["children"].GetArray())
        readSystemNode(child, node_id, location_id, alldata);
}

bool JsonReader::readSystemTree(AllData& alldata){
    rapidjson::Value& node = document["system_tree"]["system_nodes"];
    uint64_t parent_id     = node["parent"].GetUint64();
    uint64_t location_id   = node["data"]["location_id"].GetUint64();

    readSystemNode(node, parent_id, 0, alldata);
}