###############################################################################
###############################################################################
#
#                       CMakeList for mtools library 
#
###############################################################################
###############################################################################
cmake_minimum_required(VERSION 3.10)

message(STATUS "")
message(STATUS "***************************************************************")
message(STATUS "*                                                             *")
message(STATUS "*                   MTOOLS CONFIGURATION                      *")
message(STATUS "*                                                             *")
message(STATUS "***************************************************************")
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
        
    # only Debug and Release configurations
    SET(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

endif()
	 
	      
###############################################################################
# Project name and version
###############################################################################
file(READ VERSION MTOOLS_VERSION 10)
message(STATUS "")
message(STATUS "library version: ${MTOOLS_VERSION}")
message(STATUS "-------------------------------------")
message(STATUS "")

project(mtools)


###############################################################################
# Locate the external libraries needed. 
###############################################################################


# load the custom find modules (for cimg and cairo). 
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake )


# use cotire for precompiled headers if present
include(cotire OPTIONAL)
message(STATUS "")
option(USE_COTIRE "Use precompiled headers generated by cotire" OFF) 
if (COMMAND cotire)
	message(STATUS "COTIRE FOUND : set USE_COTIRE=ON to use PCH")
else()
	message(STATUS "COTIRE NOT FOUND : not using precompild headers")
endif()
message(STATUS "")



#### REQUIRED ####
message(STATUS "")
message(STATUS "Checking external dependencies.")
message(STATUS "")


# zlib 
find_package(ZLIB REQUIRED)

# fltk, use custom find method
find_package(FLTK_alt REQUIRED)

# libpng
find_package(PNG REQUIRED)

# libjpeg, use custom find method 
find_package(JPEG_alt REQUIRED)

# cimg, use custom find method 
find_package(CIMG_alt REQUIRED)

#tinyXML2
find_package(TinyXML2 REQUIRED)

if (NOT ZLIB_FOUND OR NOT FLTK_FOUND OR NOT PNG_FOUND OR NOT JPEG_FOUND OR NOT CIMG_FOUND OR NOT TinyXML2_FOUND)
	message(STATUS "")
	message(STATUS "***************************************************************")	
	message(STATUS "*                           ERROR                             *")
	message(STATUS "***************************************************************")
	message(STATUS "Some required libraries could not be found.")
	message(FATAL_ERROR "Aborting configuration script..." )
else()
	message(STATUS "")
	message(STATUS "OK. All required library present.")
	message(STATUS "-------------------------------------")
	message(STATUS "")
endif()

#### OPTIONAL ####
message(STATUS "")
message(STATUS "Checking optional libraries.")
message(STATUS "")

# cairo, use custom find method 
find_package(CAIRO_alt)

#opengl 
find_package(OpenGL)

#opencl
find_package(OpenCL)

#openmp
find_package(OpenMP)

#tgx
find_package(TGX)


###############################################################################
# set the configuration options and create mtools-config.hpp
###############################################################################


	
option(CONSOLE_ONLY "console only mode" OFF)    
if (CONSOLE_ONLY)
    set(MTOOLS_BASIC_CONSOLE 1)
else ()
    set(MTOOLS_BASIC_CONSOLE 0)
endif ()


option(USE_SSE "use SSE specific code" OFF)    
if (USE_SSE)
    set(MTOOLS_SSE 1)
else ()
    set(MTOOLS_SSE 0)
endif ()


if (CAIRO_FOUND)
	option(USE_CAIRO "build with Cairo support" OFF)    
endif()
if (USE_CAIRO)
    set(MTOOLS_CAIRO 1)
else ()
    set(MTOOLS_CAIRO 0)
endif ()


# for the time being, we disable usage of libjpeg-turbo specifics (even if loaded)
set(MTOOLS_JPEG_TURBO 0)


if (OpenGL_OpenGL_FOUND)
    option(USE_OPENGL "build with OpenGL support" ON)    
endif()
if (USE_OPENGL)
    set(MTOOLS_OPENGL 1)
else ()
    set(MTOOLS_OPENGL 0)
