#
# try to find fltk, use vcpkg if present
# if not, try to use fltk-config
# otherwise, fall back to the official cmake module FindFLTK
#
# if found, set FLTK_FOUND and FLTK_alt_FOUND
#
# define also
#
#     FLTK_OPTIONS
#     FLTK_LIBRARIES
#     FLTK_INCLUDE_DIR
#


macro(findfltklib FLTKLIBNAME FLTKLIBFILE)
	find_library(${FLTKLIBNAME}  ${FLTKLIBFILE})
	if(NOT ${FLTKLIBNAME})
		message(FATAL_ERROR "${FLTKLIBFILE}.lib not found (using vcpkg)")
	endif()	
endmacro()


if (_vcpkg_dir)

	# using vcpkg, bypass usual find method to allwo both debug and release build
	
	findfltklib(FLTK_MAIN_LIBRARY_RELEASE  fltk)
	findfltklib(FLTK_MAIN_LIBRARY_DEBUG  fltkd)
	
	findfltklib(FLTK_GL_LIBRARY_RELEASE  fltk_gl)
	findfltklib(FLTK_GL_LIBRARY_DEBUG  fltk_gld)
	
	findfltklib(FLTK_IMAGES_LIBRARY_RELEASE  fltk_images)
	findfltklib(FLTK_IMAGES_LIBRARY_DEBUG  fltk_imagesd)
	
	findfltklib(FLTK_FORMS_LIBRARY_RELEASE  fltk_forms)
	findfltklib(FLTK_FORMS_LIBRARY_DEBUG  fltk_formsd)

	find_path(FLTK_INCLUDE_DIR "FL/Fl.H")
	if(NOT FLTK_INCLUDE_DIR)
		message(FATAL_ERROR "FL/Fl.H not found (using vcpkg)")
	endif()	
		
	set(FLTK_LIBRARY_RELEASE "${FLTK_MAIN_LIBRARY_RELEASE}" "${FLTK_GL_LIBRARY_RELEASE}" "${FLTK_IMAGES_LIBRARY_RELEASE}" "${FLTK_FORMS_LIBRARY_RELEASE}" "comctl32" "opengl32" "glu32")
	set(FLTK_LIBRARY_DEBUG "${FLTK_MAIN_LIBRARY_DEBUG}" "${FLTK_GL_LIBRARY_DEBUG}" "${FLTK_IMAGES_LIBRARY_DEBUG}" "${FLTK_FORMS_LIBRARY_DEBUG}" "comctl32" "opengl32" "glu32")
		
	# Create FLTK_LIBRARIES from FLTK_LIBRARY_DEBUG and FLTK_LIBRARY_RELEASE
	select_library_configurations(FLTK)		
		
	get_filename_component(FLTK_R_PATH "${FLTK_MAIN_LIBRARY_RELEASE}" DIRECTORY)
	get_filename_component(FLTK_D_PATH "${FLTK_MAIN_LIBRARY_DEBUG}" DIRECTORY)		
	message(STATUS "Found FLKT (via vcpkg): release [${FLTK_R_PATH}]  debug [${FLTK_D_PATH}]")
		
	set(FLTK_FOUND 1)
	set(FLTK_alt_FOUND 1)

	mark_as_advanced
	mark_as_advanced(FLTK_D_PATH)
	mark_as_advanced(FLTK_R_PATH)
	mark_as_advanced(FLTK_MAIN_LIBRARY_RELEASE)
	mark_as_advanced(FLTK_MAIN_LIBRARY_DEBUG)
	mark_as_advanced(FLTK_GL_LIBRARY_RELEASE)
	mark_as_advanced(FLTK_GL_LIBRARY_DEBUG)
	mark_as_advanced(FLTK_IMAGES_LIBRARY_RELEASE)
	mark_as_advanced(FLTK_IMAGES_LIBRARY_DEBUG)
	mark_as_advanced(FLTK_FORMS_LIBRARY_RELEASE)
	mark_as_advanced(FLTK_FORMS_LIBRARY_DEBUG)
	
	
else()

	# try to find fltk-config	
	find_program(FLTKCONFIG_PROG fltk-config)

	if (FLTKCONFIG_PROG)

		set(FLTK_FOUND 1)
		set(FLTK_alt_FOUND 1)

		execute_process(COMMAND "${FLTKCONFIG_PROG}" "--cxxflags" OUTPUT_VARIABLE FLTK_OPTIONS OUTPUT_STRIP_TRAILING_WHITESPACE)
			string(REGEX REPLACE "\n$" "" FLTK_OPTIONS "${FLTK_OPTIONS}")
			string(STRIP ${FLTK_OPTIONS} FLTK_OPTIONS)

		execute_process(COMMAND "${FLTKCONFIG_PROG}" "--ldflags" OUTPUT_VARIABLE FLTK_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
			string(REGEX REPLACE "\n$" "" FLTK_LIBRARIES "${FLTK_LIBRARIES}")
			string(STRIP ${FLTK_LIBRARIES} FLTK_LIBRARIES)

		set(FLTK_INCLUDE_DIR "")
		message(STATUS "Found FLKT (via fltk-config): ${FLTKCONFIG_PROG}")

	else ()

		# try to find fltk with the cmake bundled FindFLTK module
		message(STATUS "fltk-config NOT found, looking for FLTK...")
		set(FLTK_SKIP_FLUID TRUE)
		set(FLTK_SKIP_FORMS TRUE)
		find_package(FLTK)
		if (FLTK_FOUND)
			set(FLTK_alt_FOUND 1)
		endif()

	endif()

	mark_as_advanced(FLTKCONFIG_PROG)

endif()



