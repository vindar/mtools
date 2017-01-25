@echo off

>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
if '%errorlevel%' NEQ '0' (
	echo You must run the script with admin rights...
	pause > nul
    exit /B    
) 

CD /D "%~dp0"
echo ===================================
echo mtools setup
echo ===================================
echo.
echo This script must be run AFTER running
echo 'setup-mtools-dependencies.bat'. 
echo.


echo - Setting the global environment variables MTOOLS_LIB....
echo on

setx /m MTOOLS_LIB "%CD%\\" 

echo off
echo.
echo - Copying 'mtools_config_OpenCL.hpp'
echo.

echo on
copy /y  "%OPENCL_LIB%\mtools_config_OpenCL_CHOSEN.hpp"  "%MTOOLS_LIB%\mtools\headers\mtools_config_OpenCL.hpp"
echo off

echo done !
pause > nul

