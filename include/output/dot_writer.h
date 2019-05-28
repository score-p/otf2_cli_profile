#ifndef DOT_WRITER_H
#define DOT_WRITER_H

#include "all_data.h"
#include "create_dot.h"
#include <algorithm>

class Dot_writer{
public:
    Dot_writer(Params& params): params(params)
    {}
private:
    std::ofstream result_file;
    Params params;
    Data data;
    std::vector<Node*> nodes;

    // metaData
    double min_time = std::numeric_limits<uint64_t>::max();
    double max_time = 0;
    double timerange = 0;
    double total_time = 0;

    // graph parameter
    const std::string splines = "ortho";
    const float ranksep = 1.5;

    // node parameter
    const std::string shape = "record";
    const std::string colorscheme = "spectral9";
    const int num_colors = 9;

public:

    bool open(std::string filename);

    void close();

    void add_node(Node* node);

    void print();

private:

    // gathers global min, max, total time
    void gather_meta();

    // print node to dot file
    void print_node(Node& node);

    int get_node_color(const double time);

    // filter nodes
    void filter();

    // mark top x nodes, which took the most time
    void top_nodes();
    
    // mark predecessor nodes to be printed
    void mark_predecessors(Node* node);

};

#endif