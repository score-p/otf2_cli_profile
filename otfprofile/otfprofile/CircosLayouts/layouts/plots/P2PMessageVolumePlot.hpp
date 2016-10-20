
#pragma once

#include "IHeatPlot.hpp"

using namespace std;

class P2PMessageVolumePlot : public IHeatPlot
{
public:

    P2PMessageVolumePlot(string layoutName, AllData& alldata) :
    IHeatPlot(layoutName, alldata)
    {

    }

    string getName(void)
    {
        return "P2PMessageVolumePlot";
    }

    string getShortName(void)
    {
        return "p2pmvp";
    }


private:

    void QSort(vector< pair< uint64_t, pair< uint64_t, bool > > >& data, int left, int right) {

	    int j ,i;
	    i = left;
	    j = right;

	    uint64_t tmp;
	    uint64_t piv = data[left+(right-left)/2].second.first;
	
	    while(i <= j) {
	
		    while( data[i].second.first > piv) {

			    i++;

		    }

		    while( data[j].second.first < piv) {

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
            double &p2pHeatMin, double &p2pHeatMax, double &p2pSumMin, double &p2pSumMax)
    {
        p2pHeatMin = -1;
        p2pHeatMax = 0;
        p2pSumMin = -1;
        p2pSumMax = 0;

        set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];

        /* get ranges */
        bool firstEntry = true;
        for (set < uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++)
        {

            uint64_t childId = *it;
            map< uint64_t, MessageData >::const_iterator it_P2P =
                    alldata.messageMapPerSystemTreeNode.find(childId);

            if (firstEntry)
            {
                p2pHeatMax = (it_P2P->second.bytes_send.max > it_P2P->second.bytes_recv.max) ?
                    it_P2P->second.bytes_send.max : it_P2P->second.bytes_recv.max;
                p2pHeatMin = (it_P2P->second.bytes_send.min < it_P2P->second.bytes_recv.min) ?
                    it_P2P->second.bytes_send.min : it_P2P->second.bytes_recv.min;
                p2pSumMax = (it_P2P->second.bytes_send.sum > it_P2P->second.bytes_recv.sum) ?
                    it_P2P->second.bytes_send.sum : it_P2P->second.bytes_recv.sum;
                p2pSumMin = (it_P2P->second.bytes_send.sum < it_P2P->second.bytes_recv.sum) ?
                    it_P2P->second.bytes_send.sum : it_P2P->second.bytes_recv.sum;

                firstEntry = false;
            } else
            {
                if (p2pHeatMax < it_P2P->second.bytes_send.max)
                    p2pHeatMax = it_P2P->second.bytes_send.max;
                if (p2pHeatMin > it_P2P->second.bytes_send.min)
                    p2pHeatMin = it_P2P->second.bytes_send.min;

                if (p2pSumMax < it_P2P->second.bytes_send.sum)
                    p2pSumMax = it_P2P->second.bytes_send.sum;
                if (p2pSumMin > it_P2P->second.bytes_send.sum)
                    p2pSumMin = it_P2P->second.bytes_send.sum;

                if (p2pHeatMax < it_P2P->second.bytes_recv.max)
                    p2pHeatMax = it_P2P->second.bytes_recv.max;
                if (p2pHeatMin > it_P2P->second.bytes_recv.min)
                    p2pHeatMin = it_P2P->second.bytes_recv.min;

                if (p2pSumMax < it_P2P->second.bytes_recv.sum)
                    p2pSumMax = it_P2P->second.bytes_recv.sum;
                if (p2pSumMin > it_P2P->second.bytes_recv.sum)
                    p2pSumMin = it_P2P->second.bytes_recv.sum;
            }
        }

                /*sorting the bytes_send.sum/recv.sum to fit in the colour value range*/
                vector< pair< uint64_t, pair< uint64_t, bool > > > sorted_values;

                for( set< uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++ ) {

                    map< uint64_t, MessageData >::iterator it_data = alldata.messageMapPerSystemTreeNode.find( *it );

                    if( it_data == alldata.messageMapPerSystemTreeNode.end() ) {

                        continue;

                    }

                    sorted_values.push_back( make_pair( *it, make_pair( it_data->second.bytes_send.sum, 0 ) ) );
                    sorted_values.push_back( make_pair( *it, make_pair( it_data->second.bytes_recv.sum, 1 ) ) );

                }

                QSort( sorted_values, 0, sorted_values.size() - 1 );

                vector< pair< uint64_t, pair< uint64_t, bool > > >::iterator it_switch = sorted_values.begin();

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

                for( int j = 0; j <= k; j++ ) {

                    for( int i = interval_size; i > 0; i-- ) {

                        it_switch->second.first = j;

                        if( i - 1 == 0 && interval_add > 0 ) {

                            it_switch++;
                            interval_add--;

                            it_switch->second.first = j;

                        }

                        it_switch++;

                    }

                }

                /* iterate over all base-level entries */
                for (set < uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++)
                {

                    uint64_t groupId = parentMap[*it];
                    Functioncords &coords = FunctionCoordinateMap[*it];

                    map< uint64_t, MessageData >::const_iterator it_P2P =
                          alldata.messageMapPerSystemTreeNode.find(*it);

                    vector< pair< uint64_t, pair< uint64_t, bool> > >::iterator it_in = sorted_values.begin();

                    int count = 0;

                    for( ; it_in != sorted_values.end(); it_in++ ) {

                        if( *it == it_in->first ) {

                            if( it_in->second.second == 0 ) {

                                file_sumheat << "group_" << groupId << " " << coords.start << " " << coords.center << " "
                                << it_in->second.first << endl;

                            } else {

                                file_sumheat << "group_" << groupId << " " << coords.center << " " << coords.end << " "
                                << it_in->second.first << endl;

                            }

                            count++;

                            if( count >= 2 ) {

                                break;

                            }

                        }

                    }

//totally useless
                    file_maxheat << "group_" << groupId << " " << coords.start << " " << coords.center << " "
                            << it_P2P->second.bytes_send.max << endl;
                    file_maxheat << "group_" << groupId << " " << coords.center << " " << coords.end << " "
                            << it_P2P->second.bytes_recv.max << endl;

                    double bytes_send_avg = 0.;
                    if (it_P2P->second.bytes_send.cnt)
                      bytes_send_avg = (double)(it_P2P->second.bytes_send.sum / it_P2P->second.bytes_send.cnt);
                    double bytes_recv_avg = 0.;
                    if (it_P2P->second.bytes_recv.cnt)
                      bytes_recv_avg = (double)(it_P2P->second.bytes_recv.sum / it_P2P->second.bytes_recv.cnt);

                    file_avgheat << "group_" << groupId << " " << coords.start << " " << coords.center << " "
                            << bytes_send_avg << endl;
                    file_avgheat << "group_" << groupId << " " << coords.center << " " << coords.end << " "
                            << bytes_recv_avg << endl;

                    file_minheat << "group_" << groupId << " " << coords.start << " " << coords.center << " "
                            << it_P2P->second.bytes_send.min << endl;
                    file_minheat << "group_" << groupId << " " << coords.center << " " << coords.end << " "
                            << it_P2P->second.bytes_recv.min << endl;
                }


                //Manipulation of the color mapping
                p2pSumMin = 0;

                p2pSumMax = 11;

                p2pHeatMin = 0;

                p2pHeatMax = 11;
        
    }

};

