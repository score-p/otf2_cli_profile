
#pragma once

#include "IPlot.hpp"

using namespace std;

class FunctionsGroupsHistogramPlot : public IPlot
{
public:

    FunctionsGroupsHistogramPlot(string layoutName, AllData& alldata) :
    IPlot(layoutName, alldata),
    maxTime(0.0)
    {
        createUsedFGMap();
    }

    string getName(void)
    {
        return "FunctionsGroupsHistogramPlot";
    }

    string getShortName(void)
    {
        return "fghp";
    }

    double getWidth(void)
    {
        return 0.1;
    }

    void createData(void)
    {
        if (usedFunctionGroupsMap.size() == 0)
          return;

        uint64_t functionGroupId;

        ofstream file;

        file.open((getShortName() + string("_histo.txt")).c_str());

        set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
        for (set < uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++)
        {
          uint64_t childId = *it;
          uint64_t groupId = parentMap[childId];
          map<uint64_t, double> timePerFunctionGroup;

          // create TimePerFunctionGroup histogram for this base-level entry
#ifdef OTFPROFILE_MPI          
            map< Pair, FunctionData, ltPair >::iterator it_funcId       = alldata.functionMapPerSystemTreeNodeReduce.lower_bound(Pair(childId, 0));
            map< Pair, FunctionData, ltPair >::iterator it_funcId_ub    = alldata.functionMapPerSystemTreeNodeReduce.upper_bound(Pair(childId, (uint64_t)-1));
#else //OTFPROFILE_MPI                 
            map< Pair, FunctionData, ltPair >::iterator it_funcId       = alldata.functionMapPerSystemTreeNode.lower_bound(Pair(childId, 0));
            map< Pair, FunctionData, ltPair >::iterator it_funcId_ub    = alldata.functionMapPerSystemTreeNode.upper_bound(Pair(childId, (uint64_t)-1));
#endif //OTFPROFILE_MPI

        for( ; it_funcId != it_funcId_ub; it_funcId++ ) {

            functionGroupId = alldata.functionIdToGroupIdMap.find(it_funcId->first.b)->second;

            

if( functionGroupId == mpiGroupId ) {

            map<uint64_t, double>::iterator tfgIter = timePerFunctionGroup.find( 0 );

            if (tfgIter == timePerFunctionGroup.end()) {

                timePerFunctionGroup[ 0 ] = it_funcId->second.excl_time.sum;

            } else {

                tfgIter->second += it_funcId->second.excl_time.sum;

            }
} else {

            map<uint64_t, double>::iterator tfgIter = timePerFunctionGroup.find( 1 );

            if (tfgIter == timePerFunctionGroup.end()) {

                timePerFunctionGroup[ 1 ] = it_funcId->second.excl_time.sum;

            } else {

                tfgIter->second += it_funcId->second.excl_time.sum;

            }

}

        }

          Functioncords& childCoords = FunctionCoordinateMap[childId];
          file << "group_" << groupId << " " << childCoords.start << " " << childCoords.end << " ";

          double time_sum = 0.0;

        map< uint64_t, double >::const_iterator it_Func     = timePerFunctionGroup.begin();
        map< uint64_t, double >::const_iterator it_Func_e   = timePerFunctionGroup.end();

        for (; it_Func != it_Func_e; it_Func++ ) {

            if (it_Func != timePerFunctionGroup.begin()) {

                file << ",";

            }

            file << it_Func->second;
            time_sum += it_Func->second;

        }

          file << endl;

          if (time_sum > maxTime) {

              maxTime = time_sum;
            
          }

        }

        file.close();
    }

    void createColors(ofstream &file)
    { 
        
        if (usedFunctionGroupsMap.size() == 0)
          return;

      file << getShortName() << "_color_" << 1 << " = " << "255,0,0" << endl;
      file << getShortName() << "_color_" << 2 << " = " << "0,255,0" << endl;

    }

    void createConf(ofstream &file)
    {
        if (usedFunctionGroupsMap.size() == 0)
          return;

        file << "<plot>" << endl;
        file << "type = histogram" << endl;
        file << "file = " << (getShortName() + std::string("_histo.txt")).c_str() << endl;
        file << "r1   = " << startRadius << "r" << endl;
        file << "r0   = " << startRadius - getWidth() << "r" << endl;
        file << "min  = 0" << endl;
        file << "max  = " << maxTime << endl;
        file << "fill_color = ";
        file << getShortName() << "_color_" << 1; 
        file << "," << getShortName() << "_color_" << 2;
        file << endl;
        file << "color         = lgrey" << endl;
        file << "fill_under    = yes" << endl;
        file << "thickness     = 2p" << endl;
        file << "extend_bin    = yes" << endl;
        file << "label_snuggle = yes" << endl;
        file << "stroke_thickness = 1" << endl;
        file << "stroke_color  = grey" << endl;
        file << "</plot>" << endl;
    }

private:
    map< uint64_t, string > usedFunctionGroupsMap;
    double maxTime;

    uint64_t mpiGroupId;

    void addColor(ofstream &file, uint64_t id, const std::string color)
    {
      file << getShortName() << "_color_" << id << " = " << color << endl;
    }

    void createUsedFGMap()
    {
        uint64_t functionGroupId;
        string functionGroupName;

#ifdef OTFPROFILE_MPI
        for (map< Pair, FunctionData, ltPair >::iterator it_funcId =
                alldata.functionMapPerSystemTreeNodeReduce.begin();
                it_funcId != alldata.functionMapPerSystemTreeNodeReduce.end();
                it_funcId++)
#else //OTFPROFILE_MPI
        for (map< Pair, FunctionData, ltPair >::iterator it_funcId =
                alldata.functionMapPerSystemTreeNode.begin();
                it_funcId != alldata.functionMapPerSystemTreeNode.end();
                it_funcId++)
#endif //OTFPROFILE_MPI
        {
            uint64_t functionId = it_funcId->first.b;
            map< uint64_t, uint64_t >::const_iterator groupIdIter =
                    alldata.functionIdToGroupIdMap.find(functionId);
            if (groupIdIter != alldata.functionIdToGroupIdMap.end())
            {
                functionGroupId = groupIdIter->second;
                functionGroupName = alldata.functionGroupIdNameMap.find(functionGroupId)->second;
                usedFunctionGroupsMap[functionGroupId] = functionGroupName;

                if( 0 == strcmp( "MPI", functionGroupName.c_str() ) ) {

                    mpiGroupId = functionGroupId;

                }

            }
        }
    }
};


