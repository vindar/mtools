# Try to find libjpeg library
# This search module defines
#
#  JPEG_FOUND and JPEG_alt_FOUND if the library is found
#  JPEG_INCLUDE_DIR - the libjpeg include directory
#  JPEG_LIBRARIES   - the required libraries
#


if (JPEG_alt_FOUND) 
	return()
endif()

if (VCPKG_TOOLCHAIN)

	# use vcpkg to find libjpeg
	find_path(JPEG_INCLUDE_DIR "jpeglib.h")

	if(JPEG_INCLUDE_DIR)
		# debug and relase lib have the same name so we find them using the known vcpkg layout and 
		# their respective location wrt the include directory
		get_filename_component(JPEG_BASE_PATH "${JPEG_INCLUDE_DIR}" DIRECTORY)
		set(JPEG_LIBRARY_RELEASE  "${JPEG_BASE_PATH}/lib/jpeg.lib")
		set(JPEG_LIBRARY_DEBUG  "${JPEG_BASE_PATH}/debug/lib/jpeg.lib")

		if (EXISTS ${JPEG_LIBRARY_RELEASE} AND EXISTS ${JPEG_LIBRARY_DEBUG}) # make sure the libraries do exist
			select_library_configurations(JPEG) # Create JPEG_LIBRARIES from JPEG_LIBRARY_DEBUG and JPEG_LIBRARY_RELEASE				
			message(STATUS "Found JPEG (via vcpkg): release [${JPEG_LIBRARY_RELEASE}]  debug [${JPEG_LIBRARY_DEBUG}]")		
			set(JPEG_FOUND 1)
		endif()
	endif()	
	
endif()

if (NOT JPEG_FOUND) 
	# use the official FindJPEG module
	find_package(JPEG)
endif()


if (JPEG_FOUND)
	set(JPEG_alt_FOUND 1)
else()
	if (JPEG_alt_FIND_REQUIRED)
		message(SEND_ERROR "libjpeg NOT found")
	else()
		message(STATUS "libjpeg NOT found.")
	endif()
endif()


mark_as_advanced(JPEG_INCLUDE_DIR)


