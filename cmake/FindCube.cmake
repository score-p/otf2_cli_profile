# Copyright (c) 2018, Technische Universität Dresden, Germany
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted
# provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions
#    and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
#    and the following disclaimer in the documentation and/or other materials provided with the
#    distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
#    or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Find Cube library
if (CUBE_LIBRARIES AND CUBE_INCLUDE_DIRS)
  set (CUBE_FIND_QUIETLY TRUE)
endif()

# Find Cube
find_program(CUBELIB_CONFIG NAMES cubelib-config
    PATHS
    /opt/cubelib/bin
    HINTS
    ${PATH}
)
if(NOT CUBELIB_CONFIG OR NOT EXISTS ${CUBELIB_CONFIG})

    find_program(CUBE_CONFIG NAMES cube-config
        PATHS
        /opt/cube/bin
        HINTS
        ${PATH}
    )

endif()

# cube version 4.4 or later
if(CUBELIB_CONFIG OR EXISTS ${CUBELIB_CONFIG})

    # MESSAGE(STATUS "CUBE: cubelib-config found. (using ${CUBELIB_CONFIG}")
    execute_process(COMMAND ${CUBELIB_CONFIG} "--version" OUTPUT_VARIABLE CUBE_VERSION)

    string(STRIP "${CUBE_VERSION}" CUBE_VERSION)

    execute_process(COMMAND ${CUBELIB_CONFIG} "--include" OUTPUT_VARIABLE CUBE_INCLUDE_DIRS)
    string(REPLACE "\n" "" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})
    string(REPLACE "-I" ";" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})

    execute_process(COMMAND ${CUBELIB_CONFIG} "--ldflags" OUTPUT_VARIABLE _LINK_LD_ARGS)
    string( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    foreach( _ARG ${_LINK_LD_ARGS} )
        if(${_ARG} MATCHES "^-L")
            string(REGEX REPLACE "^-L" "" _ARG "${_ARG}")
            string(STRIP "${_ARG}" _ARG)
            set(CUBE_LINK_DIRS ${CUBE_LINK_DIRS} ${_ARG})
        endif()
    endforeach(_ARG)

    execute_process(COMMAND ${CUBELIB_CONFIG} "--libs" OUTPUT_VARIABLE _ARG)

    string(STRIP "${_ARG}" _ARG)
    string(REGEX REPLACE "^-l" "" _ARG "${_ARG}")

    find_library(CUBE_LIB NAMES ${_ARG}
        HINTS ${CUBE_LINK_DIRS} NO_DEFAULT_PATH
    )
    set(CUBE_LIBRARIES ${CUBE_LIBRARIES} ${CUBE_LIB})

# cube pre version 4.4
elseif(CUBE_CONFIG OR EXISTS ${CUBE_CONFIG})

    execute_process(COMMAND ${CUBE_CONFIG} "--version" OUTPUT_VARIABLE CUBE_VERSION)
    string(STRIP ${CUBE_VERSION} CUBE_VERSION)

    execute_process(COMMAND ${CUBE_CONFIG} "--cube-include-path" OUTPUT_VARIABLE CUBE_INCLUDE_DIRS)
    string(REPLACE "\n" "" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})
    string(REPLACE "-I" ";" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})

    execute_process(COMMAND ${CUBE_CONFIG} "--cube-ldflags " OUTPUT_VARIABLE _LINK_LD_ARGS)
    string( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    foreach( _ARG ${_LINK_LD_ARGS} )
        if(${_ARG} MATCHES "^-L")
            string(REGEX REPLACE "^-L" "" _ARG ${_ARG})
            string(STRIP "${_ARG}" _ARG)
            set(CUBE_LINK_DIRS ${CUBE_LINK_DIRS} ${_ARG})
        endif()

        if(${_ARG} MATCHES "^-l")
            string(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
            string(STRIP "${_ARG}" _ARG)
            # NO_DEFAULT_PATH - We have to "filter" -lm, as g++ links it anyways. And then stuff explodes
            find_library(_CUBE_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${CUBE_LINK_DIRS} NO_DEFAULT_PATH
            )
            if(_CUBE_LIB_FROM_ARG)
                set(CUBE_LIBRARIES ${CUBE_LIBRARIES} ${_CUBE_LIB_FROM_ARG})
            endif()
            unset(_CUBE_LIB_FROM_ARG CACHE)
        endif()
    endforeach(_ARG)
else()
    if (CUBE_INC_DIR AND CUBE_LIBS AND CUBE_LIB_DIR)
        find_path(CUBE_INCLUDE_DIRS NAMES Cube.h HINTS ${CUBE_INC_DIR})

        string( REPLACE " " ";" _CUBE_LIBS ${CUBE_LIBS} )
        foreach( _ARG ${_CUBE_LIBS} )
            if(${_ARG} MATCHES "^-l")
                string(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
                string(STRIP "${_ARG}" _ARG)
            endif()
            find_library(_CUBE_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${CUBE_LIB_DIR} NO_DEFAULT_PATH
            )
            if(_CUBE_LIB_FROM_ARG)
                set(CUBE_LIBRARIES ${CUBE_LIBRARIES} ${_CUBE_LIB_FROM_ARG})
            endif()
            unset(_CUBE_LIB_FROM_ARG CACHE)
        endforeach(_ARG)
        unset(_CUBE_LIBS CACHE)
    endif()
endif()
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUBE
    REQUIRED_VARS CUBE_LIBRARIES CUBE_INCLUDE_DIRS
    FAIL_MESSAGE "CUBE: No cube-config found. Try to find Cube manually by setting CUBE_INC_DIR, CUBE_LIB_DIR and CUBE_LIBS."
)

mark_as_advanced(CUBE_INCLUDE_DIRS CUBE_LIBRARIES)