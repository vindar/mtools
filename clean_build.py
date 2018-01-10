import shutil
import os

if (os.path.isdir("build")):
	shutil.rmtree('build')
	
os.makedirs('build')
