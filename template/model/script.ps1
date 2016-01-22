#
# Copyright 2015 Arvind Singh
#
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
#
# -----------------------------------------------------------------------------
#
# Script for creating a project skeleton setup for mtools
# the mtools directory must be set in the environment variable MTOOLS_LIB
#
# This script is called by 'createProject.bat' and creates the project inside
# the current directory
#

function checkEnvVar($evar, $scr)
{
if (-Not (Test-Path env:\$evar)) 
	{
	Write-Host "The environment variable " $evar " is not set."
	Write-Host "Did you run  " $scr " with admin. privileges ?"
	Write-Host "See the file 'INSTALL.txt' for details...."
	Exit
	}  
}

#check that MTOOLS_LIB environment variable is set. 
checkEnvVar "MTOOLS_LIB" "setEnvVar-mtools.bat"
checkEnvVar "MTOOLS_DEPENDENCIES_PATH"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "CAIRO_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "CIMG_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "FLTK_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "FREETYPE_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "LIBJPEG_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "LIBPNG_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "PIXMAN_LIB"  "setEnvVar-mtools-dependencies.bat"
checkEnvVar "ZLIB_LIB"  "setEnvVar-mtools-dependencies.bat"

	
# Get a new guid
$guid = [guid]::NewGuid()
$guid = $guid -split "`n"

# Get the project name
$name = Read-Host 'Project Name ? '

# Create the project
mkdir $name  > $null

# Copy the files
Copy-Item ($env:MTOOLS_LIB + "\template\model\emptyProject.sln") -destination (".\" + $name + "\" + $name + ".sln")
Copy-Item ($env:MTOOLS_LIB + "\template\model\emptyProject.vcxproj") -destination (".\" + $name + "\" + $name + ".vcxproj")
Copy-Item ($env:MTOOLS_LIB + "\template\model\emptyProject.vcxproj.filters") -destination (".\" + $name + "\" + $name + ".vcxproj.filters")
Copy-Item ($env:MTOOLS_LIB + "\template\model\main.cpp") -destination (".\" + $name + "\main.cpp")
Copy-Item ($env:MTOOLS_LIB + "\template\model\makefile") -destination (".\" + $name + "\makefile")
Copy-Item ($env:MTOOLS_LIB + "\template\model\stdafx.cpp") -destination (".\" + $name + "\stdafx.cpp")
Copy-Item ($env:MTOOLS_LIB + "\template\model\stdafx.h") -destination (".\" + $name + "\stdafx.h")

# change name and guid in .sln file
$slnfile = (".\" + $name + "\" + $name + ".sln")
(Get-Content $slnfile) -replace '\[PROJECT_NAME\]', ($name) | Set-Content ($slnfile)
(Get-Content $slnfile) -replace '\[PROJECT_GUID\]', ($guid) | Set-Content ($slnfile)

# change name and guid in .vcxproj file
$vcxprojfile = (".\" + $name + "\" + $name + ".vcxproj")
(Get-Content $vcxprojfile) -replace '\[PROJECT_NAME\]', ($name) | Set-Content ($vcxprojfile)
(Get-Content $vcxprojfile) -replace '\[PROJECT_GUID\]', ($guid) | Set-Content ($vcxprojfile)

# change name in main.cpp
$mainfile = (".\" + $name + "\main.cpp")
(Get-Content $mainfile) -replace '\[PROJECT_NAME\]', ($name) | Set-Content ($mainfile)

# change name in makefile
$makfile = (".\" + $name + "\makefile")
(Get-Content $makfile) -replace '\[PROJECT_NAME\]', ($name) | Set-Content ($makfile)

# done
Write-Host ""
Write-Host ("Project " + $name + " created.")
Write-Host ""
