#pragma once

#include "datastructs.h"

#include <sstream>
#include <cmath>
#include <algorithm>

#include "circosCommon.hpp"
#include "plots/IPlot.hpp"
#include "links/ILink.hpp"

#define PLOTS_DELTA 0.01

using namespace std;

class ILayout
{
public:

    bool P2P;
    bool CollOp;
    double cnt_range;
    int colorcount;
    uint64_t send_max_val, send_min_val, send_cnt_val, send_sum_val;
    uint64_t recv_max_val, recv_min_val, recv_cnt_val, recv_sum_val;
    uint64_t coll_cnt;

    uint64_t k, start, funcDis, functionGroupId;
    map< uint64_t, string > usedFunctionGroupsMap;

    ILayout(AllData& alldata) :
    alldata(alldata),
    outerRadius(0.9),
    currentPlotRadius(outerRadius),
    totalPlotsWidth(0.0),
    links(NULL)
    {
    }

    virtual ~ILayout(void)
    {
        for (vector<IPlot*>::const_iterator iter = plots.begin();
                iter != plots.end(); ++iter)
        {
            delete *iter;
        }

        if (links)
        {
            delete links;
            links = NULL;
        }
    }

    virtual string getName(void) = 0;

    void addPlot(IPlot *plot)
    {
        if (plot)
        {
            plot->setStartRadius(currentPlotRadius);
            plots.push_back(plot);
            currentPlotRadius -= plot->getWidth() + PLOTS_DELTA;
            totalPlotsWidth += plot->getWidth() + PLOTS_DELTA;
        }
    }

    void addLinks(ILink *links)
    {
        if (links)
        {
            links->setStartRadius(currentPlotRadius - PLOTS_DELTA);
            this->links = links;
            currentPlotRadius -= links->getWidth() + PLOTS_DELTA;
            totalPlotsWidth += links->getWidth() + PLOTS_DELTA;
        }
    }

    void createPlotsConf(void)
    {
        cout << "\tCreate Plots.Conf" << endl;

        ofstream of_plots_conf;
        of_plots_conf.open((getName() + std::string("_plots.conf")).c_str());

        if (of_plots_conf.is_open())
        {
            of_plots_conf << "<plots>" << endl;

            if (links)
                links->createPlotsConf(of_plots_conf);

            for (vector<IPlot*>::const_iterator iter = plots.begin();
                    iter != plots.end(); ++iter)
            {
                (*iter)->createConf(of_plots_conf);
            }

            of_plots_conf << "</plots>" << endl;
        }

        of_plots_conf.close();
    }

    void createColors(void)
    {
        cout << "\tCreate Colors" << endl;

        ofstream of_colors;
        of_colors.open((getName() + string(".colors")).c_str());

        if (of_colors.is_open())
        {
            of_colors << "process_color = hsv(200,1,1)" << endl;

            for (vector<IPlot*>::const_iterator iter = plots.begin();
                    iter != plots.end(); ++iter)
            {
                (*iter)->createColors(of_colors);
            }
        }
        of_colors.close();
    }

