/** @file randomgraph.hpp */
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
#include "../misc/error.hpp"
#include "../maths/specialFunctions.hpp"
#include "classiclaws.hpp"

#include <cmath>
#include <random>

namespace mtools
	{


	/**
	 * Perform a uniform permutation of a vector. 
	 *
	 * @tparam	random_t	Type of the random number generator
	 * @tparam	Vector  	Type of the vector. Must implement size() and operator[]. 
	 * 						For example: std::vector of std::deque
	 * @param [in,out]	gen	the rng
	 * @param [in,out]	vec	the vector
	 *
	 * @return	An uint32.
	 **/
	template<class Vector, class random_t> inline void randomShuffle(Vector & vec, random_t & gen)
		{
		cout << vec << "\n";
		const size_t l = vec.size();
		if (l < 2) return;
		for (size_t i = l-1; i > 0; --i)
			{
			size_t j = (size_t)(Unif(gen)*(i + 1));
			std::remove_reference<decltype(vec[0])>::type temp(vec[i]); 
			vec[i] = vec[j];
			vec[j] = temp;
			}
		cout << vec << "\n";
		}


	/**
	 * Construct a uniform random permuation of {0,...,n}
	 *
	 * @tparam	random_t	Type of the random number generator
	 * @tparam	Vector  	Type of the vector. Must implement resize() and operator[].
	 * 						For example: std::vector of std::deque
	 * @param	n		   	permutation of size n+1
	 * @param [in,out]	gen	the rng
	 *
	 * @return	a uniform random permutation. 
	 **/
	template<class Vector, class random_t> inline Vector uniformRandomPermutation(int n, random_t & gen)
		{
		Vector vec;
		vec.resize(n);
		for (int i = 0; i < n; i++) { vec[i] = i; }
		randomShuffle(vec, gen);
		return vec;
		}





	/* construct dyck word */

	}


/* end of file  */


