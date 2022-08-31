#include <fstream>
#include <iostream>
#include <string>
#include "all_data.h"
#include "data_out.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "definitions.h"
#include "main_structs.h"

bool DataOut(AllData& alldata){


    if (alldata.metaData.myRank != 0)
        return true;

    // (vlevel, bool master_only, msg)
    alldata.verbosePrint(1, true, "producing json output");

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    display(writer, alldata);
    std::string fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname);
    outfile << buffer.GetString() << std::endl;
    //std::cout << buffer.GetString() << std::endl;

    return true;
}

template <typename Writer>
void display(Writer& writer, AllData alldata){
    writer.StartObject();
        display_meta_data(alldata, writer);
        display_meta_data_profiler(alldata, writer);
        display_definitions(alldata, writer);
        display_params(alldata, writer);
        display_system_tree(alldata, writer);
        display_data_tree(alldata, writer);
    writer.EndObject();
}

template <typename Writer>
void display_data_tree(AllData alldata, Writer& writer){
    writer.String("callTree");
    writer.StartObject();
        writer.Key("rootNodes");
        writer.StartArray();
            for(const auto& node : alldata.call_path_tree.root_nodes){
                display_node(node.second, writer);
            }
        writer.EndArray();
    writer.EndObject();
}

template <typename Writer>
void display_node(std::shared_ptr<tree_node> node, Writer& writer){
    writer.StartObject();
        writer.Key("regionId");
        writer.Uint64(node->function_id);

        writer.Key("parent");
        if(node->parent != NULL)
            writer.Uint64(node->parent->function_id);
        else
            writer.Null();

        writer.Key("nodeData");
        writer.StartArray();
            for(const auto& data : node->node_data){
                writer.StartObject();
                    writer.Key("locationId");
                    writer.Uint64(data.first);
                    writer.Key("fData");
                    writer.StartObject();
                        writer.Key("count");
                        writer.Uint64(data.second.f_data.count);
                        writer.Key("inclTime");
                        writer.Uint64(data.second.f_data.incl_time);
                        writer.Key("exclTime");
                        writer.Uint64(data.second.f_data.excl_time);
                    writer.EndObject();
                    writer.Key("mData");
                    writer.StartObject();
                        writer.Key("countSend");
                        writer.Uint64(data.second.m_data.count_send);
                        writer.Key("countRecv");
                        writer.Uint64(data.second.m_data.count_recv);
                        writer.Key("bytesSend");
                        writer.Uint64(data.second.m_data.bytes_send);
                        writer.Key("bytesRecv");
                        writer.Uint64(data.second.m_data.bytes_recv);
                    writer.EndObject();
                    writer.Key("cData");
                    writer.StartObject();
                        writer.Key("countSend");
                        writer.Uint64(data.second.c_data.count_send);
                        writer.Key("countRecv");
                        writer.Uint64(data.second.c_data.count_recv);
                        writer.Key("bytesSend");
                        writer.Uint64(data.second.c_data.bytes_send);
                        writer.Key("bytesRecv");
                        writer.Uint64(data.second.c_data.bytes_recv);
                    writer.EndObject();
                    writer.Key("metrics");
                    writer.StartArray();
                        for(const auto& metricData : data.second.metrics){
                            writer.StartObject();
                                writer.Key(std::to_string(metricData.first).c_str());
                                writer.StartObject();
                                    writer.Key("metricDataType");
                                    writer.Uint(static_cast<uint8_t>(metricData.second.type));
                                    writer.Key("dataIncl");
                                    writer.StartObject();
                                        writer.Key("u");
                                        writer.Uint64(metricData.second.data_incl.u);
                                        writer.Key("s");
                                        writer.Int64(metricData.second.data_incl.s);
                                        writer.Key("d");
                                        writer.Double(metricData.second.data_incl.d);
                                    writer.EndObject();
                                    writer.Key("dataExcl");
                                    writer.StartObject();
                                        writer.Key("u");
                                        writer.Uint64(metricData.second.data_excl.u);
                                        writer.Key("s");
                                        writer.Int64(metricData.second.data_excl.s);
                                        writer.Key("d");
                                        writer.Double(metricData.second.data_excl.d);
                                    writer.EndObject();
                                writer.EndObject();
                            writer.EndObject();
                        }
                    writer.EndArray();
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("children");
        writer.StartArray();
            for(const auto& child : node->children){
                display_node(child.second, writer);
            }
        writer.EndArray();

        writer.Key("hasP2p");
        writer.Bool(node->has_p2p);

        writer.Key("hasCollop");
        writer.Bool(node->has_collop);

        writer.EndObject();
}

template <typename Writer>
void display_definitions(AllData alldata, Writer& writer){
    writer.Key("definitions");
    writer.StartObject();
        writer.Key("paradigms");
        writer.StartArray();
            for(const auto& paradigm : alldata.definitions.paradigms.get_all()){
                writer.StartObject();
                    writer.Key(std::to_string(paradigm.first).c_str());
                    writer.String(paradigm.second.name.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("regions");
        writer.StartArray();
            for(const auto& region : alldata.definitions.regions.get_all()){
                writer.StartObject();
                    writer.Key("regionId");
                    writer.Uint(region.first);
                    writer.Key("name");
                    writer.String(region.second.name.c_str());
                    writer.Key("paradigmId");
                    writer.Uint(region.second.paradigm_id);
                    writer.Key("sourceLine");
                    writer.Uint(region.second.source_line);
                    writer.Key("fileName");
                    writer.String(region.second.file_name.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metrics");
        writer.StartArray();
            for(const auto& it : alldata.definitions.metrics.get_all()){
                const definitions::Metric& metric = it.second;
                writer.StartObject();
                        writer.Key("metricId");
                        writer.Uint64(it.first);
                        writer.Key("name");
                        writer.String(metric.name.c_str());
                        writer.Key("description");
                        writer.String(metric.description.c_str());
                        writer.Key("metricType");
                        writer.Uint(static_cast<uint> (metric.metricType));
                        writer.Key("metricMode");
                        writer.Uint(static_cast<uint> (metric.metricMode));
                        writer.Key("type");
                        writer.Uint(static_cast<uint> (metric.type));
                        writer.Key("base");
                        writer.Uint(static_cast<uint> (metric.base));
                        writer.Key("exponent");
                        writer.Int64(metric.exponent);
                        writer.Key("unit");
                        writer.String(metric.unit.c_str());
                        writer.Key("allowed");
                        writer.Bool(metric.allowed);
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metricClasses");
        writer.StartArray();
            for(const auto& it : alldata.definitions.metric_classes.get_all()){
                const definitions::Metric_Class& metric_class = it.second;

                writer.StartObject();
                        writer.Key("metricClassId");
                        writer.Uint64(it.first);
                        writer.Key("numOfMetrics");
                        writer.Uint(metric_class.num_of_metrics);
                        writer.Key("metricMember");
                        writer.StartArray();
                            for(const auto& member : metric_class.metric_member){
                                writer.StartObject();
                                    writer.Key(std::to_string(member.first).c_str());
                                    writer.Uint(member.second);
                                writer.EndObject();
                            }
                        writer.EndArray();
                        writer.Key("metricOccurrence");
                        writer.Uint(static_cast<uint> (metric_class.metric_occurrence));
                        writer.Key("recorderKind");
                        writer.Uint(static_cast<uint> (metric_class.recorder_kind));
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("ioParadigms");
        writer.StartArray();
            for(const auto& io_paradigm : alldata.definitions.io_paradigms.get_all()){
                writer.StartObject();
                    writer.Key(std::to_string(io_paradigm.first).c_str());
                    writer.String(io_paradigm.second.name.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("iohandles");
        writer.StartArray();
            for(const auto& iohandle : alldata.definitions.iohandles.get_all()){
                writer.StartObject();
                    writer.Key(std::to_string(iohandle.first).c_str());
                    writer.StartObject();
                        writer.Key("name");
                        writer.String(iohandle.second.name.c_str());
                        writer.Key("ioParadigm");
                        writer.Uint(iohandle.second.io_paradigm);
                        writer.Key("file");
                        writer.Uint64(iohandle.second.file);
                        writer.Key("parent");
                        writer.Uint64(iohandle.second.parent);
                    writer.EndObject();
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("groups");
        writer.StartArray();
            for(const auto& it : alldata.definitions.groups.get_all()){
                const definitions::Group& group = it.second;
                writer.StartObject();
                    writer.Key("groupId");
                    writer.Uint64(it.first);
                    writer.Key("name");
                    writer.String(group.name.c_str());
                    writer.Key("type");
                    writer.Uint(group.type);
                    writer.Key("paradigmId");
                    writer.Uint(group.paradigm_id);
                    writer.Key("members");
                    writer.StartArray();
                        for(const auto& member : group.members)
                            writer.Uint64(member);
                    writer.EndArray();
                writer.EndObject();
            }
        writer.EndArray();


    writer.EndObject();
}

template <typename Writer>
void display_meta_data(AllData alldata, Writer& writer){
    writer.Key("metaData");
    writer.StartObject();
        writer.Key("timerResolution");
        writer.Double(alldata.metaData.timerResolution);

        writer.Key("numRanks");
        writer.Uint64(alldata.metaData.numRanks);

        writer.Key("inputFileName");
        writer.String(alldata.params.input_file_name.c_str());
        
    writer.EndObject();
}

template <typename Writer>
void display_meta_data_profiler(AllData alldata, Writer& writer){
    writer.Key("metaDataProfiler");
    writer.StartObject();
    writer.Key("communicators");
        writer.StartArray();
            for(const auto& comm : alldata.metaData.communicators){
                writer.StartObject();
                writer.Key(std::to_string(comm.first).c_str());
                writer.Uint64(comm.second);
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("processIdToName");
        writer.StartArray();
            for(const auto& procs : alldata.metaData.processIdToName){
                writer.StartObject();
                    writer.Key(std::to_string(procs.first).c_str());
                    writer.String(procs.second.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metricIdToName");
        writer.StartArray();
            for(const auto& metric : alldata.metaData.metricIdToName){
                writer.StartObject();
                    writer.Key(std::to_string(metric.first).c_str());
                    writer.String(metric.second.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metricClassToMetric");
        writer.StartArray();
            for(const auto& metricClassId : alldata.metaData.metricClassToMetric){
                writer.StartObject();
                    writer.Key("metricClassId");
                    writer.Uint64(metricClassId.first);
                    writer.StartArray();
                        for(const auto& metricMembers : metricClassId.second){
                            writer.Key("numberOfMetrics");
                            writer.Uint64(metricMembers.first);
                            writer.Key("metricMember");
                            writer.Uint64(metricMembers.second);
                        }
                    writer.EndArray();
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("myRank");
        writer.Uint64(alldata.metaData.myRank);

        #ifdef OTFPROFILER_MPI
            writer.Key("packBufferSize");
            writer.Uint64(alldata.metaData.packBufferSize);
            writer.Key("packBuffer");
            writer.String(alldata.metaData.packBuffer);
        #endif
    
    writer.EndObject();
}

template <typename Writer>
void display_params(AllData alldata, Writer& writer){
    writer.Key("Params");
    writer.StartObject();
        writer.Key("maxFileHandles");
        writer.Uint(alldata.params.max_file_handles);
        writer.Key("bufferSize");
        writer.Uint(alldata.params.buffer_size);
        writer.Key("verboseLevel");
        writer.Uint(alldata.params.verbose_level);
        writer.Key("readMetrics");
        writer.Bool(alldata.params.read_metrics);
        writer.Key("outputTypeSet");
        writer.Bool(alldata.params.output_type_set);
        writer.Key("createCube");
        writer.Bool(alldata.params.create_cube);
        writer.Key("createJson");
        writer.Bool(alldata.params.create_json);
        writer.Key("dataDump");
        writer.Bool(alldata.params.data_dump);
        writer.Key("summarizeIt");
        writer.Bool(alldata.params.summarize_it);
        writer.Key("inputFileName");
        writer.String(alldata.params.input_file_name.c_str());
        writer.Key("inputFilePrefix");
        writer.String(alldata.params.input_file_prefix.c_str());
        writer.Key("outputFilePrefix");
        writer.String(alldata.params.output_file_prefix.c_str());
    writer.EndObject();
}

template <typename Writer>
void display_system_tree(AllData alldata, Writer& writer){
    writer.Key("systemTree");
    writer.StartObject();
        writer.Key("systemNodes");
        display_system_node(alldata.definitions.system_tree.get_root(), alldata, writer);
        writer.Key("size");
        writer.Uint(alldata.definitions.system_tree.size());
        writer.Key("numNodesPerLevel");
        writer.StartArray();
            for(const auto& num : alldata.definitions.system_tree.all_level())
                writer.Uint(num);
        writer.EndArray();



    writer.EndObject();
}

template <typename Writer>
void display_system_node(std::shared_ptr<definitions::SystemTree::SystemNode> node, AllData alldata, Writer& writer){
    writer.StartObject();
        writer.Key("parent");
        if(node->parent == nullptr)
            writer.Uint(static_cast<uint32_t>(-1));
        else
            writer.Uint(node->parent->data.node_id);

        writer.Key("data");
        writer.StartObject();
            writer.Key("nodeId");
            writer.Uint(node->data.node_id);
            writer.Key("name");
            writer.String(node->data.name.c_str());
            writer.Key("classId");
            writer.Uint(static_cast<uint8_t> (node->data.class_id));
            writer.Key("locationId");
            writer.Uint64(node->data.location_id);
            writer.Key("level");
            writer.Uint(node->data.level);
        writer.EndObject();

        writer.Key("children");
        writer.StartArray();
            for(const auto& child : node->children)
                display_system_node(child.second, alldata, writer);
        writer.EndArray();
    writer.EndObject();
}
