#!/usr/bin/env python
#
# Invoque cmake to build the library
# usage: ./cmake-mtools [CMAKE_OPTIONS...]
#

import sys
import os
import subprocess

carg = sys.argv
del carg[0]


if (len(carg) > 0) and (carg[0][0:2] != '-D'):
	str = """
******************************************
* CMake script for library vindar/mtools *
******************************************
Usage: ./cmake_tools.py [OPTIONS...]

List of possible options:
    -DUSE_COTIRE=X            use precompiled headers (default X=0)
    -DCONSOLE_ONLY=X          disable graphics (default X=0)
    -DUSE_CAIRO=X             use cairo if present (default X=0)
    -DUSE_SSE=X		      use SSE specific instructions (default X=0)
    -DUSE_OPENMP=X            use OpenMP if present (default X=0)
    -DUSE_OPENCL=X            use OpenCL if present (default X=0)
    -DUSE_OPENGL=X            use OpenGL if present (default X=1)
	
    -DLOCAL_INSTALL=X         use library directly from install path. 
    -DCMAKE_INSTALL_PREFIX=install/path/    set path if LOCAL_INSTALL=1

    -DCMAKE_CXX_COMPILER=/path/to/compiler  set the c++ compiler to use 

"""
	print(str); 
else:
	carg.insert(0,'cmake');
	carg.append('..');
	# on windows, we build x64 binaries
	if sys.platform.startswith('win32'):
		carg.insert(1,'-A');
		carg.insert(2,'x64');
	# invoque cmake with the correct arguments
	if (not os.path.exists('build')):
		os.makedirs('build')	
	abspath = os.path.abspath(__file__)
	dname = os.path.dirname(abspath)
	os.chdir(dname + "/build")
	subprocess.call(carg)

