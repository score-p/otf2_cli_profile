#include <fstream>
#include <iostream>
#include <string>
#include <stack>
#include <algorithm>

#include "create_dot.h"
#include "dot_writer.h"

Data& read_data( AllData& alldata) {
    
    Data* data = new Data;
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
        
        // mpi time resolution
        double timerResolution = (double)alldata.metaData.timerResolution;

        // acumulate data over all locations
        for (const auto& location : region.node_data) {
            // filter per rank
            if(alldata.params.rank == -1 || alldata.params.rank == location.first){
                
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
        }

        // average time
        node->avg_incl_time = node->sum_incl_time / region.node_data.size();
        node->avg_excl_time = node->sum_excl_time / region.node_data.size();


        if(parent_nodes.size() != 0){
            
            // get parent of this node
            // add this node to their children
            parent_nodes.top()->children.push_back(node);

            // add parent to this node
            node->parent = parent_nodes.top();
            parent_nodes.pop();

        }

        // temporarily add this node to stack for child-parent relation see above
        for( int i = 0; i < node->num_children; ++i )
            parent_nodes.push(node);

        //finish
        data->push_back(node);
    }
    return *data;
}



bool CreateDot(AllData& alldata) {
    alldata.verbosePrint(1, true, "producing dot output");
    Data data = read_data(alldata);
    Dot_writer writer(data, alldata.params);
    writer.open("res.dot");
    writer.print();
    writer.close();
    return true;
}


