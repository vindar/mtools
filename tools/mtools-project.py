#!/usr/bin/env python
#
# Copyright 2015 Arvind Singh 
# This file is part of the mtools library.
#
# mtools is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mtools  If not, see <http://www.gnu.org/licenses/>.


############################################################################
#                                                                          #
#              script: create an empty mtools project.                     #
#                                                                          #
############################################################################



################### main.cpp ####################

mainFile = r"""/***********************************************
 * project: [PROJECT_NAME_PLH]
 * date: [PROJECT_DATE_PLH]
 ***********************************************/

#include "mtools/mtools.hpp" 


int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc,argv); // required on OSX, does nothing on Linux/Windows
    mtools::parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
    mtools::cout << "Hello World\n"; 
    mtools::cout.getKey();	
    return 0;
    }
	
/* end of file main.cpp */
"""

################### CMakeLists.txt ####################


cmakeFile = r"""################################################
# CMakeLists for project: [PROJECT_NAME_PLH]
# date: [PROJECT_DATE_PLH]
#
# generated by mtools-project.py
################################################

cmake_minimum_required(VERSION 3.10.1)

# look for vcpkg on windows
if( WIN32 )
	if (DEFINED ENV{VCPKG_DIR})
		string(REPLACE "\\" "/" _vcpkg_dir "$ENV{VCPKG_DIR}")
	else ()
		find_file( _vcpkg_exe "vcpkg.exe" PATHS ENV PATH)
		if (_vcpkg_exe)			
			get_filename_component(_vcpkg_dir ${_vcpkg_exe} DIRECTORY)
		endif()		
	endif()	
	if (_vcpkg_dir)	
		set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_dir}/scripts/buildsystems/vcpkg.cmake")					
		message(STATUS "Windows: vcpkg found at [${_vcpkg_dir}]")	
	else()
		message(STATUS "Windows: vcpkg not found.")	
	endif()
endif()
	 
# for linux/macos
if (NOT WIN32)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall") # compile with warnings enabled
	if (NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)				  # release build is default
	endif ()
endif()

project([PROJECT_NAME_PLH])
add_executable("${PROJECT_NAME}" main.cpp)

find_package(mtools REQUIRED)
target_link_libraries("${PROJECT_NAME}" mtools)


#### external dependencies ####

# add here other dependencies such as:
#    find_package(GSL)
#    target_link_libraries("${PROJECT_NAME}" GSL::gsl)

###############################


# set the project as the default startup project in visual studio.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")

# move CMake specific project inside filter "CMakePredefinedTargets".
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CustomTargets")


#end of file
"""

############################################################################
# the python script

import os
import shutil
import sys
import datetime
import subprocess

# input for both versions
if sys.version_info[0] >= 3:
    myinput = input
else:
    myinput = raw_input
	
	
# display an error msg
def error(msg):
	print "*** ERROR ***"
	print msg
	raw_input("Press Enter to continue...")
	sys.exit(0)
	
	
# make replacement in string then save the file
def repl(str,filename):
	str = str.replace("[PROJECT_NAME_PLH]",project_name)
	str = str.replace("[PROJECT_DATE_PLH]",project_date)
	filepath = project_dir + "/" + filename
	try:	
		fout = open(filepath,"w")
		fout.write(str);
		fout.close()
	except:
		error("cannot write file [" + filepath + "]")	

		
# get the date
project_date = str(datetime.date.today())	
		
# get the project name		
if (len(sys.argv) > 1):
	project_name = sys.argv[1]
else:
	project_name = myinput("Name of the project to create ? ") 

# create the project directories
project_dir  = os.getcwd() + "/" + project_name
project_build = project_dir + "/build"

if os.path.exists(project_dir):
	error("directory [" + project_dir + "] already exist")
try:	
	os.makedirs(project_dir)
	os.makedirs(project_build)
except:
	error("cannot create project directory [" + project_dir + "]")	
	
# copy the files		
repl(mainFile,"main.cpp")
repl(cmakeFile,"CMakeLists.txt")

# run the cmake command to build project the files
os.chdir(project_build)
if sys.platform.startswith('win32'):
	import subprocess
	subprocess.call(['cmake', '-A', 'x64', '..']) # x64 build on windows
else:
	subprocess.call(['cmake', '..'])
	
	
#done !
print "\n*** Project " + project_name + " created ! ***"
print "***  the project files are located in the '\\build' directory\n"
raw_input("Press Enter to continue...")
# end of script mtools-project.py
############################################################################








