
#pragma once

#include "IHeatPlot.hpp"

using namespace std;

class P2PMPIRuntimePlot : public IHeatPlot
{
public:

    P2PMPIRuntimePlot(string layoutName, AllData& alldata) :
    IHeatPlot(layoutName, alldata)
    {

    }

    string getName(void)
    {
        return "P2PMPIRuntimePlot";
    }

    string getShortName(void)
    {
        return "p2pmpirt";
    }

private:

    void QSort(vector< pair< uint64_t, double > >& data, int left, int right) {

	    int j ,i;
	    i = left;
	    j = right;
	
	    uint64_t tmp;
	    double piv = data[left+(right-left)/2].second;

	    while(i <= j) {

		    while( data[i].second > piv) {

			    i++;

		    }

		    while( data[j].second < piv) {

			    j--;

		    }
		
		    if( i <= j ) {
		
                swap( data[i], data[j] );

			    i++;
			    j--;

		    }

	    }

	    if( left < j ) {

		    QSort(data, left, j);

	    }
	
	    if( i < right ) {

		    QSort( data, i, right );

	    }

    }

    void createDataInternal(ofstream &file_minheat, ofstream &file_avgheat,
            ofstream &file_maxheat, ofstream &file_sumheat,
            double &excl_p2pTimeGlobalMin, double &excl_p2pTimeGlobalMax,
            double &excl_p2pTimeGlobalSumMin, double &excl_p2pTimeGlobalSumMax)
    {
        bool P2PChanged = false;

        double excl_p2pTimeBaseMin, excl_p2pTimeBaseMax, excl_p2pTimeBaseSum;

        excl_p2pTimeGlobalMin    = -1;
        excl_p2pTimeGlobalMax    = 0;
        excl_p2pTimeGlobalSumMin = -1;
        excl_p2pTimeGlobalSumMax = 0;

        vector< pair< uint64_t, double > > sorted_values;

        /* iterate over all base-level members */
        set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
        for (set < uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++)
        {
            uint64_t childId = *it;
            uint64_t groupId = parentMap[childId];
            uint64_t p2pCnt = 0;

            P2PChanged = false;

            /* iterate over all function calls, select all calls of this node */

            #ifdef OTFPROFILE_MPI

            for(map< Pair, FunctionData, ltPair >::iterator it_P2P =
                alldata.functionMapPerSystemTreeNodeReduce.begin();
                it_P2P != alldata.functionMapPerSystemTreeNodeReduce.end();
                it_P2P++)
            {

            #else

            for(map< Pair, FunctionData, ltPair >::iterator it_P2P =
                alldata.functionMapPerSystemTreeNode.begin();
                it_P2P != alldata.functionMapPerSystemTreeNode.end();
                it_P2P++)
            {

            #endif

            

                if (childId != it_P2P->first.a)
                  continue;

                uint64_t functionId = it_P2P->first.b;

                if( alldata.functionGroupIdNameMap.find( alldata.functionIdToGroupIdMap.find( functionId )->second )->second != "MPI" ) {

                    continue;
    
                }

                //first P2P Function on this base member
                if (P2PChanged == false)
                {
                    excl_p2pTimeBaseMin = it_P2P->second.excl_time.min;
                    excl_p2pTimeBaseMax = it_P2P->second.excl_time.max;
                    excl_p2pTimeBaseSum = it_P2P->second.excl_time.sum;

                    P2PChanged = true;
                    //further P2P functions on this base member

                    sorted_values.push_back( make_pair( childId, it_P2P->second.excl_time.sum ) );

                } else
                {
                    if (excl_p2pTimeBaseMin > it_P2P->second.excl_time.min)
                        excl_p2pTimeBaseMin = it_P2P->second.excl_time.min;
                    if (excl_p2pTimeBaseMax < it_P2P->second.excl_time.max)
                        excl_p2pTimeBaseMax = it_P2P->second.excl_time.max;
                    excl_p2pTimeBaseSum += it_P2P->second.excl_time.sum;

                    sorted_values.back().second += it_P2P->second.excl_time.sum;

                }
                p2pCnt++;
            }

            //base member did P2P communication functions - thanks sherlock
            if (P2PChanged)
            {
              /* update globals */
              if (excl_p2pTimeGlobalMin == -1 || excl_p2pTimeGlobalMin > excl_p2pTimeBaseMin)
                  excl_p2pTimeGlobalMin = excl_p2pTimeBaseMin;
              if (excl_p2pTimeGlobalMax == 0 || excl_p2pTimeGlobalMax < excl_p2pTimeBaseMax)
                  excl_p2pTimeGlobalMax = excl_p2pTimeBaseMax;

              if (excl_p2pTimeGlobalSumMin == -1 || excl_p2pTimeGlobalSumMin > excl_p2pTimeBaseSum)
                  excl_p2pTimeGlobalSumMin = excl_p2pTimeBaseSum;
              if (excl_p2pTimeGlobalSumMax == 0 || excl_p2pTimeGlobalSumMax < excl_p2pTimeBaseSum)
                  excl_p2pTimeGlobalSumMax = excl_p2pTimeBaseSum;

              /* write data for this base member */ //besides sum nothing is actually used
              Functioncords& coords = FunctionCoordinateMap[childId];

              //file_sumheat << "group_" << groupId << " " << coords.start << " " << coords.end << " "
              //        << excl_p2pTimeBaseSum << endl;
              file_maxheat << "group_" << groupId << " " << coords.start << " " << coords.end << " "
                      << excl_p2pTimeBaseMax << endl;
              file_avgheat << "group_" << groupId << " " << coords.start << " " << coords.end << " "
                      << (int) (excl_p2pTimeBaseSum / p2pCnt) << endl;
              file_minheat << "group_" << groupId << " " << coords.start << " " << coords.end << " "
                      << excl_p2pTimeBaseMin << endl;

            }
        }

        QSort( sorted_values, 0, sorted_values.size() - 1 );

        int interval_size, interval_add, k;

        if( sorted_values.size() < 11 ) {

            k = sorted_values.size() - 1;

            interval_size = 1;
            interval_add = 0;

        } else {

            k = 10;

            interval_size = sorted_values.size() / 11;
            interval_add = sorted_values.size() - ( 11 * interval_size );

        }

        vector< pair < uint64_t, double > >::iterator it_switch = sorted_values.begin();

        for( int j = 0 ; j <= k; j++ ) {

            for( int i = interval_size; i > 0; i-- ) {

                it_switch->second = j;

                if( i - 1 == 0 && interval_add > 0 ) {

                    it_switch++;
                    interval_add--;

                    it_switch->second = j;

                }

                it_switch++;

            }

        }

        for( int i = 0; i < sorted_values.size(); i++ ) {

            Functioncords& coords = FunctionCoordinateMap[ sorted_values[i].first ];

            file_sumheat << "group_" << parentMap[ sorted_values[i].first ] << " " << coords.start << " " << coords.end << " "
                    << sorted_values[i].second << endl;

        }

    excl_p2pTimeGlobalMin = 0;
    excl_p2pTimeGlobalMax = 11;

    excl_p2pTimeGlobalSumMin = 0;
    excl_p2pTimeGlobalSumMax = 11;

        if (alldata.params.circos_quartile)
        {

            double excl_range = excl_p2pTimeGlobalMax - excl_p2pTimeGlobalMin;
            double excl_rangeSum = excl_p2pTimeGlobalSumMax - excl_p2pTimeGlobalSumMin;

            excl_p2pTimeGlobalMin = excl_p2pTimeGlobalMin +
                    (int) (excl_range * ((double) alldata.params.circos_quartile_percent / 100.0));

            excl_p2pTimeGlobalSumMin = excl_p2pTimeGlobalSumMin +
                    (int)(excl_rangeSum * ((double) alldata.params.circos_quartile_percent / 100.0));

        }

    }
};

