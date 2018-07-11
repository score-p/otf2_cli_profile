#include <fstream>
#include <string>

#include "create_json.h"

void _create_JSON(Data_map& data){
	// open file for json output	
	std::ofstream result_file;
	// result_file.open (alldata.params.output_file_prefix+".json");
	result_file.open ("result.json");
	
	// comma seperator between json objects / after objects excepts last one
	const auto separator = ",\n";
	const auto* sep = "";

	result_file 
		<< "{\n"
		<< "\"regions\": [\n";

	for ( auto& region : data){
		for ( auto& location : region.second){

			// comma seperator
			result_file << sep;
			sep = separator;

			result_file << "\t{\n";
			
			result_file 
				<< "\t\"region\": \"" << location.second.region << "\",\n"
				<< "\t\"location\": \"" << location.first << "\",\n"	
		        << "\t\"invocations\": \"" << location.second.count << "\",\n"		        
		        << "\t\"min_incl_time\": \"" << location.second.min_incl_time << "\",\n"
		        << "\t\"max_incl_time\": \"" << location.second.max_incl_time << "\",\n"		        
		        << "\t\"min_excl_time\": \"" << location.second.min_excl_time << "\",\n"		        
		        << "\t\"max_excl_time\": \"" << location.second.max_excl_time << "\",\n"
		        << "\t\"accumulated_incl_time\": \"" << location.second.sum_incl_time << "\",\n"
		 		<< "\t\"accumulated_excl_time\": \"" << location.second.sum_excl_time << "\",\n"
		        << "\t\"avg_incl_time\": \"" << location.second.sum_incl_time / location.second.count << "\",\n"		        
		        << "\t\"avg_excl_time\": \"" << location.second.sum_excl_time / location.second.count << "\"\n"
			<< "\t}";
		}
	}

	result_file 
		<< "\n\t]\n"
		<< "}" 
	<< std::endl;

	result_file.close();
};

bool create_JSON(AllData& alldata) {

	if (alldata.metaData.myRank != 0)
		return true;
	
	// (vlevel, bool master_only, msg)
	alldata.verbosePrint(1, true, "producing json output");

	Data_map data =	read_data(alldata);

	_create_JSON(data);

	return true;
};