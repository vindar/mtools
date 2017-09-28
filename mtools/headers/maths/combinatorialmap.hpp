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

#include "../misc/internal/mtools_export.hpp"
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


	#ifdef _DEBUG
	#define CHECKCONSISTENCY checkConsistency();
	#else
	#define CHECKCONSISTENCY (0);
	#endif



	/* forward declaration from graph.hpp */
	template<typename GRAPH>  bool isGraphSimple(const GRAPH & gr);
	template<typename GRAPH>  bool isGraphEmpty(const GRAPH & gr);
	template<typename GRAPH> GRAPH resizeGraph(const GRAPH & graph, size_t newSize);
	template<typename GRAPH> GRAPH permuteGraph(const GRAPH & graph, const Permutation  & perm);


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
				CHECKCONSISTENCY;
				}


			/** Create a n-gon (a cycle with n=edges i.e. 2n darts) */
			CombinatorialMap(int n)
				{
				makeNgon(n);
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
			bool operator==(const CombinatorialMap & cm) const
				{
				return ((_root == cm._root) && (_sigma == cm._sigma) && (_alpha == cm._alpha)&&(_vertices == cm._vertices)&&(_faces == cm._faces));
				}


			/**
			* Number of non-oriented edges of the graph
			**/
			inline int nbEdges() const { return ((int)_alpha.size()) / 2; }


			/**
			* Number of half-edges (ie darts) of the graph
			* This is equal to twice nbEdges().
			**/
			inline int nbDarts() const { return ((int)_alpha.size()); }


			/**
			 * Return the index of the root dart (i.e. oriented edge).
			 **/
			inline int root() const { return _root; }


			/**
			 * Re-roots the map on another dart.
			 **/
			inline void reroot(int newRootDart)
				{
				MTOOLS_ASSERT((newRootDart >= 0) && (newRootDart < nbDarts()));
				_root = newRootDart;
				}


			/**
			* Permutation alpha: involution that matches the darts
			* together.
			**/
			inline int alpha(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbDarts()));
				return _alpha[i]; 
				}


			/**
			* Permutation sigma. Give the next dart when rotating 
			* around a vertex in the positive orientation.
			**/
			inline int sigma(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbDarts()));
				return _sigma[i]; 
				}


			/**
			* Inverse of permutation sigma. Give the previous dart when 
			* rotating around a vertex in the positive orientation. 
			* (slower that sigma).
			**/
			inline int invsigma(int i) const
				{
				MTOOLS_ASSERT((i >= 0) && (i < nbDarts()));
				int prev = i; 
				while (_sigma[prev] != i) { prev = _sigma[prev]; }
				return prev;
				}


			/**
			* Permutation phi. Same as sigma(alpha(.)).
			* Rotates around a face (or equivalently around a vertex of 
			* the dual graph).
			**/
			inline int phi(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbDarts()));
				return _sigma[_alpha[i]]; 
				}


			/**
			* Inverse of permutation phi. 
			* Rotate around a face in the other direction 
			* (slower than phi).
			**/
			inline int invphi(int i) const
				{
				MTOOLS_ASSERT((i >= 0) && (i < nbDarts()));
				return _alpha[invsigma(i)];
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
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbDarts()));
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
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbDarts()));
				int n = 1, j= sigma(dartIndex);
				while (j != dartIndex) { j = sigma(j); n++; }
				return n;
				}


			/**
			* Return a vector of size nbDarts() that describe the 
			* start vertex index of each dart.
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
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbDarts()));
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
			int faceSize(int dartIndex) const
				{
				MTOOLS_ASSERT((dartIndex >= 0) && (dartIndex < nbDarts()));
				int n = 1, j = phi(dartIndex);
				while (j != dartIndex) { j = phi(j); n++; }
				return n;
				}


			/**
			 * Return a vector of size nbDarts() that describe the face index
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
				const size_t l = nbDarts();
				cm._sigma.resize(l);
				for (int i = 0; i < (int)l; i++) { cm._sigma[i] = phi(i); } // invert phi becomes sigma
				cm._vertices = _faces;
				cm._nbvertices = _nbfaces;
				cm._faces = _vertices;
				cm._nbfaces = _nbvertices;
				cm._root = _root;
				return cm;
				}


			/**
			 * Create a n-gon (cycle graph with n edges). 
			 * Odd numbered darts are on one side and even numbered darts on the other side with 
			 * matching (2i) <-> (2i+1).
			 *
			 * @param	n	the number of non oriented edges in the cycle.
			 **/
			void makeNgon(int n)
				{
				_root = 0;
				_nbvertices = n;
				_nbfaces = 2;				
				_alpha.resize(2*n);
				_sigma.resize(2*n);
				_faces.resize(2 * n);
				_vertices.resize(2*n);
				for (int i = 0;i < n; i++) 
					{ 
					_alpha[2*i] = 2*i + 1; _alpha[2*i + 1] = 2*i; 
					_faces[2*i] = 0; _faces[2*i + 1] = 1; 
					_sigma[2*i + 1] = ((2*i + 2)%(2*n)); _sigma[(2*i + 2)%(2*n)] = 2*i + 1;
					_vertices[2*i + 1] = ((i+1)%n); _vertices[(2*i + 2)%(2*n)] = ((i+1)%n);
					}
				CHECKCONSISTENCY;
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
				CHECKCONSISTENCY;
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
				CHECKCONSISTENCY;
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
				CHECKCONSISTENCY;
				return _toGraph<GRAPH>();
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
				const int l = nbDarts();
				for (int i = 0; i < l; i++) 
					{ 
					_triangulateFace(i); 
					}
				_computeFaceSet();
				CHECKCONSISTENCY;
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
			 * @param	dartIndex	The index of a dart that identifies the face to triangulate 
			 * 						(note that this is NOT the index of the face itself!).
			 *
			 * @return	The degree of the face that was triangulated.
			 **/
			int triangulateFace(int dartIndex)
				{
				int d = _triangulateFace(dartIndex);
				_computeFaceSet();
				CHECKCONSISTENCY;
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
			CombinatorialMap getPermute(const Permutation  & perm) const
				{
				CHECKCONSISTENCY;
				const int l = nbDarts();
				MTOOLS_ASSERT((int)perm.size() == l);
				CombinatorialMap cm;
				cm._alpha.resize(l);
				cm._sigma.resize(l);
				cm._vertices.resize(l);
				cm._faces.resize(l);
				for (int i = 0; i < l; i++)
					{
					cm._sigma[i]    = perm.inv(_sigma[perm[i]]);
					cm._alpha[i]    = perm.inv(_alpha[perm[i]]);
					cm._vertices[i] = _vertices[perm[i]];
					cm._faces[i]    = _faces[perm[i]];
					}
				cm._nbvertices = _nbvertices;
				cm._nbfaces = _nbfaces;
				cm._root = perm.inv(_root);
				CHECKCONSISTENCY;
				return cm;
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
				CHECKCONSISTENCY;
				// we need to make sure that the numbering of the edges follow the contour of the tree.
				const int len = nbDarts();
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
					Permutation perm(ord);
					auto _alpha2 = _alpha;
					auto _sigma2 = _sigma;
					for (int i = 0; i < len; i++)
						{
						_sigma[i] = perm.inv(_sigma2[perm[i]]);
						_alpha[i] = perm.inv(_alpha2[perm[i]]);
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
				CHECKCONSISTENCY;
				return std::make_tuple(A,B,C);
				}


			/**
			 * Adds a triangle inside a face.
			 * 
			 * The triangle is glued against the NEXT oriented edge following dartindex on the same face i.e.
			 * phi(dartIndex) =: E. This creates an additionnal vertex inside the face and two edges from it to
			 * the endpoints of E.
			 * 
			 * Thus, the procedure effectively increases the number of darts by 4, the number of non-
			 * oriented edge by two, the number of face by 1 and the number of vertices by 1.
			 *
			 * This method only creates new darts, vertices and faces but does not change any previous numbering 
			 * (of darts, vertices, faces etc...).
			 * 
			 * @param	dartIndex	The dart PRECEDING the one to which the face should be added.
			 */
			void addTriangle(int dartIndex)
				{
				CHECKCONSISTENCY;
				_addTriangle(dartIndex);
				CHECKCONSISTENCY;
				}


			/**
			 * Add a triangle inside a face, effectively splitting it into 3 faces, the center one being the
			 * triangle. Just like addTriangle(), the base of the triangle is the NEXT edge after dartIndexBase 
			 * on the same face i.e. phi(dartIndexBase). The third vertex of the triangle is the END vertex 
			 * of dartIndexTarget. 
			 *
			 * The number of faces is increased by two. The number of dart by 4, the number of non-oriented 
			 * edges by 2 and the number of vertices remains the same. 
			 *
			 * NB: the method work also for double edge but not for loop i.e. it is forbidden that
			 *     dartIndexTarget = dartIndexBase or phi(dartIndexBase)...
			 *
			 * This method only creates new darts, vertices and faces but does not change any previous numbering
			 * (of darts, vertices, faces etc...).
			 *
			 * @param	dartIndexBase  		The dart PRECEDING the base of the triangle
			 * @param	dartIndexTarget		The dart whose ENDPOINT is the third vertex of the triangle
			 * @param   collapsedoubleedge  true to collapse double edges created if dartIndexTarget is either
			 * 								invphi(dartIndexBase) or phi(phi(dartIndexBase)). In this case,
			 * 								the parralel edges that should be created are ignored.
			 *
			 * Returns the size (len) of the face that contain [dartIndexTarget].
			 *         the size of the face that contain [dartIndexBase] is (initialFaceSize - len + 1).
			 *         
			 *         -> if collapsedoubleedge = true, the method still return 2 when a face was not 
			 *            created since it would have has size 2.
			 */
			int addSplittingTriangle(int dartIndexBase, int dartIndexTarget, bool collapsedoubleedge = true)
				{
				CHECKCONSISTENCY;
				int len = _addSplittingTriangle(dartIndexBase, dartIndexTarget, collapsedoubleedge);
				CHECKCONSISTENCY;
				return len;
				}



			/**
			 * Collapse a face of size 2 into a single edge.
			 * 
			 * The method remove the dart and alpha[dart]. This destroy the face of size two leaving only the 
			 * other edge of the face.
			 * 
			`* -> Reduces the number of edges by 1 (hence the number of darts by 2) and reduces the number
			 *    of faces by 1. The numbering of vertices remain unchanged but the numbering of the dart and 
			 *    faces are modified.
			 *
			 * -> The method cannot be use to remove double edges that do not define a face of size two (ie if
			 *    there are edges in between).
			 *
			 * @param	dart	The dart to remove (with its alpha[dart]). It must belong to the face of size 2.
			 **/
			void collapseFaceOfSize2(int dart)
				{
				CHECKCONSISTENCY;
				const int f1 = _collapseFaceOfSize2(dart);  // remove the face
				const int f2 = _nbfaces - 1;
				const int l = (int)_alpha.size();
				for (int i = 0; i < l; i++) // face f1 has disappeared, move f2 to f1
					{ 
					if (_faces[i] == f2) { _faces[i] = f1; }
					}
				_nbfaces--;
				CHECKCONSISTENCY;
				}


			/**
			 * Algorithm to peel a given face of the map.
			 *
			 * @param	predart   	The dart that specify the face to peel (the fisrt step peels the edge FOLLOWING predart just as
			 * 						for addTriangle() and addSplittingTriangle() 
			 * 						
			 * @param	fun		 	The function to call at each step of the peeling process.
			 * 						int fun(int & edgetopeel,int facesize)
			 * 						     - predartpeel : the dart PRECEDING the one that should be peeled. A default choice is   
			 * 						                     proposed but can be changed.  
			 * 						     - facesize    : the number of edges on this face.    
			 * 						      
			 * 						the function should return the action to take:
			 * 						     -2   : stop peeling this face.
			 * 						     -1   : create a triangle with a new vertex and base phi(predartpeel i.e. the edge AFTER predartpeel.
			 * 						    k>=0  : create a triangle with base phi(predartpeel) and third vertex the ENDPOINT of dart index k
			 *  						    
			 * @param    collapsedoubleedge    true to collapse double edges created when the splitting position k = fun() returned is either 
			 * 								   k = invphi(predartpeel) or k = phi(phi(predartpeel)). In this case, the face of size 2 are not
			 * 								   explored and the parralel edges that should be created are collapsed
			 * 
			 * Peeling default stategy:
			 *
			 *  - if a new verrtex is added, the proposed dart at the next step is the same as the previous step i.e. predartpeel  .
			 *  - if the n-gon is split in two. the proposed darts at the next step are predartpeel and the dart k returned by the function.   
			 *   
			 */
			void boltzmannPeelingAlgo(int predart, std::function< int(int,int)> fun, bool collapsedoubleedge = true)
				{
				CHECKCONSISTENCY;
				_boltzmannPeelingAlgo(predart, fun, faceSize(predart), collapsedoubleedge); // run the algorithm recursively
				CHECKCONSISTENCY;
				return;
				}


			/**
			 * Remove all the vertices that are contained between two parallel double edges and loops and
			 * then collapse the remaining faces of size 2 into a single edge. Use the root face to
			 * determine whether a vertex is "inside" or "outside" a double edge: the root face is always
			 * outside (hence it is not removed).
			 * 
			 *  - This method returns a type III graph (ie without double edges or loops).
			 * 
			 *  - the numbering of faces, vertices and darts is modified !
			 *
			 * @return the permutation that describe how the label of darts changed. 
			 **/
			Permutation collapsetoTypeIII()
				{
				CHECKCONSISTENCY;
				return _collapsetoTypeIII();
				}



			/**
			* Print infos about the map into a string
			* set the detailed to true to print the complete map
			**/
			std::string toString(bool detailed = false) const
				{
				CHECKCONSISTENCY;
				std::string s("CombinatorialMap: (");
				s += mtools::toString(nbDarts()) + " darts)\n";
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
					s += std::string("alpha     = [ "); for (int i = 0; i < (int)_alpha.size(); i++) { s += mtools::toString(_alpha[i]) + " "; } s += "]\n";
					s += std::string("sigma     = [ "); for (int i = 0; i < (int)_sigma.size(); i++) { s += mtools::toString(_sigma[i]) + " "; } s += "]\n";
					s += std::string("vertices  = [ "); for (int i = 0; i < (int)_vertices.size(); i++) { s += mtools::toString(_vertices[i]) + " "; } s += "]\n";
					s += std::string("faces     = [ "); for (int i = 0; i < (int)_vertices.size(); i++) { s += mtools::toString(_faces[i]) + " "; } s += "]\n";
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
				CHECKCONSISTENCY;
				}



			/**
			* Make sure that the object is in a consistent state.
			* The macro CHECKCONSISTENCY call this method if _DEBUG is set. 
			**/
			void checkConsistency() const
				{
				const size_t l = _alpha.size();
				MTOOLS_INSURE(l >= 2);
				MTOOLS_INSURE(_sigma.size() == l);
				MTOOLS_INSURE(_vertices.size() == l);
				MTOOLS_INSURE(_faces.size() == l);
				MTOOLS_INSURE((_root >= 0) && (_root < (int)l));

				// alpha must be fixed point free involution
				MTOOLS_INSURE(isPermutationVector(_alpha));
				for (size_t i = 0;i < l; i++)
					{
					MTOOLS_INSURE(_alpha[i] != i);
					MTOOLS_INSURE(_alpha[_alpha[i]] == i);
					}

				// _sigma can be any permutation
				MTOOLS_INSURE(isPermutationVector(_sigma));

				// make sure the vertices vector is ok
				MTOOLS_INSURE((_nbvertices > 0) && (_nbvertices <= (int)l));

				{ // check that the vertice vector is ok.
				std::vector<int> vv(l, -1);
				std::set<int> vs;
				for (int i = 0;i < (int)l; i++)
					{
					MTOOLS_INSURE((_vertices[i] >= 0) && (_vertices[i] < _nbvertices));
					if (vs.find(_vertices[i]) != vs.end()) { MTOOLS_INSURE(_vertices[i] == vv[i]); }
					else
						{
						MTOOLS_INSURE(vv[i] == -1);
						vv[i] = _vertices[i];
						vs.insert(_vertices[i]);
						int j = _sigma[i];
						while (j != i)
							{
							MTOOLS_INSURE(vv[j] == -1);
							MTOOLS_INSURE(_vertices[j] == _vertices[i]);
							vv[j] = _vertices[i];
							j = _sigma[j];
							}
						}
					}
				MTOOLS_INSURE(vs.size() == _nbvertices);
				}

				{ // check that the face vector is ok.
				std::vector<int> ff(l, -1);
				std::set<int> fs;
				for (int i = 0; i < (int)l; i++)
					{
					MTOOLS_INSURE((_faces[i] >= 0) && (_faces[i] < _nbfaces));
					if (fs.find(_faces[i]) != fs.end()) { MTOOLS_INSURE(_faces[i] == ff[i]); }
					else
						{
						MTOOLS_INSURE(ff[i] == -1);
						ff[i] = _faces[i];
						fs.insert(_faces[i]);
						int j = _sigma[_alpha[i]];
						while (j != i)
							{
							MTOOLS_INSURE(ff[j] == -1);
							MTOOLS_INSURE(_faces[j] == _faces[i]);
							ff[j] = _faces[i];
							j = _sigma[_alpha[j]];
							}
						}
					}
				MTOOLS_INSURE(fs.size() == _nbfaces);
				}
				return;
				}





		private:



			/* Private method that does not check for consistency */
			void _addTriangle(int dartIndex)
				{
				const int l = (int)_alpha.size();
				_alpha.resize(l + 4);
				_sigma.resize(l + 4);
				_vertices.resize(l + 4);
				_faces.resize(l + 4);

				const int F = _faces[dartIndex];
				const int a = _alpha[dartIndex];
				const int b = _sigma[a];
				const int c = _alpha[b];
				const int d = _sigma[c];
				const int v1 = _vertices[a];
				const int v2 = _vertices[c];

				_alpha[l + 0] = l + 1;  _alpha[l + 1] = l + 0;
				_alpha[l + 2] = l + 3;  _alpha[l + 3] = l + 2;

				_sigma[a] = l + 0; _sigma[l + 0] = b;
				_sigma[c] = l + 3; _sigma[l + 3] = d;
				_sigma[l + 1] = l + 2; _sigma[l + 2] = l + 1;

				_vertices[l + 0] = v1;
				_vertices[l + 3] = v2;
				_vertices[l + 1] = _nbvertices;
				_vertices[l + 2] = _nbvertices;
				_nbvertices++;

				_faces[b] = _nbfaces;
				_faces[l + 3] = _nbfaces;
				_faces[l + 1] = _nbfaces;
				_faces[l + 0] = F;
				_faces[l + 2] = F;
				_nbfaces++;
				}


			/* Private method */
			int _addSplittingTriangle(int dartIndexBase, int dartIndexTarget, bool collapsedoubleedge)
				{
				MTOOLS_ASSERT((dartIndexBase >= 0) && (dartIndexBase < (int)_alpha.size()));
				MTOOLS_ASSERT((dartIndexTarget >= 0) && (dartIndexTarget < (int)_alpha.size()));
				MTOOLS_INSURE(_faces[dartIndexBase] == _faces[dartIndexTarget]);
				MTOOLS_INSURE(dartIndexTarget != dartIndexBase);
				MTOOLS_INSURE(dartIndexTarget != phi(dartIndexBase));

				bool ignore1 = false;
				bool ignore2 = false;
				if (collapsedoubleedge)
					{
					if (dartIndexBase == phi(dartIndexTarget)) { ignore1 = true; }
					if (phi(phi(dartIndexBase)) == dartIndexTarget) { ignore2 = true; }
					}
				if (ignore1 && ignore2) return 2; // we have a triangle that should not be split.

				if (ignore2)
					{ // only 1 edge to add. 
					const int l = (int)_alpha.size();
					_alpha.resize(l + 2);
					_sigma.resize(l + 2);
					_vertices.resize(l + 2);
					_faces.resize(l + 2);
					const int F = _faces[dartIndexBase];
					const int a = _alpha[dartIndexBase];
					const int b = _sigma[a];
					const int c = _alpha[b];
					const int d = _sigma[c]; // equal to dartIndexTarget here
					const int e = _alpha[dartIndexTarget];
					const int f = _sigma[e];
					const int v1 = _vertices[a];
					const int v2 = _vertices[c];
					const int v3 = _vertices[e];
					_alpha[l + 0] = l + 1;  _alpha[l + 1] = l + 0;
					_sigma[a] = l + 0; _sigma[l + 0] = b;
					_sigma[e] = l + 1; _sigma[l + 1] = f;
					_vertices[l + 0] = v1;
					_vertices[l + 1] = v3;
					_faces[l + 0] = F;
					_faces[b] = _nbfaces;
					_faces[d] = _nbfaces;
					_faces[l + 1] = _nbfaces;
					_nbfaces++;
					return 2;
					}

				if (ignore1)
					{ // only 1 edge to add
					int len = faceSize(dartIndexBase); // compute size of face before changes
					const int l = (int)_alpha.size();
					_alpha.resize(l + 2);
					_sigma.resize(l + 2);
					_vertices.resize(l + 2);
					_faces.resize(l + 2);
					const int F = _faces[dartIndexBase];
					const int a = _alpha[dartIndexBase];
					const int b = _sigma[a];
					const int c = _alpha[b];
					const int d = _sigma[c];
					const int e = _alpha[dartIndexTarget];
					const int f = _sigma[e]; // equal to dartIndexBase here
					const int v1 = _vertices[a];
					const int v2 = _vertices[c];
					const int v3 = _vertices[e];
					_alpha[l + 0] = l + 1;  _alpha[l + 1] = l + 0;
					_sigma[c] = l + 1; _sigma[l + 1] = d;
					_sigma[e] = l + 0; _sigma[l + 0] = f;
					_vertices[l + 0] = v3;
					_vertices[l + 1] = v2;
					_faces[l + 0] = F;
					_faces[f] = _nbfaces;
					_faces[b] = _nbfaces;
					_faces[l + 1] = _nbfaces;
					_nbfaces++;
					return len - 1;
					}

				// normale setting, rwo edges to add
				const int l = (int)_alpha.size();
				_alpha.resize(l + 4);
				_sigma.resize(l + 4);
				_vertices.resize(l + 4);
				_faces.resize(l + 4);

				const int F = _faces[dartIndexBase];
				const int a = _alpha[dartIndexBase];
				const int b = _sigma[a];
				const int c = _alpha[b];
				const int d = _sigma[c];

				const int e = _alpha[dartIndexTarget];
				const int f = _sigma[e];

				const int v1 = _vertices[a];
				const int v2 = _vertices[c];
				const int v3 = _vertices[e];

				_alpha[l + 0] = l + 1;  _alpha[l + 1] = l + 0;
				_alpha[l + 2] = l + 3;  _alpha[l + 3] = l + 2;

				_sigma[a] = l + 0; _sigma[l + 0] = b;
				_sigma[c] = l + 3; _sigma[l + 3] = d;
				_sigma[e] = l + 2; _sigma[l + 2] = l + 1; _sigma[l + 1] = f;

				_vertices[l + 0] = v1;
				_vertices[l + 3] = v2;
				_vertices[l + 1] = v3;
				_vertices[l + 2] = v3;

				_faces[l + 0] = F;
				_faces[b] = _nbfaces;
				_faces[l + 3] = _nbfaces;
				_faces[l + 1] = _nbfaces;
				_nbfaces++;

				int len = 1;
				int k = d;
				while (k != (l + 2))
					{
					_faces[k] = _nbfaces; k = phi(k); len++;
					}
				_faces[k] = _nbfaces;
				_nbfaces++;
				return len;
				}


			/* private method. leaves _faces[] and _nbfaces inconsistent since there
			   is one less face (but renumbering is not performed and _nbfaces is not
			   decremented).
			   Return the (now available) index of the face that was removed. */
			int _collapseFaceOfSize2(int dart)
				{
				const int l = (int)_alpha.size();
				const int dart2 = _alpha[dart];
				MTOOLS_ASSERT(l >= 4);
				MTOOLS_ASSERT(phi(dart) != dart2);       // not a flat face (this would reove a vertex)
				MTOOLS_ASSERT(phi(phi(dart)) == dart);   // but indeed a real face of degree two
				const int F = _faces[dart]; //
				int a = l - 2;
				int b = l - 1;
				_swapdarts(dart, a);
				_swapdarts(dart2, b);
				int c = phi(a);
				int d = _alpha[c];
				_sigma[invsigma(b)] = c;
				_sigma[d] = _sigma[a];
				_faces[c] = _faces[b];
				if (_root == a) { _root = d; }
				else if (_root == b) { _root = c; }
				_alpha.resize(l - 2);
				_sigma.resize(l - 2);
				_vertices.resize(l - 2);
				_faces.resize(l - 2);
				return F;
				}


			/* internal method */
			void _boltzmannPeelingAlgo(int preDart, std::function<int(int &, int)> fun, int fsize, bool collapsedoubleedge)
				{
				MTOOLS_INSURE((preDart >= 0) && (preDart < (int)_alpha.size()));
				std::queue<std::pair<int, int> > que;
				que.push(std::pair<int, int>(preDart, fsize));
				while (que.size() > 0)
					{
					int facesize, preedge;
					std::tie(preedge,facesize) = que.front(); que.pop();
					int res = fun(preedge, facesize); // query the peeling action, may change preedege
					MTOOLS_INSURE((res >= -2)&&(res < ((int)_alpha.size())));
					if (res == -1) 
						{ // discover a new triangle
						_addTriangle(preedge); que.push(std::pair<int, int>(preedge, facesize + 1)); 
						}
					else
						{ // split the n-gon
						if (res >= 0)
							{
							int fs2 = _addSplittingTriangle(preedge, res, collapsedoubleedge);
							int fs1 = facesize - fs2 + 1;
							// push sub n-gons if needed
							if ((fs1 > 2) || (!collapsedoubleedge)) { que.push(std::pair<int, int>(preedge, fs1)); }
							if ((fs2 > 2) || (!collapsedoubleedge)) { que.push(std::pair<int, int>(res, fs2)); }
							}
						}
					}
				return;
				}


			/* move a dart from i to f, used by _swapdarts 
			   leave the object in an inconsistent state */
			void _movedart(int i, int f)
				{
				int a = _alpha[i];
				_alpha[f] = a;
				_alpha[a] = f;
				int n = _sigma[i];
				int p = invsigma(i);
				_sigma[f] = ((n == i) ? f : n);
				_sigma[p] = f;
				_faces[f] = _faces[i];
				_vertices[f] = _vertices[i];
				if (_root == i) { _root = f; }
				}


			/* swap indexes i1 and i2 without modifiying the graph */
			void _swapdarts(int i, int j)
				{
				/* TODO : implement without resize() */
				if (i == j) return;
				const int l = (int)_alpha.size();
				_alpha.resize(l + 1);
				_sigma.resize(l + 1);
				_vertices.resize(l + 1);
				_faces.resize(l + 1);
				_movedart(i, l);
				_movedart(j, i);
				_movedart(l, j);
				_alpha.resize(l);
				_sigma.resize(l);
				_vertices.resize(l);
				_faces.resize(l);
				}
				
				
			/* Compute the vertex set from sigma and alpha */
			void _computeVerticeSet()
				{
				const int l = (int)_alpha.size();
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
				const int l = (int)_alpha.size();;
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
				// now, _nbfaces and _faces are dirty. Call _computeFaceSet() to make it straight.
				return d;
				}



			/* private method */
			Permutation _collapsetoTypeIII()
				{
				MTOOLS_INSURE(_vertices[_root] != _vertices[_alpha[_root]]); // make sure that the root is not a loop !
				const int l = (int)_alpha.size();
				// make a list of all double edges and loop
				std::vector<int> darttype(l, 0); // 0 = normal edge, 1 = double edge, 2 = loop
				{
				std::map<std::pair<int, int>, int> mape;
				for (int i = 0; i < l; i++)
					{
					int va = _vertices[i];
					int vb = _vertices[_alpha[i]];
					if (va == vb) { darttype[i] = 2; }
					else
						{
						int & n = mape[{va, vb}];
						if (n == 0) { n = (i+1); } else { darttype[i] = 1; darttype[n-1] = 1; }
						}
					}
				}
				// we start from the root face and explore the graph's faces, marking along the way all the pairs of darts that belong to faces explored. 
				std::vector<int> exploreddarts(l, 0); // -1 = explored, 0 = not explored
				{
				std::vector<int> exploredfaces(_nbfaces, 0);
				std::vector<int> ve1; ve1.reserve(_nbfaces);
				std::vector<int> ve2; ve2.reserve(_nbfaces);
				exploredfaces[_faces[_root]] = 1;
				ve1.push_back(_root);
				while (ve1.size() > 0)
					{
					ve2.clear();
					for (auto it = ve1.begin(); it != ve1.end(); it++)
						{
						int e = (*it);
						do {
							const int oe = _alpha[e];
							exploreddarts[e]  = -1;
							exploreddarts[oe] = -1;
							const int tt = darttype[e];
							if (tt == 0) // normal edge: explore the other side
								{ 
								const int of = _faces[oe];
								if (exploredfaces[of] == 0) { exploredfaces[of] = 1; ve2.push_back(oe); }
								}
							else
								{ 
								if (tt == 1) // double edge, skip the face of size 2.
									{
									const int va = _vertices[e];
									int z = _sigma[oe];
									while (_vertices[_alpha[z]] != va) { z = _sigma[z]; }
									MTOOLS_ASSERT(z != oe);
									const int of = _faces[z];
									if (exploredfaces[of] == 0) { exploredfaces[of] = 1; ve2.push_back(z); }
									}
								}
							e = phi(e);
							}
						while (e != (*it));
						}
					ve1.swap(ve2);
					}
				}
				// tag the additional edges to remove to loops and keep only one double edge of each type.
				{
				std::map<std::pair<int, int>, int> mape;
				exploreddarts[_root] = -2; exploreddarts[_alpha[_root]] = -2;
				const int rva = _vertices[_root], rvb = _vertices[_alpha[_root]];
				mape[{rva, rvb}] = 1; mape[{rvb, rva}] = 1;
				for (int i = 0; i < l; i++)
					{
					int oi = _alpha[i];
					if (exploreddarts[i] == -1) // only look at darts whose status is yet unknown
						{
						int va = _vertices[i], vb = _vertices[oi];
						if (va == vb) { exploreddarts[i] = 0; exploreddarts[oi] = 0; } // remove loops
						else
							{
							int & j1 = mape[{va, vb}];
							int & j2 = mape[{vb, va}];
							MTOOLS_ASSERT(j1 == j2);
							if (j1 == 0) { exploreddarts[i] = -2; exploreddarts[oi] = -2; } // keep the fist pair of dart
							else { exploreddarts[i] = 0; exploreddarts[oi] = 0; MTOOLS_ASSERT(j1 == 1); } // but remove the other ones
							j1++; j2++;
							}
						}
					}
				}
				// ok, the new map is composed of all the darts e such that exploreddarts[e] == -2.
				// we fix the _sigma vector and count the number of dart remaining
				int nbdarts = 0;
				for (int i = 0;i < l; i++)
					{
					MTOOLS_ASSERT((exploreddarts[i] == -2) || (exploreddarts[i] == 0));
					if (exploreddarts[i] == -2) // the dart is kept in the new graph. 
						{
						int j = _sigma[i]; 
						while (exploreddarts[j] != -2) {j = _sigma[j]; }
						_sigma[i] = j;
						nbdarts++;
						}
					}
				// ok, we can now reorder and troncate 
				Permutation perm(exploreddarts);
				std::vector<int > sigma2(l);
				std::vector<int > alpha2(l);
				for (int i = 0; i < l; i++)
					{
					sigma2[i] = perm.inv(_sigma[perm[i]]);
					alpha2[i] = perm.inv(_alpha[perm[i]]);
					}
				_root = perm.inv(_root);
				_alpha = alpha2; _alpha.resize(nbdarts);
				_sigma = sigma2; _sigma.resize(nbdarts);
				_computeFaceSet();
				_computeVerticeSet();
				CHECKCONSISTENCY;
				// done !
				return perm;
				}



			/* convert to a graph, private method */
			template<typename GRAPH> GRAPH _toGraph() const
				{
				const int l = nbDarts();
				GRAPH gr;
				gr.resize(_nbvertices);
				for (int i = 0; i < l; i++)
					{
					int v = _vertices[i];
					if (gr[v].size() == 0)
						{
						gr[v].push_back(_vertices[_alpha[i]]);
						int j = _sigma[i];
						while (j != i)
							{
							gr[v].push_back(_vertices[_alpha[j]]);
							j = _sigma[j];
							}
						}
					}
				return gr;
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
	
