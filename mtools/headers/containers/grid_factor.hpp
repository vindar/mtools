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

    /* set the value of the object pointed by pos (no safe check) using the assignment operator
    * if the leaf becomes full, return the associated index between 0 and NB_SPECIAL-1, otherwise, return NB_SPECIAL */
    /*
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
        */
    /* update the count[] array by recomputing the value for each object
    * If the box contain a single value, return it otherwise return NB_SPECIAL */
    /*
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
    */






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
    template < size_t D, typename T, size_t R = internals_grid::defaultR<D>::val, size_t NB_SPECIAL = 256 > class Grid_factor
    {

        typedef internals_grid::_box<D, T, R> *     _pbox;
        typedef internals_grid::_node<D, T, R> *    _pnode;
        typedef internals_grid::_leafFactor<D, T, R, NB_SPECIAL> *    _pleafFactor;

    public:

        /**
        * Alias for a D-dimensional int64 vector representing a position in the grid.
        **/
        typedef iVec<D> Pos;


        /**
         * Constructor. An empty grid (no objet of type T is created).
         *
         * @param   minSpecial  The minimum value of the special objects.
         * @param   maxSpecial  The maximum value of the special objects.
         * @param   callDtors   true to call the destructors of the T objects when destroyed. 
         **/
        Grid_factor(int64 minSpecial, int64 maxSpecial, bool callDtors)
            { 
            _reset(minSpecial, maxSpecial, callDtors);
            }


        /**
        * Constructor. Loads a grid from a file. If the loading fails, the grid is constructed empty.
        *
        * @param   filename    Filename of the file.
        **/
        Grid_factor(const std::string & filename)
            { 
            _reset(0, 0, true);
            load(filename);
            }


        /**
        * Destructor. Destroys the grid. The destructors of all the T objects in the grid are invoqued.
        * In order to prevent calling the dtors of T objects, call `reset(false)` prior to destructing
        * the grid.
        **/
        ~Grid_factor() { _reset(); }


        /**
        * Copy Constructor. Makes a Deep copy of the grid. The class T must be copyable by the copy
        * operator `T(const T&)`.
        **/
        template<size_t NB_SPECIAL2> Grid_factor(const Grid_factor<D, T, R, NB_SPECIAL2> & G)
            {
            _reset(0, 0, true);
            this->operator=(G);
            }


        /**
        * Assignement operator. Create a deep copy of G. The class T must be copyable by the copy
        * operator T(const T&). The assignement operator `operator=(const T &)` is never used and need
        * not be defined. If the grid is not empty, it is first reset : all the object inside are
        * destroyed and their dtors are invoqued.
        **/
        template<size_t NB_SPECIAL2> Grid_factor<D, T, R, NB_SPECIAL> & operator=(const Grid_factor<D, T, R, NB_SPECIAL2> & G)
            {
            static_assert((NB_SPECIAL2 <= NB_SPECIAL), "The number of NB_SPECIAL of special object of the source must be smaller than than of the destination");
            if ((&G) == this) { return(*this); }
            _reset(G._minSpec,G._maxSpec,G._callDtors);
            // *********************** TODO ************************
            return(*this);
            }


        /**
        * Resets the grid. Keep the previous minSpecial, maxSpecial et callDtor parameters. 
        **/
        void reset()
            {
            _reset();
            _createBaseNode();
            }


        /**
         * Resets the grid and change the range of the special values.
         *
         * @param   minSpecial  the new minimum value for the special parameters.
         * @param   maxSpecial  the new maximum value for the special parameters.
         * @param   callDtors   true to call dthe destructors of the object when they are destroyed. 
         **/
        void reset(int64 minSpecial, int64 maxSpecial, bool callDtors = true)
            {
            _reset(minSpecial,maxSpecial,callDtors);
            _createBaseNode();
            }



        //void serialize(OArchive & ar)
        // 
        //void deserialize(IArchive & ar)

        //bool save(const std::string & filename) const
        
        //bool load(std::string filename)
        


        /**
        * Returns a string with some information concerning the object.
        *
        * @param   debug   Set this flag to true to enable the debug mode where the whole tree structure
        *                  of the lattice is written into the string [should not be used for large grids].
        *
        * @return  an info string.
        **/
        std::string toString(bool debug = false) const
            {
            std::string s;
            s += std::string("Grid_factor<") + mtools::toString(D) + " , " + typeid(T).name() + " , " + mtools::toString(R) + " , " + mtools::toString(NB_SPECIAL) + ">\n";
            s += std::string(" - Memory used : ") + mtools::toString((_poolLeaf.footprint() + _poolNode.footprint() + _poolSpec.footprint()) / (1024 * 1024)) + "MB\n";
            s += std::string(" - Range min = ") + _rangemin.toString(false) + "\n";
            s += std::string(" - Range max = ") + _rangemax.toString(false) + "\n";
            s += std::string(" - Special object range [") + mtools::toString(_minSpec) + " , " + mtools::toString(_maxSpec) + "]\n";
            int64 
            for (int i = 0;i < (_maxSpec - _minSpec + 1); i++)
                {
                s += std::string(" - special (") + mtools::toString(_minSpec + i) + ") = " + mtools::toString(_tabSpecNB[i]) + "\n";
                }

            s += std::string(" - Number of 'normal' objects = ") + mtools::toString(_nbNormalObj) + "\n";
            if (debug) {s += "\n" + _printTree(_getRoot(),"");}
            return s;
            }



    private:

        static_assert(NB_SPECIAL > 0, "the number of special objects must be > 0, use Grid_basic otherwise.");
        static_assert(D > 0, "template parameter D (dimension) must be non-zero");
        static_assert(R > 0, "template parameter R (radius) must be non-zero");
        static_assert(std::is_constructible<T>::value || std::is_constructible<T, Pos>::value, "The object T must either be default constructible T() or constructible with T(const Pos &)");
        static_assert(std::is_copy_constructible<T>::value, "The object T must be copy constructible T(const T&).");








        /* create the data for a leafFactor using the positional constructor.
         * set the ->data and ->count fields in the leaf and update the globbl counter for special objects. 
         * return _maxSpec+1 if the leaf is not composed of a single special object 
         * return the number of the special object otherwise */
        inline int64 _createDataLeaf(internals_grid::_leafFactor<D, T, R, NB_SPECIAL> * pleaf, Pos pos, metaprog::dummy<true> dum) const
            {
            Pos center = pos; // save the center position
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            for (size_t i = 0; i < D; ++i) { pos[i] -= R; } // go to the first cell
            int64 off = -1; // offset of the last special object created. 
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
            {
                new(pleaf->data + x) T(pos); // create from positionnal constructor
                int64 val = (int64)(*(data + x)); // convert to int64
                if ((val >= _minSpec) && (val <= _maxSpec)) // check if it is a special value
                    { // yes
                    off = val - _minSpec;                // the offset of the special object
                    (pleaf->count[off])++;               // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;                 // increase the global counter about the number of these special objects 
                    }
                else {_nbNormalObj++;} // increase the global counter for the number of normal object
                for (size_t i = 0; i < D; ++i) { if (pos[i] < (center[i] + (int64)R)) { pos[i]++;  break; } pos[i] -= (2 * R); } // move to the next cell.
                }
            if ((off < 0) || (_tabSpecNB[off] < metaprog::power<(2 * R + 1), D>::value)) return (_maxSpec + 1); // no all object are the same special object 
            return(off + _minSpec); // yes, all the elements correspond to the same special object. 
            }


        /* create the data for a leafFactor using the default constructor.
        * set the ->data and ->count fields in the leaf and update the globbl counter for special objects.
        * return _maxSpec+1 if the leaf is not composed of a single special object
        * return the number of the special object otherwise */
        inline int64 _createDataLeaf(T * data, Pos pos, metaprog::dummy<false> dum) const
            { 
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            int64 off = -1; // offset of the last special object created. 
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)
                {                 
                new(data + i) T(); 
                int64 val = (int64)(*(data + i)); // convert to int64
                if ((val >= _minSpec) && (val <= _maxSpec)) // check if it is a special value
                    { // yes
                    off = val - _minSpec;                // the offset of the special object
                    (pleaf->count[off])++;               // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;                 // increase the global counter about the number of these special objects 
                    }
                else { _nbNormalObj++; } // increase the global counter for the number of normal object
                }
            if ((off < 0)|| (_tabSpecNB[off] < metaprog::power<(2 * R + 1), D>::value)) return (_maxSpec + 1); // no all object are the same special object 
            return(off + _minSpec); // yes, all the elements correspond to the same special object. 
            }



        /* Create a leaf at a given position. Can possibly return a dummy node. */
        inline _pbox _allocateLeaf(_pbox above, const Pos & centerpos) const
            {
            _pleafFactor p = _poolLeaf.allocate(); // allocate the memory
            int v = _createDataLeaf(p->data, centerpos, metaprog::dummy<std::is_constructible<T, Pos>::value>()); // create the data
            if (v > _maxSpec) // check if the leaf contain only a single type of special object
                { // no, we keep the leaf as it is. 
                p->center = centerpos;
                p->rad = 1;
                p->father = above;
                return p;
                }
            // yes
            auto pdummy = _setSpecial(v, p->data); // save the special object if needed and then get the associated dummy node
            if (_callDtors) _poolLeaf.destroy(p); else _poolLeaf.deallocate(p); // delete the leaf eventually calling the destructors
            return pdummy;
            }


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


        /* Allocate a node, call constructor from above */
        inline _pnode _allocateNode(_pbox above, const Pos & centerpos, _pbox fill) const
            {
            MTOOLS_ASSERT(above->rad > R);
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = fill; }
            p->center = centerpos;
            p->rad = (above->rad - 1) / 3;
            p->father = above;
            return p;
            }




        /* create a node, call constructor from below */
        inline _pnode _allocateNode(_pbox below) const
            {
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = nullptr; }
            p->tab[(metaprog::power<3, D>::value - 1) / 2] = below;
            p->center = below->center;
            p->rad = ((below->rad == 1) ? R : (below->rad * 3 + 1));
            p->father = nullptr;
            return p;
            }



        /* Reset the object and change the min and max values for the special objects
        * and the call dtor flag */
        void _reset(int64 minVal, int64 maxVal, bool callDtors)
            {
            MTOOLS_ASSERT(minVal <= maxVal);
            MTOOLS_ASSERT(maxVal - minVal < NB_SPECIAL);
            _reset();
            _minSpec = minVal;
            _maxSpec = maxVal;
            _callDtors = callDtors;
            }


        /* Reset the object */
        void _reset()
            {
            if (_callDtors)
                {
                _poolNode.destroyAll();
                _poolLeaf.destroyAll();
                _poolSpec.destroyAll();
                }
            else
                {
                _poolNode.deallocateAll();
                _poolLeaf.deallocateAll();
                _poolSpec.deallocateAll();
                }
            _pcurrent = nullptr;
            _rangemin.clear((int64)1);
            _rangemax.clear((int64)-1);

            memset(_tabSpecObj, 0, sizeof(_tabSpecObj));
            memset(_tabSpecNB, 0, sizeof(_tabSpecNB));
            _nbNormalObj = 0;
            return;
            }


        mutable _pbox _pcurrent;                // pointer to the current box
        mutable Pos   _rangemin;                // the minimal accessed range
        mutable Pos   _rangemax;                // the maximal accessed range

        mutable SingleAllocator<internals_grid::_leafFactor<D, T, R, NB_SPECIAL>,200>  _poolLeaf;   // pool for leaf objects
        mutable SingleAllocator<internals_grid::_node<D, T, R>,200 >  _poolNode;                    // pool for node objects
        mutable SingleAllocator<T, NB_SPECIAL + 1 >  _poolSpec;                                     // pool for special objects

        T* _tabSpecObj[NB_SPECIAL];                                                                 // array of T pointers to the special objects.
        mutable uint64 _tabSpecNB[NB_SPECIAL];                                                      // total number of special objects of each type. 
        mutable uint64 _nbNormalObj;                                                                // number of objects which are not special

        static const internals_grid::_node<D, T, R> _dummyNodes[NB_SPECIAL];                        // dummy nodes array used solely to indicate special objects. 
        int64 _minSpec,_maxSpec;                                                                    // min and max value of special objects
        bool _callDtors;                                                                            // should we call the dtors of T objects. 





        /* get a pointer to a special object with a given value, 
           return nullptr if it does not exist */
        inline T * _getSpecialObject(int64 value)
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
            return _tabSpecObj[value - _minSpec];
            }


        /* get a pointer to a special object associated with a special node
           return nullptr if the node is not special */
        inline T * _getSpecialObject(internals_grid::_box<D, T, R> * p)
            {
            auto off = p - _dummyNodes;
            if ((off >= 0) && (off < NB_SPECIAL))
                { // yes, this is a special object
                MTOOLS_ASSERT(_tabSpecObj[off] != nullptr); // make sure the special object was previously created.
                return _tabSpecObj[off];
                }
            return nullptr;
            }


        /* return the adress of the special node associated with a special object */
        inline internals_grid::_box<D, T, R> * _getSpecialNode(int64 value)
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
            return _dummyNodes + (value - _minSpec);
            }


        /* set a special object. Does nothing if already set.
           Return the associated dummy node */
        inline internals_grid::_box<D, T, R> * _setSpecial(int64 value, T* obj)
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
            MTOOLS_ASSERT(((int64)(*obj)) == value);
            const auto off = value - _minSpec;
            T* & p = _tabSpecObj[off];
            if (p == nullptr)
                {
                p = _poolSpec.allocate(); // get a place to store the special element
                new(p) T(*obj); // use copy placement new
                }
            return _dummyNodes + off;
            }



    };


}


/* end of file */

