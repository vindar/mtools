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

#include "../misc/error.hpp"

#include <cstdint>
#include <limits>
#include <complex>


// macro to force inlining of a function. 
#if defined (_MSC_VER) 
#define MTOOLS_FORCEINLINE __forceinline
#else
#define MTOOLS_FORCEINLINE __attribute__((always_inline)) inline
#endif




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

   /** Defines an alias representing an unsigned char. */
   typedef unsigned char	uchar;

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


	/** Round down to the previous power of 2. If x is a power of 2, return x. If x <= 0 returns 0. **/
	MTOOLS_FORCEINLINE int32 pow2rounddown(int32 z) { int32 x = (z >> 1); x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; return((x + 1) & (0xFFFFFFFE | z)); }

	/** Round down to the previous power of 2. If x is a power of 2, return x.  If x=0 returns 0. **/
	MTOOLS_FORCEINLINE uint32 pow2rounddown(uint32 z) { uint32 x = (z >> 1); x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; return((x + 1) & (0xFFFFFFFE | z)); }

	/** Round down to the previous power of 2. If x is a power of 2, return x, If x <= 0 returns 0. **/
	MTOOLS_FORCEINLINE int64 pow2rounddown(int64 z) { int64 x = (z >> 1); x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; x |= x >> 32; return((x + 1) & (0xFFFFFFFFFFFFFFFEULL | z)); }

	/** Round down to the previous power of 2. If x is a power of 2, return x.  If x=0 returns 0. **/
	MTOOLS_FORCEINLINE uint64 pow2rounddown(uint64 z) { uint64 x = (z >> 1); x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; x |= x >> 32; return((x + 1) & (0xFFFFFFFFFFFFFFFEULL | z)); }


	/** Round up to the next power of 2. If x is a power of 2 return x. If x = 0 return 0 (works only for 0 <= x <= 2^30). **/
	MTOOLS_FORCEINLINE int32 pow2roundup(int32 x) { --x; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; return x + 1; }

	/** round up to the next power of 2. If x is a power of 2 return x. If x = 0 return 0 (works only for x <= 2^31). **/
	MTOOLS_FORCEINLINE uint32 pow2roundup(uint32 x) { --x; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; return x + 1; }

	/** Round up to the next power of 2. If x is a power of 2 return x. If x = 0 return 0 (works only for 0 <= x <= 2^62). **/
	MTOOLS_FORCEINLINE int64 pow2roundup(int64 x) { --x; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; x |= x >> 32; return x + 1; }

	/** Round up to the next power of 2. If x is a power of 2 return x. If x = 0 return 0 (works only for x <= 2^63). **/
	MTOOLS_FORCEINLINE uint64 pow2roundup(uint64 x) { --x; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; x |= x >> 32; return x + 1; }



	/**
	* Return the position of the highest bit set.
	*
	* @param	x	the value to compute the highest bit set position.
	*
	* @return	the 1-based index of the highest bit set (i.e. in {1,..,32} for x > 0) and 0 for x=0.
	**/
	MTOOLS_FORCEINLINE uint32 highestBit(uint32 x)
		{
		uint32 i = 1;
		if (!x) return 0;
		if (x & 0xFFFF0000) { i += 16;  x >>= 16; }
		if (x & 0xFF00) { i += 8; x >>= 8; }
		if (x & 0xF0) { i += 4; x >>= 4; }
		if (x & 0x0C) { i += 2; x >>= 2; }
		if (x & 0x2) { i += 1; }
		return i;
		}


	/**
	* Return the position of the highest bit set.
	*
	* @param	x	the value to compute the highest bit set position.
	*
	* @return	the 1-based index of the highest bit set (i.e. in {1,..,64} for x > 0) and 0 for x=0.
	**/
	MTOOLS_FORCEINLINE uint32 highestBit(uint64 x)
		{
		uint32 i = 1;
		if (!x) return 0;
		if (x & 0xFFFFFFFF00000000) { i += 32;  x >>= 32; }
		if (x & 0xFFFF0000) { i += 16;  x >>= 16; }
		if (x & 0xFF00) { i += 8; x >>= 8; }
		if (x & 0xF0) { i += 4; x >>= 4; }
		if (x & 0x0C) { i += 2; x >>= 2; }
		if (x & 0x2) { i += 1; }
		return i;
		}



	/* Return a value U smaller or equal to B such that */


	/**
	 * Return a value smaller or equal to B such that the multiplication 
	 * by A is safe (no overflow with int64). 
	 * 
	 * @param	A	first value to multiply
	 * @param	B	second value to multiply (must be non-negative)
	 *
	 * @return	A value U <= B such that A*U fits in a int64 (no overflow). 
	 * 			Return B if possible. 
	 **/
	MTOOLS_FORCEINLINE int64 safeMultB(int64 A, int64 B)
		{
		MTOOLS_ASSERT(B >= 0);
		if ((A == 0)||(B==0)) return B;
		const int64 max64 = 9223372036854775807;
		const int64 nB = max64 / ((A > 0) ? A : (-A));
		return ((B <= nB) ? B : nB);
		}





	/**
	 * another swap, because std::swap is troublesome....
	 **/
	template<typename T> MTOOLS_FORCEINLINE  void swap(T & a, T & b) { T c(a); a = b; b = c; }


	/**
	 * This method does nothing !
	 * 
	 * Useful for (condtional) macro definition when it should compile to nothing.
	 **/
	inline void doNothing() {}


	/**
	* Test if a (double) number is an integer.
	*/
	MTOOLS_FORCEINLINE bool isIntegerValued(double v) { return (round(v) == v); }


}


/* end of file */