    void createCircosConf(void)
    {
        cout << "\tCreate Circos.Conf" << endl;

        ofstream of_circos_conf;
        of_circos_conf.open((getName() + std::string("_circos.conf")).c_str());

        if (of_circos_conf.is_open())
        {
            of_circos_conf << "karyotype = " << (getName() + std::string(".data")).c_str() << endl;

            of_circos_conf << "chromosomes_units = 10" << endl;

            of_circos_conf << "<colors>" << endl;
            of_circos_conf << "<<include " << (getName() + std::string(".colors")).c_str() << ">>" << endl;
            of_circos_conf << "</colors>" << endl;

            of_circos_conf << "<<include " << (getName() + std::string("_ideogram.conf")).c_str() << ">>" << endl;
            of_circos_conf << "<<include " << (getName() + std::string("_plots.conf")).c_str() << ">>" << endl;

            if (links != NULL)
            {
                of_circos_conf << "<<include " << (getName() + std::string("_links.conf")).c_str() << ">>" << endl;
            }

            of_circos_conf << "<image>" << endl;
            of_circos_conf << "dir               = ." << endl;
            of_circos_conf << "file              = " << (getName() + std::string(".png")).c_str() << endl;
            of_circos_conf << "png               = yes" << endl;
            of_circos_conf << "svg               = yes" << endl;

            uint64_t radius =  (uint64_t) (500 * log10(baseSystemMembers));
            if(radius < 1000)
            {
                of_circos_conf << "radius            = 1000" << endl;
            } else
            {
                of_circos_conf << "radius            = " << (size_t) (500 * log10(baseSystemMembers)) << "p" << endl;
            }
            of_circos_conf << "angle_offset      = -90" << endl;
            of_circos_conf << "auto_alpha_colors = yes" << endl;
            of_circos_conf << "auto_alpha_steps  = 10" << endl;
            of_circos_conf << "background        = white" << endl;
            of_circos_conf << "</image>" << endl;

            of_circos_conf << "show_ticks        = yes" << endl;
            of_circos_conf << "show_tick_labels  = yes" << endl;
            of_circos_conf << "show_grid         = yes" << endl;

            of_circos_conf << "<ticks>" << endl;

            of_circos_conf << "chromosomes_display_default = yes" << endl;
            of_circos_conf << "radius               = " << outerRadius << "r" << endl;
            of_circos_conf << "orientation          = out" << endl;
            of_circos_conf << "label_multiplier     = 1e-6" << endl;

            /* iterate over all base-level members */
            uint64_t baseMemberTmpId = 0;
            string basename;
            set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
            for (set < uint64_t >::iterator it = baseMembers.begin();
                    it != baseMembers.end(); it++)
            {
                SystemTreeNode &baseMemberNode = alldata.systemTreeIdToSystemTreeNodeMap[*it];

                basename = baseMemberNode.name;

                string::size_type pos = basename.find_first_of( " " );

                basename = basename.erase( 0, pos );

                of_circos_conf << "<tick>" << endl;
                of_circos_conf << "position       = " << 0.5 + baseMemberTmpId << "u" << endl;
                of_circos_conf << "label_relative = yes" << endl;
                of_circos_conf << "format         = %d" << endl;
                of_circos_conf << "size           = 8p" << endl;
                of_circos_conf << "thickness      = 2p" << endl;
                of_circos_conf << "color          = dgrey" << endl;
                of_circos_conf << "show_label     = yes" << endl;
                of_circos_conf << "label_size     = 20" << endl;
                of_circos_conf << "label_offset   = 3p" << endl;
                of_circos_conf << "label          = " << basename << endl; 
                of_circos_conf << "grid           = yes" << endl;
                of_circos_conf << "grid_color     = vlgrey" << endl;
                of_circos_conf << "grid_thickness = 1p" << endl;
                of_circos_conf << "grid_start     = " << outerRadius << "r" << endl;
                of_circos_conf << "grid_end       = " << currentPlotRadius << "r" << endl;
                of_circos_conf << "</tick>" << endl;

                baseMemberTmpId++;
            }
            of_circos_conf << "</ticks>" << endl;

            of_circos_conf << "<<include etc/colors_fonts_patterns.conf>>" << endl;
            of_circos_conf << "<<include etc/housekeeping.conf>>" << endl;
        }

        of_circos_conf.close();
    };

