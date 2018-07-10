/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef REDUCE_DATA_H
#define REDUCE_DATA_H

#include "all_data.h"

/* reduce the data to the master process */
bool ReduceData(AllData& alldata);

/* reduce the dispersion data to the master process */
bool ReduceDataDispersion(AllData& alldata);

#endif /* REDUCE_DATA_H */
