/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#include <fstream>
#include <map>

#include <Cube.h>

#include "create_cube.h"
//TODO für tests
#include <iostream>

using namespace std;

using SystemNode_t = typename definitions::SystemTree::SystemNode_t;

bool CreateCube(AllData& alldata) {
    if (alldata.metaData.myRank != 0 /*&& !alldata.params.no_reduce*/) {
        return true;
    } /* else if ( alldata.params.no_reduce ) { //TODO no_reduce für ausgabe von teil-cubes?

             StartMeasurement(alldata, 1, true, "produce cube output");

         }*/

    alldata.verbosePrint(1, true, "producing cube output");

    cube::Cube cube_out;

    map<SystemNode_t*, cube::Node*>    MapCubeNodes;
    map<SystemNode_t*, cube::Process*> MapCubeProcesses;
    map<SystemNode_t*, cube::Thread*>  MapCubeThreads;

    map<uint64_t, cube::Metric*> MapCubeMetrics;
    map<uint64_t, cube::Region*> MapCubeRegions;
    map<tree_node*, cube::Cnode*> MapCubeCnodes;

    bool have_p2p    = false;
    bool have_collop = false;

    map<uint64_t, uint64_t> paradigmToCubeMetric_occ;
    map<uint64_t, uint64_t> paradigmToCubeMetric_time;
    map<uint64_t, uint64_t> metricToCubeMetric;
    uint64_t p2p_start;
    uint64_t collop_start;

    // systemtree engage!
    string name, class_name;

    for( auto it = alldata.definitions.system_tree.begin(); it != alldata.definitions.system_tree.end(); ++it ) {
        auto* node = &(*it);
        auto& data = it->data;
        switch(it->data.class_id) {
            case definitions::SystemClass::LOCATION:
                MapCubeThreads[node] = cube_out.def_location(
                    data.name, data.node_id, cube::CUBE_LOCATION_TYPE_CPU_THREAD, MapCubeProcesses[it->parent]);
                break;
            case definitions::SystemClass::MACHINE:
                MapCubeNodes[node] = cube_out.def_mach(data.name, "");
                break;
            case definitions::SystemClass::LOCATION_GROUP:
                MapCubeProcesses[node] = cube_out.def_location_group(
                    data.name, data.node_id, cube::CUBE_LOCATION_GROUP_TYPE_PROCESS,
                    MapCubeNodes[it->parent]);
                break;
            default:
            MapCubeNodes[node] = cube_out.def_system_tree_node(data.name, "", "node", MapCubeNodes[it->parent]);
                break;
        }
    }
    // systemtree end

    // cube-metrics engage!
    // implizite annahme dass function_data da ist (IMMER)
    MapCubeMetrics[0] = cube_out.def_met("Visits", "met_visits", "UINT64", "occ", "", "",
                                         "display function occurrence for each functiongroup", NULL,
                                         cube::CUBE_METRIC_EXCLUSIVE);

    MapCubeMetrics[1] = cube_out.def_met("Time", "met_time", "DOUBLE", "sec", "", "",
                                         "display function exclusive time for each functiongroup",
                                         NULL, cube::CUBE_METRIC_EXCLUSIVE);

    string met_visits = "Met_Visits";
    string met_time   = "Met_Time";

    for( const auto& paradigm : alldata.definitions.paradigms.get_all() ) {
        auto c_met_ref =
            paradigmToCubeMetric_occ.insert(make_pair(paradigm.first, MapCubeMetrics.size()))
                .first->second;

        MapCubeMetrics[c_met_ref] =
            cube_out.def_met(paradigm.second.name, paradigm.second.name, "UINT64", "occ", "", "", "",
                             MapCubeMetrics.find(0)->second, cube::CUBE_METRIC_EXCLUSIVE);

        c_met_ref = paradigmToCubeMetric_time.insert(make_pair(paradigm.first, MapCubeMetrics.size()))
                        .first->second;

        MapCubeMetrics[c_met_ref] =
            cube_out.def_met(paradigm.second.name, paradigm.second.name, "DOUBLE", "sec", "", "", "",
                             MapCubeMetrics.find(1)->second, cube::CUBE_METRIC_EXCLUSIVE);
    }

    if (alldata.params.read_metrics) {

        string aType = "";

        for( const auto& metric : alldata.definitions.metrics.get_all() ) {
            if( metric.second.allowed ) {

                if(metric.second.type == MetricDataType::UINT64) {
                    aType = "UINT64";
                } else if (metric.second.type == MetricDataType::DOUBLE) {
                    aType = "DOUBLE";
                } else if (metric.second.type == MetricDataType::INT64) {
                    aType = "INT64";
                }

                auto c_met_ref = metricToCubeMetric.insert(make_pair(metric.first, MapCubeMetrics.size()))
                                     .first->second;
                MapCubeMetrics[c_met_ref] =
                    cube_out.def_met(metric.second.name, metric.second.name, aType, metric.second.unit, "", "",
                                     metric.second.description, NULL, cube::CUBE_METRIC_EXCLUSIVE);
            }
        }
    }
    // cube-metrics end!

    // create all the regions
    for( auto& region : alldata.definitions.regions.get_all() ) {
        MapCubeRegions[region.first] = cube_out.def_region(
            region.second.name, region.second.name, "", "", region.second.source_line, 0, "", "", region.second.file_name);
    }
    // stop it!

    // create call path nodes -- trricky
    for (auto it = alldata.call_path_tree.begin(); it != alldata.call_path_tree.end(); ++it) {
        if (it->parent != nullptr) {
            MapCubeCnodes[it.get()] =
                cube_out.def_cnode(MapCubeRegions.find(it->function_id)->second, "", 0,
                                   MapCubeCnodes.find(it->parent)->second);

        } else {
            MapCubeCnodes[it.get()] =
                cube_out.def_cnode(MapCubeRegions.find(it->function_id)->second, "", 0, NULL);
        }

        if (!have_p2p) {
            if (it->has_p2p == true) {
                p2p_start = MapCubeMetrics.size();

                MapCubeMetrics[MapCubeMetrics.size()] =
                    cube_out.def_met("P2P Communication sent", "met_p2psendcomm", "UINT64", "occ",
                                     "", "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] =
                    cube_out.def_met("P2P Communication received", "met_p2precvcomm", "UINT64",
                                     "occ", "", "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] =
                    cube_out.def_met("P2P Bytes sent", "met_p2pbytessend", "UINT64", "Bytes", "",
                                     "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] =
                    cube_out.def_met("P2P Bytes received", "met_p2pbytesrecv", "UINT64", "Bytes",
                                     "", "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                have_p2p = true;
            }
        }

        if (!have_collop) {
            if (it->has_collop == true) {
                collop_start = MapCubeMetrics.size();

                MapCubeMetrics[MapCubeMetrics.size()] = cube_out.def_met(
                    "Collective Communication Bytes sent", "met_collopbytesout", "UINT64", "Bytes",
                    "", "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] = cube_out.def_met(
                    "Collective Communication Bytes received", "met_collopbytesin", "UINT64",
                    "Bytes", "", "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] =
                    cube_out.def_met("Collective Communication", "met_collopcomm_sum", "UINT64",
                                     "occ", "", "", "", NULL, cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] = cube_out.def_met(
                    "Collective Communication sent (occ)", "met_collopcomm_send", "UINT64", "occ",
                    "", "", "", MapCubeMetrics.find(collop_start + 2)->second,
                    cube::CUBE_METRIC_EXCLUSIVE);

                MapCubeMetrics[MapCubeMetrics.size()] = cube_out.def_met(
                    "Collective Communication received (occ)", "met_collopcomm_recv", "UINT64",
                    "occ", "", "", "", MapCubeMetrics.find(collop_start + 2)->second,
                    cube::CUBE_METRIC_EXCLUSIVE);

                have_collop = true;
            }
        }
    }

#if CUBE_REVISION_NUMBER >= 14755

    // initialize ist needed since rev 14755 (4.3.4)
    // needs to be initialised before nodes/metrics are filled with data -> else "Something is wrong
    // with ..." failure
    cube_out.initialize();

#endif

    // fill metrics!
    for (auto it = alldata.call_path_tree.begin(); it != alldata.call_path_tree.end(); ++it) {
        cube::Cnode*  tmp_cnode;
        cube::Thread* tmp_thread;

        string   met_string;
        auto* region = alldata.definitions.regions.get( it->function_id );
        if(region == nullptr) {
          std::cerr << "funtion id " << it->function_id << " not found (" << __FILE__
                    << ":" << __LINE__ << ")";
          continue;
        }
        uint64_t para_id = region->paradigm_id;
        uint64_t id = 0;
        tmp_cnode = MapCubeCnodes.find(it.get())->second;

        for( const auto& it_data :it->node_data ) {
            auto* location = alldata.definitions.system_tree.location(it_data.first);
            if(location == nullptr) {
              cerr << "Cube Output: system location not found: " << it_data.first << endl;
              continue;
            }
            tmp_thread = MapCubeThreads.find(location)->second;

            // function data
            cube_out.set_sev(MapCubeMetrics.find(0)->second, tmp_cnode, tmp_thread,
                             it_data.second.f_data.count);

            id = paradigmToCubeMetric_occ.find(para_id)->second;

            cube_out.set_sev(MapCubeMetrics.find(id)->second, tmp_cnode, tmp_thread,
                             it_data.second.f_data.count);

            cube_out.set_sev(MapCubeMetrics.find(1)->second, tmp_cnode, tmp_thread,
                             (double)it_data.second.f_data.excl_time /
                                 (double)alldata.metaData.timerResolution);

            id = paradigmToCubeMetric_time.find(para_id)->second;

            cube_out.set_sev(MapCubeMetrics.find(id)->second, tmp_cnode, tmp_thread,
                             (double)it_data.second.f_data.excl_time /
                                 (double)alldata.metaData.timerResolution);

            // message data
            if (it_data.second.m_data.count_send > 0) {
                cube_out.set_sev(MapCubeMetrics.find(p2p_start)->second, tmp_cnode, tmp_thread,
                                 it_data.second.m_data.count_send);

                cube_out.set_sev(MapCubeMetrics.find(p2p_start + 2)->second, tmp_cnode, tmp_thread,
                                 it_data.second.m_data.bytes_send);
            }

            if (it_data.second.m_data.count_recv > 0) {
                cube_out.set_sev(MapCubeMetrics.find(p2p_start + 1)->second, tmp_cnode, tmp_thread,
                                 it_data.second.m_data.count_recv);

                cube_out.set_sev(MapCubeMetrics.find(p2p_start + 3)->second, tmp_cnode, tmp_thread,
                                 it_data.second.m_data.bytes_recv);
            }

            // collop
            uint64_t sum = 0;

            if (it_data.second.c_data.count_send > 0) {
                cube_out.set_sev(MapCubeMetrics.find(collop_start + 3)->second, tmp_cnode,
                                 tmp_thread, it_data.second.c_data.count_send);

                sum += it_data.second.c_data.count_send;

                cube_out.set_sev(MapCubeMetrics.find(collop_start)->second, tmp_cnode, tmp_thread,
                                 it_data.second.c_data.bytes_send);
            }

            if (it_data.second.c_data.count_recv > 0) {
                cube_out.set_sev(MapCubeMetrics.find(collop_start + 4)->second, tmp_cnode,
                                 tmp_thread, it_data.second.c_data.count_recv);

                sum += it_data.second.c_data.count_recv;

                cube_out.set_sev(MapCubeMetrics.find(collop_start + 1)->second, tmp_cnode,
                                 tmp_thread, it_data.second.c_data.bytes_recv);
            }

            if( sum > 0 ) {

                cube_out.set_sev(MapCubeMetrics.find(collop_start + 2)->second, tmp_cnode, tmp_thread,
                             sum);

            }

            if (!it_data.second.metrics.empty()) {

                for( auto it_met : it_data.second.metrics ) {
                    auto& metric = it_met.second;
                    auto* cube_metric =
                        MapCubeMetrics.find(metricToCubeMetric.find(it_met.first)->second)->second;
                    switch (metric.type) {
                        case MetricDataType::UINT64:
                            cube_out.set_sev(cube_metric, tmp_cnode, tmp_thread,
                                             (uint64_t)metric.data_excl);
                            break;
                        case MetricDataType::INT64:
                            cube_out.set_sev(cube_metric, tmp_cnode, tmp_thread,
                                             (int64_t)metric.data_excl);
                            break;
                        case MetricDataType::DOUBLE:
                            cube_out.set_sev(cube_metric, tmp_cnode, tmp_thread,
                                             (double)metric.data_excl);
                            break;
                    }
                }
            }
        }
    }

    string   fname = alldata.params.output_file_prefix;
    ofstream out;
    /* TODO no_reduce fixen/implementieren
        if( alldata.metaData.params.no_reduce ) {

            ostringstream o;
            o << "_" << alldata.metaData.myRank;

            fname += o.str();

        }
    */
    cube_out.writeCubeReport(fname);

    return true;
}