endif ()


if (OpenCL_FOUND)
    option(USE_OPENCL "build with OpenCL support" OFF)
    set(MTOOLS_OPENCL_VERSION "${OpenCL_VERSION_MAJOR}.${OpenCL_VERSION_MINOR}" CACHE STRING "OpenCL version")
endif()
if (USE_OPENCL)
    set(MTOOLS_OPENCL 1)
else ()
    set(MTOOLS_OPENCL 0)
endif ()


if (OpenMP_CXX_FOUND)
    option(USE_OPENMP "build with openmp support" OFF)
    set(MTOOLS_OPENMP_VERSION "${OpenMP_CXX_VERSION_MAJOR}.${OpenMP_CXX_VERSION_MINOR}" CACHE STRING "OpenMP version")
endif()
if (USE_OPENMP)
    set(MTOOLS_OPENMP 1)
else ()
    set(MTOOLS_OPENMP 0)
endif ()


if (TGX_FOUND)
    option(USE_TGX "build with TGX support" ON)
endif()
if (USE_TGX)
    set(MTOOLS_TGX 1)
else ()
    set(MTOOLS_TGX 0)
endif ()



message(STATUS "")

configure_file(mtools_config.hpp.in "${CMAKE_SOURCE_DIR}/include/mtools/mtools_config.hpp" @ONLY)


