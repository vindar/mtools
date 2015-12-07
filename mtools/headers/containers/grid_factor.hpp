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


#include "../misc/misc.hpp"
#include "../maths/vec.hpp"
#include "../maths/rect.hpp"
#include "../misc/metaprog.hpp"
#include "../io/serialization.hpp"
#include "internals_grid.hpp"

#include <type_traits>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <atomic>
#include <mutex>

namespace mtools
{


    /* forward declaration of the Grid_basic class*/
    template< size_t D, typename T, size_t R> class Grid_basic;


    /**
     * A D-dimensional grid containing objects of type T. This is a container similar to a D-
     * dimensionnal 'octree'. This is the 'factor' version where 'special objects' can be factorized
     * to gain space. See Grid_basic for a simpler container with weaker requirement on the type T.
     * 
     * - The type T must be convertible to int64 (long long).  
     * 
     * - There are two types of objects : 'normal' and 'special' object. An object is said to be
     * special if its int64 value belong to a certain (possible empty) range [minSpec, maxSpec].
     * Special object have the property that they can be factorized: many sites containing the same
     * special object (ie with same value) may, in fact, share the same reference to the object.
     * These allows to save some space. So it is advisable that every instance object which convert
     * to the same special value should be equivalent. RQ: even though special valued object may be
     * factorized, not everyone of them share the same reference in general so there is usually
     * multiple instance of the same special object (special object are factorized when a leaf in
     * the octree become full with all its element sharing the same special value, in this case, the
     * whole leaf is replaced by a ponter to a single special object).
     * 
     * - On the other hand, it is guaranteed that 'normal' object i.e. those whose converted int64
     * value does not belong to the special range [minSpec,maxSpec] are uniquely associated to a
     * site and are never copied, moved Thus, pointer to non-special elements are never invalidated.
     * 
     * 
     * 
     * - The special range can be modified on the fly (this induces a factorization which can be
     * time consumming) or even disabled by setting an empty range. In this case, the grid obtained
     * is perfectly compatible with Grid_basic and conversion back and forth between these two types
     * of grid is possible.
     * 
     * 
     * - the class T must also satisfy the following properties:  
     *     - Constructible via `T()` or `T(const Pos &)`. If the positional ctor exist it is
     *     preferred.  
     *     - Copy constructible with `T(const T&)` and assignable with `operator=(const T&)`.
     * 
     * - The grid can be serialized to file using the I/OArchive serialization classes and using the
     * default serialization method for T. Furthermore, it is possible to deserialize the object via
     * a specific constructor `T(IArchive &)` (this prevent having to construct a default object
     * first and then deserialize it).
     * 
     * - Grid_factor objects are compatible with Grid_factor objects (with the same template
     * parameters). Files saved with one object can be open with the other one and conversion using
     * copy construtor and assignement operators are implemented in both directions (provided, of
     * course, that the object T fullfills the requirement of Grid_factor and, conversely, that the
     * grid_factor objects do not have special objects at the time of conversion).
     *
     * @tparam  D           Dimension of the grid (e.g. Z^D).
     * @tparam  T           Type of objects stored in the sites of the grid. Can be any type
     *                      satisfying the requirments above.
     * @tparam  NB_SPECIAL  The maximum number of special object. This is simply an upper bound which
     *                      need not be matched.
     * @tparam  D           Type of the d.
     * @tparam  R           Radius of an elementary box i.e. each elementary box contain (2R+1)^D
     *                      objects.
     *                      | Dimension | default value for R
     *                      |-----------|-------------------
     *                      | 1         | 10000
     *                      | 2         | 100
     *                      | 3         | 20
     *                      | 4         | 6
     *                      | >= 5      | 1.
     *
     * @sa  Grid_basic
     **/
    template < size_t D, typename T, size_t NB_SPECIAL = 256 , size_t R = internals_grid::defaultR<D>::val > class Grid_factor
    {

        template<size_t D2, typename T2, size_t NB_SPECIAL2, size_t R2> friend class Grid_factor;
        template<size_t D2, typename T2, size_t R2> friend class Grid_basic;


        typedef internals_grid::_box<D, T, R> *     _pbox;
        typedef internals_grid::_node<D, T, R> *    _pnode;
        typedef internals_grid::_leafFactor<D, T, NB_SPECIAL, R> *    _pleafFactor;

    public:

        /**
        * Alias for a D-dimensional int64 vector representing a position in the grid.
        **/
        typedef iVec<D> Pos;


        /**
         * Constructor. Construct an empty grid (no objet of type T is created yet). Set minSpecial \>
         * maxSpecial (default) to disable special values.
         *
         * @param   minSpecial  The minimum value of the special objects.
         * @param   maxSpecial  The maximum value of the special objects.
         * @param   callDtors   true to call the destructors of the T objects when destroyed.
         **/
        Grid_factor(int64 minSpecial = 0, int64 maxSpecial = -1, bool callDtors = true) : _psafeT(nullptr)
            {
            reset(minSpecial, maxSpecial, callDtors);
            }


        /**
         * Constructor. Loads a grid from a file. If the loading fails, the grid is constructed empty.
         *
         * @param   filename    Filename of the file.
         **/
        Grid_factor(const std::string & filename) : _psafeT(nullptr)
            { 
            reset(0,-1,true);
            load(filename);
            }


        /**
         * Destructor. Destroys the grid. The destructors of all the T objects in the grid are invoqued
         * if the callDtor flag is set and are dropped into oblivion otherwise.
         **/
        ~Grid_factor() { _reset(); if (_callDtors) { delete _psafeT; } }


        /**
         * Copy Constructor. Create a deep copy of the source, with the same special values and the same
         * statistics (bounding box and min/max values) and the same flag for calling the destructors.
         * 
         * The source must have the same D,T,R template parameters. The template parameter NB_SPECIAL
         * need not be the same but must be large enough to hold all the special values (which is the
         * case if it is larger or equal to NB_SPECIAL2 of the source).
         *
         * @tparam  NB_SPECIAL2 the template paramter for the max number of special object of the source.
         * @param   G   the source Grid_factor to copy.
         **/
        template<size_t NB_SPECIAL2> Grid_factor(const Grid_factor<D, T, NB_SPECIAL2, R> & G) : _psafeT(nullptr)
            {
            MTOOLS_INSURE(((!G._existSpecial()) || (G._specialRange()<= NB_SPECIAL))); // make sure we can hold all the special element of the source.
            reset(0, -1, true);
            this->operator=(G);
            }


        /**
        * Copy Constructor. The resulting grid is exactly the same as the source, with the same special
        * values and the same statistics (bounding box and min/max values) and the same flag for
        * calling the destructors.
        *
        * Version with the same template paramter NB_SPECIAL.
        * 
        * @param   G   the source Grid_factor to copy.
        **/
        Grid_factor(const Grid_factor<D, T, NB_SPECIAL, R> & G) : _psafeT(nullptr)
            {
            reset(0, -1, true);
            this->operator=(G);
            }


        /**
         * Copy Constructor. construct the object from a grid_basic object. The special object range is
         * set to [-1,0] (i.e. empty) so that there is no factorization.
         *
         * @param   G   The basic_grid to process.
         **/
        Grid_factor(const Grid_basic<D, T, R> & G) : _psafeT(nullptr)
            {
            reset(0, -1, true);
            this->operator=(G);
            }


        /**
         * Assignement operator. Create a deep copy of G.  The resulting grid is exactly the same as the
         * source, with the same special value and the same statistics (bounding box and min max value)
         * and the same flag for calling the destructors.
         * 
         * 
         * The source must have the same D,T,R template paramters. The template parameter NB_SPECIAL
         * must be large enough to hold all the special values (which is the case if it is larger or
         * equal to NB_SPECIAL of the source).
         * 
         * If the grid is not empty, it is first reset() and the dtor of current object are called
         * depending on the status of the callDtor flag.
         *
         * @tparam  NB_SPECIAL2 the template paramter for the max number of special object of the source.
         * @param   G   the source grid.
         *
         * @return  the object for chaining.
         **/
        template<size_t NB_SPECIAL2> Grid_factor<D, T, NB_SPECIAL, R> & operator=(const Grid_factor<D, T, NB_SPECIAL2, R> & G)
            {
            std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
            MTOOLS_INSURE(((!G._existSpecial()) || (G._specialRange() <= NB_SPECIAL))); // make sure we can hold all the special element of the source.
            const void * p1 = (const void *)(&G);
            const void * p2 = (const void *)(this);
            if (p1 == p2) { return(*this); } // nothing to do
            _reset(G._minSpec,G._maxSpec,G._callDtors); // reset the grid and set the special range and dtor flag
            _rangemin = G._rangemin;    // same statistics
            _rangemax = G._rangemax;    //
            _minVal = G._minVal;        //
            _maxVal = G._maxVal;        //
            // copy the special objects. 
            for (int64 i = 0; i < _specialRange(); i++)
                {
                _tabSpecNB[i] = G._tabSpecNB[i];
                if (G._tabSpecObj[i] != nullptr)
                    {
                    _tabSpecObj[i] = _poolSpec.allocate();
                    new(_tabSpecObj[i]) T(*(G._tabSpecObj[i]));
                    }
                }
            _nbNormalObj = G._nbNormalObj;
            // copy the whole tree structure
            _pcurrent = _copyTree<NB_SPECIAL2>(nullptr, G._getRoot(), G);
            _pcurrentpeek = (_pbox)_pcurrent;
            return(*this);
            }


        /**
         * Assignement operator. Create a deep copy of G.  The resulting grid is exactly the same as the
         * source, with the same special value and the same statistics (bounding box and min max value)
         * and the same flag for calling the destructors.
         * 
         * If the grid is not empty, it is first reset() and the dtor of current object are called
         * depending on the status of the callDtor flag.
         * 
         * Version with the same template paramter NB_SPECIAL.
         *
         * @param   G   the source grid.
         *
         * @return  the object for chaining.
         **/
        Grid_factor<D, T, NB_SPECIAL, R> & operator=(const Grid_factor<D, T, NB_SPECIAL, R> & G)
            {
            return operator=<NB_SPECIAL>(G);
            }


        /**
         * Assignment operator. Create a copy of a basic_grid object into this Grid_factor object. The
         * special value range is set to [-1,0] (ie empty) so there is no factorization.
         * 
         * If the grid is not empty, it is first reset() and the dtor of current objects are called
         * depending on the status of the callDtor flag.
         *
         * @param   G   The basic_grid to copy.
         *
         * @return  the object for chaining.
         **/
        Grid_factor<D, T, NB_SPECIAL, R> & operator=(const Grid_basic<D, T, R> & G); // definition below to prevent circular inclusion


        /**
         * Saves the grid into a file (using the archive format). The file is compressed if it ends by
         * the extension ".gz", ".gzip" or ".z".
         * 
         * Grid file for grid_Factor and Grid_basic are compatible provided the template parameters D,R
         * and T are the same (NB_SPECIAL may differ but must always be large enough to accomodate the
         * range of special object). In particular, if the Grid_factor is saved without special objects
         * (by calling for exemple removeSpecialObjects() prior to saving it) then it may subsequently be
         * loaded by a Grid_basic object.
         *
         * @param   filename    The filename.
         *
         * @return  true if it succeeds, false if it fails.
         **/
        bool save(const std::string & filename) const
            {
            try
                {
                OArchive ar(filename);
                ar & (*this); // use the serialize method.
                }
            catch (...) 
                {
                MTOOLS_DEBUG("Error saving Grid_factor object");
                return false; 
                } // error
            return true; // ok
            }


        /**
         * Loads the given file. The file may have been created by saving either a Grid_basic or a
         * Grid_factor object with same template parameter T, D, R.
         * 
         * If the grid is non-empty, it is first reset, possibly calling the dtor depending on the
         * status of the callDtor flag.
         * 
         * The template paramter NB_SPECIAL may differ from that of the object used when saving the
         * grid. It must simply be large enough to hold the special value range of the grid described in
         * the file.
         *
         * @param   filename    The filename to load.
         *
         * @return  true if it succeeds, false if it fails.
         **/
        bool load(const std::string & filename)
            {
            try
                {
                IArchive ar(filename);
                ar & (*this); // use the deserialize method.
                }
            catch (...) 
                {
                MTOOLS_DEBUG("Error loading Grid_factor object");
                reset(0, -1, true); 
                return false; 
                } // error
            return true; // ok
            }


        /**
        * Serializes the grid into an OArchive. If T implement a serialize method recognized by
        * OArchive, it is used for serialization otherwise OArchive uses the default serialization
        * method which correspond to a basic memcpy() of the object memory
        *
        * @param [in,out]  ar  The archive object to serialise the grid into.
        *
        * @sa  deserialize, class OArchive, class IArchive
        **/
        void serialize(OArchive & ar) const
            {
            ar << "\nBegining of Grid_factor<" << D << " , [" << std::string(typeid(T).name()) << "] , " << NB_SPECIAL << " , " << R << ">\n";
            ar << "Version";    ar & ((uint64)1); ar.newline();
            ar << "Template D"; ar & ((uint64)D); ar.newline();
            ar << "Template R"; ar & ((uint64)R); ar.newline();
            ar << "object T";   ar & std::string(typeid(T).name()); ar.newline();
            ar << "sizeof(T)";  ar & ((uint64)sizeof(T)); ar.newline();
            ar << "call dtors"; ar & (_callDtors); ar.newline();
            ar << "_rangemin";  ar & _rangemin; ar.newline();
            ar << "_rangemax";  ar & _rangemax; ar.newline();
            ar << "_minSpec";   ar & _minSpec; ar.newline();
            ar << "_maxSpec";   ar & _maxSpec; ar.newline();
            ar << "List of special objects\n";
            for (int64 i = 0; i < _specialRange(); i++)
                {
                ar << "Object (" << _minSpec + i << ")";
                if (_tabSpecObj[i] == nullptr) { ar & false; } else { ar & true; ar & (*_tabSpecObj[i]); }
                ar.newline();
                }
            ar << "Grid tree\n";
            _serializeTree(ar, _getRoot());
            ar << "\nEnd of Grid_factor<" << D << " , [" << std::string(typeid(T).name()) << "] , " << NB_SPECIAL << " , " << R << ">\n";
            }


        /**
         * Deserializes the grid from an IArchive. If T has a constructor of the form T(IArchive &), it
         * is used for deserializing the T objects in the grid. Otherwise, if T implements one of the
         * serialize methods recognized by IArchive, the objects in the grid are first position/default
         * constructed and then deserialized using those methods. If no specific deserialization method
         * is found, IArchive falls back on its default deserialization method. memcpy().
         * 
         * If the grid is non-empty, it is first reset, possibly calling the ctor of the existing T
         * objects depending on the status of the callCtor flag.
         *
         * @param [in,out]  ar  The archive to deserialize the grid from.
         *
         * @sa  serialize, class OArchive, class IArchive
         **/
        void deserialize(IArchive & ar)
            {
            try
                {
                std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
                _reset(-1, 0, true); // reset the object, do not create the root node
                uint64 ver;         ar & ver;      if (ver != 1) { MTOOLS_DEBUG("wrong version"); throw ""; }
                uint64 d;           ar & d;        if (d != D) { MTOOLS_DEBUG("wrong dimension"); throw ""; }
                uint64 r;           ar & r;        if (r != R) { MTOOLS_DEBUG("wrong R parameter"); throw ""; }
                std::string stype;  ar & stype;
                uint64 sizeofT;     ar & sizeofT;  if (sizeofT != sizeof(T)) { MTOOLS_DEBUG("wrong sizeof(T)"); throw ""; }
                ar & _callDtors;
                ar & _rangemin;
                ar & _rangemax;
                ar & _minSpec;
                ar & _maxSpec;
                if (((int64)NB_SPECIAL) < _specialRange()) { MTOOLS_DEBUG("NB_SPECIAL too small to fit all special values"); throw ""; }
                for (int64 i = 0; i < _specialRange(); i++)
                    {
                    bool b; ar & b;
                    if (b)
                        {
                        _tabSpecObj[i] = _poolSpec.allocate();      // allocate the memory
                        _deserializeObjectT(ar, _tabSpecObj[i]);    // deserialize the object
                        MTOOLS_ASSERT((((int64)(*(_tabSpecObj[i]))) == (i + _minSpec)));
                        }
                    }
                _pcurrent = _deserializeTree(ar, nullptr);
                _pcurrentpeek = (_pbox)_pcurrent;
                }
            catch (...)
                {
                callDtors(false); // prevent calling the destructor of object when we release memory (since we do not know whch one may be in an invalid state)
                reset(0, -1, true); // put the object in a valid state
                MTOOLS_DEBUG("Aborting deserialization of Grid_factor object");
                throw; // rethrow
                }
            }


        /**
         * Change the range of special objects. This method first expand the whole tree structure to
         * remove all factorization of the tree then set the new range for the special objects and
         * subsequently re-factorizes the tree using the new paramters.
         * 
         * This method can be time and memory consumming
         * 
         * Set newMaxSpec \< newMinSpec to disable special object and expand the whole tree (or simply call the `removeSpecialObjects()` method).
         *
         * @param   newMinSpec  the new minimum value for the special objects range.
         * @param   newMaxSpec  the new maximum value for the special object range.
         **/
        void changeSpecialRange(int64 newMinSpec, int64 newMaxSpec)
            {
            std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
            MTOOLS_INSURE(((newMaxSpec < newMinSpec) || ((newMaxSpec - newMinSpec) < ((int64)NB_SPECIAL))));
            _expandTree(); // we expand the whole tree, removing every dummy links
            if (_callDtors) _poolSpec.destroyAll(); else _poolSpec.deallocateAll(); // release the memory for all the special objects saved
            memset(_tabSpecObj, 0, sizeof(_tabSpecObj)); // clear the list of pointer to special objects
            memset(_tabSpecNB, 0, sizeof(_tabSpecNB));   // clear the count for special objects
            _nbNormalObj = 0; // reset the number of normal objects
            _minSpec = newMinSpec; // set the new min value for the special objects
            _maxSpec = newMaxSpec; // set the new max value for the special objects
            _recountTree(); // recount all the leafs with the new special object range
            if (!_existSpecial()) return; // no need to simplify since there are no special objects
            _simplifyTree(); // simplify the tree using the new special objects
            }


        /**
         * Removes the special objects. This makes the grid compatible with Grid_basic object
         * 
         *  Same as 'changeSpecialRange(0, -1)'.
         **/
        void removeSpecialObjects()
            {
            changeSpecialRange(0, -1);
            }


        /**
         * Make sure the object is in a valid factorized state and recompute the statistic about the T
         * object inside.
         * 
         * This method must be called after a direct modification of the internal state of the object
         * using the access() method.
         * 
         * If acess() is not used, the method is useless and should not be called as it can be time
         * consumming...
         **/
        void simplify() const
            {   
            std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
            if (!_existSpecial()) return; // nothing to do if there is no special objects
            memset(_tabSpecNB, 0, sizeof(_tabSpecNB));   // clear the count for special objects
            _nbNormalObj = 0; // reset the number of normal objects
            _recountTree(); // recount all the leafs with the new special object range
            _simplifyTree(); // simplify the tree using the new special objects
            }


        /**
         * Resets the grid, possibly calling the destructor of the T object depending on the status of
         * callDtor. Keep the current values for the minSpecial, maxSpecial et callDtor parameters.
         **/
        void reset()
            {
            std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
            _reset();
            _createBaseNode();
            }


        /**
         * Resets the grid and modify the range of the special values and the callDtor flag. Set
         * newMaxSpec \< newMinSpec to disable special object.
         * 
         * the modification to the calldtors flag is made AFTER destruction i.e. the previous status is
         * used when releasing memory. `Call callDtor(status)` prior to calling reset to modify the way
         * object are released with this method.
         *
         * @param   minSpecial  the new minimum value for the special parameters.
         * @param   maxSpecial  the new maximum value for the special parameters.
         * @param   callDtors   true to call the destructors of the object when they are destroyed.
         **/
        void reset(int64 minSpecial, int64 maxSpecial, bool callDtors = true)
            {
            std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
            _reset(minSpecial,maxSpecial,callDtors);
            _createBaseNode();
            }


        /**
         * Check if we currently call the destructor of object when they are not needed anymore.
         *
         * @return  true if we call the dtors and false otherwise.
         **/
        bool callDtors() const { return _callDtors; }


        /**
         * Set whether we should, from now on, call the destructor of object when they are not needed
         * anymore.
         *
         * @param   callDtor    true to call the destructors.
         **/
        void callDtors(bool callDtor) { _callDtors = callDtor; }


        /**
         * Lower bound of the special value range.
         *
         * @return  the minimum value of a special object.
         **/
        int64 minSpecialValue() const { return _minSpec; }


        /**
         * Upper bound of the special value range
         *
         * @return  the maximum value of a special object
         **/
        int64 maxSpecialValue() const { return _maxSpec; }


        /**
         * Return a bounding box containing the set of all positions accessed either by get() or set()
         * methods (peek does not count). The method returns maxpos \< minpos if no element have ever 
         * been accessed.
         *
         * @param [in,out]  minpos  a vector with the minimal coord. in each direction.
         * @param [in,out]  maxpos  a vector with the maximal coord. in each direction.
         **/
        inline void getPosRange(Pos & minpos, Pos & maxpos) const { minpos = _rangemin; maxpos = _rangemax; }


        /**
         * Return a bounding rectangle for the position of all the elements accessed either by get()
         * or set() methods (peek does not count). The rectangle is empty if no elements have ever been
         * accessed/created.
         * 
         * This method is specific for dimension D = 2.
         *
         * @return  an iRect containing the spacial range of element accessed.
         **/
        inline iRect getPosRangeiRect() const 
            {
            static_assert(D == 2, "getRangeiRect() method can only be used when the dimension template parameter D is 2");
            return mtools::iRect(_rangemin.X(), _rangemax.X(), _rangemin.Y(), _rangemax.Y());
            }


        /**
         * The all time (since the last reset) minimum value of all elements ever created/set in the
         * grid (obtained by casting the element into int64). This value may be different from the min
         * value of all elements really accessed since other elements can be silently created.
         *
         * @return  The minimum value (converted as int64) of all the elements ever created in the grid.
         **/
        inline int64 minValue() const { return _minVal; }


        /**
         * The all time (since the last reset) maximum value of all element ever created/set in the grid
         * (obtained by casting the element into int64). This value may be different from the max value
         * of all elements really accessed since other elements can be silently created.
         *
         * @return  The maximum value (converted as int64) of all the elements ever created in the grid.
         **/
        inline int64 maxValue() const { return _minVal; }


        /**
         * Sets the value at a given site. If the value at the site does not exist prior to the call, it
         * is first created and then the assignement is performed with `T::operator=()`.
         *
         * @param   pos The position of the site to access.
         * @param   val The value to set.
         **/
        inline void set(const Pos & pos, const T & val) { _set(pos,&val); }


        /**
         * Set a value at a given position. Dimension 1 specialization.
         **/
        inline void set(int64 x, const T & val) { static_assert(D == 1, "template parameter D must be 1"); return _set(Pos(x),&val); }


        /**
        * Set a value at a given position. Dimension 1 specialization.
        **/
        inline void set(int64 x, int64 y, const T & val) { static_assert(D == 2, "template parameter D must be 2"); return _set(Pos(x,y), &val); }


        /**
        * Set a value at a given position. Dimension 1 specialization.
        **/
        inline void set(int64 x, int64 y, int64 z, const T & val) { static_assert(D == 3, "template parameter D must be 3"); return _set(Pos(x,y,z), &val); }


        /**
        * Get the value at a given position (the value is created if needed). 
        *
        * @param   pos The position.
        *
        * @return  A const reference to the value at that position.
        **/
        inline const T & get(const Pos & pos) const { return _get(pos); }


        /**
        * Get the value at a given position (creates it if it does not exist). Dimension 1 specialization.
        **/
        inline const T & get(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }


        /**
        * Get the value at a given position (creates it if it does not exist). Dimension 2 specialization.
        **/
        inline const T & get(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
        * Get the value at a given position (creates it if it does not exist). Dimension 3 specialization.
        **/
        inline const T & get(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
        * get a value at a given position. Same as the get() method.
        * This creates the object if it does not exist yet.
        **/
        inline const T & operator[](const Pos & pos) const { return _get(pos); }


        /**
        * get a value at a given position. Same as the get() method.
        * This creates the object if it does not exist yet.
        **/
        inline const T & operator()(const Pos & pos) const { return _get(pos); }


        /**
        * get a value at a given position. Dimension 1 specialization.
        * This creates the object if it does not exist yet.
        **/
        inline const T & operator()(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }


        /**
        * get a value at a given position. Dimension 2 specialization.
        * This creates the object if it does not exist yet.
        **/
        inline const T & operator()(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
        * get a value at a given position. Dimension 3 specialization.
        * This creates the object if it does not exist yet.
        **/
        inline const T & operator()(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
         * Access the element at a given position. If the element does not exist, it is created first.
         * The reference returned is non const hence may be modified.
         * 
         * @warning NEVER EVER MODIFY THE VALUE OF A SPECIAL ELEMENT AS IT MAY BE SHARED BY MANY OTHER
         * SITES (STILL PROBABLY NOT ALL OF THEM EITHER). It is safe to change the value of a 'normal'
         * element to that of another 'normal' element. It is also possible to change the value of a
         * normal element to that of a special element but in this case you should call 'simplify()'
         * afterward to insure that the whole grid goes back in its fully factorized state. Otherwise,
         * anything can happen... Use this method at your own risk !
         *
         * @param   pos The position to access.
         *
         * @return  A reference to the element at position pos.
         **/
        inline T & access(const Pos & pos) { return _get(pos); }


        /**
         * Return a pointer to the object at a given position. If the value at the site was not yet
         * created, returns nullptr. This method does not modify the object at all and is particularly
         * suited when drawing the lattice using, for instance, the LatticeDrawer class.
         * 
         * @warning Contrarily to Grid_Basic::peek(), this method is NOT threadsafe ! It must not be used
         *          while concurrently accessing the object. See safePeek() for a threadsafe alternative.
         *
         * @param   pos The position to peek.
         *
         * @return  nullptr if the value at that site was not yet created. A const pointer to it
         *          otherwise.
        **/
        inline const T * peek(const Pos & pos) const
            {
            _pbox cp = _pcurrentpeek;
            if (((_pbox)cp) == ((_pbox)nullptr)) return nullptr;
            // check if we are at the right place
            if (cp->isLeaf())
                {
                _pleafFactor p = (_pleafFactor)(cp);
                MTOOLS_ASSERT(_isLeafFull(p) == (_maxSpec + 1)); // the leaf cannot be full
                if (p->isInBox(pos)) { return(&(p->get(pos))); }
                MTOOLS_ASSERT(cp->father != nullptr); // a leaf must always have a father
                cp = p->father;
                }
            // no, going up...
            _pnode q = (_pnode)(cp);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { _pcurrentpeek = q; return nullptr; }
                q = (_pnode)q->father;
                }
            // and down..
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) { _pcurrentpeek = q; return nullptr; }
                T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                if (obj != nullptr) { _pcurrentpeek = q; return obj; }
                if (b->isLeaf()) 
                    { 
                    _pcurrentpeek = b; 
                    return(&(((_pleafFactor)b)->get(pos))); 
                    }
                q = (_pnode)b;
                }
            }


        /**
         * Return a pointer to the object at a given position. If the value at the site was not yet
         * created, returns nullptr. This method does not modify the object at all and is particularly
         * suited when drawing the lattice using, for instance, the LatticeDrawer class.
         * 
         * This version is partially threadsafe: it is possible to call safePeek() while reading or
         * writing or modifiying the object. However, there must be no concurrent access with peek()
         * or safePeek() (all other public methods are ok). 
         * 
         * The pointer returned is copied in a special portion of memory and is safe from any
         * modification until safePeek() is called again.
         * 
         * Implementation of the method uses a lock and a copy so it is slower that the peek()
         * method. However, it does not affect the speed of the get/set operation (it slows it down only
         * when factorization takes place, but this take much more time than the safePeek() method
         * anyway).
         *
         * @param   pos The position to peek.
         *
         * @return  nullptr if the value at that site was not yet created. A const pointer to it
         *          otherwise.
        **/
        inline const T * safePeek(const Pos & pos) const
            {
            std::lock_guard<std::recursive_mutex> lock(_peekmut); // acquire the lock
            _pbox cp = _pcurrentpeek;
            if (((_pbox)cp) == ((_pbox)nullptr)) return nullptr;
            // check if we are at the right place
            if (cp->isLeaf())
                {
                _pleafFactor p = (_pleafFactor)(cp);
                MTOOLS_ASSERT(_isLeafFull(p) == (_maxSpec + 1)); // the leaf cannot be full
                if (p->isInBox(pos)) { return _copySafeT(&(p->get(pos))); }
                MTOOLS_ASSERT(cp->father != nullptr); // a leaf must always have a father
                cp = p->father;
                }
            // no, going up...
            _pnode q = (_pnode)(cp);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { _pcurrentpeek = q; return nullptr; }
                q = (_pnode)q->father;
                }
            // and down..
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) { _pcurrentpeek = q; return nullptr; }
                T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                if (obj != nullptr) { _pcurrentpeek = q; return _copySafeT(obj); }
                if (b->isLeaf()) { _pcurrentpeek = b; return _copySafeT(&(((_pleafFactor)b)->get(pos))); }
                q = (_pnode)b;
                }
            }


        /**
        * peek at a value at a given position. Dimension 1 specialization.
        * Returns nullptr if the object at this position is not created.
        **/
        inline const T * peek(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return peek(Pos(x)); }


        /**
        * Safe peek at a value at a given position. Dimension 1 specialization.
        * Returns nullptr if the object at this position is not created.
        **/
        inline const T * safePeek(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return safePeek(Pos(x)); }


        /**
        * peek at a value at a given position. Dimension 2 specialization.
        * Returns nullptr if the object at this position is not created.
        **/
        inline const T * peek(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return peek(Pos(x, y)); }


        /**
        * Safe peek at a value at a given position. Dimension 2 specialization.
        * Returns nullptr if the object at this position is not created.
        **/
        inline const T * safePeek(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return safePeek(Pos(x, y)); }


        /**
        * peek at a value at a given position. Dimension 3 specialization.
        * Returns null if the object at this position is not created.
        **/
        inline const T * peek(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return peek(Pos(x, y, z)); }


        /**
        * Safe peek at a value at a given position. Dimension 3 specialization.
        * Returns null if the object at this position is not created.
        **/
        inline const T * safePeek(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return safePeek(Pos(x, y, z)); }


        /**
        * Return the memory currently allocated by the grid (in bytes).
        **/
        size_t memoryAllocated() const { return sizeof(*this) + _poolLeaf.footprint() + _poolNode.footprint() + _poolSpec.footprint() + ((_psafeT == nullptr) ? 0 : sizeof(T)); }


        /**
        * Return the memory currently used by the grid (in bytes).
        **/
        size_t memoryUsed() const { return sizeof(*this) + _poolLeaf.used() + _poolNode.used() + _poolSpec.used() + ((_psafeT == nullptr) ? 0 : sizeof(T)); }
        

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
            s += std::string("Grid_factor<") + mtools::toString(D) + " , " + typeid(T).name() + " , " + mtools::toString(NB_SPECIAL) + " , " + mtools::toString(R)  + ">\n";
            s += std::string(" - Memory : ") + mtools::toStringMemSize(memoryUsed()) + " / " + mtools::toStringMemSize(memoryAllocated()) + "\n";
            s += std::string(" - Min position accessed = ") + _rangemin.toString(false) + "\n";
            s += std::string(" - Max position accessed = ") + _rangemax.toString(false) + "\n";
            s += std::string(" - Min value created = ") + mtools::toString(_minVal) + "\n";
            s += std::string(" - Max value created = ") + mtools::toString(_maxVal) + "\n";
            s += std::string(" - Special object value range [") + mtools::toString(_minSpec) + " , " + mtools::toString(_maxSpec) + "]";     
            uint64 totE = 0;
            if (!_existSpecial()) { s += std::string(" NONE!\n"); } else { s += std::string("\n"); }
            for (int i = 0;i < _specialRange(); i++)
                {
                s += std::string("    [") + ((_tabSpecObj[i] == nullptr) ? " " : "X") + "] value (" + mtools::toString(_minSpec + i) + ") = " + mtools::toString(_tabSpecNB[i]) + "\n";
                totE += _tabSpecNB[i];
                }
            s += std::string(" - Number of 'normal' objects = ") + mtools::toString(_nbNormalObj) + "\n";
            totE += _nbNormalObj;
            s += std::string(" - Total number of objects = ") + mtools::toString(totE) + "\n";
            if (debug) {s += "\n" + _printTree(_getRoot(),"");}
            return s;
            }


        /**
         * Find a box containing position pos such that all the points inside the (closed) box have the
         * same special value (or are all undefined). The coordinates of the box are put in boxMin and
         * boxMax and the function return a pointer to the commun value (or nullptr if no value defined
         * on this box)
         * 
         * If no such box can be found, the method sets boxMin = boxMax = pos (but still return a 
         * pointer to the value et pos).
         * 
         * The box returned is always a square and correspond to the largest full box containing pos
         * in the "quadtree-like" grid structure.
         * 
         * Although the box returned always contain position pos, it can be close (or even on) the
         * boundary of the box which may be undesired. To get another box where pos is further away 
         * from the boundary, use findFullBoxCentered() instead.
         * 
         * @warning This method is NOT threadsafe and uses the same pointer as get() and set().
         *
         * @param   pos             The position to check.
         * @param [in,out]  boxMin  Vector to put the minimum value of the box in each direction.
         * @param [in,out]  boxMax  Vector to put the maximum value of the box in each direction.
         *
         * @return  A pointer to the element at position pos or nullptr if it does not exist.
         **/
        inline const T * findFullBox(const Pos & pos, Pos & boxMin, Pos & boxMax) const
            {
            _pbox cp = _pcurrent;
            MTOOLS_ASSERT(cp != nullptr);
            // check if we are at the right place
            if (cp->isLeaf())
                {
                _pleafFactor p = (_pleafFactor)(cp);
                MTOOLS_ASSERT(_isLeafFull(p) == (_maxSpec + 1)); // the leaf cannot be full
                if (p->isInBox(pos)) { boxMin = pos; boxMax = pos; return(&(p->get(pos))); } // just a singleton, box = [pos,pos]^d
                MTOOLS_ASSERT(cp->father != nullptr); // a leaf must always have a father
                cp = p->father;
                }
            // no, going up...
            _pnode q = (_pnode)(cp);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) 
                    { // the point is outside of largest boundary box
                    int64 r = 3*q->rad + 1; 
                    for (size_t i = 0; i < D; ++i)
                        {
                        int64 u = pos[i]; if (u < 0) {u = -u;}
                        while(u > r) { r = 3*r + 1; }
                        }
                    // r is the radius of the box containing pos
                    r = (r - 1) / 3;
                    for (size_t i = 0; i < D; i++) 
                        { 
                        const int64 a = pos[i]; 
                        const int64 sb =  ((a < -r) ? (-(2*r + 1)) : ((a > r) ? (2*r + 1) : 0)); 
                        boxMin[i] = sb - r; boxMax[i] = sb + r; // TODO, we could find bigger if we just want a rectangle and not a square...
                        }
                    _pcurrent = q; 
                    return nullptr; 
                    } 
                q = (_pnode)q->father;
                }
            // and down..
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) 
                    { // subbox does not exist yet
                    const int64 rad = q->rad;
                    boxMin = q->subBoxCenter(pos);
                    boxMax = boxMin;
                    boxMin -= rad;
                    boxMax += rad;
                    _pcurrent = q; 
                    return nullptr; 
                    }
                T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                if (obj != nullptr) 
                        { // good we have full box
                        const int64 rad = q->rad;
                        boxMin = q->subBoxCenter(pos);
                        boxMax = boxMin;
                        boxMin -= rad; 
                        boxMax += rad;
                        _pcurrent = q;
                        return obj;
                        }
                if (b->isLeaf())
                    {
                    boxMin = pos; boxMax = pos;
                    _pcurrent = b;
                    return(&(((_pleafFactor)b)->get(pos))); // just a singleton
                    }
                q = (_pnode)b;
                }
            }


        /**
         *  Specialization for dimension 2 using an iRect structure.
         * 
         *  @warning This method is NOT threadsafe and uses the same pointer as get() and set().
         *
         * @param   pos         The position to check.
         * @param [in,out]  r   The iRect to put the rectangle found.
         *
         * @return  A pointer to the element at position pos or nullptr if it does not exist.
         **/
        inline const T * findFullBox(const Pos & pos, iRect & r) const
            {
            static_assert(D == 2, "findFullBox(Pos,iRect) method can only be used when the dimension template parameter D is 2");
            Pos boxmin, boxmax;
            const T * res = findFullBox(pos, boxmin, boxmax);
            r.xmin = boxmin[0]; r.xmax = boxmax[0];
            r.ymin = boxmin[1]; r.ymax = boxmax[1];
            return res;
            }


        /**
        * Find a box containing position pos such that all the point inside the (closed) box have the
        * same special value (or are all undefined). The coordinate of the box are put in boxMin and
        * boxMax and the function return a pointer to the commun value (or nullptr if no value defined
        * on this box)
        *
        * If no such box can be found (for instance if the value at pos is defined but not special)
        * then the function sets boxMin = boxMax = pos (and returns the value at pos).
        *
        * Compared to findFullBox(), this method tries to find a box for which pos is near the center
        * of the box (i.e. the distance to the boundary is larger):
        * 
        * - The method is slower than findFullBox() but always returns a box where the distance of pos  
        * to the boundary is at least that returned by findFullBox() yet the volume of the box may be   
        * smaller.
        * - The box returned need not be a square (as in findFullBox()).
        *
        * @warning This method is NOT threadsafe and uses the same pointer as get() and set().
        *
        * @param   pos             The position to check.
        * @param [in,out]  boxMin  Vector to put the minimum value of the box in each direction.
        * @param [in,out]  boxMax  Vector to put the maximum value of the box in each direction.
        *
        * @return  A pointer to the element at position pos or nullptr if it does not exist.
        **/
        inline const T * findFullBoxCentered(const Pos & pos, Pos & boxMin, Pos & boxMax) const
            {
            // TODO. pfffffff that should be fun... 
            // until then, just return the findFullBox() without any improvement
            return findFullBox(pos, boxMin, boxMax);
            }


         /**
         *  Specialization for dimension 2 using an iRect structure.
         *
         *  @warning This method is NOT threadsafe and uses the same pointer as get() and set().
         *
         * @param   pos         The position to check.
         * @param [in,out]  r   The iRect to put the rectangle found.
         *
         * @return  A pointer to the element at position pos or nullptr if it does not exist.
         **/
        inline const T * findFullBoxCentered(const Pos & pos, iRect & bestRect) const
            {
            static_assert(D == 2, "findFullBoxCentered(Pos,iRect) method can only be used when the dimension template parameter D is 2");
            const T* pv = findFullBox(pos, bestRect); // get the non optimized box.
            if (bestRect.lx() == 0) return pv;   // no box found, nothing more to do.

            iRect baseRect = bestRect;                  // the base rectangle is the best rectangle.
            int64 lbest = bestRect.boundaryDist(pos);   // current distance to the boundary

        findFullBoxCentered_loop: // beginning of a loop at a given scale.

            const int64 lbase = baseRect.boundaryDist(pos); // distance from the boundary of the base rectangle to pos
            const int64 diambase = baseRect.lx() + 1; // diameter of the base rectangle
            if (lbase + diambase <= lbest) { return pv; } // we cannot improve the distance to the boundary by using boxes baseRect, we are done !
                                                      
            //  flags
            static const int flagBorderUp = 2;
            static const int flagBorderDown = 64;
            static const int flagBorderLeft = 8;
            static const int flagBorderRight = 16;
            static const int flagCornerUpLeft = 1;
            static const int flagCornerUpRight = 4;
            static const int flagCornerDownLeft = 32;
            static const int flagCornerDownRight = 128;
            static const int flagBorder = 2 + 64 + 8 + 16;
            static const int flagCorner = 1 + 4 + 32 + 128;
                                                         
            int flag = 0;   // flag describing which adjacent boxes are set.

            //  check which border boxes are set
            const Pos basecenter = baseRect.center();   // center of the base box            
            const Pos borderUp = Pos(basecenter.X(), basecenter.Y() + diambase);
            const Pos cornerUpLeft = Pos(basecenter.X() - diambase, basecenter.Y() + diambase);
            const Pos cornerUpRight = Pos(basecenter.X() + diambase, basecenter.Y() + diambase);
            _checkBorder(flag, diambase, pv, bestRect, borderUp, flagBorderUp, cornerUpLeft, flagCornerUpLeft, cornerUpRight, flagCornerUpRight);   // check the up border and possibly the adjacent corners.

            const Pos borderLeft = Pos(basecenter.X() - diambase, basecenter.Y());
            const Pos cornerDownLeft = Pos(basecenter.X() - diambase, basecenter.Y() - diambase);
            _checkBorder(flag, diambase, pv, bestRect, borderLeft, flagBorderLeft, cornerUpLeft, flagCornerUpLeft, cornerDownLeft, flagCornerDownLeft);  // check the left border and possibly the adjacent corners.

            const Pos borderRight = Pos(basecenter.X() + diambase, basecenter.Y());
            const Pos cornerDownRight = Pos(basecenter.X() + diambase, basecenter.Y() - diambase);
            _checkBorder(flag, diambase, pv, bestRect, borderRight, flagBorderRight, cornerUpRight, flagCornerUpRight, cornerDownRight, flagCornerDownRight); // check the right border and possibly the adjacent corners.

            const Pos borderDown = Pos(basecenter.X(), basecenter.Y() - diambase);
            _checkBorder(flag, diambase, pv, bestRect, borderDown, flagBorderDown, cornerDownLeft, flagCornerDownLeft, cornerDownRight, flagCornerDownRight); // check the down border and possibly the adjacent corners.
                        
            // possible rectangle extension
            #define box1Up iRect(baseRect.xmin, baseRect.xmax, baseRect.ymin, baseRect.ymax + diambase)
            #define box1Down iRect(baseRect.xmin, baseRect.xmax, baseRect.ymin - diambase, baseRect.ymax)
            #define box1Left iRect(baseRect.xmin - diambase, baseRect.xmax, baseRect.ymin, baseRect.ymax)
            #define box1Right iRect(baseRect.xmin, baseRect.xmax + diambase, baseRect.ymin, baseRect.ymax)
            #define line2UpDown iRect(baseRect.xmin, baseRect.xmax, baseRect.ymin - diambase, baseRect.ymax + diambase)
            #define line2LeftRight iRect(baseRect.xmin - diambase, baseRect.xmax + diambase, baseRect.ymin, baseRect.ymax)
            #define box2UpLeft iRect(baseRect.xmin - diambase, baseRect.xmax, baseRect.ymin, baseRect.ymax + diambase)
            #define box2UpRight iRect(baseRect.xmin, baseRect.xmax + diambase, baseRect.ymin, baseRect.ymax + diambase)
            #define box2DownLeft iRect(baseRect.xmin - diambase, baseRect.xmax, baseRect.ymin - diambase, baseRect.ymax)
            #define box2DownRight iRect(baseRect.xmin, baseRect.xmax + diambase, baseRect.ymin - diambase, baseRect.ymax)
            #define rect3Up iRect(baseRect.xmin - diambase, baseRect.xmax + diambase, baseRect.ymin, baseRect.ymax + diambase)
            #define rect3Down iRect(baseRect.xmin - diambase, baseRect.xmax + diambase, baseRect.ymin - diambase, baseRect.ymax)
            #define rect3Left iRect(baseRect.xmin - diambase, baseRect.xmax, baseRect.ymin - diambase, baseRect.ymax + diambase)
            #define rect3Right iRect(baseRect.xmin, baseRect.xmax + diambase, baseRect.ymin - diambase, baseRect.ymax + diambase)
            #define rect4 iRect(baseRect.xmin - diambase, baseRect.xmax + diambase, baseRect.ymin - diambase, baseRect.ymax + diambase)

            // switch depending on the border box flag
            switch (flag & flagBorder)
                {
                case 0:
                    { // nothing more to do at this level, we try at a lower level.
                goToNextLevel:
                    if (diambase == (2 * R + 1)) return pv; // already at the bottom level, we stop
                    const int64 nrad = (diambase - 3) / 6; // radius of a sub-box of baseRect
                    Pos newcenter; // compute the new center
                    const int64 off = (2 * nrad + 1);
                    const int64 diffX = pos.X() - basecenter.X(); newcenter.X() = basecenter.X() + ((diffX < -nrad) ? -off : ((diffX > nrad) ? off : 0));
                    const int64 diffY = pos.Y() - basecenter.Y(); newcenter.Y() = basecenter.Y() + ((diffY < -nrad) ? -off : ((diffY > nrad) ? off : 0));
                    if (newcenter == basecenter) { return pv; } // same center: going further down will not improve the solution so we stop 
                    baseRect.xmin = newcenter.X() - nrad; baseRect.xmax = newcenter.X() + nrad; // set the new base
                    baseRect.ymin = newcenter.Y() - nrad; baseRect.ymax = newcenter.Y() + nrad;
                    goto findFullBoxCentered_loop; // go to the next level
                    }
                case flagBorderUp:
                    { // only the up border
                    _extendWith(bestRect, lbest, box1Up, pos); goto goToNextLevel;
                    }
                case flagBorderDown:
                    { // only the down border
                    _extendWith(bestRect, lbest, box1Down, pos); goto goToNextLevel;
                    }
                case flagBorderLeft:
                    { // only the left border
                    _extendWith(bestRect, lbest, box1Left, pos); goto goToNextLevel;
                    }
                case flagBorderRight:
                    { // only the right border
                    _extendWith(bestRect, lbest, box1Right, pos); goto goToNextLevel;
                    }
                case (flagBorderUp | flagBorderDown) :
                    { // only the up and down border
                    _extendWith(bestRect, lbest, line2UpDown, pos); goto goToNextLevel;
                    }
                case (flagBorderLeft | flagBorderRight) :
                    { // only the left and right border
                    _extendWith(bestRect, lbest, line2LeftRight, pos); goto goToNextLevel;
                    }
                case (flagBorderUp | flagBorderLeft) :
                    { // only the up and left border
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpLeft, flagCornerUpLeft); // check if the corner if set
                    if (flag & flagCornerUpLeft) { _extendWith(bestRect, lbest, box2UpLeft, pos); goto goToNextLevel; }
                    _extendWith(bestRect, lbest, box1Up, pos);
                    _extendWith(bestRect, lbest, box1Left, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderUp | flagBorderRight) :
                    { // only the up and right border
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpRight, flagCornerUpRight); // check if the corner if set
                    if (flag & flagCornerUpRight) { _extendWith(bestRect, lbest, box2UpRight, pos); goto goToNextLevel; }
                    _extendWith(bestRect, lbest, box1Up, pos);
                    _extendWith(bestRect, lbest, box1Right, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderDown | flagBorderLeft) :
                    { // only the down and left border
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownLeft, flagCornerDownLeft); // check if the corner if set
                    if (flag & flagCornerDownLeft) { _extendWith(bestRect, lbest, box2DownLeft, pos); goto goToNextLevel; }
                    _extendWith(bestRect, lbest, box1Down, pos);
                    _extendWith(bestRect, lbest, box1Left, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderDown | flagBorderRight) :
                    { // only the down and right border
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownRight, flagCornerDownRight); // check if the corner if set
                    if (flag & flagCornerDownRight) { _extendWith(bestRect, lbest, box2DownRight, pos); goto goToNextLevel; }
                    _extendWith(bestRect, lbest, box1Down, pos);
                    _extendWith(bestRect, lbest, box1Right, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderLeft | flagBorderUp | flagBorderRight) :
                    { // 3 borders except down
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpRight, flagCornerUpRight);
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpLeft, flagCornerUpLeft);
                    if ((flag & flagCornerUpRight) && (flag & flagCornerUpLeft))
                        {
                        _extendWith(bestRect, lbest, rect3Up, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerUpLeft)
                        {
                        _extendWith(bestRect, lbest, box2UpLeft, pos);
                        _extendWith(bestRect, lbest, box1Right, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerUpRight)
                        {
                        _extendWith(bestRect, lbest, box2UpRight, pos);
                        _extendWith(bestRect, lbest, box1Left, pos);
                        goto goToNextLevel;
                        }
                    _extendWith(bestRect, lbest, box1Left, pos);
                    _extendWith(bestRect, lbest, box1Up, pos);
                    _extendWith(bestRect, lbest, box1Right, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderLeft | flagBorderDown | flagBorderRight) :
                    { // 3 borders except up
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownRight, flagCornerDownRight);
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownLeft, flagCornerDownLeft);
                    if ((flag & flagCornerDownRight) && (flag & flagCornerDownLeft))
                        {
                        _extendWith(bestRect, lbest, rect3Down, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerDownLeft)
                        {
                        _extendWith(bestRect, lbest, box2DownLeft, pos);
                        _extendWith(bestRect, lbest, box1Right, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerDownRight)
                        {
                        _extendWith(bestRect, lbest, box2DownRight, pos);
                        _extendWith(bestRect, lbest, box1Left, pos);
                        goto goToNextLevel;
                        }
                    _extendWith(bestRect, lbest, box1Left, pos);
                    _extendWith(bestRect, lbest, box1Down, pos);
                    _extendWith(bestRect, lbest, box1Right, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderUp | flagBorderLeft | flagBorderDown) :
                    { // 3 borders except left
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpLeft, flagCornerUpLeft);
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownLeft, flagCornerDownLeft);
                    if ((flag & flagCornerUpLeft) && (flag & flagCornerDownLeft))
                        {
                        _extendWith(bestRect, lbest, rect3Left, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerUpLeft)
                        {
                        _extendWith(bestRect, lbest, box2UpLeft, pos);
                        _extendWith(bestRect, lbest, box1Down, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerDownLeft)
                        {
                        _extendWith(bestRect, lbest, box2DownLeft, pos);
                        _extendWith(bestRect, lbest, box1Up, pos);
                        goto goToNextLevel;
                        }
                    _extendWith(bestRect, lbest, box1Up, pos);
                    _extendWith(bestRect, lbest, box1Left, pos);
                    _extendWith(bestRect, lbest, box1Down, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderUp | flagBorderRight | flagBorderDown) :
                    { // 3 borders except right
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpRight, flagCornerUpRight);
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownRight, flagCornerDownRight);
                    if ((flag & flagCornerUpRight) && (flag & flagCornerDownRight))
                        {
                        _extendWith(bestRect, lbest, rect3Right, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerUpRight)
                        {
                        _extendWith(bestRect, lbest, box2UpRight, pos);
                        _extendWith(bestRect, lbest, box1Down, pos);
                        goto goToNextLevel;
                        }
                    if (flag & flagCornerDownRight)
                        {
                        _extendWith(bestRect, lbest, box2DownRight, pos);
                        _extendWith(bestRect, lbest, box1Up, pos);
                        goto goToNextLevel;
                        }
                    _extendWith(bestRect, lbest, box1Up, pos);
                    _extendWith(bestRect, lbest, box1Right, pos);
                    _extendWith(bestRect, lbest, box1Down, pos);
                    goto goToNextLevel;
                    }
                case (flagBorderUp | flagBorderDown | flagBorderLeft | flagBorderRight) :
                    { // all four borders are set
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpLeft, flagCornerUpLeft);
                    _checkCorner(flag, diambase, pv, bestRect, cornerUpRight, flagCornerUpRight);
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownLeft, flagCornerDownLeft);
                    _checkCorner(flag, diambase, pv, bestRect, cornerDownRight, flagCornerDownRight);
                    switch (flag & flagCorner) // switch depending on the corners set
                        {
                        case 0:
                            { // no corner
                            _extendWith(bestRect, lbest, box1Up, pos);
                            _extendWith(bestRect, lbest, box1Down, pos);
                            _extendWith(bestRect, lbest, box1Left, pos);
                            _extendWith(bestRect, lbest, box1Right, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft) :
                            {
                            _extendWith(bestRect, lbest, box2UpLeft, pos);
                            _extendWith(bestRect, lbest, box1Right, pos);
                            _extendWith(bestRect, lbest, box1Down, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpRight) :
                            {
                            _extendWith(bestRect, lbest, box2UpRight, pos);
                            _extendWith(bestRect, lbest, box1Left, pos);
                            _extendWith(bestRect, lbest, box1Down, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerDownLeft) :
                            {
                            _extendWith(bestRect, lbest, box2DownLeft, pos);
                            _extendWith(bestRect, lbest, box1Right, pos);
                            _extendWith(bestRect, lbest, box1Up, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerDownRight) :
                            {
                            _extendWith(bestRect, lbest, box2DownRight, pos);
                            _extendWith(bestRect, lbest, box1Left, pos);
                            _extendWith(bestRect, lbest, box1Up, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft | flagCornerDownRight) :
                            {
                            _extendWith(bestRect, lbest, box2UpLeft, pos);
                            _extendWith(bestRect, lbest, box2DownRight, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpRight | flagCornerDownLeft) :
                            {
                            _extendWith(bestRect, lbest, box2UpRight, pos);
                            _extendWith(bestRect, lbest, box2DownLeft, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft | flagCornerUpRight) :
                            {
                            _extendWith(bestRect, lbest, rect3Up, pos);
                            _extendWith(bestRect, lbest, box1Down, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerDownLeft | flagCornerDownRight) :
                            {
                            _extendWith(bestRect, lbest, rect3Down, pos);
                            _extendWith(bestRect, lbest, box1Up, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft | flagCornerDownLeft) :
                            {
                            _extendWith(bestRect, lbest, rect3Left, pos);
                            _extendWith(bestRect, lbest, box1Right, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpRight | flagCornerDownRight) :
                            {
                            _extendWith(bestRect, lbest, rect3Right, pos);
                            _extendWith(bestRect, lbest, box1Left, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft | flagCornerUpRight | flagCornerDownLeft) :
                            {
                            _extendWith(bestRect, lbest, rect3Up, pos);
                            _extendWith(bestRect, lbest, rect3Left, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft | flagCornerUpRight | flagCornerDownRight) :
                            {
                            _extendWith(bestRect, lbest, rect3Up, pos);
                            _extendWith(bestRect, lbest, rect3Right, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerDownLeft | flagCornerDownRight | flagCornerUpLeft) :
                            {
                            _extendWith(bestRect, lbest, rect3Down, pos);
                            _extendWith(bestRect, lbest, rect3Left, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerDownLeft | flagCornerDownRight | flagCornerUpRight) :
                            {
                            _extendWith(bestRect, lbest, rect3Down, pos);
                            _extendWith(bestRect, lbest, rect3Right, pos);
                            goto goToNextLevel;
                            }
                        case (flagCornerUpLeft | flagCornerUpRight | flagCornerDownLeft | flagCornerDownRight) :
                            { // everything is set 
                            _extendWith(bestRect, lbest, rect4, pos);
                            return pv; //done, we will not imprve anymore so we quit !
                            }
                        }
                    }
                }
            MTOOLS_ERROR("wtf...");
            #undef box1Up
            #undef box1Down
            #undef box1Left
            #undef box1Right
            #undef line2UpDown
            #undef line2LeftRight
            #undef box2UpLeft
            #undef box2UpRight
            #undef box2DownLeft
            #undef box2DownRight
            #undef rect3Up
            #undef rect3Down
            #undef rect3Left
            #undef rect3Right
            #undef rect4
            return nullptr;
            }






        /***************************************************************
        * Private section
        ***************************************************************/

    private:

        /* Make sure template parameters are OK */
        static_assert(NB_SPECIAL > 0, "the number of special objects must be > 0, use Grid_basic otherwise.");
        static_assert(D > 0, "template parameter D (dimension) must be non-zero");
        static_assert(R > 0, "template parameter R (radius) must be non-zero");
        static_assert(std::is_constructible<T>::value || std::is_constructible<T, Pos>::value, "The object T must either be default constructible T() or constructible with T(const Pos &)");
        static_assert(std::is_copy_constructible<T>::value, "The object T must be copy constructible T(const T&).");
        static_assert(std::is_convertible<T, int64>::value, "The object T must be convertible to int64");
        static_assert(metaprog::has_assignementOperator<T>::value, "The object T must be assignable via operator=()");




        /* Used by findFullBoxCentered (2D specialization). Check if a given border adjacent box of same size is full. */
        inline void _checkBorder(int & flag, const int64 diam, const T * pv, const iRect & refRect, const Pos & borderPos, int flagBorder, const Pos & cornerPos1, int flagCorner1, const Pos & cornerPos2, int flagCorner2) const
            {
            if (refRect.isInside(borderPos)) { flag |= flagBorder; return; } // if borderPos is in the reference rectangle, set to 1 and do not check the corner positions
            iRect RR;
            if ((findFullBox(borderPos, RR) != pv)||(RR.lx() == 0)) { return; } // if the box is a singleton or does not contain the correct value, we quit
            const int64 rd = RR.lx() + 1; // diameter of the box found
            if (rd < diam) { return; } // the box is too small, we quit
            flag |= flagBorder; // ok, the border box is good.
            if (rd > diam)
                { // the box is stricly larger than diam so at least one of the border point should be in it
                if (RR.isInside(cornerPos1)) { flag |= flagCorner1; } // yes, corner point 1 is in
                if (RR.isInside(cornerPos2)) { flag |= flagCorner2; } // yes, corner point 2 is in
                MTOOLS_ASSERT(flag & (flagCorner1 | flagCorner2));
                }
            return;
            }


        /* Used by findFullBoxCentered (2D specialization). Check if a given cornder adjacent box of same size is full. */
        inline void _checkCorner(int & flag, const int64 diam, const T * pv, const iRect & refRect, const Pos & cornerPos, int flagCorner) const
            {
            if (flag & flagCorner) return; // alread set, nothing to do
            if (refRect.isInside(cornerPos)) { flag |= flagCorner; return; } // if pos is in the reference rectangle, set the flag
            iRect RR;
            if ((findFullBox(cornerPos, RR) != pv) || (RR.lx() == 0)) { return; } // if the box is a singleton or does not contain the correct value, we quit
            if ((RR.lx() + 1) < diam) { return; } // box too small
            flag |= flagCorner;
            return;
            }


        /* Used by findFullBoxCentered (2D specialization). Compute the new best rectangle */
        inline void _extendWith(iRect & currentBest, int64 & lbest, iRect newRect, const Pos & pos) const
            {
            const int64 lnew = newRect.boundaryDist(pos);
            if (lnew > lbest)
                { // improvement
                newRect.enlargeWith(currentBest); // try to enlarge newrect is possible
                currentBest = newRect;  // then set it as the new solution
                lbest = currentBest.boundaryDist(pos); // save the new distance from the boundary
                return;
                }
            currentBest.enlargeWith(newRect);       // Only useful at the very first level 
            lbest = currentBest.boundaryDist(pos);  // when newRect contain currentbest
            return;
            }


        /* print the tree, for debug purpose only */
        std::string _printTree(_pbox p, std::string tab) const
            {
            if (p == nullptr) { return(tab + " NULLPTR\n"); }
            T * obj = _getSpecialObject(p);
            if (obj != nullptr) 
                { 
                int64 v = (int64)(*obj);
                return(tab + " SPECIAL (" + mtools::toString(v) + ")\n");
                }
            if (p->isLeaf())
                {
                std::string r = tab + " Leaf: center = " + p->center.toString(false) + "\n";
                return r;
                }
            std::string r = tab + " Node: center = " + p->center.toString(false) + "  Radius = " + mtools::toString(p->rad) + "\n";
            tab += "    |";
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { r += _printTree(((_pnode)p)->tab[i], tab); }
            return r;
            }


        /* update the _rangemin and _rangemax member */
        inline void _updatePosRange(const Pos & pos) const
            {
            for (size_t i = 0; i < D; i++) { if (pos[i] < _rangemin[i]) { _rangemin[i] = pos[i]; } else if (pos[i] > _rangemax[i]) { _rangemax[i] = pos[i]; } }
            }


        /* update _minVal and _maxVal */
        inline void _updateValueRange(int64 v) const
            {
            if (v < _minVal) _minVal = v;
            if (v > _maxVal) _maxVal = v;
            }


        /* set the object at a given position.
        * keep the tree consistent and simplified */
        inline void _set(const Pos & pos, const T * val)
            {
            _pbox pc = _pcurrent;
            MTOOLS_ASSERT(pc != nullptr);
            _updatePosRange(pos);
            if (pc->isLeaf())
                {
                if (((_pleafFactor)pc)->isInBox(pos)) 
                    { 
                    _pcurrent = _setLeafValue(val, pos, (_pleafFactor)pc); //set the value and then simplify if needed
                    return; 
                    }
                MTOOLS_ASSERT(pc->father != nullptr); // a leaf must always have a father
                pc = pc->father;
                }
            // going up...
            _pnode q = (_pnode)pc;
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { q->father = _allocateNode(q); }
                q = (_pnode)q->father;
                }
            // ...and down
            while (1)
                {
                _pbox & b = q->getSubBox(pos); // the subbox to look into
                if (b == nullptr)
                    { // subbox never created..
                    if (q->rad == R)
                        { // the subbox to create is a leaf
                        b = _allocateLeaf(q, q->subBoxCenter(pos)); // create the leaf
                        _pcurrent = _setLeafValue(val, pos, (_pleafFactor)b); // set the value and then factorize if needed 
                        return;
                        }
                    // create the subnode
                    b = _allocateNode(q, q->subBoxCenter(pos), nullptr);
                    q = (_pnode)b;
                    }
                else
                    {
                    T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                    if (obj != nullptr) 
                        { // yes, dummy link
                        int64 nv = (int64)(*val);       // the new object value
                        int64 ov = _getSpecialValue(b); // the current object special value for this region
                        if (ov == nv) { _pcurrent = q; _updateValueRange(nv); return; } // same values so there is nothing to do really
                        // not the same value, we must partially expand the tree.  
                        _pbox dum = b; // the dummy link
                        while(1)
                            {
                            _pbox & bb = q->getSubBox(pos);
                            if (q->rad == R)
                                { // the subbox to create is a leaf
                                bb = _allocateLeafCst(q, q->subBoxCenter(pos),obj,ov); // create the leaf with all element equal to the special object
                                _pcurrent = _setLeafValue(val, pos, (_pleafFactor)bb); // set the value (and then factorize if needed but here nothing is done) 
                                return;
                                }
                            // the subbox to create is a node
                            bb = _allocateNode(q, q->subBoxCenter(pos), dum); // create the subnode and fill its tab with the same dummy value
                            q = (_pnode)bb;
                            }
                        } 
                    // it is a real link
                    if (b->isLeaf()) 
                        {
                        _pcurrent = _setLeafValue(val, pos, (_pleafFactor)b); //set the value and then simplify if needed
                        return; 
                        }
                    q = (_pnode)b; // go down
                    }
                }
            }



        /* get the object at a given position. 
         * keep the tree consistent and simplified 
         * the object is created if it does not exist */
        inline T & _get(const Pos & pos) const
            {
            _pbox pc = _pcurrent;
            MTOOLS_ASSERT(pc != nullptr);
            _updatePosRange(pos);
            if (pc->isLeaf())
                {
                if (((_pleafFactor)pc)->isInBox(pos)) 
                        { 
                        MTOOLS_ASSERT(_isLeafFull((_pleafFactor)pc) == (_maxSpec + 1)); // the leaf cannot be full
                        return(((_pleafFactor)pc)->get(pos)); 
                        }
                MTOOLS_ASSERT(pc->father != nullptr); // a leaf must always have a father
                pc = pc->father;
                }
            // going up...
            _pnode q = (_pnode)pc;
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { q->father = _allocateNode(q);}
                q = (_pnode)q->father;
                }
            // ...and down
            while (1)
                {
                _pbox & b = q->getSubBox(pos); // the subbox to look into
                if (b == nullptr)
                    { // the subbox was never created..
                    if (q->rad == R)
                        { // the subbox to create is a leaf
                        _pleafFactor L = _allocateLeaf(q, q->subBoxCenter(pos)); // create a leaf
                        int64 cv = _isLeafFull(L);
                        if (cv == (_maxSpec + 1))
                            {  // the leaf is not full
                            b = L; _pcurrent = b; 
                            return(L->get(pos));
                            }
                        // the leaf is full
                        b = _setSpecial(cv, L->data); // save the special value if needed and set the link inside the father node tab
                        _releaseLeaf(L);
                        _pcurrent = _simplifyNode(q);
                        return(*_getSpecialObject(cv));
                        }
                    // create the subnode
                    b = _allocateNode(q, q->subBoxCenter(pos), nullptr);
                    q = (_pnode)b;
                    }
                else
                    {
                    T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                    if (obj != nullptr) { _pcurrent = q; return(*obj); } // yes, we return the associated value 
                    // no, b is a real link
                    if (b->isLeaf()) 
                            { 
                            MTOOLS_ASSERT(_isLeafFull((_pleafFactor)b) == (_maxSpec + 1)); // the leaf cannot be full
                            _pcurrent = b; return(((_pleafFactor)b)->get(pos)); 
                            }
                    q = (_pnode)b;
                    }
                }
            }


        /* get the root of the tree */
        inline _pbox _getRoot() const
            {
            if (_pcurrent == nullptr) return nullptr;
            _pbox p = _pcurrent;
            while (p->father != nullptr) { p = p->father; }
            return p;
            }


        /* Reset the object and change the min and max values for the special objects and the calldtor flag
           (the calldtors is set AFTER destruction ie the previous status is used when releasing memory) */
        void _reset(int64 minSpec, int64 maxSpec, bool callDtors)
            {
            MTOOLS_INSURE(((maxSpec < minSpec) || (maxSpec - minSpec < ((int64)NB_SPECIAL))));
            _reset();
            _minSpec = minSpec;
            _maxSpec = maxSpec;
            _callDtors = callDtors;
            }



        /* Reset the object */
        void _reset()
            {
            _poolNode.deallocateAll();
            if (_callDtors)
                {
                _poolLeaf.destroyAll();
                _poolSpec.destroyAll();
                }
            else
                {
                _poolLeaf.deallocateAll();
                _poolSpec.deallocateAll();
                }
            _pcurrent = nullptr;
            _pcurrentpeek = nullptr;
            _rangemin.clear(std::numeric_limits<int64>::max());
            _rangemax.clear(std::numeric_limits<int64>::min());
            
            _minVal = std::numeric_limits<int64>::max();
            _maxVal = std::numeric_limits<int64>::min();

            memset(_tabSpecObj, 0, sizeof(_tabSpecObj));
            memset(_tabSpecNB, 0, sizeof(_tabSpecNB));
            _nbNormalObj = 0;
            return;
            }




        /***************************************************************
        * Working with leafs and nodes
        ***************************************************************/


        /* recursive method for serialization of the tree 
           used by serialize() */
            void _serializeTree(OArchive & ar, _pbox p) const
            {
            if (p == nullptr) { ar & ((char)'V'); return; } // void pointer
            if (_getSpecialObject(p) != nullptr)
                {
                ar & ((char)'S'); // special object
                ar & ((int64)_getSpecialValue(p));
                ar.newline();
                return;
                }
            if (p->isLeaf())
                {
                ar & ((char)'L');
                ar & p->center;
                ar & p->rad;
                for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i) { ar & (((_pleafFactor)p)->data[i]); }
                ar.newline();
                return;
                }
            ar & ((char)'N');
            ar & p->center;
            ar & p->rad;
            ar.newline();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { _serializeTree(ar, ((_pnode)p)->tab[i]); }
            return;
            }


        /* recursive method for deserialization of the tree 
           used by deserialize() */
        _pbox _deserializeTree(IArchive & ar, _pbox father)
            {
            char c;
            ar & c;
            if (c == 'V') return nullptr;
            if (c == 'S')
                {
                uint64 val;
                ar & val;
                MTOOLS_ASSERT(_isSpecial(val));
                _updateValueRange(val); // possibly a new extremum value
                int64 n = 2*(father->rad) + 1; int64 pow = 1; for (size_t i = 0; i < D; i++) { pow *= n; }
                _tabSpecNB[val - _minSpec] += pow; // add to the correct number of special object to the global counter
                return _getSpecialNode(val);
                }
            if (c == 'L')
                {
                return _deserializeLeaf(ar, father);
                }
            if (c == 'N')
                {
                _pnode p = _poolNode.allocate();
                ar & p->center;
                ar & p->rad;
                p->father = father;
                for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = _deserializeTree(ar, p); }
                return p;
                }
            MTOOLS_DEBUG(std::string("Unknown tag [") + std::string(1,c) + "]");
            throw "";
            }



        /* recursive method for copying a tree from G used by operator=(Grid_basic) */
        _pbox _copyTreeFromGridBasic(_pbox father, _pbox p)
            {
            MTOOLS_ASSERT(p != nullptr);
            if (p->isLeaf())
                { // we must copy a leaf
                _pleafFactor F = _poolLeaf.allocate();
                F->center = p->center;
                F->rad = p->rad;
                F->father = father;
                memset(F->count, 0, sizeof(F->count)); // reset the number of each type of special object
                for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x) { new(F->data + x) T((((_pleafFactor)p)->data[x])); }
                _nbNormalObj += metaprog::power<(2 * R + 1), D>::value;
                return F;
                }
            // we must copy a node
            _pnode N = _poolNode.allocate();
            N->center = p->center;
            N->rad = p->rad;
            N->father = father;
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i)
                {
                const _pbox B = ((_pnode)p)->tab[i];
                N->tab[i] = ((B == nullptr) ? nullptr : _copyTreeFromGridBasic(N, B));
                }
            return N;
            }


        /* recursive method for copying a tree from G
        * used by operator=() */
        template<size_t NB_SPECIAL2> _pbox _copyTree(_pbox father, _pbox p, const Grid_factor<D, T, NB_SPECIAL2, R> & G)
            {
            MTOOLS_ASSERT(p != nullptr);
            if (G._getSpecialObject(p) != nullptr)
                { // special node from G
                return _getSpecialNode(G._getSpecialValue(p)); // get the corresponding dummy node for this object 
                }
            // node is not special
            if (p->isLeaf())
                { // we must copy a leaf
                _pleafFactor F = _poolLeaf.allocate();
                F->center = p->center;
                F->rad = p->rad;
                F->father = father;
                memset(F->count, 0, sizeof(F->count)); // reset the number of each type of special object
                for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                    {
                    new(F->data + x) T((((_pleafFactor)p)->data[x])); // copy ctor
                    int64 val = (int64)(F->data[x]); // convert to int64
                    if (_isSpecial(val)) { (F->count[val - _minSpec])++; }
                    }
                return F;
                }
            // we must copy a node
            _pnode N = _poolNode.allocate();
            N->center = p->center;
            N->rad = p->rad;
            N->father = father;
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i)
                {
                const _pbox B = ((_pnode)p)->tab[i];
                N->tab[i] = ((B == nullptr) ? nullptr : _copyTree<NB_SPECIAL2>(N, B, G));
                }
            return N;
            }


        /* expand the whole tree 
         * _pcurrent is set to the root of the tree
         */
        void _expandTree()
            {
            _pcurrent = _getRoot();
            _pcurrentpeek = _pcurrent;
            MTOOLS_ASSERT(_pcurrent != nullptr);
            _expandBelowNode((_pnode)_pcurrent);
            }


        /* expand the whole tree below a given node. */
        void _expandBelowNode(_pnode N)
            {
            if (N->rad > R)
                { 
                // children of this node are also nodes
                for (size_t i = 0; i < metaprog::power<3, D>::value; ++i)
                    {
                    const _pbox K = N->tab[i];
                    if (K != nullptr)
                        {
                        if (_getSpecialObject(K) != nullptr)
                            { // we expand special nodes
                            N->tab[i] = _allocateNode(N, N->subBoxCenterFromIndex(i), K); // create it
                            }
                        _expandBelowNode((_pnode)(N->tab[i])); // and recurse
                        }
                    }
                return; 
                }
            // children of this node are leaves
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i)
                {
                const _pbox K = N->tab[i];
                T * pv = _getSpecialObject(K);
                if (pv != nullptr) // check if the child node is special
                    { // yes we expand it
                    N->tab[i] = _allocateLeafCst(N, N->subBoxCenterFromIndex(i), pv, _getSpecialValue(K)); // create it
                    }
                }
            }


        /* Simplify the whole tree. 
         * _pcurrent and _pcurrentpeek are set to the (possibly new) root of the tree */
        void _simplifyTree() const
            {
            _pbox p = _getRoot(); // find the root of the tree
            MTOOLS_ASSERT(p != nullptr);
            _pbox q = _simplifyBelow(p); // simplify the tree
            if (q == p) { _pcurrent = p; return; }
            // we must create the root father
            MTOOLS_ASSERT(_getSpecialObject(q) != nullptr); // q should be a dummy link
            MTOOLS_ASSERT(p->father == nullptr);
            _pnode F = _allocateNode(p);  // the new root
            _pbox & B = F->getSubBox(p->center); // get the corresponding pointer in the father tab
            MTOOLS_ASSERT(B == p); // make sure everything is coherent
            B = q; // set the new value
            _releaseBox(p);
            _pcurrent = F;
            _pcurrentpeek = F;
            }


        /* simplify the whole sub-tree below a given node/leaf */ 
        _pbox _simplifyBelow(_pbox N) const
            {
            if (N == nullptr) return nullptr; // nothing to do
            if (_getSpecialObject(N) != nullptr) return N; // already a special node
            // we have a real pointer (no dummy or nullptr)
            if (N->isLeaf()) // we have a leaf
                {  
                _pleafFactor L = (_pleafFactor)N;
                int64 val = _isLeafFull(L);
                if (val <= _maxSpec) { return _setSpecial(val, L->data); } // yes, the leaf is full and we can factorize it
                return N; // no factorization is possible
                }
            // we have a node
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) // we iterate over the children
                {
                _pbox & C = ((_pnode)N)->tab[i];
                _pbox K = _simplifyBelow(C); // simplify if possible
                if (K != C) { _releaseBox(C); C = K; } // yes, we release the memory and save the dummy link
                }
            // now that every children is simplified, we go again to see if we can simplify the node itself
            _pbox p = ((_pnode)N)->tab[0]; // the first child of the node
            if (_getSpecialObject(p) == nullptr) return N; // the first child is not special: no further simplification
            for (size_t i = 1; i < metaprog::power<3, D>::value; ++i)
                {
                if (((_pnode)N)->tab[i] != p) return N; // not the same special value for all children: no further simplification
                }
            return p; // yep, we can simplify using the dummy pointer p
            }


        /* simplify the branche going up above a given node and return
        the (possibly new) end point of the branche */
        _pnode _simplifyNode(_pnode N) const
            {
            while (1)
                {
                _pbox p = N->tab[0]; // first child of the node
                if (_getSpecialObject(p) == nullptr) return N; // the first child is not special, no simplification
                for (size_t i = 1; i < metaprog::power<3, D>::value; ++i)
                    {
                    if (N->tab[i] != p) return N; // not the same special value for all children: no simplification
                    }
                // yes, we can simplify
                if (N->father == nullptr) { N->father = _allocateNode(N); } // make sure the father exist 
                    { // this part is mutex protected from safePeek()
                    std::lock_guard<std::recursive_mutex> lock(_peekmut);
                    _pnode F = (_pnode)(N->father); // the father node
                    if (_pcurrentpeek == N) { _pcurrentpeek = F; }
                    _pbox & B = F->getSubBox(N->center); // get the corresponding pointer in the father tab
                    MTOOLS_ASSERT(B == N); // make sure everything is coherent
                    B = p; // set the new value
                    _releaseNode(N);
                    N = F; // go to the father;
                    }
                }
            }


        /* recount all the leaf in the tree.
        * this method also increase the global counters for special objects and normal objects 
        * set _pcurrent and _pcurrentpeek to the root of the tree */
        void _recountTree() const
            {
            _pcurrent = _getRoot();
            _pcurrentpeek = _pcurrent;
            _recountBelow(_pcurrent);
            }
            

        /* recompute the count[] array of all the leafs of a given subtree
         * this method also increase the global counters for special objects 
         * and normal objects */
        void _recountBelow(_pbox N) const
            {
            MTOOLS_ASSERT(((N != nullptr) && (_getSpecialObject(N) == nullptr))); // must be a 'normal node'
            if (N->isLeaf()) // we have a leaf
                {
                _recountLeaf((_pleafFactor)N); // we recount it
                return;
                }
            // we have a node
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) // we iterate over the children
                {
                const _pbox b = ((_pnode)N)->tab[i];
                if ( b != nullptr) // skip null nodes
                    {
                    if (_getSpecialObject(b) != nullptr)
                        {
                        int64 n = 2 * (N->rad) + 1; int64 pow = 1; for (size_t i = 0; i < D; i++) { pow *= n; }
                        _tabSpecNB[_getSpecialValue(b) - _minSpec] += pow; // add to the correct number of special object to the global counter
                        }
                    else
                        { // normal node
                        _recountBelow(((_pnode)N)->tab[i]); // we recount it...
                        }
                    }
                }
            return;
            }


        /* Recompute from scratch the count[] array of a leaf. 
           This method also increase the global counters for 
           special objects and normal object */
        void _recountLeaf(_pleafFactor L) const 
            {
            memset(L->count, 0, sizeof(L->count)); // reset the number of each type of special object
            if (!_existSpecial()) { _nbNormalObj += metaprog::power<(2 * R + 1), D>::value; return; } // no special value, all the value are therefore normal   
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                int64 val = (int64)(L->data[x]); // convert to int64
                if (_isSpecial(val)) // check if it is a special value
                    { // yes
                    auto off = val - _minSpec;           // the offset of the special object
                    (L->count[off])++;                   // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;                 // increase the global counter about the number of these special objects 
                    }
                else { _nbNormalObj++; } // increase the global counter for the number of normal object
                }
            }


        /* check if a leaf is full and thus should be factorized
         * return the value of the special object if it is full and _maxSpec + 1 otherwise */
        inline int64 _isLeafFull(_pleafFactor L) const
            {
            MTOOLS_ASSERT(L != nullptr);
            int64 val = (int64)(*(L->data));
            if (_isSpecial(val))
                {
                MTOOLS_ASSERT( (L->count[val - _minSpec] <= metaprog::power<(2 * R + 1), D>::value) );
                if (L->count[val - _minSpec] == metaprog::power<(2 * R + 1), D>::value) { return val; }
                }
            return (_maxSpec + 1);
            }


        /* Set the value in a leaf.
         * - Factorizes the leaf and its nodes above if needed.
         * - Return the leaf or the first node above */
        inline _pbox _setLeafValue(const T * obj, const Pos & pos, _pleafFactor leaf)
            {
            T * oldobj = &(leaf->get(pos));     // the previous object
            int64 oldvalue = (int64)(*oldobj);  // the previous object value
            int64 value = (int64)(*obj);        // the new object value
            if (oldvalue == value)
                { // the old and new object have the same value
                if (_isSpecial(value)) 
                    { // it is a special value
                    MTOOLS_ASSERT( (leaf->count[value - _minSpec] <= metaprog::power<(2 * R + 1), D>::value) );
                    if (leaf->count[value - _minSpec] == metaprog::power<(2 * R + 1), D>::value) // check if the value is the only one in this leaf
                        { // yes the leaf can be factorized
                        std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect this part from safePeek()
                        _pnode F = (_pnode)(leaf->father); // the father of the leaf
                        if (_pcurrentpeek == leaf) { _pcurrentpeek = F; }
                        MTOOLS_ASSERT(F != nullptr);
                        _pbox & B = F->getSubBox(pos); // the pointer to the leaf in the father tab
                        MTOOLS_ASSERT(B == ((_pbox)leaf)); // make sure we are coherent.
                        B = _setSpecial(value, obj); // save the special object if needed and set the dummy node in place of the pointer to the leaf
                        _releaseLeaf(leaf);
                        return _simplifyNode(F); // we try to simplify the tree starting from the father
                        }
                    return leaf; // return the leaf (it cannot be factorized)
                    } 
                (*oldobj) = (*obj); // not a special value, simply replace it using the assignement operator
                return leaf; // return the leaf (it cannot be factorized)
                }
            // old and new do not have the same value
            _updateValueRange(value); // possibly a new extreme value
            (*oldobj) = (*obj); // save the new value
            if (_isSpecial(oldvalue)) 
                { //old value was special, decrement the global count and the leaf count 
                auto off = oldvalue - _minSpec; 
                MTOOLS_ASSERT(_tabSpecNB[off] > 0);
                MTOOLS_ASSERT(leaf->count[off] > 0);
                (_tabSpecNB[off])--; (leaf->count[off])--;
                } 
            else 
                { // old value was normal, decrement the global count
                MTOOLS_ASSERT(_nbNormalObj >0);
                _nbNormalObj--;
                }
            if (_isSpecial(value))
                { // new value is special, increment the global and local count
                auto off = value - _minSpec; (_tabSpecNB[off])++; (leaf->count[off])++; // increment its count
                MTOOLS_ASSERT( (leaf->count[off] <= metaprog::power<(2 * R + 1), D>::value) );
                if (leaf->count[off] == metaprog::power<(2 * R + 1), D>::value) 
                    { // ok, we can factorize and remove this leaf.
                    std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect this part from safePeek()
                    _pnode F = (_pnode)(leaf->father); // the father of the leaf
                    if (_pcurrentpeek == leaf) { _pcurrentpeek = F; }
                    MTOOLS_ASSERT(F != nullptr);
                    _pbox & B = F->getSubBox(pos); // the pointer to the leaf in the father tab
                    MTOOLS_ASSERT(B == ((_pbox)leaf)); // make sure we are coherent.
                    B = _setSpecial(value, obj); // save the special object if needed and set the dummy node in place of the pointer to the leaf
                    _releaseLeaf(leaf);
                    return _simplifyNode(F); // we try to simplify the tree starting from the father
                    } 
                }
            else
                { // new value is not special, just increase the global count
                _nbNormalObj++;
                }
            return leaf; // return the leaf since it cannot be factorized
            }



        /***************************************************************
        * Memory allocation : creating Nodes and Leafs
        ***************************************************************/


        /* release the memory associated with a node/leaf and call the dtor
         depending on the status of _callDtors */
        inline void _releaseBox(_pbox N) const 
            {
            MTOOLS_ASSERT(N != nullptr); // the node must not be nullptr
            MTOOLS_ASSERT(_getSpecialObject(N) == nullptr); // the node must not be special
            if (N->isLeaf()) { _releaseLeaf((_pleafFactor)N); return; }
            _releaseNode((_pnode)N);
            }


        /* release the memory associated with a node */
        inline void _releaseNode(_pnode N) const
            {
             MTOOLS_ASSERT(N != nullptr); // the node must not be nullptr
             MTOOLS_ASSERT(_getSpecialObject(N) == nullptr); // the node must not be special
             MTOOLS_ASSERT(!(N->isLeaf()));
             _poolNode.deallocate(N);
            }
        

        /* release the memory associated with a leaf */
        inline void _releaseLeaf(_pleafFactor L) const
            {
            MTOOLS_ASSERT(L != nullptr); // the node must not be nullptr
            MTOOLS_ASSERT(_getSpecialObject(L) == nullptr); // the node must not be special
            MTOOLS_ASSERT(L->isLeaf());
            if (_callDtors) { _poolLeaf.destroy(L); } else { _poolLeaf.deallocate(L); }
            }



        /* deserialize a single object, use positional constructor first and then deserialize */
        inline void _deserializeObjectT_sub(IArchive & ar, T * p, metaprog::dummy<false> dum) { new(p) T(Pos(0));  ar &  (*p); }

        /* deserialize a single object, use default constructor first and then deserialize */
        inline void _deserializeObjectT_sub(IArchive & ar, T * p, metaprog::dummy<true> dum)  { new(p) T(); ar &  (*p); }

        /* deserialize a single object using a constructor and then the deserialization method */
        inline void _deserializeObjectT(IArchive & ar, T * p, metaprog::dummy<false> dum) {_deserializeObjectT_sub(ar, p, metaprog::dummy<std::is_constructible<T>::value>());}

        /* deserialize a single object using the IArchive constructor */
        inline void _deserializeObjectT(IArchive & ar, T * p, metaprog::dummy<true> dum) { new(p) T(ar); }

        /* deserialize a single object, use the correct constructor  */
        inline void _deserializeObjectT(IArchive & ar, T * p)
            {
            _deserializeObjectT(ar, p, metaprog::dummy< std::is_constructible<T, IArchive>::value>());
            }


        /* deserialize data, use positional constructor first and then deserialize */
        inline void _deserializeDataLeaf_sub(IArchive & ar, _pleafFactor L, metaprog::dummy<true> dum)
            {
            Pos pos = L->center;
            for (size_t i = 0; i < D; ++i) { pos[i] -= R; } // go to the first cell
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                new(L->data + x) T(pos); // create the object using the positional constructor
                ar &  (L->data[x]); // deserialize
                for (size_t i = 0; i < D; ++i) { if (pos[i] < (L->center[i] + (int64)R)) { pos[i]++;  break; } pos[i] -= (2 * R); } // move to the next cell.
                }
            return;
            }


        /* deserialize data, use default constructor first and then deserialize */
        inline void _deserializeDataLeaf_sub(IArchive & ar, _pleafFactor L, metaprog::dummy<false> dum)
            {
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                new(L->data + x) T();  // create the object using the default constructor
                ar &  (L->data[x]); // deserialize
                }
            return;
            }


        /* deserialize data, use a constructor before deserialization */
        inline void _deserializeDataLeaf(IArchive & ar, _pleafFactor L, metaprog::dummy<false> dum)
            {
            _deserializeDataLeaf_sub(ar, L, metaprog::dummy<std::is_constructible<T,Pos>::value>()); // call the correct constructor method
            }


        /* deserialize data, use IArchive constructor */
        inline void _deserializeDataLeaf(IArchive & ar, _pleafFactor L, metaprog::dummy<true> dum)
            {
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i) { new(L->data + i) T(ar); }
            }


        /* deserialize a leaf, call the correct constructor for the T objects */
        inline _pleafFactor _deserializeLeaf(IArchive & ar, _pbox father)
            {
            MTOOLS_ASSERT(father->rad == R);
            _pleafFactor L = _poolLeaf.allocate();
            L->father = father;
            ar & L->center;
            ar & L->rad;
            MTOOLS_ASSERT(L->rad == 1);
            _deserializeDataLeaf(ar, L, metaprog::dummy< std::is_constructible<T, IArchive>::value>()); // reconstruct the data, calling the correct ctor
            memset(L->count, 0, sizeof(L->count));
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)
                {
                int64 val = (int64)(*(L->data + i)); // convert to int64
                _updateValueRange(val); // possibly a new extremum value
                if (_isSpecial(val)) // check if it is a special value
                    { // yes
                    auto off = val - _minSpec;       // the offset of the special object
                    (L->count[off])++;               // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;             // increase the global counter about the number of these special objects 
                    }
                else { _nbNormalObj++; } // increase the global counter for the number of normal object
                }
            return L;
            }


        /* sub-method using the positional constructor */
        void _createDataLeaf(_pleafFactor pleaf, Pos pos, metaprog::dummy<true> dum) const
            {
            Pos center = pos; // save the center position
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            for (size_t i = 0; i < D; ++i) { pos[i] -= R; } // go to the first cell
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                new(pleaf->data + x) T(pos); // create using the positionnal constructor
                int64 val = (int64)(*(pleaf->data + x)); // convert to int64
                _updateValueRange(val); // possibly a new extremum value
                if (_isSpecial(val)) // check if it is a special value
                    { // yes
                    auto off = val - _minSpec;           // the offset of the special object
                    (pleaf->count[off])++;               // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;                 // increase the global counter about the number of these special objects 
                    }
                else { _nbNormalObj++; } // increase the global counter for the number of normal object
                for (size_t i = 0; i < D; ++i) { if (pos[i] < (center[i] + (int64)R)) { pos[i]++;  break; } pos[i] -= (2 * R); } // move to the next cell.
                }
            return;
            }


        /* sub-method using the default constructor */
        void _createDataLeaf(_pleafFactor pleaf, Pos pos, metaprog::dummy<false> dum) const
            {
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)
                {
                new(pleaf->data + i) T();
                int64 val = (int64)(*(pleaf->data + i)); // convert to int64
                _updateValueRange(val); // possibly a new extremum value
                if (_isSpecial(val)) // check if it is a special value
                    { // yes
                    auto off = val - _minSpec;           // the offset of the special object
                    (pleaf->count[off])++;               // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;                 // increase the global counter about the number of these special objects 
                    }
                else { _nbNormalObj++; } // increase the global counter for the number of normal object
                }
            return;
            }


        /* Create a leaf at a given position. */
        _pleafFactor _allocateLeaf(_pbox above, const Pos & centerpos) const
            {
            _pleafFactor p = _poolLeaf.allocate(); // allocate the memory
            _createDataLeaf(p, centerpos, metaprog::dummy<std::is_constructible<T, Pos>::value>()); // create the data
            p->center = centerpos;
            p->rad = 1;
            p->father = above;
            return p;
            }


        /* Create a leaf at a given position, setting all the object as copy of obj.
         * This method does not modify the global counters */
        _pleafFactor _allocateLeafCst(_pbox above, const Pos & centerpos, const T * obj, int64 value) const
            {
            _pleafFactor pleaf = _poolLeaf.allocate(); // allocate the memory
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i) { new(pleaf->data + i) T(*obj); } // init all objects with copy ctor
            MTOOLS_ASSERT(value == (int64)(*obj)); // make sure obj and value match
            if (_isSpecial(value)) { (pleaf->count[value - _minSpec]) = metaprog::power<(2 * R + 1), D>::value; } // update the count array when obj is special
            pleaf->center = centerpos;
            pleaf->rad = 1;
            pleaf->father = above;
            return pleaf;
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
            _pcurrentpeek = (_pbox)_pcurrent;
            return;
            }


        /* create a node, call constructor from above and fill the tab with pfill*/
        inline _pnode _allocateNode(_pbox above, const Pos & centerpos, _pbox pfill) const
            {
            MTOOLS_ASSERT(above->rad > R); 
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = pfill; }
            p->center = centerpos;
            p->rad = (above->rad - 1) / 3;
            p->father = above;
            return p;
            }


        /* create a node, call constructor from below : creates a new root */
        inline _pnode _allocateNode(_pbox below) const
            {
            MTOOLS_ASSERT(below->center == Pos(0)); // a new root should always be centered
            MTOOLS_ASSERT(below->rad >= R);
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = nullptr; }
            p->tab[(metaprog::power<3, D>::value - 1) / 2] = below;
            p->center = below->center;
            p->rad = (below->rad * 3 + 1);
            p->father = nullptr;
            return p;
            }


        /***************************************************************
        * Dealing with special values 
        ***************************************************************/

        /* return true if val is a special value */
        inline bool _isSpecial(int64 val) const { return((val <= _maxSpec)&&(val >= _minSpec)); }


        /* return true if the special value range is non empty */
        inline bool _existSpecial() const { return (_maxSpec >= _minSpec); }

        /* the number of special element, negative or zero if there are none */
        inline int64 _specialRange() const { return (_maxSpec - _minSpec + 1); }

        /* Return the value associated with a dummy node
         * No checking that the node is indeed a dummy node */
        inline int64  _getSpecialValue(_pbox p) const
            {
            int64 off = ((_pnode)p) - (_dummyNodes);
            off += _minSpec;
            MTOOLS_ASSERT(_isSpecial(off));
            return off;
            }
            

        /* Return a pointer to the object associated with a special value 
           No checking that the the value is correct or that an object was associated with it */
        inline T * _getSpecialObject(int64 value) const
            {
            MTOOLS_ASSERT(_isSpecial(value));
            MTOOLS_ASSERT(_tabSpecObj[value - _minSpec] != nullptr);
            return _tabSpecObj[value - _minSpec];
            }


        /* Return a pointer to the object associated with a dummy node
           Return nullptr if the node is not special */
        inline T * _getSpecialObject(_pbox p) const
            {
            int64 off = ((_pnode)p) - (_dummyNodes);
            if ((off >= 0) && (off < (int64)NB_SPECIAL))
                { 
                MTOOLS_ASSERT(_tabSpecObj[off] != nullptr); // make sure the special object was previously created.
                return _tabSpecObj[off];
                }
            return nullptr;
            }


        /* return the dummy node associated with a special value 
         * No checking that the value is a correct special value */
        inline _pbox _getSpecialNode(int64 value) const
            {
            MTOOLS_ASSERT(_isSpecial(value));
            return _dummyNodes + (value - _minSpec);
            }


        /* set a special object. Does nothing if already set.
           Return the a pointer to the associated dummy node 
           No checking that value and obj are correct */
        inline _pbox _setSpecial(int64 value, const T* obj) const
            {
            MTOOLS_ASSERT(_isSpecial(value));
            MTOOLS_ASSERT(((int64)(*obj)) == value);
            const auto off = value - _minSpec;
            T* & p = _tabSpecObj[off];
            if (p == nullptr)
                {
                _updateValueRange(value); // possibly a new extremum value
                p = _poolSpec.allocate(); // get a place to store the special element
                new(p) T(*obj); // use copy placement new
                }
            return _dummyNodes + off; // return a pointer to the correpsonding dummy node
            }


        /* copy source into _psafeT using either the assignement or copy operator 
         * used by safePeek() */
        inline const T * _copySafeT(const T * source) const
            {
            if (_psafeT == nullptr) { _psafeT = new T(*source); } else { (*_psafeT) = (*source); }
            return _psafeT;
            }


        /***************************************************************
        * Internal state
        ***************************************************************/

        mutable std::recursive_mutex  _peekmut; // mutex used for safePeek().
        mutable T *   _psafeT;                  // place to store the safePeeked object

        mutable _pbox _pcurrentpeek;            // pointer to the current box used for peeking
        mutable _pbox _pcurrent;                // pointer to the current box
        mutable Pos   _rangemin;                // the minimal accessed range
        mutable Pos   _rangemax;                // the maximal accessed range

        mutable int64 _minVal;                  // current minimum value in the grid
        mutable int64 _maxVal;                  // current maximum value in the grid

        mutable SingleAllocator<internals_grid::_leafFactor<D, T, NB_SPECIAL, R> >  _poolLeaf;      // pool for leaf objects
        mutable SingleAllocator<internals_grid::_node<D, T, R> >  _poolNode;                        // pool for node objects
        mutable SingleAllocator<T, NB_SPECIAL + 1 >  _poolSpec;                                     // pool for special objects

        mutable T* _tabSpecObj[NB_SPECIAL];                                                         // array of T pointers to the special objects.
        mutable uint64 _tabSpecNB[NB_SPECIAL];                                                      // total number of special objects of each type. 
        mutable uint64 _nbNormalObj;                                                                // number of objects which are not special

        mutable internals_grid::_node<D, T, R> _dummyNodes[NB_SPECIAL];                             // dummy nodes array used solely to indicate special objects. 
        
        int64 _minSpec, _maxSpec;                                                                   // min and max value of special objects
        bool _callDtors;                                                                            // should we call the dtors of T objects. 



    };

}

/* now we can include grid_basic */
#include "grid_basic.hpp"

namespace mtools
{

    /* implementation of the assignement operator from grid_basic */
    template<size_t D, typename T, size_t NB_SPECIAL, size_t R> Grid_factor<D, T, NB_SPECIAL, R> & Grid_factor<D, T, NB_SPECIAL, R>::operator=(const Grid_basic<D, T, R> & G)
        {
        std::lock_guard<std::recursive_mutex> lock(_peekmut); // protect from safePeek()
        _reset(0,-1, G._callDtors); // reset the grid and set the special range and dtor flag
        _rangemin = G._rangemin;    // same statistics
        _rangemax = G._rangemax;    //
        _pcurrent = _copyTreeFromGridBasic(nullptr, G._getRoot());  // copy the whole tree structure
        _pcurrentpeek = (_pbox)_pcurrent;
        return *this;
        }



}


/* end of file */

