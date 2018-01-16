# Try to find the Cairo library
# This search module defines
#
#  CAIRO_FOUND and CAIRO_alt_FOUND if the library is found
#  CAIRO_INCLUDE_DIRS - the cairo include directory
#  CAIRO_LIBRARIES    - the required libraries
#


macro(find_library_flag LIBNAME LIBFILE FLAG)
	set(TMPLIBNAME "TMPLIBNAME-NOTFOUND")
	find_library(TMPLIBNAME ${LIBFILE})
	if(NOT TMPLIBNAME)
		set(FLAG 0)
	else()
		set(${LIBNAME} ${${LIBNAME}} ${TMPLIBNAME})
	endif()	
endmacro()


if (CAIRO_alt_FOUND) 
	return()
endif()


if (VCPKG_TOOLCHAIN)
	# use vcpkg to find cairo
	set (FOUND_ALL_LIB 1)
	set (CAIRO_LIBRARY_RELEASE "")
	set (CAIRO_LIBRARY_DEBUG "")
	
	find_library_flag(CAIRO_LIBRARY_RELEASE  cairo FOUND_ALL_LIB)
	find_library_flag(CAIRO_LIBRARY_RELEASE  pixman-1 FOUND_ALL_LIB)
	find_library_flag(CAIRO_LIBRARY_DEBUG  cairod FOUND_ALL_LIB)
	find_library_flag(CAIRO_LIBRARY_DEBUG  pixman-1d FOUND_ALL_LIB)

	find_path(CAIRO_INCLUDE_DIRS "cairo.h")
	
	if (FOUND_ALL_LIB AND CAIRO_INCLUDE_DIRS)	
		select_library_configurations(CAIRO) # create CAIRO_LIBRARIES from CAIRO_LIBRARY_DEBUG and CAIRO_LIBRARY_RELEASE		
		list (GET CAIRO_LIBRARY_RELEASE 0 RELPATH)
		list (GET CAIRO_LIBRARY_DEBUG 0 DEBPATH)		
		message(STATUS "Found CAIRO (via vcpkg): release [${RELPATH}]  debug [${DEBPATH}]")		
		set(CAIRO_FOUND 1)
	endif()
endif()


if (NOT CAIRO_FOUND)
	#try to find cairo the usual way
	
	find_package(PkgConfig QUIET)

	if (PKG_CONFIG_FOUND)
	
		pkg_search_module(CAIRO cairo QUIET)
		if (CAIRO_FOUND)
			list (GET CAIRO_LIBRARIES 0 RELPATH)
			message(STATUS "Found CAIRO (pkg-config): [${RELPATH}]")		
			set(CAIRO_FOUND 1)
		endif()
	
	else()
	
		set (FOUND_ALL_LIB 1)
		set (CAIRO_LIBRARY "")
	
		find_library_flag(CAIRO_LIBRARY  cairo FOUND_ALL_LIB)
		find_library_flag(CAIRO_LIBRARY pixman-1 FOUND_ALL_LIB)
		find_path(CAIRO_INCLUDE_DIRS "cairo.h")
	
		if (FOUND_ALL_LIB AND CAIRO_INCLUDE_DIRS)
			set(CAIRO_LIBRARIES ${CAIRO_LIBRARY})
			list (GET CAIRO_LIBRARY 0 RELPATH)
			message(STATUS "Found CAIRO (direct search): [${RELPATH}]")		
			set(CAIRO_FOUND 1)
		endif()
		
	endif()
endif()


if (CAIRO_FOUND)
	set(CAIRO_alt_FOUND 1)
else()
	if (CAIRO_alt_FIND_REQUIRED)
		message(SEND_ERROR "Cairo NOT found")
	else()
		message(STATUS "Cairo NOT found.")
	endif()
endif()

mark_as_advanced(CAIRO_LIBRARIES CAIRO_INCLUDE_DIRS)
mark_as_advanced(TMPLIBNAME)



