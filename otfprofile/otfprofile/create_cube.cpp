#include <fstream>
#include <map>
#include <set>

#include <cube/Cube.h>

#include "otfprofile.h"
#include "create_cube.h"

using namespace std;

cube::Cube cube_out;


/* store Cube Data */

map< uint64_t , cube::Machine* > MapCubeMachines;   
map< uint64_t , cube::Node*    > MapCubeNodes; 
map< uint64_t , cube::Process* > MapCubeProcesses;
map< uint64_t , cube::Thread*  > MapCubeThreads;

map< string   , cube::Metric*  > MapCubeMetrics;
map< uint64_t , cube::Region*  > MapCubeRegions;
map< uint64_t , cube::Cnode*   > MapCubeCnodes;


/* create Systemtree and maps containing all systemtreenodes, processes and threads 
   as a reference to cube::Node*, cube::Process* and cube::Thread*
   as well as cube::Machine* */
   
bool CreateSystemtree(AllData& alldata) {
    
    string name, class_name;//name_machine, name_node, name_locationgroup, name_location;
    
    uint64_t parentID,ID, id;//NodeID, ProcID, LocationID;
    
    map< uint64_t, SystemTreeNode >::iterator it = alldata.systemTreeIdToSystemTreeNodeMap.begin();
    map< uint64_t, SystemTreeNode >::iterator it_e = alldata.systemTreeIdToSystemTreeNodeMap.end();

    for(; it != it_e; it++) {

            switch ( it->second.class_id ) {
                case SYSTEMTREE_LOCATION:

                        name                          = it->second.name;
                        parentID                      = it->second.parent;
                        id                            = it->second.location;

                        MapCubeThreads[ id ]          = cube_out.def_location( name, id, cube::CUBE_LOCATION_TYPE_CPU_THREAD, MapCubeProcesses[ parentID ] );

                        break;

                case SYSTEMTREE_LOCATION_GROUP:

                        name                          = it->second.name;
                        parentID                      = it->second.parent;

                        MapCubeProcesses[ it->first ] = cube_out.def_location_group( name, it->first, cube::CUBE_LOCATION_GROUP_TYPE_PROCESS, MapCubeNodes[ parentID ] );
                        break;

                case SYSTEMTREE_OTHER:          /*FALLTHRU*/
                case SYSTEMTREE_BLADE:          /*FALLTHRU*/
                case SYSTEMTREE_CAGE:           /*FALLTHRU*/
                case SYSTEMTREE_CABINET:        /*FALLTHRU*/
                case SYSTEMTREE_CABINET_ROW:    /*FALLTHRU*/
                case SYSTEMTREE_NODE:   

                        name                          = it->second.name;
                        parentID                      = it->second.parent;

                        MapCubeNodes[ it->first ]     = cube_out.def_system_tree_node( name, "", "node", MapCubeNodes[ parentID ] );
                        

                        break;

                case SYSTEMTREE_MACHINE:

                        MapCubeNodes[ it->first ]     = cube_out.def_mach( it->second.name, "");

                        break;

                default:

                        break;

            }

    }

    return true;
}

