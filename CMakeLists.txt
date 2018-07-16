cmake_minimum_required (VERSION 3.5)

project (OTF-Profiler)

# Set the version number
set (OTFProfiler_VERSION_MAJOR 2)
set (OTFProfiler_VERSION_MINOR 0)
set (OTFProfiler_VERSION_PATCH 0)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include/reader")
include_directories("${PROJECT_SOURCE_DIR}/include/output")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

include(cmake/DefaultBuildType.cmake)

#TODO: checks for MPI, CUBE

option (USE_OTF "USE OTF library" ON)
if (USE_OTF)
     find_package (OTF)
     if (OTF_FOUND)
         set (HAVE_OPEN_TRACE_FORMAT true)
     else ()
         unset (HAVE_OPEN_TRACE_FORMAT)
     endif (OTF_FOUND)
endif (USE_OTF)

option (USE_OTF2 "Use OTF2 library" ON)
if (USE_OTF2)
    find_package (OTF2)
    if (OTF2_FOUND)
        set (HAVE_OTF2 true)
    else ()
        unset (HAVE_OTF2)
    endif (OTF2_FOUND)
endif (USE_OTF2)

option (USE_CUBE "USE Cube package (library and GUI)" ON)
if (USE_CUBE)
    find_package (Cube)
    if (CUBE_FOUND)
        set (HAVE_CUBE true)
    else ()
        unset (HAVE_CUBE)
    endif (CUBE_FOUND)
endif (USE_CUBE)

# option (BUILD_MPI_VERSION "Build MPI parallel version of OTF-Profiler")

option (USE_JSON "USE JSON outputter (via rapidjson library)" ON)
if(USE_JSON)
    find_package(rapidjson)
    if(RAPIDJSON_FOUND)
	set(HAVE_JSON true)
    else()
	unset(HAVE_JSON)
    endif(RAPIDJSON_FOUND)
endif(USE_JSON)

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
endif (HAVE_OPEN_TRACE_FORMAT AND USE_OTF)

if (HAVE_OTF2 AND USE_OTF2)
    include_directories("${OTF2_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/reader/OTF2Reader.cpp)
    list(APPEND EXTRA_LIBS "${OTF2_LIBRARIES}")
endif (HAVE_OTF2 AND USE_OTF2)

if (HAVE_CUBE AND USE_CUBE)
    include_directories("${CUBE_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/output/create_cube.cpp)
    list(APPEND EXTRA_LIBS "${CUBE_LIBRARIES}")
endif (HAVE_CUBE AND USE_CUBE)

if (HAVE_JSON AND USE_JSON)
    include_directories("${RAPIDJSON_INCLUDE_DIRS}")
    list(APPEND SOURCE_FILES src/output/create_json.cpp)
endif (HAVE_JSON AND USE_JSON)

if (HAVE_MPI AND BUILD_MPI_VERSION)

endif (HAVE_MPI AND BUILD_MPI_VERSION)

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
    target_compile_definitions(otf-profiler-mpi PUBLIC OTFPROFILER_MPI)
    target_compile_features(otf-profiler-mpi PUBLIC cxx_std_11)
    add_executable(otf-profiler-mpi ${SOURCE_FILES} src/reduce_data.cpp)
    target_link_libraries (otfprofiler-mpi ${EXTRA-LIBS})
endif (HAVE_MPI AND USE_MPI)
