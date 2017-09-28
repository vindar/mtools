/** @file misc.hpp */
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

#include "../misc/internal/mtools_export.hpp"

#include <cstdint>
#include <limits>


namespace mtools
{

   typedef int8_t       int8;
   typedef uint8_t      uint8;
   typedef int16_t      int16;
   typedef uint16_t     uint16;
   typedef int32_t      int32;
   typedef uint32_t     uint32;
   typedef int64_t      int64;
   typedef uint64_t     uint64;

   /** Defines an alias const char *. */
   typedef const char * cp_char;

   /** Defines an alias to const wchar_t *.  */
   typedef const wchar_t * cp_wchar_t;

   /** Defines an alias const char *. */
   typedef char * p_char;

   /** Defines an alias to const wchar_t *.  */
   typedef wchar_t * p_wchar_t;

   /** Defines an alias to const void *. */
   typedef const void * cp_void;

   /** Defines an alias to void *. */
   typedef void * p_void;

   /*  stl complex class by default.  */
   template<typename T> using complex = std::complex<T>; 


   /** Numeric constants */

   const double PI        = 3.141592653589793238;
   const double TWOPI     = 6.283185307179586477;
   const double PIOVERTWO = 1.570796326794896619;

   const double NaN = std::numeric_limits<double>::quiet_NaN();

   const double INF = std::numeric_limits<double>::infinity();


#ifndef M_PI
#define M_PI    3.141592653589793238
#endif

#ifndef M_PI_2
#define M_PI_2  1.570796326794896619
#endif

#ifndef M_2PI
#define M_2PI   6.283185307179586477
#endif



   /* round up to the next power of 2 */
   inline int pow2roundup(int x)
	   {
	   --x; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
	   return x + 1;
	   }

}


/* end of file */

