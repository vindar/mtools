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
			* Query if the circle is non-empty ie if the radius is strictly positive.
			**/
			bool isNonempty() const { return (radius > (FPTYPE)0); }


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
				MTOOLS_INSURE(radius >= (FPTYPE)0);
				const FPTYPE d = std::abs(center);
				if (!(d + radius < 1)) { // horocycle
					MTOOLS_INSURE(d >(FPTYPE)0);
					if (!(-radius < (FPTYPE)0)) return Circle(center/d, (FPTYPE)1); else return Circle(center/d, -radius);
					}
				if (!(radius > (FPTYPE)0)) { return Circle(center, (FPTYPE)1); }
				const FPTYPE r2 = radius*radius;
				const FPTYPE d2 = std::norm(center);
				const FPTYPE num = 1 - 2*radius + r2 - d2;
				if (!(num > (FPTYPE)0)) { // horocycle
					MTOOLS_INSURE(d > (FPTYPE)0);
					if (!(-radius < (FPTYPE)0)) return Circle(center / d, (FPTYPE)1); else return Circle(center / d, -radius);
					}
				const FPTYPE denom = 1 + 2 * radius + r2 - d2;
				const FPTYPE s = sqrt(num / denom);
				MTOOLS_INSURE(!isnan(s));
				if (!(d > 0)) { return Circle(center, s); }
				const FPTYPE denom2 = 1 - 2*d + d2 - r2;
				if (!(denom2 > (FPTYPE)0)) { // horocycle
					if (!(-radius < (FPTYPE)0)) return Circle(center / d, (FPTYPE)1); else return Circle(center / d, -radius);
					}
				const FPTYPE num2 = 1 + 2*d + d2 - r2;
				const FPTYPE v = sqrt(num2 / denom2);
				const FPTYPE l = (v - 1) / (v + 1);
				const FPTYPE g = l / d;
				MTOOLS_INSURE(!isnan(g));
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
				/* TODO : fix NaN and other possible overflow during calculation (like in the method  euclidianToHyperbolic()) */
				const FPTYPE & s = radius;
				if (s < 0) { return Circle(((FPTYPE)1 + s)*center, -s); } // horocycle
				MTOOLS_INSURE((s > 0)&&(s<=1));
				if (s >= 1) return Circle(center, (FPTYPE)0);
				const FPTYPE l = std::abs(center);  const FPTYPE g = (1 + l) / (1 - l);
				const FPTYPE a = g / s;               const FPTYPE d = (a - 1) / (a + 1);
				const FPTYPE b = g*s;               const FPTYPE k = (b - 1) / (b + 1);
				return Circle(((l <= 0) ? complex<FPTYPE>((FPTYPE)0, (FPTYPE)0) : (((d + k) / (l*((FPTYPE)2)))*center)), (d - k) / ((FPTYPE)2));
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



	}

/* end of file */



