cmake_minimum_required (VERSION 3.8)

# Set the version number
project ( OTFProfiler VERSION 2.0.0 )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include/reader")
include_directories("${PROJECT_SOURCE_DIR}/include/output")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

include(cmake/DefaultBuildType.cmake)


option (USE_OTF "USE OTF library" OFF)
if (USE_OTF)
     find_package (OTF)
     set (HAVE_OPEN_TRACE_FORMAT OTF_FOUND)
endif()

option (USE_OTF2 "Use OTF2 library" ON)
if (USE_OTF2)
    find_package (OTF2)
    set (HAVE_OTF2 OTF2_FOUND)
endif()

option (USE_Cubelib "USE Cube package (library and GUI)" ON)
if (USE_Cubelib)
    find_package (Cubelib)
    set (HAVE_Cubelib Cubelib_FOUND)
endif()

option (USE_MPI "Build MPI parallel version of OTF-Profiler" OFF)
if (USE_MPI)
    find_package(MPI REQUIRED)
    set (HAVE_MPI MPI_FOUND)
endif()

option (USE_JSON "USE JSON outputter (via rapidjson library)" OFF)
if(USE_JSON)
    find_package(rapidjson)
    set(HAVE_JSON RAPIDJSON_FOUND)
endif()

option (USE_DATA_OUT "export everything into json format" ON)
if(USE_DATA_OUT)
    find_package(rapidjson)
    set(HAVE_DATA_OUT RAPIDJSON_FOUND)
endif()

set(SOURCE_FILES
    src/reader/tracereader.cpp
    src/data_tree.cpp
    src/otf-profiler.cpp
    src/definitions.cpp
)

if (HAVE_OPEN_TRACE_FORMAT AND USE_OTF)
    include_directories("${OTF_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/reader/OTFReader.cpp)
    list(APPEND EXTRA_LIBS "${OTF_LIBRARIES}")
endif()

if (HAVE_OTF2 AND USE_OTF2)
    include_directories("${OTF2_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/reader/OTF2Reader.cpp)
    list(APPEND EXTRA_LIBS "${OTF2_LIBRARIES}")
endif()

if (HAVE_Cubelib AND USE_Cubelib)
    include_directories("${Cubelib_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/output/create_cube.cpp)
    list(APPEND EXTRA_LIBS "${Cubelib_LIBRARIES}")
endif()

if (HAVE_JSON AND USE_JSON)
    include_directories("${RAPIDJSON_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/output/create_json.cpp)
endif ()


list(APPEND SOURCE_FILES src/output/create_dot.cpp)
list(APPEND SOURCE_FILES src/output/dot_writer.cpp)

if (HAVE_DATA_OUT AND USE_DATA_OUT)
    include_directories("${RAPIDJSON_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/output/data_out.cpp)
endif()


if (HAVE_JSON)
    list(APPEND SOURCE_FILES src/reader/jsonreader.cpp)
endif()
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/otf-profiler-config.h.in"
  "${PROJECT_BINARY_DIR}/otf-profiler-config.h"
)

# build sequential version of OTF-Profiler
add_executable (otf-profiler ${SOURCE_FILES})
# Requiring language standard C++ 11
target_compile_features(otf-profiler PUBLIC cxx_std_11)
target_link_libraries(otf-profiler ${EXTRA_LIBS})

# add the install targets
install (TARGETS otf-profiler DESTINATION bin)

# build MPI parallel version of OTF-Profiler
if (HAVE_MPI AND USE_MPI)
    add_executable(otf-profiler-mpi ${SOURCE_FILES} src/reduce_data.cpp)
    target_compile_definitions(otf-profiler-mpi PUBLIC OTFPROFILER_MPI)
    target_compile_features(otf-profiler-mpi PUBLIC cxx_std_11)
    target_link_libraries (otf-profiler-mpi ${EXTRA_LIBS} ${MPI_CXX_LIBRARIES})
endif()
