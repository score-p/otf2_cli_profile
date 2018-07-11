#include <fstream>
#include <string>

#include "create_csv.h"
// #include "data_output.h"



void _create_CSV(Data_map& data){
	// open file for csv output	
	std::ofstream result_file;
	// result_file.open (alldata.params.output_file_prefix+".csv");
	result_file.open ("result.csv");
	
	// comma seperator between csv objects / after objects excepts last one
	const auto separator = ",";
	const auto* sep = "";

	result_file 
		<< "region," 
		<< "location," 
		<< "invocations," 
		<< "min_incl_time," 
		<< "max_incl_time," 
		<< "min_excl_time," 
		<< "max_excl_time," 
		<< "accumulated_incl_time," 
		<< "accumulated_excl_time," 
		<< "avg_incl_time," 
		<< "avg_excl_time" 
	<< std::endl;

	for ( auto& region : data){
		for ( auto& location : region.second){

			// comma seperator
			result_file << sep << std::endl;
			sep = separator;

			result_file 
				<< location.second.region << sep
				<< location.first << sep
		        << location.second.count << sep		        
		        << location.second.min_incl_time << sep
		        << location.second.max_incl_time << sep		        
		        << location.second.min_excl_time << sep		        
		        << location.second.max_excl_time << sep
		        << location.second.sum_incl_time << sep
		 		<< location.second.sum_excl_time << sep
		        << location.second.sum_incl_time / location.second.count << sep		        
		        << location.second.sum_excl_time / location.second.count;
		}
	}

	result_file.close();
};

bool create_CSV(AllData& alldata) {

	if (alldata.metaData.myRank != 0)
		return true;
	
	// (vlevel, bool master_only, msg)
	alldata.verbosePrint(1, true, "producing csv output");

	Data_map data =	read_data(alldata);

	_create_CSV(data);

	return true;
};