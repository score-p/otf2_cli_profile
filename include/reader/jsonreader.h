#ifndef JSONREADER_H
#define JSONREADER_H

#include "tracereader.h"
#include "rapidjson/document.h"

class JsonReader : public TraceReader{
public:
    JsonReader() = default;

    ~JsonReader() { close();}

    bool initialize(AllData& alldata);
    void close();

    bool readDefinitions(AllData& alldata);
    bool readEvents(AllData& alldata);
    bool readStatistics(AllData& alldata);
    bool readSystemTree(AllData& alldata);

private:
    rapidjson::Document document;
};

#endif