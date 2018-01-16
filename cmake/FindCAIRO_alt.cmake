# - Try to find the cairo library
# Once done this will define
#
#  CAIRO_FOUND and CAIRO_alt_FOUND - system has cairo
#  CAIRO_INCLUDE_DIRS - the cairo include directory
#  CAIRO_LIBRARIES - the required libraries
#


macro(find_library_flag LIBNAME LIBFILE FLAG)

	find_library(TMPLIBNAME  ${LIBFILE})
	if(NOT TMPLIBNAME)
		set(FLAG 0)
	else()
		set(LIBNAME ${LIBNAME} ${TMPLIBNAME})
	endif()	
	message(STATUS "TMPLIBNAME : ${TMPLIBNAME}")
endmacro()



if (VCPKG_TOOLCHAIN)

	# using vcpkg, bypass usual find method to manage both debug and release build
	
	set (FOUND_ALL_LIB 1)
	set (CAIRO_LIBRARY_RELEASE "")
	set (CAIRO_LIBRARY_DEBUG "")
	
	find_library_flag(CAIRO_LIBRARY_RELEASE  cairo FOUND_ALL_LIB)
	message(STATUS "CAIRO_LIBRARY_RELEASE : ${CAIRO_LIBRARY_RELEASE}")
	message(STATUS "CAIRO_LIBRARY_DEBUG : ${CAIRO_LIBRARY_DEBUG}")
	find_library_flag(CAIRO_LIBRARY_RELEASE  pixman FOUND_ALL_LIB)
	message(STATUS "CAIRO_LIBRARY_RELEASE : ${CAIRO_LIBRARY_RELEASE}")
	message(STATUS "CAIRO_LIBRARY_DEBUG : ${CAIRO_LIBRARY_DEBUG}")
	find_library_flag(CAIRO_LIBRARY_DEBUG  cairod FOUND_ALL_LIB)
	message(STATUS "CAIRO_LIBRARY_RELEASE : ${CAIRO_LIBRARY_RELEASE}")
	message(STATUS "CAIRO_LIBRARY_DEBUG : ${CAIRO_LIBRARY_DEBUG}")
	find_library_flag(CAIRO_LIBRARY_DEBUG  pixmand FOUND_ALL_LIB)
	message(STATUS "CAIRO_LIBRARY_RELEASE : ${CAIRO_LIBRARY_RELEASE}")
	message(STATUS "CAIRO_LIBRARY_DEBUG : ${CAIRO_LIBRARY_DEBUG}")
	
	if (NOT FOUND_ALL_LIB)
		message(status "cairo NOT found")
	endif()
	
	find_path(CAIRO_INCLUDE_DIRS "cairo.h")

	if(CAIRO_INCLUDE_DIRS AND CAIRO_MAIN_LIBRARY_RELEASE AND CAIRO_MAIN_LIBRARY_DEBUG AND CAIRO_PIXMAN_LIBRARY_RELEASE AND CAIRO_PIXMAN_LIBRARY_DEBUG)
	
		set(CAIRO_LIBRARY_RELEASE "{CAIRO_MAIN_LIBRARY_RELEASE}" "{CAIRO_PIXMAN_LIBRARY_RELEASE}")
		set(CAIRO_LIBRARY_DEBUG "{CAIRO_MAIN_LIBRARY_DEBUG}" "{CAIRO_PIXMAN_LIBRARY_DEBUG}")
		
		# Create CAIRO_LIBRARIES from CAIRO_LIBRARY_DEBUG and CAIRO_LIBRARY_RELEASE
		select_library_configurations(CAIRO)		
		
		get_filename_component(CAIRO_R_PATH "${CAIRO_MAIN_LIBRARY_RELEASE}" DIRECTORY)
		get_filename_component(CAIRO_D_PATH "${CAIRO_MAIN_LIBRARY_DEBUG}" DIRECTORY)		
		message(STATUS "Found CAIRO (via vcpkg): release [${CAIRO_R_PATH}]  debug [${CAIRO_D_PATH}]")
		
		set(CAIRO_FOUND 1)
		set(CAIRO_alt_FOUND 1)

	else()
	
		message(STATUS "CAIRO NOT found.)")

	endif()	
			
	
else()

	find_package(PkgConfig)

	if (PKG_CONFIG_FOUND)
		pkg_search_module(CAIRO cairo)
	endif()

	if (NOT CAIRO_FOUND AND NOT PKG_CONFIG_FOUND)
		find_path(CAIRO_INCLUDE_DIRS cairo.h)
		find_library(CAIRO_LIBRARIES cairo)

		if (CAIRO_LIBRARIES AND CAIRO_INCLUDE_DIRS)
			set(CAIRO_FOUND 1)
			message(STATUS "Found Cairo: ${CAIRO_LIBRARIES}")
		else()	
			message(SEND_ERROR "Could not find Cairo")
		endif()
	endif()

	if (CAIRO_FOUND)
		set(CAIRO_alt_FOUND 1)
	endif()	

endif()


mark_as_advanced(CAIRO_LIBRARIES CAIRO_INCLUDE_DIRS)


