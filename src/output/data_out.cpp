
#include <fstream>
#include <iostream>
#include <string>
#include "data_out.h"
#include "all_data.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h" // human readable json



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

    // aggregated data about regions
    writer.StartObject();
    // display_regions(data, writer); // to be removed
    display_data_tree(alldata, writer);
    // meta_data
    display_meta_data(alldata, writer);

    display_definitions(alldata, writer);
    // time measurement
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
                    // writer.Key("location");
                    // writer.Uint64(data.first);
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

        writer.Key("have_message");
        writer.StartArray();
            for(const auto& msg : node->have_message){
                writer.StartObject();
                writer.Key("count_send");
                writer.Uint64(msg.second->count_send);
                writer.Key("count_recv");
                writer.Uint64(msg.second->count_recv);
                writer.Key("bytes_send");
                writer.Uint64(msg.second->bytes_send);
                writer.Key("bytes_recv");
                writer.Uint64(msg.second->bytes_recv);
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("have_collop");
        writer.StartArray();
            for(const auto& collop : node->have_collop){
                writer.StartObject();
                writer.Key("count_send");
                writer.Uint64(collop.second->count_send);
                writer.Key("count_recv");
                writer.Uint64(collop.second->count_recv);
                writer.Key("bytes_send");
                writer.Uint64(collop.second->bytes_send);
                writer.Key("bytes_recv");
                writer.Uint64(collop.second->bytes_recv);
                writer.EndObject();

            }
        writer.EndArray();
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
                writer.Key("key");
                writer.Uint64(comm.first);
                writer.Key("value");
                writer.Uint64(comm.second);
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("processIdToName");
        writer.StartArray();
            for(const auto& procs : alldata.metaData.processIdToName){
                writer.StartObject();
                writer.Key("processID");
                writer.Uint64(procs.first);
                writer.Key("name");
                writer.String(procs.second.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        writer.Key("metricIdToName");
        writer.StartArray();
            for(const auto& metric : alldata.metaData.metricIdToName){
                writer.StartObject();
                writer.Key("metricID");
                writer.Uint64(metric.first);
                writer.Key("value");
                writer.String(metric.second.c_str());
                writer.EndObject();
            }
        writer.EndArray();

        //does this even work??
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