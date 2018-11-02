#include "dot_writer.h"


bool Dot_writer::open(std::string filename = "result.dot"){
    result_file.open(filename);
    if(result_file.is_open()){
        
        // header of dot/graph file
        result_file 
        << "digraph call_tree {\n"
        << "graph [splines=ortho, ranksep=1.5];\n"
        << "node [shape = record, colorscheme=spectral9];\n"
        << "edge [];\n"
        << std::endl;
        
        return true;
    } 
    else
    {
        std::cout << "Error opening file" << std::endl;
        return false;
    }
}

void Dot_writer::close(){
    result_file << "}" << std::endl;
    result_file.close();
}

void Dot_writer::filter(){
    bool filter = false;

    if(params.top_nodes != 0){
        top_nodes();
        filter = true;
    }

    if(params.node_min_ratio != 0 && params.node_min_ratio != 100){

        double ratio = total_time / 100 * params.node_min_ratio;

        int node_count = 0;
        for(const auto& node : nodes){
            if(node->sum_excl_time > ratio){

                // check if printing more nodes than "top_nodes" marked
                if(params.top_nodes != 0 && node_count == params.top_nodes)
                    break;
                ++node_count;

                node->state = NodeState::full;
                mark_predecessors(node);
            } else if(node->state != NodeState::partial){
                node->state = NodeState::dontprint;
            }
        }
        filter = true;
    } 

    if(!filter){
        for( const auto& node : nodes )
            node->state = NodeState::full;
    }
}

void Dot_writer::print_node(Node& node){
    result_file 
    << "\""             << node.call_id       << "\" [\n"
    << " label = \""    << node.region        << "\\l\n";

    if(node.state == NodeState::full){
        result_file
        << " invocations: " << node.invocations   << "\\l\n"
        << " include time:"                       << "\\l\n"
        << "  min: "        << node.min_incl_time << "\\l\n"
        << "  max: "        << node.max_incl_time << "\\l\n"
        << "  sum: "        << node.sum_incl_time << "\\l\n"
        << "  avg: "        << node.avg_incl_time << "\\l\n"
        << " exclude time:"                       << "\\l\n"
        << "  min: "        << node.min_excl_time << "\\l\n"
        << "  max: "        << node.max_excl_time << "\\l\n"
        << "  sum: "        << node.sum_excl_time << "\\l\n"
        << "  avg: "        << node.avg_excl_time << "\\l\n";
    }
    
    result_file << " \"\n";
    
    // colorize node
    result_file
    << " fillcolor=" << node_color(node.sum_excl_time) << ",\n"
    << " style=filled\n";

    // closing node tag
    result_file << "];" << std::endl;

    // set edge between this node and its parent
    if( node.parent){
        result_file 
        << node.parent->call_id
        << " -> "
        << node.call_id
        << ";\n"
        << std::endl;
    }

    // mark as printed
    node.state = NodeState::printed; 
}

void Dot_writer::gather_meta(){
    for( const auto& node : nodes ){

        if( node->sum_excl_time < min_time )
            min_time = node->sum_excl_time;

        if( node->sum_excl_time > max_time )
            max_time = node->sum_excl_time;

        total_time += node->sum_excl_time;
    }

    timerange = max_time - min_time;
}

void Dot_writer::mark_predecessors( Node* node){

    Node* curr = node;

    while( curr->parent != nullptr ){
        curr = curr->parent;

        if(curr->state == NodeState::dontprint)
            curr->state = NodeState::partial;
        else
            break;
    }
}

void Dot_writer::top_nodes(){

    std::sort(nodes.begin(), nodes.end(), [](Node* a, Node* b){
        return a->sum_excl_time > b->sum_excl_time;
    });

    int num = params.top_nodes;
    for( auto it = nodes.begin(); it != nodes.begin()+num; ++it ){
        if(it == nodes.end())
            break;

        (*it)->state = NodeState::full;
        mark_predecessors(*it);
    }
}

int Dot_writer::node_color(const double time){
    int color_code = num_colors;
    for( int i = 0; i < num_colors; ++i ){
        if( time >= timerange/num_colors*i+min_time )
            color_code = num_colors-i;
    }
    return color_code;
}

void Dot_writer::add_node(Node* node){
    nodes.push_back(node);
}

void Dot_writer::print(){

    gather_meta();

    filter();
    
    for( auto& node : nodes ){
        if(node->state != NodeState::dontprint)
            print_node(*node);
    }
}