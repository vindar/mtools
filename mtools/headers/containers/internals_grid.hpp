/** @file internals_grid.hpp */
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


#include "misc/metaprog.hpp"
#include "misc/memory.hpp"

namespace mtools
{

    namespace internals_grid
    {

        /* forward declaration */
        template<size_t D, typename T, size_t R> struct _box;
        template<size_t D, typename T, size_t R> struct _node;
        template<size_t D, typename T, size_t R> struct _leaf;
        template<size_t D, typename T, size_t NB_SPECIAL, size_t R > struct _leafFactor;


        /* Box object */
        template<size_t D, typename T, size_t R> struct _box
        {
            _box() {}
            ~_box() = default;

            typedef iVec<D>             Pos;
            typedef _box<D, T, R> *     _pbox;
            typedef _leaf<D, T, R> *    _pleaf;
            typedef _node<D, T, R> *    _pnode;

            Pos             center;         // position of the center of this box
            uint64          rad;            // radius of a subbox (1 if the object is a leaf and > 1) otherwise
            _pbox           father;         // pointer to the father, null if at the root

            /* return true is this object is a leaf*/
            inline bool isLeaf() const { return (rad == 1); }

        private:
            _box(const _box &) = delete;                // no copy
            _box & operator=(const _box &) = delete;    //
        };



        /* Node object */
        template<size_t D, typename T, size_t R> struct _node : public _box < D, T, R >
        {
            _node() {};
            ~_node() {};

            typedef iVec<D>             Pos;
            typedef _box<D, T, R> *     _pbox;
            typedef _leaf<D, T, R> *    _pleaf;
            typedef _node<D, T, R> *    _pnode;

            _pbox tab[metaprog::power<3,D>::value];    // the array of pointer to the sub-boxes

            /* return true if the point belong to this box, false otherwise */
            inline bool isInBox(const Pos & pos) const { int64 l = (3 * this->rad + 1); for (size_t i = 0; i < D; ++i) { int64 u = pos[i] - this->center[i];  if ((u > l) || (u < -l)) { return false; } } return true; }

            /* return a reference to the element in tab containing the pointer to the subbox containing the point pos (no boundary check) */
            inline _pbox & getSubBox(const Pos & pos)
                {
                size_t m = 1, r = 0;
                for (size_t i = 0; i < D; ++i) { int64 a = (pos[i] - this->center[i]); r += ((a < -(((int64)this->rad))) ? 0 : ((a >((int64)this->rad)) ? 2 : 1))*m; m *= 3; }
                return(tab[r]);
                }

            /* compute the center of a sub-box containing a given point */
            inline Pos subBoxCenter(const Pos & pos) const
                {
                Pos subcenter;
                for (size_t i = 0; i < D; i++) { int64 a = (pos[i] - this->center[i]); subcenter[i] = this->center[i] + ((a < -((int64)this->rad)) ? (-((int64)(2 * this->rad + 1))) : ((a >((int64)this->rad)) ? (2 * this->rad + 1) : 0)); }
                return subcenter;
                }

            /* compute the center of a sub-box given its index in the tab array */
            inline Pos subBoxCenterFromIndex(size_t j) const
                {
                Pos subcenter;
                for (size_t i = 0; i < D; i++) { size_t k = (j % 3); subcenter[i] = this->center[i] + ((k == 0) ? (-((int64)(2 * this->rad + 1))) : ((k == 1) ? 0 : ((int64)(2 * this->rad + 1)))); j /= 3; }
                return subcenter;
                }


        private:
            _node(const _node &) = delete;                // no copy
            _node & operator=(const _node &) = delete;    //

        };



        /* Leaf object */
        template<size_t D, typename T, size_t R> struct _leaf : public _box < D, T, R >
        {

            typedef iVec<D>             Pos;
            typedef _box<D, T, R> *     _pbox;
            typedef _leaf<D, T, R> *    _pleaf;
            typedef _node<D, T, R> *    _pnode;


            _leaf() {}
            ~_leaf() {};

            T  data[metaprog::power<(2*R+1),D>::value];           // the elements of an elementary box

            /* return true if the point belong to this box, false otherwise */
            inline bool isInBox(const Pos & pos) { for (size_t i = 0; i < D; ++i) { int64 u = (pos[i] - this->center[i]); if ((u >(int64)R) || (u < -((int64)R))) { return false; } } return true; }

            /* return a reference to the object pointed by pos (no safe check) */
            inline T & get(const Pos & pos) { size_t off = 0, A = 1; for (size_t i = 0; i < D; ++i) { off += (size_t)((pos[i] - this->center[i] + R)*A); A *= (2 * R + 1); } return(data[off]); }

        private:
            _leaf(const _leaf &) = delete;                // no copy
            _leaf & operator=(const _leaf &) = delete;    //
        };



        /* Leaf Factor object  */
        template<size_t D, typename T, size_t NB_SPECIAL, size_t R> struct _leafFactor : public _leaf< D, T, R>
        {
            _leafFactor() {}
            ~_leafFactor() {};

            size_t count[NB_SPECIAL]; // number of special element of each type

        private:
            _leafFactor(const _leafFactor &) = delete;                // no copy
            _leafFactor & operator=(const _leafFactor &) = delete;    //
        };





        /* the default value for the radius of an elementary sub-grid */
        template<size_t D> struct defaultR { static const size_t val = ((D == 1) ? 10000 : ((D == 2) ? 100 : ((D == 3) ? 20 : ((D == 4) ? 6 : ((D == 5) ? 3 : 1))))); };



    }
}


/* end of file */
