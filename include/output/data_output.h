// aggregate data for output
#ifndef OUTPUT_H
#define OUTPUT_H

// #include <iostream>
// #include <map>
// #include <limits>
#include "all_data.h"

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
typedef std::map<uint64_t, std::map<uint64_t, Data> > Data_map;

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

#endif