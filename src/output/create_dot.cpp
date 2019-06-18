#include "create_dot.h"

bool CreateDot(AllData& alldata) {
    alldata.verbosePrint(1, true, "producing dot output");

    Dot_writer writer(alldata.params);

    writer.open();

    writer.read_data(alldata);

    writer.print();

    writer.close();

    return true;
}