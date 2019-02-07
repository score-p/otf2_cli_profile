if(RAPIDJSON_INCLUDE_DIRS)
	set(RAPIDJSON_FIND_QUIETLY TRUE)
endif()

find_path(RAPIDJSON_INCLUDE_DIRS NAMES rapidjson/rapidjson.h HINTS ${RAPIDJSON_INC_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RAPIDJSON
	REQUIRED_VARS RAPIDJSON_INCLUDE_DIRS
    FAIL_MESSAGE "RAPIDJSON: rapidjson package not found."
)
mark_as_advanced(RAPIDJSON_INCLUDE_DIRS)
