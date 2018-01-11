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

