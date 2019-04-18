
#include <fstream>
#include <iostream>
#include <string>
#include "data_out.h"
#include "all_data.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "definitions.h"
#include "main_structs.h"


static Data_map read_data(AllData& alldata){

    Data_map data;

    double incl_time = 0;
    double excl_time = 0;

    double timerResolution = (double)alldata.metaData.timerResolution;

    // move through data tree, through all regions...
    for (auto it_node = alldata.call_path_tree.begin();
        it_node != alldata.call_path_tree.end(); ++it_node ) {

        //and location
        for ( auto it_location : it_node->node_data){

            Data& stat = data[it_node->function_id][it_location.first];

            // region
            uint64_t region = it_node->function_id;
            stat.region = alldata.definitions.regions.get(region)->name;

            // location
            stat.location = it_location.first;

            // invocations
            stat.count += it_location.second.f_data.count;

            incl_time = (double)it_location.second.f_data.incl_time / timerResolution;
            excl_time = (double)it_location.second.f_data.excl_time / timerResolution;

            // min incl time
            if( stat.min_incl_time > incl_time )
                stat.min_incl_time = incl_time;

            // min excl time
            if( stat.min_excl_time > excl_time )
                stat.min_excl_time = excl_time;

            // max incl time
            if( stat.max_incl_time < incl_time )
                stat.max_incl_time = incl_time;

            // max excl time
            if( stat.max_excl_time < excl_time )
                stat.max_excl_time = excl_time;

            stat.sum_incl_time += incl_time;
            stat.sum_excl_time += excl_time;
        }
    }

    return data;
}

bool DataOut(AllData& alldata){


    if (alldata.metaData.myRank != 0)
        return true;

    // (vlevel, bool master_only, msg)
    alldata.verbosePrint(1, true, "producing json output");

    Data_map data =  read_data(alldata);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    display(writer, data, alldata);
    std::string fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname);
    outfile << buffer.GetString() << std::endl;
    std::cout << buffer.GetString() << std::endl;

    return true;
}


template <typename Writer>
void display(Writer& writer, Data_map data, AllData alldata){

    writer.StartObject();
        // display_regions(data, writer); // to be removed
        display_data_tree(alldata, writer);
        display_meta_data(alldata, writer);
        display_definitions(alldata, writer);
        display_params(alldata, writer);
        display_system_tree(alldata, writer);
    writer.EndObject();
}

