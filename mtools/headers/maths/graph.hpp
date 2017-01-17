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
#include "combinatorialmap.hpp"

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
	* Explore the graph starting from a root vertex and following the oriented edges.
	* Accept lambda functions.
	*
	* See method computeDistances() below for an example of how to use it. 
	*
	* @param	gr	  	The graph.
	* @param	origin	The vertex to start exploration from.
	* @param	fun   	The function to call at each visited vertex. Of the form:
	* 					bool fun(int vert, int dist)
	* 					  - vert  : the vertice currently visited
	* 					  - dist  : the distance of vert from the stating position
	* 					  - return: true to explore its neighobur and false to stop.
	*
	* @return	the total number of vertices visited.
	**/
	template<typename GRAPH> int exploreGraph(const GRAPH & gr, int origin, std::function<bool(int, int)> fun)
		{
		const size_t l = gr.size();
		std::vector<char> vis(l, 0);
		std::vector<int> tempv1; tempv1.reserve(l);
		std::vector<int> tempv2; tempv2.reserve(l);
		std::vector<int> * pv1 = &tempv1;
		std::vector<int> * pv2 = &tempv2;
		vis[origin] = 1;
		pv1->push_back(origin);
		int sum = 1;
		int d = 0;
		while (pv1->size() != 0)
			{
			pv2->clear();
			const size_t l = pv1->size();
			for (int i = 0; i < l; i++)
				{
				const int k = pv1->operator[](i);
				if (fun(k, d))
					{
					for (auto it = gr[k].begin(); it != gr[k].end(); ++it)
						{
						const int n = (*it);
						if (vis[n] == 0) { vis[n] = 1;  pv2->push_back(n); sum++; }
						}
					}
				}
			d++;
			if (pv1 == &tempv1) 
				{ 
				pv1 = &tempv2; 
				pv2 = &tempv1; 
				} 
			else 
				{ 
				pv1 = &tempv1; 
				pv2 = &tempv2; 
				}
			}
		return sum;
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
		int md = 0;
		int nbvis = exploreGraph(gr, rootVertex, [&](int vert, int d) -> bool { dist[vert] = d; if (d > md) { md = d; } return true; });
		maxdistance = md;
		connected = (nbvis == l);
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
	 * Return the type of the graph.
	 *
	 * @param	gr	The graph.
	 *
	 * @return	Return a tuple with the following meaning
	 * 			- <0>:  valid					true if all edges index are within [0,graph.size()[.
	 * 			- <1>:  oriented   			true if the graph is oriented (edge set is not symmetric). 
	 * 			- <2>:  isolated outgoing		true if the graph has vertices without outgoing edge
	 * 			- <3>:  isolated ingoing		true if the graph has vertices without ingoing edge
	 * 			- <4>:  isolated both			true if the graph has vertices without outgoing nor ingoing edge
	 * 			- <5>:  has loop.				true if the graph has loops.
	 * 			- <6>:  has double edge.		true is the graph has double edges  
	 * 			- <7>:  number of oriented edges in the graph.
	 * 			- <8>:  number of vertices in the graph.  
	 * 			- <9>:  maximum outgoing degre of the vertices (not counting loops)
	 * 			- <10>: maximum ingoing degre of the vertices  (not counting loops)
	 * 			- <11>: minimum outgoing degre of the vertices (not counting loops)
	 * 			- <12>: minimum ingoing degre of the vertices  (not counting loops)
	 * 			- <13>: = d if the graph is non oriented and all the vertices have degree d except one with degree n. -1 otherwise
	 * 			- <14>: = n if the graph is non oriented and all the vertices have degree d except one with degree n. -1 otherwise
	 **/
	template<typename GRAPH> std::tuple<bool,bool,bool, bool, bool, bool,bool, int,int,int,int, int, int, int, int> graphType(const GRAPH & gr)
		{
		const int nbv = (int)gr.size();	// number of vertices
		int nbe = 0;				// number of edges
		std::map<std::pair<int, int>, std::pair<int, int> >  mapedge;	// store edges count to check for symmetry
		std::vector<int> invec; invec.resize(nbv);			// in degre
		std::vector<int> outvec; outvec.resize(nbv);		// out degree
		bool isValid = true;			// graph is valid ?
		bool hasLoop = false;			// graph has loops ?
		for (int i = 0; i < nbv; i++)
			{ // iterate over the vertices
			const int ne = (int)gr[i].size();
			nbe += ne;
			for (auto it = gr[i].begin(); it != gr[i].end(); ++it)
				{
				const int j = *it;
				if ((j < 0) || (j >= nbv)) { isValid = false; }
				else
					{
					if (i == j) { hasLoop = true; }
					else
						{
						invec[j]++; 
						outvec[i]++;
						if (i < j) { mapedge[{i, j}].first++; } else { mapedge[{j, i}].second++; }
						}					
					}
				}
			}
		bool hasIsolatedOut  = false;
		bool hasIsolatedIn   = false;
		bool hasIsolatedBoth = false;
		for (size_t i = 0; i < nbv; i++)
			{
			if (invec[i] == 0) { hasIsolatedIn = true; }
			if (outvec[i] == 0) { hasIsolatedOut = true;  if (invec[i] == 0) { hasIsolatedBoth = true; } }
			}			
		bool isOriented = false;
		bool hasDoubleEdge = false;
		for (auto it = mapedge.begin(); it != mapedge.end(); ++it)
			{
			auto & value = it->second;
			if (value.first != value.second) { isOriented = true; }
			if ((value.first > 1)|| (value.second > 1)) { hasDoubleEdge = true; }
			}
		std::sort(invec.begin(), invec.end());
		std::sort(outvec.begin(), outvec.end());
		int maxoud = -1;
		int maxind = -1;
		int minoud = -1;
		int minind = -1;
		int other = -1;
		int alone = -1;
		if (invec.size() > 0)  { minind = invec.front(); maxind = invec.back(); }
		if (outvec.size() > 0) { minoud = outvec.front(); maxoud = outvec.back(); }
		if ((isOriented == false)&&(invec.size() >= 2))
			{
			if (invec[1] == invec.back()) { alone = invec.front(); other = invec.back(); }
			if (invec[invec.size()-2] == invec.front()) { alone = invec.back(); other = invec.front(); }
			}
		MTOOLS_ASSERT(isOriented || ((maxoud == maxind) && (nbe % 2 == 0)));
		return std::make_tuple( isValid,		// 0
								isOriented,		// 1
								hasIsolatedOut,	// 2
								hasIsolatedIn,	// 3
								hasIsolatedBoth,// 4
								hasLoop,		// 5
								hasDoubleEdge,	// 6
								nbe,			// 7
								nbv,			// 8 
								maxoud,			// 9
								maxind,			// 10
								minoud,			// 11
								minind,			// 12
								other,          // 13
								alone			// 14
								);
		}


	/** Convenience method for graphType() **/
	inline bool graphType_isValid(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<0>(typ); }

	/**  Convenience method for graphType() **/
	inline bool graphType_isOriented(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<1>(typ); }

	/**  Convenience method for graphType() **/
	inline bool graphType_hasIsolatedOut(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<2>(typ); }

	/**  Convenience method for graphType() **/
	inline bool graphType_hasIsolatedIn(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<3>(typ); }

	/**  Convenience method for graphType() **/
	inline bool graphType_hasIsolatedBoth(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<4>(typ); }

	/**  Convenience method for graphType() **/
	inline bool graphType_hasLoop(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<5>(typ); }

	/**  Convenience method for graphType() **/
	inline bool graphType_hasDoubleEdge(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<6>(typ); }

	/**  Convenience method for graphType() **/
	inline int graphType_nbEdges(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<7>(typ); }

	/**  Convenience method for graphType() **/
	inline int graphType_nbVertices(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<8>(typ); }

	/**  Convenience method for graphType() **/
	inline int graphType_maxOutDegree(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<9>(typ); }

	/**  Convenience method for graphType() **/
	inline int graphType_maxInDegree(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<10>(typ); }

	/**  Convenience method for graphType() **/
	inline int graphType_minOutDegree(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<11>(typ); }

	/**  Convenience method for graphType() **/
	inline int graphType_minInDegree(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::get<12>(typ); }

	/**  Convenience method for graphType() **/
	inline std::pair<int, int> graphType_biType(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return std::pair<int,int>(std::get<13>(typ), std::get<14>(typ)); }

	/**
	* Convenience method for graphType(). 
	* Return true if the graph is simple i.e. if it is unoriented, without loop and double edge. 
	**/
	inline bool graphType_isSimple(std::tuple<bool, bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int> typ) { return ((graphType_isValid(typ))&&(!graphType_isOriented(typ)) && (!graphType_hasLoop(typ)) && (!graphType_hasDoubleEdge(typ))); }


	/**
	* Print information about the graph into a string
	**/
	template<typename GRAPH> std::string graphInfo(const GRAPH & gr)
		{
		std::string s;
		auto res = graphType(gr);
		if (gr.size() == 0) { s += std::string("EMPTY GRAPH\n"); return s; }
		if (!graphType_isValid(res)) { s += std::string("!!! INVALID GRAPH !!!!\n"); return s; }
		if (graphType_isOriented(res))
			{
			// oriented graph
			s += std::string("ORIENTED GRAPH\n");
			if (graphType_hasLoop(res)) { s += std::string("    -> WITH LOOPS\n"); }
			else { s += std::string("    -> no loop.\n"); }
			if (graphType_hasDoubleEdge(res)) { s += std::string("    -> WITH DOUBLE EDGES\n"); }
			else { s += std::string("    -> no double edge.\n"); }
			s += std::string(" - Vertices         : ") + toString(graphType_nbVertices(res)) + "\n";
			s += std::string(" - Oriented edges   : ") + toString(graphType_nbEdges(res));
			s += std::string(" - out degree range : [") + toString(graphType_minOutDegree(res)) + "," + toString(graphType_maxOutDegree(res)) + "]\n";
			s += std::string(" - in  degree range : [") + toString(graphType_minInDegree(res)) + "," + toString(graphType_maxInDegree(res)) + "]\n";
			s += std::string(" - Isolated vertice out   : ") + toString(graphType_hasIsolatedOut(res)) + "\n";
			s += std::string(" - Isolated vertices in   : ") + toString(graphType_hasIsolatedOut(res)) + "\n";
			s += std::string(" - Isolated vertices both : ") + toString(graphType_hasIsolatedOut(res)) + "\n";
			return s;
			}
		// non-oriented graph
		if (!graphType_isSimple(res))
			{ // no simple
			s += std::string("UNORIENTED GRAPH\n");
			if (graphType_hasLoop(res)) { s += std::string("    -> WITH LOOPS\n"); }
			else { s += std::string("    -> no loop.\n"); }
			if (graphType_hasDoubleEdge(res)) { s += std::string("    -> WITH DOUBLE EDGES\n"); }
			else { s += std::string("    -> no double edge.\n"); }

			s += std::string("Vertices     : ") + toString(graphType_nbVertices(res)) + "\n";
			s += std::string("Edges        : ") + toString(graphType_nbEdges(res) / 2) + "\n";
			s += std::string("degree range : [") + toString(graphType_minInDegree(res)) + "," + toString(graphType_maxInDegree(res)) + "]\n";
			bool connected;
			int maxd;
			computeDistances(gr, 0, maxd, connected);
			if (connected)
				{
				s += std::string("Graph is CONNECTED. Estimated diameter [") + toString(maxd) + "," + toString(2 * maxd) + "]\n";
				}
			else
				{
				s += std::string("Graph is NOT connected.\n");
				s += std::string("   - isolated vertices    : ") + toString(graphType_hasIsolatedBoth(res)) + "\n";
				}
			}
		// simple non oriented graph
		s += std::string("SIMPLE NON-ORIENTED GRAPH (no loop nor double edge)\n");
		CombinatorialMap cm(gr);
		auto gr2 = cm.getDual().toGraph();
		auto res2 = graphType(cm.getDual().toGraph());
		s += std::string("   Edges        : ") + mtools::toString(cm.nbEdges()) + "\n";
		s += std::string("   Faces        : ") + mtools::toString(cm.nbFaces()); if (cm.isTree()) {s += std::string(" (TREE)");}  s += "\n";
		s += std::string("     |-> degree : [") + toString(graphType_minInDegree(res2)) + "," + toString(graphType_maxInDegree(res2)) + "]\n";
		s += std::string("   Vertices     : ") + mtools::toString(cm.nbVertices()) + "\n";
		s += std::string("     |-> degree : [") + toString(graphType_minInDegree(res)) + "," + toString(graphType_maxInDegree(res)) + "]\n";
		bool connected;
		int maxd;
		computeDistances(gr, 0, maxd, connected);
		if (!connected)
			{
			s += std::string("Graph is NOT connected.\n");
			s += std::string("   - isolated vertices    : ") + toString(graphType_hasIsolatedBoth(res)) + "\n";
			return s;
			}		
		s += std::string("Connected graph. Estimated diameter [") + toString(maxd) + "," + toString(2 * maxd) + "]\n";
		s += std::string("Genus : ") + mtools::toString(cm.genus());   if (cm.isPlanar()) s += std::string(" -> PLANAR GRAPH\n");
		int other, alone; 
		std::tie(other, alone) = graphType_biType(res);
		int other2, alone2;
		std::tie(other2, alone2) = graphType_biType(res2);
		if (other > 0)
			{
			if (other == alone) { s += std::string("REGULAR GRAPH: every site has degree ") + toString(other) + "\n"; }
			else { s += std::string("ALMOST REGULAR GRAPH: every site has degree ") + toString(other) + " except one with degree " + toString(alone) + "\n"; }
			}
		if (other2 > 0)
			{
			if (other2 == alone2) { s += std::string("ANGULATION: every face has degree ") + toString(other2) + "\n"; }
			else { s += std::string("ANGULATION WITH BOUNDARY: every face has degree ") + toString(other2) + " except one with degree " + toString(alone2) + "\n"; }
			}
		return s;
		}



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





	/**
	additionnal info on the graph

	- planar ? number of faces ?
	- is a tree ? 
	- is degree regular
	- max degree of vertices, min degree
	- diameter

	**/

	}



/* end of file */

