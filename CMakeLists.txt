###############################################################################
###############################################################################
#
#                       CMakeList for mtools. 
#
###############################################################################
###############################################################################
cmake_minimum_required(VERSION 3.10.1)

message(STATUS "")
message(STATUS "********************************************")
message(STATUS "*           MTOOLS CONFIGURATION           *")
message(STATUS "********************************************")
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
message(STATUS "")
message(STATUS "Configuring the library [version ${MTOOLS_VERSION}]")
message(STATUS "-------------------------------------")
message(STATUS "")


###############################################################################
# Project name
###############################################################################
project(mtools)


###############################################################################
# Locate the external libraries needed. 
###############################################################################

message(STATUS "")
message(STATUS "Checking external dependencies.")
message(STATUS "")

# load the custom find modules (for cimg and cairo). 
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake )


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

message(STATUS "")
message(STATUS "Checking optional libraries.")
message(STATUS "")

#opengl 
find_package(OpenGL)

#opencl
find_package(OpenCL)

#openmp
find_package(OpenMP)


###############################################################################
# set the configuration options
###############################################################################

message(STATUS "")
message(STATUS "compilation options:")
message(STATUS "")

option(CONSOLE_ONLY "console only mode" OFF)    
if (CONSOLE_ONLY)
    set(MTOOLS_BASIC_CONSOLE 1)
    message(STATUS "  CONSOLE_ONLY = 1  (console mode)")
else ()
    set(MTOOLS_BASIC_CONSOLE 0)
    message(STATUS "  CONSOLE_ONLY = 0  (graphic mode)")
endif ()

option(USE_SSE "use SSE specific code" OFF)    
if (USE_SSE)
    set(MTOOLS_SSE 1)
    message(STATUS "  USE_SSE = 1       (enable specific SSE code)")
else ()
    set(MTOOLS_SSE 0)
    message(STATUS "  USE_SSE = 0       (disable specific SSE code)")
endif ()


if (OpenGL_OpenGL_FOUND)
    option(USE_OPENGL "build with OpenGL support" ON)    
endif()
if (USE_OPENGL)
    set(MTOOLS_OPENGL 1)
    message(STATUS "  USE_OPENGL = 1    (enabled)")
else ()
    set(MTOOLS_OPENGL 0)
    message(STATUS "  USE_OPENGL = 0    (disabled)")
endif ()


if (OpenCL_FOUND)
    option(USE_OPENCL "build with OpenCL support" ON)
    set(MTOOLS_OPENCL_VERSION "${OpenCL_VERSION_MAJOR}.${OpenCL_VERSION_MINOR}" CACHE STRING "OpenCL version")
endif()
if (USE_OPENCL)
    set(MTOOLS_OPENCL 1)
    message(STATUS "  USE_OPENCL = 1    (enabled, version ${MTOOLS_OPENCL_VERSION})")
else ()
    set(MTOOLS_OPENCL 0)
    message(STATUS "  USE_OPENCL = 0    (disabled)")
endif ()


if (OpenMP_CXX_FOUND)
    option(USE_OPENMP "build with openmp support" ON)
    set(MTOOLS_OPENMP_VERSION "${OpenMP_CXX_VERSION_MAJOR}.${OpenMP_CXX_VERSION_MINOR}" CACHE STRING "OpenMP version")
endif()
if (USE_OPENMP)
    set(MTOOLS_OPENMP 1)
    message(STATUS "  USE_OPENMP = 1    (enabled, version ${MTOOLS_OPENMP_VERSION})")
else ()
    set(MTOOLS_OPENMP 0)
    message(STATUS "  USE_OPENMP = 0    (disabled)")
endif ()

message(STATUS "")


###############################################################################
# create the configuration file mtools_config.hpp
###############################################################################

configure_file(mtools_config.hpp.in "${CMAKE_SOURCE_DIR}/include/mtools/mtools_config.hpp" @ONLY)


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
# set installation path. 
###############################################################################

if ( WIN32 )
    set(DEFAULT_INSTALL_TYPE ON)
else()
    set(DEFAULT_INSTALL_TYPE OFF)
endif()

option(MTOOLS_LOCAL_INSTALL "local installation" ${DEFAULT_INSTALL_TYPE})    

if (MTOOLS_LOCAL_INSTALL)
    message(STATUS "")
    message(STATUS "mtools is installed locally.")
    message(STATUS "")
else()
    message(STATUS "")
    message(STATUS "Library will be installed in [${CMAKE_INSTALL_PREFIX}].")
    message(STATUS "")    
endif()


###############################################################################
# create the library
###############################################################################


add_library(mtools ${_mtools_cpp_files} ${_mtools_hpp_files})

target_include_directories(mtools PRIVATE ./include/mtools)         # for commodity, so that cpp source file of the lib do not need to subdir mtools. 

if (MTOOLS_LOCAL_INSTALL)
    # windows (no system wide installation)
    target_include_directories(mtools PUBLIC                            
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>       # - local include dir : when building mtools, test and the example
                               $<INSTALL_INTERFACE:include> )                               # - same but not prefixed : for the exported target 
else()
    target_include_directories(mtools PUBLIC                            
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>       # - local include dir : when building mtools, test and the example
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include> )       # - install include dir : for the exported target 
endif()


target_link_libraries(mtools PUBLIC ZLIB::ZLIB)

target_link_libraries(mtools PUBLIC ${FLTK_LIBRARIES})
target_include_directories(mtools PUBLIC ${FLTK_INCLUDE_DIR})

target_link_libraries(mtools PUBLIC PNG::PNG)

target_link_libraries(mtools PUBLIC ${JPEG_LIBRARIES})
target_include_directories(mtools PUBLIC ${JPEG_INCLUDE_DIR})

target_link_libraries(mtools PUBLIC ${CAIRO_LIBRARIES})
target_include_directories(mtools PUBLIC ${CAIRO_INCLUDE_DIRS})

target_include_directories(mtools PUBLIC ${CIMG_INCLUDE_DIR})


if (USE_OPENGL)
    target_link_libraries(mtools PUBLIC OpenGL::GL)
endif ()

if (USE_OPENCL)
    target_link_libraries(mtools PUBLIC OpenCL::OpenCL)
endif ()

if (USE_OPENMP)
    target_link_libraries(mtools PUBLIC OpenMP::OpenMP_CXX)
endif ()



###############################################################################
# installation of the library
###############################################################################


if (MTOOLS_LOCAL_INSTALL)

    export(TARGETS mtools FILE mtools-config.cmake)    # export the package
    export(EXPORT mtools FILE mtools-config.cmake)    # export the package
    export(PACKAGE mtools)                              # register the package

else()

    set(_include_install_dir "include/mtools")
    set(_lib_install_dir "lib/mtools")
    
    install(TARGETS mtools EXPORT mtools DESTINATION ${_lib_install_dir})
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/mtools"  DESTINATION "include")
    install(EXPORT mtools DESTINATION ${_lib_install_dir})

endif()


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



###############################################################################
# configure the examples
###############################################################################
add_subdirectory(./examples EXCLUDE_FROM_ALL)


message(STATUS "")
message(STATUS "********************************************")
message(STATUS "*      MTOOLS CONFIGURATION COMPLETED      *")
message(STATUS "********************************************")
message(STATUS "")

###############################################################################
#end of file
###############################################################################

