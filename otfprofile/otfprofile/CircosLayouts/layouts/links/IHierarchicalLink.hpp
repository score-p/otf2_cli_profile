#pragma once

#include <iomanip>

#include "ILink.hpp"

using namespace std;

#define RADIUS_FACTOR_1 0.7
#define RADIUS_FACTOR_2 0.55

class IHierarchicalLink : public ILink
{
public:

    IHierarchicalLink(string layoutName, AllData& alldata) :
    ILink(layoutName, alldata),
    aggregate(alldata.params.circos_aggregate)
    {
    }

    virtual double getWidth(void)
    {
        return startRadius * (1 - RADIUS_FACTOR_2 * RADIUS_FACTOR_1);
    }

    void createConf(ofstream &file)
    {
        if (file.is_open())
        {
            std::string ribbonStyle = "no";
            if (aggregate)
              ribbonStyle = "yes";

            file << "<links>" << endl;

            file << "<link>" << endl;
            file << "file          = " << getShortName() << std::string("_links_short.txt") << endl;
            file << "radius        = " << startRadius - 0.01 << "r" << endl;
            file << "bezier_radius = 0r" << endl;
            file << "thickness     = " << LINK_WIDTH << endl;
            file << "ribbon        = " << "no" << endl;
            file << "stroke_color  = lgrey" << endl;
            file << "stroke_thickness = 0" << endl;
            file << "</link>" << endl;

            file << "<link>" << endl;
            file << "file          = " << getShortName() << std::string("_links_middle.txt") << endl;
            file << "radius        = " << startRadius * RADIUS_FACTOR_1 - 0.01 << "r" << endl;
            file << "bezier_radius = 0r" << endl;
            file << "thickness     = " << LINK_WIDTH << endl;
            file << "ribbon        = " << ribbonStyle << endl;
            file << "stroke_color  = lgrey" << endl;
            file << "stroke_thickness = 1" << endl;
            file << "</link>" << endl;

            file << "<link>" << endl;
            file << "file          = " << getShortName() << std::string("_links_long.txt") << endl;
            file << "radius        = " << startRadius * RADIUS_FACTOR_2 * RADIUS_FACTOR_1 - 0.01 << "r" << endl;
            file << "bezier_radius = 0r" << endl;
            file << "thickness     = " << LINK_WIDTH << endl;
            file << "ribbon        = " << ribbonStyle << endl;
            file << "stroke_color  = lgrey" << endl;
            file << "stroke_thickness = 1" << endl;
            file << "</link>" << endl;

            file << "</links>" << endl;
        }
    }

    void createPlotsConf(ofstream &file)
    {
        //ends
        file << "<plot>" << endl;
        file << "type  		= scatter" << endl;
        file << "file  		= " << getShortName() << std::string("_ends_short.txt") << endl;
        file << "glyph      = triangle" << endl;
        file << "glyph_size = 12p" << endl;
        file << "min        = 0" << endl;
        file << "max        = 1" << endl;
        file << "r0         = " << startRadius + 0.01 << "r" << endl;
        file << "r1         = " << startRadius - 0.01 << "r" << endl;
        file << "fill_color = dgrey" << endl;
        file << "</plot>" << endl;

        file << "<plot>" << endl;
        file << "type  		= scatter" << endl;
        file << "file  		= " << getShortName() << std::string("_ends_middle.txt") << endl;
        file << "glyph      = triangle" << endl;
        file << "glyph_size = 11p" << endl;
        file << "min        = 0" << endl;
        file << "max        = 1" << endl;
        file << "r0         = " << startRadius * RADIUS_FACTOR_1 + 0.01 << "r" << endl;
        file << "r1         = " << startRadius * RADIUS_FACTOR_1 - 0.01 << "r" << endl;
        file << "fill_color = dgrey" << endl;
        file << "</plot>" << endl;

        file << "<plot>" << endl;
        file << "type  		= scatter" << endl;
        file << "file  		= " << getShortName() << std::string("_ends_long.txt") << endl;
        file << "glyph      = triangle" << endl;
        file << "glyph_size = 9p" << endl;
        file << "min        = 0" << endl;
        file << "max        = 1" << endl;
        file << "r0         = " << startRadius * RADIUS_FACTOR_2 * RADIUS_FACTOR_1 + 0.02 << "r" << endl;
        file << "r1         = " << startRadius * RADIUS_FACTOR_2 * RADIUS_FACTOR_1 - 0.01 << "r" << endl;
        file << "fill_color = dgrey" << endl;
        file << "</plot>" << endl;
    }

    void createData(void)
    {
        ofstream of_links_short, of_links_middle, of_links_long,
                of_ends_short, of_ends_middle, of_ends_long;

        of_links_short.open((getShortName() + std::string("_links_short.txt")).c_str());
        of_links_middle.open((getShortName() + std::string("_links_middle.txt")).c_str());
        of_links_long.open((getShortName() + std::string("_links_long.txt")).c_str());
        of_ends_short.open((getShortName() + std::string("_ends_short.txt")).c_str());
        of_ends_middle.open((getShortName() + std::string("_ends_middle.txt")).c_str());
        of_ends_long.open((getShortName() + std::string("_ends_long.txt")).c_str());

        createDataInternal(of_links_short, of_links_middle, of_links_long,
                of_ends_short, of_ends_middle, of_ends_long); //danke für diese zeile

        of_ends_long.close();
        of_ends_middle.close();
        of_ends_short.close();
        of_links_long.close();
        of_links_middle.close();
        of_links_short.close();
    }

protected:
  bool aggregate;

