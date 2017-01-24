/** @file combinatorialmap.hpp */
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
#include "dyckword.hpp"

namespace mtools
	{



	/* forward declaration from graph.hpp */
	template<typename GRAPH>  bool isGraphSimple(const GRAPH & gr);
	template<typename GRAPH>  bool isGraphEmpty(const GRAPH & gr);


	/**
	* Class that encode a rooted combinatorial map.
	* i.e. an unoriented graph together with a rotation system.
	* 
	* The graph which has n edges and is encoded with two permutations
	* of size 2n that represents the mapping for half edges (darts)
	* 
	*  - alpha : Involution that matches half-edges together. 
	*  - sigma : Rotation around a vertex. sigma[i] point to next   
	*            half-edge around the vertex when rotating in the 
	*            positive orientation.
	*            
	* Also define phi = sigma(alpha(.)) the rotation around a face in          
	* positive orientation thus (alpha,phi) is the combinatorial map 
	* associated with the dual graph. 
	*            
	* The map is rooted at a given half-edge.
	**/
	class CombinatorialMap
		{

		public:

			/** Default constructor. create a map with a single edge */
			CombinatorialMap()
				{
				_root = 0;
				_alpha.resize(2);
				_sigma.resize(2);
				_alpha[0] = 1; _alpha[1] = 0;
				_sigma[0] = 0; _sigma[1] = 1;
				_computeVerticeSet();
				_computeFaceSet();
				}


			/**
			* Construct a rooted planar tree from a Dick word.
			* The total number of edges of the tree is dw.nbedges().
			*
			* if weigth = 1, it is the classic rooted tree encoded with the Dyck word. 
			* if weight > 1, creates a tree where each non leaf vertex has exactly weight
			*                neighbour leafs and there is dw.ups() interior edges (ie 
			*                dw.ups() + 1 non leaf vertices)
			*                 
			* the root edge is set to 0 and always a leaf edge (ie such that sigma(0) = 0).
			* the edges 0,1,2,3 are ordered according to the exploration of the face ie
			* such that phi(i) = i+1. The numbering of the vertices also start from the
			* root vertex (ie vertice(0) =0) and follow the contour of the tree.
			**/
			CombinatorialMap(const DyckWord & dw)
				{
				fromDyckWord(dw);
				}


			/**
			* Construct the object from a graph. The graph must be non-oriented or the method may crash.
			* The numbering of the vertices is preserved.
			*
			* @tparam	GRAPH	Type of the graph (eg std::vector<std::vector<int>> or similar)
			* @param	gr		  	the graph.
			* @param	root	  	The oriented root edge. If it does not belong to the graph, the dart edge
			* 						is set to 0.
   		    **/
			template<typename GRAPH> CombinatorialMap(const GRAPH & gr, std::pair<int, int> root = std::pair<int, int>(-1, -1))
				{
				fromGraph(gr,root);
				}


			/**
			 * Equality operator. Return true is the object are exactly the same.
			 * The map must have the same root, the same ordering of edges and the
			 * same ordering of vertice and faces to compare equal. 
			 **/
			bool operator==(const CombinatorialMap & cm)
				{
				return ((_root == cm._root) && (_sigma == cm._sigma) && (_alpha == cm._alpha)&&(_vertices == cm._vertices)&&(_faces == cm._faces));
				}


			/**
			* Number of non-oriented edges of the graph
			**/
			inline int nbEdges() const { return ((int)_alpha.size())/2; }


			/**
			* Number of half-edges (ie darts) of the graph
			* This is equal to twice nbEdges().
			**/
			inline int nbHalfEdges() const { return ((int)_alpha.size()); }


			/**
			 * Return the index of the root dart (i.e. oriented edge).
			 **/
			inline int root() const { return _root; }


			/**
			* Permutation alpha: involution that matches the darts
			* together.
			**/
			inline const int & alpha(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges()));
				return _alpha[i]; 
				}


			/**
			* Permutation sigma. Give the next dart when rotating 
			* around a vertex in the positive orientation.
			**/
			inline const int & sigma(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges()));
				return _sigma[i]; 
				}


			/**
			* Permutation phi. Same as sigma(alpha(.)).
			* Rotates around a face (or equivalently around a vertex of 
			* the dual graph).
			**/
			inline const int & phi(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges()));
				return _sigma[_alpha[i]]; 
				}


			/**
			* Number of vertices of the graph. 
			**/
			int nbVertices() const { return _nbvertices; }


			/**
			* Return the index of the vertex which is the start point
			* of dartIndex.
			**/
			int vertice(int dartIndex) const
				{
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbHalfEdges()));
				return _vertices[dartIndex];
				}


			/**
			* Compute the degree of the vertex that is the start point of a given dart.
			*
			* @param	dartIndex	the dart index whose start vertex's degree is to be computed.
			* 						(note that this is NOT the index of the vertex itself!).
			*
			* @return	the degree.
			**/
			int vertexDegree(int dartIndex) const 
				{
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbHalfEdges()));
				int n = 1, j= sigma(dartIndex);
				while (j != dartIndex) { j = sigma(j); n++; }
				return n;
				}


			/**
			* Return a vector of size nbHalfEdges() that describe the 
			* start vertex of each dart.
			**/
			std::vector<int> getVerticeVector() const { return _vertices; }


			/**
			* Number of faces of the graph.
			**/
			int nbFaces() const { return _nbfaces; }


			/**
			* Return the index of the face to which the dartIndex belongs.
			**/
			int face(int dartIndex) const
				{
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbHalfEdges()));
				return _faces[dartIndex];
				}


			/**
			* Compute the number of edge that compose the face to which a given dart belongs.
			*
			* @param	dartIndex	the dart index whose associated face size is to be computed.
			* 						(note that this is NOT the index of the face itself!).
			*
			* @return	the number of edges in the face.
			**/
			int faceSize(int dartIndex)
				{
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbHalfEdges()));
				int n = 1, j = phi(dartIndex);
				while (j != dartIndex) { j = phi(j); n++; }
				return n;
				}


			/**
			 * Return a vector of size nbHalfEdges() that describe the face
			 * for each dart.
			 **/
			std::vector<int> getFaceVector() const { return _faces; }


			/**
			 * Query the genus of the combinatorial map (euler characteristic).
			 * The return value is zero i.i.f. this combinatorial map corresponds
			 * to a planar embedding of the graph.
			 * 
			 * The genus is given by: V - E + F = 2 - 2g
			 **/
			inline int genus() const
				{
				int khi = _nbvertices - nbEdges() + _nbfaces;
				MTOOLS_ASSERT((khi % 2) == 0);
				return((2 - khi)/2);
				}


			/**
			* Query if the graph is connected tree.
			*
			* @return	true if it is a connected tree, false if not.
			**/
			inline bool isTree() const { return (nbFaces() == 1); }


			/**
			* Query if the combinatorial map define a planar embedding 
			* of the underlying graph. This is equivalent to checking 
			* if the genus of the map is zero.
			*
			* @return	true if the embeding is planar, false if not.
			**/
			inline bool isPlanar() const { return (genus() == 0); }


			/**
			* Construct the dual combinatorial map
			* It is obtained by exchanging the role of sigma and phi.
			* Defines an involution.
			*
			* @return	The dual graph
			**/
			CombinatorialMap getDual() const
				{
				CombinatorialMap cm;
				cm._alpha = _alpha;	// same alpha
				const size_t l = nbHalfEdges(); 
				cm._sigma.resize(l);
				for (int i = 0; i < l; i++) { cm._sigma[i] = phi(i); } // invert phi becomes sigma
				cm._vertices = _faces;
				cm._nbvertices = _nbfaces;
				cm._faces = _vertices;
				cm._nbfaces = _nbvertices;
				cm._root = _root;
				return cm;
				}


			/**
			* Construct a rooted planar tree from a Dick word.
			* The total number of edges of the tree is dw.nbedges().
			*
			* if weigth = 1, it is the classic rooted tree encoded with the Dyck word.
			* if weight > 1, creates a tree where each non leaf vertex has exactly weight
			*                neighbour leafs and there is dw.ups() interior edges (ie
			*                dw.ups() + 1 non leaf vertices)
			*
			* the root edge is set to 0 and always a leaf edge (ie such that sigma(0) = 0).
			* the edges 0,1,2,3 are ordered according to the exploration of the face ie
			* such that phi(i) = i+1. The numbering of the vertices also start from the
			* root vertex (ie vertice(0) =0) and follow the contour of the tree.
			**/
			void fromDyckWord(const DyckWord & dw)
				{
				const int n = dw.nbedges();
				MTOOLS_ASSERT(n > 0);           // tree must have at least 1 edges
				_sigma.reserve(2 * (n + 1));	// make it faster to add an edge later on. 
				_alpha.reserve(2 * (n + 1));	// useful when performing Poulalhon-Schaeffer bijection
				_sigma.resize(2 * n);
				_alpha.resize(2 * n);
				const int nbuds = dw.weight() - 1;
				_root = 0; // rooted at the first edge
				std::vector<int> st;
				st.reserve((int)(sqrt(dw.nups())) + 1);
				// match the half-edges. 
				if (nbuds == 0)
					{ // weight = 1, general tree
					for (int i = 0; i < 2 * n; i++)
						{
						if (dw[i] == 1) { st.push_back(i); }
						else
							{
							_alpha[st.back()] = i;
							_alpha[i] = st.back();
							st.pop_back();
							}
						}
					}
				else
					{
					int h = 0;
					std::vector<int> buds_passed;
					buds_passed.reserve((int)(sqrt(dw.nups())) + 1);
					int j = 1;
					_alpha[0] = 2 * n - 1;
					_alpha[2 * n - 1] = 0;
					buds_passed.push_back(1); // passed one bud at height 0
					for (int i = 0; i < dw.length() - 1; i++)
						{
						if (dw[i] == 1)
							{
							st.push_back(j);
							buds_passed.push_back(0);
							h++;
							j++;
							}
						else
							{
							if (buds_passed[h] == nbuds)
								{
								h--;
								_alpha[st.back()] = j;
								_alpha[j] = st.back();
								st.pop_back();
								buds_passed.pop_back();
								j++;
								}
							else
								{
								++(buds_passed[h]);
								_alpha[j] = j + 1;
								_alpha[j + 1] = j;
								j += 2;
								}
							}
						}
					MTOOLS_ASSERT(h == 0);
					MTOOLS_ASSERT(buds_passed[0] == nbuds);
					}
				MTOOLS_ASSERT(st.size() == 0);
				// construct sigma by going around the exterior face
				for (int i = 0; i < (2 * n); i++) { _sigma[i] = (_alpha[i] + 1) % (2 * n); }
				_computeVerticeSet();
				_computeFaceSet();
				}


			/**
			 * Construct the object from a graph.  
			 * The graph must be simple i.e.
			 *    - non-oriented  
			 *    - without double edges  
			 *    - without loops
			 *    - without isolated vertices
			 * 
			 * The numbering of the vertices is preserved.
			 *
			 * @tparam	GRAPH	Type of the graph (eg std::vector<std::vector<int>> or similar)
			 * @param	gr		  	the graph.
			 * @param	root	  	The oriented root edge. If it does not belong to the graph, the dart edge
			 * 						is set to 0.
			 *
			 * @return	a map that give the index of each oriented edge in the permutations ie map[{u,v}] = i
			 * 			means that the oriented edge from u to v in the graph correspond to the arrow index i
			 * 			in the combinatorial map.
			 **/
			template<typename GRAPH> std::map< std::pair<int, int>, int> fromGraph(const GRAPH & gr, std::pair<int,int> root = std::pair<int, int>(-1,-1))
				{
				MTOOLS_ASSERT(isGraphSimple(gr)); // make sure the graph is simple (unoriented without loop nor double edges). 
				MTOOLS_ASSERT(!isGraphEmpty(gr)); // make sure the graph is not empty
				const int nbv = (int)gr.size();
				int totaldarts = 0;
				for (int i = 0; i < nbv; i++) { totaldarts += (int)gr[i].size(); } // compute the number of darts
				_root = 0;	// default choice for the root if the edge provided does not exist
				_sigma.clear();
				_alpha.clear();
				_sigma.reserve(totaldarts + 1);
				_alpha.reserve(totaldarts + 1);
				_sigma.resize(totaldarts);
				_alpha.resize(totaldarts);
				_vertices.reserve(totaldarts + 1);
				_vertices.resize(totaldarts);
				_nbvertices = nbv;
				std::map< std::pair<int, int>, int> mapEdge;
				int e = 0;
				for (int i = 0; i < nbv; i++)
					{
					int firste = e;
					for (auto it = gr[i].begin(); it != gr[i].end(); ++it)
						{
						_vertices[e] = i;
						if ((root.first == i) && (root.second == (*it))) { _root = e; } // found the root
						if (it != gr[i].begin()) { _sigma[e - 1] = e; } // chain to the previous one
						auto res = mapEdge.find(std::pair<int, int>(*it,i)); // did we already insert the opposite edge ? 
						if (res != mapEdge.end())
							{ 
							_alpha[e] = res->second; // opposite edge already found
							_alpha[res->second] = e; // link them together
							}
						mapEdge[std::pair<int, int>(i, *it)] = e; // insert the edge in the map in any case.
						e++; // next edge
						}
					if (e != firste) { _sigma[e - 1] = firste; } // complete rotation cycle if not empty
					}
				_computeFaceSet();
				return mapEdge;
				}


			/**
			 * Converts the object into a graph.
			 * Inverse operation of fromGraph() in the sense that toGraph(fromGraph(G)) = G
			 * [but fromGraph(toGraph(CM)) may be different from CM].
			 *
			 * @tparam	GRAPH	Type of the graph. For example std::vector<std::vector<int> >.
			 *
			 * @return	The graph object. The numbering of the vertices is unchanged. 
			 * 			
			 **/
			template<typename GRAPH> GRAPH toGraph() const
				{
				const int l = nbHalfEdges();
				GRAPH gr;
				gr.resize(_nbvertices);
				for(int i = 0; i < l; i++)
					{
					int v = _vertices[i];
					if (gr[v].size() == 0)
						{
						gr[v].push_back(_vertices[_alpha[i]]);
						int j = _sigma[i];
						while(j != i)
							{
							gr[v].push_back(_vertices[_alpha[j]]);
							j = _sigma[j];
							}
						}
					}
				return gr;
				}


			/**
			* Default choice for graph is std::vector<std::vector<int> >
			**/
			std::vector<std::vector<int> > toGraph() const
				{
				return toGraph< std::vector<std::vector<int> > >();
				}


			/**
			 * Construct a triangulation by adding a single vertice inside each face of degree larger than 3.
			 * 
			 * - The numbering of the vertices already present is unchanged and the new ones follow.
			 * - The numbering of the faces may change, even for faces that where already triangulations !
			 *
			 * @return	the number of vertices inserted (which is also the number of faces that were not
			 * 			triangles).
			 **/
			int triangulate() 
				{
				const int nbv = nbVertices();
				const int l = nbHalfEdges();
				for (int i = 0; i < l; i++) 
					{ 
					_triangulateFace(i); 
					}
				_computeFaceSet();
				return nbVertices() - nbv;
				}


			/**
			 * Triangulate a face belonging to a given dart. If the face is already a triangle, does
			 * nothing. Otherwise, add a vertice inside the face and then connect every vertice of the face
			 * to it.
			 * 
			 * - The numbering of the vertices already present is unchanged and the new one follow.
			 * - The numbering of the faces may change, even for faces that where already present !
			 *
			 * @param	dartIndex	The index of a dart of the face to triangulate 
			 * 						(note that this is NOT the index of the face itself!).
			 *
			 * @return	The degree of the face that was triangulated.
			 **/
			int triangulateFace(int dartIndex)
				{
				int d = _triangulateFace(dartIndex);
				_computeFaceSet();
				return d;
				}


			/**
			* Return a combinatorial map where the indice have been permutated 
			* in such way that:
			* 
			*   - the darts at position k in the returned object was at position   
			*     perm[k] in the original map.
			*     
			* The permutation does not change the numbering of the vertices.
			* 
			* @param	perm   	The permutation
			* @param	invperm	its inverse (optional)
			**/
			CombinatorialMap getPermute(const Permutation  & perm, const Permutation & invperm) const
				{
				const int l = nbHalfEdges();
				MTOOLS_ASSERT((int)perm.size() == l);
				CombinatorialMap cm;
				cm._alpha.resize(l);
				cm._sigma.resize(l);
				cm._vertices.resize(l);
				cm._faces.resize(l);
				for (int i = 0; i < l; i++)
					{
					cm._sigma[i]    = invperm[_sigma[perm[i]]];
					cm._alpha[i]    = invperm[_alpha[perm[i]]];
					cm._vertices[i] = _vertices[perm[i]];
					cm._faces[i]    = _faces[perm[i]];
					}
				cm._nbvertices = _nbvertices;
				cm._nbfaces = _nbfaces;
				cm._root = invperm[_root];
				return cm;
				}


			/**
			* Same as above but without providing the the inverse of the permutation.
			**/
			CombinatorialMap getPermute(const Permutation & perm)
				{
				return getPermute(perm, invertPermutation(perm));
				}


			/**
			 * Apply Poulalhon & Schaeffer bijection to convert a B tree (ie where all the non-leaf vertices
			 * have exactly 2 leaf neighbours) to a simple triangulation.
			 * 
			 * If the tree has n inner edges (ie n+1 inner vertices), then the resulting triangulation has
			 * n+3 vertices.
			 * 
			 * Algorithm from: Poulalhon & Schaeffer. Optimal Coding and Sampling of Triangulations
			 * Algorithmica (2006) 46: 505. 
			 * Implementation adaptated from Laurent Ménard's code :-)
			 *
			 * @return	the indexes of the 3 oriented half edges (a,b,c) that determine the root face 
			 * 			oriented counterclockise (the oriented root edge is a).
			 **/
			std::tuple<int,int,int> btreeToTriangulation()
				{
				// we need to make sure that the numbering of the edges follow the contour of the tree.
				const int len = nbHalfEdges();
				bool needreorder = false;
				std::vector<int> ord(len, -1);
				int x0 = 0; while (_sigma[x0] != x0) { x0++; MTOOLS_ASSERT(x0 < len); } // find an half edge x0 that is a leaf. 
				if (x0 != 0) { needreorder = true; }
				ord[x0] = 0; // set it a the root
				int x = phi(x0), i = 1;
				while (x != x0) 
					{// iterate on the (unique face) to find the order
					if (x != i) { needreorder = true; }
					ord[x] = i; i++; x = phi(x);
					} 
				MTOOLS_INSURE(i == len); // make sure we are dealing with a tree
				// reorder the darts if needed.
				if (needreorder)
					{
					auto perm = getSortPermutation(ord);
					auto invperm = invertPermutation(perm);
					auto _alpha2 = _alpha;
					auto _sigma2 = _sigma;
					for (int i = 0; i < len; i++)
						{
						_sigma[i] = invperm[_sigma2[perm[i]]];
						_alpha[i] = invperm[_alpha2[perm[i]]];
						}
					}				
				// ok, now have a tree in canonical order, we can apply the algorithm.				
				const int ne = (int)_alpha.size() / 2;	// number of edges
				const int nv = (ne - 2) / 3 + 1;    // number of inner vertices. 
				std::list< std::pair<int, int> > buds; // position of the buds and number of inner edges following them
				for (int i = 0; i < 2 * ne; i++) { if (_sigma[i] == i) { buds.push_back({ i,0 }); } } // find the positions of the buds
				MTOOLS_INSURE(buds.size() == nv * 2);
				auto it = buds.begin();
				for (int i = 0; i < nv * 2 - 1; i++)
					{
					auto pit = it; ++it;
					pit->second = it->first - pit->first - 2;
					}
				auto nit = it; nit++;
				MTOOLS_INSURE(nit == buds.end());
				it->second = (2 * ne) - it->first - 2;
				// partial closure
				it = buds.begin();
				while (it != buds.end())
					{
					const int l = it->second;
					if (l < 2) { ++it; }
					else
						{
						const int a = it->first;
						_sigma[a] = _sigma[_alpha[_sigma[_alpha[_sigma[_alpha[a]]]]]];
						_sigma[_alpha[_sigma[_alpha[_sigma[_alpha[a]]]]]] = a;
						if (it == buds.begin())
							{
							buds.back().second += (l - 1);
							buds.pop_front();
							it = buds.begin();
							}
						else
							{
							auto pit = it; --pit;
							pit->second += (l - 1);
							buds.erase(it);
							it = pit;
							}
						}
					}
				// find the 4 special vertices
				it = buds.begin();
				while (it->second != 0) { ++it; }
				const auto itA = it;
				++it;
				const auto itA2 = it;
				++it;
				while (it->second != 0) { ++it; }
				const auto itB = it;
				++it;  if (it == buds.end()) it = buds.begin();
				const auto itB2 = it;
				// construct a new edge
				_sigma.resize(2 * ne + 2);
				_alpha.resize(2 * ne + 2);
				_alpha[2 * ne] = 2 * ne + 1;
				_alpha[2 * ne + 1] = 2 * ne;
				// close the first half
				_sigma[itA2->first] = 2 * ne;
				_sigma[2 * ne] = itB->first;
				it = itA2;
				while (it != itB)
					{
					auto pit = it;
					it++;
					_sigma[it->first] = pit->first;
					}
				//close the second half
				_sigma[itB2->first] = 2 * ne + 1;
				_sigma[2 * ne + 1] = itA->first;
				it = itB2;
				while (it != itA)
					{
					auto pit = it;
					it++;
					if (it == buds.end()) it = buds.begin();
					_sigma[it->first] = pit->first;
					}
				int A = 2 * ne + 1;
				int B = _sigma[_alpha[A]];
				int C = _sigma[_alpha[B]];
				_root = A;
				_computeVerticeSet();
				_computeFaceSet();
				return std::make_tuple(A,B,C);
				}



			/**
			* Print infos about the map into a string
			* set the detailed to true to print the complete map
			**/
			std::string toString(bool detailed = false) const
				{
				std::string s("CombinatorialMap: (");
				s += mtools::toString(nbHalfEdges()) + " darts)\n";
				s += std::string("   edges    : ") + mtools::toString(nbEdges()) + "\n";
				s += std::string("   vertices : ") + mtools::toString(nbVertices()) + "\n";
				s += std::string("   faces    : ") + mtools::toString(nbFaces());
				if (isTree()) s += std::string(" (TREE)");
				s += "\n";
				s += std::string("   genus    : ") + mtools::toString(genus());
				if (genus() == 0) s += std::string(" (PLANAR EMBEDDING)");
				s += "\n";
				s += std::string("   root pos : ") + mtools::toString(root()) + "\n";
				if (detailed)
					{

					s += std::string("alpha     = [ "); for (int i = 0; i < _alpha.size(); i++) { s += mtools::toString(_alpha[i]) + " "; } s += "]\n";
					s += std::string("sigma     = [ "); for (int i = 0; i < _sigma.size(); i++) { s += mtools::toString(_sigma[i]) + " "; } s += "]\n";
					s += std::string("vertices  = [ "); for (int i = 0; i < _vertices.size(); i++) { s += mtools::toString(_vertices[i]) + " "; } s += "]\n";
					s += std::string("faces     = [ "); for (int i = 0; i < _vertices.size(); i++) { s += mtools::toString(_faces[i]) + " "; } s += "]\n";
					}
				return s;
				}


			/**
			* Serialise/deserialize the object
			**/
			template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0)
				{
				ar & _root;
				ar & _nbvertices;
				ar & _nbfaces;
				ar & _alpha;
				ar & _sigma;
				ar & _vertices;
				ar & _faces;
				}




		private:


			/* swap indexes i1 and i2 without modifiying the graph */
			/*
			void _swapIndexes(int i1, int i2)
				{
				if (i1 == i2) return;	// nothing to do
				// update alpha
				int a1 = _alpha[i1];
				int a2 = _alpha[i2];
				_alpha[i1] = a2;
				_alpha[a2] = i1;
				_alpha[i2] = a1;
				_alpha[a1] = i2;
				// update sigma
				int sr1 = _sigma[i1];
				int sr2 = _sigma[i2];
				int lr1 = i1; while (_sigma[lr1] != i1) { lr1 = _sigma[lr1]; }
				int lr2 = i2; while (_sigma[lr2] != i2) { lr2 = _sigma[lr2]; }
				if (sr1 == lr1) { _sigma[i2] = i2; } else { _sigma[i2] = sr1; _sigma[lr1] = i2; }
				if (sr2 == lr2) { _sigma[i1] = i1; } else { _sigma[i1] = sr2; _sigma[lr2] = i1; }
				// update the face and vertice sets
				int tmpf = _faces[i1]; _faces[i1] = _faces[i2]; _faces[i2] = tmpf;
				int tmpv = _vertices[i1]; _vertices[i1] = _vertices[i2]; _vertices[i2] = tmpf;
				// update the root
				if (_root == i1) { _root = i2; } else { if (_root == i2) { _root = i1; } }
				}
			*/


			/* Compute the vertex set from sigma and alpha */
			void _computeVerticeSet()
				{
				const int l = nbHalfEdges();
				_vertices.clear();
				_vertices.resize(l, -1);
				_nbvertices = 0;
				for (int i = 0; i < l; i++)
					{
					if (_vertices[i] < 0)
						{
						_vertices[i] = _nbvertices;
						int j = sigma(i);
						while (j != i)
							{
							MTOOLS_ASSERT(_vertices[j] < 0);
							_vertices[j] = _nbvertices;
							j = sigma(j);
							}
						_nbvertices++;
						}
					}
				}


			/* compute the faces set from sigma and alpha */
			void _computeFaceSet()
				{
				const int l = nbHalfEdges();
				_faces.clear();
				_faces.resize(l, -1);
				_nbfaces = 0;
				for(int i = 0; i < l; i++)
					{
					if (_faces[i] < 0)
						{
						_faces[i] = _nbfaces;
						int j = phi(i);
						while (j != i)
							{
							MTOOLS_ASSERT(_faces[j] < 0);
							_faces[j] = _nbfaces;
							j = phi(j);
							}
						_nbfaces++;
						}
					}
				}


			/* Triangulate a face but do not recompute the _faces vector */
			int _triangulateFace(int dartIndex)
				{
				const int d = faceSize(dartIndex);
				MTOOLS_ASSERT(d >= 3);
				if (d == 3) return 3; // nothing to do
				int f = (int)_alpha.size();
				int i = dartIndex;
				_alpha.resize(f + 2*d);
				_sigma.resize(f + 2*d);
				_vertices.resize(f + 2*d);
				for (int h = 0; h < d; h++)
					{
					const int nexti = phi(i);
					_vertices[f]   = _vertices[_alpha[i]];
					_vertices[f + 1] = _nbvertices;
					_sigma[f + 1] = ((h > 0) ? (f-1) : ((int)_sigma.size() - 1) );
					_alpha[f + 1] = f;
					_sigma[f] = _sigma[_alpha[i]];
					_sigma[_alpha[i]] = f;
					_alpha[f] = f + 1;
					f += 2;
					i = nexti;
					}
				_nbvertices++;
				// now, _nbfaces and _faces are dirty !
				// should call _computeFaceSet() to make it straight.
				return d;
				}


			int _root;					// root half-edge
			int _nbvertices;
			int _nbfaces;
			std::vector<int> _alpha;	// involution that matches half edges
			std::vector<int> _sigma;	// rotations around vertices
			std::vector<int> _vertices;	// index of vertices associated with half edges
			std::vector<int> _faces;	// index of faces associated with half edges

		};



	}

/* end of file */

