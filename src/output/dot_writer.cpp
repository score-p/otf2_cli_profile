#include "dot_writer.h"
#include <array>

void Dot_writer::read_data(AllData& alldata) {

    std::stack<Node*> parent_nodes;

    int call_id = 0;

    for ( const auto& region : alldata.call_path_tree ) {

        Node* node = new Node;

        node->call_id = call_id;
        ++call_id;

        std::string region_name = alldata.definitions.regions.get(region.function_id)->name;
        node->region = region_name;

        node->num_children = region.children.size();

        // mpi time resolution
        double timerResolution = (double)alldata.metaData.timerResolution;

        // accumulate data over all locations
        for (const auto& location : region.node_data) {
            // filter per rank
            if(alldata.params.rank == -1 || alldata.params.rank == location.first){

                node->invocations += location.second.f_data.count;

                double incl_time = location.second.f_data.incl_time.sum / timerResolution;
                if (node->min_incl_time > incl_time)
                    node->min_incl_time = incl_time;
                if (node->max_incl_time < incl_time)
                    node->max_incl_time = incl_time;
                node->sum_incl_time += incl_time;

                double excl_time = location.second.f_data.excl_time.sum / timerResolution;
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

        // set parent <-> child relationship
        if(parent_nodes.size() != 0){

            parent_nodes.top()->children.push_back(node);

            node->parent = parent_nodes.top();
            parent_nodes.pop();
        }

        for( int i = 0; i < node->num_children; ++i )
            parent_nodes.push(node);
        nodes.push_back(node);

        // accumulate total sum time of all nodes
        total_time += node->sum_excl_time;
    }
}

bool Dot_writer::open(){
    std::string filename = params.output_file_prefix + ".dot";
    result_file.open(filename);
    if(result_file.is_open()){

        // header of dot/graph file
        result_file
        << "digraph call_tree {\n"

        << "graph "
            << "[splines="  << splines
            << ", ranksep=" << ranksep << "];\n"

        << "node "
            << "[shape = "  << shape
            << ", fontname = \"Courier New\""
            << ", colorscheme=" << colorscheme
        << "];\n"

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

void Dot_writer::print(){

    if(params.top_nodes != 0)
        top_nodes();

    if(params.node_min_ratio > 0 && params.node_min_ratio < 100)
        filter = true;
    double ratio = total_time / 100 * params.node_min_ratio;
    int node_count = 0;

    for( const auto& node : nodes ){
        // mark nodes which should be printed
        // if no filtering is applied print all
        if(filter){
            if(node->sum_excl_time > ratio){

                // check if printing more nodes than number of "top_nodes"
                if(params.top_nodes != 0 && node_count == params.top_nodes)
                    break;
                ++node_count;

                node->state = NodeState::full;
                mark_predecessors(node);
            }else if(node->state != NodeState::partial){
                node->state = NodeState::dontprint;
            }
        }else{
            node->state = NodeState::full;
        }

        // get global min & max time skip when it's a filtered node
        if(filter && node->state != NodeState::full)
            continue;

        if( node->sum_excl_time < min_time )
            min_time = node->sum_excl_time;

        if( node->sum_excl_time > max_time )
            max_time = node->sum_excl_time;
    }

    for( auto& node : nodes ){
        if(node->state != NodeState::dontprint)
            print_node(*node);
    }
}

std::array<std::string, 3> time_units = {"s", "ms", "µs"};
std::tuple<double, std::string> formatting_time(double time){
    double t;
    std::string unit;
    for (int i = 0; i < time_units.size(); ++i) {
        if(time >= 1){
            t = time;
            unit = time_units[i];
            break;
        }else{
            time *= 1000;
        }
    }
    return std::make_tuple(t, unit);
}

std::ostream& operator<< (std::ostream &out, const std::tuple<double, std::string>& time){
    out << std::setprecision(3) << std::fixed << std::get<0>(time) << std::get<1>(time);
    return out;
}

void Dot_writer::print_node(Node& node){
    result_file
    << "\""             << node.call_id    << "\" [\n"
    << " label = \""    << node.region     << "\\l\n";

    std::string sp = "\\ ";
    std::string tab = "\\   "; // 3 spaces

    if(node.state == NodeState::full){
        result_file
        << "invocations: "  << node.invocations                    << "\\l\n"
        << "include time:"                                         << "\\l\n"
        << tab << "min: "   << formatting_time(node.min_incl_time) << "\\l\n"
        << tab << "max: "   << formatting_time(node.max_incl_time) << "\\l\n"
        << tab << "sum: "   << formatting_time(node.sum_incl_time) << "\\l\n"
        << tab << "avg: "   << formatting_time(node.avg_incl_time) << "\\l\n"
        << "exclude time:"                                         << "\\l\n"
        << tab << "min: "   << formatting_time(node.min_excl_time) << "\\l\n"
        << tab << "max: "   << formatting_time(node.max_excl_time) << "\\l\n"
        << tab << "sum: "   << formatting_time(node.sum_excl_time) << "\\l\n"
        << tab << "avg: "   << formatting_time(node.avg_excl_time) << "\\l\n";
    }

    result_file << " \"\n";

    // colorize node
    if(node.state == NodeState::full){
        result_file << " fillcolor=" << get_node_color(node.sum_excl_time) << ",\n";
        result_file << " style=filled\n";
    }

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

// void Dot_writer::filter_nodes(){
    // double ratio = total_time / 100 * params.node_min_ratio;
    // int node_count = 0;
    // for(const auto& node : nodes){
    //     if(node->sum_excl_time > ratio){

    //         // check if printing more nodes than number of "top_nodes"
    //         if(params.top_nodes != 0 && node_count == params.top_nodes)
    //             break;
    //         ++node_count;

    //         node->state = NodeState::full;
    //         mark_predecessors(node);
    //     } else if(node->state != NodeState::partial){
    //         node->state = NodeState::dontprint;
    //     }
    // }
    // filter = true;
// }


void Dot_writer::mark_predecessors(Node* node){

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
    filter = true;
}

int Dot_writer::get_node_color(const double time){
    double timerange = max_time - min_time;
    int color_code = num_colors;
    for( int i = 0; i < num_colors; ++i ){
        if( time >= timerange/num_colors*i+min_time )
            color_code = num_colors-i;
    }
    return color_code;
}
