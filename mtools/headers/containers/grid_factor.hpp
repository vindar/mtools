/** @file grid_factor.hpp */
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


#include "misc/misc.hpp"
#include "maths/vec.hpp"
#include "misc/metaprog.hpp"
#include "io/serialization.hpp"
#include "internals_grid.hpp"

#include <type_traits>
#include <string>
#include <fstream>
#include <list>
#include <vector>

namespace mtools
{





/* Leaf Factor object */
template<size_t D, typename T, size_t R, int64 MIN_SPECIALVALUE, int64 MAX_SPECIALVALUE> struct _leafFactor : public _box < D, T, R >
    {
    static const size_t NB_SPECIAL = (size_t)(MAX_SPECIALVALUE - MIN_SPECIALVALUE + 1); // number of special value objects. 

    _leaf() {}
    ~_leaf() {};

    T  data[metaprog::power<(2 * R + 1), D>::value];           // the elements of an elementary box
    size_t count[NB_SPECIAL];                                  // number of special element of each type. 

                                                               /* return true if the point belong to this box, false otherwise */
    inline bool isInBox(const Pos & pos) { for (size_t i = 0; i < D; ++i) { int64 u = (pos[i] - center[i]); if ((u >(int64)R) || (u < -((int64)R))) { return false; } } return true; }

    /* return a reference to the object pointed by pos (no safe check) */
    inline T & get(const Pos & pos) { size_t off = 0, A = 1; for (size_t i = 0; i < D; ++i) { off += (size_t)((pos[i] - center[i] + R)*A); A *= (2 * R + 1); } return(data[off]); }

    /* set the value of the object pointed by pos (no safe check) using the assignment operator
    * if the leaf becomes full, return the associated index between 0 and NB_SPECIAL-1, otherwise, return NB_SPECIAL */
    inline T & set(const Pos & pos, const T & obj)
        {
        size_t off = 0, A = 1;
        for (size_t i = 0; i < D; ++i) { off += (size_t)((pos[i] - center[i] + R)*A); A *= (2 * R + 1); }
        int oldv = (int64)(data[off]); // old value
        int newv = (int64)(obj);       // new value
        data[off] = obj; // store the object
        if ((oldv >= MIN_SPECIALVALUE) && (oldv <= MAX_SPECIALVALUE)) { count[(size_t)(oldv - MIN_SPECIALVALUE)]--; }
        if ((newv >= MIN_SPECIALVALUE) && (newv <= MAX_SPECIALVALUE))
            {
            size_t nb = ++(count[(size_t)(newv - MIN_SPECIALVALUE)]);
            if (nb == metaprog::power<(2 * R + 1), D>::value) return((size_t)(newv - MIN_SPECIALVALUE)); // yep, we are full
            }
        return NB_SPECIAL; // not full
        }

    /* update the count[] array by recomputing the value for each object
    * If the box contain a single value, return it otherwise return NB_SPECIAL */
    inline size_t computeCount()
        {
        memset(count, 0, sizeof(count));
        for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)
            {
            int64 v = (int64)(data[i]); if ((v >= MIN_SPECIALVALUE) && (v <= MAX_SPECIALVALUE)) { count[(size_t)(v - MIN_SPECIALVALUE)]++; }
            }
        int64 w = (int64)(data[0]); // just need to test if full for the value any pos. 
        if ((w >= MIN_SPECIALVALUE) && (w <= MAX_SPECIALVALUE))
            {
            size_t n = (size_t)(w - MIN_SPECIALVALUE); if (count[n] == metaprog::power<(2 * R + 1), D>::value) return n; // yes, we are full 
            }
        return(NB_SPECIAL); // not full.
        }


