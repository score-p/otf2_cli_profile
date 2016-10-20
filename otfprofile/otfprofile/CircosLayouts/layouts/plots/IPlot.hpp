#pragma once

#include "../IDrawable.hpp"

using namespace std;

class IPlot : public IDrawable
{
public:

    IPlot(string layoutName, AllData& alldata) :
    IDrawable(layoutName, alldata)
    {
    }

    virtual ~IPlot(void)
    {

    }

    virtual void createData(void) = 0;

    virtual void createColors(ofstream &file) = 0;

    virtual void createConf(ofstream &file) = 0;
};

