#pragma once

#include "datastructs.h"

using namespace std;

class IDrawable
{
public:
    IDrawable(string layoutName, AllData& alldata) :
    layoutName(layoutName),
    alldata (alldata),
    startRadius(1.0)
    {
    }

    virtual ~IDrawable(void)
    {

    }

    virtual void setStartRadius(double startRadius)
    {
        this->startRadius = startRadius;
    }

    virtual string getName(void) = 0;

    virtual string getShortName(void) = 0;

    virtual double getWidth(void) = 0;

protected:
    string layoutName;
    AllData& alldata;
    double startRadius;
};

