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
#include "combinatorics.hpp"
#include "dyckword.hpp"


namespace mtools
	{



	/**
	* Class that encode a planar graph into a combinatorial map.
	*
	* a planar graph with n edges is encoded with two permutation
	* of size 2n (representing half-edges or arrows) 
	*
	* alpha : Involution that matches half-edges together. 
	* sigma : rotation around a vertex. sigma[i] point to next half
	*         edge around the vertex when rotationg counterclockise.         
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
				}



			/**
			* Construct a rooted planar tree from a Dick word.
			*
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
				}


			/**
			* Number of edges of the graph
			**/
			int nbedges() const { return (int)_alpha.size() / 2; }


			/**
			* Number of vertices of the graph. 
			* THIS IS SLOW : calls findVertices(nbv) and return nbv.
			**/
			int nbvertices() const
				{
				int nbv;
				findVertices(nbv);
				return nbv;
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
				int nbv;
				std::vector<int> vert = findVertices(nbv);	// start vertices of the half edges
				GRAPH gr;
				gr.resize(nbv);
				for (int i = 0; i < vert.size(); i++)
					{
					int v = vert[i];
					if (gr[v].size() == 0)
						{
						gr[v].push_back(vert[_alpha[i]]);
						int j = _sigma[i];
						while (j != i)
							{
							gr[v].push_back(vert[_alpha[j]]);
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
				return std::make_tuple(A,B,C);
				}


			/**
			* Print the into a string
			**/
			std::string toString() const
				{
				int nbv;
				auto vert = findVertices(nbv);
				std::string s("CombinatorialMap: ");
				s += mtools::toString(_alpha.size() / 2) + " edges, " + mtools::toString(nbv) + " vertices (root at " + mtools::toString(_root) + ")\n";
				s += "alpha = [ "; for (int i = 0; i < _alpha.size(); i++) { s += mtools::toString(_alpha[i]) + " "; } s += "]\n";
				s += "sigma = [ "; for (int i = 0; i < _sigma.size(); i++) { s += mtools::toString(_sigma[i]) + " "; } s += "]\n";
				s += "vert  = [ "; for (int i = 0; i < vert.size(); i++) { s += mtools::toString(vert[i]) + " "; } s += "]\n";
				return s;
				}


			/**
			* Serialise/deserialize. Works with boost and with the custom serialization classes
			* OArchive and IArchive. the method performs both serialization and deserialization.
			**/
			template<typename U> void serialize(U & Archive, const int version = 0)
				{
				Archive & _root;
				Archive & _alpha;
				Archive & _sigma;
				}


			/**
			* Create a vector associating each half-edge with its correpsonding vertex. put the total
			* number of vertices in nbv.
			**/
			std::vector<int> findVertices(int & nbv) const
				{
				std::vector<int> vert(_alpha.size(), -1);
				nbv = 0;
				for (int i = 0; i < vert.size(); i++)
					{
					if (vert[i] < 0)
						{
						vert[i] = nbv;
						int j = _sigma[i];
						while (j != i)
							{
							MTOOLS_ASSERT(vert[j] < 0);
							vert[j] = nbv;
							j = _sigma[j];
							}
						nbv++;
						}
					}
				return vert;
				}


			/**
			* Same as above but does not indicate the total number of vertices.
			**/
			std::vector<int> findVertices() const
				{
				int nbv;
				return findVertices(nbv);
				}


		private:


			/* swap indexes i1 and i2 without modifiying the graph */
			void swapIndexes(int i1, int i2)
				{
				if (i1 == i2) return;
				int a1 = _alpha[i1];
				int a2 = _alpha[i2];
				_alpha[i1] = a2;
				_alpha[a2] = i1;
				_alpha[i2] = a1;
				_alpha[a1] = i2;
				int sr1 = _sigma[i1];
				int sr2 = _sigma[i2];
				int lr1 = i1; while (_sigma[lr1] != i1) { lr1 = _sigma[lr1]; }
				int lr2 = i2; while (_sigma[lr2] != i2) { lr2 = _sigma[lr2]; }
				if (sr1 == lr1) { _sigma[i2] = i2; }
				else { _sigma[i2] = sr1; _sigma[lr1] = i2; }
				if (sr2 == lr2) { _sigma[i1] = i1; }
				else { _sigma[i1] = sr2; _sigma[lr2] = i1; }
				}


			std::vector<int> _alpha;	// involution that matches half edges
			std::vector<int> _sigma;	// rotations around vertices
			int _root;					// root half-edge

		};



	}

/* end of file */

