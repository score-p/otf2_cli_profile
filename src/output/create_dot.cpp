#include <fstream>
#include <iostream>
#include <string>
#include <stack>
#include <algorithm>

#include "create_dot.h"
struct Node {
    uint32_t call_id = 0;
    
    //name of function, block ...
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

class Dot_writer{
public:
    Dot_writer(Data& data, Params& params): data(data), params(params)
    {}
private:
    std::ofstream result_file;
    Params params;
    Data data;

    //metaData
    double min_time = std::numeric_limits<uint64_t>::max();
    double max_time = 0;
    double timerange = 0;
    std::map<uint32_t, Node*> printed_nodes;

    // graph parameter
    const std::string splines = "ortho";
    const float ranksep = 1.5;
    const std::string shape = "record";
    const std::string colorscheme = "spectral9";
    const int num_colors = 9;
public:

    bool open(std::string filename = "result.dot"){
        result_file.open(filename);
        if(result_file.is_open()){
            // print header of graph file
            result_file 
                << "digraph call_tree {\n"
                << "graph [splines=ortho, ranksep=1.5];\n"
                << "node [shape = record, colorscheme=spectral9];\n"
                << "edge [];\n"
            << std::endl;
            getMeta();
            return true;
        } 
        else
        {
            std::cout << "Error opening file" << std::endl;
            return false;
        }
    }

    void close(){
        result_file << "}" << std::endl;
        result_file.close();
    }


    void print(){
        //top_nodes!!!
        if(params.top_nodes != 0){
            top_nodes();
            return;
        }
        if(params.node_min_ratio == 0 || params.node_min_ratio == 100){
            for( const auto& region : data ){
                print_node(*region);
            }
            return;
        }
        else {
            for(const auto& region : data){
                if(region->sum_excl_time > params.node_min_ratio){
                    print_node(*region);
                    print_predecessor(*region);
                }
            }
        }
    }
private:
    void getMeta(){
        
        for( const auto& region : data ){
            if( region->sum_excl_time < min_time )
                min_time = region->sum_excl_time;
            if( region->sum_excl_time > max_time )
                max_time = region->sum_excl_time;
        }
        timerange = max_time - min_time;
    }

    // draw all predessor
    void print_predecessor( Node& region){
        Node* curr = &region;
        while( curr->parent != nullptr){
            curr = region.parent;
            // draw if parent node hasn't been drawn yet else break
            if(printed_nodes.find(curr->call_id) == printed_nodes.end())
                print_node_partial(*curr);
            else
                break;
        }
    }

    void top_nodes(){
        int num = params.top_nodes;
        std::sort(data.begin(), data.end(), [](Node* a, Node* b){
            return a->sum_excl_time > b->sum_excl_time;
        });


        for( auto it = data.begin(); it != data.begin()+num-1; ++it ){
            if(it == data.end())
                break;

            print_node(**it);
            print_predecessor(**it);
        }
    }

    void print_node_partial(Node& region){
        result_file 
                << "\"" << region.call_id << "\" ["
                << "label = \"" 
                << region.region << "\"" << std::endl;
        
        // colorize node, 9 colors
        result_file
            << " fillcolor=" << node_color(region.sum_excl_time) << ",\n"
            << " style=filled\n";

        // closing tag
        result_file << "];\n" << std::endl;

        // set edge netween node and parent
        if( region.parent){
            result_file 
                << region.parent->call_id
                << " -> "
                << region.call_id
                << ";"
            << std::endl;
        }

        printed_nodes[region.call_id] = &region;
    }

    void print_node(Node& region){
        // filter nodes
        result_file 
            << "\"" << region.call_id << "\" [\n"
            << " label = \"" 
            << region.region << "\\l\n"
            << " invocations: " << region.invocations << "\\l\n"
            << " include time:" << "\\l\n"
            << "  min: " << region.min_incl_time << "\\l\n"
            << "  max: " << region.max_incl_time << "\\l\n"
            << "  sum: " << region.sum_incl_time << "\\l\n"
            << "  avg: " << region.avg_incl_time << "\\l\n"
            << " exclude time:" << "\\l\n"
            << "  min: " << region.min_excl_time << "\\l\n"
            << "  max: " << region.max_excl_time << "\\l\n"
            << "  sum: " << region.sum_excl_time << "\\l\n"
            << "  avg: " << region.avg_excl_time << "\\l\n"
            << " \"\n";
        
        // colorize node, 9 colors
        result_file
            << " fillcolor=" << node_color(region.sum_excl_time) << ",\n"
            << " style=filled\n";

        // closing tag
        result_file << "];\n" << std::endl;

        // set edge netween node and parent
        if( region.parent){
            result_file 
                << region.parent->call_id
                << " -> "
                << region.call_id
                << ";"
            << std::endl;
        }

        printed_nodes[region.call_id] = &region;
    }

    int node_color(const double time){
        int color_code = num_colors;
        for( int i = 0; i < num_colors; ++i ){
            if( time >= timerange/num_colors*i+min_time )
                color_code = num_colors-i;
        }
        return color_code;
    }
};

bool CreateDot(AllData& alldata) {
    alldata.verbosePrint(1, true, "producing dot output");
    Data data = read_data(alldata);
    Dot_writer writer(data, alldata.params);
    writer.open("res.dot");
    writer.print();
    writer.close();
    return true;
}


