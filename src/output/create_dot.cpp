#include <fstream>
#include <iostream>
#include <string>
#include <stack>

#include "create_dot.h"

struct Node {
	uint32_t call_id = 0;
	
	std::string region;
	uint32_t invocations = 0;
	uint32_t num_children = 0;
	
	double min_incl_time = std::numeric_limits<uint64_t>::max();
	double max_incl_time = 0;
	double sum_incl_time = 0;

	double min_excl_time = std::numeric_limits<uint64_t>::max();
	double max_excl_time = 0;
	double sum_excl_time = 0;
};

typedef std::vector<Node> Data;



Data read_data(AllData& alldata){
	
	Data data;
	
	for (auto& region : alldata.call_path_tree) {
		Node node;

		std::string region_name = alldata.definitions.regions.get(region.function_id)->name;
		
		node.region = region_name;
		
		node.num_children = region.children.size();

		double timerResolution = (double)alldata.metaData.timerResolution;
		// acumulate data over all locations
		for (auto& location : region.node_data) {	

			node.invocations += location.second.f_data.count;

			double incl_time = location.second.f_data.incl_time / timerResolution;
			if (node.min_incl_time > incl_time)
				node.min_incl_time = incl_time;
			if (node.max_incl_time < incl_time)
				node.max_incl_time = incl_time;
			node.sum_incl_time += incl_time;

			double excl_time = location.second.f_data.excl_time / timerResolution;
			if (node.min_excl_time > excl_time)
				node.min_excl_time = excl_time;
			if (node.max_excl_time < excl_time)
				node.max_excl_time = excl_time;
			node.sum_excl_time += excl_time;			
		}

		data.push_back(node);
	}
	return data;

};

void write_dot(Data data) {

	std::stack<uint32_t> tmp;
	int call_id = 0;

	std::ofstream result_file;
	result_file.open ("result.dot");

	result_file 
		<< "digraph call_tree {\n"
		<< "graph [splines=ortho];\n"
		<< "node [shape = record];\n"
		<< "edge [];\n"
	<< std::endl;

	for ( auto& region : data ){
		result_file 
			<< "\"" << call_id << "\" [\n"
			<< "label = \"" 
			<< "" << region.region << "\\l\n"
			<< "invocations:" << region.invocations << "\\l\n"
			<< "include time:" << "\\l\n"
			<< "min: " << region.min_incl_time << "\\l\n"
			<< "max: " << region.max_incl_time << "\\l\n"
			<< "sum: " << region.sum_incl_time << "\\l\n"
			<< "avg: " << region.sum_incl_time / region.invocations << "\\l\n"
			<< "exclude time:" << "\\l\n"
			<< "min: " << region.min_excl_time << "\\l\n"
			<< "max: " << region.max_excl_time << "\\l\n"
			<< "sum: " << region.sum_excl_time << "\\l\n"
			<< "avg: " << region.sum_excl_time / region.invocations << "\\l\n"
			<< "\"\n"
			<< "];"
		<< std::endl;
		
		// if parent set connection
		if( call_id != 1){
			result_file 
				<< tmp.top()
				<< " -> "
				<< call_id
				<< ";"
			<< std::endl;
			tmp.pop();
		}

		// trick to remember parents
		for (int i = 0; i < region.num_children; ++i)
			tmp.push(call_id);

		++call_id; 
	}

	result_file << "}" << std::endl;


};

bool CreateDot(AllData& alldata){
	alldata.verbosePrint(1, true, "producing dot output");
	Data data = read_data(alldata);
	write_dot(data);
	return true;
};