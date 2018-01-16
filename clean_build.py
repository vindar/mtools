#!/usr/bin/env python
#
# clean the /build sub-direcotry
#

import shutil
import os
#import time 

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

if (os.path.isdir("build")):
    shutil.rmtree('build')
#    time.sleep(1)

os.makedirs('build')

with open('build/build_directory','w') as out:
    out.write('This directory (will) contain the CMake generated project files.')


