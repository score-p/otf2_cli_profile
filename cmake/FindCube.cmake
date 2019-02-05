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
IF (CUBE_LIBRARIES AND CUBE_INCLUDE_DIRS)
  set (CUBE_FIND_QUIETLY TRUE)
ENDIF (CUBE_LIBRARIES AND CUBE_INCLUDE_DIRS)

# Find Cube
FIND_PROGRAM(CUBELIB_CONFIG NAMES cubelib-config
    PATHS
    /opt/cubelib/bin
    HINTS
    ${PATH}
)
IF(NOT CUBELIB_CONFIG OR NOT EXISTS ${CUBELIB_CONFIG})

    FIND_PROGRAM(CUBE_CONFIG NAMES cube-config
        PATHS
        /opt/cube/bin
        HINTS
        ${PATH}
    )
ENDIF()

# cube version 4.4 or later
IF(CUBELIB_CONFIG OR EXISTS ${CUBELIB_CONFIG})

    MESSAGE(STATUS "CUBE: cubelib-config found. (using ${CUBELIB_CONFIG}")
    execute_process(COMMAND ${CUBELIB_CONFIG} "--version" OUTPUT_VARIABLE CUBE_VERSION)

    STRING(STRIP "${CUBE_VERSION}" CUBE_VERSION)

    execute_process(COMMAND ${CUBELIB_CONFIG} "--include" OUTPUT_VARIABLE CUBE_INCLUDE_DIRS)
    STRING(REPLACE "\n" "" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})
    STRING(REPLACE "-I" ";" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})

    execute_process(COMMAND ${CUBELIB_CONFIG} "--ldflags" OUTPUT_VARIABLE _LINK_LD_ARGS)
    STRING( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    FOREACH( _ARG ${_LINK_LD_ARGS} )
        IF(${_ARG} MATCHES "^-L")
            STRING(REGEX REPLACE "^-L" "" _ARG "${_ARG}")
            STRING(STRIP "${_ARG}" _ARG)
            SET(CUBE_LINK_DIRS ${CUBE_LINK_DIRS} ${_ARG})
        ENDIF(${_ARG} MATCHES "^-L")
    ENDFOREACH(_ARG)

    execute_process(COMMAND ${CUBELIB_CONFIG} "--libs" OUTPUT_VARIABLE _ARG)

    STRING(STRIP "${_ARG}" _ARG)
    STRING(REGEX REPLACE "^-l" "" _ARG "${_ARG}")

    FIND_LIBRARY(CUBE_LIB NAMES ${_ARG}
        HINTS ${CUBE_LINK_DIRS} NO_DEFAULT_PATH
    )
    SET(CUBE_LIBRARIES ${CUBE_LIBRARIES} ${CUBE_LIB})

# cube pre version 4.4
ELSEIF(CUBE_CONFIG OR EXISTS ${CUBE_CONFIG})
    message(STATUS "CUBE: cube-config found. (using ${CUBE_CONFIG})")

    execute_process(COMMAND ${CUBE_CONFIG} "--version" OUTPUT_VARIABLE CUBE_VERSION)
    STRING(STRIP ${CUBE_VERSION} CUBE_VERSION)

    execute_process(COMMAND ${CUBE_CONFIG} "--cube-include-path" OUTPUT_VARIABLE CUBE_INCLUDE_DIRS)
    STRING(REPLACE "\n" "" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})
    STRING(REPLACE "-I" ";" CUBE_INCLUDE_DIRS ${CUBE_INCLUDE_DIRS})

    execute_process(COMMAND ${CUBE_CONFIG} "--cube-ldflags " OUTPUT_VARIABLE _LINK_LD_ARGS)
    STRING( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    FOREACH( _ARG ${_LINK_LD_ARGS} )
        IF(${_ARG} MATCHES "^-L")
            STRING(REGEX REPLACE "^-L" "" _ARG ${_ARG})
            STRING(STRIP "${_ARG}" _ARG)
            SET(CUBE_LINK_DIRS ${CUBE_LINK_DIRS} ${_ARG})
        ENDIF(${_ARG} MATCHES "^-L")

        IF(${_ARG} MATCHES "^-l")
            STRING(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
            STRING(STRIP "${_ARG}" _ARG)
            # NO_DEFAULT_PATH - We have to "filter" -lm, as g++ links it anyways. And then stuff explodes
            FIND_LIBRARY(_CUBE_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${CUBE_LINK_DIRS} NO_DEFAULT_PATH
            )
            IF(_CUBE_LIB_FROM_ARG)
                SET(CUBE_LIBRARIES ${CUBE_LIBRARIES} ${_CUBE_LIB_FROM_ARG})
            ENDIF(_CUBE_LIB_FROM_ARG)
            UNSET(_CUBE_LIB_FROM_ARG CACHE)
        ENDIF(${_ARG} MATCHES "^-l")
ENDFOREACH(_ARG)
ELSE()
   MESSAGE(STATUS "CUBE: No cube-config found. Try to find Cube manually by setting CUBE_INC_DIR, CUBE_LIB_DIR and CUBE_LIBS.")

    if (CUBE_INC_DIR AND CUBE_LIBS AND CUBE_LIB_DIR)
        find_path(CUBE_INCLUDE_DIRS NAMES Cube.h HINTS ${CUBE_INC_DIR})

        STRING( REPLACE " " ";" _CUBE_LIBS ${CUBE_LIBS} )
        FOREACH( _ARG ${_CUBE_LIBS} )
            IF(${_ARG} MATCHES "^-l")
                STRING(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
                STRING(STRIP "${_ARG}" _ARG)
            ENDIF(${_ARG} MATCHES "^-l")
            FIND_LIBRARY(_CUBE_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${CUBE_LIB_DIR} NO_DEFAULT_PATH
            )
            IF(_CUBE_LIB_FROM_ARG)
                SET(CUBE_LIBRARIES ${CUBE_LIBRARIES} ${_CUBE_LIB_FROM_ARG})
            ENDIF(_CUBE_LIB_FROM_ARG)
            UNSET(_CUBE_LIB_FROM_ARG CACHE)
        ENDFOREACH(_ARG)
        UNSET(_CUBE_LIBS CACHE)
    endif(CUBE_INC_DIR AND CUBE_LIBS AND CUBE_LIB_DIR)
ENDIF()
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CUBE
    REQUIRED_VARS CUBE_LIBRARIES CUBE_INCLUDE_DIRS
)

mark_as_advanced(CUBE_INCLUDE_DIRS CUBE_LIBRARIES)