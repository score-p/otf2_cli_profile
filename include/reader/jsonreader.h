#ifndef JSONREADER_H
#define JSONREADER_H

#include "tracereader.h"
#include "rapidjson/document.h"

class JSONReader : public TraceReader{
public:
    JSONReader() = default;

    ~JSONReader() { close();}

    void close();
    bool initialize(AllData& alldata);
    bool readDefinitions(AllData& alldata);
    bool readEvents(AllData& alldata);
    bool readStatistics(AllData& alldata);
private:
    rapidjson::Document document;
};

#endif