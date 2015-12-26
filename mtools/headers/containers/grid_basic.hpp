/** @file grid_basic.hpp */
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

#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/metaprog.hpp"
#include "../io/serialization.hpp"
#include "internals_grid.hpp"

#include <string>
#include <atomic>


namespace mtools
{

    /* forward declaration of the Grid_basic class*/
    template< size_t D, typename T, size_t NB_SPECIAL, size_t R> class Grid_factor;


    /**
     * A D-dimensional grid Z^d where each site contain an object of type T. This is a container
     * similar to a D-dimensionnal 'octree'. This is the 'basic' version: fastest get/set method and
     * need only T to be constructible. See Grid_factor for the more advanced (but with more
     * requirement on T) version where similar objects are 'factorized'.
     * 
     * - Each site of Z^d contain a unique objet of type T (which is not shared with any other
     * site). Objects at each sites are created on the fly. They are not deleted until the grid is
     * destroyed or reset.
     * 
     * - Internally, the grid Z^d is represented as a tree whose leafs are elementary sub-boxes of
     * the form [x-R,x+R]^D where R is a template parameter. Nodes of the tree have arity 3^D. The
     * tree grows dynamically to fit the set of sites accessed. This means that an object at some
     * position x may be created even without ever being accesssed.
     * 
     * - Access time to any given site is logarithmic with respect to its distance from the
     * previously accessed element.
     * 
     * - No objet of type T is ever deleted, copied or moved around during the whole life of the
     * grid. Thus, pointers/references to elements are never invalidated. Destructors of all the
     * objects created are called when the grid is destoyed or reset (unless the caller specifically
     * requests not to call the dtors).
     * 
     * - The type T need only be constructible with `T()` or `T(const Pos &)`. If both ctor exist,
     * the positional constructor is used. Furthermore, if T has a copy constructor, then the whole
     * grid object can be copied/assigned via the copy ctor/assignement operator (the assignement
     * operator of T is not used, even when assigning the grid).
     * 
     * - The grid can be serialized to file using the I/OArchive serialization classes and using the
     * default serialization method for T. Furthermore, it is possible to deserialize the object via
     * a specific constructor `T(IArchive &)` (this prevent having to construct a default object
     * first and then deserialize it).
     * 
     * - Grid_basic objects are compatible with Grid_factor objects (with the same template
     * parameters). Files saved with one object can be open with the other one and conversion using
     * copy construtor and assignement operators are implemented in both directions (provided, of
     * course, that the object T fullfills the requirement of Grid_factor and, conversely, that the
     * grid_factor objects do not have special objects at the time of conversion).
     *
     * @tparam  D   Dimension of the grid Z^D.
     * @tparam  T   Type of objects stored in each sites of the grid. This can be any type that has a
     *              public default constructor or a public positionnal constructor.
     * @tparam  R   Radius of an elementary box (leaf of the tree) i.e. each elementary box contain
     *              (2R+1)^D objects.
     *              | Dimension | default value for R
     *              |-----------|-------------------
     *              | 1         | 10000
     *              | 2         | 100
     *              | 3         | 20
     *              | 4         | 6
     *              | >= 5      | 1.
     *
     * @sa  Grid_factor
     **/
    template<size_t D, typename T, size_t R = internals_grid::defaultR<D>::val > class Grid_basic
    {

        template<size_t D2, typename T2, size_t NB_SPECIAL2, size_t R2> friend class Grid_factor;

        typedef internals_grid::_box<D, T, R> *     _pbox;
        typedef internals_grid::_node<D, T, R> *    _pnode;
        typedef internals_grid::_leaf<D, T, R> *    _pleaf;


    public:

        /**
         * Alias for a D-dimensional int64 vector representing a position in the grid.
         **/
        typedef iVec<D> Pos;


        /**
         * Constructor. An empty grid (no objet of type T is created).
         *
         * @param   callDtors   true (default) if we should call the destructor of T object when memory
         *                      is released. Setting this to false can speed up memory relase for basic
         *                      type that do not have 'important' destructors.
         **/
        Grid_basic(bool callDtors = true) : _pcurrent((_pbox)nullptr), _pcurrentpeek((_pbox)nullptr), _rangemin(std::numeric_limits<int64>::max()), _rangemax(std::numeric_limits<int64>::min()), _callDtors(callDtors) 
            { 
            _createBaseNode(); 
            }


        /**
         * Constructor. Loads a grid from a file. If the loading fails, the grid is constructed empty.
         *
         * @param   filename    Filename of the file.
         **/
        Grid_basic(const std::string & filename) : _pcurrent((_pbox)nullptr), _pcurrentpeek((_pbox)nullptr), _rangemin(std::numeric_limits<int64>::max()), _rangemax(std::numeric_limits<int64>::min()), _callDtors(true) {load(filename);}
        
        
        /**
        * Constructor. Loads a grid from a file. If the loading fails, the grid is constructed empty.
        *
        * @param   str    Filename of the file.
        **/
        Grid_basic(const char * str) : _pcurrent((_pbox)nullptr), _pcurrentpeek((_pbox)nullptr), _rangemin(std::numeric_limits<int64>::max()), _rangemax(std::numeric_limits<int64>::min()), _callDtors(true) { load(std::string(str)); } // needed together with the std::string ctor to prevent implicit conversion to bool and call of the wrong ctor.


        /**
         * Destructor. Destroys the grid. The destructors of all the T objects in the grid are invoqued.
         * In order to prevent calling the dtors of T objects, invoque `callDtors(false)` prior to
         * destructing the grid.
         **/
        ~Grid_basic() { _destroyTree(); }


        /**
         * Copy Constructor. Makes a Deep copy of the grid. The source must have the same template
         * parameters D,T,R and the class T must be copyable by the copy operator `T(const T&)`.
         *
         * @param   G   The const Grid_basic<D,T,R> & to process.
         **/
        Grid_basic(const Grid_basic<D, T, R> & G) : _pcurrent((_pbox)nullptr), _pcurrentpeek((_pbox)nullptr), _rangemin(std::numeric_limits<int64>::max()), _rangemax(std::numeric_limits<int64>::min()), _callDtors(true)
            {
            static_assert(std::is_copy_constructible<T>::value, "The object T must be copy-constructible T(const T&) in order to use the copy constructor of the grid.");
            this->operator=(G);
            }


        /**
         * Copy constructor from a grid factor object. The source object must have the same template
         * parameters D,T,R and not have special values (use `Grid_factor::removeSpecialObjects()` to
         * remove special object if needed prior to converting).
         *
         * @param   G   The source Grid_factor object to copy.
         **/
        template<size_t NB_SPECIAL> Grid_basic(const Grid_factor<D, T, NB_SPECIAL, R> & G) : _pcurrent((_pbox)nullptr), _pcurrentpeek((_pbox)nullptr), _rangemin(std::numeric_limits<int64>::max()), _rangemax(std::numeric_limits<int64>::min()), _callDtors(true)
            {
            static_assert(std::is_copy_constructible<T>::value, "The object T must be copy-constructible T(const T&) in order to use the copy constructor of the grid.");
            this->operator=(G);
            }


        /**
         * Assignement operator. Create a deep copy of G. The source object must have the same template
         * parameters D,T,R. The class T must be copyable by the copy operator `T(const T&)` [the
         * assignement operator `operator=(const T &)` is never used and need not be defined]. If the
         * grid is not empty, it is first reset: all the objects inside are destroyed and their dtors
         * are invoqued if the callDtors flag is set.
         *
         * @param   G   the grid to copy.
         *
         * @return  the object for chaining.
         **/
        Grid_basic<D, T, R> & operator=(const Grid_basic<D, T, R> & G)
            {
            static_assert(std::is_copy_constructible<T>::value,"The object T must be copy constructible T(const T&) in order to use assignement operator= on the grid.");
            if ((&G) == this) { return(*this); }
            _destroyTree();
            _rangemin = G._rangemin;
            _rangemax = G._rangemax;
            _pcurrent = _copy(G._getRoot(),nullptr);
            _pcurrentpeek = (_pbox)_pcurrent;
            _callDtors = G._callDtors;
            return(*this);
            }


        /**
         * Assignment operator. Create a deep copy of a Grid_factor object. The source must have the
         * same template parameters D,T,R and not have special values (use
         * `Grid_factor::removeSpecialObjects()` to remove special object if needed prior to
         * converting). If the grid is not empty, it is first reset: all the objects inside are
         * destroyed and their dtors are invoqued if the callDtors flag is set.
         *
         * @param   G   The source Grid_factor object to copy.
         *
         * @return  The object for chaining.
         **/
        template<size_t NB_SPECIAL> Grid_basic<D, T, R> & operator=(const Grid_factor<D, T, NB_SPECIAL, R> & G);


        /**
         * Resets the grid to its initial state. Call the destructor of all the T objects if the flag
         * callDtors is set. When the method returns, there are no living T object inside the grid.
         **/
        void reset() { _destroyTree(); _createBaseNode(); }


        /**
         * Serializes the grid into an OArchive. If T implement a serialize method recognized by
         * OArchive, it is used for serialization otherwise OArchive uses its default serialization
         * method (which correspond to a basic memcpy() of the object memory).
         *
         * @param [in,out]  ar  The archive object to serialise the grid into.
         *
         * @sa  class OArchive, class IArchive
         **/
        void serialize(OArchive & ar) const
            {
            ar << "\nBegining of Grid_basic<" << D << " , [" << std::string(typeid(T).name()) << "] , " << R << ">\n";
            ar << "Version";    ar & ((uint64)1); ar.newline();
            ar << "Template D"; ar & ((uint64)D); ar.newline();
            ar << "Template R"; ar & ((uint64)R); ar.newline();
            ar << "object T";   ar & std::string(typeid(T).name()); ar.newline();
            ar << "sizeof(T)";  ar & ((uint64)sizeof(T)); ar.newline();
            ar << "call dtors"; ar & _callDtors; ar.newline();
            ar << "_rangemin";  ar & _rangemin; ar.newline();
            ar << "_rangemax";  ar & _rangemax; ar.newline();
            ar << "_minSpec";   ar & ((int64)0); ar.newline();
            ar << "_maxSpec";   ar & ((int64)-1); ar.newline();
            ar << "Grid tree\n";
            _serializeTree(ar, _getRoot());
            ar << "\nEnd of Grid_basic<" << D << " , [" << std::string(typeid(T).name()) << "] , " << R << ">\n";
            }


        /**
         * Deserializes the grid from an IArchive. If T has a constructor of the form T(IArchive &), it
         * is used for deserializing the T objects in the grid. Otherwise, if T implements one of the
         * serialize methods recognized by IArchive, the objects in the grid are first position/default
         * constructed and then deserialized using those methods. If no specific deserialization method
         * is found, IArchive falls back to its default derialization method.
         * 
         * If the grid is non-empty, it is first reset, possibly calling the ctor of the existing T
         * objects depending on the status of the callCtor flag.
         *
         * @param [in,out]  ar  The archive to deserialize the grid from.
         *
         * @sa  class OArchive, class IArchive
         **/
        void deserialize(IArchive & ar)
            {
            try
                {
                _destroyTree();
                uint64 ver;         ar & ver;      if (ver != 1) { MTOOLS_THROW("wrong version");}
                uint64 d;           ar & d;        if (d != D) { MTOOLS_THROW("wrong dimension");}
                uint64 r;           ar & r;        if (r != R) { MTOOLS_THROW("wrong R parameter");}
                std::string stype;  ar & stype;
                uint64 sizeofT;     ar & sizeofT;  if (sizeofT != sizeof(T)) { MTOOLS_THROW("wrong sizeof(T)");}
                ar & _callDtors;
                ar & _rangemin;
                ar & _rangemax;
                int64 minSpec; ar & minSpec;
                int64 maxSpec; ar & maxSpec;
                if (minSpec <= maxSpec) { MTOOLS_THROW("The file contain special objects hence must be opened using a Grid_factor instead of a Grid_basic object"); }
                _pcurrent = _deserializeTree(ar, nullptr);
                _pcurrentpeek = (_pbox)_pcurrent;
                }
            catch(...)
                {
                _callDtors = false;
                _destroyTree();    // object are dumped into oblivion, may result in a memory leak.
                _callDtors = true;
                _createBaseNode();
                MTOOLS_THROW("Aborting deserialization of Grid_basic object");
                }
            }


        /**
         * Saves the grid into a file (using the archive format). The file is compressed if it ends by
         * the extension ".gz", ".gzip" or ".z".
         * 
         * Grid file for grid_Factor and Grid_basic are compatible so that the file can subsequently be
         * opened with a Grid_factor object (provided the template parameters T, R and D are the same
         * and T fullfills the requirement of Grid_factor).
         * 
         * The method simply call serialize() to create the archive file.
         *
         * @param   filename    The filename to save.
         *
         * @return  true on success, false on failure.
         *
         * @sa  load, serialize, deserialize, class OArchive, class IArchive
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
                MTOOLS_DEBUG("Error saving Grid_basic object");
                return false;
                } // error
            return true; // ok
            }


        /**
         * Loads a grid from a file. Grid files are compatible between classes hence this method can
         * also load file created from a grid_factor class provided that the grid_factor object had no
         * special object at the time of saving the file.
         * 
         * If the grid is non-empty, it is first reset, possibly calling the dtors of T object depending
         * on the status of the callDtor flag.
         * 
         * The method simply call deserialize() to recreate the grid from the archive file.
         *
         * @param   filename    The filename to load.
         *
         * @return  true on success, false on failure [in this case, the lattice is reset].
         *
         * @sa  save, class OArchive, class IArchive
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
                MTOOLS_DEBUG("Error loading Grid_basic object");
                _callDtors = false; _destroyTree(); _callDtors = true; _createBaseNode(); 
                return false; 
                } // error
            return true; // ok
            }


        /**
         * Return the range of elements accessed. The method returns an empty box if no element 
         * was ever accessed.
         *
         * @param [in,out]  rangeBox    Box to put the range into.
         **/
        inline void getPosRange(iBox<D> & rangeBox) const 
            { 
            rangeBox.min = _rangemin; 
            rangeBox.max = _rangemax;
            }


        /**
         * Check if we should call the destructors of T objects when they are not needed anymore.
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
        * Return the memory currently allocated by the grid (in bytes).
        **/
        size_t memoryAllocated() const { return sizeof(*this) + _poolLeaf.footprint() + _poolNode.footprint(); }


        /**
        * Return the memory currently used by the grid (in bytes).
        **/
        size_t memoryUsed() const { return sizeof(*this) + _poolLeaf.used() + _poolNode.used(); } 


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
            s += std::string("Grid_basic<") + mtools::toString(D) + " , " + typeid(T).name() + " , " + mtools::toString(R) + ">\n";
            s += std::string(" - Memory : ") + mtools::toStringMemSize(memoryUsed()) + " / " + mtools::toStringMemSize(memoryAllocated()) + "\n";
            s += std::string(" - Range min = ") + _rangemin.toString(false) + "\n";
            s += std::string(" - Range max = ") + _rangemax.toString(false) + "\n";
            if (debug) { s += "\n" + _printTree(_getRoot(), ""); }
            return s;
            }


