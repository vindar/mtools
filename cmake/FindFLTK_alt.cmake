#
# try to find fltk, use vcpkg if present
# if not, try to use fltk-config
# otherwise, fall back to the official cmake module FindFLTK
#
# if found, set FLTK_FOUND and FLTK_alt_FOUND
#
#     FLTK_OPTIONS      - list of option to pass to the compiler
#     FLTK_LIBRARIES    - list of libraries
#     FLTK_INCLUDE_DIR  - fltk include directory (contains the /FL sub-directory)
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


if (FLTK_alt_FOUND) 
	return()
endif()


if (VCPKG_TOOLCHAIN)
	# use vcpkg to find fltk
	set (FOUND_ALL_LIB 1)
	set (FLTKGL_LIB 1)
	set (FLTK_LIBRARY_RELEASE "")
	set (FLTK_LIBRARY_DEBUG "")
	
	find_library_flag(FLTK_LIBRARY_RELEASE  fltk FOUND_ALL_LIB)
	find_library_flag(FLTK_LIBRARY_RELEASE  fltk_images FOUND_ALL_LIB)
	find_library_flag(FLTK_LIBRARY_RELEASE  fltk_forms FOUND_ALL_LIB)
	find_library_flag(FLTK_LIBRARY_RELEASE  fltk_gl FLTKGL_LIB)
	find_library_flag(FLTK_LIBRARY_DEBUG  fltkd FOUND_ALL_LIB)
	find_library_flag(FLTK_LIBRARY_DEBUG  fltk_imagesd FOUND_ALL_LIB)
	find_library_flag(FLTK_LIBRARY_DEBUG  fltk_formsd FOUND_ALL_LIB)
	find_library_flag(FLTK_LIBRARY_DEBUG  fltk_gld FLTKGL_LIB)
		
	find_path(FLTK_INCLUDE_DIR "FL/Fl.H")
	
	if (FOUND_ALL_LIB AND FLTK_INCLUDE_DIR)		
		set(FLTK_LIBRARY_RELEASE "${FLTK_LIBRARY_RELEASE}" "comctl32")
		set(FLTK_LIBRARY_DEBUG "${FLTK_LIBRARY_DEBUG}" "comctl32")		
		if (FLTKGL_LIB)
			set(FLTK_LIBRARY_RELEASE "${FLTK_LIBRARY_RELEASE}" "opengl32" "glu32")
			set(FLTK_LIBRARY_DEBUG "${FLTK_LIBRARY_DEBUG}" "opengl32" "glu32")
		endif()	
		select_library_configurations(FLTK) # create FLTK_LIBRARIES from FLTK_LIBRARY_DEBUG and FLTK_LIBRARY_RELEASE		
		list (GET FLTK_LIBRARY_RELEASE 0 RELPATH)
		list (GET FLTK_LIBRARY_DEBUG 0 DEBPATH)		
		message(STATUS "Found FLTK (via vcpkg): release [${RELPATH}]  debug [${DEBPATH}]")		
		set(FLTK_FOUND 1)
	endif()
endif()


if (NOT FLTK_FOUND)

	# try to find fltk-config	
	find_program(FLTKCONFIG_PROG fltk-config)

	if (FLTKCONFIG_PROG)

		execute_process(COMMAND "${FLTKCONFIG_PROG}" "--cxxflags" OUTPUT_VARIABLE FLTK_OPTIONS OUTPUT_STRIP_TRAILING_WHITESPACE)
			string(REGEX REPLACE "\n$" "" FLTK_OPTIONS "${FLTK_OPTIONS}")
			string(STRIP ${FLTK_OPTIONS} FLTK_OPTIONS)

		execute_process(COMMAND "${FLTKCONFIG_PROG}" "--ldflags" OUTPUT_VARIABLE FLTK_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
			string(REGEX REPLACE "\n$" "" FLTK_LIBRARIES "${FLTK_LIBRARIES}")
			string(STRIP ${FLTK_LIBRARIES} FLTK_LIBRARIES)

		set(FLTK_INCLUDE_DIR "")
		message(STATUS "Found FLKT (via fltk-config): [${FLTKCONFIG_PROG}]")
		set(FLTK_FOUND 1)

	else ()

		# try to find fltk with the official FindFLTK module
		message(STATUS "fltk-config NOT found, looking for FLTK...")
		set(FLTK_SKIP_FLUID TRUE)
		set(FLTK_SKIP_FORMS TRUE)
		find_package(FLTK)

	endif()

endif()


if (FLTK_FOUND)
	set(FLTK_alt_FOUND 1)
else()
	if (FLTK_alt_FIND_REQUIRED)
		message(SEND_ERROR "FLTK NOT found")
	else()
		message(STATUS "FLTK NOT found.")
	endif()
endif()


mark_as_advanced(TMPLIBNAME)


