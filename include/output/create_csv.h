#ifndef CREATE_CSV_H
#define CREATE_CSV_H

#include "all_data.h"
#include "data_tree.h"
#include "data_output.h"

Data_map read_data(AllData& alldata);

void _create_CSV(Data_map& data);

bool create_CSV(AllData& alldata);

#endif