        /**
         * Sets the value at a given site. This method require T to be assignable via T.operator=. If the
         * value at the site does not exist prior to the call of the method, it is first created then the
         * assignement is performed.
         *
         * @param   pos The position of the site to access.
         * @param   val The value to set.
         **/
        inline void set(const Pos & pos, const T & val) { _get(pos) = val; }


        /**
        * Set a value at a given position. Dimension 1 specialization.
        **/
        inline void set(int64 x, const T & val) { static_assert(D == 1, "template parameter D must be 1"); _get(Pos(x)) = val; return; }


        /**
        * Set a value at a given position. Dimension 2 specialization.
        **/
        inline void set(int64 x, int64 y, const T & val) { static_assert(D == 2, "template parameter D must be 2"); _get(Pos(x,y)) = val; return; }


        /**
        * Set a value at a given position. Dimension 3 specialization.
        **/
        inline void set(int64 x, int64 y, int64 z, const T & val) { static_assert(D == 3, "template parameter D must be 3"); _get(Pos(x,y,z)) = val; return; }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         *
         * @param   pos The position.
         *
         * @return  A reference to the value.
         **/
        inline T & get(const Pos & pos) { return _get(pos); }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         * (const version)
         *
         * @param   pos The position.
         *
         * @return  A const reference to the value.
         **/
        inline const T & get(const Pos & pos) const { return _get(pos); }


        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created. Dimension 1 specialization.
        **/
        inline T & get(int64 x) { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }

        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created. Dimension 1 specialization. (const version)
        **/
        inline const T & get(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }


        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created. Dimension 2 specialization.
        **/
        inline T & get(int64 x, int64 y) { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created. Dimension 2 specialization. (const version)
        **/
        inline const T & get(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created. Dimension 3 specialization.
        **/
        inline T & get(int64 x, int64 y, int64 z) { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created. Dimension 3 specialization. (const version)
        **/
        inline const T & get(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         *
         * @param   pos The position.
         *
         * @return  A reference to the value.
         **/
        inline T & operator[](const Pos & pos) { return _get(pos); }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         * (const version)
         *
         * @param   pos The position.
         *
         * @return  A const reference to the value.
         **/
        inline const T & operator[](const Pos & pos) const { return _get(pos); }


        /**
        * Get a value at a given position. If the T object at that site does not exist, it is created.
        * Same as the get() method.
        *
        * @param   pos The position.
        *
        * @return  A reference to the value.
        **/
        inline T & operator()(const Pos & pos) { return _get(pos); }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created
         * Same as the get() method. (const version).
         *
         * @param   pos The position.
         *
         * @return  A reference to the value.
         **/
        inline const T & operator()(const Pos & pos) const { return _get(pos); }


        /**
        * get a value at a given position. Dimension 1 specialization.
        * This creates the object if it does not exist yet.
        **/
        inline T & operator()(int64 x) { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }

        /**
         * get a value at a given position. Dimension 1 specialization. (const version)
         * This creates the object if it does not exist yet.
         **/
        inline const T & operator()(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }


        /**
        * get a value at a given position. Dimension 2 specialization.
        * This creates the object if it does not exist yet.
        **/
        inline T & operator()(int64 x, int64 y) { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
         * get a value at a given position. Dimension 2 specialization. (const version)
         * This creates the object if it does not exist yet.
         **/
        inline const T & operator()(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
        * get a value at a given position. Dimension 3 specialization.
        * This creates the object if it does not exist yet.
        **/
        inline T & operator()(int64 x, int64 y, int64 z) { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
         * get a value at a given position. Dimension 3 specialization. (const version)
         * This creates the object if it does not exist yet.
         **/
        inline const T & operator()(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
        * Return a pointer to the object at a given position. If the value at the site was not yet
        * created, returns nullptr. This method does not create sites and is suited when drawing the
        * lattice using, for instance, the LatticeDrawer class.
        *
        * This method is threadsafe : it is safe to call peek() while accessing the object via a get()
        * or a set() or one of their operator [], operator() equivalents. It may even be called while
        * the grid is being modified by assign/load operations. However, peek() should not be called
        * while another peek() is in progress. To perform multiple simulatneous peek, use the peek
        * version with hint below.
        *
        * The implementation of this method is lock free hence there is zero penalty to peeking while
        * still reading/setting the object !
        *
        * @param   pos The position to peek.
        *
        * @return  nullptr if the value at that site was not yet created. A const pointer to it
        *          otherwise.
        **/
        inline const T * peek(const Pos & pos) const
            {
            _pbox c = _pcurrentpeek;
            if (c == nullptr) return nullptr;
            // check if we are at the right leaf
            if (c->isLeaf())
                {
                _pleaf p = (_pleaf)(c);
                if (p->isInBox(pos)) return(&(p->get(pos)));
                c = p->father;
                if (c == nullptr) return nullptr;
                }
            // no we go up...
            _pnode q = (_pnode)(c);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { _pcurrentpeek = q; return nullptr; }
                q = (_pnode)q->father;
                }
            // and down...
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) { _pcurrentpeek = q; return nullptr; }
                if (b->isLeaf()) { _pcurrentpeek = b; return(&(((_pleaf)b)->get(pos))); }
                q = (_pnode)b;
                }
            }


        /**
        * Return a pointer to the object at a given position. If the value at the site was not yet
        * created, returns nullptr. This method does not create sites and is suited when drawing the
        * lattice using, for instance, the LatticeDrawer class.
        *
        * This version is similar to the classical peek version but use an additonal hint parameter
        * which enable several simultaneous peek. The hint parameter must be set to nullptr for the
        * first call and then, the value modified after a call to peek must forwarded to the next
        * peek().
        *
        *  Using this version of peek is perfectly threadsafe. The object can be simultaneously
        *  modified, peeked (with or without hint) or even assigned/loaded !
        *
        * The implementation of this method is lock free hence there is zero penalty to peeking while
        * still reading/setting the object !
        *
        * @param   pos             The position to peek.
        * @param [in,out]  hint    the hint pointer. Must be set to nullptr for the first call and then
        *                          the same variable must be forwarded on each subsequent call.
        *
        * @return  nullptr if the value at that site was not yet created. A const pointer to it
        *          otherwise.
        **/
        inline const T * peek(const Pos & pos, void* & hint) const
            {
            if (hint == nullptr) { hint = _pcurrentpeek; }
            _pbox c = (_pbox)hint;
            if (c == nullptr) return nullptr;
            // check if we are at the right leaf
            if (c->isLeaf())
                {
                _pleaf p = (_pleaf)(c);
                if (p->isInBox(pos)) return(&(p->get(pos)));
                c = p->father;
                if (c == nullptr) return nullptr;
                }
            // no we go up...
            _pnode q = (_pnode)(c);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { hint = q; return nullptr; }
                q = (_pnode)q->father;
                }
            // and down...
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) { hint = q; return nullptr; }
                if (b->isLeaf()) { hint = b; return(&(((_pleaf)b)->get(pos))); }
                q = (_pnode)b;
                }
            }


        /**
         * peek at a value at a given position. Dimension 1 specialization.
         * 
         * This method is threasafe wrt get/set : see peek() documentation for details.
         * 
         * Returns nullptr if the object at this position is not created.
         **/
        inline const T * peek(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return peek(Pos(x)); }


        /**
         * peek at a value at a given position. Dimension 2 specialization.
         * 
         * This method is threasafe wrt get/set : see peek() documentation for details.
         * 
         * Returns nullptr if the object at this position is not created.
         **/
        inline const T * peek(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return peek(Pos(x, y)); }


        /**
         * peek at a value at a given position. Dimension 3 specialization.
         *
         * This method is threasafe wrt get/set : see peek() documentation for details.
         *
         * Returns null if the object at this position is not created.
         **/
        inline const T * peek(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return peek(Pos(x, y, z)); }


        /**
         * Find a box containing position pos such that all the points inside the (closed) box have the
         * same special value or are all undefined. Contrarily to Grid_factor object, Grid_basic objects
         * have no NO special value so this method is useful only for finding empty boxes where all the
         * elements have not yet been defined.
         * 
         * The coordinates of the resulting box are put in outBox and the function return nullptr if an 
         * empty box was found or a pointer to the element at pos otherwise. If it does not return nullptr, 
         * then no box was found and the method sets outBox to the single point pos.
         * 
         * The box returned is always a square and correspond to the largest empty box containing pos in
         * the "quadtree-like" grid structure.
         * 
         * Although the box returned always contain position pos, it can be close (or even on) the
         * boundary of the box which may be undesired. To get another box where pos is further away from
         * the boundary, use findFullBoxCentered() instead.
         * 
         * @warning This method is NOT threadsafe and uses the same pointer as get() and set().
         *
         * @param   pos             The position to check.
         * @param [in,out]  outBox  The box to put the solution.
         * 
         * @return  A pointer to the element at position pos or nullptr if it does not exist.
         **/
        inline const T * findFullBox(const Pos & pos, iBox<D> & outBox) const
            {
            Pos & boxMin = outBox.min;
            Pos & boxMax = outBox.max;
            _pbox cp = _pcurrent;
            MTOOLS_ASSERT(cp != nullptr);
            // check if we are at the right place
            if (cp->isLeaf())
                {
                _pleaf p = (_pleaf)(cp);
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
                    int64 r = 3 * q->rad + 1;
                    for (size_t i = 0; i < D; ++i)
                        {
                        int64 u = pos[i]; if (u < 0) { u = -u; }
                        while (u > r) { r = 3 * r + 1; }
                        }
                    // r is the radius of the box containing pos
                    r = (r - 1) / 3;
                    for (size_t i = 0; i < D; i++)
                        {
                        const int64 a = pos[i];
                        const int64 sb = ((a < -r) ? (-(2 * r + 1)) : ((a > r) ? (2 * r + 1) : 0));
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
                if (b->isLeaf())
                    {
                    boxMin = pos; boxMax = pos;
                    _pcurrent = b;
                    return(&(((_pleaf)b)->get(pos))); // just a singleton
                    }
                q = (_pnode)b;
                }
            }


        /**
        * Improve findFullBox() by trying to find an empty box where position pos is further away 
        * from its boundary. As for findFullBox(), this method only finds boxes in which no
        * element have been defined yet (since there is no special objects). See the eponym method 
        * of the grid Grid_factor class for additionnal details.
        *
        *  @warning This method is NOT threadsafe and uses the same pointer as get() and set().
        *
        * @param   pos         The position to check.
        * @param [in,out]  bestRect   The box to put the solution.
        *
        * @return  Nullptr if the element at pos is not yet defined, a pointer to it otherwise.
        **/
        inline const T * findFullBoxCentered(const Pos & pos, iBox<D> & bestRect) const
            {
            static_assert(D == 2, "findFullBoxCentered() only implemented for dimension 2 yet...");
            const T* pv = findFullBox(pos, bestRect); // get the non optimized box.
            if ((pv != nullptr)||(bestRect.lx() == 0)) return pv;   // no box found, nothing more to do.

            iBox2 baseRect = bestRect;                  // the base rectangle is the best rectangle.
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
            #define box1Up iBox2(baseRect.min[0], baseRect.max[0], baseRect.min[1], baseRect.max[1] + diambase)
            #define box1Down iBox2(baseRect.min[0], baseRect.max[0], baseRect.min[1] - diambase, baseRect.max[1])
            #define box1Left iBox2(baseRect.min[0] - diambase, baseRect.max[0], baseRect.min[1], baseRect.max[1])
            #define box1Right iBox2(baseRect.min[0], baseRect.max[0] + diambase, baseRect.min[1], baseRect.max[1])
            #define line2UpDown iBox2(baseRect.min[0], baseRect.max[0], baseRect.min[1] - diambase, baseRect.max[1] + diambase)
            #define line2LeftRight iBox2(baseRect.min[0] - diambase, baseRect.max[0] + diambase, baseRect.min[1], baseRect.max[1])
            #define box2UpLeft iBox2(baseRect.min[0] - diambase, baseRect.max[0], baseRect.min[1], baseRect.max[1] + diambase)
            #define box2UpRight iBox2(baseRect.min[0], baseRect.max[0] + diambase, baseRect.min[1], baseRect.max[1] + diambase)
            #define box2DownLeft iBox2(baseRect.min[0] - diambase, baseRect.max[0], baseRect.min[1] - diambase, baseRect.max[1])
            #define box2DownRight iBox2(baseRect.min[0], baseRect.max[0] + diambase, baseRect.min[1] - diambase, baseRect.max[1])
            #define rect3Up iBox2(baseRect.min[0] - diambase, baseRect.max[0] + diambase, baseRect.min[1], baseRect.max[1] + diambase)
            #define rect3Down iBox2(baseRect.min[0] - diambase, baseRect.max[0] + diambase, baseRect.min[1] - diambase, baseRect.max[1])
            #define rect3Left iBox2(baseRect.min[0] - diambase, baseRect.max[0], baseRect.min[1] - diambase, baseRect.max[1] + diambase)
            #define rect3Right iBox2(baseRect.min[0], baseRect.max[0] + diambase, baseRect.min[1] - diambase, baseRect.max[1] + diambase)
            #define rect4 iBox2(baseRect.min[0] - diambase, baseRect.max[0] + diambase, baseRect.min[1] - diambase, baseRect.max[1] + diambase)

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
                    baseRect.min[0] = newcenter.X() - nrad; baseRect.max[0] = newcenter.X() + nrad; // set the new base
                    baseRect.min[1] = newcenter.Y() - nrad; baseRect.max[1] = newcenter.Y() + nrad;
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


        static_assert(D > 0, "template parameter D (dimension) must be non-zero");
        static_assert(R > 0, "template parameter R (radius) must be non-zero");
        static_assert(std::is_constructible<T>::value || std::is_constructible<T, Pos>::value, "The object T must either be default constructible T() or constructible with T(const Pos &)");


        /* Used by findFullBoxCentered (2D specialization). Check if a given border adjacent box of same size is full. */
        inline void _checkBorder(int & flag, const int64 diam, const T * pv, const iBox2 & refBox2, const Pos & borderPos, int flagBorder, const Pos & cornerPos1, int flagCorner1, const Pos & cornerPos2, int flagCorner2) const
            {
            if (refBox2.isInside(borderPos)) { flag |= flagBorder; return; } // if borderPos is in the reference rectangle, set to 1 and do not check the corner positions
            iBox2 RR;
            if ((findFullBox(borderPos, RR) != pv) || (RR.lx() == 0)) { return; } // if the box is a singleton or does not contain the correct value, we quit
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
        inline void _checkCorner(int & flag, const int64 diam, const T * pv, const iBox2 & refBox2, const Pos & cornerPos, int flagCorner) const
            {
            if (flag & flagCorner) return; // alread set, nothing to do
            if (refBox2.isInside(cornerPos)) { flag |= flagCorner; return; } // if pos is in the reference rectangle, set the flag
            iBox2 RR;
            if ((findFullBox(cornerPos, RR) != pv) || (RR.lx() == 0)) { return; } // if the box is a singleton or does not contain the correct value, we quit
            if ((RR.lx() + 1) < diam) { return; } // box too small
            flag |= flagCorner;
            return;
            }


        /* Used by findFullBoxCentered (2D specialization). Compute the new best rectangle */
        inline void _extendWith(iBox2 & currentBest, int64 & lbest, iBox2 newRect, const Pos & pos) const
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


        /* get sub method
         * the tree is consistent at all time */
        inline T & _get(const Pos & pos) const
            {
            MTOOLS_ASSERT(_pcurrent != (_pbox)nullptr);
            _pbox c = _pcurrent;
            _updaterange(pos);
            if (c->isLeaf())
                {
                if (((_pleaf)c)->isInBox(pos)) { return(((_pleaf)c)->get(pos)); }
                MTOOLS_ASSERT(c->father != nullptr); // a leaf must always have a father
                c = c->father;
                }
            // going up...
            _pnode q = (_pnode)c;
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { q->father = _allocateNode(q); }
                q = (_pnode)q->father;
                }
            // ...and down
            while(1)
                {
                _pbox & b = q->getSubBox(pos);
                if (b == nullptr)
                    {
                    if (q->rad == R)
                        {
                        b = _allocateLeaf(q, q->subBoxCenter(pos));
                        T & result = ((_pleaf)b)->get(pos);
                        _pcurrent = b;
                        return(result);
                        }
                    q = _allocateNode(q, q->subBoxCenter(pos), nullptr);
                    b = q;
                    }
                else
                    {
                    if (b->isLeaf()) { T & result = ((_pleaf)b)->get(pos); _pcurrent = b; return(result); }
                    q = (_pnode)b;
                    }
                }
            }


        /* update _rangemin and _rangemax */
        inline void _updaterange(const Pos & pos) const
            {
            for (size_t i = 0; i < D; i++) 
                {
                const int64 x = pos[i];
                if (x < _rangemin[i]) { _rangemin[i] = x; }  
                if (x > _rangemax[i]) { _rangemax[i] = x; }
                }
            }


        /* get the root of the tree */
        inline _pbox _getRoot() const
            {
            if (_pcurrent == ((_pbox)nullptr)) return nullptr;
            _pbox p = _pcurrent;
            while (p->father != nullptr) { p = p->father; }
            return p;
            }


        /* print the tree, for debug purpose only */
        std::string _printTree(_pbox p, std::string tab) const
            {
            if (p == nullptr) { return(tab + "NULLPTR\n"); }
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


        /* recursive method for serialization of the tree */
        template<typename ARCHIVE> void _serializeTree(ARCHIVE & ar, _pbox p) const
            {
            if (p == nullptr) { ar & ((char)'V'); return; } // void pointer
            if (p->isLeaf())
                {
                ar & ((char)'L');
                ar & p->center;
                ar & p->rad;
                for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)  { ar & (((_pleaf)p)->data[i]); }
                return;
                }
            ar & ((char)'N');
            ar & p->center;
            ar & p->rad;
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i)  { _serializeTree(ar,((_pnode)p)->tab[i]); }
            return;
            }


        /* reconstruction of the object of a leaf from the constructor T(IArchive &)*/
        inline void _reconstructLeaf(IArchive & ar, T * data, Pos center, metaprog::dummy<true> dum)
            {
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)  { new(data + i) T(ar); }
            }


        /* reconstruction of the object of a leaf when no constructor from archive */
        inline void _reconstructLeaf(IArchive & ar, T * data, Pos center, metaprog::dummy<false> dum)
            {
            _createDataLeaf(data, center, metaprog::dummy<std::is_constructible<T, Pos>::value>());          // first call the default or the positionnal ctor for every object
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)  { ar &  (data[i]); }        // and then deserializes
            }


        /* recursive method for deserialization of the tree */
        template<typename ARCHIVE> _pbox _deserializeTree(ARCHIVE & ar, _pbox father)
            {
            char c;
            ar & c;
            if (c == 'V') return nullptr;
            if (c == 'L')
                {
                MTOOLS_ASSERT(father->rad == R);
                _pleaf p = _poolLeaf.allocate();
                ar & p->center;
                ar & p->rad;
                MTOOLS_ASSERT(p->rad == 1);
                p->father = father;
                _reconstructLeaf(ar, p->data, p->center, metaprog::dummy< std::is_constructible<T, IArchive>::value>());
                return p;
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
            MTOOLS_THROW(std::string("Unknown tag [") + std::string(1, c) + "]");
            return nullptr;
            }


        /* Release all the allocated  memory and reset the tree */
        void _destroyTree()
            {
            _pcurrentpeek = nullptr;
            _pcurrent = nullptr;
            _rangemin.clear(std::numeric_limits<int64>::max());
            _rangemax.clear(std::numeric_limits<int64>::min());
            _poolNode.destroyAll();
            if (_callDtors) { _poolLeaf.destroyAll(); } else { _poolLeaf.deallocateAll(); }
            return;
            }


        /* make a copy of the subtree starting from pg
        and set the father of the root to pere. Return the root
        of the sub tree created. */
        _pbox _copy(_pbox pg, _pbox pere)
            {
            if (pg == nullptr) { return nullptr; }
            if (pg->isLeaf())
                {
                _pleaf p = _poolLeaf.allocate();
                p->center = pg->center;
                p->rad = pg->rad;
                p->father = pere;
                for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i) { new(p->data + i) T(((_pleaf)pg)->data[i]); }
                return p;
                }
            _pnode p = _poolNode.allocate();
            p->center = pg->center;
            p->rad = pg->rad;
            p->father = pere;
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = _copy(((_pnode)pg)->tab[i], p); }
            return p;
           }


