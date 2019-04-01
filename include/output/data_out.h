// aggregate data for output
#ifndef DATA_OUT_H
#define DATA_OUT_H

#include "all_data.h"
#include "data_tree.h"

struct Data {
    std::string region;
    uint64_t location;
    uint32_t count;
    double min_incl_time = std::numeric_limits<uint64_t>::max();
    double min_excl_time = std::numeric_limits<uint64_t>::max();
    double max_incl_time = 0;
    double max_excl_time = 0;
    double sum_incl_time = 0;
    double sum_excl_time = 0;
};

// <region_ref, <location_id, Data>
typedef std::map<uint64_t, std::map<uint64_t, Data>> Data_map;

static Data_map read_data(AllData& alldata);

template <typename Writer>
void display_regions(Data_map data, Writer& writer);

template <typename Writer>
void display_definitions(AllData alldata, Writer& writer);

template <typename Writer>
void display_data_tree(AllData alldata, Writer& writer);

template <typename Writer>
void display(Writer& writer, Data_map data, AllData alldata);

template <typename Writer>
void display_meta_data(AllData alldata, Writer& writer);

bool DataOut(AllData& alldata);

#endif