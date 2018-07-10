if (HAVE_OPEN_TRACE_FORMAT)
    add_library(otf-reader OTFReader.cpp)
endif (HAVE_OPEN_TRACE_FORMAT)

if (HAVE_OTF2)
    add_library(otf2-reader OTF2Reader.cpp)
endif (HAVE_OTF2)
