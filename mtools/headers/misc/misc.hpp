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


   /** Numeric constants */

   const double PI  = 3.1415926535897932384626433832795;

   const double TWOPI = 6.283185307179586476925286766559;

   const double NaN = std::numeric_limits<double>::quiet_NaN();

   const double INF = std::numeric_limits<double>::infinity();


}


/* end of file */

