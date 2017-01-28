/** @file circle.hpp */
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


#include "../misc/misc.hpp" 
#include "../misc/stringfct.hpp" 
#include "../misc/error.hpp"


namespace mtools
	{


	/**
	 * Class that define a circle inside the (complex) plane.
	 *
	 * @tparam	FPTYPE	floating point type to use.
	 **/
	template<typename FPTYPE = double> class Circle
		{

		public:


			/**
			* Constructor.
			**/
			Circle(std::complex<FPTYPE> & centerpos, const FPTYPE & rad) : center(centerpos), radius(rad) {}


			/**
			 * Sets the rounding precision (default 0).
			 **/
			static void setPrecision(const FPTYPE & precision) { _eps = precision; }


			/**
			* Query if the circle is non-empty ie if the radius is strictly positive.
			**/
			bool isNonempty() const { return (radius > (FPTYPE)0); }


			/**
			* Query if this circle may be interpreted as an hyperbolic circle
			* i.e. if it is non-empty and lies inside the closed unit disk.
			**/
			bool isHyperbolic() const { return((isNonEmpty()) && (std::norm(center) + radius <= (FPTYPE)1 + _eps)); }


			/**
			* Query if the circle is an horocycle i.e. it is non-empty, lies inside the closed unit disk and
			* is tangent to the unit circle.
			**/
			bool isHorocycle() const
				{
				FPTYPE d = std::norm(center) + radius;
				return((isNonEmpty()) && (d <= (FPTYPE)1 + _eps) && (d >= (FPTYPE)1 - _eps));
				}


			/**
			 * Convert a circle from a euclidian representation to the hyperbolic one.
			 *
			 * The radius for the hyperbolic circle is the 'x-radius' ie such  that x = 1 - exp(-2h)
			 * where h is the real hyperbolic radius. Hence it is a number between 0 (null radius) and 1
			 * (infinite radius).
			 *
			 * @return	The 'hyperbolic' circle given by its hyperbolic center inside the unit disk and its
			 * 			hyperbolic x-radii.
			 * 			If the circle is an horocycle Then the center is set at the tangency point and the
			 * 			radius is set as the negative of the euclidian radius (doing this instead of setting
			 * 			the x-radius to 1 enables to go back to euclidian later on).
			 **/
			Circle euclidianToHyperbolic() const
				{
				MTOOLS_ASSERT(radius >= -_eps); // radius should not be negative
				const FPTYPE d = std::abs(center);
				if (radius <= _eps) { MTOOLS_ASSERT(d <= (FPTYPE)1 + _eps); return *this; } // radius is zero. 
				MTOOLS_ASSERT(d + radius <= (FPTYPE)1 + _eps); // the circle should be contained in the closed unit disk
				if (d + radius >= (FPTYPE)1 - _eps) 
					return Circle(((d<=_eps) ? (complex<FPTYPE>((FPTYPE)0, (FPTYPE)0)) :  (center/d)), -radius); // horocycle
				const FPTYPE d2 = std::norm(center);
				const FPTYPE radius2 = radius*radius;
				const FPTYPE s = sqrt((1 - 2 * radius + radius2 - d2) / (1 + 2 * radius + radius2 - d2));
				const FPTYPE hyp_rad = (FPTYPE)1 - s*s;
				if (hyp_rad >= (FPTYPE)1 - _eps) return Circle(center / d, -radius); // horocycle
				if (d <= _eps) return Circle(complex<FPTYPE>((FPTYPE)0, (FPTYPE)0), hyp_rad);
				const FPTYPE v = sqrt(((FPTYPE)1 + 2 * d + d2 - radius2) / ((FPTYPE)1 - 2 * d + d2 - radius2));
				const FPTYPE l = (v - 1) / (v + 1);
				const FPTYPE g = l / d;
				return Circle(g*center, hyp_rad);
				}


			/**
			 * Convert an hyperbolic circle to its euclidian representation.
			 *
			 * The input hyperbolic circle is parametrised by its hyperbolic center and its hyperbolic x-
			 * radius defined by x = 1 - exp(-2h) where h is the real hyperbolic radius.
			 * If the hyperbolic circle is an horocycle, its radius should be the inverse of the euclidian
			 * raddii (as obtained with euclidianToHyperbolic() ).
			 *
			 * @return	The corresponding euclidian circle.
			 **/
			Circle hyperbolicToEuclidian() const
				{
				if (radius <= _eps) // radii is negative or zero
					{
					if (radius >= -_eps) return *this; // null radius
					return Circle((1 + radius)*center, -radius);	// horocycle				
					}
				MTOOLS_ASSERT(radius < (FPTYPE)1); // should be a real circle
				const FPTYPE s = sqrt(1 - radius);
				const FPTYPE l = std::abs(center);  const FPTYPE g = (1 + l) / (1 - l);
				const FPTYPE a = g / s;               const FPTYPE d = (a - 1) / (a + 1);
				const FPTYPE b = g*s;               const FPTYPE k = (b - 1) / (b + 1);
				return Circle(((l <= _eps) ? complex<FPTYPE>((FPTYPE)0, (FPTYPE)0) : (((d + k) / (l*((FPTYPE)2)))*center)), (d - k) / ((FPTYPE)2));
				}



			/**
			* serialise/deserialize the tranformation. Works with boost and with the custom serialization classes
			* OArchive and IArchive. the method performs both serialization and deserialization.
			**/
			template<typename U> void serialize(U & Archive, const int version = 0)
				{
				Archive center & radius;
				}


			/**
			* Print the circle into a string.
			**/
			std::string toString() const
				{
				return(std::string("Circle[center=") + mtools::toString(center) + ", radius=" + mtools::toString(radius) + "]");
				}


			complex<FPTYPE> center;

			FPTYPE radius;

		private:

			static FPTYPE _eps; // precision. 

		};




	template<typename FPTYPE> FPTYPE Circle<FPTYPE>::_eps = (FPTYPE)0;


	}

/* end of file */