  virtual void createDataInternal(ofstream &of_links_short, ofstream &of_links_middle,
          ofstream &of_links_long, ofstream &of_ends_short,
          ofstream &of_ends_middle, ofstream &of_ends_long) = 0;

  template<class VALUE_TYPE>
  void writeLinks(VALUE_TYPE value, VALUE_TYPE maxValue, VALUE_TYPE minValue,
                  uint64_t senderId, uint64_t receiverId,
                  SystemTreeClass treeClass,
                  uint64_t numElements, uint64_t ctr,
                  ofstream &of_links_short, ofstream &of_links_middle,
                  ofstream &of_links_long, ofstream &of_ends_short,
                  ofstream &of_ends_middle, ofstream &of_ends_long)
  {

        map <uint64_t, Functioncords>::iterator sender;
        map <uint64_t, Functioncords>::iterator receiver;

        sender = FunctionCoordinateMap.find(senderId);
        receiver = FunctionCoordinateMap.find(receiverId);

        uint64_t groupIdSend;
        uint64_t groupIdRecv;

        if (!aggregate || treeClass == baseSystemClass)
        {
            /* base-level communication */
            groupIdSend = parentMap[senderId];
            groupIdRecv = parentMap[receiverId];
        } else if (treeClass == groupSystemClass) {

            /* group-level communication */
            groupIdSend = senderId;
            groupIdRecv = receiverId;

        } else {

            ChildrenMap senderChildren, receiverChildren;
            getChildrenByClass(alldata, alldata.systemTreeIdToSystemTreeNodeMap[senderId],
                    groupSystemClass, senderChildren);
            getChildrenByClass(alldata, alldata.systemTreeIdToSystemTreeNodeMap[receiverId],
                    groupSystemClass, receiverChildren);

            groupIdSend = senderChildren.begin()->first;
            groupIdRecv = receiverChildren.begin()->first;

        }

        VALUE_TYPE range = maxValue - minValue + 1;

        // link color 1-9
        assert(value - minValue <= range);
        int link_color = 1 + (int) (6.0 * (double) (value - minValue) / (double) range);
        if (range == 1)
            link_color = 4;
        int link_transp = 1 + MAX_LINK_TRANSP - (double) MAX_LINK_TRANSP * (double) link_color / 7.;
        int link_width = LINK_WIDTH + 2. * (double) link_color / 9.;

        double dSenderId = senderId;
        double dReceiverId = receiverId;

        double procDistance = sqrt( pow( ( 360. / numElements * ( dSenderId - dReceiverId ) ), 2 ) );

        double link_bezier;

        if (procDistance != 180) {

            procDistance = (uint64_t) procDistance % 180;

            link_bezier = 0.2 - 0.4 * (double) procDistance / 180.;

        } else {

            link_bezier = -0.2;

        }

        ofstream *link_stream = NULL;
        ofstream *ends_stream = NULL;

        const uint64_t linkRadOffset =
          std::floor(double(sender->second.center - sender->second.start) / 2.);
        uint64_t link_start_left = sender->second.center - linkRadOffset;
        uint64_t link_start_right = sender->second.center - linkRadOffset;
        uint64_t link_end_left = receiver->second.center + linkRadOffset;
        uint64_t link_end_right = receiver->second.center + linkRadOffset;

        if (aggregate && (treeClass != baseSystemClass))
        {
          link_start_left = sender->second.center -
                  ((double)(sender->second.center - sender->second.start) * (double) (value - minValue) / (double) range);
          link_start_right = sender->second.center;
          link_end_left = receiver->second.center;
          link_end_right = receiver->second.center +
                  ((double)(receiver->second.end - receiver->second.center) * (double) (value - minValue) / (double) range);
        }

        std::string link_palette;
        if (treeClass == baseSystemClass)
        {
            /* base-level communication */
            link_bezier += 0.25;
            link_palette = "reds-9-seq-";
            link_stream = &of_links_short;
            ends_stream = &of_ends_short;
        } else
          if (treeClass == groupSystemClass)
          {
            /* group-level communication */
            link_palette = "blues-9-seq-";
            link_stream = &of_links_middle;
            ends_stream = &of_ends_middle;
            link_bezier += 0.1;
          } else
          {
            link_palette = "greens-9-seq-";
            link_stream = &of_links_long;
            ends_stream = &of_ends_long;
            link_bezier = 0.;
          }

        if (link_bezier > -0.0001 && link_bezier < 0.0001)
          link_bezier = 0.0;

        *link_stream << "group_" << groupIdSend << " " << link_start_left << " " << link_start_right << " "
                << "group_" << groupIdRecv << " " << link_end_left << " " << link_end_right << " "
                << "color=" << link_palette << 2 + link_color << "_a" << link_transp
                << ",z=" << 80 - (link_color * 10 + ctr % 10)
                << std::setprecision(2) << ",bezier_radius=" << link_bezier << "r"
                << ",thickness=" << link_width << "p" << endl;

        *ends_stream << "group_" << groupIdRecv << " " << link_end_left << " " << link_end_right << " 0.1" << endl;
  }

};

