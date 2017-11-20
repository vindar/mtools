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



#################################################
#                                               #
# main.cpp                                      #
#                                               #
#################################################
mainFile = r"""
/***********************************************
 * [PROJECT_NAME] project
 ***********************************************/

#include "stdafx.h" // precompiled headers

// mtools main header, define MTOOLS_BASIC_CONSOLE to disable mtools's graphic console
#include "mtools.hpp" 


int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc,argv);   // required on OSX, does nothing on Linux/Windows
    mtools::parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
    mtools::cout << "Hello World\n"; 
    mtools::cout.getKey();	
    return 0;
    }
	
/* end of file main.cpp */
"""


#################################################
#                                               #
# makefile                                      #
#                                               #
#################################################
makeFile = r"""
OUTFILE = [PROJECT_NAME]

OUTDIR = bin

ALL_FILES := $(wildcard *.cpp)
CPP_FILES := $(filter-out stdafx.cpp,$(ALL_FILES))

WORKDIR = `pwd`
CXX = g++

CXXFLAGS = -std=c++1z -Wall `fltk-config --cxxflags` `pkg-config cairo --cflags` `pkg-config pixman-1 --cflags`
LDFLAGS =   -lmtools `fltk-config --ldstaticflags` `pkg-config cairo --libs` -lfreetype -ljpeg -lpng `pkg-config pixman-1 --libs` -lz

CHECKCOMPILERGCC = $(shell $(CXX) --version | grep 'GCC\|gcc\|g++')
ifneq ("$(CHECKCOMPILERGCC)","")
# gcc specific options
    LDFLAGS += -latomic
    CXXFLAGS += -fopenmp
endif

CHECKCOMPILERLLVM = $(shell $(CXX) --version | grep 'LLVM\|clang')
ifneq ("$(CHECKCOMPILERLLVM)","")
# clang specific options
endif

.PHONY: release nographics debug debug_nographics buildprog clean

release: CXXFLAGS += -O2 -DNDEBUG
release: buildprog

nographics: CXXFLAGS += -O2 -DNDEBUG -DMTOOLS_BASIC_CONSOLE
nographics: buildprog

debug: CXXFLAGS += -g -DDEBUG
debug: buildprog

debug_nographics: CXXFLAGS += -g -DDEBUG -DMTOOLS_BASIC_CONSOLE
debug_nographics: buildprog

buildprog: $(OUTFILE)

$(OUTFILE): $(CPP_FILES)
	@test -d $(OUTDIR) || mkdir -p $(OUTDIR)
	$(CXX) $(CPP_FILES) $(CXXFLAGS) $(LDFLAGS) -o $(OUTDIR)/$(OUTFILE)

clean: 
	@rm -rf $(OUTDIR)
"""


#################################################
#                                               #
# [project].sln                                 #
#                                               #
#################################################
slnFile = r"""Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 14
VisualStudioVersion = 14.0.22823.1
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "[PROJECT_NAME]", "[PROJECT_NAME].vcxproj", "{[PROJECT_GUID]}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Debug|x64 = Debug|x64
		Release|Win32 = Release|Win32
		Release|x64 = Release|x64
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{[PROJECT_GUID]}.Debug|Win32.ActiveCfg = Debug|Win32
		{[PROJECT_GUID]}.Debug|Win32.Build.0 = Debug|Win32
		{[PROJECT_GUID]}.Debug|x64.ActiveCfg = Debug|x64
		{[PROJECT_GUID]}.Debug|x64.Build.0 = Debug|x64
		{[PROJECT_GUID]}.Release|Win32.ActiveCfg = Release|Win32
		{[PROJECT_GUID]}.Release|Win32.Build.0 = Release|Win32
		{[PROJECT_GUID]}.Release|x64.ActiveCfg = Release|x64
		{[PROJECT_GUID]}.Release|x64.Build.0 = Release|x64
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
"""


