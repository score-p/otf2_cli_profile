/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef DATA_TREE_H
#define DATA_TREE_H

#include "main_structs.h"

#include <deque>
#include <memory>
#include <tuple>

class tree_iter;
class tree_node;

class data_tree {
   public:
    data_tree();

    data_tree(std::map<uint64_t, std::tuple<uint64_t, uint64_t, std::shared_ptr<tree_node>>>& mapping);

    tree_node* insert_node(uint64_t function_id, tree_node* parent);
    std::shared_ptr<tree_node> insert_node(uint64_t function_id, std::shared_ptr<tree_node> parent);
    void insert_node(std::shared_ptr<tree_node> aNode);

    void merge_tree(data_tree& rhs_tree);
    void insert_sub_tree(std::shared_ptr<tree_node>& parent, std::shared_ptr<tree_node>& n_node);

    /* map< node,parent > */  // mapping darf !NICHT! unordered sein -> Reihenfolge ist wichtig!
    void serialize_data(std::map<uint64_t, std::pair<uint64_t, uint64_t>>&         mapping,
                        std::deque<std::tuple<uint64_t, uint64_t, FunctionData*>>& f_data,
                        std::deque<std::tuple<uint64_t, uint64_t, MessageData*>>&  m_data,
                        std::deque<std::tuple<uint64_t, uint64_t, CollopData*>>&   c_data,
                        std::deque<std::tuple<uint64_t, uint64_t, uint64_t, MetricData*>>& met_data);

    /* functionId , node* */
    std::map<uint64_t, std::shared_ptr<tree_node>> root_nodes;

    /* <rank, data> */

    // bringts irgendwie nicht -> aufteilung public/private

    tree_iter begin();
    tree_iter end();

   private:
    void merge_node(std::shared_ptr<tree_node>& lhs_node, std::shared_ptr<tree_node>& rhs_node);
};

class tree_node {
    // TODO message map needs peer information in real version -> nicht wikrlich sofern wir nicht
    // zumindest nen teil-message-matching betreiben wollen (min. matching: process x hat n
    // nachichten an y gesendet mit m bytes volumen - dabei muss nur entweder sender oder
    // receiver seite vorhanden sein)
    // -> ist für circos wenn überhaupt wichtig -> links von x zu y usw.
   public:
    tree_node(const uint64_t _function_id, tree_node* _parent);
    tree_node(const uint64_t _function_id, const std::shared_ptr<tree_node>& _parent);
    tree_node(const uint64_t _function_id, const std::shared_ptr<tree_node>& _parent, const uint64_t process_num);
    tree_node(const uint64_t _function_id);

    ~tree_node();

    void add_data(const uint64_t location_id, const FunctionData& fdata);
    void add_data(const uint64_t location_id, const MessageData& mdata);
    void add_data(const uint64_t location_id, const CollopData& cdata);
    void add_data(const uint64_t location_id, const uint64_t metric_id, const MetricData& metdata);

    // std::shared_ptr<tree_node> parent;
    tree_node* parent;

    uint64_t function_id;

    /* function_id, pointer to node */
    std::map<uint64_t, std::shared_ptr<tree_node>> children;

    /* data containers */
    /* maps with location id as key -> value = NodeData -> function, p2p or collop */
    std::map<uint64_t, NodeData> node_data;

    std::map<uint64_t, MessageData*> have_message;  // TODO raus damit -> sinnlos und fehleranfällig
    std::map<uint64_t, CollopData*> have_collop;    // TODO raus damit -> sinnlos und fehleranfällig
    // TODO workaround
    //--->TODO funktion implementieren die aus node_data heraus findet ob collop bzw p2p da ist -> umständlich
    bool has_p2p    = false;
    bool has_collop = false;

    uint64_t  last_loc;
    NodeData* last_data;
};

class tree_iter {
   public:
    tree_iter(data_tree& _tree) {
        node_ptr = _tree.root_nodes.begin()->second.get();
        tree_ptr = &_tree;
    };

    tree_iter(tree_node* _rhs_node, data_tree* _rhs_tree) : node_ptr(_rhs_node), tree_ptr(_rhs_tree){};

    tree_iter(const tree_iter& _rhs_it) : node_ptr(_rhs_it.node_ptr), tree_ptr(_rhs_it.tree_ptr){};

    tree_node* get() { return node_ptr; }

    tree_iter operator=(tree_iter& _rhs_it) {
        node_ptr = _rhs_it.node_ptr;
        tree_ptr = _rhs_it.tree_ptr;
    };

    tree_node& operator*() { return *node_ptr; };

    tree_iter& operator++() {
        assert((node_ptr != nullptr) || (tree_ptr != nullptr));

        if (!node_ptr->children.empty()) {
            node_ptr = node_ptr->children.begin()->second.get();

        } else {
            while (true) {
                if (node_ptr->parent != nullptr) {
                    auto child_it = node_ptr->parent->children.find(node_ptr->function_id);

                    ++child_it;

                    if (child_it == node_ptr->parent->children.end()) {
                        node_ptr = node_ptr->parent;

                        continue;
                    }

                    node_ptr = child_it->second.get();

                    break;

                } else {
                    auto child_it = tree_ptr->root_nodes.find(node_ptr->function_id);

                    ++child_it;

                    if (child_it != tree_ptr->root_nodes.end()) {
                        node_ptr = child_it->second.get();

                    } else {
                        node_ptr = nullptr;
                        tree_ptr = nullptr;
                    }

                    break;
                }
            }
        }

        return *this;
    };

    tree_iter operator++(int) {
        tree_iter it(node_ptr, tree_ptr);

        ++*this;

        return it;
    };

    bool operator==(const tree_iter& _rhs_it) {
        return ((node_ptr == _rhs_it.node_ptr) && (tree_ptr == _rhs_it.tree_ptr));
    };

    bool operator!=(const tree_iter& _rhs_it) {
        return ((node_ptr != _rhs_it.node_ptr) || (tree_ptr != _rhs_it.tree_ptr));
    };

    tree_node* operator->() { return node_ptr; }

   protected:
    tree_node* node_ptr;
    data_tree* tree_ptr;
};

#endif