/* create MetricTree and save the data into MapCubeMetrics */
bool CreateMetrics(AllData& alldata) {

    MapCubeMetrics["Met_Visits"]        = cube_out.def_met("Visits" , "met_visits", "UINT64", "occ", "", "",
                                                           "display function occurrence for each functiongroup" ,  
                                                           NULL, cube::CUBE_METRIC_EXCLUSIVE ); 

    MapCubeMetrics["Met_Time"]          = cube_out.def_met("Time"   , "met_time"  , "DOUBLE" ,"sec", "", "", 
                                                           "display function exclusive time for each functiongroup", 
                                                           NULL, cube::CUBE_METRIC_EXCLUSIVE );
    
    for(map< uint64_t, string >::iterator it = alldata.functionGroupIdNameMap.begin();
        it != alldata.functionGroupIdNameMap.end();
        it++) {

        string met_visits = "Met_Visits" + it->second;
        string met_time   = "Met_Time" + it->second;

        MapCubeMetrics[met_visits]  = cube_out.def_met(it->second , it->second, "UINT64", "occ", "", "", 
                                                       "" , MapCubeMetrics.find("Met_Visits")->second, 
                                                       cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics[met_time]    = cube_out.def_met(it->second , it->second, "DOUBLE", "sec", "", "", 
                                                       "" , MapCubeMetrics.find("Met_Time")->second, 
                                                       cube::CUBE_METRIC_EXCLUSIVE );
    }

    if(alldata.P2PFuncIds.size() != 0) {

        MapCubeMetrics["Met_P2PSendComm"]         = cube_out.def_met("P2P Communication sent" , "met_p2psendcomm", "UINT64", 
                                                                     "occ", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_P2PRecvComm"]         = cube_out.def_met("P2P Communication received" , "met_p2precvcomm", "UINT64", 
                                                                     "occ", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_P2PBytesSend"]        = cube_out.def_met("P2P Bytes sent" , "met_p2pbytessend", "UINT64",
                                                                     "Bytes", "", "", "", NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_P2PBytesRecv"]        = cube_out.def_met("P2P Bytes received" , "met_p2pbytesrecv", "UINT64", 
                                                                     "Bytes", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );
    }

    if( alldata.CollopDataPerNode.size() > 0 ) {

        MapCubeMetrics["Met_CollOpBytesOut"]      = cube_out.def_met("Collective Communication Bytes sent" , "met_collopbytesout", "UINT64",
                                                                     "Bytes", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_CollOpBytesIn"]       = cube_out.def_met("Collective Communication Bytes received" , "met_collopbytesin", "UINT64",
                                                                     "Bytes", "", "", "" , NULL,        
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_CollOpComm_sum"]          = cube_out.def_met("Collective Communication" , "met_collopcomm_sum", "UINT64",
                                                                     "occ", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_CollOpComm_send"]          = cube_out.def_met("Collective Communication sent (occ)" , "met_collopcomm_send", "UINT64",
                                                                     "occ", "", "", "" , MapCubeMetrics.find( "Met_CollOpComm_sum" )->second, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_CollOpComm_recv"]          = cube_out.def_met("Collective Communication received (occ)" , "met_collopcomm_recv", "UINT64",
                                                                     "occ", "", "", "" , MapCubeMetrics.find( "Met_CollOpComm_sum" )->second, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

    }

    if( alldata.RmaMsgData.size() > 0 ) {

        MapCubeMetrics["Met_RmaGet"]      = cube_out.def_met("RMA get" , "met_rmaget", "UINT64",
                                                                     "occ", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_RmaPut"]      = cube_out.def_met("RMA put" , "met_rmaput", "UINT64",
                                                                     "occ", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_RmaGetBytes"]      = cube_out.def_met("RMA get" , "met_rmaget_bytes", "UINT64",
                                                                     "Bytes", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

        MapCubeMetrics["Met_RmaPutBytes"]      = cube_out.def_met("RMA put" , "met_rmaput_bytes", "UINT64",
                                                                     "Bytes", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );

    }

    /* debug metric -> writes node_ids as metrik */
/*
    MapCubeMetrics["NodeID"]          = cube_out.def_met("Debug: NodeId's" , "node_id", "UINT64",
                                                                     "occ", "", "", "" , NULL, 
                                                                     cube::CUBE_METRIC_EXCLUSIVE );
*/
    return true;

}

/* create Regions to the corresponding function and save the data into MapCubeRegions */
bool CreateRegions(AllData& alldata) {


    //RMA hack
    if( alldata.RmaMsgData.size() > 0 ) {

        MapCubeRegions[3153794382] = cube_out.def_region("RMA_VIRTUAL_GROUPING", "RMA_VIRTUAL_GROUPING", "", "", 0, 0, "", "", "");

    }

    /*only beginSCL and SCLFile are actually used*/

    long beginSCL = 0;
    long endSCL   = 0;
    
    string SCLFile = "";

    for(map< uint64_t, string >::iterator it_functionIdNameMap = alldata.functionIdNameMap.begin(); 
        it_functionIdNameMap != alldata.functionIdNameMap.end(); 
        it_functionIdNameMap++) {

		map<uint64_t, functionDetail>::iterator it_fDetails;

		if( (it_fDetails = alldata.functionDetails.find(it_functionIdNameMap->first)) != alldata.functionDetails.end() ) {

			beginSCL = alldata.functionDetails.find(it_functionIdNameMap->first)->second.beginLine;
			SCLFile = alldata.functionDetails.find(it_functionIdNameMap->first)->second.name;
			MapCubeRegions[it_functionIdNameMap->first] = cube_out.def_region(it_functionIdNameMap->second, it_functionIdNameMap->second, "", "", beginSCL, endSCL, "", "", SCLFile);

		}
		
          
	}

    return true;

}

/* create Cnodes for all functions which were called at least one time and save data into MapCubeCnodes */
void CreateNodes(AllData& alldata) {

    string mod = "";
    int line = 0;

	map<uint64_t, CallPathTreeNode >::iterator it = alldata.callPathTree.tree.begin();

    if( alldata.RmaMsgData.size() > 0 ) {

        MapCubeCnodes[(uint64_t) -1] = cube_out.def_cnode(MapCubeRegions.find(3153794382)->second, mod, line, NULL);

    }
	
	for(; it != alldata.callPathTree.tree.end(); it++) {

		if(it->second.parent_id == 0) {

			MapCubeCnodes[it->first] = cube_out.def_cnode(MapCubeRegions.find(it->second.function_id)->second, mod, line, NULL);

		} else {
			
			uint64_t p_nid = alldata.callPathTree.tree.find(it->second.parent_id)->first;
			MapCubeCnodes[it->first ] = cube_out.def_cnode(MapCubeRegions.find(it->second.function_id)->second, mod, line, MapCubeCnodes.find(p_nid)->second);

		}

	}


}

/* read existing maps and create cubex */
bool CreateCube(AllData& alldata) {

#ifndef OTFPROFILE_MPI

        StartMeasurement(alldata, 1, false, "produce cube output");

#endif

#ifdef OTFPROFILE_MPI        

        if( alldata.myRank != 0 && !alldata.params.no_reduce ) {         

			return true;

		} else if( alldata.myRank == 0 && !alldata.params.no_reduce ) {

            StartMeasurement(alldata, 1, false, "produce cube output");

        } else if ( alldata.params.no_reduce ) {

            StartMeasurement(alldata, 1, true, "produce cube output");

        }

#endif

    VerbosePrint(alldata, 1, true, "producing cube output\n");

        CreateSystemtree(alldata);    
        CreateMetrics(alldata);
        CreateRegions(alldata);
        CreateNodes(alldata);

        uint64_t     FuncGroupId;
        string       met_string;   
        
        cube::Cnode*  tmp_cnode;
        cube::Thread* tmp_thread;        
        
 /* insert values to the corresponding metrics */
    //Occ Time

    map< uint64_t, CallPathTreeNode>::iterator it = alldata.callPathTree.tree.begin();

    map< pair< uint64_t, uint64_t>, uint64_t>::iterator it_mapping = alldata.callPathTree.mapping.begin();

	for(; it != alldata.callPathTree.tree.end(); it++) {


        if(MapCubeCnodes.find(it->first) != MapCubeCnodes.end()){ 
        
            tmp_cnode = MapCubeCnodes.find(it->first)->second; 

        } else {

                continue;

        }

        for(; it_mapping != alldata.callPathTree.mapping.end(); it_mapping++) {

            if(it_mapping->second == it->first) {

                FuncGroupId = alldata.functionIdToGroupIdMap.find(  it->second.function_id )->second;
                met_string  = "Met_Visits" + alldata.functionGroupIdNameMap.find(FuncGroupId)->second;

            } else {

                continue;

            }
                

           if(MapCubeThreads.find(it_mapping->first.second) != MapCubeThreads.end()) {

                tmp_thread = MapCubeThreads.find(it_mapping->first.second)->second; 

            } else {

                continue;

            }    
                    
            cube_out.set_sev(MapCubeMetrics.find("Met_Visits")->second, tmp_cnode, tmp_thread, 
                alldata.functionDataPerCPTNode.find(
                pair< uint64_t, uint64_t >(it->first, it_mapping->first.second ) 
                )->second.count.cnt);
           

           cube_out.set_sev(MapCubeMetrics.find(met_string)->second, tmp_cnode, tmp_thread,
                alldata.functionDataPerCPTNode.find(
                pair< uint64_t, uint64_t >(it->first, it_mapping->first.second )
                )->second.count.cnt);
 
               
            met_string  = "Met_Time" + alldata.functionGroupIdNameMap.find(FuncGroupId)->second;

                    
            cube_out.set_sev(MapCubeMetrics.find("Met_Time")->second, tmp_cnode, tmp_thread, 
                alldata.functionDataPerCPTNode.find(
                pair< uint64_t, uint64_t >(it->first, it_mapping->first.second )
                )->second.excl_time.sum);

            cube_out.set_sev(MapCubeMetrics.find(met_string)->second, tmp_cnode, tmp_thread, 
                alldata.functionDataPerCPTNode.find(
                pair< uint64_t, uint64_t >(it->first, it_mapping->first.second ) 
                )->second.excl_time.sum); 

           /* debug section -> shows CallPathTreeNode_id's as metric */
/*
            cube_out.add_sev(MapCubeMetrics.find("NodeID")->second, 
                        tmp_cnode,
                        tmp_thread, 
                        it->first);
/**/
        }

        it_mapping = alldata.callPathTree.mapping.begin();

    }

    //P2P occ send recv 

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it_sr;    
    
    it_sr = alldata.MessageDataPerNode.begin();

    for(; it_sr != alldata.MessageDataPerNode.end(); it_sr++) {

        if(MapCubeCnodes.find(it_sr->first.first) != MapCubeCnodes.end()){ 
        
            tmp_cnode = MapCubeCnodes.find(it_sr->first.first)->second; 

        } else {

            continue;

        }

        if(MapCubeThreads.find(it_sr->first.second ) != MapCubeThreads.end()) {
        
            tmp_thread = MapCubeThreads.find(it_sr->first.second)->second;

        } else {

                continue;
    
        }

        if(it_sr->second.bytes_send.sum > 0 ) {

            cube_out.add_sev(MapCubeMetrics.find("Met_P2PSendComm")->second, 
                             tmp_cnode,
                             tmp_thread, 
                             it_sr->second.count_send.sum);
                                 
            cube_out.add_sev(MapCubeMetrics.find("Met_P2PBytesSend")->second, 
                             tmp_cnode,
                             tmp_thread, 
                             it_sr->second.bytes_send.sum);

        }

        if (it_sr->second.bytes_recv.sum > 0) {

            cube_out.add_sev(MapCubeMetrics.find("Met_P2PRecvComm")->second, 
                             tmp_cnode,
                             tmp_thread, 
                             it_sr->second.count_recv.sum);

            cube_out.add_sev(MapCubeMetrics.find("Met_P2PBytesRecv")->second, 
                             tmp_cnode,
                             tmp_thread, 
                             it_sr->second.bytes_recv.sum);  

        } 

    }
       
    //Coll occ send recv

    if( alldata.CollopDataPerNode.size() > 0 ) {

        map< pair< uint64_t, uint64_t >, MessageData >::iterator it_coll = alldata.CollopDataPerNode.begin();
        map< pair< uint64_t, uint64_t >, MessageData >::iterator it_coll_e = alldata.CollopDataPerNode.end();

        for( ; it_coll != it_coll_e; it_coll++ ) {

            if( MapCubeThreads.find( it_coll->first.second ) != MapCubeThreads.end() ) {

                tmp_thread = MapCubeThreads.find( it_coll->first.second )->second;

            } else {

                continue;

            }

            if( MapCubeCnodes.find( it_coll->first.first ) != MapCubeCnodes.end() ) {

                tmp_cnode = MapCubeCnodes.find( it_coll->first.first )->second;

            } else {

                continue;

            }

            cube_out.add_sev( MapCubeMetrics.find( "Met_CollOpComm_sum" )->second,
                              tmp_cnode,
                              tmp_thread,
                              ( it_coll->second.count_send.sum + it_coll->second.count_recv.sum ) );

            if( it_coll->second.count_send.sum > 0 ){

                cube_out.add_sev( MapCubeMetrics.find( "Met_CollOpComm_send" )->second,
                                  tmp_cnode,
                                  tmp_thread,
                                  it_coll->second.count_send.sum );

            }

            if( it_coll->second.count_recv.sum > 0 ) {
                
                cube_out.add_sev( MapCubeMetrics.find( "Met_CollOpComm_recv" )->second,
                                  tmp_cnode,
                                  tmp_thread,
                                  it_coll->second.count_recv.sum );

            }

            cube_out.add_sev( MapCubeMetrics.find( "Met_CollOpBytesOut" )->second,
                              tmp_cnode,
                              tmp_thread,
                              it_coll->second.bytes_send.sum );

            cube_out.add_sev( MapCubeMetrics.find( "Met_CollOpBytesIn" )->second,
                              tmp_cnode,
                              tmp_thread,
                              it_coll->second.bytes_recv.sum );

        }

    }

    if( alldata.RmaMsgData.size() > 0 ) {

        map< pair< uint64_t, uint64_t >, RmaData >::iterator it_rma = alldata.RmaMsgData.begin();
        map< pair< uint64_t, uint64_t >, RmaData >::iterator it_rma_e = alldata.RmaMsgData.end();

        uint64_t nodeId = - 1;

        for( ; it_rma != it_rma_e; it_rma++ ) {

            if( MapCubeThreads.find( it_rma->first.second ) != MapCubeThreads.end() ) {

                tmp_thread = MapCubeThreads.find( it_rma->first.second )->second;

            } else {

                continue;

            }

            if( MapCubeCnodes.find( it_rma->first.first/* nodeId*/ ) != MapCubeCnodes.end() ) {

                tmp_cnode = MapCubeCnodes.find( it_rma->first.first/* nodeId*/ )->second;

            } else {

                continue;

            }


            if(it_rma->second.rma_put_cnt > 0 ) {

                cube_out.add_sev(MapCubeMetrics.find("Met_RmaPut")->second, 
                                 tmp_cnode,
                                 tmp_thread, 
                                 it_rma->second.rma_put_cnt);
              
            }

            if(it_rma->second.rma_get_cnt > 0 ) {

                cube_out.add_sev(MapCubeMetrics.find("Met_RmaGet")->second, 
                                 tmp_cnode,
                                 tmp_thread, 
                                 it_rma->second.rma_get_cnt);
              
            }

            if(it_rma->second.rma_put_bytes > 0 ) {

                cube_out.add_sev(MapCubeMetrics.find("Met_RmaPutBytes")->second, 
                                 tmp_cnode,
                                 tmp_thread, 
                                 it_rma->second.rma_put_bytes);
              
            }

            if (it_rma->second.rma_get_bytes > 0) {

                cube_out.add_sev(MapCubeMetrics.find("Met_RmaGetBytes")->second, 
                                 tmp_cnode,
                                 tmp_thread, 
                                 it_rma->second.rma_get_bytes);

            } 

        }

    }
    

    
    string  fname = alldata.params.output_file_prefix;
    ofstream out;

    if( alldata.params.no_reduce ) {

        ostringstream o;
        o << "_" << alldata.myRank;
        
        fname += o.str();
        
    }

    cube_out.writeCubeReport( fname );

    StopMeasurement(alldata, false, "produce cube output");

    return true;
}