#################################################
#                                               #
# [project].vcxproj                             #
#                                               #
#################################################
vcxprojFile = r"""<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="14.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32"> <Configuration>Debug</Configuration> <Platform>Win32</Platform> </ProjectConfiguration> 
	<ProjectConfiguration Include="Debug|x64"> <Configuration>Debug</Configuration> <Platform>x64</Platform> </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32"> <Configuration>Release</Configuration> <Platform>Win32</Platform> </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"> <Configuration>Release</Configuration> <Platform>x64</Platform> </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{[PROJECT_GUID]}</ProjectGuid> <Keyword>Win32Proj</Keyword> <RootNamespace>[PROJECT_NAME]</RootNamespace> <TargetPlatformVersion>8.1</TargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration"> <ConfigurationType>Application</ConfigurationType> <UseDebugLibraries>true</UseDebugLibraries> <PlatformToolset>v140</PlatformToolset> <CharacterSet>NotSet</CharacterSet> </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration"> <ConfigurationType>Application</ConfigurationType> <UseDebugLibraries>true</UseDebugLibraries> <PlatformToolset>v140</PlatformToolset> <CharacterSet>NotSet</CharacterSet> </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration"> <ConfigurationType>Application</ConfigurationType> <UseDebugLibraries>false</UseDebugLibraries> <PlatformToolset>v140</PlatformToolset> <WholeProgramOptimization>true</WholeProgramOptimization> <CharacterSet>NotSet</CharacterSet> </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration"> <ConfigurationType>Application</ConfigurationType> <UseDebugLibraries>false</UseDebugLibraries> <PlatformToolset>v140</PlatformToolset> <WholeProgramOptimization>true</WholeProgramOptimization> <CharacterSet>NotSet</CharacterSet> </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings"> </ImportGroup>
  <ImportGroup Label="Shared"> </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'"> <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" /> </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="PropertySheets"> <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" /> </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'"> <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" /> </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="PropertySheets"> <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" /> </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'"> <LinkIncremental>true</LinkIncremental> <OutDir>$(ProjectDir)build\$(Platform)\</OutDir> <IntDir>$(ProjectDir)build\$(Platform)\temp\$(Configuration)\</IntDir> <TargetName>$(ProjectName)-debug</TargetName> </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'"> <LinkIncremental>true</LinkIncremental> <OutDir>$(ProjectDir)build\$(Platform)\</OutDir> <IntDir>$(ProjectDir)build\$(Platform)\temp\$(Configuration)\</IntDir> <TargetName>$(ProjectName)-debug</TargetName> </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'"> <LinkIncremental>false</LinkIncremental> <OutDir>$(ProjectDir)build\$(Platform)\</OutDir> <IntDir>$(ProjectDir)build\$(Platform)\temp\$(Configuration)\</IntDir> </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'"> <LinkIncremental>false</LinkIncremental> <OutDir>$(ProjectDir)build\$(Platform)\</OutDir> <IntDir>$(ProjectDir)build\$(Platform)\temp\$(Configuration)\</IntDir> </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <PrecompiledHeader>Use</PrecompiledHeader> <WarningLevel>Level3</WarningLevel> <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>;WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>$(CAIRO_LIB)/source/;$(CIMG_LIB)/source/;$(FLTK_LIB)/source/;$(FREETYPE_LIB)/source/include/freetype/;$(LIBJPEG_LIB)/source/;$(LIBPNG_LIB)/source/;$(MTOOLS_LIB)/mtools/headers/;$(PIXMAN_LIB)/source/pixman/;$(ZLIB_LIB)/source/</AdditionalIncludeDirectories>
	  <AdditionalOptions>/Zm400 %(AdditionalOptions)</AdditionalOptions>
	  <OpenMPSupport>true</OpenMPSupport>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem> <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalLibraryDirectories>$(CAIRO_LIB)/win32/;$(FLTK_LIB)/win32/;$(FREETYPE_LIB)/win32/;$(LIBJPEG_LIB)/win32/;$(LIBPNG_LIB)/win32/;$(MTOOLS_LIB)/mtools/lib/;$(PIXMAN_LIB)/win32/;$(ZLIB_LIB)/win32/</AdditionalLibraryDirectories>
      <AdditionalDependencies>cairod.lib;fltkd.lib;fltkgld.lib;fltkimagesd.lib;fltkjpegd.lib;freetyped.lib;libpngd.lib;mtools32d.lib;pixmand.lib;zlibd.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>
	  <AdditionalOptions> %(AdditionalOptions)</AdditionalOptions>	  	  
    </Link>
	<Manifest>
      <EnableDpiAwareness>true</EnableDpiAwareness>
    </Manifest>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <PrecompiledHeader>Use</PrecompiledHeader> <WarningLevel>Level3</WarningLevel> <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>;WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>$(CAIRO_LIB)/source/;$(CIMG_LIB)/source/;$(FLTK_LIB)/source/;$(FREETYPE_LIB)/source/include/freetype/;$(LIBJPEG_LIB)/source/;$(LIBPNG_LIB)/source/;$(MTOOLS_LIB)/mtools/headers/;$(PIXMAN_LIB)/source/pixman/;$(ZLIB_LIB)/source/</AdditionalIncludeDirectories>
	  <AdditionalOptions>/Zm400 %(AdditionalOptions)</AdditionalOptions>
	  <OpenMPSupport>true</OpenMPSupport>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem> <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalLibraryDirectories>$(CAIRO_LIB)/x64/;$(FLTK_LIB)/x64/;$(FREETYPE_LIB)/x64/;$(LIBJPEG_LIB)/x64/;$(LIBPNG_LIB)/x64/;$(MTOOLS_LIB)/mtools/lib/;$(PIXMAN_LIB)/x64/;$(ZLIB_LIB)/x64/</AdditionalLibraryDirectories>
      <AdditionalDependencies>cairod.lib;fltkd.lib;fltkgld.lib;fltkimagesd.lib;fltkjpegd.lib;freetyped.lib;libpngd.lib;mtools64d.lib;pixmand.lib;zlibd.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>
	  <AdditionalOptions> %(AdditionalOptions)</AdditionalOptions>	  	  
    </Link>
	<Manifest>
      <EnableDpiAwareness>true</EnableDpiAwareness>
    </Manifest>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel> <PrecompiledHeader>Use</PrecompiledHeader> <Optimization>MaxSpeed</Optimization> <FunctionLevelLinking>true</FunctionLevelLinking> <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>;WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>$(CAIRO_LIB)/source/;$(CIMG_LIB)/source/;$(FLTK_LIB)/source/;$(FREETYPE_LIB)/source/include/freetype/;$(LIBJPEG_LIB)/source/;$(LIBPNG_LIB)/source/;$(MTOOLS_LIB)/mtools/headers/;$(PIXMAN_LIB)/source/pixman/;$(ZLIB_LIB)/source/</AdditionalIncludeDirectories>
      <DebugInformationFormat>None</DebugInformationFormat>
	  <AdditionalOptions>/Zm400 %(AdditionalOptions)</AdditionalOptions>
	  <OpenMPSupport>true</OpenMPSupport>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem> <GenerateDebugInformation>true</GenerateDebugInformation> <EnableCOMDATFolding>true</EnableCOMDATFolding> <OptimizeReferences>true</OptimizeReferences>
      <AdditionalLibraryDirectories>$(CAIRO_LIB)/win32/;$(FLTK_LIB)/win32/;$(FREETYPE_LIB)/win32/;$(LIBJPEG_LIB)/win32/;$(LIBPNG_LIB)/win32/;$(MTOOLS_LIB)/mtools/lib/;$(PIXMAN_LIB)/win32/;$(ZLIB_LIB)/win32/</AdditionalLibraryDirectories>
      <AdditionalDependencies>cairo.lib;fltk.lib;fltkgl.lib;fltkimages.lib;fltkjpeg.lib;freetype.lib;libpng.lib;mtools32.lib;pixman.lib;zlib.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>
	  <AdditionalOptions> %(AdditionalOptions)</AdditionalOptions>	  
    </Link>
	<Manifest>
      <EnableDpiAwareness>true</EnableDpiAwareness>
    </Manifest>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel> <PrecompiledHeader>Use</PrecompiledHeader> <Optimization>MaxSpeed</Optimization> <FunctionLevelLinking>true</FunctionLevelLinking> <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>;WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>$(CAIRO_LIB)/source/;$(CIMG_LIB)/source/;$(FLTK_LIB)/source/;$(FREETYPE_LIB)/source/include/freetype/;$(LIBJPEG_LIB)/source/;$(LIBPNG_LIB)/source/;$(MTOOLS_LIB)/mtools/headers/;$(PIXMAN_LIB)/source/pixman/;$(ZLIB_LIB)/source/</AdditionalIncludeDirectories>
      <DebugInformationFormat>None</DebugInformationFormat>
	  <AdditionalOptions>/Zm400 %(AdditionalOptions)</AdditionalOptions>
	  <OpenMPSupport>true</OpenMPSupport>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem> <GenerateDebugInformation>true</GenerateDebugInformation> <EnableCOMDATFolding>true</EnableCOMDATFolding> <OptimizeReferences>true</OptimizeReferences>
      <AdditionalLibraryDirectories>$(CAIRO_LIB)/x64/;$(FLTK_LIB)/x64/;$(FREETYPE_LIB)/x64/;$(LIBJPEG_LIB)/x64/;$(LIBPNG_LIB)/x64/;$(MTOOLS_LIB)/mtools/lib/;$(PIXMAN_LIB)/x64/;$(ZLIB_LIB)/x64/</AdditionalLibraryDirectories>
      <AdditionalDependencies>cairo.lib;fltk.lib;fltkgl.lib;fltkimages.lib;fltkjpeg.lib;freetype.lib;libpng.lib;mtools64.lib;pixman.lib;zlib.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>
	  <AdditionalOptions> %(AdditionalOptions)</AdditionalOptions>	  
    </Link>
	<Manifest>
      <EnableDpiAwareness>true</EnableDpiAwareness>
    </Manifest>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClInclude Include="stdafx.h" />

  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="main.cpp" />
    <ClCompile Include="stdafx.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Create</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Create</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">Create</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|x64'">Create</PrecompiledHeader>
    </ClCompile>
	
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>
"""


