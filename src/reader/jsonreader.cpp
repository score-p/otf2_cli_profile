#include "jsonreader.h"
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
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
    const rapidjson::Value& regions = document["definitions"]["regions"];
    assert(regions.IsArray());
    for(rapidjson::SizeType i = 0; i < regions.Size(); ++i){
        uint64_t region_id = regions[i]["regionId"].GetUint64();
        definitions::Region region{
            regions[i]["name"].GetString(),
            regions[i]["paradigmId"].GetUint(),
            regions[i]["sourceLine"].GetUint(),
            regions[i]["fileName"].GetString()
        };
        alldata.definitions.regions.add(region_id, region);
    }

    // parse definitions::metrics
    const rapidjson::Value& metrics = document["definitions"]["metrics"];
    for(const auto& metric : metrics.GetArray()){

        uint64_t metric_id = metric["metricId"].GetUint64();

        definitions::Metric new_metric {
            metric["name"].GetString(),
            metric["description"].GetString(),
            static_cast<MetricType> (metric["metricType"].GetUint()),
            static_cast<MetricMode> (metric["metricMode"].GetUint()),
            static_cast<MetricDataType> (metric["type"].GetUint()),
            static_cast<MetricBase> (metric["base"].GetUint()),
            metric["exponent"].GetInt64(),
            metric["unit"].GetString(),
            metric["allowed"].GetBool()
        };

    alldata.definitions.metrics.add(metric_id, new_metric);
    }

    // parse definitions::metric_classes
    const rapidjson::Value& metric_classes = document["Definitions"]["metricClasses"];
    for(const auto& metric_class : metric_classes.GetArray()){

        uint64_t metric_class_id = metric_class["metricClassId"].GetUint64();

        definitions::Metric_Class new_metric_class {
            static_cast<uint8_t>(metric_class["numOfMetrics"].GetUint()),
            std::map<uint8_t, uint32_t>(),
            static_cast<MetricOccurrence> (metric_class["metricOccurrence"].GetUint()),
            static_cast<RecorderKind> (metric_class["recorderKind"].GetUint())
        };

        for(const auto& member : metric_class["metricMember"].GetArray()){
            uint32_t key    = std::stoul(member.MemberBegin()->name.GetString());
            uint32_t value  = member.MemberBegin()->value.GetUint();
            new_metric_class.metric_member[key] = value;

        }

        alldata.definitions.metric_classes.add(metric_class_id, new_metric_class);
    }

    // parse definitions::paradigms
    const rapidjson::Value& paradigms = document["definitions"]["paradigms"];
    for(const auto& paradigm : paradigms.GetArray()){
        uint32_t paradigm_id = std::stoul(paradigm.MemberBegin()->name.GetString());
        std::string value    = paradigm.MemberBegin()->value.GetString();

        alldata.definitions.paradigms.add(paradigm_id, definitions::Paradigm{value});
    }

    // parse definitions::io_paradigms
    const rapidjson::Value& io_paradigms = document["definitions"]["ioParadigms"];
    for(const auto& io_paradigm : io_paradigms.GetArray()){
        uint32_t paradigm_id = std::stoul(io_paradigm.MemberBegin()->name.GetString());
        std::string value    = io_paradigm.MemberBegin()->value.GetString();

        alldata.definitions.io_paradigms.add(paradigm_id, definitions::Paradigm{value});
    }


    // parse definitions::iohandles

    const rapidjson::Value& iohandles = document["definitions"]["iohandles"];
    for(const auto& iohandle : iohandles.GetArray()){
        uint64_t    iohandle_id    = std::stoll(iohandle.MemberBegin()->name.GetString());
        std::string name           = iohandle.MemberBegin()->value["name"].GetString();
        uint32_t    io_paradigm    = iohandle.MemberBegin()->value["ioParadigm"].GetUint();
        uint64_t    file           = iohandle.MemberBegin()->value["file"].GetUint64();
        uint64_t    parent         = iohandle.MemberBegin()->value["parent"].GetUint64();

        alldata.definitions.iohandles.add(iohandle_id, definitions::IoHandle{name, io_paradigm, file, parent});
    }

    // parse definitions::groups
    const rapidjson::Value& groups = document["definitions"]["groups"];
    for(const auto& group : groups.GetArray()){

        uint32_t group_id = group["groupId"].GetUint64();
        std::vector<uint64_t> members;
        for(const auto& member : group["members"].GetArray()){
            members.push_back(member.GetUint64());
        }

        definitions::Group new_group{
            group["name"].GetString(),
            static_cast<uint8_t>(group["type"].GetUint()),
            group["paradigmId"].GetUint(),
            members
        };

        alldata.definitions.groups.add(group_id, new_group);
    }

    readSystemTree(alldata);

    return true;
}

