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
 * project: 
 * created:
 ***********************************************/

#include "mtools/mtools.hpp" 


int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc,argv); // required on OSX, does nothing on Linux/Windows
    mtools::parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
    mtools::cout << "Hello World\n"; 
    mtools::cout.getKey();	
    return 0;
    }
	
/* end of file main.cpp */
"""



