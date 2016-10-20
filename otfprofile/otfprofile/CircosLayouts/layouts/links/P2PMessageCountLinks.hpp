#pragma once

using namespace std;

class P2PMessageCntLinks : public IHierarchicalLink
{
public:

    P2PMessageCntLinks(string layoutName, AllData& alldata, bool aggregate) :
    IHierarchicalLink(layoutName, alldata, aggregate)
    {
    }

    string getName(void)
    {
        return "P2PMessageCntLinks";
    }

    string getShortName(void)
    {
        return "p2pmcl";
    }

private:
    void createDataInternal(ofstream &of_links_short, ofstream &of_links_middle,
          ofstream &of_links_long, ofstream &of_ends_short,
          ofstream &of_ends_middle, ofstream &of_ends_long)
    {
        /* get max number of messages */
        uint64_t maxMsgs = 0;
        uint64_t minMsgs = 0;
        size_t ctr = 0;

        getValueRange(maxMsgs, minMsgs);

        /* iterate over all group-level entries */
        set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
        for (set < uint64_t >::iterator it = baseMembers.begin();
                it != baseMembers.end(); it++)
        {
          uint64_t senderId = *it;

#ifdef OTFPROFILE_MPI                
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePairReduce.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePairReduce.end;
#else //OTFPROFILE_MPI             
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePair.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePair.end();
#endif //OTFPROFILE_MPI                


          for( ; it_mmprp != it_mmprp_end; it_mmprp++)
          {
              if (it_mmprp->first.a == senderId)
              {
                uint64_t receiverId = it_mmprp->first.b;

                if(it_mmprp->second.bytes_send.cnt > 0)
                {
                    writeLinks(it_mmprp->second.bytes_send.cnt, maxMsgs, minMsgs,
                               senderId, receiverId, baseSystemMembers, ctr,
                               of_links_short, of_links_middle, of_links_long,
                               of_ends_short, of_ends_middle, of_ends_long);
                    ctr++;
                }
              }
          }
        }
    }

    void getValueRange(uint64_t &max, uint64_t &min)
    {
      /* all members of base-most level for the minimum */
      set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
      for (set < uint64_t >::iterator it = baseMembers.begin();
              it != baseMembers.end(); it++)
      {
          uint64_t senderId = *it;

#ifdef OTFPROFILE_MPI                
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePairReduce.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePairReduce.end;
#else //OTFPROFILE_MPI                
    map< Pair, MessageData, ltPair >::iterator it_mmprp     = alldata.messageMapPerSystemTreeNodePair.begin();
    map< Pair, MessageData, ltPair >::iterator it_mmprp_end = alldata.messageMapPerSystemTreeNodePair.end();
#endif //OTFPROFILE_MPI                

          for( ; it_mmprp != it_mmprp_end; it_mmprp++)
          {
              if (it_mmprp->first.a != senderId)
                continue;

              MessageData &data = it_mmprp->second;

              if (it == baseMembers.begin() || data.bytes_send.cnt > max)
              {
                  max = data.bytes_send.cnt;
              }

              if (it == baseMembers.begin() || (min > data.bytes_send.cnt))
              {
                  min = data.bytes_send.cnt;
              }
          }
      }
    }
};

