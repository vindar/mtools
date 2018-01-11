# - Try to find the FLTK library via fltk-config
#   Otherwise, call  the classic FindFLTK script
#

find_program(FLTKCONFIG_PROG fltk-config)

if (FLTKCONFIG_PROG)

	set(FLTK_FOUND 1)
	set(FltkConfig_FOUND 1)

	execute_process(COMMAND "${FLTKCONFIG_PROG}" "--cxxflags" OUTPUT_VARIABLE FLTK_OPTIONS OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX REPLACE "\n$" "" FLTK_OPTIONS "${FLTK_OPTIONS}")
        string(STRIP ${FLTK_OPTIONS} FLTK_OPTIONS)

	execute_process(COMMAND "${FLTKCONFIG_PROG}" "--ldflags" OUTPUT_VARIABLE FLTK_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "\n$" "" FLTK_LIBRARIES "${FLTK_LIBRARIES}")
	string(STRIP ${FLTK_LIBRARIES} FLTK_LIBRARIES)

	set(FLTK_INCLUDE_DIR "")
	message(STATUS "Found FLKT (via fltk-config): ${FLTKCONFIG_PROG}")

else ()

	message(STATUS "fltk-config NOT found, looking for FLTK...")
	find_package(FLTK)
	if (FLTK_FOUND)
		set(FLTKCONFIG_PROG 1)
	endif()

endif()

