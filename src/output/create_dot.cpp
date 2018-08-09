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

	// temporary stack for saving parent nodes call_id
	std::stack<uint32_t> tmp;
	int call_id = 0;

	std::ofstream result_file;
	result_file.open ("result.dot");

	// find min and max value for excl time
	double min_sum = std::numeric_limits<uint64_t>::max();
	double max_sum = 0;
	for (auto& region : data){
		if(region.sum_excl_time < min_sum)
			min_sum = region.sum_excl_time;
		if(region.sum_excl_time > max_sum)
			max_sum = region.sum_excl_time;
	}
	double range = max_sum-min_sum;

	// head of graph file
	result_file 
		<< "digraph call_tree {\n"
		<< "graph [splines=ortho];\n"
		<< "node [shape = record, colorscheme=spectral9];\n"
		<< "edge [];\n"
	<< std::endl;

	// node
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
			<< "\"\n";
		
		// colorize node, 9 colors
		int color_code = 9;
		for (int i = 1; i < 9; ++i){
			std::cout << region.sum_excl_time << std::endl;
			std::cout << range/9*i+min_sum << std::endl;
			if(region.sum_excl_time >= range/9*i+min_sum){
				color_code = 10-i;
		};
		result_file
			<< "fillcolor=" << color_code << ",\n"
			<< "style=filled";

		// close node
		result_file
			<< "];"
		<< std::endl;

		// set edge to parent
		if( call_id != 0){
			result_file 
				<< tmp.top()
				<< " -> "
				<< call_id
				<< ";"
			<< std::endl;
			tmp.pop();
		}

		// push node "num_children" times on stack
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