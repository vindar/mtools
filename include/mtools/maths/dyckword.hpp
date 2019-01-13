/** @file dyckword.hpp */
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


namespace mtools
	{


	/**
	* Class representing a dyck word with a given weight for the ups.
	*
	* weight = 1 <-> encode a planar tree with nup edges 
	* weight > 1 <-> encode a planar tree where with nup interior edges (ie edges
	*                between non-leaf neighbour). Each non-leaf neighobur has exactly   
	*                weight-1 leaf neighbours.
	*                
	* When weight > 1, there are weight adlissible rooting of the word that
	* satisfy the prefix condition.
	*
	* NOTE: the case weight > 1 encode the set of tree considered by Poulalhon and Schaeffer                
	*       for their bijection with simple planar triangulation. 
	**/
	class DyckWord
		{

		public:

			/** default ctor. Empty dyck word with 1 ups and weight 1.  
			 *  The corresponding tree is reduced to a single edge.
			 **/
			DyckWord() : _weight(1), _nup(1), _root(0), _vec(3, 0)
				{
				_vec[0] = 1;
				}


			/** ctor. Construct a simple simple dyck word of a given lenght and weight.  
			 * All the ups are first, followed by all the down.
			 **/
			DyckWord(int nup, int weight = 1) : _weight(weight), _nup(nup), _root(0)
				{
				MTOOLS_ASSERT(weight > 0);
				MTOOLS_ASSERT(nup >= 0);
				_vec.resize((_weight == 1) ? (2 * _nup + 1) : ((1 + _weight)*_nup + (_weight - 1)));
				for (int i = 0; i < _nup; i++) { _vec[i] = 1; }
				}


			/**
			 * Shuffles the world uniformly. 
			 * 
			 * If weight > 1, there are weight possible choices that make a legal word.
			 *   
			 * @param	upminimum	true to choose a rooting such that the word start with 
			 * 						an up. False to choose any rooting possible rooting uniformly
			 * 						among the weight possible ones (there is only one legal rooting
			 * 						when weight = 1).  
			 **/
			template<typename random_t> void shuffle(random_t gen,bool upminimum = true)
				{
				randomShuffle(_vec, gen); // shuffle the word.
				reroot(); // find a minimum that always start with an up. 
				if ((_weight == 1) || (upminimum)) return; // done
				// choose another rooting uniformly among all other.
				const int l = (int)_vec.size();
				int mx = -((int)(Unif(gen)*_weight)); // there are _weight choices
				if (mx == 0) return;
				int x = 0;
				for (int i = 0; i < l; i++)
					{
					x += ((this->operator[](i) == 0) ? -1 : _weight);
					if (x == mx) { _root = ((_root + i + 1) % l); return; }
					}
				MTOOLS_ERROR("should not be possible...");
				}


			/** Access a Dyck word letter (circular). **/
			const char & operator[](int i) const
				{
				const int l = (int)_vec.size();
				const int p = i + _root;
				return _vec[(p%l) + ((p<0) ? l : 0)];
				}


			/**
			* Total length of the word. This method count the downward 'marker' step.
			* weight = 1: this is 2*nup + 1
			* weight > 1: this is (1 + weight)*nup + (weight - 1) [the word is rooted at a bud 
			*             so the word ends when the RW reaches -(weigth-1)]
			**/
			inline int length() const { return (int)_vec.size(); }


			/**
			* return the total number of edges of the tree encoded by this word.
			* weight = 1: this is just the number of ups.
			* weight > 1: this is nup*weight + weight - 1
			**/
			inline int nbedges() const { return (int)(_nup*_weight) + (_weight - 1); }


			/**
			* Return the weight associated with this word.
			* weight = 1: encode a regular tree  
			* weight > 1: encode a tree where each vertex which is NOT a leaf has
			*             exactly weight-1 neighbour which are leaves. 
			**/
			inline int weight() const { return _weight; }


			/**
			* Return the number of ups of this word. 
			* If weight = 1, this correspond to the total number of edges in the encoded tree
			* If weight > 1, this correspond to the number of interior edges (those connecting 
			*                non-leaf vertices) in the encoded tree.
			**/
			inline int nups() const { return _nup; }


			/**
			* Print the word into a string
			**/
			std::string toString() const
				{
				OSS os; 
				os << "[";
				for (int i = 0; i < (int)_vec.size(); i++) { os <<  ((this->operator[](i))*_weight + '0'); }
				os << "]";
				return os.str();
				}


			/**
			* Serialise/deserialize. Works with boost and with the custom serialization classes
			* OBaseArchive and IBaseArchive. the method performs both serialization and deserialization.
			**/
			template<typename U> void serialize(U & Archive, const int version = 0)
				{
				Archive & _weight;
				Archive & _nup;
				Archive & _root;
				Archive & _vec;
				}


		private:


			/**
			 * Reroots the word such that it satisfies the prefix condition but
			 * also that it starts with an up. 
			 **/
			inline void reroot()
				{
				if (_nup == 0) { _root = 0; return; }
				const int l = (int)_vec.size();
				for (int i = 0; i < l; i++) { if (_vec[i] != 0) { _root = i; } } // choose root such that the word start with an up
				int x = 0, min_x = 0, min_index = 0;
				for (int i = 0; i < l; i++) // find the absolute minimum.
					{
					x += ((this->operator[](i) == 0) ? -1 : _weight);
					if (x < min_x) { min_x = x; min_index = i; }
					}
				_root = ((_root + min_index + 1) % l); // reroot, if the min is in the interior of the interval, it must start again with an up...
				}


			int _weight;			// weight of the ups
			int _nup;				// number of ups. 
			int _root;				// position of the root
			std::vector<char> _vec;	// the word itself

		};



	}

/* end of file */

