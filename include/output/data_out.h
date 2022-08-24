/*
Reads all data( call_path_tree, definitons, etc. ) and outputs it into a jsonfile using RapidJson.
When changing names, syntax or adding/removing properties jsonreader needs to be updated as well.

*/
#ifndef DATA_OUT_H
#define DATA_OUT_H

#include "all_data.h"
//#include "data_tree.h"

#include <memory>
#include "definitions.h"

template <typename Writer>
void display(Writer& writer, AllData alldata);

template <typename Writer>
void display_definitions(AllData alldata, Writer& writer);

template <typename Writer>
void display_data_tree(AllData alldata, Writer& writer);

template <typename Writer>
void display_meta_data(AllData alldata, Writer& writer);

template <typename Writer>
void display_params(AllData alldata, Writer& writer);

template <typename Writer>
void display_system_node(std::shared_ptr<definitions::SystemTree::SystemNode> node, AllData alldata, Writer& writer);

template <typename Writer>
void display_system_tree(AllData alldata, Writer& writer);

bool DataOut(AllData& alldata);

#endif