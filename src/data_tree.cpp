/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#include "data_tree.h"

#include <stack>

#include <bits/utility.h>
#include <cstddef>
#include <type_traits>

using namespace std;

data_tree::data_tree() {}

// generating a tree out of a mapping sent with MPI - ReduceData - (has no data inside a tree node)
data_tree::data_tree(map<uint64_t, tuple<uint64_t, uint64_t, shared_ptr<tree_node>>>& mapping) {
    shared_ptr<tree_node> tmp_node;

    for (auto it = mapping.begin(); it != mapping.end(); it++) {
        if (get<1>(it->second) != (uint64_t)-1) {
            tmp_node = insert_node(get<0>(it->second), get<2>(mapping.find(get<1>(it->second))->second));

        } else {
            tmp_node = insert_node(get<0>(it->second), (shared_ptr<tree_node>)nullptr);
        }

        get<2>(it->second) = tmp_node;
    }
}

// add a node without data
shared_ptr<tree_node> data_tree::insert_node(uint64_t function_id, shared_ptr<tree_node> parent) {
    if (parent == nullptr) {
        shared_ptr<tree_node> tmp = std::make_shared<tree_node>(function_id, parent);

        root_nodes.insert(make_pair(function_id, tmp));

        return tmp;

    } else {
        auto node = parent->children.find(function_id);

        if (node == parent->children.end()) {
            shared_ptr<tree_node> tmp = std::make_shared<tree_node>(function_id, parent);

            parent->children.insert(node, make_pair(function_id, tmp));

            return tmp;

        } else {
            return nullptr;
        }
    }
}

// same as above with different intake and return
tree_node* data_tree::insert_node(uint64_t function_id, tree_node* parent) {
    if (parent == nullptr) {
        shared_ptr<tree_node> tmp = std::make_shared<tree_node>(function_id, parent);

        root_nodes.insert(make_pair(function_id, tmp));

        return tmp.get();

    } else {
        auto node = parent->children.find(function_id);

        if (node == parent->children.end()) {
            shared_ptr<tree_node> tmp = std::make_shared<tree_node>(function_id, parent);

            parent->children.insert(make_pair(function_id, tmp));

            return tmp.get();

        } else {
            return nullptr;
        }
    }
}

// copy a node into the tree - with it's data
void data_tree::insert_node(shared_ptr<tree_node> aNode) {
    if (aNode->parent == NULL) {
        root_nodes.insert(make_pair(aNode->function_id, aNode));
    } else {
        auto node = aNode->parent->children.find(aNode->function_id);

        if (node == aNode->parent->children.end()) {
            aNode->parent->children.insert(make_pair(aNode->function_id, aNode));
        }
    }
}

// merge a (temporary) tree into "main" tree
void data_tree::merge_tree(data_tree& rhs_tree) {
    for (auto it : rhs_tree.root_nodes) {
        auto lhs_node = root_nodes.find(it.first);

        if (lhs_node == root_nodes.end()) {
            root_nodes.insert(it);

        } else {
            merge_node(lhs_node->second, it.second);
        }
    }
}

// merge two nodes -- copying rhs_node's content into lhs's
// should only be used if one knows that the data inside a node is unique (location wise)
void data_tree::merge_node(shared_ptr<tree_node>& lhs_node, shared_ptr<tree_node>& rhs_node) {
    for (auto it : rhs_node->node_data) {
        lhs_node->node_data.insert(it);
    }

    lhs_node->have_collop.insert(rhs_node->have_collop.begin(), rhs_node->have_collop.end());
    lhs_node->have_message.insert(rhs_node->have_message.begin(), rhs_node->have_message.end());

    // test for children == children
    for (auto it : rhs_node->children) {
        auto lhs_node_o = lhs_node->children.find(it.first);

        if (lhs_node_o != lhs_node->children.end()) {
            merge_node(lhs_node_o->second, it.second);
        } else {
            insert_sub_tree(lhs_node, it.second);
        }
    }
}

// TODO zu geringe funktionalität? -> benötigen wir es gesondert?
void data_tree::insert_sub_tree(shared_ptr<tree_node>& parent, shared_ptr<tree_node>& n_node) {
    n_node->parent = parent.get();  // shared_ptr<tree_node>(parent);

    parent->children.insert(make_pair(n_node->function_id, n_node));
}

/*
    TODO ist aufgebläht -> wird schlimmer um so mehr daten kommen -> andere implementation möglich?
    ohne die kombinierte funktionalität zu zerfasern
    -> evtl neuimplementation mit iterrator
*/
// iterate through the tree, generate a mapping and fill the given data deques
// for communication via MPI
// only getting called by serialize_data - not intended for usage on it's own
void getting_serial(map<uint64_t, pair<uint64_t, uint64_t>>&                 mapping,
                    deque<tuple<uint64_t, uint64_t, FunctionData*>>&         f_data,
                    deque<tuple<uint64_t, uint64_t, MessageData*>>&          m_data,
                    deque<tuple<uint64_t, uint64_t, CollopData*>>&           c_data,
                    deque<tuple<uint64_t, uint64_t, uint64_t, MetricData*>>& met_data, shared_ptr<tree_node>& aNode,
                    uint64_t& counter, stack<uint64_t>& node_stack) {
    if (!node_stack.empty()) {
        // insert as common node
        mapping.insert(make_pair(counter, make_pair(aNode->function_id, node_stack.top())));
        node_stack.push(counter);

    } else {
        // insert as root node
        mapping.insert(make_pair(counter, make_pair(aNode->function_id, (uint64_t)-1)));
        node_stack.push(counter);
    }

    // add all the data \o/
    for (auto it = aNode->node_data.begin(); it != aNode->node_data.end(); ++it) {
        f_data.push_back(make_tuple(counter, it->first, &it->second.f_data));

        if (aNode->has_p2p) {
            m_data.push_back(make_tuple(counter, it->first, &it->second.m_data));
        }

        if (aNode->has_collop) {
            c_data.push_back(make_tuple(counter, it->first, &it->second.c_data));
        }

        for (auto it_metric = it->second.metrics.begin(); it_metric != it->second.metrics.end(); ++it_metric)
            met_data.push_back(make_tuple(counter, it->first, it_metric->first, &it_metric->second));
    }
    // counter works as an improvised node id
    counter++;

    // recursive call
    for (auto it = aNode->children.begin(); it != aNode->children.end(); it++) {
        getting_serial(mapping, f_data, m_data, c_data, met_data, it->second, counter, node_stack);
    }

    node_stack.pop();
}

