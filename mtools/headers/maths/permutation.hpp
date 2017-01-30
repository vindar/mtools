/** @file combinatorics.hpp */
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
#include "vec.hpp"
#include "box.hpp"

namespace mtools
	{



	/**
	* Perform a uniform shuffle of a vector.
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
		const size_t l = vec.size();
		if (l < 2) return;
		for (size_t i = l - 1; i > 0; --i)
			{
			size_t j = (size_t)(Unif(gen)*(i + 1));
			std::remove_reference<decltype(vec[0])>::type temp(vec[i]);
			vec[i] = vec[j];
			vec[j] = temp;
			}
		}




	/**  
	*  Class representing a permutation of {0,...,N-1} 
	**/
	class Permutation
		{

		public:

		/**
		* Constructor. empty permutation. 
		**/
		Permutation() : _perm(), _invperm() {}


		/**
		 * Constructor. Identity permutation of a given size.
		 **/
		Permutation(size_t size) : _perm(), _invperm()	{ setIdentity(size); }


		/**
		 * Constructor. Create the permutation that sorts the given sequence of label in increasing order. 
		 * (perm[i] = k means that the label initially at position i is now at position k
		 *  after they are sorted increasingly).
		 **/
		template<typename T> Permutation(const std::vector<T> & labels) : _perm(), _invperm() { setSortPermutation(labels); }


		/**
		 * Sets the permutation as the identity on the set of size elements.
		 **/
		void setIdentity(size_t size)
			{
			_perm.resize(size); _invperm.resize(size);
			for (int i = 0; i < (int)size; i++) { _perm[i] = i; _invperm[i] = i; }
			}


		/**
		* Sets the permutation as the transposition of i and j in {0,...,size-1}
		**/
		void setTransposition(size_t i, size_t j, size_t size)
			{
			MTOOLS_INSURE((i < size) && (j < size));
			setIdentity(size);
			_perm[i] = (int)j; _perm[j] = (int)i;
			_invperm[i] = (int)j; _invperm[j] = (int)i;
			}


		/**
		* Sets the permutation as the cycle _perm[i] = (i + k)%size
		**/
		void setCycle(int k, size_t size)
			{
			_perm.resize(size); _invperm.resize(size);
			int l = (int)size;
			if (l == 0) return;
			const int k1 = (((k % l) + l) % l); // makes sure k \in {0,size-1}
			const int k2 = l - k1; 
			for (size_t i = 0; i < size; i++)
				{
				_perm[i] = (i + k1) % l; _invperm[i] = (i + k2) % l;
				}
			}


		/* Set the permutation as the involution perm[i] = size-1 - i */
		void setMirror(size_t size)
			{
			_perm.resize(size); _invperm.resize(size);
			for (int i = 0; i < size; i++) { _perm[i] = (int)size - 1 - i; _invperm[i] = (int)size - 1 - i; }
			}


		/**
		 * Sets the object as the permutation that reorders labels in increasing order.
		 * (perm[i] = k means that the label initially at position i is now at position k
		 *  after they are sorted increasingly).
		 **/
		template<typename T> void setSortPermutation(const std::vector<T> & labels)
			{
			const size_t l = labels.size();
			setIdentity(l);
			if (l == 0) return;
			sort(_perm.begin(), _perm.end(), [&](const int & x, const int & y) { return labels[x] < labels[y]; });
			_recomputeInvert();
			}



		/**
		* Create a random permutation, uniform among all permutations of a given size.
		**/
		template<class random_t> inline void setRandomPermutation(size_t size, random_t & gen)
			{
			setIdentity(size); shuffle(gen);
			}


		/**
		 * Shuffles the permutation, making it uniform among all permutation of this size. 
		 * Same as setRandomPermutation() but without changing the current size. 
		 **/
		template<class random_t> inline void shuffle(random_t & gen)
			{
			if (_perm.size() != 0) { mtools::randomShuffle(_perm); _recomputeInvert(); }
			}



		/**
		* Re-order a vector of labels according to the permutation.
		* perm[i] = k means that label L(k) initially at position k must be put at pos i.
		* 
		* see getAntiPermute() for the opposite transformation.
		**/
		template<typename VECTOR> VECTOR getPermute(const VECTOR & labels) const
			{
			const size_t l = labels.size();
			MTOOLS_INSURE(_perm.size() == l);
			VECTOR res(l);
			for (size_t i = 0; i < l; i++) { res[i] = labels[_perm[i]]; }
			return res;
			}


		/**
		* Re-order a vector of labels according to the permutation.
		* perm[i] = k means that label L(i) initially at position i must be put at pos k.
		*
		* see getPermute() for the opposite transformation.
		**/
		template<typename VECTOR> VECTOR getAntiPermute(const VECTOR & labels) const
			{
			const size_t l = labels.size();
			MTOOLS_INSURE(_perm.size() == l);
			VECTOR res(l);
			for (size_t i = 0; i < l; i++) { res[_perm[i]] = labels[i]; }
			return res;
			}


		/**
		* Invert the permutation (very fast, just a swap). 
		**/
		void invert() { _perm.swap(_invperm); }


		/**
		 * Return the inverse permutation
		 **/
		Permutation getInverse() const { Permutation P(*this); P.invert(); return P; }


		/**
		 * Convert the permutation to a vector
		 **/
		operator std::vector<int>() const { return _perm; }

		/**
		 * Query the size of the permutation
		 **/
		size_t size() const { return _perm.size(); }


		/**
		 * Resize the permutation. 
		 * - If the size is increased, then perm[k] = k for all new elements. 
		 * - If the size is decreased, the subset {newsize,..., size-1} must  
		 * be stable by the permutation otherwise an error is raised.
		 **/
		void resize(int newsize)
			{
			const size_t l = _perm.size();
			if (newsize >= l) // increase size
				{ 
				_perm.resize(newsize); _invperm.resize(newsize);
				for (size_t i = l; i < newsize; i++) { _perm[i] = (int)i; _invperm[i] = (int)i; }
				return;
				}

			for (size_t i = newsize; i < l; i++)
				{
				if (_perm[i] < newsize) { MTOOLS_ERROR(std::string("Subset is not stable: perm[") + mtools::toString(i) + "]=" + mtools::toString(_perm[i]) + " < newsize = " + mtools::toString(newsize)); }
				}
			_perm.resize(newsize); _invperm.resize(newsize);
			}


		/**
		* Clear the object, making it a empty permutation.
		**/
		void clear() { _perm.clear(); _invperm.clear(); }


		/**
		 * Return perm[index].
		 **/
		int operator[](int index) const { MTOOLS_ASSERT((index > 0) && (index < _perm.size())); return _perm[index]; }


		 /**
		 * Return invperm[index].
		 **/
		int inv(int index) const { MTOOLS_ASSERT((index > 0) && (index < _perm.size())); return _invperm[index]; }


		/**
		 * Composition operator ie (P1*P2)[k] = P1[P2[k]]
		 **/
		Permutation operator*(const Permutation & P2) const
			{
			MTOOLS_INSURE(_perm.size() == P2.size());
			const int l = (int)_perm.size();
			Permutation R;
			R._perm.resize(l); R._invperm.resize(l);
			for (int i = 0;i < l;i++) { R._perm[i] = _perm[P2._perm[i]]; R._invperm[i] = P2._invperm[_invperm[i]]; }
			return R;
			}


		/**
		* serialise the permutation
		**/
		template<typename U> void serialize(U & Archive, const int version = 0) const 
			{
			Archive & _perm;
			}


		/**
		* deserialise the permutation
		**/
		template<typename U> void deserialize(U & Archive, const int version = 0)
			{
			Archive & _perm;
			_recomputeInvert();
			}


		/**
		* Print the transformation into a string.
		**/
		std::string toString(bool details = false) const
			{
			const size_t l = _perm.size();
			if (l == 0) return "Permutation[empty]";
			std::string s("Permutation[0,"); s += mtools::toString(l - 1) + "]";
			if (details)
				{
				s += "\n";
				for (int i = 0;i < l;i++) { s += mtools::toString(i) + "\t -> \t" + mtools::toString(_perm[i]) + "\n"; }
				}
			return s;
			}


		private:


		/* re-compute the inverse permutation */
		void _recomputeInvert()
			{
			const size_t l = _perm.size();
			_invperm.resize(l);	
			for (size_t i = 0; i < l; i++) { _invperm[_perm[i]] = (int)i; } 
			}
		
		std::vector<int> _perm;
		std::vector<int> _invperm;
		};



	}

/* end of file */

