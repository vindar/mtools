#!/usr/bin/env python

import shutil
import os

if (os.path.isdir("build")):
    shutil.rmtree('build')

os.makedirs('build')
with open('build/build_directory','a'):
    os.utime('build/build_directory', None)


