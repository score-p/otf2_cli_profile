#ifndef DOT_WRITER_H
#define DOT_WRITER_H

#include <fstream>
#include <algorithm>
#include <stack>
#include <string>
#include <tuple>
#include <iomanip>

#include "all_data.h"

enum NodeState{
    full, partial, dontprint, printed
};

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

    NodeState state = NodeState::dontprint;
};

class Dot_writer{
public:
    Dot_writer(const Params& params): params(params){}
    ~Dot_writer(){
        for(const auto& node : nodes){
            delete node;
        }
    }

private:
    std::ofstream result_file;
    const Params& params;
    std::vector<Node*> nodes;
    bool filter = false;

    // metaData
    double min_time     = std::numeric_limits<uint64_t>::max();
    double max_time     = 0;
    double total_time   = 0;

    // graph parameter
    const std::string splines   = "ortho";
    const float ranksep         = 1.5;

    // node parameter
    const std::string shape         = "record";
    const std::string colorscheme   = "spectral9";
    const int num_colors            = 9;

public:

    bool open();

    void close();

    // void add_node(Node* node);

    // void add_nodes(std::vector<Node*>& nodes);

    void read_data(AllData& alldata);

    void print();

private:

    // print node to dot file
    void print_node(Node& node);

    int get_node_color(const double time);

    // void filter_nodes();

    // mark top x nodes, which took the most time
    void top_nodes();

    // mark predecessor nodes to be printed
    void mark_predecessors(Node* node);
};
#endif