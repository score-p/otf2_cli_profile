#include "circos_fix.h"
#include <string>


using namespace std;

/* expensive flattening of data based on a call path to fit with the circos module */

uint64_t getParentIdByClass_fix( AllData& allData, uint64_t nodeId, SystemTreeClass treeClassType);

void setup_data( AllData& alldata ) {


    //mapping between cpt_rank notation an systree_rank notation < cpt_id, systree_id >
    map< uint64_t, uint64_t > mapping;
    map< SystemTreeClass, uint64_t > systemClasses;

    typedef map<uint64_t, uint64_t> ParentIdMap;
    ParentIdMap parentMap;


        uint64_t baseSystemMembers = 0;
        uint64_t groupSystemMembers = 0;
        uint64_t topSystemMembers = 0;

        SystemTreeClass baseSystemClass = SYSTEMTREE_UNKNOWN;
        SystemTreeClass groupSystemClass = SYSTEMTREE_UNKNOWN;
        SystemTreeClass topSystemClass = SYSTEMTREE_UNKNOWN;

        SystemTreeClass startClass = SYSTEMTREE_LOCATION_GROUP;

        for (unsigned treeClass = (unsigned)startClass;
                treeClass < SYSTEMTREE_UNKNOWN; ++treeClass)
        {
          SystemTreeClass sysTreeClass = (SystemTreeClass)treeClass;
          map< SystemTreeClass, set < uint64_t > >::const_iterator iter =
                  alldata.systemTreeClassToNodeIdsMap.find(sysTreeClass);

          if (iter != alldata.systemTreeClassToNodeIdsMap.end() &&
                  iter->second.size() > 0)
          {

            if (baseSystemClass == SYSTEMTREE_UNKNOWN)
            {
              if ( iter->second.size() < 257 )
              {
                baseSystemClass = sysTreeClass;
                continue;
              }
            } else
            {
              if (groupSystemClass == SYSTEMTREE_UNKNOWN)
              {
                groupSystemClass = sysTreeClass;
                groupSystemMembers = iter->second.size();
                continue;
              }
              else
              {
                topSystemClass = sysTreeClass;
                topSystemMembers = iter->second.size();
                break;
              }
            }
          }
        }

        if (groupSystemClass == SYSTEMTREE_UNKNOWN)
        {
          cout << "Error: Failed to get system topology" << endl;
          exit(-1);
        }

        baseSystemMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass].size();

        /* list to enumerate used system tree classes */
        systemClasses[baseSystemClass] = baseSystemMembers;
        if (groupSystemClass != SYSTEMTREE_UNKNOWN)
          systemClasses[groupSystemClass] = groupSystemMembers;
        if (topSystemClass != SYSTEMTREE_UNKNOWN)
          systemClasses[topSystemClass] = topSystemMembers;

        set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
        for (set < uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++)
        {
          uint64_t parent_id = getParentIdByClass_fix(alldata, *it, groupSystemClass);
          parentMap[*it] = parent_id;
          /* if topSystemClass in invalid, set parent to 0 so all groups have the same parent */
          parentMap[parent_id] = getParentIdByClass_fix(alldata, parent_id, topSystemClass);
        }



    set< uint64_t >::iterator it    = baseMembers.begin();
    set< uint64_t >::iterator it_e  = baseMembers.end();

    map< uint64_t, SystemTreeNode >::iterator it_stid = alldata.systemTreeIdToSystemTreeNodeMap.begin();
    map< uint64_t, SystemTreeNode >::iterator it_stid_e = alldata.systemTreeIdToSystemTreeNodeMap.end();

    for(; it_stid != it_stid_e; it_stid++ ) {

        for( it = baseMembers.begin(); it != it_e; it++ ) {

            if( it_stid->second.parent == *it ) {

                mapping.insert( make_pair( it_stid->second.location, *it ) );

            }

        }

    }
    
    map< pair< uint64_t, uint64_t >, FunctionData >::iterator it_fdat   = alldata.functionDataPerCPTNode.begin();
    map< pair< uint64_t, uint64_t >, FunctionData >::iterator it_fdat_e = alldata.functionDataPerCPTNode.end();

    for( ; it_fdat != it_fdat_e; it_fdat++ ) {

        uint64_t tmp_func_id;
        uint64_t tmp_rank;

        tmp_rank = mapping.find( it_fdat->first.second )->second;
        tmp_func_id = alldata.callPathTree.tree.find( it_fdat->first.first )->second.function_id;

        #ifdef OTFPROFILE_MPI

            map< Pair, FunctionData, ltPair >::iterator tmp_it = alldata.functionMapPerSystemTreeNodeReduce.find( Pair( tmp_rank, tmp_func_id ) );

            if( tmp_it != alldata.functionMapPerSystemTreeNodeReduce.end() ) {

                tmp_it->second.add( it_fdat->second );

            }else{

                alldata.functionMapPerSystemTreeNodeReduce.insert( make_pair( Pair( tmp_rank, tmp_func_id ), it_fdat->second ) );

            }

        #else

            map< Pair, FunctionData, ltPair >::iterator tmp_it = alldata.functionMapPerSystemTreeNode.find( Pair( tmp_rank, tmp_func_id ) );

            if( tmp_it != alldata.functionMapPerSystemTreeNode.end() ) {

                tmp_it->second.add( it_fdat->second );

            }else{

                alldata.functionMapPerSystemTreeNode.insert( make_pair( Pair( tmp_rank, tmp_func_id ), it_fdat->second ) );

            }

        #endif
        

    }

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it_mdat    = alldata.MessageDataPerNode.begin();
    map< pair< uint64_t, uint64_t >, MessageData >::iterator it_mdat_e  = alldata.MessageDataPerNode.end();

    for( ; it_mdat != it_mdat_e; it_mdat++ ) {

        uint64_t tmp_rank = mapping.find( it_mdat->first.second )->second;

        map< uint64_t, MessageData >::iterator tmp_it = alldata.messageMapPerSystemTreeNode.find( tmp_rank );

        if( tmp_it != alldata.messageMapPerSystemTreeNode.end() ) {

            tmp_it->second.add( it_mdat->second );

        }else{

            alldata.messageMapPerSystemTreeNode.insert( make_pair( tmp_rank, it_mdat->second ) );

        }

    }

    map< Triple, CollectiveData, ltTriple >::iterator it_coldat     = alldata.collectiveMapPerRankPair.begin();
    map< Triple, CollectiveData, ltTriple >::iterator it_coldat_e   = alldata.collectiveMapPerRankPair.end();

    for( ; it_coldat != it_coldat_e; it_coldat++ ) {

        uint64_t tmp_rank   = mapping.find( it_coldat->first.a )->second;
        uint64_t tmp_class  = it_coldat->first.c;

        map< Pair, MessageData, ltPair >::iterator tmp_it = alldata.collectiveMapPerSystemTreeNode.find( Pair( tmp_rank, tmp_class ) );

        if( tmp_it != alldata.collectiveMapPerSystemTreeNode.end() ) {

            tmp_it->second.add( it_coldat->second );

        } else {
            alldata.collectiveMapPerSystemTreeNode.insert( make_pair( Pair( tmp_rank, tmp_class ), it_coldat->second ) );

        }

        uint64_t tmp_rank2 = mapping.find( it_coldat->first.b )->second;

        map< Triple, CollectiveData, ltTriple >::iterator tmp_it2 = alldata.collectiveMapPerSystemTreeNodePair.find( Triple( tmp_rank, tmp_rank2, it_coldat->first.c ) );

        if( tmp_it2 != alldata.collectiveMapPerSystemTreeNodePair.end() ) {

            tmp_it2->second.add( it_coldat->second );

        } else {

            alldata.collectiveMapPerSystemTreeNodePair.insert( make_pair( Triple( tmp_rank, tmp_rank2, it_coldat->first.c ), it_coldat->second ) );

        }

    }


    map< Pair, MessageData, ltPair>::iterator it_m_rp   = alldata.messageMapPerRankPair.begin();
    map< Pair, MessageData, ltPair>::iterator it_m_rp_e = alldata.messageMapPerRankPair.end();

    for( ; it_m_rp != it_m_rp_e; it_m_rp++ ) {

        uint64_t tmp_first, tmp_second;

        tmp_first   = mapping.find( it_m_rp->first.a )->second;
        tmp_second  = mapping.find( it_m_rp->first.b )->second;

        #ifdef OTFPROFILE_MPI
        
            map< Pair, MessageData, ltPair >::iterator tmp_it = alldata.messageMapPerSystemTreeNodePairReduce.find( Pair( tmp_first, tmp_second ) );

            if( tmp_it != alldata.messageMapPerSystemTreeNodePairReduce.end() ) {

                tmp_it->second.add( it_m_rp->second );

            }else{

                alldata.messageMapPerSystemTreeNodePairReduce.insert( make_pair( Pair( tmp_first, tmp_second ), it_m_rp->second ) );

            }

        #else

            map< Pair, MessageData, ltPair >::iterator tmp_it = alldata.messageMapPerSystemTreeNodePair.find( Pair( tmp_first, tmp_second ) );

            if( tmp_it != alldata.messageMapPerSystemTreeNodePair.end() ) {

                tmp_it->second.add( it_m_rp->second );

            }else{

                alldata.messageMapPerSystemTreeNodePair.insert( make_pair( Pair( tmp_first, tmp_second ), it_m_rp->second ) );

            }

        #endif

    }

    
    

}


