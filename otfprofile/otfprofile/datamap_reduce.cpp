#include "datamap_reduce.h"


#ifdef OTFPROFILE_MPI

/* merge "local" (rhs) and "global" (alldata) tree [AND mapping] */
void mergeTree( AllData& alldata, CallPathTree rhs_tree ){

	bool found = false;

	map<uint64_t, CallPathTreeNode>::iterator it; //iterator for inner loop over the "global" tree

	map<uint64_t, CallPathTreeNode>::iterator it_rhs = rhs_tree.tree.begin(); //iterator for outer loop over the "local" tree
	map<uint64_t, CallPathTreeNode>::iterator it_rhs_e = rhs_tree.tree.end();

	map<pair<uint64_t, uint64_t>, uint64_t>::iterator it_map; //pointer/iterator to save the find result from mapping

	for( ; it_rhs != it_rhs_e; it_rhs++ ){

		found = false;

        //toll for declaring root nodes only with parent == 0 
		if( it_rhs->second.parent_id == 0 ) {

            //if the node is a root and is found inside the global tree -> just insert into mapping
			if( it_rhs->second.function_id == alldata.callPathTree.tree.find(1)->second.function_id ){

				alldata.callPathTree.intoMapping( rhs_tree.mapping, it_rhs->first, 1 );

			} else {

                for( it = alldata.callPathTree.tree.begin(); it != alldata.callPathTree.tree.end(); it++ ) {

                    //if the node is already present insert just into the mapping 
                    if( it->second.function_id == it_rhs->second.function_id && it->second.parent_id == 0 ) {

                        alldata.callPathTree.intoMapping( rhs_tree.mapping, it_rhs->first, it->first );
                        found = true;
                        break;

                    }

                }

                //if not found -> insert as new node in tree ( mapping included \o/ )
                if( !found ) {


				    alldata.callPathTree.tree.insert( pair<uint64_t, CallPathTreeNode>( alldata.callPathTree.tree.size() + 1, it_rhs->second ) );
					alldata.callPathTree.intoMapping( rhs_tree.mapping, it_rhs->first, alldata.callPathTree.tree.size() );

                } 
					
			}

		} else {

            //similiar to the first half - with exception of the root node behaviour
			
            it_map = rhs_tree.mapping.begin();
            map<pair<uint64_t, uint64_t>, uint64_t>::iterator it_map_e = rhs_tree.mapping.end();
            map<pair<uint64_t, uint64_t>, uint64_t>::iterator it_find;

            for( ; it_map != it_map_e; it_map++) {
 
                if( it_map->second == it_rhs->second.parent_id ) {

                    it_find = alldata.callPathTree.mapping.find( pair< uint64_t, uint64_t >( it_map->first.first, it_map->first.second ) );

                    if( it_find != alldata.callPathTree.mapping.end() ) {

                        found = true;
                        break;

                    }

                }

            }

            if( !found) {

                continue;

            }

            found = false;

			for( it = alldata.callPathTree.tree.begin(); it != alldata.callPathTree.tree.end(); it++ ) {

				if( it->second.function_id == it_rhs->second.function_id && it->second.parent_id == it_find->second ){

					alldata.callPathTree.intoMapping( rhs_tree.mapping, it_rhs->first, it->first );
					found = true;

					break;

				} 

			}
            
			if( !found ){

				alldata.callPathTree.tree.insert( pair<uint64_t, CallPathTreeNode>( alldata.callPathTree.tree.size() + 1, 
                                                                                    CallPathTreeNode( it_rhs->second.function_id, it_find->second ) ) );
		        alldata.callPathTree.intoMapping( rhs_tree.mapping, it_rhs->first, alldata.callPathTree.tree.size() );

			}

		}

	}

}


/* merge the functiondata -- no exeception or errors thrown if nodes aren't present */
void mergeData( AllData& alldata, map< pair< uint64_t, uint64_t >, FunctionData > fData, CallPathTree rhs_tree ) { 


        map< pair< uint64_t, uint64_t >, FunctionData >::iterator it = fData.begin();
        map< pair< uint64_t, uint64_t >, FunctionData >::iterator it_e = fData.end();

        map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp;
        map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp_e = alldata.callPathTree.mapping.end();
        map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp_r_e = rhs_tree.mapping.end();

        uint64_t a, b, c;
        FunctionData insertMeToo;
        //iterate over the local function data
        for(; it != it_e; it++) {

            //search inside local mapping for the key pair of (first) node_id and rank
            for(it_mapp = rhs_tree.mapping.begin(); it_mapp != it_mapp_r_e; it_mapp++) { 

                if( it_mapp->second == it->first.first ) {

                    break;

                }

            }

            //look for the node in the global mapping -> just to be safe
            it_mapp = alldata.callPathTree.mapping.find( it_mapp->first );

            if( it_mapp != it_mapp_e ) {

                pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( it_mapp->second, it->first.second );

                insertMeToo = it->second;

                alldata.functionDataPerCPTNode.insert( pair< pair< uint64_t, uint64_t >, FunctionData >( insertMe, insertMeToo ) );
                //insert the actual data into the global function data map
                alldata.functionDataPerCPTNode.find( insertMe );

            }

        }

}

