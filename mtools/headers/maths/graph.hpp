/** @file graph.hpp */
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
#include "../random/classiclaws.hpp"
#include "permutation.hpp"

namespace mtools
	{

	/** Defines aliases representing a generic graph. **/
	typedef std::vector<std::vector<int> >	Graph1;
	typedef std::vector<std::deque<int> >	Graph2;
	typedef std::vector<std::list<int> >	Graph3;

	typedef Graph1 Graph; // default choice. 
	


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
	 * Compute the distance from a given vertex in the graph.
	 *
	 * @param	gr				   	The graph.
	 * @param	rootVertex		   	index of the vertex to compute the distance to.
	 * @param [in,out]	maxdistance	used to indicate the max distance found.
	 * @param [in,out]	connected  	used to indicate if the graph is connected.
	 *
	 * @return	a vector containing the distance for each vertex of the graph (distance is -1 if not
	 * 			in the same connected component). Flag connected is set to true is gr is connected and
	 * 			false otherwise. maxdistance is filled with the max distance found. 
	 **/
	template<typename GRAPH> std::vector<int> computeDistances(const GRAPH & gr, int rootVertex, int & maxdistance, bool & connected)
		{
		const size_t l = gr.size();
		std::vector<int> dist(l, -1);
		std::vector<int> tempv1; tempv1.reserve(l);
		std::vector<int> tempv2; tempv2.reserve(l);
		std::vector<int> * pv1 = &tempv1;
		std::vector<int> * pv2 = &tempv2;
		dist[rootVertex] = 0;
		pv1->push_back(rootVertex);
		size_t visited = 1;
		int d = 1;
		int vd = 0;
		while(pv1->size() != 0)
			{
			pv2->clear();
			const size_t l = pv1->size();
			for (int i = 0; i < l; i++)
				{
				const int k = pv1->operator[](i);
				for (auto it = gr[k].begin(); it != gr[k].end(); ++it)
					{
					const int n = (*it);
					if (dist[n] < 0) { pv2->push_back(n); dist[n] = d; visited++; vd = d; }
					}
				}
			d++;
			if (pv1 == &tempv1) { pv1 = &tempv2; pv2 = &tempv1; } else { pv1 = &tempv1; pv2 = &tempv2; }
			}
		maxdistance = vd;
		connected = (visited == l);
		return dist;
		}


	/**
	/* As above but without indication on the connectness and max distance.
	**/
	template<typename GRAPH> std::vector<int> computeDistances(const GRAPH & gr, int rootVertex)
		{
		bool b;
		int d;
		return computeDistances(gr, rootVertex,d,b);
		}


	/**
	 * Query if the graph is connected from vertex x (ie any position may be reached starting from
	 * rootvertex following the oriented edges).
	 **/
	template<typename GRAPH> bool isConnected(const GRAPH & gr, int rootVertex = 0)
		{
		bool b;
		int d;
		computeDistances(gr, rootVertex, d, b);
		return b;
		}


	/**
	* Query if a graph has loops 
	* ie if there exist an edge u->u
	**/
	template<typename GRAPH> bool hasLoop(const GRAPH & gr)
		{
		size_t l = gr.size();
		for (size_t i = 0; i < l; i++)
			{
			for (auto it = gr[i].begin(); it != gr[i].end(); ++it)
				{
				if ((*it) == i) return true;
				}
			}
		return false;
		}


	/**
	* Query if a graph has double edges
	* ie if there exist two edges u->v for the same u,v. 
	**/
	template<typename GRAPH> bool hasDoubleEdges(const GRAPH & gr);


	/**
	* Query if gr is unoriented
	* ie for any u,v, there is the same number of edges u->v and v->u
	**/
	template<typename GRAPH> bool isUnoriented(const GRAPH & gr);


	/**
	 * Troncate a graph, keeping only the sub-graph consisting of all the verticess a distance at
	 * most radius from centerVertex (and the edges joining these vertices).
	 *
	 * @param	gr				The graph.
	 * @param	centerVertex	The center vertex.
	 * @param	radius			The radius of the (closed) ball to keep
	 * @param	dist			(optional) The distance vector of all points to centerVertex. 
	 *
	 * @return	- first member:  the troncated graph.
	 * 			- second member: the number N of boundary vertices (is adjacent to a vertex removed). they are
	 * 			                 indexed between [0,N-1] in the new graph. 
	 * 			- third member:  the permutation that describe how the vertices are mapped to the new graph. 
	 * 			                 perm[i] is the new indice of the vertex that was initially at i.
	 * 			                 If (perm[i] >= newgraph.size()), this means the vertex i was removed.
	 **/
	template<typename GRAPH> std::tuple<GRAPH, int, Permutation> keepBall(const GRAPH & gr, int centerVertex, int radius, const std::vector<int> & dist);


	/**
	 * same as above but does not need to pass the distance vector.  
	 **/
	template<typename GRAPH> std::tuple<GRAPH, int, Permutation> keepBall(const GRAPH & gr, int centerVertex, int radius)
		{
		return keepBall(gr, centerVertex, radius, computeDistances(gr, centerVertex));
		}


	/**
	* Troncate a graph, removing all the verticess a distance less or equal to radius 
	* from centerVertex (and the edges joining these vertices).
	*
	* @param	gr				The graph.
	* @param	centerVertex	The center vertex.
	* @param	radius			The radius of the (closed) ball around vertexCenter to to removed
	* @param	dist			(optional) The distance vector of all points to centerVertex.
	*
	* @return	- first member:  the troncated graph.
	* 			- second member: the number N of boundary vertices (is adjacent to a vertex removed). they are
	* 			                 indexed between [0,N-1] in the new graph.
	* 			- third member:  the permutation that describe how the vertices are mapped to the new graph.
	* 			                 perm[i] is the new indice of the vertex that was initially at i.
	* 			                 If (perm[i] >= newgraph.size()), this means the vertex i was removed.
	**/
	template<typename GRAPH> std::tuple<GRAPH, int, Permutation> removeBall(const GRAPH & gr, int centerVertex, int radius, const std::vector<int> & dist);



	/**
	* same as above but does not need to pass the distance vector.
	**/
	template<typename GRAPH> std::tuple<GRAPH, int, Permutation> removeBall(const GRAPH & gr, int centerVertex, int radius)
		{
		return removeBall(gr, centerVertex, radius, computeDistances(gr, centerVertex));
		}



	/***
	Info on a graph
	- unoriented ? 
	- loop ?
	- double edges ?
	- conected ?
	- planar ? 
	- number of vertices
	- number of oriented/non-oriented edges ?
	- number of faces ?
	- is a tree ? 
	- is degree regular
	- max degree of vertices
	- diameter
	*/
	/**
	* TODO
	* - compute distance
	* - is connected
	* - is tree
	* - number of faces.
	* - is symmetric (unoriented)
	* - troncate.

	**/

	}



/* end of file */

