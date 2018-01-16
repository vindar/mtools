# Try to find the CImg library
# This search module defines
#
#  CIMG_FOUND and CIMG_alt_FOUND if the library id found
#  CIMG_INCLUDE_DIR - the cimg include directory
#

if (CIMG_alt_FOUND) 
	return()
endif()

find_path(CIMG_INCLUDE_DIR CImg.h)

if(CIMG_INCLUDE_DIR)
	set(CIMG_FOUND 1)
	message(STATUS "Found CImg: [${CIMG_INCLUDE_DIR}/CImg.h]")
endif()

if (CIMG_FOUND)
	set(CIMG_alt_FOUND 1)
else()
	if (CIMG_alt_FIND_REQUIRED)
		message(SEND_ERROR "CImg NOT found")
	else()
		message(STATUS "CImg NOT found.")
	endif()
endif()

mark_as_advanced(CIMG_INCLUDE_DIR)

