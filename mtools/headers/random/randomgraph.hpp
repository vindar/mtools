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
#include "../misc/stringfct.hpp" 
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
		const size_t l = vec.size();
		if (l < 2) return;
		for (size_t i = l-1; i > 0; --i)
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



	/**  
	 * Class representing a dyck word with a given weight for the ups (default 1)
	 * weight = 1 <-> encode a planar tree with nup edges
	 * weight > 1 <-> encode a planar tree where each non-leaf node has exactly weight-1 leafs, with nup non-leafs edges. 
	 * the word is rooted at its absolute minimum.
	 **/
	class DyckWord
		{

		public:

			/** ctor. Empty dyck word (0 ups)
			 **/
			DyckWord() : _weight(1), _nup(0), _root(0), _vec(1,0)
				{
				}


			/** ctor. simple dyck word. Up then down
			**/
			DyckWord(int nup, int weight = 1) : _weight(weight), _nup(nup), _root(0)
				{
				MTOOLS_ASSERT(weight > 0);
				MTOOLS_ASSERT(nup >= 0);
				_vec.resize((_weight == 1) ? (2 * _nup + 1) : ((1 + _weight)*_nup + (_weight - 1)));
				for (int i = 0; i < _nup; i++) { _vec[i] = 1; }
				}


			/**
			 * Shuffles the world uniformly
			 **/
			template<typename random_t> void shuffle(random_t gen)
				{
				randomShuffle(_vec, gen);
				reroot();
				}


			/** Access a Dyck word letter **/
			const char & operator[](int i) const
				{
				const int l = (int)_vec.size();
				const int p = i + _root;
				return _vec[(p%l) + ((p<0) ? l : 0)];
				}


			/**
			* Total length of the word. 
			* This count the last downward "marker" step.
			* if (weight > 1), the word is rooted at a bud so there is only weigth-1 step down from the origin
			**/
			inline int length() const { return (int)_vec.size(); }


			/**
			 * return the number of edges of the tree encoded by this word. 
			 * ths is just the number of ups if weight = 1.
			 **/
			inline int nbedges() const { return (int)(_nup*_weight) + (_weight - 1); }


			/**
			 * Return the weight associated with this word.
			 **/
			inline int weight() const { return _weight; }


			/**
			 * Return the number of ups of this word
			 * if weight > 1, this correpsond to the number of interior edges (those connecting non-leaf vertices)
			 * in the encoded tree. 
			 **/
			inline int nups() const { return _nup; }


			/**
			 * Print the word into a string
			 **/
			std::string toString() const
				{
				std::string s("[");
				for (int i = 0; i < _vec.size(); i++) { s += ((this->operator[](i))*_weight + '0'); }				
				return s + "]";
				}


			/**
			* Serialise/deserialize. Works with boost and with the custom serialization classes
			* OArchive and IArchive. the method performs both serialization and deserialization.
			**/
			template<typename U> void serialize(U & Archive, const int version = 0)
				{
				Archive & _weight;
				Archive & _nup;
				Archive & _root;
				Archive & _vec;
				}


		private:

			int _weight;			// weight of the ups
			int _nup;				// number of ups. total length is 
			int _root;				// position of the root
			std::vector<char> _vec;	// the word itself



			/** Reroots the word, finding one of the minimum */
			inline void reroot()
				{
				int x = 0;
				int min_x = 0; 
				int min_index = 0;
				const int l = (int)_vec.size();
				for (int i = 0; i < l; i++)
					{
					x += ((_vec[i] == 0) ? -1 : _weight);
					if (x < min_x) { min_x = x; min_index = i; }
					}
				_root = ((min_index + 1) % l);
				}


		};



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
		 * The number of edges of the tree is dw.nbedges().
		 * 
		 * if weigth = 1, the number of edges is also equal to dw.nups(). 
		 *
		 * if (weigth > 1) then
		 *    - the number of non-leaf vertices is dw.nups()+1
		 *    - the number of inner edges is dw.nups()
		 **/
		CombinatorialMap(const DyckWord & dw) 
			{
			const int n = dw.nbedges();
			_sigma.reserve(2*(n+1));	// make it faster to add an edge later on. 
			_alpha.reserve(2*(n+1));	// when performing Poulalhon-Schaeffer bijection
			_sigma.resize(2*n);
			_alpha.resize(2*n);
			const int nbuds = dw.weight() - 1;
			_root = 0; // rooted at the first edge
			std::vector<int> st;
			st.reserve((int)(sqrt(dw.nups())) + 1);
			// match the half-edges. 
			if (nbuds == 0) 
				{ // general tree
				for (int i = 0; i < 2*n; i++) 
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
				_alpha[0] = 2*n - 1;
				_alpha[2*n - 1] = 0;
				buds_passed.push_back(1); // passed one bud at height 0
				for (int i = 0; i < dw.length()-1; i++) 
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
			// construct sigma going around the exterior face
			for (int i = 0; i < (2*n); i++)  { _sigma[i] = (_alpha[i] + 1) % (2*n); }
			}


		/**
		 * Number of edges of the graph
		 **/
		int nbedges() const  { return (int)_alpha.size()/2; }

		/**
		 * Converts this object to a graph.
		 **/
		std::vector<std::vector<int> > toGraph() 
			{
			int nbv;
			std::vector<int> vert = findVertices(nbv);	// start vertices of the half edges
			std::vector<std::vector<int> > gr;
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
		 * Convert the graph from a B tree (ie where all the non-leaf vertices have exactly 2 leaf
		 * neighbours) to a triangulation.
		 * 
		 * If n is the number of inner edges (ie connecting non-leaf vertices) in the tree , then the
		 * resulting triangulation has n+3 vertices.
		 * 
		 * Algorithm from Poulalhon & Schaeffer. 
		 * Optimal Coding and Sampling of Triangulations
		 * Algorithmica (2006) 46: 505.
		 * 
		 * Adaptated from Laurent Ménard implementation :-)
		 *
		 * put in a,b,c the index of the oriented root face (oriented root edge is a)
		 **/
		void btreeToTriangulation(int & A, int & B, int & C)
			{
			const int ne  = (int)_alpha.size() /2;	// number of edges
			const int nv = (ne - 2) / 3 + 1;    // number of inner vertices. 

			std::list< std::pair<int,int> > buds; // position of the buds and number of inner edges following them

			for(int i = 0; i < 2*ne; i++) { if (_sigma[i] == i) { buds.push_back({ i,0 }); } } // find the positions of the buds
			MTOOLS_INSURE(buds.size() == nv*2);
			auto it = buds.begin();
			for (int i = 0; i < nv*2 - 1; i++) 
				{
				auto pit = it; ++it;
				pit->second = it->first - pit->first - 2;
				}
			auto nit = it; nit++;
			MTOOLS_INSURE( nit == buds.end());
			it->second = (2*ne) - it->first - 2;

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
			_sigma.resize(2*ne + 2);
			_alpha.resize(2*ne + 2);
			_alpha[2*ne] = 2*ne + 1;
			_alpha[2*ne + 1] = 2*ne;
			// close the first half
			_sigma[itA2->first] = 2*ne;
			_sigma[2*ne] = itB->first;
			it = itA2;
			while (it != itB)
				{
				auto pit = it;
				it++;
				_sigma[it->first] = pit->first;
				}
			//close the second half
			_sigma[itB2->first] = 2*ne + 1;
			_sigma[2*ne + 1] = itA->first;
			it = itB2;
			while (it != itA)
				{
				auto pit = it;
				it++;
				if (it == buds.end()) it = buds.begin();
				_sigma[it->first] = pit->first;
				}

			A = 2 * ne + 1;
			B = _sigma[_alpha[A]];
			C = _sigma[_alpha[B]];
			return;
			}


		/**
		* Print the word into a string
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



		private:


			/* swap indexes i1 and i2 without modifiying the graph */
			void swapIndexes(int i1,int i2)
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
				if (sr1 == lr1) { _sigma[i2] = i2; } else { _sigma[i2] = sr1; _sigma[lr1] = i2; }
				if (sr2 == lr2) { _sigma[i1] = i1; } else { _sigma[i1] = sr2; _sigma[lr2] = i1; }
				}




			std::vector<int> _alpha;	// involution that matches half edges
			std::vector<int> _sigma;	// rotations around vertices
			int _root;					// root half-edge

		};







	}


/* end of file  */