    void createIdeogrammConf()
    {
        cout << "\tCreate Ideogramm.Conf" << endl;

        ofstream of_ideogram_conf;
        of_ideogram_conf.open((getName() + std::string("_ideogram.conf")).c_str());

        if (of_ideogram_conf.is_open())
        {

            of_ideogram_conf << "<ideogram>" << endl;

            of_ideogram_conf << "<spacing>" << endl;
            of_ideogram_conf << "default = 0.01r" << endl;

            map< SystemTreeClass, set < uint64_t > >::const_iterator treeIter =
                alldata.systemTreeClassToNodeIdsMap.find(groupSystemClass);

            typedef set < uint64_t > IdSet;
            for (IdSet::const_iterator groupIter = treeIter->second.begin();
                    groupIter != treeIter->second.end();)
            {
                IdSet::const_iterator currentIter = groupIter;
                ++groupIter;
                IdSet::const_iterator nextIter = groupIter;
                if (nextIter == treeIter->second.end())
                    nextIter = treeIter->second.begin();

                double spacing = 0.25;
                if (alldata.systemTreeIdToSystemTreeNodeMap[*currentIter].parent !=
                        alldata.systemTreeIdToSystemTreeNodeMap[*nextIter].parent)
                {
                    spacing = 1.0;
                    cout << "different parent between " << *currentIter << " and " <<
                            *nextIter << ": " <<
                            alldata.systemTreeIdToSystemTreeNodeMap[*currentIter].parent << ", "
                            << alldata.systemTreeIdToSystemTreeNodeMap[*nextIter].parent << endl;
                }

                of_ideogram_conf << "<pairwise group_" << *currentIter
                        << " group_" << *nextIter << ">" << endl;
                of_ideogram_conf << "spacing = " << spacing << "r" << endl;
                of_ideogram_conf << "</pairwise>" << endl;
            }

            of_ideogram_conf << "</spacing>" << endl;

            of_ideogram_conf << "radius            = " << outerRadius << "r" << endl;
            of_ideogram_conf << "thickness         = 0p" << endl;
            of_ideogram_conf << "fill              = no" << endl;
            of_ideogram_conf << "stroke_color      = dgrey" << endl;
            of_ideogram_conf << "stroke_thickness  = 0p" << endl;
            if (groupSystemClass == SYSTEMTREE_UNKNOWN ||
                    groupSystemMembers == baseSystemMembers)
            {
              of_ideogram_conf << "show_label        = node" << endl;
            } else
            {
              of_ideogram_conf << "show_label        = yes" << endl;
            }
            of_ideogram_conf << "label_color       = black" << endl;
            of_ideogram_conf << "label_font        = default" << endl;
            of_ideogram_conf << "label_radius      = " << outerRadius << "r + 150p" << endl;
            of_ideogram_conf << "label_size        = 16" << endl;
            of_ideogram_conf << "label_parallel    = yes" << endl;
            of_ideogram_conf << "show_bands        = no" << endl;
            of_ideogram_conf << "fill_bands        = no" << endl;
            of_ideogram_conf << "band_transparency = 5" << endl;

            of_ideogram_conf << "</ideogram>" << endl;
        }
        of_ideogram_conf.close();
    };

    void createLinksConf(void)
    {
        cout << "\tCreate Links.Conf" << endl;

        if (links)
        {
            ofstream of_links_conf;
            of_links_conf.open((getName() + std::string("_links.conf")).c_str());
            links->createConf(of_links_conf);
            of_links_conf.close();
        }
    };

    void createLayout(void)
    {
        cout << "Create Layout: " << getName() << endl;

        getSystemTreeClasses();

        uint64_t baseElemStart = 0;
        uint64_t baseElemSize = 10;

        ofstream of_karyotype, of_bands;
        of_karyotype.open((getName() + std::string(".data")).c_str());
        of_bands.open((getName() + std::string("_bandtext.txt")).c_str());

        map< SystemTreeClass, set < uint64_t > >::const_iterator treeIter =
                alldata.systemTreeClassToNodeIdsMap.find(groupSystemClass);

        for (set < uint64_t >::const_iterator groupIter = treeIter->second.begin();
                groupIter != treeIter->second.end(); ++groupIter)
        {
            uint64_t groupId = *groupIter;
            SystemTreeNode& groupTreeNode = alldata.systemTreeIdToSystemTreeNodeMap[groupId];
            ChildrenMap children;
            getChildrenByClass(alldata, groupTreeNode, baseSystemClass, children);

            /* add coordinate information for group-level element */
            Functioncords coords;
            coords.start = baseElemStart;
            coords.center = baseElemStart + (children.size() * baseElemSize / 2);
            coords.end = baseElemStart + children.size() * baseElemSize;
            FunctionCoordinateMap[groupId] = coords;

            /* add coordinate information for base-level element */
            for (ChildrenMap::const_iterator childIter = children.begin();
                    childIter != children.end(); ++childIter)
            {
                Functioncords coordsChild;
                coordsChild.start = baseElemStart;
                coordsChild.center = baseElemStart + (baseElemSize / 2);
                coordsChild.end = baseElemStart + baseElemSize;
                FunctionCoordinateMap[childIter->first] = coordsChild;

                baseElemStart += baseElemSize;
            }

            of_bands << "group_" << groupId << " " << coords.start
                    << " " << coords.end << endl;

            string groupName = groupTreeNode.name;
            replace(groupName.begin(), groupName.end(), ' ', '_');
            of_karyotype << "chr - group_" <<
                    groupId << " " << groupName << " " <<
                    coords.start << " " << coords.end << " process_color" << endl;
        }

        of_karyotype.close();
        of_bands.close();

        createPlots();
        createColors();
        createCircosConf();
        createIdeogrammConf();
        createPlotsConf();
        createLinksConf();
        createLinks();

        FunctionCoordinateMap.clear();
    };

protected:

