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
			* Constructor. Circle of null radius centered at the origin.
			**/
			Circle() : center(complex<FPTYPE>((FPTYPE)0, (FPTYPE)0)), radius((FPTYPE)0) {}


			/**
			* Constructor.
			**/
			Circle(const std::complex<FPTYPE> & centerpos, const FPTYPE & rad) : center(centerpos), radius(rad) {}


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
			 * The radius for the hyperbolic circle is the s-radius ie such that s = exp(-h) where h 
			 * is the real hyperbolic radius. Hence it is a number between 0 (infinite radius) and 1
			 * (null radius).
			 *
			 * @return	The 'hyperbolic' circle given by its hyperbolic center inside the unit disk and its
			 * 			hyperbolic s-radius
			 * 			If the circle is an horocycle Then the center is set to the tangency point and the
			 * 			radius is set as the negative of the euclidian radius (doing this instead of setting
			 * 			the s-radius to 0 enables to go back to euclidian later on).
			 **/
			Circle euclidianToHyperbolic() const
				{
				MTOOLS_ASSERT(radius >= (FPTYPE)0);				// radius should not be negative
				const FPTYPE d = std::abs(center);				// distance to origin
				MTOOLS_ASSERT(d + radius <= (FPTYPE)1 + _eps); // the circle should be contained in the closed unit disk
				if (radius <= _eps) {  return Circle(center, 1 - radius); } // radius is zero. 				
				if (d + radius >= (FPTYPE)1 - _eps) return Circle(((d<=_eps) ? (complex<FPTYPE>((FPTYPE)0, (FPTYPE)0)) :  (center/d)), -radius); // horocycle
				const FPTYPE d2 = std::norm(center);
				const FPTYPE radius2 = radius*radius;
				const FPTYPE s = sqrt((1 - 2 * radius + radius2 - d2) / (1 + 2 * radius + radius2 - d2));
				if (s <= _eps) return Circle(((d <= _eps) ? (complex<FPTYPE>((FPTYPE)0, (FPTYPE)0)) : (center / d)), -radius); // horocycle
				if (d <= _eps) return Circle(complex<FPTYPE>((FPTYPE)0, (FPTYPE)0), s);
				const FPTYPE v = sqrt(((FPTYPE)1 + 2 * d + d2 - radius2) / ((FPTYPE)1 - 2 * d + d2 - radius2));
				const FPTYPE l = (v - 1) / (v + 1);
				const FPTYPE g = l / d;
				return Circle(g*center, s);
				}


			/**
			 * Convert an hyperbolic circle to its euclidian representation.
			 *
			 * The input hyperbolic circle is gven by its hyperbolic center and its hyperbolic s-radius defined 
			 * by s = exp(-h) where h is the real hyperbolic radius. If the hyperbolic circle is an horocycle, its 
			 * radius should be the inverse of the euclidian one (compatible with euclidianToHyperbolic() ).
			 *
			 * @return	The corresponding euclidian circle.
			 **/
			Circle hyperbolicToEuclidian() const
				{
				const FPTYPE & s = radius; // radius is the s-radius
				if (s < 0) { return Circle(((FPTYPE)1 + s)*center, -s); } // horocycle
				MTOOLS_ASSERT(s > 0); // should be a real circle, not infinite raduis.
				if (s >= (FPTYPE)1 - _eps) return Circle(center, (FPTYPE)1 - s);
				const FPTYPE l = std::abs(center);  const FPTYPE g = (1 + l) / (1 - l);
				const FPTYPE a = g / s;               const FPTYPE d = (a - 1) / (a + 1);
				const FPTYPE b = g*s;               const FPTYPE k = (b - 1) / (b + 1);
				return Circle(((l <= _eps) ? complex<FPTYPE>((FPTYPE)0, (FPTYPE)0) : (((d + k) / (l*((FPTYPE)2)))*center)), (d - k) / ((FPTYPE)2));
				}


			/**
			 * shift the center of the circle
			 **/
			Circle & operator+=(const complex<FPTYPE> & pos) { center += pos; return *this; }


			/**
			* shift the center of the circle
			**/
			Circle & operator-=(const complex<FPTYPE> & pos) { center -= pos; return *this; }


			/**
			* Apply the transformation z-> lambda z to the circle.
			**/
			Circle & operator*=(const FPTYPE & lambda) { radius *= lambda; center *= lambda; return *this; }


			/**
			* Apply the transformation z-> z/lambda to the circle.
			**/
			Circle & operator/=(const FPTYPE & lambda) { radius /= lambda; center /= lambda; return *this; }


			/**
			* serialise/deserialize the tranformation. Works with boost and with the custom serialization classes
			* OArchive and IArchive. the method performs both serialization and deserialization.
			**/
			template<typename U> void serialize(U & Archive, const int version = 0)
				{
				Archive & center & radius;
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



		/**
		* shift the center of the circle
		**/
		template<typename FPTYPE> Circle<FPTYPE> operator+(Circle<FPTYPE> circle, const complex<FPTYPE> & pos) { circle += pos; return circle; }


		/**
		* shift the center of the circle
		**/
		template<typename FPTYPE> Circle<FPTYPE> operator-(Circle<FPTYPE> circle, const complex<FPTYPE> & pos) { circle -= pos; return circle; }


		/**
		* Apply the transformation z-> lambda z to the circle.
		**/
		template<typename FPTYPE> Circle<FPTYPE> operator*(Circle<FPTYPE> circle, const complex<FPTYPE> & lambda) { circle *= lambda; return circle; }
		template<typename FPTYPE> Circle<FPTYPE> operator*(const complex<FPTYPE> & lambda, Circle<FPTYPE> circle) { circle *= lambda; return circle; }


		/**
		* Apply the transformation z-> z/lambda to the circle.
		**/
		template<typename FPTYPE> Circle<FPTYPE> operator/(Circle<FPTYPE> circle, const complex<FPTYPE> & lambda) { circle /= lambda; return circle; }




	template<typename FPTYPE> FPTYPE Circle<FPTYPE>::_eps = (FPTYPE)0;


	}

/* end of file */



