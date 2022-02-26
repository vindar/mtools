# Try to find the TGX library (source)
# This search module defines
#
#  TGX_FOUND and TGX_alt_FOUND if the library source are found
#  TGX_INCLUDE_DIR - the tgx include directory
#

if (TGX_FOUND) 
	return()
endif()

# look for tgx in a directory named "tgx" next to "mtools" main directory.  "${CMAKE_SOURCE_DIR}/../tgx/src/"
find_path(TGX_INCLUDE_DIR tgx.h PATHS "${CMAKE_SOURCE_DIR}/../" PATH_SUFFIXES "tgx/src/")

if(TGX_INCLUDE_DIR)
	set(TGX_FOUND 1)
	message(STATUS "Found TGX: [${TGX_INCLUDE_DIR}/tgx.h]")
endif()

if (TGX_FOUND)
	set(TGX_alt_FOUND 1)
else()
	if (TGX_FIND_REQUIRED)
		message(SEND_ERROR "TGX NOT found")
	else()
		message(STATUS "TGX NOT found.")
	endif()
endif()

mark_as_advanced(TGX_INCLUDE_DIR)