    void getSystemTreeClasses()
    {
      /* get base-, group- and topSystemClass */
        baseSystemMembers = 0;
        groupSystemMembers = 0;
        topSystemMembers = 0;

        baseSystemClass = SYSTEMTREE_UNKNOWN;
        groupSystemClass = SYSTEMTREE_UNKNOWN;
        topSystemClass = SYSTEMTREE_UNKNOWN;

        SystemTreeClass startClass = SYSTEMTREE_LOCATION_GROUP;

        for (unsigned treeClass = (unsigned)startClass;
                treeClass < SYSTEMTREE_UNKNOWN; ++treeClass)
        {
          SystemTreeClass sysTreeClass = (SystemTreeClass)treeClass;
          map< SystemTreeClass, set < uint64_t > >::const_iterator iter =
                  alldata.systemTreeClassToNodeIdsMap.find(sysTreeClass);

          if (iter != alldata.systemTreeClassToNodeIdsMap.end() &&
                  iter->second.size() > 0)
          {
            cout << "Level '" << getSystemTreeClassName(sysTreeClass) << "' has " <<
                    iter->second.size() << " entries" << endl;

            if (baseSystemClass == SYSTEMTREE_UNKNOWN)
            {

            /*  
             *  limit of displayable objects in one system tree level
             *  the parent level with the summarized data will be drawn into the circos diagram
             */

              if (iter->second.size() < 257 ) {

                baseSystemClass = sysTreeClass;
                continue;

              } else {

                cout << "Warning: there are to many entires in this level to be displayed!" << endl;
                cout << "Warning: trying to aggregate values on the next level" << endl;

              }

            } else
            {
              if (groupSystemClass == SYSTEMTREE_UNKNOWN)
              {

                groupSystemClass = sysTreeClass;
                groupSystemMembers = iter->second.size();

                continue;
              }
              else
              {

                topSystemClass = sysTreeClass;
                topSystemMembers = iter->second.size();
                break;

              }
            }
          }
        }

        if (groupSystemClass == SYSTEMTREE_UNKNOWN)
        {

          cout << "Error: Failed to get system topology" << endl;
          exit(-1);

        } else if( alldata.systemTreeClassToNodeIdsMap.find(baseSystemClass)->second.size() < 2 ) {

            cout << "Error: not enough entries to diplay data" << endl;
            exit( -1 );

        }

        baseSystemMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass].size();

        /* list to enumerate used system tree classes */
        systemClasses[baseSystemClass] = baseSystemMembers;
        if (groupSystemClass != SYSTEMTREE_UNKNOWN)
          systemClasses[groupSystemClass] = groupSystemMembers;
        if (topSystemClass != SYSTEMTREE_UNKNOWN)
          systemClasses[topSystemClass] = topSystemMembers;

        cout << "Base level  '" << getSystemTreeClassName(baseSystemClass) << "' has " << baseSystemMembers << " entries" << endl;
        cout << "Group level '" << getSystemTreeClassName(groupSystemClass) << "' has " << groupSystemMembers << " entries" << endl;
        cout << "Top level   '" << getSystemTreeClassName(topSystemClass) << "' has " << topSystemMembers << " entries" << endl;

        set < uint64_t >& baseMembers = alldata.systemTreeClassToNodeIdsMap[baseSystemClass];
        for (set < uint64_t >::iterator it = baseMembers.begin(); it != baseMembers.end(); it++)
        {
          uint64_t parent_id = getParentIdByClass(alldata, *it, groupSystemClass);
          parentMap[*it] = parent_id;
          /* if topSystemClass in invalid, set parent to 0 so all groups have the same parent */
          parentMap[parent_id] = getParentIdByClass(alldata, parent_id, topSystemClass);
        }
    }

    void createPlots(void)
    {
        cout << "\tCreate Plots" << endl;

        for (vector<IPlot*>::const_iterator iter = plots.begin();
                iter != plots.end(); ++iter)
        {
            (*iter)->createData();
        }
    }

    void createLinks(void)
    {
        cout << "\tCreate Links" << endl;

        if (links)
            links->createData();
    }

    AllData& alldata;

    double outerRadius;
    double currentPlotRadius;
    double totalPlotsWidth;

    vector<IPlot*> plots;
    ILink *links;
};