#################################################
#                                               #
# [project].vcxproj.filters                     #
#                                               #
#################################################
vcxfiltersFile = r"""<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>
      <Extensions>h;hh;hpp;hxx;hm;inl;inc;xsd</Extensions>
    </Filter>
    <Filter Include="Resource Files">
      <UniqueIdentifier>{67DA6AB6-F800-4c08-8B7A-83BB121AAD01}</UniqueIdentifier>
      <Extensions>rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav;mfcribbon-ms</Extensions>
    </Filter>

</ItemGroup>
<ItemGroup>
<ClInclude Include="stdafx.h"> <Filter>Header Files</Filter> </ClInclude>

</ItemGroup>
<ItemGroup>
<ClCompile Include="stdafx.cpp"> <Filter>Source Files</Filter> </ClCompile>
<ClCompile Include="main.cpp"> <Filter>Source Files</Filter> </ClCompile>

</ItemGroup>
</Project>
"""


#################################################
#                                               #
# stdafx.cpp                                    #
#                                               #
#################################################
stdafxcppFile = r"""
// precompiled header file.
#include "stdafx.h"
"""


#################################################
#                                               #
# stdafx.h                                      #
#                                               #
#################################################
stdafxhFile = r"""
// precompiled header
#pragma once

// *** STL ***
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>  
#include <cwchar>
#include <locale>

#include <utility>
#include <type_traits>
#include <typeinfo>
#include <string>
#include <algorithm>
#include <limits>
#include <initializer_list>
#include <chrono>

#include <ostream>
#include <fstream>
#include <iostream>

#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>


// *** fltk ***
#if defined (_MSC_VER) 
#pragma warning( push )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4319 )
#endif
#include "FL/Fl.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Box.H"
#include "FL/Fl_Input.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Check_Button.H"
#include "FL/Fl_Round_Button.H"
#include "FL/Fl_Toggle_Button.H"
#include "FL/Fl_Value_Slider.H"
#include "FL/Fl_Scroll.H"
#include "FL/Fl_Progress.H"
#include "FL/Fl_Pack.H"
#include "FL/Fl_Text_Display.H"
#include "FL/fl_ask.H"
#include "FL/Fl_Color_Chooser.H"
#include "FL/Fl_File_Chooser.H"
#include "FL/filename.H"
#include "FL/fl_draw.H"
#if defined (_MSC_VER) 
#pragma warning( pop )
#endif


/* end of file stdafx.h */
"""