        /* create the data for an elementary sub box using the T(const Pos & v) version */
        inline void _createDataLeaf(T * data, Pos pos, metaprog::dummy<true> dum) const
            {
            Pos center = pos;
            for (size_t i = 0; i < D; ++i) { pos[i] -= R; }
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                new(data + x) T(pos);
                for (size_t i = 0; i < D; ++i)
                    {
                    if (pos[i] < (center[i] + (int64)R)) { pos[i]++;  break; }
                    pos[i] -= (2 * R);
                    }
                }
            }


        /* create the data for an elementary sub box using the default T() version */
        inline void _createDataLeaf(T * data, Pos pos, metaprog::dummy<false> dum) const { for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i) { new(data + i) T(); } }


        /* Allocate a leaf, call constructor from above with default initialization (ie either T() or T(Pos) */
        inline _pleaf _allocateLeaf(_pbox above, const Pos & centerpos) const
            {
            _pleaf p = _poolLeaf.allocate();
            _createDataLeaf(p->data, centerpos, metaprog::dummy<std::is_constructible<T,Pos>::value>());
            p->center = centerpos;
            p->rad = 1;
            p->father = above;
            return p;
            }


        /* create the base node which centers the tree around the origin */
        inline void _createBaseNode()
            {
            MTOOLS_ASSERT(_pcurrent == (_pbox)nullptr);   // should only be called when the tree dos not exist.
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = nullptr; }
            p->center = Pos(0);
            p->rad = R;
            p->father = nullptr;
            _pcurrent = p;
            _pcurrentpeek = p;
            return;
            }


        /* Allocate a node, call constructor from above */
        inline _pnode _allocateNode(_pbox above, const Pos & centerpos, _pbox fill) const
            {
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = fill; }
            p->center = centerpos;
            p->rad = (above->rad - 1)/3;
            p->father = above;
            return p;
            }


        /* Allocate a node, call constructor from below */
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


        mutable std::atomic<_pbox> _pcurrent;        // pointer to the current box
        mutable std::atomic<_pbox> _pcurrentpeek;    // pointer to the current box for peek operations
        mutable Pos   _rangemin;        // the minimal range
        mutable Pos   _rangemax;        // the maximal range
        bool _callDtors;                // should we call the destructors

        mutable SingleAllocator<internals_grid::_leaf<D, T, R> >  _poolLeaf;       // the two memory pools
        mutable SingleAllocator<internals_grid::_node<D, T, R> >  _poolNode;       //

    };


}


/* now we can include grid_factor */
#include "grid_factor.hpp"


namespace mtools
{


    template<size_t D, typename T, size_t R> template<size_t NB_SPECIAL> Grid_basic<D, T, R> & Grid_basic<D, T, R>::operator=(const Grid_factor<D, T, NB_SPECIAL, R> & G)
        {
        static_assert(std::is_copy_constructible<T>::value, "The object T must be copy constructible T(const T&) in order to use assignement operator= on the grid.");
        MTOOLS_INSURE(G._specialRange() <= 0); // there must not be any special objects. 
        _destroyTree();
        _rangemin = G._rangemin;
        _rangemax = G._rangemax;
        _pcurrent = _copy(G._getRoot(), nullptr);
        _pcurrentpeek = (_pbox)_pcurrent;
        _callDtors = G._callDtors;
        return(*this);
        }



}

/* end of file */