//equivalent to mergeData in structure but with different maps
void mergeMData( AllData& alldata, map< pair< uint64_t, uint64_t >, MessageData > mData, CallPathTree rhs_tree ) {

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it = mData.begin();
    map< pair< uint64_t, uint64_t >, MessageData >::iterator it_e = mData.end();

    map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp;
    map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp_r_e = rhs_tree.mapping.end();

    for(; it != it_e; it++ ) {


        for(it_mapp = rhs_tree.mapping.begin(); it_mapp != it_mapp_r_e; it_mapp++) {

            if( it_mapp->second == it->first.first ) {

                break;

            }

        }

        it_mapp = alldata.callPathTree.mapping.find( it_mapp->first );

        if( it_mapp != alldata.callPathTree.mapping.end() ) {

            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( it_mapp->second, it->first.second );
            MessageData insertMeToo = MessageData( it->second );

            if( alldata.MessageDataPerNode.find( insertMe ) == alldata.MessageDataPerNode.end() ) {

                alldata.MessageDataPerNode.insert( pair< pair< uint64_t, uint64_t >, MessageData >( insertMe, insertMeToo ) );

            } else {

                alldata.MessageDataPerNode.find( insertMe )->second.add( insertMeToo );

            }

        }

    }

}

//equivalent to mergeData in structure but with different maps
void mergeCData( AllData& alldata, map< pair< uint64_t, uint64_t >, MessageData > cData, CallPathTree rhs_tree ) {

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it = cData.begin();
    map< pair< uint64_t, uint64_t >, MessageData >::iterator it_e = cData.end();

    map< pair< uint64_t, uint64_t>, uint64_t >::iterator it_mapp;
    map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp_r_e = rhs_tree.mapping.end();

    for(; it != it_e; it++) {

        for(it_mapp = rhs_tree.mapping.begin(); it_mapp != it_mapp_r_e; it_mapp++) {

            if( it_mapp->second == it->first.first ) {

                break;

            }

        }

        it_mapp = alldata.callPathTree.mapping.find( it_mapp->first );

        if( it_mapp != alldata.callPathTree.mapping.end() ) {

            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( it_mapp->second, it->first.second );
            MessageData insertMeToo = MessageData( it->second );
    
            alldata.CollopDataPerNode.insert( pair< pair< uint64_t, uint64_t >, MessageData >( insertMe, insertMeToo ) );

        }

    }

}


void mergeRmaData( AllData& alldata, map< pair< uint64_t, uint64_t >, RmaData > rma_Data, CallPathTree rhs_tree ) {

    map< pair< uint64_t, uint64_t >, RmaData >::iterator it = rma_Data.begin();
    map< pair< uint64_t, uint64_t >, RmaData >::iterator it_e = rma_Data.end();

    map< pair< uint64_t, uint64_t>, uint64_t >::iterator it_mapp;
    map< pair< uint64_t, uint64_t >, uint64_t >::iterator it_mapp_r_e = rhs_tree.mapping.end();

    for(; it != it_e; it++) {

        for(it_mapp = rhs_tree.mapping.begin(); it_mapp != it_mapp_r_e; it_mapp++) {

            if( it_mapp->second == it->first.first || ( it->first.first == (uint64_t ) -1 ) ) {

                break;

            }

        }

        it_mapp = alldata.callPathTree.mapping.find( it_mapp->first );

        if( it->first.first == (uint64_t ) -1 ) {

            alldata.RmaMsgData.insert( pair< pair< uint64_t, uint64_t >, RmaData>( make_pair( (uint64_t) -1, it->first.second ), it->second ) );

        } else if( it_mapp != alldata.callPathTree.mapping.end() ) {

            pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >( it_mapp->second, it->first.second );
            RmaData insertMeToo = RmaData( it->second );
    
            alldata.RmaMsgData.insert( pair< pair< uint64_t, uint64_t >, RmaData >( insertMe, insertMeToo ) );

        } 

    }


}


#endif

