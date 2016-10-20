
#pragma once

#include "IPlot.hpp"

using namespace std;

class IHeatPlot : public IPlot
{
public:

    IHeatPlot(string layoutName, AllData& alldata) :
    IPlot(layoutName, alldata),
    heatMapWidth(0.03)
    {

    }

    double getWidth(void)
    {
        return heatMapWidth;
    }

    void createData(void)
    {
        ofstream file_minheat, file_avgheat, file_maxheat, file_sumheat, file_text;

        file_minheat.open((getShortName() + std::string("_minheat.txt")).c_str());
        file_avgheat.open((getShortName() + std::string("_avgheat.txt")).c_str());
        file_maxheat.open((getShortName() + std::string("_maxheat.txt")).c_str());
        file_sumheat.open((getShortName() + std::string("_sumheat.txt")).c_str());

        dataHeatMin = -1;
        dataHeatMax = 0;
        dataSumMin = -1;
        dataSumMax = 0;
        createDataInternal(file_minheat, file_avgheat, file_maxheat, file_sumheat,
                dataHeatMin, dataHeatMax, dataSumMin, dataSumMax);

        dataHeatMin = std::max(0., dataHeatMin);
        dataHeatMax = std::max(0., dataHeatMax);
        dataSumMin = std::max(0., dataSumMin);
        dataSumMax = std::max(0., dataSumMax);

        file_sumheat.close();
        file_maxheat.close();
        file_avgheat.close();
        file_minheat.close();
    }

    void createColors(ofstream &file)
    {
        //heatmap colors // unnüütz
        /*for (int i = 1; i <= 9; ++i)
        {
            file << getShortName() << "_heat_color_" << i << " = spectral-11-div-" << i << endl; //seqhsv(" << i << ",1,1)" << endl;
        }*/
    }

    void createConf(ofstream &file)
    {
        const double heatStartRadius = startRadius;

        /*const double partWidth = heatMapWidth / 4.;

        //heatmap - min
        file << "<plot>" << endl;
        file << "type  = heatmap" << endl;
        file << "file  = " << getShortName() << "_minheat.txt" << endl;
        file << "r1    = " << heatStartRadius << "r" << endl;
        file << "r0    = " << heatStartRadius - partWidth << "r" << endl;
        file << "color = ylorrd-9-seq" << endl;
        file << "min           = " << dataHeatMin << endl;
        file << "max           = " << dataHeatMax << endl;
        file << "extend_bin    = yes" << endl;
        file << "label_snuggle = yes" << endl;
        file << "</plot>" << endl;

        //heatmap - avg
        file << "<plot>" << endl;
        file << "type  = heatmap" << endl;
        file << "file  = " << getShortName() << "_avgheat.txt" << endl;
        file << "r1    = " << heatStartRadius - partWidth << "r" << endl;
        file << "r0    = " << heatStartRadius - 2 * partWidth << "r" << endl;
        file << "color = ylorrd-9-seq" << endl;
        file << "min           = " << dataHeatMin << endl;
        file << "max           = " << dataHeatMax << endl;
        file << "extend_bin    = yes" << endl;
        file << "label_snuggle = yes" << endl;
        file << "</plot>" << endl;

        //heatmap - max
        file << "<plot>" << endl;
        file << "type  = heatmap" << endl;
        file << "file  = " << getShortName() << "_maxheat.txt" << endl;
        file << "r1    = " << heatStartRadius - 2 * partWidth << "r" << endl;
        file << "r0    = " << heatStartRadius - 3 * partWidth << "r" << endl;
        file << "color = ylorrd-9-seq" << endl;
        file << "min           = " << dataHeatMin << endl;
        file << "max           = " << dataHeatMax << endl;
        file << "extend_bin    = yes" << endl;
        file << "label_snuggle = yes" << endl;
        file << "</plot>" << endl;

        //heatmap - sum
        file << "<plot>" << endl;
        file << "type  = heatmap" << endl;
        file << "file  = " << getShortName() << "_sumheat.txt" << endl;
        file << "r1    = " << heatStartRadius - 3 * partWidth << "r" << endl;
        file << "r0    = " << heatStartRadius - 4 * partWidth << "r" << endl;
        file << "color = ylorrd-9-seq" << endl;
        file << "min           = " << dataSumMin << endl;
        file << "max           = " << dataSumMax << endl;
        file << "extend_bin    = yes" << endl;
        file << "label_snuggle = yes" << endl;
        file << "</plot>" << endl;*/

        //heatmap - sum
        file << "<plot>" << endl;
        file << "type  = heatmap" << endl;
        file << "file  = " << getShortName() << "_sumheat.txt" << endl;
        file << "r1    = " << heatStartRadius << "r" << endl;
        file << "r0    = " << heatStartRadius - heatMapWidth << "r" << endl;
        file << "color = spectral-11-div" << endl;
        file << "min           = " << dataSumMin << endl;
        file << "max           = " << dataSumMax << endl;
        file << "extend_bin    = yes" << endl;
        file << "label_snuggle = yes" << endl;
        file << "</plot>" << endl;
    }


protected:
    virtual void createDataInternal(ofstream &file_minheat, ofstream &file_avgheat,
            ofstream &file_maxheat, ofstream &file_sumheat,
            double &dataHeatMin, double &dataHeatMax, double &dataSumMin,
            double &dataSumMax) = 0;

private:
    double dataHeatMin, dataHeatMax, dataSumMin, dataSumMax;

    const double heatMapWidth;
};


