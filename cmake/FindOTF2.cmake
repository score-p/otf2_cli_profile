option(OTF2_USE_STATIC_LIBS "Link OTF2 statically." ON)
if(OTF2_USE_STATIC_LIBS)
  set(_OTF2_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif()

if(OTF2_CONFIG_PATH)
    find_program(OTF2_CONFIG NAMES otf2-config
        PATHS
        /opt/otf2_2_3/bin
        HINTS
        ${OTF2_CONFIG_PATH}
    )
else(OTF2_CONFIG_PATH)
    find_program(OTF2_CONFIG NAMES otf2-config
        PATHS
        /opt/otf2_2_3/bin
    )
endif()

if(NOT OTF2_CONFIG OR NOT EXISTS ${OTF2_CONFIG})
    set(OTF2_VERSION ${OTF2_REQUIRED_VERSION})
else()

    execute_process(COMMAND ${OTF2_CONFIG} "--interface-version" OUTPUT_VARIABLE OTF2_VERSION)
    string(REPLACE ":" "." OTF2_VERSION ${OTF2_VERSION})
    string(STRIP ${OTF2_VERSION} OTF2_VERSION)

    execute_process(COMMAND ${OTF2_CONFIG} "--version" OUTPUT_VARIABLE OTF2_VERSION_STR)
    string(REPLACE "otf2-config: version " "" OTF2_VERSION_STR ${OTF2_VERSION_STR})
    string(REGEX MATCH "^[0-9]*" OTF2_VERSION_MAJOR ${OTF2_VERSION_STR})

    string(STRIP ${OTF2_VERSION_STR} OTF2_VERSION_MINOR)
    string(REGEX MATCH "\." OTF2_VERSION_MINOR ${OTF2_VERSION_MINOR})
    message("OTF2 Minor: " ${OTF2_VERSION_MINOR} " String: " ${OTF2_VERSION_STR})
    # string(REGEX REPLACE "[0-9]*\." "" OTF2_VERSION_MINOR ${OTF2_VERSION_MINOR})

    execute_process(COMMAND ${OTF2_CONFIG} "--cppflags" OUTPUT_VARIABLE OTF2_INCLUDE_DIRS)
    string(REPLACE "\n" "" OTF2_INCLUDE_DIRS ${OTF2_INCLUDE_DIRS})
    string(REPLACE "-I" ";" OTF2_INCLUDE_DIRS ${OTF2_INCLUDE_DIRS})

    execute_process(COMMAND ${OTF2_CONFIG} "--ldflags" OUTPUT_VARIABLE _LINK_LD_ARGS)
    string( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    foreach( _ARG ${_LINK_LD_ARGS} )
        if(${_ARG} MATCHES "^-L")
            string(REGEX REPLACE "^-L" "" _ARG ${_ARG})
            string(STRIP "${_ARG}" _ARG)
            set(OTF2_LINK_DIRS ${OTF2_LINK_DIRS} ${_ARG})
        endif()
    endforeach(_ARG)

    execute_process(COMMAND ${OTF2_CONFIG} "--libs" OUTPUT_VARIABLE _LINK_LD_ARGS)
    string( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    foreach( _ARG ${_LINK_LD_ARGS} )
        if(${_ARG} MATCHES "^-l")
            string(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
            string(STRIP "${_ARG}" _ARG)
            # NO_DEFAULT_PATH - We have to "filter" -lm, as g++ links it anyways. And then stuff explodes
            find_library(_OTF2_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${OTF2_LINK_DIRS} NO_DEFAULT_PATH
            )
            if(_OTF2_LIB_FROM_ARG)
                set(OTF2_LIBRARIES ${OTF2_LIBRARIES} ${_OTF2_LIB_FROM_ARG})
            endif()
            unset(_OTF2_LIB_FROM_ARG CACHE)
        endif()
    endforeach(_ARG)

    find_program(OTF2_PRINT "otf2-print" PATHS "${OTF2_LINK_DIRS}/.." PATH_SUFFIXES "bin")
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(OTF2
    REQUIRED_VARS OTF2_CONFIG OTF2_LIBRARIES OTF2_INCLUDE_DIRS OTF2_PRINT
    VERSION_VAR OTF2_VERSION
    FAIL_MESSAGE "OTF2: otf2 package not found."
)

if(NOT OTF2_FOUND)
    unset(OTF2_PRINT)
    unset(OTF2_LINK_DIRS)
    unset(OTF2_LIBRARIES)
endif()

if(OTF2_USE_STATIC_LIBS)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_OTF2_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
endif()