void read_node(const rapidjson::Value& node, AllData& alldata, std::shared_ptr<tree_node> parent){

    uint64_t function_id = node["regionId"].GetUint64();
    auto tmp_node        = std::make_shared<tree_node>(function_id);
    tmp_node->parent     = parent.get();

    // parse node_data

    for(auto& data : node["nodeData"].GetArray()){

        uint64_t location_id = data["locationId"].GetUint64();
        NodeData& node_data  = tmp_node->node_data[location_id];

        const rapidjson::Value& f_data = data["fData"];
        tmp_node->add_data(
            location_id,
            FunctionData{
                f_data["count"].GetUint64(),
                f_data["inclTime"].GetUint64(),
                f_data["exclTime"].GetUint64(),
            }
        );

        const rapidjson::Value& m_data = data["mData"];
        tmp_node->add_data(
            location_id,
            MessageData{
                m_data["countSend"].GetUint64(),
                m_data["countRecv"].GetUint64(),
                m_data["bytesSend"].GetUint64(),
                m_data["bytesRecv"].GetUint64()
            }
        );

        const rapidjson::Value& c_data = data["cData"];

        tmp_node->add_data(
            location_id,
            CollopData{
                c_data["countSend"].GetUint64(),
                c_data["countRecv"].GetUint64(),
                c_data["bytesSend"].GetUint64(),
                c_data["bytesRecv"].GetUint64()
            }
        );

        const rapidjson::Value& metrics = data["metrics"];
        for(const auto& metric : metrics.GetArray()){
            auto metric_id = std::stoull(metric.MemberBegin()->name.GetString());
            tmp_node->add_data(
                location_id,
                metric_id,
                {
                    static_cast<MetricDataType> (metric.MemberBegin()->value["metricDataType"].GetUint()),
                    {metric.MemberBegin()->value["dataIncl"]["u"].GetUint64()},
                    {metric.MemberBegin()->value["dataExcl"]["u"].GetUint64()}
                }
            );
        }
    }

    // parse children
    const rapidjson::Value& children = node["children"];
    for(auto& child : node["children"].GetArray()){
        read_node(child, alldata, tmp_node);
    }

    tmp_node->has_p2p    = node["hasP2p"].GetBool();
    tmp_node->has_collop = node["hasCollop"].GetBool();


    alldata.call_path_tree.insert_node(tmp_node);
}

bool JsonReader::readEvents(AllData& alldata){

    const rapidjson::Value& root_nodes = document["callTree"]["rootNodes"];

    assert(root_nodes.IsArray());
    for(auto& node : root_nodes.GetArray()){
        read_node(node, alldata, nullptr);

    }
    return true;
}

// parse Metadata
bool JsonReader::readStatistics(AllData& alldata){

    std::map<uint64_t, uint64_t>& communicators = alldata.metaData.communicators;

    const rapidjson::Value& comm = document["metaDataProfiler"]["communicators"];
    assert(comm.IsArray());

    for(const auto& var : comm.GetArray()){
        uint64_t key       = std::stoull(var.MemberBegin()->name.GetString());
        uint64_t value     = var.MemberBegin()->value.GetUint();
        communicators[key] = value;
    }

    // parse processIdToName
    const rapidjson::Value& procs = document["metaDataProfiler"]["processIdToName"];
    assert(procs.IsArray());

    std::map<uint64_t, std::string>& processId_data = alldata.metaData.processIdToName;

    for(rapidjson::SizeType i     = 0; i < procs.Size(); ++i){
        uint64_t processID        = procs[i]["processId"].GetUint();
        std::string name          = procs[i]["name"].GetString();
        processId_data[processID] = name;
    }

    // parse metricIdToName

    const rapidjson::Value& metric = document["metaDataProfiler"]["metricIdToName"];
    assert(metric.IsArray());

    std::map<uint64_t, std::string>& metricId_data = alldata.metaData.metricIdToName;

    for(rapidjson::SizeType i = 0; i < metric.Size(); ++i){
        uint64_t metricID       = metric[i]["metricId"].GetUint();
        std::string name        = metric[i]["name"].GetString();
        metricId_data[metricID] = name;
    }

    // parse metricClassToMetric

    const rapidjson::Value& metricClassToMetric = document["metaDataProfiler"]["metricClassToMetric"];

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
    alldata.metaData.timerResolution = document["meta_data"]["timerResolution"].GetDouble();
    alldata.metaData.myRank          = document["metaDataProfiler"]["myRank"].GetUint();
    alldata.metaData.numRanks        = document["meta_data"]["numRanks"].GetUint();
    alldata.params.input_file_name   = document["meta_data"]["inputFileName"].GetString();
    #ifdef OTFPROFILER_MPI
        alldata.metaData.packBufferSize = document["metaDataProfiler"]["packBufferSize"].GetUint64();
        alldata.metaData.packBuffer     = document["metaDataProfiler"]["packBuffer"].GetUint64();
    #endif
    return true;
}

// parse system_nodes, locationgroups and locations
void readSystemNode(const rapidjson::Value& node, uint64_t parent_id, uint64_t parent_location_id, AllData& alldata){

    std::string name        = node["data"]["name"].GetString();
    uint64_t    node_id     = node["data"]["nodeId"].GetUint64();
    uint64_t    location_id = node["data"]["locationId"].GetUint64();
    uint8_t     class_id    = node["data"]["classId"].GetUint();
    definitions::SystemClass systemclass = static_cast<definitions::SystemClass> (class_id);

    alldata.definitions.system_tree.insert_node(name, node_id, systemclass, parent_id, location_id, parent_location_id);

    for(const auto& child : node["children"].GetArray())
        readSystemNode(child, node_id, location_id, alldata);
}

bool JsonReader::readSystemTree(AllData& alldata){
    rapidjson::Value& node = document["systemTree"]["systemNodes"];
    uint64_t parent_id     = node["parent"].GetUint64();
    uint64_t location_id   = node["data"]["locationId"].GetUint64();

    readSystemNode(node, parent_id, 0, alldata);
    return true;
}
