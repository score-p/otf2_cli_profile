#include "dot_writer.h"


bool Dot_writer::open(std::string filename = "result.dot"){
    result_file.open(filename);
    if(result_file.is_open()){
        // print header of graph file
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

void Dot_writer::print_node(Node& region, bool full_node = true){
    // filter nodes
    result_file 
        << "\""             << region.call_id       << "\" [\n"
        << " label = \""    << region.region        << "\\l\n";

    if(full_node){
        result_file
            << " invocations: " << region.invocations   << "\\l\n"
            << " include time:"                         << "\\l\n"
            << "  min: "        << region.min_incl_time << "\\l\n"
            << "  max: "        << region.max_incl_time << "\\l\n"
            << "  sum: "        << region.sum_incl_time << "\\l\n"
            << "  avg: "        << region.avg_incl_time << "\\l\n"
            << " exclude time:"                         << "\\l\n"
            << "  min: "        << region.min_excl_time << "\\l\n"
            << "  max: "        << region.max_excl_time << "\\l\n"
            << "  sum: "        << region.sum_excl_time << "\\l\n"
            << "  avg: "        << region.avg_excl_time << "\\l\n";
    }
    
    result_file << " \"\n";
    
    // colorize node, 9 colors
    result_file
        << " fillcolor=" << node_color(region.sum_excl_time) << ",\n"
        << " style=filled\n";

    // closing tag
    result_file << "];" << std::endl;

    // set edge netween node and parent
    if( region.parent){
        result_file 
            << region.parent->call_id
            << " -> "
            << region.call_id
            << ";\n"
        << std::endl;
    }
    // remember which nodes have been drawn
    printed_nodes[region.call_id] = &region;

    
}

void Dot_writer::print(Data& data){
    getMeta(data);
    if(params.top_nodes != 0){
        top_nodes(data);
        return;
    }
    if(params.node_min_ratio != 0 || params.node_min_ratio != 100){
        for(const auto& region : data){
            if(region->sum_excl_time > params.node_min_ratio){
                print_node(*region);
                print_predecessors(*region);
            }
        }
        return;
    }
    else {
        for( const auto& region : data ){
            print_node(*region);
        }
        return;
    }
}

void Dot_writer::getMeta(Data& data){
    
    for( const auto& region : data ){
        if( region->sum_excl_time < min_time )
            min_time = region->sum_excl_time;
        if( region->sum_excl_time > max_time )
            max_time = region->sum_excl_time;
    }
    timerange = max_time - min_time;
}

// draw all predessor
void Dot_writer::print_predecessors( Node& region){
    Node* curr = &region;
    while( curr->parent != nullptr){
        curr = region.parent;
        // draw if parent node hasn't been drawn yet else break
        if(printed_nodes.find(curr->call_id) == printed_nodes.end())
            print_node(*curr, false);
        else
            break;
    }
}

void Dot_writer::top_nodes(Data& data){
    int num = params.top_nodes;
    std::sort(data.begin(), data.end(), [](Node* a, Node* b){
        return a->sum_excl_time > b->sum_excl_time;
    });


    for( auto it = data.begin(); it != data.begin()+num; ++it ){
        if(it == data.end())
            break;

        print_node(**it);
        print_predecessors(**it);
    }
}


/* class Nodes{
    private:
        string label;
        std::vector<Attributes*> attributes;

    }

*/
void Dot_writer::print_node(){
    //print label
    //print attributes
    //color
    //edge

}
int Dot_writer::node_color(const double time){
    int color_code = num_colors;
    for( int i = 0; i < num_colors; ++i ){
        if( time >= timerange/num_colors*i+min_time )
            color_code = num_colors-i;
    }
    return color_code;
}