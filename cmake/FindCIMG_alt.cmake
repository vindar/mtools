# - Try to find the CImg library
# Once done this will define
#
#  CIMG_FOUND and CIMG_alt_FOUND
#  CIMG_INCLUDE_DIR - the cimg include directory
#

find_path(CIMG_INCLUDE_DIR CImg.h)

if(CIMG_INCLUDE_DIR)
	set(CIMG_FOUND 1)
	message(STATUS "Found CImg: ${CIMG_INCLUDE_DIR}")
else()	
	message(FATAL_ERROR "Could not find CImg")
endif()

if (CIMG_FOUND)
	set(CIMG_alt_FOUND 1)
endif()

mark_as_advanced(CIMG_INCLUDE_DIR)

