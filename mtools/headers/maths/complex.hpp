/** @file complex.hpp */
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


	template<typename T> using complex = std::complex<T>;   ///< using stl complex class by default. 





	template<typename  T> class Mobius
		{

		public:

		/** Default constructor. set the identity transformation. */
		Mobius() : a((T)1.0), b(T(0.0)), c((T)0.0), d(T(1.0)) {}

		/**
		 * Constructor. constuct the mobius transformation z -> (az + b)/(cz + d).
		 */
		Mobius(const mtools::complex<T> & aa,const mtools::complex<T> & bb,const mtools::complex<T> & cc, const mtools::complex<T> & dd) : a(aa), b(bb), c(cc), d(dd) {}
		

		/**   
		/* Mobius tranformation z -> (z - c)/(conj(c)z - 1).
		 * This swaps c and 0  while preserving the unit disk if |c| < 1.
		 **/
		Mobius(const mtools::complex<T> & c) : a((T)1.0), b(-c), c(std::conj(c)), d((T)(-1.0)) {}


		/**
		 * Composition of two mobius tranformation.
		 */
		Mobius operator*(const Mobius & M) const 
			{
			return Mobius(a*M.a + b*M.c, a*M.b + b*M.d, c*M.a + d*M.c, c*M.b + d*M.d);
			}


		/**
		 * Compute the image of a point.
		 */
		mtools::complex<T> operator*(const mtools::complex<T> & z) const
			{
			return (a * z + b)/(c * z + d);
			}


		/**
		 * Return the invert transformation.
		 */
		Mobius invert() const
			{
			return Mobius(d, -b, -c, a);
			}


		/**
		 * Compute the image of a circle by the mobius transformation. It is again a circle (but beware
		 * that the new center is not the image of the originial center).
		 */
		std::pair<std::complex<T>, T> imageCircle(const std::complex<T> & center, const T & rad) const
			{
			return std::pair<std::complex<T>, T>(
				((a*center + b)*(std::conj(c*center + d)) - rad*rad*a*(std::conj(c))) / (std::norm(c*center + d) - rad*rad*(std::norm(c))),
				(rad*(std::abs(a*d - b*c))) / (std::abs(std::norm(c*center + d) - rad*rad*norm(c)))
				);
			}


		/**
		* serialise/deserialize the tranformation. Works with boost and with the custom serialization classes
		* OArchive and IArchive. the method performs both serialization and deserialization.
		**/
		template<typename U> void serialize(U & Archive, const int version = 0)
			{
			Archive & a & b & c & d;
			}


		/**
		* Print the the vector into a string.
		**/
		std::string toString() const
			{
			std::string s("Mobius[");
			return(s + mtools::toString(a) + "," + mtools::toString(b) + "," + mtools::toString(c) + "," + mtools::toString(d) + "]");
			}



		mtools::complex<T> a;
		mtools::complex<T> b;
		mtools::complex<T> c;
		mtools::complex<T> d;

		};



	}

/* end of file */

