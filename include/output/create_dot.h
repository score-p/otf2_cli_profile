#ifndef CREATE_DOT_H
#define CREATE_DOT_H

#include <fstream>
#include <string>
#include <stack>
#include <algorithm>

#include "all_data.h"

bool CreateDot(AllData& alldata);

enum NodeState
{
    full, partial, dontprint, printed
};

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

    NodeState state = NodeState::dontprint;

};

typedef std::vector<Node*> Data;

#endif

