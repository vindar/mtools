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

#include"../io/console.hpp"

namespace mtools
	{



	/**
	* Class that encode a (rooted) combinatorial map.
	* i.e. an unoriented graph together with a rotation system 
	* rooted on a oriented edge. 
	* 
	* The graph has n edges and is encoded with two permutations
	* of size 2n (representing half-edges or arrows) 
	*     - alpha : Involution that matches half-edges together. 
	*     - sigma : Rotation around a vertex. sigma[i] point to   
	*               next half edge around the vertex when rotationg
	*               in the positive orientation.
	*     - phi:    = sigma(alpha(.)). Rotation around a face in          
	*               positive orientation.
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
			* The tree is always rooted at a leaf edge, from the leaf to the interior vertice.
			**/
			CombinatorialMap(const DyckWord & dw)
				{
				fromDyckWord(dw);
				}


			/**
			 * Constructor from a graph (eg std::vector<std::vector<int> > or similar).
			 * the graph must be non oriented (ie symmetric) or the method may crash. 
			 **/
			template<typename GRAPH> CombinatorialMap(const GRAPH & gr)
				{
				fromGraph(gr);
				}


			/**
			 * Equality operator. Return true is the object are exactly the same.
			 * The map must have the same root and the same ordering of vetices to compare equal. 
			 **/
			bool operator==(const CombinatorialMap & cm)
				{
				return ((_root == cm._root) && (_sigma == cm._sigma) && (_alpha == cm._alpha));
				}


			/**
			* Number of edges of the graph
			* The number of half-edge (ie arrows) is twice that number.
			**/
			inline int nbEdges() const { return ((int)_alpha.size())/2; }


			/**
			* Number of half-edges (ie arrows) of the graph
			* The number of half-edge (ie arrows) is twice that number.
			**/
			inline int nbHalfEdges() const { return ((int)_alpha.size()); }


			/**
			* Permutation alpha: involution that
			* matches the half edges together.
			**/
			inline const int & alpha(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges()));
				return _alpha[i]; 
				}


			/**
			* Permutation sigma. Give the next half edge
			* when rotating around a vertex in positive
			* orientation.
			**/
			inline const int & sigma(int i) const 
				{ 
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges()));
				return _sigma[i]; 
				}


			/**
			* Permutation phi. Same as sigma(alpha(.)).
			* Rotates around a face (or equivalently around a vertex of the dual graph).
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
			* Return the vertex associated with half edge i
			**/
			int vertice(int i) const
				{
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges()));
				return _vertices[i];
				}


			/**
			*Return the vertice vector mapping each half edge to its face index.
			**/
			std::vector<int> getVerticeVector() const { return _vertices; }


			/**
			* Return the vertice vector mapping each half edge to its face index.
			* Also put in nbv the number of vertices.
			**/
			std::vector<int> getVerticeVector(int & nbv) const { nbv = nbVertices(); return _vertices; }


			/**
			* Number of faces of the graph.
			**/
			int nbFaces() const { return _nbfaces; }


			/**
			* Return the face associated with half edge i
			**/
			int face(int i) const 
				{
				MTOOLS_ASSERT((i >= 0) && (i < nbHalfEdges())); 
				return _faces[i];
				}


			/**
			 *Return the face vector mapping each half edge to its face index.
			 **/
			std::vector<int> getFaceVector() const { return _faces; }


			/**
			*Return the face vector mapping each half edge to its face index.
			* Also put in nbf the number of vertices.
			**/
			std::vector<int> getFaceVector(int & nbf) const { nbf = nbFaces(); return _faces; }


			/**
			 * Query the gneus of the combinatorial map (from euler characteristic).
			 * The return value is zero iif it is a planar embedding of the graph.
			 * 
			 * the relation holds: V - E + F = 2 - 2g
			 * @return	The genus of the combinatorial map..
			 **/
			inline int genus() const
				{
				int khi = _nbvertices - nbEdges() + _nbfaces;
				MTOOLS_ASSERT((khi % 2) == 0);
				return((2 - khi)/2);
				}


			/**
			* Query if the graph is (connected) tree.
			*
			* @return	true if it is tree, false if not.
			**/
			inline bool isTree() const { return (nbFaces() == 1); }


			/**
			* Query if the combinatorial map is planar.
			* This is equivalent to checking if the genus of the map is zero.
			*
			* @return	true if the embeding is planar, false if not.
			**/
			inline bool isPlanar() const { return (genus() == 0); }


			/**
			* construct the dual combinatorial map
			*
			* @return	The dual graph
			**/
			CombinatorialMap getDual() const
				{
				CombinatorialMap cm(*this);
				const size_t l = nbHalfEdges();
				for (int i = 0; i < l; i++) { cm._sigma[i] = phi(i); }
				cm._computeFaceSet();
				cm._computeVerticeSet();
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
			* The tree is always rooted at a leaf edge, from the leaf to the interior vertice.
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
			 * Load the object from a graph. 
			 * the graph must be non oriented (ie symmetric) or the method may crash.
			 *
			 * @tparam	GRAPH	Type of the graph (eg std::vector<std::vector<int> > or similar)
			 * @param	gr	the graph
			 *
			 * @return	a map that give the index of each oriented edge in the permutations ie
			 * 			map[{u,v}] = i means that the oriented edge from u to v in the graph correpsond
			 * 			to the arrow index i in the combinatorial map. 
			 **/
			template<typename GRAPH> std::map< std::pair<int, int>, int> fromGraph(const GRAPH & gr)
				{
				const size_t l = gr.size();
				int te = 0;
				for (size_t i = 0; i < l; i++) { te += (int)gr[i].size(); } // compute the number of half edges
				_root = 0;
				_sigma.clear();
				_alpha.clear();
				_sigma.reserve(te + 1);
				_alpha.reserve(te + 1);
				_sigma.resize(te);
				_alpha.resize(te);
				// construct the involution
				int j = 0; while (j < te) { _alpha[j] = j + 1; _alpha[j + 1] = j; j += 2; }
				// construct sigma
				std::map< std::pair<int, int>, int> mapEdge;
				int freeindex = 0;
				for (size_t i = 0; i < l; i++)
					{
					int firstindex = -1;
					int previndex = -1;
					for (auto it = gr[i].begin(); it != gr[i].end(); ++it)
						{
						auto res = mapEdge.insert({ std::pair<int, int>((int)i, *it), freeindex }); // try to insert the edge
						if (res.second)
							{ // insertion successful: first time we encounter this edge. 
							mapEdge.insert({ std::pair<int, int>(*it, (int)i), freeindex + 1 }); // insert the oposite edge
							freeindex += 2;
							}
						int index = res.first->second; // index of the edge
						if (previndex < 0) { firstindex = index; } else { _sigma[previndex] = index; }
						previndex = index;
						}
					if (firstindex >= 0) { _sigma[previndex] = firstindex; }
					}
				MTOOLS_ASSERT(freeindex = te);
				_computeVerticeSet();
				_computeFaceSet();
				return mapEdge;
				}


			/**
			 * Converts the object into a graph.
			 *
			 * @tparam	GRAPH	Type of the graph. For example std::vector<std::vector<int> >.
			 *
			 * @return	The corresponding graph object
			 **/
			template<typename GRAPH> GRAPH toGraph() const
				{
				const int l = nbHalfEdges();
				GRAPH gr;
				gr.resize(_nbvertices);
				for(int i = 0; i < l; i++)
					{
					int v = _vertices[i];

					if (v == 9)
						{
						cout << "ok";
						}
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
			* Permute the indices of alpha and sigma according to a permutation
			*This changes the numbering of the vertices
			* 
			* @param	perm   	The permutation
			* @param	invperm	its inverse
			**/
			void permute(const Permutation  & perm, const Permutation & invperm)
				{
				const size_t l = _sigma.size();
				MTOOLS_ASSERT(perm.size() == l);
				auto sigma2 = _sigma;
				auto alpha2 = _alpha;
				for (int i = 0; i < l; i++)
					{
					_sigma[i] = invperm[sigma2[perm[i]]];
					_alpha[i] = invperm[alpha2[perm[i]]];
					}
				_root = invperm[_root];
				_computeVerticeSet();
				_computeFaceSet();
				}


			/**
			* Same as above but when the inverse was not previously precalculated
			**/
			void permute(const Permutation & perm)
				{
				permute(perm, invertPermutation(perm));
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

				_canonicalTreeOrdering();

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
			* Print the into a string
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
				if (genus() == 0) s += std::string(" (PLANARY EMBEDDING)");
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
			* Serialise
			**/
			void serialize(OArchive & ar, const int version = 0)
				{
				ar & _root;
				ar & _alpha;
				ar & _sigma;
				}


			/**
			* Deserialise
			**/
			void deserialize(IArchive & ar, const int version = 0)
				{
				ar & _root;
				ar & _alpha;
				ar & _sigma;
				_computeVerticeSet();
				_computeFaceSet();
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


			/* Permute sigma and alpha in such way that phi = sigma(alpha(.)) if the circular
			permuation i->i+1 
			The map must be a tree. */
			void _canonicalTreeOrdering()
				{
				std::vector<int> ord(nbHalfEdges(), -1);
				int i = 1;
				ord[0] = 0;
				int x = phi(0);
				while (x != 0) { ord[x] = i; i++; x = phi(x); }
				MTOOLS_INSURE(i == nbHalfEdges()); // make sure we are dealing with a tree
				permute(getSortPermutation(ord));
				}

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


			std::vector<int> _alpha;	// involution that matches half edges
			std::vector<int> _sigma;	// rotations around vertices
			int _root;					// root half-edge
			std::vector<int> _vertices;	// index of vertices associated with half edges
			int _nbvertices;
			std::vector<int> _faces;	// index of faces associated with half edges
			int _nbfaces;
		};



	}

/* end of file */

