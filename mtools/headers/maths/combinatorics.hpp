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

	/** Defines an alias representing a permutation of {0,1,...,n} */
	typedef std::vector<int> Permutation;


	/**
	* Return the permutation associated with the ordering of labels in non-decreasing order.
	* The label themselves are NOT reordered.
	*
	* @tparam	LABELS	Type of the object to reorder, typically std::vector<int>.
	* 					- Must be accessible via operator[].
	* 					- elements must be comparable with operator<() to allow sorting.
	*
	* @param	labels	The labels used to compute the re-ordering.
	*
	* @return	the reordering permutation. perm[i] = k means that the label initially at
	* 			position k is now at position i after reordering.
	*           Call method permute(labels,perm) to effectively sort the labels.
	**/
	template<typename LABELS> Permutation getSortPermutation(const LABELS & labels)
		{
		Permutation  res;
		const int l = (int)labels.size();
		if (l == 0) return res;
		res.resize(l);
		for (int i = 0; i < l; i++) { res[i] = i; }
		sort(res.begin(), res.end(), [&](const int & x, const int & y) { return labels[x] < labels[y]; });
		return res;
		}


	/**
	* Compute the inverse of a permutation.
	*
	* @param	perm	the permutation. Must be bijection of {0,...,perm.size()-1}.
	*
	* @return	the inverse permutation such that return[perm[k]] = k for all k.
	**/
	inline Permutation invertPermutation(const Permutation & perm)
		{
		Permutation invperm;
		const size_t l = perm.size();
		if (l > 0)
			{
			invperm.resize(l);
			for (size_t i = 0; i < l; i++) { invperm[perm[i]] = (int)i; }
			}
		return invperm;
		}


	/**
	* Re-order the labels according to the permutation perm.
	* (obtained for example from getSortPermutation() )
	*
	* @tparam	VECTOR	Type of the object to reorder, typically std::vector<T>.
	* 					- Must be accessible via operator[].
	*
	* @param	labels	The labels to reorder.
	* @param	perm	the permutation, perm[i] = k means that label at position k must be put at pos i.
	**/
	template<typename VECTOR> VECTOR permute(const VECTOR & labels, const Permutation & perm)
		{
		const size_t l = labels.size();
		MTOOLS_INSURE(perm.size() == l);
		VECTOR res;
		if (l == 0) return res;
		res.resize(l);
		for (size_t i = 0; i < l; i++)
			{
			res[i] = labels[perm[i]];
			}
		return res;
		}


	/**
	* Reorder the vertices of a graph according to a permutation.
	*
	* @tparam	GRAPH   	Type of the graph, typically std::vector< std::list<int> >.
	* 						- The outside container must be accessible via operator[].
	* 						- The inside container must accept be iterable and contain
	*                         elements convertible to size_t (corresponding to the indexes
	*                          the neighour vertices).
	* @param	graph	  	The graph to reorder.
	* @param	perm    	The permutation to apply: perm[i] = k means that the vertex with index k
	*                       must now become the vertex at index i in the new graph.
	* @param	invperm    	The inverse permutation of perm. (use the other permuteGraph() method if
	*						not previously computed).
	*
	* @return  the permuted graph.
	**/
	template<typename GRAPH> GRAPH permuteGraph(const GRAPH & graph, const Permutation  & perm, const Permutation & invperm)
		{
		const size_t l = graph.size();
		MTOOLS_INSURE(perm.size() == l);
		if (l == 0) return GRAPH();
		GRAPH res = permute<GRAPH>(graph, perm);	// permute the order of the vertices. 
		for (size_t i = 0; i < l; i++)
			{
			for (auto it = res[i].begin(); it != res[i].end(); it++)
				{
				(*it) = invperm[*it];
				}
			}
		return res;
		}


	/**
	* Reorder the vertices of a graph according to a permutation.
	* Same as above but also compute the inverse permutation.
	*/
	template<typename GRAPH> GRAPH permuteGraph(const GRAPH & graph, const Permutation & perm)
		{
		return(permuteGraph(graph, perm, invertPermutation(perm)));
		}


	/**
	* Convert a graph from type A to type B.
	*
	**/
	template<typename GRAPH_A, typename GRAPH_B> GRAPH_B convertGraph(const GRAPH_A & graph)
		{
		GRAPH_B res;
		const size_t l = graph.size();
		if (l == 0) return res;
		res.resize(l);
		for (size_t i = 0; i < l; i++)
			{
			auto & lv1 = graph[i];
			auto & lv2 = res[i];
			for (auto it = lv1.begin(); it != lv1.end(); ++it) { lv2.push_back(*it); }
			}
		return res;
		}



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



	}

/* end of file */