// function to serialize data for the MPI communication
void data_tree::serialize_data(map<uint64_t, pair<uint64_t, uint64_t>>&                 mapping,
                               deque<tuple<uint64_t, uint64_t, FunctionData*>>&         f_data,
                               deque<tuple<uint64_t, uint64_t, MessageData*>>&          m_data,
                               deque<tuple<uint64_t, uint64_t, CollopData*>>&           c_data,
                               deque<tuple<uint64_t, uint64_t, uint64_t, MetricData*>>& met_data) {
    stack<uint64_t> node_stack;
    uint64_t        counter = 0;  //<- gibt die node_id an die sonst nicht existiert, sie ist für das
                                  // mapping allerdings wichtig -> reduce-Schritt

    auto it   = root_nodes.begin();
    auto it_e = root_nodes.end();

    for (; it != it_e; it++) {
        getting_serial(mapping, f_data, m_data, c_data, met_data, it->second, counter, node_stack);
    }
}

tree_iter data_tree::begin() {
    if (this != nullptr && !root_nodes.empty()) {
        return tree_iter(*this);
    } else {
        return tree_iter(nullptr, nullptr);
    }
}

tree_iter data_tree::end() {
    tree_iter it(nullptr, nullptr);

    return it;
}

tree_node::tree_node(const uint64_t _function_id, tree_node* _parent)
    : function_id(_function_id),
      parent(_parent),
      last_loc((uint64_t)-1),
      last_data(nullptr),
      has_p2p(false),
      has_collop(false) {}

tree_node::tree_node(const uint64_t _function_id, const shared_ptr<tree_node>& _parent)
    : function_id(_function_id),
      parent(_parent.get()),
      last_loc((uint64_t)-1),
      last_data(nullptr),
      has_p2p(false),
      has_collop(false) {}

tree_node::tree_node(const uint64_t _function_id, const shared_ptr<tree_node>& _parent, const uint64_t process_num)
    : function_id(_function_id),
      parent(_parent.get()),
      last_loc((uint64_t)-1),
      last_data(nullptr),
      has_p2p(false),
      has_collop(false) {}

tree_node::tree_node(const uint64_t _function_id)
    : function_id(_function_id),
      parent(nullptr),
      last_loc((uint64_t)-1),
      last_data(nullptr),
      has_p2p(false),
      has_collop(false) {}

tree_node::~tree_node() {}

// adding data to call path node and saving the location_id and a pointer to the last used section
// to speed up repeatedly acces to data of the same location (useful on location/stream wise reading of traces
void tree_node::add_data(const uint64_t location_id, const FunctionData& fdata) {
    if (last_loc == location_id) {
        last_data->f_data += fdata;
    } else {
        last_loc = location_id;

        auto it = node_data.find(location_id);

        if (it != node_data.end()) {
            it->second.f_data += fdata;

            last_data = &it->second;

        } else {
            last_data = &node_data.insert(make_pair(location_id, fdata)).first->second;
        }
    }
}

void tree_node::add_data(const uint64_t location_id, const MessageData& mdata) {
    if (last_loc == location_id) {
        last_data->m_data += mdata;
    } else {
        last_loc = location_id;

        auto it = node_data.find(location_id);

        if (it != node_data.end()) {
            it->second.m_data += mdata;

            last_data = &it->second;

        } else {
            last_data = &node_data.insert(make_pair(location_id, mdata)).first->second;
        }
    }

    have_message.insert(make_pair(location_id, &last_data->m_data));
    // TODO workaround
    has_p2p = true;
}

void tree_node::add_data(const uint64_t location_id, const CollopData& cdata) {
    if (last_loc == location_id) {
        last_data->c_data += cdata;
    } else {
        last_loc = location_id;

        auto it = node_data.find(location_id);

        if (it != node_data.end()) {
            it->second.c_data += cdata;

            last_data = &it->second;

        } else {
            last_data = &node_data.insert(make_pair(location_id, cdata)).first->second;
        }
    }

    have_collop.insert(make_pair(location_id, &last_data->c_data));
    // TODO workaround
    has_collop = true;
}

void tree_node::add_data(const uint64_t location_id, const uint64_t metric_id, const MetricData& metdata) {
    if (last_loc != location_id) {
        last_loc = location_id;

        auto it = node_data.find(location_id);

        if (it != node_data.end())
            last_data = &it->second;
        else
            last_data = &node_data.insert(make_pair(location_id, NodeData{})).first->second;
    }

    last_data->metrics[metric_id] = metdata;
}