    private:
        //            _leafFactor(const _leafFactor &) = delete;                // no copy
        //            _leafFactor & operator=(const _leafFactor &) = delete;    //
    };







    /**
     * A D-dimensional grid containing objects of type T.
     * 
     * Version with factorization: there are "special object" which are unique.
     * 
     * DUPLICATES: each site of Z^d contain an object but the special objets appear only once and
     * all the sites containing the same special object are, in fact, sharing the same reference.
     * 
     * GUARANTEE: every object which is NOT a special object is unique and is never deleted, moved
     * around or copied during the whole life of the grid. Thus, pointer to non-special elements are
     * never invalidated. On the other hand, special objects may be created, deleted and multiple
     * sites with the same special objet will point to the same reference. Every destructor is
     * called when the object is detroyed or when reset() is called.  
     *
     * The objet T must satisfy the following properties:
     * 
     * | Requir.  | Property.                                                          
     * |----------|--------------------------------------------------------------------
     * | REQUIRED | Constructor `T()` or `T(const Pos &)`. If the positional constructor exist it is used instead of `T()`.                                        
     * | REQUIRED | Copy constructor `T(const T&)`. [The assignement `operator=()` is not needed].
     * | OPTIONAL | Comparison `operator==()`. If T is comparable with `==`, then this operator is used for testing whether an object is special. Otherwise, basic memcmp() comparison is used.                      
     * | OPTIONAL | Serialization via `serialize(OArchive &)` and `deserialize(IArchive &)` or constructor `T(IArchive &)`. Otherwise, basic memcpy is used when saving to/loading from file. 
     * 
     * @tparam  D   Dimension of the grid (e.g. Z^D).
     *              
     * @tparam  T   Type of objects stored in the sites of the grid. Can be any type satisfying the requirment above.
     *              
     * @tparam  R   Radius of an elementary box i.e. each elementary box contain (2R+1)^D objects.
	 *              | Dimension | default value for R
	 *              |-----------|-------------------
	 *              | 1         | 10000
	 *              | 2         | 100
	 *              | 3         | 20
	 *              | 4         | 6
	 *              | >= 5      | 1
     *              
     * @sa Grid_basic, Grid_static
     **/
    template < size_t D, typename T, int MIN_SPECIALVALUE = 0, int MAX_SPECIALVALUE = 127, size_t R = internals_grid::defaultR<D>::val> class Grid_factor
    {

        static const size_t NB_SPECIAL = (size_t)(MAX_SPECIALVALUE - MIN_SPECIALVALUE + 1); // number of special value objects. 
    
        typedef internals_grid::_box<D, T, R> *     _pbox;
        typedef internals_grid::_node<D, T, R> *    _pnode;
        typedef internals_grid::_leaf<D, T, R> *    _pleaf;

    public:

        /**
        * Alias for a D-dimensional int64 vector representing a position in the grid.
        **/
        typedef iVec<D> Pos;

        /**
        * Constructor. Create an empty grid (no objet of type T is created).
        **/
        Grid_factor() : _pcurrent(nullptr), _rangemin(1), _rangemax(-1) { memset(_tabSpecObj, 0, sizeof(_tabSpecObj)); _createBaseNode(); }


        /**
        * Constructor. Loads a grid from a file. If the loading fails, the grid is constructed empty.
        *
        * @param   filename    Filename of the file.
        **/
        Grid_factor(const std::string & filename) : _pcurrent(nullptr), _rangemin(1), _rangemax(-1) { memset(_tabSpecObj, 0, sizeof(_tabSpecObj)); load(filename); }


        /**
        * Destructor. Destroys the grid. The destructors of all the T objects in the grid are invoqued.
        * In order to prevent calling the dtors of T objects, call `reset(false)` prior to destructing
        * the grid.
        **/
        ~Grid_factor() { _destroyTree(true); }


        /**
        * Copy Constructor. Makes a Deep copy of the grid.
        **/
        Grid_factor(const Grid_basic<D, T, R> & G) : _pcurrent(nullptr), _rangemin(1), _rangemax(-1) { memset(_tabSpecObj, 0, sizeof(_tabSpecObj)); this->operator=(G); }


        /**
         * Assignement operator. Create a deep copy of G. If the grid is not empty, it is first reset :
         * all the object inside are destroyed and their dtors are invoqued.
         **/
        // TODO, can use different grid with different special values !!!!!!
        Grid_factor<D, T, R> & operator=(const Grid_factor<D, T,MIN_SPECIALVALUE,MAX_SPECIALVALUE,R> & G)
            {
            if ((&G) == this) { return(*this); }
            _destroyTree(true);
            _rangemin = G._rangemin;
            _rangemax = G._rangemax;
            _pcurrent = _copy(G._getRoot(), nullptr);
            return(*this);
            }


        /**
        * Resets the grid.
        *
        * @param   callObjDtor [default true]. True to call the destructors of the T objects in the
        *                      grid. Set this to false to release all the memory without calling the
        *                      objects destructors ~T().
        **/
        void reset(bool callObjDtor = true) { _destroyTree(callDtors); _createBaseNode(); }


    private:

        static_assert(NB_SPECIAL > 0, "the number of special obects must be >0, use Grid_basic otherwise.");
        static_assert(D > 0, "template parameter D (dimension) must be non-zero");
        static_assert(R > 0, "template parameter R (radius) must be non-zero");
        static_assert(std::is_constructible<T>::value || std::is_constructible<T, Pos>::value, "The object T must either be default constructible T() or constructible with T(const Pos &)");
        static_assert(std::is_copy_constructible<T>::value, "The object T must be copy constructible T(const T&).");



        /* create the base node which centers the tree around the origin */
        inline void _createBaseNode()
          {
            MTOOLS_ASSERT(_pcurrent == nullptr);   // should only be called when the tree dos not exist.
            _pcurrent = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { ((_pnode)_pcurrent)->tab[i] = nullptr; }
            _pcurrent->center = Pos(0);
            _pcurrent->rad = R;
            _pcurrent->father = nullptr;
            return;
            }

        /* Release all the allocated  memory and reset the tree */
        void _destroyTree(bool callObjDtor)
            {
            if (callObjDtor)
                {
                _poolSpecObj.destroyAll();
                _poolNode.destroyAll();
                _poolLeaf.destroyAll();
                }
            else
                {
                _poolSpecObj.deallocateAll();
                _poolNode.deallocateAll();
                _poolLeaf.deallocateAll();
                }
            memset(_tabSpecObj, 0, sizeof(_tabSpecObj));
            _pcurrent = nullptr;
            _rangemin.clear((int64)1);
            _rangemax.clear((int64)-1);
            return;
            }



        /* return the offset of the special object if there is one or NB_SPECIAL otherwise */ 
        size_t isSpecialNode(internals_grid::_node<D, T, R> * p)
            {
            auto off = p - _dummyNodes;
            if ((off >= 0) && (off < NB_SPECIAL)) { return((size_t)off); }
            return NB_SPECIAL;
            }

        /* return the special object of a given index, return nullptr if it does not exist */
        inline T * getSpecialObj(size_t index)
            {
            MTOOLS_ASSERT((index >= 0) && (index < NB_SPECIAL));
            return _tabSpecObj[index];
            }

        /* set the value of a special object using the copy ctor */
        inline void setSpecialObj(T & obj)
            {
            int64 v = (int64)obj; // value of the object
            MTOOLS_ASSERT((v >= MIN_SPECIALVALUE) && (v <= MAX_SPECIALVALUE));
            size_t n = (size_t)(v - MIN_SPECIALVALUE);
            MTOOLS_ASSERT(_tabSpecObj[n] == nullptr);
            T * p = _poolSpecObj.allocate();
            new(p) T(obj); // construction via the copy ctor
            _tabSpecObj[n] = p; // save the adress of the special object
            }


        static const internals_grid::_node<D, T, R> _dummyNodes[NB_SPECIAL]; // dummy node which are used to indicate special objects. 
        T* _tabSpecObj[NB_SPECIAL];                                          // array of pointers to the special objects.

        mutable _pbox _pcurrent;                // pointer to the current box
        mutable Pos   _rangemin;                // the minimal range
        mutable Pos   _rangemax;                // the maximal range

        mutable SingleAllocator<internals_grid::_leafFactor<D, T, R>,200 >  _poolLeaf; // pool for leaf objects
        mutable SingleAllocator<internals_grid::_node<D, T, R>,200 >  _poolNode;       // pool for node objects
        mutable SingleAllocator< T, NB_SPECIAL >  _poolSpecObj;                    // pool for special objects 

    };


}


/* end of file */