###############################################################################
# list all sources and header files of the library. 
###############################################################################
file(GLOB_RECURSE _mtools_cpp_files ./src/*.cpp ./src/*.c)
file(GLOB_RECURSE _mtools_hpp_files ./include/*.hpp ./include/*.h ./include/*.inl)

if (USE_TGX)
    file(GLOB_RECURSE _tgx_hpp_files "${TGX_INCLUDE_DIR}/*.h" "${TGX_INCLUDE_DIR}/*.hpp" "${TGX_INCLUDE_DIR}/*.inl")
    file(GLOB_RECURSE _tgx_cpp_files "${TGX_INCLUDE_DIR}/*.cpp" "${TGX_INCLUDE_DIR}/*.c")
endif()


###############################################################################
# create the library
###############################################################################


if (USE_TGX)
    # with tgx
    add_library(mtools STATIC ${_mtools_cpp_files} ${_mtools_hpp_files} ${_tgx_cpp_files} ${_tgx_hpp_files})
	target_include_directories(mtools PUBLIC ${TGX_INCLUDE_DIR})

else()
    # without tgx
    add_library(mtools STATIC ${_mtools_cpp_files} ${_mtools_hpp_files})

endif()

target_include_directories(mtools PRIVATE ./include/mtools)         # for commodity, so that cpp source file of the lib do not need to subdir mtools. 



# set property for cotire (optional precompiled header tool) does nothing if not present
set_target_properties(mtools PROPERTIES
	COTIRE_UNITY_SOURCE_PRE_UNDEFS "max;min;MTOOLS_ASSERT;MTOOLS_INSURE;MTOOLS_ERROR;MTOOLS_DEBUG;MTOOLS_THROW"
	COTIRE_UNITY_SOURCE_POST_UNDEFS "max;min;MTOOLS_ASSERT;MTOOLS_INSURE;MTOOLS_ERROR;MTOOLS_DEBUG;MTOOLS_THROW")


option(LOCAL_INSTALL "local installation" ON)    

if (LOCAL_INSTALL)
    # local installation
    target_include_directories(mtools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)		# - local include directory
else()

    if (USE_TGX)
        message(FATAL_ERROR "Cannot perform system wide installation with TGX extension: use local install instead..." )
    endif()
    
	# system wide installation
    target_include_directories(mtools PUBLIC                            
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>   # - local include dir : when building mtools, test and the example
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include> )   # - install include dir : for the exported target 
endif()


# link with libatomic on gcc
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_link_libraries(mtools PUBLIC atomic)
endif()


# link with zlib
# target_link_libraries(mtools PUBLIC ZLIB::ZLIB) <-- do not use because export are not forwarded. 
target_link_libraries(mtools PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(mtools PUBLIC ${ZLIB_INCLUDE_DIRS})

#link with fltk
if (WIN32)
    target_link_libraries(mtools PUBLIC ${FLTK_LIBRARIES})
else()
    if (USE_OPENGL)    
        target_link_libraries(mtools PUBLIC ${FLTK_LIBRARIES_GL})
    else()
        target_link_libraries(mtools PUBLIC ${FLTK_LIBRARIES})
    endif()
endif()

target_include_directories(mtools PUBLIC ${FLTK_INCLUDE_DIR})
target_compile_options(mtools PUBLIC ${FLTK_OPTIONS})

#link with libpng
# target_link_libraries(mtools PUBLIC PNG::PNG)  <-- do not use because export are not forwarded. 
target_link_libraries(mtools PUBLIC ${PNG_LIBRARIES})
target_include_directories(mtools PUBLIC ${PNG_INCLUDE_DIRS})
target_compile_definitions(mtools PUBLIC ${PNG_DEFINITIONS})

#link with libjpeg
target_link_libraries(mtools PUBLIC ${JPEG_LIBRARIES})
target_include_directories(mtools PUBLIC ${JPEG_INCLUDE_DIR})

#link with cimg
target_include_directories(mtools PUBLIC ${CIMG_INCLUDE_DIR})

#link with TinyXML2
target_link_libraries(mtools PUBLIC ${TinyXML2_LIBRARIES})
target_include_directories(mtools PUBLIC ${TinyXML2_INCLUDE_DIRS})

#link with cairo
if (USE_CAIRO)
	target_link_libraries(mtools PUBLIC ${CAIRO_LIBRARIES})
	target_include_directories(mtools PUBLIC ${CAIRO_INCLUDE_DIRS})
endif ()

#link with opengl
if (USE_OPENGL)
    # target_link_libraries(mtools PUBLIC OpenGL::GL) <-- do not use because export are not forwarded.
	target_link_libraries(mtools PUBLIC ${OPENGL_LIBRARIES})
	target_include_directories(mtools PUBLIC ${OPENGL_INCLUDE_DIR})	
endif ()

#link with opencl
if (USE_OPENCL)
    # target_link_libraries(mtools PUBLIC OpenCL::OpenCL) <-- do not use because export are not forwarded.
	target_link_libraries(mtools PUBLIC ${OpenCL_LIBRARIES})
	target_include_directories(mtools PUBLIC ${OpenCL_INCLUDE_DIRS})
endif ()

#link with openmp
if (USE_OPENMP)
    # target_link_libraries(mtools PUBLIC OpenMP::OpenMP_CXX) <-- do not use because export are not forwarded.
	target_link_libraries(mtools PUBLIC ${OpenMP_CXX_LIBRARIES})
	target_compile_options(mtools PUBLIC ${OpenMP_CXX_FLAGS})
endif ()



# if cotire present and USE_COTIRE is on, use it
if (COMMAND cotire)
	if (USE_COTIRE)
		cotire(mtools)
	endif()
endif()


###############################################################################
# C++ compile features
###############################################################################

# release is the default build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

if(WIN32)
	target_compile_options(mtools PUBLIC "/MP")
	#target_compile_options(mtools PUBLIC "/std:c++17")
	target_compile_features(mtools PUBLIC cxx_std_17)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DMTOOLS_DEBUG_FLAG")	
	# hack for RelWithDebINfo configuration otherwise compile never ends on MSVC
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi /Gm- /Ox /Ob0 /DMTOOLS_DEBUG_FLAG")	
else()
	#target_compile_options(mtools PUBLIC "-std=c++17")
        target_compile_features(mtools PUBLIC cxx_std_17)
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGL_SILENCE_DEPRECATION -DMTOOLS_DEBUG_FLAG -Wall")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DGL_SILENCE_DEPRECATION  -DMTOOLS_DEBUG_FLAG -Wall")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -DGL_SILENCE_DEPRECATION  -Wall")
endif()


###############################################################################
# installation of the library
###############################################################################


if (LOCAL_INSTALL)

    export(TARGETS mtools FILE mtools-export.cmake)    	# export the package
    file(WRITE  "${CMAKE_CURRENT_BINARY_DIR}/mtools-config.cmake"
         "set(MTOOLS_CXX_COMPILER \"${CMAKE_CXX_COMPILER}\")\nset(MTOOLS_C_COMPILER \"${CMAKE_C_COMPILER}\")\ninclude(\$\{CMAKE_CURRENT_LIST_DIR\}/mtools-export.cmake)" )
    export(PACKAGE mtools)                              # register the package

else()

    set(_include_install_dir "include/mtools")
    set(_lib_install_dir "lib/mtools")
    set(_tools_install_dir "bin")
    
    install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/tools/mtools-project.py" DESTINATION ${_tools_install_dir})
    install(TARGETS mtools EXPORT mtools DESTINATION ${_lib_install_dir})
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/mtools"  DESTINATION "include")
    install(EXPORT mtools DESTINATION ${_lib_install_dir} FILE mtools-export.cmake)
    file(WRITE  "${CMAKE_CURRENT_BINARY_DIR}/mtools-config.cmake" 
         "set(MTOOLS_CXX_COMPILER \"${CMAKE_CXX_COMPILER}\")\nset(MTOOLS_C_COMPILER \"${CMAKE_C_COMPILER}\")\ninclude(\$\{CMAKE_CURRENT_LIST_DIR\}/mtools-export.cmake)" ) 
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mtools-config.cmake" DESTINATION ${_lib_install_dir})

endif()



###############################################################################
# test project.
###############################################################################
file(GLOB _test_mtools_cpp_files ./test/*.cpp ./test/*.c)
file(GLOB _test_mtools_hpp_files ./test/*.hpp ./test/*.h ./test/*.inl)


add_executable(test_mtools ${_test_mtools_cpp_files} ${_test_mtools_hpp_files})
target_link_libraries(test_mtools mtools)


# add mtools to the precompiled headers generated by cotire for the test project (does nothing if cotire not present)
set_target_properties(test_mtools PROPERTIES COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include/mtools/mtools.hpp")
	
	
# set test_mtools as the startup project in visual studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT test_mtools)


# if cotire present and USE_COTIRE is ON, use it
if (COMMAND cotire)
	if (USE_COTIRE)
		cotire(test_mtools)
	endif()
endif()


###############################################################################
# vs filters for mtools: organized according to the directory structure. 
###############################################################################
foreach(_source IN ITEMS ${_mtools_hpp_files} ${_mtools_cpp_files})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${PROJECT_SOURCE_DIR}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if (USE_TGX)
    foreach(_source IN ITEMS ${_tgx_cpp_files})
        source_group("tgx/src" FILES "${_source}")
    endforeach()
    
    foreach(_source IN ITEMS ${_tgx_hpp_files})
        source_group("tgx/include" FILES "${_source}")
    endforeach()
    
endif()

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


###############################################################################
# display infos
###############################################################################

message(STATUS "")
message(STATUS "compilation options:")
message(STATUS "-------------------------------------")
message(STATUS "")

if (CONSOLE_ONLY)
    message(STATUS "  CONSOLE_ONLY = 1  (console mode)")
else ()
    message(STATUS "  CONSOLE_ONLY = 0  (graphic mode)")
endif ()

if (USE_SSE)
    message(STATUS "  USE_SSE = 1       (enable specific SSE code)")
else ()
    message(STATUS "  USE_SSE = 0       (disable specific SSE code)")
endif ()

if (USE_CAIRO)
    message(STATUS "  USE_CAIRO = 1     (enabled)")
else ()
    message(STATUS "  USE_CAIRO = 0     (disabled)")
endif ()

if (USE_OPENGL)
    message(STATUS "  USE_OPENGL = 1    (enabled)")
else ()
    message(STATUS "  USE_OPENGL = 0    (disabled)")
endif ()

if (USE_OPENCL)
    message(STATUS "  USE_OPENCL = 1    (enabled, version ${MTOOLS_OPENCL_VERSION})")
else ()
    message(STATUS "  USE_OPENCL = 0    (disabled)")
endif ()

if (USE_OPENMP)
    message(STATUS "  USE_OPENMP = 1    (enabled, version ${MTOOLS_OPENMP_VERSION})")
else ()
    message(STATUS "  USE_OPENMP = 0    (disabled)")
endif ()

if (USE_TGX)
    message(STATUS "  USE_TGX = 1       (enabled)")
else ()
    message(STATUS "  USE_TGX = 0       (disabled)")
endif ()

message(STATUS "")


if (COMMAND cotire)
	if (USE_COTIRE)
		message(STATUS "  USE PRECOMPILED HEADERS (USE_COTIRE = ON)")
	else()
		message(STATUS "  DO NOT USE PRECOMPILED HEADERS (USE_COTIRE = OFF)")	
	endif()
else()
	message(STATUS "  DO NOT USE PRECOMPILED HEADERS (cotire not found)")	
endif()


message(STATUS "")
message(STATUS "")
if (LOCAL_INSTALL)
    message(STATUS "  LOCAL_INSTALL = 1  -> mtools will be used locally (no installation)")
else()
    message(STATUS "  LOCAL_INSTALL = 0  -> mtools will be installed in")
	message(STATUS "  CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
endif()

message(STATUS "")    
message(STATUS "")

if (NOT WIN32)
	message(STATUS "Chosen configuration ${CMAKE_BUILD_TYPE} : [${CMAKE_CXX_FLAGS}]")
        message(STATUS "    - Debug : [${CMAKE_CXX_FLAGS_DEBUG}]")
        message(STATUS "    - RelWithDebInfo : [${CMAKE_CXX_FLAGS_RELWITHDEBINFO}]")
        message(STATUS "    - Release : [${CMAKE_CXX_FLAGS_RELEASE}]")
else()
	message(STATUS "MSVC Configurations: ")
	message(STATUS "    - Debug : [${CMAKE_CXX_FLAGS_DEBUG}]")
	message(STATUS "    - RelWithDebInfo : [${CMAKE_CXX_FLAGS_RELWITHDEBINFO}]")
	message(STATUS "    - Release : [${CMAKE_CXX_FLAGS_RELEASE}]")
endif()

message(STATUS "")
message(STATUS "")
message(STATUS "****************************************************************")
message(STATUS "*                     CONFIGURATION DONE.                      *")
message(STATUS "*                                                              *")
if (NOT WIN32)
	message(STATUS "* - Run 'make' inside the '/build' sub-directory to build the  *")
	message(STATUS "*   library                                                    *")
	message(STATUS "*                                                              *")
	if (LOCAL_INSTALL)
	message(STATUS "* - No installation (use the local build tree)                 *")
	message(STATUS "*   Add the '/tools' subdir to PATH to use 'mtools-project.py' *")	
	message(STATUS "*                                                              *")
	else()
	message(STATUS "* - Then install it with '[sudo] make install'                 *")
	endif()
	message(STATUS "*                                                              *")
	message(STATUS "* - Run 'make all' inside './examples' to build the examples   *")
else ()
	message(STATUS "* - Open the solution '/built/mtools.sln' to build the library *")
	message(STATUS "*                                                              *")
	if (LOCAL_INSTALL)
	message(STATUS "* - No installation (use the local build tree)                 *")
	message(STATUS "*   Add the '/tools' subdir to PATH to use 'mtools-config.py'  *")		
	message(STATUS "*                                                              *")
	else()
	message(STATUS "* - Then install with the INSTALL project inside the solution  *")
	endif()
	message(STATUS "*                                                              *")
	message(STATUS "* - Open the solution file ./examples/examples.sln to build    *")
	message(STATUS "*   the examples                                               *")
endif()
	message(STATUS "*                                                              *")
	message(STATUS "****************************************************************")
	message(STATUS "")

###############################################################################
#end of file
###############################################################################


