/** @file mtools-config.hpp */
//
// Program that return the options needed to build against mtools. 
// 
//
// Copyright 2015 Arvind Singh
//
// This file is part of the mtools library.
//
// mtools is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mtools  If not, see <http://www.gnu.org/licenses/>.


#include <iostream>
#include <string>
#include <stdio.h>


#include "../mtools/headers/mtools_config.hpp" // config file for mtools. 


/** replace char <32 by spaces. */
void nl_to_space(std::string & s)
{
	for (size_t i = 0; i < s.length(); i++) { if (s[i] < 32) s[i] = ' ';  }
}


/**
 * Executes a command and return the result of the standart output into a string. 
 * use a command of the form "command 2>&1" to redirect stderr to stdout.
 * The method exit(1) if the command fails.  
 **/
std::string exec_command(const std::string & command)
	{
	FILE * handle = popen(command.c_str(), "r");
	if (handle == nullptr)
		{
		std::cerr << "*** Error executing command [" << command << "] ***\n";
		exit(1);
		}
	std::string res;
	char buf[512];
	while (fgets(buf, sizeof(buf), handle) != nullptr) { res.append(buf); }
	pclose(handle);
	nl_to_space(res);
	return res;
	}




/**
 * Return the cxx flags required for compiling against mtools. 
 **/
std::string get_cxxflags()
	{
	std::string fl; 
	std::string sp(" ");

	// General flags 
	fl += "-std=c++1z"; 

	// openmp
	if (MTOOLS_USE_OPENMP) fl += sp + "-fopenmp "; 

	// fltk
	fl += sp + exec_command("fltk-config --cxxflags");

	// cairo
	if (MTOOLS_USE_CAIRO) fl += sp + exec_command("pkg-config cairo --cflags");

	return fl;
	}


/**
* Return the linker flags required for compiling against mtools.
**/
std::string get_ldflags()
{
	std::string fl;
	std::string sp(" ");

	// mtools 
	fl += "-lmtools";

	// fltk
	fl += sp + exec_command("fltk-config --ldstaticflags");

	// cairo
	if (MTOOLS_USE_CAIRO) fl += sp + exec_command("pkg-config cairo --libs");

	// openCL
	if (MTOOLS_USE_OPENCL) fl += sp + "-lOpenCL";
	 
	// libjpeg
	fl += sp + "-ljpeg";

	// libpng
	fl += sp + "-lpng";

	// zlib
	fl += sp + "-lz";

	return fl;
}




int main(int argc, char *argv[])
	{
	if (argc == 2)
		{
		std::string opt(argv[1]);
		if (opt == std::string("--cxxflags"))
			{
			std::cout << get_cxxflags();
			return 0;
			}

		if (opt == std::string("--ldflags"))
			{
			std::cout << get_ldflags();
			return 0;
			}
		}
	std::cout << "mtools-config. Get compiler/linker flags for building against mtools.\n\n";
	std::cout << "USAGE [mtools-config --flag]\n\n";
	std::cout << "with flag = cxxflags [compiler options]\n";
	std::cout << "          = ldflags [linker options]\n\n";
	return 0;
	}

/* end of file */




