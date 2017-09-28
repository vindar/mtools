/** @file mtools_export.hpp */
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


#pragma once


/* Macros for DLL import/export flag 
 *
 * Set BUILDING_MTOOLS_DLL when building the library
 * and unset it when using the library.
 **/

#ifdef BUILDING_MTOOLS_DLL

	#if defined (_MSC_VER) 
		#define MTOOLS_DLL __declspec(dllexport)
	#else
		#define MTOOLS_DLL __attribute__((visibility("default")))
	#endif

#else

	#if defined (_MSC_VER) 
		#define MTOOLS_DLL __declspec(dllimport)
	#else
		define MTOOLS_DLL
	#endif

#endif


/* end of file */


