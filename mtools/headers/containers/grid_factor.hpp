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
    template < size_t D, typename T, size_t NB_SPECIAL = 256 , size_t R = internals_grid::defaultR<D>::val > class Grid_factor
    {

        typedef internals_grid::_box<D, T, R> *     _pbox;
        typedef internals_grid::_node<D, T, R> *    _pnode;
        typedef internals_grid::_leafFactor<D, T, NB_SPECIAL, R> *    _pleafFactor;

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
        Grid_factor(int64 minSpecial = 0, int64 maxSpecial = -1, bool callDtors = true)
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
            _reset(0, -1, true);
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
        template<size_t NB_SPECIAL2> Grid_factor(const Grid_factor<D, T, NB_SPECIAL2, R> & G)
            {
            _reset(0, -1, true);
            this->operator=(G);
            }


        /**
        * Assignement operator. Create a deep copy of G. The class T must be copyable by the copy
        * operator T(const T&). The assignement operator `operator=(const T &)` is never used and need
        * not be defined. If the grid is not empty, it is first reset : all the object inside are
        * destroyed and their dtors are invoqued.
        **/
        template<size_t NB_SPECIAL2> Grid_factor<D, T, NB_SPECIAL2, R> & operator=(const Grid_factor<D, T, NB_SPECIAL2, R> & G)
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



        void serialize(OArchive & ar)
            {
                // *********************** TODO ************************
            }
        

        void deserialize(IArchive & ar)
            {
                // *********************** TODO ************************
            }


        bool save(const std::string & filename) const
            {
                // *********************** TODO ************************
            }


        bool load(std::string filename)
            {
                // *********************** TODO ************************
            }



        /**
        * Sets the value at a given site. This method require T to be assignable via T.operator=. If the
        * value at the site does not exist prior to the call of the method, it is first created then the
        * assignement is performed.
        *
        * @param   pos The position of the site to access.
        * @param   val The value to set.
        **/
        inline void set(const Pos & pos, const T & val) { _set(pos,&val); }


        /**
        * Get a value at a given position.
        *
        * @param   pos The position.
        *
        * @return  A const reference to the value at that position.
        **/
        inline const T & get(const Pos & pos) const { return _get(pos); }


        /**
         * Return a pointer to the object at a given position. If the value at the site was not yet
         * created, returns nullptr. This method modify the object at all and is particularly suited
         * when drawing the lattice using, for instance, the LatticeDrawer class.
         *
         * @param   pos The position to peek.
         *
         * @return  nullptr if the value at that site was not yet created. A const pointer to it
         *          otherwise.
         **/
        inline const T * peek(const Pos & pos) const
            {
            MTOOLS_ASSERT(_pcurrent != nullptr);
            // check if we are at the right place
            if (_pcurrent->isLeaf())
                {
                _pleafFactor p = (_pleafFactor)(_pcurrent);
                if (p->isInBox(pos)) return(&(p->get(pos)));
                MTOOLS_ASSERT(_pcurrent->father != nullptr); // a leaf must always have a father
                _pcurrent = p->father;
                }
            // no, going up...
            _pnode q = (_pnode)(_pcurrent);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { _pcurrent = q; return nullptr; }
                q = (_pnode)q->father;
                }
            // and down..
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) { _pcurrent = q; return nullptr; }
                T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                if (obj != nullptr) { return obj; }
                if (b->isLeaf()) { _pcurrent = b; return(&(((_pleafFactor)b)->get(pos))); }
                q = (_pnode)b;
                }
            }



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
            s += std::string(" - Memory used : ") + mtools::toString((_poolLeaf.footprint() + _poolNode.footprint() + _poolSpec.footprint()) / (1024 * 1024)) + "MB\n";
            s += std::string(" - Range min = ") + _rangemin.toString(false) + "\n";
            s += std::string(" - Range max = ") + _rangemax.toString(false) + "\n";
            s += std::string(" - Special object value range [") + mtools::toString(_minSpec) + " , " + mtools::toString(_maxSpec) + "]\n";         
            for (int i = 0;i < (_maxSpec - _minSpec + 1); i++)
                {
                s += std::string(" - [") + ((_tabSpecObj[i] == nullptr) ? " " : "X") + "] value (" + mtools::toString(_minSpec + i) + ") = " + mtools::toString(_tabSpecNB[i]) + "\n";
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
        static_assert(std::is_convertible<T, int64>::value, "The object T must be convertible to int64");
        static_assert(metaprog::has_assignementOperator<T>::value, "The object T must be assignable via operator=()");


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


        /* update _rangemin and _rangemax */
        inline void _updaterange(const Pos & pos) const
        {
            if (_rangemax[0] < _rangemin[0]) { _rangemin = pos; _rangemax = pos; return; }
            for (size_t i = 0; i < D; i++) { if (pos[i] < _rangemin[i]) { _rangemin[i] = pos[i]; } else if (pos[i] > _rangemax[i]) { _rangemax[i] = pos[i]; } }
        }



        /* set method */
        inline void _set(const Pos & pos, const T * val) 
            {
            MTOOLS_ASSERT(_pcurrent != nullptr);
            _updaterange(pos);
            if (_pcurrent->isLeaf())
                {
                if (((_pleafFactor)_pcurrent)->isInBox(pos)) 
                    { 
                    _pcurrent = _setLeafValue(val, pos, (_pleafFactor)_pcurrent); //set the value and then simplify if needed
                    return; 
                    }
                MTOOLS_ASSERT(_pcurrent->father != nullptr); // a leaf must always have a father
                _pcurrent = _pcurrent->father;
                }
            // going up...
            _pnode q = (_pnode)_pcurrent;
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
                        int64 cv;
                        b = _allocateLeaf(q, q->subBoxCenter(pos), cv); // create the leaf
                        _pcurrent = _setLeafValue(val, pos, (_pleafFactor)b); // set the value and then factorize if needed 
                        return;
                        }
                    // create the subnode
                    q = _allocateNode(q, q->subBoxCenter(pos), nullptr);
                    b = q;
                    }
                else
                    {
                    T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                    if (obj != nullptr) 
                        { // yes
                        int64 nv = (int64)(*val);       // the new object value
                        int64 ov = _getSpecialValue(b); // the current object special value for theis region
                        if (ov == nv) { _pcurrent = q; return; } // same values so there is nothing to do really
                        // not the same value, we must partially expand the tree.  
                        _pbox dum = b; // the dummy link
                        while (1)
                            {
                            _pbox & bb = q->getSubBox(pos);
                            if (q->rad == R)
                                { // the subbox to create is a leaf
                                bb = _allocateLeafCst(q, q->subBoxCenter(pos),obj,ov); // create the leaf with all element equal to the special object
                                _pcurrent = _setLeafValue(val, pos, (_pleafFactor)bb); // set the value (and then factorize if needed but here nothing is done) 
                                return;
                                }
                            // the subbox to create is a node
                            q = _allocateNode(q, q->subBoxCenter(pos), dum); // create the subnode and fill its tab with the same dummy value
                            bb = q;
                            }
                        } 
                    // no, it is a real link
                    if (b->isLeaf()) 
                        {
                        _pcurrent = _setLeafValue(val, pos, (_pleafFactor)b); //set the value and then simplify if needed
                        return; 
                        }
                    q = (_pnode)b;
                    }
                }
            }


        /* get method */
        inline const T & _get(const Pos & pos) const
            {
            MTOOLS_ASSERT(_pcurrent != nullptr);
            _updaterange(pos);
            if (_pcurrent->isLeaf())
                {
                if (((_pleafFactor)_pcurrent)->isInBox(pos)) { return(((_pleafFactor)_pcurrent)->get(pos)); }
                MTOOLS_ASSERT(_pcurrent->father != nullptr); // a leaf must always have a father
                _pcurrent = _pcurrent->father;
                }
            // going up...
            _pnode q = (_pnode)_pcurrent;
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
                    { // subbox never created..
                    if (q->rad == R)
                        { // the subbox to create is a leaf
                        int64 cv;
                        _pleafFactor L = _allocateLeaf(q, q->subBoxCenter(pos),cv); // create a leaf
                        if (cv == (_maxSpec + 1))
                            {  // the leaf is not factorizable
                            b = (_pbox)L; _pcurrent = b; return(L->get(pos));
                            }
                        // the leaf is factorizable
                        b = _setSpecial(cv, ((_pleafFactor)b)->data); // save the special value if needed and set the link inside the father node tab
                        if (_callDtors) _poolLeaf.destroy(L); else _poolLeaf.deallocate(L);
                        _pcurrent = _simplifyTree(q);
                        return(*_getSpecialObject(cv));
                        }
                    // create the subnode
                    q = _allocateNode(q, q->subBoxCenter(pos), nullptr);
                    b = q;
                    }
                else
                    {
                    T * obj = _getSpecialObject(b); // check if the link is a special dummy link
                    if (obj != nullptr) { _pcurrent = q; return(*obj); } // yes, we return the associated value 
                    // no, b is a real link
                    if (b->isLeaf()) { _pcurrent = b; return(((_pleafFactor)b)->get(pos)); }
                    q = (_pnode)b;
                    }
                }
            }




        /* try to simplify the tree, starting from a given node.
           return the new simplified node */
        _pnode _simplifyTree(_pnode N) const
            {
            while(1)
                {
                _pbox p = N->tab[0]; // first child of the node
                if (_getSpecialObject(p) == nullptr) return N; // the first child is not special, no simplification
                for (size_t i = 1; i < metaprog::power<3, D>::value; ++i)
                    {
                    if (N->tab[i] != p) return N; // not the same special value for all children: no simplification
                    }
                // yes, we can simplify                
                if (N->father == nullptr) { N->father = _allocateNode(N); } // make sure the father exist 
                _pnode F = (_pnode)(N->father); // the father node
                _pbox & R = F->getSubBox(N->center); // get the corresponding pointer in the father tab
                MTOOLS_ASSERT(R == N); // make sure everything is coherent
                R = p; // set the new value
                _poolNode.deallocate(N); // delete the node (no need to call dtors, there are none in node objects)
                N = F; // go to the father;
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






        /* create the data for a leafFactor using the positional constructor.
         * set the ->data and ->count fields in the leaf and update the global counter for special objects. 
         * return _maxSpec+1 if the leaf is not composed of a single special object 
         * return the number of the special object otherwise */
        int64 _createDataLeaf(_pleafFactor pleaf, Pos pos, metaprog::dummy<true> dum) const
            {
            Pos center = pos; // save the center position
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            for (size_t i = 0; i < D; ++i) { pos[i] -= R; } // go to the first cell
            int64 off = -1; // offset of the last special object created. 
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                new(pleaf->data + x) T(pos); // create from positionnal constructor
                int64 val = (int64)(*(pleaf->data + x)); // convert to int64
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
        * set the ->data and ->count fields in the leaf and update the global counter for special objects.
        * return _maxSpec+1 if the leaf is not composed of a single special object
        * return the number of the special object otherwise */
        int64 _createDataLeaf(_pleafFactor pleaf, Pos pos, metaprog::dummy<false> dum) const
            { 
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            int64 off = -1; // offset of the last special object created. 
            for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)
                {                 
                new(pleaf->data + i) T(); 
                int64 val = (int64)(*(pleaf->data + i)); // convert to int64
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


        /* Create a leaf at a given position. 
         * When the method return, if all the object in the leaf are equal to the same special value, 
         * then this value is put in communVal, otherwise, communVal is set to _maxSpec+1 */
        _pleafFactor _allocateLeaf(_pbox above, const Pos & centerpos, int64 & communVal) const
            {
            _pleafFactor p = _poolLeaf.allocate(); // allocate the memory
            communVal = _createDataLeaf(p, centerpos, metaprog::dummy<std::is_constructible<T, Pos>::value>()); // create the data
            p->center = centerpos;
            p->rad = 1;
            p->father = above;
            return p;
            }


        /* Create a leaf at a given position, setting all the object to a given one using the copy ctor 
        * do not modify the global counters */
        _pleafFactor _allocateLeafCst(_pbox above, const Pos & centerpos, const T * obj, int64 value) const
            {
            _pleafFactor pleaf = _poolLeaf.allocate(); // allocate the memory
            memset(pleaf->count, 0, sizeof(pleaf->count)); // reset the number of each type of special object 
            for(size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i) { new(pleaf->data + i) T(*obj); } // init all objects with copy ctor
            MTOOLS_ASSERT(value == (int64)(*obj)); // make sure obj and value match
            if ((value >= _minSpec) && (value <= _maxSpec)) { (pleaf->count[value - _minSpec]) = metaprog::power<(2 * R + 1), D>::value; } // update the count array when obj is special
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
            return;
            }


        /* create a node, call constructor from above */
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


        /* create a node, call constructor from below : creates a new root */
        inline _pnode _allocateNode(_pbox below) const
            {
            MTOOLS_ASSERT(below->center == Pos(0)); // a new root should always be centered
            _pnode p = _poolNode.allocate();
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = nullptr; }
            p->tab[(metaprog::power<3, D>::value - 1) / 2] = below;
            p->center = below->center;
            p->rad = ((below->rad == 1) ? R : (below->rad * 3 + 1));
            p->father = nullptr;
            return p;
            }



        /* Reset the object and change the min and max values for the special objects and the calldtor flag */
        void _reset(int64 minVal, int64 maxVal, bool callDtors)
            {
            MTOOLS_ASSERT((maxVal < minVal) || (maxVal - minVal < NB_SPECIAL));
            _reset();
            _minSpec = minVal;
            _maxSpec = maxVal;
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
            _rangemin.clear((int64)1);
            _rangemax.clear((int64)-1);

            memset(_tabSpecObj, 0, sizeof(_tabSpecObj));
            memset(_tabSpecNB, 0, sizeof(_tabSpecNB));
            _nbNormalObj = 0;
            return;
            }



        /* Set the value in a leaf.
         * - Factorizes the leaf and its nodes above if needed.
         * - Return the leaf or the first node above */
        inline _pbox _setLeafValue(const T * obj, Pos pos, _pleafFactor leaf)
            {
            T * oldobj = &(leaf->get(pos));     // the previous object
            int64 oldvalue = (int64)(*oldobj);  // the previous object value
            int64 value = (int64)(*obj);        // the new object value
            if (oldvalue == value)
                { // the old and new object have the same value
                if ((value >= _minSpec) && (value <= _maxSpec)) { return leaf; } // old and new are the same special value hence nothing to do and the leaf cannot be factorized...
                (*oldobj) = (*obj); // not a special value, simply replace it using the assignement operator
                return leaf; // the leaf cannot be factorized
                }
            // old and new do not have the same value
            (*oldobj) = (*obj); // save the new value
            if ((oldvalue >= _minSpec) && (oldvalue <= _maxSpec)) 
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
            if ((value >= _minSpec) && (value <= _maxSpec))
                { // new value is special, increment the global and local count
                auto off = value - _minSpec; (_tabSpecNB[off])++; (leaf->count[off])++; // increment its count
                MTOOLS_ASSERT( (leaf->count[off] <= metaprog::power<(2 * R + 1), D>::value) );
                if (leaf->count[off] == metaprog::power<(2 * R + 1), D>::value) 
                    { // ok, we can factorize and remove this leaf.
                    _pnode F = (_pnode)(leaf->father); // the father of the leaf
                    MTOOLS_ASSERT(F != nullptr);
                    _pbox & R = F->getSubBox(pos); // the pointer to the leaf in the father tab
                    MTOOLS_ASSERT(R == ((_pbox)leaf)); // make sure we are coherent.
                    R = _setSpecial(value, obj); // save the special object if needed and set the dummy node in place of the pointer to the leaf
                    if (_callDtors) _poolLeaf.destroy(leaf); else _poolLeaf.deallocate(leaf); // delete the leaf possibly calling the destructors
                    return _simplifyTree(F); // we try to simplify the tree starting from the father
                    } 
                }
            else
                { // new value is not special, just increase the global count
                _nbNormalObj++;
                }
            return leaf; // the leaf cannot be factorized
            }



        /* get the value associated with a special node 
        return nullptr if it does not exist */
        inline int64  _getSpecialValue(_pbox p) const
            {
            int64 off = ((_pnode)p) - (_dummyNodes);
            off += _minSpec;
            MTOOLS_ASSERT((off >= _minSpec) && (off <= _maxSpec));
            return off;
            }
            

        /* get a pointer to a special object with a given value, 
           return nullptr if it does not exist */
        inline T * _getSpecialObject(int64 value) const
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
            return _tabSpecObj[value - _minSpec];
            }


        /* get a pointer to a special object associated with a special node
           return nullptr if the node is not special */
        inline T * _getSpecialObject(_pbox p) const
            {
            auto off = ((_pnode)p) - (_dummyNodes);
            if ((off >= 0) && (off < NB_SPECIAL))
                { // yes, this is a special object
                MTOOLS_ASSERT(_tabSpecObj[off] != nullptr); // make sure the special object was previously created.
                return _tabSpecObj[off];
                }
            return nullptr;
            }


        /* return the adress of the special node associated with a special object */
        inline _pbox _getSpecialNode(int64 value) const
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
            return _dummyNodes + (value - _minSpec);
            }


        /* set a special object. Does nothing if already set.
           Return the associated dummy node */
        inline _pbox _setSpecial(int64 value, const T* obj) const
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
            return _dummyNodes + off; // return a pointer to the correpsonding dummy node
            }



        mutable _pbox _pcurrent;                // pointer to the current box
        mutable Pos   _rangemin;                // the minimal accessed range
        mutable Pos   _rangemax;                // the maximal accessed range

        mutable SingleAllocator<internals_grid::_leafFactor<D, T, NB_SPECIAL, R>, 200>  _poolLeaf;   // pool for leaf objects
        mutable SingleAllocator<internals_grid::_node<D, T, R>, 200 >  _poolNode;                    // pool for node objects
        mutable SingleAllocator<T, NB_SPECIAL + 1 >  _poolSpec;                                     // pool for special objects

        mutable T* _tabSpecObj[NB_SPECIAL];                                                         // array of T pointers to the special objects.
        mutable uint64 _tabSpecNB[NB_SPECIAL];                                                      // total number of special objects of each type. 
        mutable uint64 _nbNormalObj;                                                                // number of objects which are not special

        mutable internals_grid::_node<D, T, R> _dummyNodes[NB_SPECIAL];                             // dummy nodes array used solely to indicate special objects. 
        
        int64 _minSpec, _maxSpec;                                                                   // min and max value of special objects
        bool _callDtors;                                                                            // should we call the dtors of T objects. 



    };


}


/* end of file */

