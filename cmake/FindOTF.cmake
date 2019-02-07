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


# Find Open Trace Format library
if (OTF_LIBRARIES AND OTF_INCLUDE_DIRS)
  set (OTF_FIND_QUIETLY TRUE)
endif()

FIND_PROGRAM(OTF_CONFIG NAMES otfconfig
    PATHS
    /opt/otf/bin
    HINTS
    ${PATH}
)

if(NOT OTF_CONFIG OR NOT EXISTS ${OTF_CONFIG})

    if (OTF_INC_DIR AND OTF_LIBS AND OTF_LIB_DIR)
        find_path(OTF_INCLUDE_DIRS NAMES otf.h HINTS ${OTF_INC_DIR})

        STRING( REPLACE " " ";" _OTF_LIBS ${OTF_LIBS} )
        foreach( _ARG ${_OTF_LIBS} )
            if(${_ARG} MATCHES "^-l")
                STRING(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
                STRING(STRIP "${_ARG}" _ARG)
            endif()
            FIND_LIBRARY(_OTF_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${OTF_LIB_DIR} NO_DEFAULT_PATH
            )
            if(_OTF_LIB_FROM_ARG)
                SET(OTF_LIBRARIES ${OTF_LIBRARIES} ${_OTF_LIB_FROM_ARG})
            endif()
            UNSET(_OTF_LIB_FROM_ARG CACHE)
        endforeach(_ARG)
        UNSET(_OTF_LIBS CACHE)
    endif()

else()

    execute_process(COMMAND ${OTF_CONFIG} "--version" OUTPUT_VARIABLE OTF_VERSION)
    STRING(STRIP ${OTF_VERSION} OTF_VERSION)

    execute_process(COMMAND ${OTF_CONFIG} "--includes" OUTPUT_VARIABLE OTF_INCLUDE_DIRS)
    STRING(REPLACE "\n" "" OTF_INCLUDE_DIRS ${OTF_INCLUDE_DIRS})
    STRING(REPLACE "-I" ";" OTF_INCLUDE_DIRS ${OTF_INCLUDE_DIRS})

    execute_process(COMMAND ${OTF_CONFIG} "--libs" OUTPUT_VARIABLE _LINK_LD_ARGS)
    STRING( REPLACE " " ";" _LINK_LD_ARGS ${_LINK_LD_ARGS} )
    foreach( _ARG ${_LINK_LD_ARGS} )
        if(${_ARG} MATCHES "^-L")
            STRING(REGEX REPLACE "^-L" "" _ARG ${_ARG})
            STRING(STRIP "${_ARG}" _ARG)
            SET(OTF_LINK_DIRS ${OTF_LINK_DIRS} ${_ARG})
        endif()

        if(${_ARG} MATCHES "^-l")
            STRING(REGEX REPLACE "^-l" "" _ARG "${_ARG}")
            STRING(STRIP "${_ARG}" _ARG)
            # NO_DEFAULT_PATH - We have to "filter" -lm, as g++ links it anyways. And then stuff explodes
            FIND_LIBRARY(_OTF_LIB_FROM_ARG NAMES ${_ARG}
                HINTS ${OTF_LINK_DIRS} NO_DEFAULT_PATH
            )
            if(_OTF_LIB_FROM_ARG)
                SET(OTF_LIBRARIES ${OTF_LIBRARIES} ${_OTF_LIB_FROM_ARG})
            endif()
            UNSET(_OTF_LIB_FROM_ARG CACHE)
        endif()
    endforeach(_ARG)


endif()

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OTF
    REQUIRED_VARS OTF_LIBRARIES OTF_INCLUDE_DIRS
    FAIL_MESSAGE "OTF: No otfconfig found. Try to find OTF manually by setting OTF_INC_DIR, OTF_LIB_DIR and OTF_LIBS."
)

mark_as_advanced(OTF_INCLUDE_DIRS OTF_LIBRARIES)
