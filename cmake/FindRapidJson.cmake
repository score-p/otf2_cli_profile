if(RapidJson_INCLUDE_DIRS)
	set(RapidJson_FIND_QUIETLY TRUE)
endif()

find_path(RapidJson_INCLUDE_DIRS NAMES rapidjson/rapidjson.h HINTS ${RapidJson_INC_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RapidJson
	REQUIRED_VARS RapidJson_INCLUDE_DIRS
    FAIL_MESSAGE "RapidJson: rapidjson package not found."
)
mark_as_advanced(RapidJson_INCLUDE_DIRS)
