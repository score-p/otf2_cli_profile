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

    //metaData
    double min_time = std::numeric_limits<uint64_t>::max();
    double max_time = 0;
    double timerange = 0;
    std::map<uint32_t, Node*> printed_nodes;

    // graph parameter
    const std::string splines = "ortho";
    const float ranksep = 1.5;

    // node parameter
    const std::string shape = "record";
    const std::string colorscheme = "spectral9";
    const int num_colors = 9;

    ////
    //methods
    void getMeta(Data& data);
  
    void print_predecessors( Node& region);

    void top_nodes(Data& data);

    void print_node(Node& region, bool full_node);
    
    void print_node();
    
    int node_color(const double time);
public:

    bool open(std::string filename);

    void close();

    void print(Data& data);
};

#endif