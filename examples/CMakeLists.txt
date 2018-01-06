###############################################################################
###############################################################################
#
#                       CMakeList for the examples. 
#
###############################################################################
###############################################################################
cmake_minimum_required(VERSION 3.10.1)

   
###############################################################################
# If using Windows, look for vcpkg...
###############################################################################
if( WIN32 )
	# test if the env variable VCPKG_DIR is defined
	if (DEFINED ENV{VCPKG_DIR})
		#yes, we use it to find vcpkg.cmake
		string(REPLACE "\\" "/" _vcpkg_dir "$ENV{VCPKG_DIR}")
	else ()
		# no try to find vcpkg.exe
		find_file( _vcpkg_exe "vcpkg.exe" PATHS ENV PATH)
		if (_vcpkg_exe)			
			get_filename_component(_vcpkg_dir ${_vcpkg_exe} DIRECTORY)
		endif()		
	endif()	
	if (_vcpkg_dir)	
		set(CMAKE_TOOLCHAIN_FILE 
			"${_vcpkg_dir}/scripts/buildsystems/vcpkg.cmake")					
		message(STATUS "Windows: vcpkg found at [${_vcpkg_dir}]")	
	else()
		message(STATUS "Windows: vcpkg not found.")	
	endif()
endif()


###############################################################################
# Project name
###############################################################################
project(examples)


message(STATUS " EXAMPLES CONFIGURATIONS ")

add_executable(eden ./Eden/main.cpp)
target_link_libraries(eden mtools)


###############################################################################
#end of file
###############################################################################
