#pragma once

#include "IScatterPlot.hpp"

using namespace std;

class P2PMessageSizesPlot : public IScatterPlot
{
public:

    P2PMessageSizesPlot(string layoutName, AllData& alldata) :
    IScatterPlot(layoutName, alldata)
    {
    }

    string getName(void)
    {
        return "P2PSendMessageSizesPlot";
    }

    string getShortName(void)
    {
        return "p2psmsp";
    }

private:

    void createDataInternal(ofstream &file)
    {
      /* iterate over all group-level entries */
      set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
      for (set < uint64_t >::iterator it = baseMembers.begin();
              it != baseMembers.end(); it++)
      {
        uint64_t childId = *it;

        map< uint64_t, MessageData >::iterator itMsg = alldata.messageMapPerSystemTreeNode.find(childId);
        if (itMsg != alldata.messageMapPerSystemTreeNode.end())
        {
          uint64_t groupId = parentMap[childId];
          Functioncords& childCoords = FunctionCoordinateMap[childId];
          MessageData &msgData = itMsg->second;

          if (msgData.count_send.cnt == 0)
            continue;

          for (size_t i = 0; i < msgData.msgSizes.numBins; ++i)
          {
            if (msgData.msgSizes.hist[i] > 0)
            {

                int colour = 0.5 + (9.0 * (double) msgData.msgSizes.hist[i] / ( (double) msgData.count_send.sum + (double) msgData.count_recv.sum ) );

                if( colour < 1 ) {

                    colour = 1;

                }

              file << "group_" << groupId << " " << childCoords.start << " " <<
                  childCoords.end << " " << (double) i * 0.1 << " color=ylorrd-9-seq-" <<
                 colour
                  << endl; 

            }
          }
        }
      }
    }
};

