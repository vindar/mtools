# - Try to find the CImg library
# Once done this will define
#
#  CIMG_FOUND and CIMG_alt_FOUND
#  CIMG_INCLUDE_DIR - the cimg include directory
#

FIND_PATH(CIMG_INCLUDE_DIR CImg.h)

# Report results
IF(CIMG_INCLUDE_DIR)
	SET(CIMG_FOUND 1)
	MESSAGE(STATUS "Found CImg: ${CIMG_INCLUDE_DIR}")
ELSE(CIMG_INCLUDE_DIR)	
	IF(CImg_FIND_REQUIRED)
		MESSAGE(SEND_ERROR "Could not find CImg")
	ELSE(CImg_FIND_REQUIRED)
		MESSAGE(STATUS "Could not find CImg")	
	ENDIF(CImg_FIND_REQUIRED)
ENDIF(CIMG_INCLUDE_DIR)

IF (CIMG_FOUND)
	SET(CIMG_alt_FOUND 1)
ENDIF()

# Hide advanced variables from CMake GUIs
MARK_AS_ADVANCED(CIMG_INCLUDE_DIR)
