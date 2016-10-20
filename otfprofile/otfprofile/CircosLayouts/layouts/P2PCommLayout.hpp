
#pragma once

#include "datastructs.h"
#include "ILayout.hpp"

#include "plots/FunctionsGroupsHistogramPlot.hpp"
#include "plots/P2PMessageVolumePlot.hpp"
#include "plots/P2PMPIRuntimePlot.hpp"
#include "plots/P2PMessageSizesPlot.hpp"
#include "links/P2PMessageVolLinks.hpp"

/**
 * Layout includes:
 *  - Exclusive Time Function Group Summary
 *  - P2P Message sizes per size class (min/avg/max/sum)
 *  - P2P Message volume in bytes (min/avg/max/sum)
 *  - P2P MPI Runtime (min/avg/max/sum)
 *  - Links: P2P MPI communications (volume in bytes)
 */
class P2PCommLayout : public ILayout
{
public:

    P2PCommLayout(AllData& alldata) :
    ILayout(alldata)
    {

        addPlot(new FunctionsGroupsHistogramPlot(getName(), alldata));
        addPlot(new P2PMessageSizesPlot(getName(), alldata)); 
        addPlot(new P2PMessageVolumePlot(getName(), alldata));
        addPlot(new P2PMPIRuntimePlot(getName(), alldata));

        addLinks(new P2PMessageVolLinks(getName(), alldata)); 

    }

    string getName(void)
    {
        return "P2PComm";
    }
};






