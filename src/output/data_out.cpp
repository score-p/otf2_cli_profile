
#include <fstream>
#include <iostream>
#include <string>
#include "data_out.h"
#include "all_data.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h" // human readable json

template <typename Writer>
void display(Writer& writer, Data_map data);


bool DataOut(AllData& alldata){


    if (alldata.metaData.myRank != 0)
        return true;

    // (vlevel, bool master_only, msg)
    alldata.verbosePrint(1, true, "producing json output");

    Data_map data =  read_data(alldata);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    display(writer, data);
    std::string fname = alldata.params.output_file_prefix + ".json";
    std::ofstream outfile(fname);
    outfile << buffer.GetString() << std::endl;
    std::cout << buffer.GetString() << std::endl;

    return true;
}

template <typename Writer>
void display(Writer& writer, Data_map data){
    writer.StartObject();
    writer.String("Regions");
    writer.StartArray();
    for(const auto& region : data){
        writer.StartObject();
        // regionname
        writer.String(region.second.begin()->second.region.c_str());
        writer.StartObject();
        writer.Key("Locations");
        writer.StartArray();
        for(const auto& location : region.second){
            writer.StartObject();   //location
            writer.String("location");
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
    writer.EndObject();
}