############################################################################
# the python script itself 

import os
import shutil
import uuid
import sys

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
	str = str.replace("[PROJECT_NAME]",project_name)
	str = str.replace("[PROJECT_GUID]",project_guid)
	filepath = project_dir + "/" + filename
	try:	
		fout = open(filepath,"w")
		fout.write(str);
		fout.close()
	except:
		error("cannot write file [" + filepath + "]")	

		
# get the project name		
if (len(sys.argv) > 1):
	project_name = sys.argv[1]
else:
	project_name = myinput("Name of the project to create ? ") 

#  create a guid for the project
project_guid = str(uuid.uuid4())

# create the project directory 
project_dir  = os.getcwd() + "/" + project_name
if os.path.exists(project_dir):
	error("directory [" + project_dir + "] already exist")
try:	
	os.makedirs(project_dir)
except:
	error("cannot create directory [" + project_dir + "]")	
	
# copy the files		
repl(mainFile,"main.cpp")
repl(makeFile,"makefile")
repl(slnFile, project_name + ".sln")
repl(vcxprojFile, project_name + ".vcxproj")
repl(vcxfiltersFile, project_name + ".vcxproj.filters")
repl(stdafxcppFile,"stdafx.cpp")
repl(stdafxhFile,"stdafx.h")

#done !
print "\n*** Project " + project_name + " created ! ***"

# end of script mtoolsproject.py
############################################################################
