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
if (! os.path.exists(project_dir)):
	os.makedirs('build')
	
os.chdir(os.getcwd() + "/build")
subprocess.call(carg)

