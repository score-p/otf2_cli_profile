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
if (Cubelib_LIBRARIES AND Cubelib_INCLUDE_DIRS)
  set (Cubelib_FIND_QUIETLY TRUE)
endif()

# Find Cube
find_program(Cubelib_CONFIG NAMES cubelib-config
    PATHS
    /opt/cubelib/bin
    HINTS
    ${PATH}
)
if(NOT Cubelib_CONFIG OR NOT EXISTS ${Cubelib_CONFIG})

    find_program(CUBE_CONFIG NAMES cube-config
        PATHS
        /opt/cube/bin
        HINTS
        ${PATH}
    )

endif()

# cube version 4.4 or later
if(Cubelib_CONFIG OR EXISTS ${Cubelib_CONFIG})
    # MESSAGE(STATUS "CUBE: cubelib-config found. (using ${Cubelib_CONFIG}")
    execute_process(COMMAND ${Cubelib_CONFIG} "--version" OUTPUT_VARIABLE Cubelib_VERSION)

    string(STRIP "${Cubelib_VERSION}" Cubelib_VERSION)

    execute_process(COMMAND ${Cubelib_CONFIG} "--include" OUTPUT_VARIABLE Cubelib_INCLUDE_DIRS)
    string(REPLACE "\n" "" Cubelib_INCLUDE_DIRS ${Cubelib_INCLUDE_DIRS})
    string(REPLACE "-I" ";" Cubelib_INCLUDE_DIRS ${Cubelib_INCLUDE_DIRS})

    execute_process(COMMAND ${Cubelib_CONFIG} "--ldflags" OUTPUT_VARIABLE Cubelib_LIB_DIRS)
    string(STRIP "${Cubelib_LIB_DIRS}" Cubelib_LIB_DIRS)

    execute_process(COMMAND ${Cubelib_CONFIG} "--libs" OUTPUT_VARIABLE Cubelib_LIBRARIES )
    string(STRIP "${Cubelib_LIBRARIES}" Cubelib_LIBRARIES)

    set(Cubelib_LIBRARIES ${Cubelib_LIB_DIRS} ${Cubelib_LIBRARIES})

# cube pre version 4.4
elseif(CUBE_CONFIG OR EXISTS ${CUBE_CONFIG})

    execute_process(COMMAND ${CUBE_CONFIG} "--version" OUTPUT_VARIABLE Cubelib_VERSION)
    string(STRIP ${Cubelib_VERSION} Cubelib_VERSION)

    execute_process(COMMAND ${CUBE_CONFIG} "--cube-include-path" OUTPUT_VARIABLE Cubelib_INCLUDE_DIRS)
    string(REPLACE "\n" "" Cubelib_INCLUDE_DIRS ${Cubelib_INCLUDE_DIRS})
    string(REPLACE "-I" ";" Cubelib_INCLUDE_DIRS ${Cubelib_INCLUDE_DIRS})

    execute_process(COMMAND ${CUBE_CONFIG} "--cube-ldflags " OUTPUT_VARIABLE _LINK_LD_ARGS)
    string( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    foreach( _ARG ${_LINK_LD_ARGS} )
        if(${_ARG} MATCHES "^-L")
            string(REGEX REPLACE "^-L" "" _ARG ${_ARG})
            string(STRIP "${_ARG}" _ARG)
            set(Cubelib_LINK_DIRS ${Cubelib_LINK_DIRS} ${_ARG})
        endif()

        if(${_ARG} MATCHES "^-l")
            string(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
            string(STRIP "${_ARG}" _ARG)
            # NO_DEFAULT_PATH - We have to "filter" -lm, as g++ links it anyways. And then stuff explodes
            find_library(_Cubelib_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${Cubelib_LINK_DIRS} NO_DEFAULT_PATH
            )
            if(_Cubelib_LIB_FROM_ARG)
                set(Cubelib_LIBRARIES ${Cubelib_LIBRARIES} ${_Cubelib_LIB_FROM_ARG})
            endif()
            unset(_Cubelib_LIB_FROM_ARG CACHE)
        endif()
    endforeach(_ARG)
else()
    if (Cubelib_INC_DIR AND Cubelib_LIBS AND Cubelib_LIB_DIR)
        find_path(Cubelib_INCLUDE_DIRS NAMES Cubelib.h HINTS ${Cubelib_INC_DIR})

        string( REPLACE " " ";" _Cubelib_LIBS ${Cubelib_LIBS} )
        foreach( _ARG ${_Cubelib_LIBS} )
            if(${_ARG} MATCHES "^-l")
                string(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
                string(STRIP "${_ARG}" _ARG)
            endif()
            find_library(_Cubelib_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${Cubelib_LIB_DIR} NO_DEFAULT_PATH
            )
            if(_Cubelib_LIB_FROM_ARG)
                set(Cubelib_LIBRARIES ${Cubelib_LIBRARIES} ${_Cubelib_LIB_FROM_ARG})
            endif()
            unset(_Cubelib_LIB_FROM_ARG CACHE)
        endforeach(_ARG)
        unset(_Cubelib_LIBS CACHE)
    endif()
endif()
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cubelib
    REQUIRED_VARS Cubelib_LIBRARIES Cubelib_INCLUDE_DIRS
    FAIL_MESSAGE "Cubelib: No cube-config found. Try to find Cubelib manually by setting Cubelib_INC_DIR, Cubelib_LIB_DIR and Cubelib_LIBS."
)

mark_as_advanced(Cubelib_INCLUDE_DIRS Cubelib_LIBRARIES)
