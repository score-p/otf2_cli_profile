#include <fstream>
#include <iostream>
#include <string>
#include <stack>

#include "create_dot.h"

struct Node {
    uint32_t call_id = 0;
    
    std::string region;
    uint32_t invocations = 0;

    Node* parent = nullptr;

    uint32_t num_children = 0;
    std::vector<Node*> children;

    double min_incl_time = std::numeric_limits<uint64_t>::max();
    double max_incl_time = 0;
    double sum_incl_time = 0;
    double avg_incl_time = 0;

    double min_excl_time = std::numeric_limits<uint64_t>::max();
    double max_excl_time = 0;
    double sum_excl_time = 0;
    double avg_excl_time = 0;

};

typedef std::vector<Node*> Data;

bool filter_rank(Params& params){

}

Data read_data(AllData& alldata) {
    
    Data data;
    std::stack<Node*> parent_nodes;
    
    int call_id = 0;
    // uint64_t total_time = alldata[0]. 
    
    for ( const auto& region : alldata.call_path_tree ) {
        
        Node* node = new Node;

        //call_id
        node->call_id = call_id;
        ++call_id;

        //regionname
        std::string region_name = alldata.definitions.regions.get(region.function_id)->name;
        node->region = region_name;
        
        // number of children
        node->num_children = region.children.size();
        
        
        double timerResolution = (double)alldata.metaData.timerResolution;

        // acumulate data over all locations

        for (const auto& location : region.node_data) {  
            if(alldata.params.rank != -1 && alldata.params.rank != location.first){
                continue;
            }
            node->invocations += location.second.f_data.count;

            double incl_time = location.second.f_data.incl_time / timerResolution;
            if (node->min_incl_time > incl_time)
                node->min_incl_time = incl_time;
            if (node->max_incl_time < incl_time)
                node->max_incl_time = incl_time;
            node->sum_incl_time += incl_time;

            double excl_time = location.second.f_data.excl_time / timerResolution;
            if (node->min_excl_time > excl_time)
                node->min_excl_time = excl_time;
            if (node->max_excl_time < excl_time)
                node->max_excl_time = excl_time;
            node->sum_excl_time += excl_time;
        }

        // average time
        node->avg_incl_time = node->sum_incl_time / region.node_data.size();
        node->avg_excl_time = node->sum_excl_time / region.node_data.size();


        if(parent_nodes.size() != 0){
            
            // get parent of this node
            // add this node to their children
            parent_nodes.top()->children.push_back(node);

            // add parent
            node->parent = parent_nodes.top();
            parent_nodes.pop();

        }

        // temporarily add this node to stack for child-parent relation see above
        for( int i = 0; i < node->num_children; ++i )
            parent_nodes.push(node);

        //finish
        data.push_back(node);
    }
    return data;
}

// returns true when filter applys to one node on the remaining subgraph 
bool filter_min_ratio(const Node* node, const Params& params){
    for(const auto& child : node->children){
        if(filter_min_ratio(child, params) == true)
            return true;
    }
    if(node->sum_excl_time > params.node_min_ratio)
        return true;

    return false;
}


void print_dot(const Data& data, const Params& params) {
    
    std::ofstream result_file;
    result_file.open ("result.dot");

    // find min and max value for excl time
    double min_time = std::numeric_limits<uint64_t>::max();
    double max_time = 0;
    for( const auto& region : data ){
        if( region->sum_excl_time < min_time )
            min_time = region->sum_excl_time;
        if( region->sum_excl_time > max_time )
            max_time = region->sum_excl_time;
    }

    double timerange = max_time - min_time;

    // print header of graph file
    result_file 
        << "digraph call_tree {\n"
        << "graph [splines=ortho, ranksep=1.5];\n"
        << "node [shape = record, colorscheme=spectral9];\n"
        << "edge [];\n"
    << std::endl;

    // print node
    for( const auto& region : data ){
        if( filter_min_ratio( region, params ) == true){
            result_file 
                << "\"" << region->call_id << "\" [\n"
                << "label = \"" 
                << "" << region->region << "\\l\n";

            // Populate Node only when filter not apply
            if( region->sum_excl_time > params.node_min_ratio ){

                result_file
                    << "invocations: " << region->invocations << "\\l\n"
                    << "include time:" << "\\l\n"
                    << " min: " << region->min_incl_time << "\\l\n"
                    << " max: " << region->max_incl_time << "\\l\n"
                    << " sum: " << region->sum_incl_time << "\\l\n"
                    << " avg: " << region->avg_incl_time << "\\l\n"
                    << "exclude time:" << "\\l\n"
                    << " min: " << region->min_excl_time << "\\l\n"
                    << " max: " << region->max_excl_time << "\\l\n"
                    << " sum: " << region->sum_excl_time << "\\l\n"
                    << " avg: " << region->avg_excl_time << "\\l\n"
                    << "\"\n";
                
                // colorize node, 9 colors
                int num_colors = 9;
                int color_code = num_colors;
                for( int i = 0; i < num_colors; ++i ){
                    if( region->sum_excl_time >= timerange/num_colors*i+min_time )
                        color_code = num_colors-i;
                }
                result_file
                    << "fillcolor=" << color_code << ",\n"
                    << "style=filled";

            } else {
                result_file << "\"\n";
            }

            // closing tag
            result_file << "];" << std::endl;

            // set edge netween node and parent
            if( region->parent){
                result_file 
                    << region->parent->call_id
                    << " -> "
                    << region->call_id
                    << ";"
                << std::endl;
            }
        }
    }

    result_file << "}" << std::endl;
}

bool CreateDot(AllData& alldata) {
    alldata.verbosePrint(1, true, "producing dot output");
    Data data = read_data(alldata);
    print_dot(data, alldata.params);
    return true;
}
