# - Try to find libjpeg library
# Once done this will define
#
#  JPEG_FOUND and JPEG_alt_FOUND - system has libjpeg
#  JPEG_INCLUDE_DIR - the libjpeg include directory
#  JPEG_LIBRARIES - the required libraries
#

if (_vcpkg_dir)

	# using vcpkg, bypass usual find method to allwo both debug and release build
	
	find_path(JPEG_INCLUDE_DIR "jpeglib.h")
	
	
	if(JPEG_INCLUDE_DIR)
			
		get_filename_component(JPEG_BASE_PATH "${JPEG_INCLUDE_DIR}" DIRECTORY)
		set(JPEG_LIBRARY_RELEASE  "${JPEG_BASE_PATH}/lib/turbojpeg.lib")
		set(JPEG_LIBRARY_DEBUG  "${JPEG_BASE_PATH}/debug/lib/turbojpeg.lib")

		# Create JPEG_LIBRARIES from JPEG_LIBRARY_DEBUG and JPEG_LIBRARY_RELEASE
		select_library_configurations(JPEG)		
		
		get_filename_component(JPEG_R_PATH "${JPEG_LIBRARY_RELEASE}" DIRECTORY)
		get_filename_component(JPEG_D_PATH "${JPEG_LIBRARY_DEBUG}" DIRECTORY)		
		message(STATUS "Found JPEG (via vcpkg): release [${JPEG_R_PATH}]  debug [${JPEG_D_PATH}]")
		
		set(JPEG_FOUND 1)
		set(JPEG_alt_FOUND 1)

	else()
	
		message(FATAL_ERROR "libjpeg not found (using vcpkg)")

	endif()	
			
	
else()

	# call the official FindJPEG module
	find_package(JPEG REQUIRED)
	
	if (JPEG_FOUND)
		set (JPEG_alt_FOUND 1)
	endif()

endif()