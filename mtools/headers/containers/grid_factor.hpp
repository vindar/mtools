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
         * Constructor. An empty grid (no objet of type T is created). Set minSpecial \> maxSpecial to
         * disable special values.
         *
         * @param   minSpecial  The minimum value of the special objects.
         * @param   maxSpecial  The maximum value of the special objects.
         * @param   callDtors   true to call the destructors of the T objects when destroyed.
         **/
        Grid_factor(int64 minSpecial = 0, int64 maxSpecial = -1, bool callDtors = true)
            { 
            reset(minSpecial, maxSpecial, callDtors);
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
        * Destructor. Destroys the grid. The destructors of all the T objects in the grid are invoqued
        * if the callDtor flag is set and are droped into oblivion otherwise.
        **/
        ~Grid_factor() { _reset(); }


        /**
        * Copy Constructor. Makes a Deep copy of the grid.
        **/
        template<size_t NB_SPECIAL2> Grid_factor(const Grid_factor<D, T, NB_SPECIAL2, R> & G)
            {
            _reset(0, -1, true);
            this->operator=(G);
            }


        /**
         * Assignement operator. Create a deep copy of G. If the grid is not empty, it is first reset :
         * all the object inside are destroyed and their dtors are invoqued.
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
        * Resets the grid. Keep the current minSpecial, maxSpecial et callDtor parameters. 
        **/
        void reset()
            {
            _reset();
            _createBaseNode();
            }


        /**
         * Resets the grid and change the range of the special values. Set minSpecial \> maxSpecial to
         * disable special values.
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


        /**
         * Check if we can call the destructor of object when they are not needed anymore.
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
         * Minimum value of special object (those that are factorized).
         *
         * @return  the minimum value of a special object.
         **/
        int64 minSpecialValue() const { return _minSpec; }


        /**
         * Maximum value of special object (those that are factorized).
         *
         * @return  the maximum value of a special object
         **/
        int64 maxSpecialValue() const { return _maxSpec; }


        /**
        * Return the spacial range of elements accessed. The method returns maxpos<minpos if no element have
        * ever been accessed/created.
        *
        * @param [in,out]  minpos  a vector with the minimal coord. in each direction.
        * @param [in,out]  maxpos  a vector with the maximal coord. in each direction.
        **/
        inline void getPosRange(Pos & minpos, Pos & maxpos) const { minpos = _rangemin; maxpos = _rangemax; }


        /**
         * Return the spacial range of elements accessed in an iRect structure. The rectangle is empty
         * if no elements have ever been accessed/created. This method is specific when the dimension
         * (template paramter D) is 2.
         *
         * @return  an iRect containing the spacial range of element accessed.
         **/
        inline iRect getPosRange() const 
            {
            static_assert(D == 2, "getRangeiRect() method can only be used when the dimension template parameter D is 2");
            return mtools::iRect(_rangemin.X(), _rangemax.X(), _rangemin.Y(), _rangemax.Y());
            }


        /**
        * The current minimum value of all element ever created in the grid (obtained by casting the
        * element into int64). This value may be different from the elment really accessed since other
        * element can be silently created.
        *
        * @return  The minimum value (converted as int64) for all the element ever creaed in the grid.
        **/
        inline int64 minValue() const { return _minVal; }


        /**
         * The current maximum value of all element ever created in the grid (obtained by casting the
         * element into int64). This value may be different from the elment really accessed since other
         * element can be silently created.
         *
         * @return  The maximum value (converted as int64) for all the element ever creaed in the grid.
         **/
        inline int64 maxValue() const { return _minVal; }


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
            return false;
            }


        bool load(std::string filename)
            {
                // *********************** TODO ************************
            return false;
            }



        /**
        * Sets the value at a given site. If the value at the site does not exist prior to the call of the method, 
        * it is first created and then the assignement is performed.
        *
        * @param   pos The position of the site to access.
        * @param   val The value to set.
        **/
        inline void set(const Pos & pos, const T & val) { _set(pos,&val); }


        /**
        * Get the value at a given position (the value is created if needed). 
        *
        * @param   pos The position.
        *
        * @return  A const reference to the value at that position.
        **/
        inline const T & get(const Pos & pos) const { return _get(pos); }


        /**
         * Return a pointer to the object at a given position. If the value at the site was not yet
         * created, returns nullptr. This method does not modify the object at all and is particularly
         * suited when drawing the lattice using, for instance, the LatticeDrawer class.
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
                MTOOLS_ASSERT(_isLeafFull(p) == (_maxSpec + 1)); // the leaf cannot be full
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
                if (b->isLeaf()) 
                    { _pcurrent = b; 
                    MTOOLS_ASSERT(_isLeafFull((_pleafFactor)_pcurrent) == (_maxSpec + 1)); // the leaf cannot be full
                    return(&(((_pleafFactor)b)->get(pos))); 
                    }
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
            s += std::string(" - Min position accessed = ") + _rangemin.toString(false) + "\n";
            s += std::string(" - Max position accessed = ") + _rangemax.toString(false) + "\n";
            s += std::string(" - Min value created = ") + _minVal + "\n";
            s += std::string(" - Max value created = ") + _maxVal + "\n";
            s += std::string(" - Special object value range [") + mtools::toString(_minSpec) + " , " + mtools::toString(_maxSpec) + "]";         
            if (_maxSpec < _minSpec) { s += std::string(" NONE!\n"); } else { s += std::string("\n"); }
            for (int i = 0;i < (_maxSpec - _minSpec + 1); i++)
                {
                s += std::string("    [") + ((_tabSpecObj[i] == nullptr) ? " " : "X") + "] value (" + mtools::toString(_minSpec + i) + ") = " + mtools::toString(_tabSpecNB[i]) + "\n";
                }
            s += std::string(" - Number of 'normal' objects = ") + mtools::toString(_nbNormalObj) + "\n";
            if (debug) {s += "\n" + _printTree(_getRoot(),"");}
            return s;
            }


    private:

        /* Make sure template parameters are OK */
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
            MTOOLS_ASSERT(_pcurrent != nullptr);
            _updatePosRange(pos);
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
        inline const T & _get(const Pos & pos) const
            {
            MTOOLS_ASSERT(_pcurrent != nullptr);
            _updatePosRange(pos);
            if (_pcurrent->isLeaf())
                {
                if (((_pleafFactor)_pcurrent)->isInBox(pos)) 
                        { 
                        MTOOLS_ASSERT(_isLeafFull((_pleafFactor)_pcurrent) == (_maxSpec + 1)); // the leaf cannot be full
                        return(((_pleafFactor)_pcurrent)->get(pos)); 
                        }
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
                        if (_callDtors) _poolLeaf.destroy(L); else _poolLeaf.deallocate(L);
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


        /* Reset the object and change the min and max values for the special objects and the calldtor flag */
        void _reset(int64 minSpec, int64 maxSpec, bool callDtors)
            {
            MTOOLS_INSURE((maxSpec < minSpec) || (maxSpec - minSpec < NB_SPECIAL));
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


        /* expand the whole tree below a given node (ie remove all references 
         * to dummy node and replace them by real nodes/leafs.
         * Recursive. This operation can be time and memory consumming  */
        void _expandBelowNode(_pnode N)
            {
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i)
                {
                const _pbox K = N->tab[i];
                T * pv = _getSpecialObject(K);
                if (pv != nullptr) // check if the child node is special
                    { // yes we expand it
                    if (N->rad > R) 
                        { // it is a node
                        N->tab[i] = _allocateNode(N, N->subBoxCenterFromIndex(i), K); // create it
                        _expandBelowNode(N->tab[i]); // and recurse
                        }
                    else
                        { // it is a leaf
                        N->tab[i] = _allocateLeafCst(N, N->subBoxCenterFromIndex(i), pv, _getSpecialValue(K)); // create it
                        }
                    }
                }
            }


        /* simplify the whole sub-tree below a given node */ 
        _pbox _simplifyBelowNode(_pnode N)
            {
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) // we iterate over the children
                {
                const _pbox K = N->tab[i];
                if ((K != nullptr) && (_getSpecialObject(K) == nullptr))
                    { // the ith child is a real (not nullptr or a dummy node)
                    if (N->rad == R)
                        { // it is a leaf
                        _pleafFactor L = (_pleafFactor)K;
                        int64 val = _isLeafFull(L);
                        if (val <= _maxSpec)
                            { // yes, the leaf is full and we can factorize it
                            N->tab[i] = _setSpecial(val, L->data); // save the special element and set the dummy node instead 
                            if (_callDtors) _poolLeaf.destroy(L); else _poolLeaf.deallocate(L); // release memory
                            }
                        }
                    else
                        { //it is a node
                        _simplifyBelowNode(K); // we simplify it
                        }
                    }
                }
            // now that every children is simplified, we go again to see if we can simplify the node itself
            _pbox p = N->tab[0]; // the first child of the node
            if (_getSpecialObject(p) == nullptr) return; // the first child is not special, no further simplification
            for (size_t i = 1; i < metaprog::power<3, D>::value; ++i)
                {
                if (N->tab[i] != p) return N; // not the same special value for all children: no further simplification
                }
            }


        /* change the range of the special object and reconstruct the whole tree 
         * so that it is ciherent with the new special range */
        void _changeSpecialRange(int64 newMinSpec, int64 newMaxSpec)
            {
            _pcurrent = _getRoot(); // go to the root

            _expandBelowNode((_pnode)_pcurrent); // expand the whole tree so there is no more dummy links
            
            if (_callDtors) _poolSpec.destroyAll(); else _poolSpec.deallocateAll(); // release the memory for all the special oejcts
            memset(_tabSpecObj, 0, sizeof(_tabSpecObj)); // clear the list of pointer to special objects
            memset(_tabSpecNB, 0, sizeof(_tabSpecNB));   // clear the count for special objects
            _nbNormalObj = 0; // reset the number of normal objects

            _minSpec = newMinSpec; // new min value for the special objects
            _maxSpec = newMaxSpec; // new max value for the special objects

            _recountAllLeafs(); // recount all the leafs with the new spectial object range
            
            _simplifyTree(); // simplify the tree using the new special objects
            }


        /* recount all the leaf in a given sub-tree */
        void _recountAllLeafs(_pnode N)
            {
            if (N->isLeaf()) {}
            }


        /* recount a leaf : recompute from scratch the count[] tab 
           also increase the global counter for special object and normal object */
        void _recountLeaf(_pleafFactor L)
            {
            memset(L->count, 0, sizeof(L->count)); // reset the number of each type of special object
            if (_maxSpec > _minSpec) { _nbNormalObj += metaprog::power<(2 * R + 1), D>::value; return; } // no special value, all the value are therefore normal   
            for (size_t x = 0; x < metaprog::power<(2 * R + 1), D>::value; ++x)
                {
                int64 val = (int64)(*(L->data[x])); // convert to int64
                if ((val >= _minSpec) && (val <= _maxSpec)) // check if it is a special value
                    { // yes
                    auto off = val - _minSpec;           // the offset of the special object
                    (pleaf->count[off])++;               // increase the counter in the leaf associated with this special object
                    (_tabSpecNB[off])++;                 // increase the global counter about the number of these special objects 
                    }
                else { _nbNormalObj++; } // increase the global counter for the number of normal object
                }
            }






        /* simplify the branche going up above a given node */
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
                _pnode F = (_pnode)(N->father); // the father node
                _pbox & B = F->getSubBox(N->center); // get the corresponding pointer in the father tab
                MTOOLS_ASSERT(B == N); // make sure everything is coherent
                B = p; // set the new value
                _poolNode.deallocate(N); // delete the node (no need to call dtors, there are none in node objects)
                N = F; // go to the father;
                }
            }


        /* check if a leaf is full and thus should be factorized
         * return the value of the special object if it is full and _maxSpec + 1 otherwise */
        inline int64 _isLeafFull(_pleafFactor L) const
            {
            MTOOLS_ASSERT(L != nullptr);
            int64 val = (int64)(*(L->data));
            if ((val >= _minSpec) && (val <= _maxSpec))
                {
                MTOOLS_ASSERT( (L->count[val - _minSpec] <= metaprog::power<(2 * R + 1), D>::value) );
                if (L->count[val - _minSpec] == metaprog::power<(2 * R + 1), D>::value) { return val; }
                }
            return (_maxSpec + 1);
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
                if ((value >= _minSpec) && (value <= _maxSpec)) 
                    { // it is a special value
                    MTOOLS_ASSERT( (leaf->count[value - _minSpec] <= metaprog::power<(2 * R + 1), D>::value) );
                    if (leaf->count[value - _minSpec] == metaprog::power<(2 * R + 1), D>::value) // check if the value is the only one in this leaf
                        { // yes the leaf can be factorized
                        _pnode F = (_pnode)(leaf->father); // the father of the leaf
                        MTOOLS_ASSERT(F != nullptr);
                        _pbox & B = F->getSubBox(pos); // the pointer to the leaf in the father tab
                        MTOOLS_ASSERT(B == ((_pbox)leaf)); // make sure we are coherent.
                        B = _setSpecial(value, obj); // save the special object if needed and set the dummy node in place of the pointer to the leaf
                        if (_callDtors) _poolLeaf.destroy(leaf); else _poolLeaf.deallocate(leaf); // delete the leaf possibly calling the destructors
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
                    _pbox & B = F->getSubBox(pos); // the pointer to the leaf in the father tab
                    MTOOLS_ASSERT(B == ((_pbox)leaf)); // make sure we are coherent.
                    B = _setSpecial(value, obj); // save the special object if needed and set the dummy node in place of the pointer to the leaf
                    if (_callDtors) _poolLeaf.destroy(leaf); else _poolLeaf.deallocate(leaf); // delete the leaf possibly calling the destructors
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
                if ((val >= _minSpec) && (val <= _maxSpec)) // check if it is a special value
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
                if ((val >= _minSpec) && (val <= _maxSpec)) // check if it is a special value
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

        /* Return the value associated with a dummy node
         * No checking that the node is indeed a dummy node */
        inline int64  _getSpecialValue(_pbox p) const
            {
            int64 off = ((_pnode)p) - (_dummyNodes);
            off += _minSpec;
            MTOOLS_ASSERT((off >= _minSpec) && (off <= _maxSpec));
            return off;
            }
            

        /* Return a pointer to the object associated with a special value 
           No checking that the the value is correct or that an object was associated with it */
        inline T * _getSpecialObject(int64 value) const
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
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
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
            return _dummyNodes + (value - _minSpec);
            }


        /* set a special object. Does nothing if already set.
           Return the a pointer to the associated dummy node 
           No checking that value and obj are correct */
        inline _pbox _setSpecial(int64 value, const T* obj) const
            {
            MTOOLS_ASSERT((value >= _minSpec) && (value <= _maxSpec));
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



        /***************************************************************
        * Internal state
        ***************************************************************/

        mutable _pbox _pcurrent;                // pointer to the current box
        mutable Pos   _rangemin;                // the minimal accessed range
        mutable Pos   _rangemax;                // the maximal accessed range

        mutable int64 _minVal;                  // current minimum value in the grid
        mutable int64 _maxVal;                  // current maximum value in the grid

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

