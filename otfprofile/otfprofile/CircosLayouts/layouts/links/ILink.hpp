#pragma once

#include <fstream>

using namespace std;

#define LINK_WIDTH 2
#define MAX_LINK_TRANSP 5

class ILink : public IDrawable
{
public:

    ILink(string layoutName, AllData& alldata) :
    IDrawable(layoutName, alldata)
    {
    }

    virtual ~ILink(void)
    {

    }

    virtual double getWidth(void)
    {
        return 0;
    }

    virtual void createConf(ofstream &file)
    {
        if (file.is_open())
        {
            file << "<links>" << endl;
            file << "<link>" << endl;
            file << "file          = " << getShortName() << std::string("_links.txt") << endl;
            file << "radius        = " << startRadius - 0.01 << "r" << endl;
            file << "bezier_radius = 0r" << endl;
            file << "thickness     = " << LINK_WIDTH << endl;
            file << "</link>" << endl;
            file << "</links>" << endl;
        }
    }

    virtual void createPlotsConf(ofstream &file)
    {
        //ends
        file << "<plot>" << endl;
        file << "type  		= scatter" << endl;
        file << "file  		= " << getShortName() << std::string("_ends.txt") << endl;
        file << "glyph      = triangle" << endl;
        file << "glyph_size = 12p" << endl;
        file << "min        = 0" << endl;
        file << "max        = 1" << endl;
        file << "r0         = " << startRadius << "r" << endl;
        file << "r1         = " << startRadius - 0.01 << "r" << endl;
        file << "fill_color = dgrey" << endl;
        file << "</plot>" << endl;
    }

    virtual void createData(void) = 0;
};
