###############################################################################
###############################################################################
#
#                       CMakeList for mtools. 
#
###############################################################################
###############################################################################
cmake_minimum_required(VERSION 3.9)

message(STATUS "")
message(STATUS "*********************************")
message(STATUS "* STARTING MTOOLS CONFIGURATION *")
message(STATUS "*********************************")
message(STATUS "")


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
# Read mtools version
###############################################################################
file(READ VERSION MTOOLS_VERSION 10)
message(STATUS "building mtools version ${MTOOLS_VERSION}")


###############################################################################
# Project name
###############################################################################
project(mtools)


###############################################################################
# Locate the external libraries needed. 
###############################################################################

# load the custom find modules (for cimg and cairo). 
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/modules )


###########
# REQUIRED
###########

# zlib 
find_package(ZLIB REQUIRED)

# fltk, but we do not need fluid nor forms
set(FLTK_SKIP_FLUID TRUE)
set(FLTK_SKIP_FORMS TRUE)
find_package(FLTK REQUIRED)

# libpng
find_package(PNG REQUIRED)

# libjpeg
find_package(JPEG REQUIRED)

# cairo
find_package(CAIRO REQUIRED)

#cimg
find_package(CIMG REQUIRED)


###########
# OPTIONAL
###########

#opengl 
find_package(OpenGL)

#opencl
find_package(OpenCL)

#openmp
find_package(OpenMP)


###############################################################################
# query instruction set
###############################################################################

find_package(SSE)


###############################################################################
# set the configuration options
###############################################################################


if (OPENGL_FOUND)
    set(CONF_DEFAULT_OPENGL ON)
else()
    set(CONF_DEFAULT_OPENGL OFF)
endif()

if (OpenCL_FOUND)
    set(CONF_DEFAULT_OPENGL ON)
else()
    set(CONF_DEFAULT_OPENGL OFF)
endif()

if (OpenMP_FOUND)
    set(CONF_DEFAULT_OPENGL ON)
else()
    set(CONF_DEFAULT_OPENGL OFF)
endif()


option(USE_OPENCL "build with opencl support" ${CONF_DEFAULT_OPENCL})
option(USE_OPENGL "build with opengl support" ${CONF_DEFAULT_OPENGL})
option(USE_OPENMP "build with openmp support" ${CONF_DEFAULT_OPENMP})
option(USE_SSE "build with SSE instrution set support" OFF)
option(BASIC_CONSOLE "build console only mode (no graphics)" OFF)



###############################################################################
# create the configuration file mtools_config.hpp
###############################################################################


if (USE_OPENCL)
    set(MTOOLS_USE_OPENCL 1)
    message(STATUS "CONFIGURATION : OPENCL = ON")
else()
    set(MTOOLS_USE_OPENCL 0)
    message(STATUS "CONFIGURATION : OPENCL = OFF")
endif()

if (USE_OPENGL)
    set(MTOOLS_USE_OPENGL 1)
    message(STATUS "CONFIGURATION : OPENGL = ON")    
else()
    set(MTOOLS_USE_OPENGL 0)
    message(STATUS "CONFIGURATION : OPENGL = OFF")
endif()

if (USE_OPENMP)
    set(MTOOLS_USE_OPENMP 1)
    message(STATUS "CONFIGURATION : OPENMP = ON")    
else()
    set(MTOOLS_USE_OPENMP 0)
    message(STATUS "CONFIGURATION : OPENMP = OFF")    
endif()

if (USE_SSE)
    set(MTOOLS_USE_SSE 1)
    message(STATUS "CONFIGURATION : SSE = ON")        
else()
    set(MTOOLS_USE_SSE 0)
    message(STATUS "CONFIGURATION : SSE = OFF")        
endif()

if (BASIC_CONSOLE)
    set(MTOOLS_BASIC_CONSOLE 1)
    message(STATUS "CONFIGURATION : CONSOLE MODE")        
else()
    set(MTOOLS_BASIC_CONSOLE 0)
    message(STATUS "CONFIGURATION : GRAPHIC MODE")        
endif()


configure_file(mtools_config.hpp.in "${CMAKE_SOURCE_DIR}/include/mtools_config.hpp" @ONLY)




###############################################################################
# list all sources and header files of the library. 
###############################################################################
file(GLOB_RECURSE _mtools_cpp_files ./src/*.cpp)
file(GLOB_RECURSE _mtools_hpp_files ./include/*.hpp ./include/*.h)


###############################################################################
# list all sources and header files of the test project
###############################################################################
file(GLOB_RECURSE _test_mtools_cpp_files ./test/*.cpp)
file(GLOB_RECURSE _test_mtools_hpp_files ./test/*.hpp ./test/*.h)


###############################################################################
# create the library
###############################################################################
add_library(mtools ${_mtools_cpp_files} ${_mtools_hpp_files})

target_link_libraries(mtools PUBLIC ${ZLIB_LIBRARIES})
target_link_libraries(mtools PUBLIC ${FLTK_LIBRARIES})
target_link_libraries(mtools PUBLIC ${PNG_LIBRARIES})
target_link_libraries(mtools PUBLIC ${JPEG_LIBRARIES})
target_link_libraries(mtools PUBLIC ${CAIRO_LIBRARIES})

target_include_directories(mtools PUBLIC ./include)

target_include_directories(mtools PUBLIC ${ZLIB_INCLUDE_DIRS})
target_include_directories(mtools PUBLIC ${FLTK_INCLUDE_DIR})
target_include_directories(mtools PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(mtools PUBLIC ${JPEG_INCLUDE_DIR})
target_include_directories(mtools PUBLIC ${CAIRO_INCLUDE_DIRS})
target_include_directories(mtools PUBLIC ${CIMG_INCLUDE_DIR})


###############################################################################
# create the test project
###############################################################################
add_executable(test_mtools ${_test_mtools_cpp_files} ${_test_mtools_hpp_files})

target_link_libraries(test_mtools mtools)


###############################################################################
# Request recent C++ features
###############################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


###############################################################################
# define test_mtools as the startup project in visual studio
###############################################################################
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT test_mtools)


###############################################################################
# vs filters for mtools: organized according to the directory structure. 
###############################################################################
foreach(_source IN ITEMS ${_mtools_hpp_files} ${_mtools_cpp_files})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${PROJECT_SOURCE_DIR}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


###############################################################################
# vs filters for test_mtools: organized according to the directory structure. 
###############################################################################
foreach(_source IN ITEMS ${_test_mtools_cpp_files} ${_test_mtools_hpp_files})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${PROJECT_SOURCE_DIR}/test" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


###############################################################################
# move ALL_BUILD and ZERO_CHECK inside filter "CMakePredefinedTargets"
###############################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CustomTargets")


###############################################################################
# cleanup for cmake-gui
###############################################################################
MARK_AS_ADVANCED(FLTK_DIR)
MARK_AS_ADVANCED(FLTK_BASE_LIBRARY)
MARK_AS_ADVANCED(FLTK_FLUID_EXECUTABLE)
MARK_AS_ADVANCED(FLTK_FORMS_LIBRARY)
MARK_AS_ADVANCED(FLTK_GL_LIBRARY)
MARK_AS_ADVANCED(FLTK_IMAGES_LIBRARY)
MARK_AS_ADVANCED(FLTK_INCLUDE_DIR)



message(STATUS "")
message(STATUS "*********************************")
message(STATUS "*  FINISHED CONFIGURING MTOOLS  *")
message(STATUS "*********************************")
message(STATUS "")

###############################################################################
#end of file
###############################################################################

