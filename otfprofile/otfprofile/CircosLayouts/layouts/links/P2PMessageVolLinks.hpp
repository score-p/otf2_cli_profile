#pragma once

#include "IHierarchicalLink.hpp"

using namespace std;

class P2PMessageVolLinks : public IHierarchicalLink
{
public:

    P2PMessageVolLinks(string layoutName, AllData& alldata) :
    IHierarchicalLink(layoutName, alldata)
    {
    }

    string getName(void)
    {
        return "P2PMessageVolLinks";
    }

    string getShortName(void)
    {
        return "p2pmvl";
    }

private:

  void createDataInternalAgg(ofstream &of_links_short, ofstream &of_links_middle,
          ofstream &of_links_long, ofstream &of_ends_short,
          ofstream &of_ends_middle, ofstream &of_ends_long)
  {
    /* get max and min message volume */
    uint64_t maxMsgsVol = 0;
    uint64_t minMsgsVol = 0;
    size_t ctr = 0;

    for (map< SystemTreeClass, uint64_t >::const_iterator
            treeClassIter = systemClasses.begin();
            treeClassIter != systemClasses.end(); ++treeClassIter)
    {
      SystemTreeClass treeClass = treeClassIter->first;

      getValueRange(maxMsgsVol, minMsgsVol, treeClass);

      /* iterate over all group-level entries */
      set < uint64_t >& members = alldata.systemTreeClassToNodeIdsMap[treeClass];
      for (set < uint64_t >::iterator it = members.begin();
              it != members.end(); it++)
      {
        uint64_t senderId = *it;
        uint64_t senderParentId = parentMap[senderId];


#ifdef OTFPROFILE_MPI                
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePairReduce.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePairReduce.end();
#else //OTFPROFILE_MPI
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePair.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePair.end();
#endif //OTFPROFILE_MPI                


        for( ; it_mmprp != it_mmprp_end; it_mmprp++)
        {
            if ((it_mmprp->first.a == senderId) && (it_mmprp->second.bytes_send.sum > 0))
            {
              uint64_t receiverId = it_mmprp->first.b;
              uint64_t receiverParentId = parentMap[receiverId];

              /* aggregate runs over all levels, consider only intra-level comm. each time */
              if (senderParentId != receiverParentId)
                continue;

              /* show self-comm. only for base level */
              if ((senderId == receiverId) && (treeClass != baseSystemClass))
                continue;

              writeLinks(it_mmprp->second.bytes_send.sum, maxMsgsVol, minMsgsVol,
                         senderId, receiverId, treeClass,
                         systemClasses[baseSystemClass], ctr,
                         of_links_short, of_links_middle, of_links_long,
                         of_ends_short, of_ends_middle, of_ends_long);
              ctr++;
            }
        }
      }
    }
  }

    void createDataInternal(ofstream &of_links_short, ofstream &of_links_middle,
                            ofstream &of_links_long, ofstream &of_ends_short,
                            ofstream &of_ends_middle, ofstream &of_ends_long) {

        if (aggregate)
        {
          return createDataInternalAgg(of_links_short, of_links_middle, of_links_long,
                  of_ends_short, of_ends_middle, of_ends_long);
        }

        /* get max and min message volume */
        uint64_t maxMsgsVol = 0;
        uint64_t minMsgsVol = (uint64_t)-1;
        size_t ctr = 0;

        getValueRange(maxMsgsVol, minMsgsVol, baseSystemClass);

        /* iterate over all group-level entries */
        set < uint64_t >& members = alldata.systemTreeClassToNodeIdsMap[baseSystemClass]; 

#ifdef OTFPROFILE_MPI                
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePairReduce.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePairReduce.end();
#else //OTFPROFILE_MPI                
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePair.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePair.end();
#endif //OTFPROFILE_MPI                


        for( ; it_mmprp != it_mmprp_end; it_mmprp++) {

            if( members.find( it_mmprp->first.a ) == members.end() || ( it_mmprp->second.bytes_send.sum == 0 ) ) {

                continue;

            }

            uint64_t senderId = it_mmprp->first.a;
            uint64_t senderParentId = parentMap[senderId]; 
            
            uint64_t receiverId = it_mmprp->first.b;
            uint64_t receiverParentId = parentMap[receiverId];
            SystemTreeClass currentTreeClass = baseSystemClass;

            /* runs over base-level entries, need to decide on actual current level */
            if (senderParentId != receiverParentId) {

                uint64_t senderParentParentId = parentMap[senderParentId];
                uint64_t receiverParentParentId = parentMap[receiverParentId];
                if (senderParentParentId != receiverParentParentId) {

                    currentTreeClass = topSystemClass;

                } else {

                    currentTreeClass = groupSystemClass;

                }

            }

            writeLinks(it_mmprp->second.bytes_send.sum, maxMsgsVol, minMsgsVol,
                        senderId, receiverId, currentTreeClass,
                        systemClasses[baseSystemClass], ctr,
                        of_links_short, of_links_middle, of_links_long,
                        of_ends_short, of_ends_middle, of_ends_long);
              ctr++;

        }

    }
        

    void getValueRange(uint64_t &max, uint64_t &min, const SystemTreeClass treeClass)
    {
        /* all members of base-most level for the minimum */
        set < uint64_t >& members = alldata.systemTreeClassToNodeIdsMap[treeClass];

#ifdef OTFPROFILE_MPI                
            map< Pair, MessageData, ltPair >::iterator it_lower_bound = alldata.messageMapPerSystemTreeNodePairReduce.begin();
            map< Pair, MessageData, ltPair >::iterator it_upper_bound = alldata.messageMapPerSystemTreeNodePairReduce.end();
#else //OTFPROFILE_MPI                
            map< Pair, MessageData, ltPair >::iterator it_lower_bound = alldata.messageMapPerSystemTreeNodePair.begin();
            map< Pair, MessageData, ltPair >::iterator it_upper_bound = alldata.messageMapPerSystemTreeNodePair.end();
#endif //OTFPROFILE_MPI               

            for( ; it_lower_bound != it_upper_bound; it_lower_bound++ ) {
                
                if( members.find( it_lower_bound->first.a) != members.end() ) {

                    if (it_lower_bound->second.bytes_send.sum > max)
                    {
                        max = it_lower_bound->second.bytes_send.sum;
                    }
                    
                    if ((min > it_lower_bound->second.bytes_send.sum))
                    {
                        min = it_lower_bound->second.bytes_send.sum;
                    }

                }
            }
    }
};

