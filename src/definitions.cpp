/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#include "definitions.h"
namespace definitions {

SystemIterator SystemTree::begin() const { return SystemIterator(root); }

SystemIterator SystemTree::end() const { return SystemIterator(root, nullptr); }

SystemTree* SystemTree::copy_reduced(const SystemTree& sys_tree, uint32_t level) {
    auto* n_tree = new SystemTree();

    for (auto it = sys_tree.begin(); it != sys_tree.end(); ++it) {
        if (level >= it->data.level)
            n_tree->insert_node(*it);
        else {
            auto* parent = it->parent;
            while (parent->data.level > level)
                parent = parent->parent;  // TODO parent-ception

            auto parent_loc_id = parent->data.location_id;
            switch (it->data.class_id) {
                case SystemClass::LOCATION:
                    n_tree->locations.insert(
                        std::make_pair(it->data.location_id, n_tree->location_grps[parent_loc_id]));
                    break;
                case definitions::SystemClass::LOCATION_GROUP:
                    n_tree->location_grps.push_back(n_tree->system_nodes[parent_loc_id]);
                    break;

                default:
                    n_tree->system_nodes.push_back(n_tree->system_nodes[parent_loc_id]);
                    break;
            }
        }
    }

    return n_tree;
}
}