uint64_t getParentIdByClass_fix(
        AllData& allData,
        uint64_t nodeId,
        SystemTreeClass treeClassType)
{
  if (treeClassType == SYSTEMTREE_UNKNOWN || treeClassType == SYSTEMTREE_OTHER)
    return 0;

  SystemTreeNode& node = allData.systemTreeIdToSystemTreeNodeMap[nodeId];
  if (node.class_id == treeClassType)
    return nodeId;
  else
    return getParentIdByClass_fix(allData, node.parent, treeClassType);
}


void clean_up() {

    string input;
    int pseudo;

        input =  "rm fghp_histo.txt P2PComm_bandtext.txt P2PComm_circos.conf P2PComm.colors P2PComm.data P2PComm_ideogram.conf P2PComm_links.conf "; 
        input += "P2PComm_plots.conf p2pmpirt_avgheat.txt p2pmpirt_maxheat.txt p2pmpirt_minheat.txt p2pmpirt_sumheat.txt p2pmvl_ends_long.txt ";
        input += "p2pmvl_ends_middle.txt p2pmvl_ends_short.txt p2pmvl_links_long.txt p2pmvl_links_middle.txt p2pmvl_links_short.txt ";
        input += "p2pmvp_avgheat.txt p2pmvp_maxheat.txt p2pmvp_minheat.txt p2pmvp_sumheat.txt p2psmsp_scatter.txt";

        pseudo = system( input.c_str() );

}