template <typename Writer>
void display_regions(Data_map data, Writer& writer){
    writer.Key("Regions");
    writer.StartArray();
    for(const auto& region : data){
        writer.StartObject();
        // regionname
        writer.Key(region.second.begin()->second.region.c_str());
        writer.StartObject();
        writer.Key("Locations");
        writer.StartArray();
        for(const auto& location : region.second){
            writer.StartObject();   //location
            writer.Key("location");
            writer.Uint64(location.first);
            writer.Key("invocations");
            writer.Uint(location.second.count);
            writer.Key("min_incl_time");
            writer.Double(location.second.min_incl_time);
            writer.Key("max_incl_time");
            writer.Double(location.second.max_incl_time);
            writer.Key("min_excl_time");
            writer.Double(location.second.min_excl_time);
            writer.Key("max_excl_time");
            writer.Double(location.second.max_excl_time);
            writer.Key("accumulated_incl_time");
            writer.Double(location.second.sum_incl_time);
            writer.Key("accumulated_excl_time");
            writer.Double(location.second.sum_excl_time);
            writer.Key("avg_incl_time");
            writer.Double(location.second.sum_incl_time / location.second.count);
            writer.Key("avg_excl_time");
            writer.Double(location.second.sum_excl_time / location.second.count);
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        writer.EndObject();

    }
    writer.EndArray();
}

template <typename Writer>
void display_definitions(AllData alldata, Writer& writer){
    writer.Key("Definitions");
    writer.StartObject();
        writer.Key("regions");
        writer.StartArray();
            for(const auto& region : alldata.definitions.regions.get_all()){
                writer.StartObject();
                    writer.Key("id");
                    writer.Uint(region.first);
                    writer.Key("name");
                    writer.String(region.second.name.c_str());
                    writer.Key("paradigm_id");
                    writer.Uint(region.second.paradigm_id);
                    writer.Key("source_line");
                    writer.Uint(region.second.source_line);
                    writer.Key("file_name");
                    writer.String(region.second.file_name.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metrics");
        writer.StartArray();
            for(const auto& it : alldata.definitions.metrics.get_all()){
                const definitions::Metric& metric = it.second;
                writer.StartObject();
                    writer.Key(std::to_string(it.first).c_str());
                    writer.StartObject();
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
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metric_classes");
        writer.StartArray();
            for(const auto& it : alldata.definitions.metric_classes.get_all()){
                const definitions::Metric_Class& metric_class = it.second;

                writer.StartObject();
                    writer.Key(std::to_string(it.first).c_str());
                    writer.StartObject();
                        writer.Key("num_of_metrics");
                        writer.Uint(metric_class.num_of_metrics);
                        writer.Key("metric_member");
                        writer.StartArray();
                            for(const auto& member : metric_class.metric_member){
                                writer.StartObject();
                                    writer.Key(std::to_string(member.first).c_str());
                                    writer.Uint(member.second);
                                writer.EndObject();
                            }
                        writer.EndArray();
                        writer.Key("metric_occurrence");
                        writer.Uint(static_cast<uint> (metric_class.metric_occurrence));
                        writer.Key("recorder_kind");
                        writer.Uint(static_cast<uint> (metric_class.recorder_kind));
                    writer.EndObject();
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("paradigms");
        writer.StartArray();
            for(const auto& paradigm : alldata.definitions.paradigms.get_all()){
                writer.StartObject();
                    writer.Key(std::to_string(paradigm.first).c_str());
                    writer.String(paradigm.second.name.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("io_paradigms");
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
                        writer.Key("io_paradigm");
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

                    writer.Key(std::to_string(it.first).c_str());
                    writer.StartObject();

                        writer.Key("name");
                        writer.String(group.name.c_str());

                        writer.Key("type");
                        writer.Uint(group.type);

                        writer.Key("paradigm_id");
                        writer.Uint(group.paradigm_id);

                        writer.Key("members");
                        writer.StartArray();
                            for(const auto& member : group.members)
                                writer.Uint64(member);
                        writer.EndArray();


                    writer.EndObject();
                writer.EndObject();
            }
        writer.EndArray();


    writer.EndObject();
}

template <typename Writer>
void display_node(std::shared_ptr<tree_node> node, Writer& writer){
    writer.StartObject();
        writer.Key("function_id");
        writer.Uint64(node->function_id);

        writer.Key("parent");
        if(node->parent != NULL)
            writer.Uint64(node->parent->function_id);
        else
            writer.Null();

        writer.Key("node_data");
        writer.StartArray();
            for(const auto& data : node->node_data){
                writer.StartObject();
                    writer.Key(std::to_string(data.first).c_str());
                    writer.StartObject();
                        writer.Key("f_data");
                        writer.StartObject();
                            writer.Key("count");
                            writer.Uint64(data.second.f_data.count);
                            writer.Key("incl_time");
                            writer.Uint64(data.second.f_data.incl_time);
                            writer.Key("excl_time");
                            writer.Uint64(data.second.f_data.excl_time);
                        writer.EndObject();
                        writer.Key("m_data");
                        writer.StartObject();
                            writer.Key("count_send");
                            writer.Uint64(data.second.m_data.count_send);
                            writer.Key("count_recv");
                            writer.Uint64(data.second.m_data.count_recv);
                            writer.Key("bytes_send");
                            writer.Uint64(data.second.m_data.bytes_send);
                            writer.Key("bytes_recv");
                            writer.Uint64(data.second.m_data.bytes_recv);
                        writer.EndObject();
                        writer.Key("c_data");
                        writer.StartObject();
                            writer.Key("count_send");
                            writer.Uint64(data.second.c_data.count_send);
                            writer.Key("count_recv");
                            writer.Uint64(data.second.c_data.count_recv);
                            writer.Key("bytes_send");
                            writer.Uint64(data.second.c_data.bytes_send);
                            writer.Key("bytes_recv");
                            writer.Uint64(data.second.c_data.bytes_recv);
                        writer.EndObject();
                        writer.Key("metrics");
                            writer.StartArray();
                                for(const auto& metricData : data.second.metrics){
                                    writer.StartObject();
                                        writer.Key(std::to_string(data.first).c_str());
                                            writer.StartObject();
                                                writer.Key("MetricDataType");
                                                writer.Uint(static_cast<uint8_t>(metricData.second.type));
                                                writer.Key("data_incl");
                                                writer.StartObject();
                                                    writer.Key("u");
                                                    writer.Uint64(metricData.second.data_incl.u);
                                                    writer.Key("s");
                                                    writer.Int64(metricData.second.data_incl.s);
                                                    writer.Key("d");
                                                    writer.Double(metricData.second.data_incl.d);
                                                writer.EndObject();
                                            writer.EndObject();
                                    writer.EndObject();
                                }
                            writer.EndArray();
                    writer.EndObject();
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("children");
        writer.StartArray();
            for(const auto& child : node->children){
                writer.StartObject();
                writer.Key(std::to_string(child.first).c_str());
                display_node(child.second, writer);
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("has_p2p");
        writer.Bool(node->has_p2p);

        writer.Key("has_collop");
        writer.Bool(node->has_collop);

        writer.EndObject();
}

template <typename Writer>
void display_data_tree(AllData alldata, Writer& writer){
    writer.String("data_tree");
    writer.StartObject();
        writer.Key("root_nodes");
        writer.StartArray();
            for(const auto& node : alldata.call_path_tree.root_nodes){
                writer.StartObject();
                    writer.Key(std::to_string(node.first).c_str());
                    display_node(node.second, writer);
                writer.EndObject();
            }
        writer.EndArray();
    writer.EndObject();
}

template <typename Writer>
void display_meta_data(AllData alldata, Writer& writer){
    writer.Key("meta_data");
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
            for(const auto& class_id : alldata.metaData.metricClassToMetric){
                writer.Key("metricClassId");
                writer.Uint64(class_id.first);
                writer.StartArray();
                    for(const auto& num_metric : class_id.second){
                        writer.Key("metricId");
                        writer.Uint64(num_metric.first);
                        writer.Key("value");
                        writer.Uint64(num_metric.second);
                    }
                writer.EndArray();
            }
        writer.EndArray();

        writer.Key("timerResolution");
        writer.Uint64(alldata.metaData.timerResolution);

        writer.Key("myRank");
        writer.Uint64(alldata.metaData.myRank);

        writer.Key("numRanks");
        writer.Uint64(alldata.metaData.numRanks);

        #ifdef OTFPROFILER_MPI
            writer.Key("packBufferSize");
            writer.Uint64(alldata.metaData.packBufferSize);
            writer.Key("packBuffer");
            writer.String(alldata.metaData.Buffer);
        #endif
    writer.EndObject();
}


template <typename Writer>
void display_params(AllData alldata, Writer& writer){
    writer.Key("Params");
    writer.StartObject();
        writer.Key("max_file_handles");
        writer.Uint(alldata.params.max_file_handles);
        writer.Key("buffer_size");
        writer.Uint(alldata.params.buffer_size);
        writer.Key("verbose_level");
        writer.Uint(alldata.params.verbose_level);
        writer.Key("read_metrics");
        writer.Bool(alldata.params.read_metrics);
        writer.Key("output_type_set");
        writer.Bool(alldata.params.output_type_set);
        writer.Key("create_cube");
        writer.Bool(alldata.params.create_cube);
        writer.Key("create_json");
        writer.Bool(alldata.params.create_json);
        writer.Key("data_out");
        writer.Bool(alldata.params.data_out);
        writer.Key("summarize_it");
        writer.Bool(alldata.params.summarize_it);
        writer.Key("input_file_name");
        writer.String(alldata.params.input_file_name.c_str());
        writer.Key("input_file_prefix");
        writer.String(alldata.params.input_file_prefix.c_str());
        writer.Key("output_file_prefix");
        writer.String(alldata.params.output_file_prefix.c_str());
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
            writer.Key("node_id");
            writer.Uint(node->data.node_id);
            writer.Key("name");
            writer.String(node->data.name.c_str());
            writer.Key("class_id");
            writer.Uint(static_cast<uint8_t> (node->data.class_id));
            writer.Key("location_id");
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

template <typename Writer>
void display_system_tree(AllData alldata, Writer& writer){
    writer.Key("system_tree");
    writer.StartObject();
        writer.Key("system_nodes");
        display_system_node(alldata.definitions.system_tree.get_root(), alldata, writer);
        writer.Key("_size");
        writer.Uint(alldata.definitions.system_tree.size());
        writer.Key("num_nodes_per_level");
        writer.StartArray();
            for(const auto& num : alldata.definitions.system_tree.all_level())
                writer.Uint(num);
        writer.EndArray();



    writer.EndObject();
}