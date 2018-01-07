###############################################################################
###############################################################################
#
#                       CMakeList for the examples. 
#
###############################################################################
###############################################################################
cmake_minimum_required(VERSION 3.10.1)

message(STATUS "")
message(STATUS "")
message(STATUS "Configuring the examples")
message(STATUS "------------------------")
message(STATUS "")
   
###############################################################################
# Project name
###############################################################################
project(examples)


###############################################################################
# get the list of all sub-directories in dirlist
###############################################################################
FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

SET(dirlist "")
FOREACH(child ${children})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
    LIST(APPEND dirlist ${child})
    ENDIF()
ENDFOREACH()


###############################################################################
# iterate over each sub-directory
###############################################################################
FOREACH(projectname ${dirlist})
    set(projectdir "${CMAKE_CURRENT_SOURCE_DIR}/${projectname}")
    if(EXISTS "${projectdir}/custom.cmake")
        message(STATUS "[custom build]  - ${projectname}")        
        include(${projectdir}/custom.cmake)
    else()
        message(STATUS "[default build] - ${projectname}")        
        file(GLOB_RECURSE src_files ${projectdir}/*.cpp ${projectdir}/*.hpp ${projectdir}/*.h)
        add_executable(${projectname} ${src_files})
    endif()
    target_link_libraries(${projectname} mtools)            
    
ENDFOREACH()


###############################################################################
#end of file
###############################################################################
