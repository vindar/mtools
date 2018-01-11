#!/usr/bin/env python
#
# clean the /build sub-direcotry
#

import shutil
import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

if (os.path.isdir("build")):
    shutil.rmtree('build')

os.makedirs('build')
with open('build/build_directory','w'):
    os.utime('build/build_directory', None)


