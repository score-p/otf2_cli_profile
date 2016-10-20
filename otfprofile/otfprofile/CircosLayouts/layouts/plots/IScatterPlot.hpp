
#pragma once

#include "IPlot.hpp"

using namespace std;

class IScatterPlot : public IPlot
{
public:

    IScatterPlot(string layoutName, AllData& alldata) :
    IPlot(layoutName, alldata)
    {
        srand(time(NULL));
    }

    ~IScatterPlot()
    {

    }

    double getWidth(void)
    {
        return 0.06;
    }

    void createData(void)
    {
        ofstream file_scatter;
        file_scatter.open((getShortName() + std::string("_scatter.txt")).c_str());

        createDataInternal(file_scatter);

        file_scatter.close();
    }

    void createColors(ofstream &file)
    {

    }

    void createConf(ofstream &file)
    {
        file << "<plot>" << endl;

        file << "show  = yes" << endl;
        file << "type  = scatter" << endl;

        file << "file  = " << getShortName() << "_scatter.txt" << endl;
        file << "r1    = " << startRadius << "r" << endl;
        file << "r0    = " << startRadius - getWidth() << "r" << endl;
        file << "max   = 1.0" << endl;
        file << "min   = 0.0" << endl;

        file << "glyph            = circle" << endl;
        file << "glyph_size       = " <<  (int)(4 * log10(alldata.allProcesses.size())) << endl;
        file << "color            = ylorrd-9-seq" << endl;
        file << "stroke_color     = ylorrd-9-seq" << endl;
        file << "stroke_thickness = 0" << endl;

        file << "<backgrounds>" << endl;
        file << "<background>" << endl;
        file << "color = vvlgrey" << endl;
        file << "</background>" << endl;
        file << "</backgrounds>" << endl;

        file << "<axes>" << endl;
        file << "<axis>" << endl;
        file << "color     = lgrey" << endl;
        file << "thickness = 2" << endl;
        file << "spacing   = 0.1r" << endl;
        file << "</axis>" << endl;
        file << "</axes>" << endl;

        file << "</plot>" << endl;
    }

protected:

    virtual void createDataInternal(ofstream &file) = 0;
};


