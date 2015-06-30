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


#include "misc/error.hpp"
#include "maths/vec.hpp"
#include "misc/metaprog.hpp"
#include "io/serialization.hpp"
#include "internals_grid.hpp"

#include <string>

namespace mtools
{


    /**
     * A D-dimensional grid Z^d where each site contain an object of type T. This is the basic class
     * without "factorization" of objects.
     *
     * - Each site of Z^d contain a unique objet of type T (which is not shared with any other
     * site). Objects at each sites are created on the fly. They are not deleted until the grid is
     * destroyed or reset.
     *
     * - Internally, the grid Z^d is represented as a tree whose leafs are elementary sub-boxes of
     * size [1,2R+1]^D. The tree grows dynamically to fit the set of sites accessed. Objects associated
     * to sites are created by bunch, (2R+1)^D at a time. This means that an object at some position
     * x may be created even without ever being accesssed.
     *
     * - Access time to any given site is logarithmic with respect to its distance from the previously
     * accessed element.
     *
     * - GUARANTEE: No objet of type T is ever deleted, copied or moved around during the whole life of the
     * grid. Thus, pointers/references to elements are never invalidated. Every destructors are called when
     *  the grid is destoyed or reset (unless the caller specifically requests not to call the dtors).
     *
     * - The type T must satisfy the following properties:
     *
     * | Requir.  | Property.
     * |----------|-------------------------------------------------------------------
     * | REQUIRED | Public constructor `T()` or `T(const Pos &)`. If both ctor exist, the positional constructor is favored.
     * | OPTIONAL | Copy constructor `T(const T&)`. If the copy ctor exist, then the whole grid object can be copied/assigned. [the assignement `T.operator=()` is never needed nor used].
     *
     * - Serialization is performed using the OArchive class and look serializing methods of the class object T.
     * It is also possible to deserialize the object via the specific constructor `T(IArchive &)`. If no serialization
     * method is implemented, then OArchive use the basic serialization using memcpy().
	 *
	 * @tparam  D   Dimension of the grid Z^D.
     *
     * @tparam  T   Type of objects stored in each sites of the grid. This can be any type satisfying the requirements above.
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
     * @sa Grid_factor, Grid_static, Pos
     *
     * @code{.cpp}
     // ***********************************************************
     // Simulation of a Linearly Edge Reinforced Random Walk on Z^2.
     // ***********************************************************
     using namespace mtools;

     // structure at each site of Z^2.
     struct siteInfo
        {
        siteInfo() : up(1), right(1), V(0) {}    // ctor, set the intial weights of the edges to 1
        double up, right;   // weights of the up and right edges.
        int64 V;            // number of visits to the site.
        static int64 maxV;  // maximum number of visits of any site.
        };

     int64 siteInfo::maxV = 0;
     Grid_basic<2, siteInfo> G;   // the grid
     MT2004_64 gen;

     // site are colored w.r.t. the local time of the walk.
     RGBc colorLERRW(iVec2 pos)
        {
        const siteInfo * S = G.peek(pos);
        if ((S == nullptr) || (S->V == 0)) return RGBc::c_TransparentWhite;
        return RGBc::jetPaletteLog(S->V, 0, S->maxV, 1.2);
        }

     // Simulate the LERRW with reinforcment parameter delta for steps unit of time.
     void makeLERRW(uint64 steps, double delta)
     {
     Chronometer();
     cout << "Simulating " << steps << " steps of the LERRW with reinf. param " << delta << ".\n";
     ProgressBar<uint64> PB(steps,"Simulating..");
     iVec2 pos = { 0, 0 };
     uint64 t = 0;
     for (uint64 n = 0; n < steps; n++)
        {
        PB.update(n);
        siteInfo & S = G[pos];                          // info at the current site
        if ((++S.V) > S.maxV) { S.maxV = S.V; }
        double & right = S.right;                       // get a reference to the weight
        double & up = S.up;                             // of the 4 adjacent edges of the
        double & left = G(pos.X() - 1, pos.Y()).right;  // current position.
        double & down = G(pos.X(), pos.Y() - 1).up;     //
        double e = gen.rand_double0()*(left + right + up + down);
        if (e < left) { left += delta; pos.X()--; }
        else {
        if (e < (left + right)) { right += delta; pos.X()++; }
        else {
        if (e < (left + right + up)) { up += delta; pos.Y()++; }
        else {
        down += delta; pos.Y()--; }}}
        }
     PB.hide();
     cout << "\nSimulation completed in = " << Chronometer() / 1000.0 << " seconds.\n";
     std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid";
     G.save(fn); // save the grid in fn
     cout << "- saved in file " << fn << "\n";
     Plotter2D Plotter;
     auto L = makePlot2DLattice(LatticeObj<colorLERRW>::get());
     Plotter[L];
     Plotter.gridObject(true)->setUnitCells();
     Plotter.plot();
     return;
     }

     int main()
     {
     makeLERRW(100000000, 0.5);
     return 0;
     }
     * @endcode
     **/
    template<size_t D, typename T, size_t R = internals_grid::defaultR<D>::val > class Grid_basic
    {

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
         **/
        Grid_basic() : _pcurrent(nullptr), _rangemin(1), _rangemax(-1) { _createBaseNode(); }


        /**
         * Constructor. Loads a grid from a file. If the loading fails, the grid is constructed empty.
         *
         * @param   filename    Filename of the file.
         **/
        Grid_basic(const std::string & filename) : _pcurrent(nullptr), _rangemin(1), _rangemax(-1) {load(filename);}


        /**
         * Destructor. Destroys the grid. The destructors of all the T objects in the grid are invoqued.
         * In order to prevent calling the dtors of T objects, call `reset(false)` prior to destructing
         * the grid.
         **/
        ~Grid_basic() { _destroyTree(true); }


        /**
         * Copy Constructor. Makes a Deep copy of the grid. The class T must be copyable by the copy
         * operator `T(const T&)`.
         **/
        Grid_basic(const Grid_basic<D, T, R> & G) : _pcurrent(nullptr), _rangemin(1), _rangemax(-1)
            {
            static_assert(std::is_copy_constructible<T>::value, "The object T must be copy-constructible T(const T&) in order to use the copy constructor of the grid.");
            this->operator=(G);
            }


        /**
         * Assignement operator. Create a deep copy of G. The class T must be copyable by the copy
         * operator T(const T&). The assignement operator `operator=(const T &)` is never used and need
         * not be defined. If the grid is not empty, it is first reset : all the object inside are
         * destroyed and their dtors are invoqued.
         **/
        Grid_basic<D, T, R> & operator=(const Grid_basic<D, T, R> & G)
            {
            static_assert(std::is_copy_constructible<T>::value,"The object T must be copy constructible T(const T&) in order to use assignement operator= on the grid.");
            if ((&G) == this) { return(*this); }
            _destroyTree(true);
            _rangemin = G._rangemin;
            _rangemax = G._rangemax;
            _pcurrent = _copy(G._getRoot(),nullptr);
            return(*this);
            }


        /**
         * Resets the grid.
         *
         * @param   callObjDtors    [default true]. True to call the destructors of the T objects in the
         *                          grid. Set this to false to release all the memory without calling the
         *                          objects destructors ~T().
         **/
        void reset(bool callObjDtors = true) { _destroyTree(callObjDtors); _createBaseNode(); }


        /**
         * Serializes the grid into an OArchive. If T implement a serialize method recognized by
         * OArchive, it is used for serialization otherwise OaRchive uses the default serialization
         * method which correpsond to memcpy().
         *
         * @param [in,out]  ar  The archive object to serialise the grid into.
         *
         * @sa  class OArchive, class IArchive
         **/
        void serialize(OArchive & ar)
            {
            ar << "Grid_basic<" << D << " , [" << std::string(typeid(T).name()) << "] , " << R << ">\n"; // comment
            ar & ((uint32)1);           // version
            ar & ((uint32)D);           // dimension
            ar & ((uint32)R);           // elementary radius
            ar & ((uint32)sizeof(T));   // size of T
            ar & _rangemin;             // the min range
            ar & _rangemax;             // the max range
            ar & ((uint32)0);           // no special objects
            ar & ((uint32)0);           // reserved, = 0
            ar.newline(); ar << "end of Grid_basic<" << D << " , [" << std::string(typeid(T).name()) << "] , " << R << ">\n"; // comment
            _serializeTree(ar, _getRoot());
            }


        /**
         * Deserializes the grid from an IArchive. If T has a constructor of the form T(IArchive &), it
         * is used for deserializing the T objects in the grid. Otherwise, if T implements one of the
         * serialize methods recognized by IArchive, the objects in the grid are first position/default
         * constructed and then deserialized using those methods. If no specific deserialization procedure
         * is implemented, the object is treated as a POD and is deserialized using basic memcpy().
         *
         * @param [in,out]  ar  The archive to deserialize the grid from.
         *
         * @sa  class OArchive, class IArchive
         **/
        void deserialize(IArchive & ar)
            {
            try
                {
                _destroyTree(true);
                uint32 ver, d, r, sizeoft, nbspec, res;
                ar & ver;       if (ver != 1) throw "wrong version";
                ar & d;         if (d != D) throw "wrong dimension";
                ar & r;         if (r != R) throw "wrong R parameter";
                ar & sizeoft;   if (sizeoft != sizeof(T)) throw "wrong sizeof(T)";
                ar & _rangemin;
                ar & _rangemax;
                ar & nbspec;
                ar & res;       if (res != 0) throw "wrong reserved parameter (not 0)";
                _pcurrent = _deserializeTree(ar, nullptr);
                }
            catch(...)
                {
                _destroyTree(false);    // object are dumped into oblivion, may result in a memory leak.
                _createBaseNode();
                throw; // rethrow
                }
            }


        /**
         * Saves the grid into a file. Grid files are compatible between all grid classes. The object
         * are serialized using the OArchive serialization procedure.
         *
         * @param   filename    The filename to save.
         *
         * @return  true if  success, false if failure.
         *
         * @sa serialize, deserialize, class OArchive, class IArchive
         **/
        bool save(const std::string & filename) const
            {
            try
                {
                OArchive ar(filename);
                ar & (*this); // use the serialize method.
                }
            catch (...) {return false;} // error
            return true; // ok
            }


        /**
         * Loads a grid from a file. Grid files are compatible between all grid classes. If the grid is
         * non-empty, it is first reset and the destructors of all T objects inside it are called.
         *
         * If T has a constructor of the form T(IArchive &), it is usedwhen reconstructing the T objects
         * in the grid. Otherwise, if T implements one of the serialize methods recognized by IArchive,
         * the objects in the grid are first position/default constructed and then deserialized using those
         * methods. If no specific deserialization procedure is implemented, the class T is treated as a
         * POD and is deserialized using basic memcpy().         *
         *
         * @param   filename    The filename to load.
         *
         * @return  true on success, false on failure [in this case, the lattice is reset].
         *
         * @sa  class OArchive, class IArchive
         **/
        bool load(std::string filename)
            {
            try
                {
                IArchive ar(filename);
                ar & (*this); // use the deserialize method.
                }
            catch (...) { _destroyTree(true); _createBaseNode(); return false; } // error
            return true; // ok
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
            s += std::string("Grid_basic<") + mtools::toString(D) + " , " + typeid(T).name() + " , " + mtools::toString(R) + ">\n";
            s += std::string(" - Memory used : ") + mtools::toString((_poolLeaf.footprint() + _poolNode.footprint()) / (1024 * 1024)) + "MB\n";
            s += std::string(" - Range min = ") + _rangemin.toString(false) + "\n";
            s += std::string(" - Range max = ") + _rangemax.toString(false) + "\n";
            if (debug) {s += "\n" + _printTree(_getRoot(),"");}
            return s;
            }


        /**
         * Return the range of elements accessed. The method returns maxpos<minpos if no element have
         * ever been accessed/created.
         *
         * @param [in,out]  minpos  a vector with the minimal coord. in each direction.
         * @param [in,out]  maxpos  a vector with the maximal coord. in each direction.
         **/
        inline void getRange(Pos & minpos, Pos & maxpos) const { minpos = _rangemin; maxpos = _rangemax;}


        /**
         * Sets the value at a given site. This method require T to be assignable via T.operator=. If the
         * value at the site does not exist prior to the call of the method, it is first created then the
         * assignement is performed.
         *
         * @param   pos The position of the site to access.
         * @param   val The value to set.
         **/
        inline void set(const Pos & pos, const T & val)
            {
            _get(pos) = val;
            }


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
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         *
         *
         * @param   pos The position.
         *
         * @return  A const reference to the value.
         **/
        inline T & get(const Pos & pos) { return _get(pos); }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         * (const version)
         *
         * @param   pos The position.
         *
         * @return  A const reference to the value
         **/
        inline const T & operator[](const Pos & pos) const { return _get(pos); }


        /**
         * Get a value at a given position. If the T object at that site does not exist, it is created.
         *
         * @param   pos The position.
         *
         * @return  A reference to the value.
         **/
        inline T & operator[](const Pos & pos) { return _get(pos); }


        /**
         * Return a pointer to the object at a given position. If the value at the site was not yet
         * created, returns nullptr. This method does not create sites and is suited for use to draw the
         * lattice using, for instance, the LatticeDrawer class.
         *
         * @param   pos The position to peek.
         *
         * @return  nullptr if the value at that site was not yet created. A const pointer to it
         *          otherwise.
         **/
        inline const T * peek(const Pos & pos) const
            {
            MTOOLS_ASSERT(_pcurrent != nullptr);
            // first we go up
            if (_pcurrent->isLeaf())
                {
                _pleaf p = (_pleaf)(_pcurrent);
                if (p->isInBox(pos)) return(&(p->get(pos)));
                if (p->father == nullptr) return nullptr;
                _pcurrent = p->father;
                }
            _pnode q = (_pnode)(_pcurrent);
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr) { _pcurrent = q; return nullptr; }
                q = (_pnode)q->father;
                }
            // now we go down
            while (1)
                {
                _pbox b = q->getSubBox(pos);
                if (b == nullptr) { _pcurrent = q; return nullptr; }
                if (b->isLeaf())  { _pcurrent = b; return(&(((_pleaf)b)->get(pos))); }
                q = (_pnode)b;
                }
            }


        /**
         * get a value at a given position. Dimension 1 specialization. (const version)
         * This creates the object if it does not exist yet.
         **/
        inline const T & operator()(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }


        /**
         * get a value at a given position. Dimension 2 specialization. (const version)
         * This creates the object if it does not exist yet.
         **/
        inline const T & operator()(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
         * get a value at a given position. Dimension 3 specialization. (const version)
         * This creates the object if it does not exist yet.
         **/
        inline const T & operator()(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
         * get a value at a given position. Dimension 1 specialization. (non-const version)
         * This creates the object if it does not exist yet.
         **/
        inline T & operator()(int64 x) { static_assert(D == 1, "template parameter D must be 1"); return _get(Pos(x)); }


        /**
         * get a value at a given position. Dimension 2 specialization. (non-const version)
         * This creates the object if it does not exist yet.
         **/
        inline T & operator()(int64 x, int64 y) { static_assert(D == 2, "template parameter D must be 2"); return _get(Pos(x, y)); }


        /**
         * get a value at a given position. Dimension 3 specialization. (non-const version)
         * This creates the object if it does not exist yet.
         **/
        inline T & operator()(int64 x, int64 y, int64 z)  { static_assert(D == 3, "template parameter D must be 3"); return _get(Pos(x, y, z)); }


        /**
         * peek at a value at a given position. Dimension 1 specialization. (const version)
         * Returns nullptr if the object at this position is not created.
         **/
        inline const T * peek(int64 x) const { static_assert(D == 1, "template parameter D must be 1"); return peek(Pos(x)); }


        /**
         * peek at a value at a given position. Dimension 2 specialization. (const version)
         * Returns nullptr if the object at this position is not created.
         **/
        inline const T * peek(int64 x, int64 y) const { static_assert(D == 2, "template parameter D must be 2"); return peek(Pos(x, y)); }


        /**
         * peek at a value at a given position. Dimension 3 specialization. (const version)
         * Returns null if the object at this position is not created.
         **/
        inline const T * peek(int64 x, int64 y, int64 z) const { static_assert(D == 3, "template parameter D must be 3"); return peek(Pos(x, y, z)); }




        /************************** PRIVATE PART ***************************************************************************************************************************/


    private:


        static_assert(D > 0, "template parameter D (dimension) must be non-zero");
        static_assert(R > 0, "template parameter R (radius) must be non-zero");
        static_assert(std::is_constructible<T>::value || std::is_constructible<T, Pos>::value, "The object T must either be default constructible T() or constructible with T(const Pos &)");

    // TODO FIX CONST CORRECTNESS, DIRTY MUTABLE...


        /* get sub method */
        inline T & _get(const Pos & pos) const
            {
            MTOOLS_ASSERT(_pcurrent != nullptr);
            _updaterange(pos);
            if (_pcurrent->isLeaf())
                {
                if (((_pleaf)_pcurrent)->isInBox(pos)) { return(((_pleaf)_pcurrent)->get(pos)); }
                if (_pcurrent->father == nullptr) { _pcurrent->father = _allocateNode(_pcurrent); }
                _pcurrent = _pcurrent->father;
                }
            // going up...
            _pnode q = (_pnode)_pcurrent;
            while (!q->isInBox(pos))
                {
                if (q->father == nullptr)
                    {
                    q->father = _allocateNode(q);
                    }
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
                        _pcurrent = b;
                        return(((_pleaf)_pcurrent)->get(pos));
                        }
                    q = _allocateNode(q, q->subBoxCenter(pos), nullptr);
                    b = q;
                    }
                else
                    {
                    if (b->isLeaf()) { _pcurrent = b; return(((_pleaf)b)->get(pos)); }
                    q = (_pnode)b;
                    }
                }
            }


        /* update _rangemin and _rangemax */
        inline void _updaterange(const Pos & pos) const
            {
            if (_rangemax[0] < _rangemin[0]) { _rangemin = pos; _rangemax = pos; return; }
            for (size_t i = 0; i < D; i++) {if (pos[i] < _rangemin[i]) { _rangemin[i] = pos[i]; } else if (pos[i] > _rangemax[i]) { _rangemax[i] = pos[i]; } }
            }


        /* get the root of the tree */
        inline _pbox _getRoot() const
            {
            if (_pcurrent == nullptr) return nullptr;
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
        template<typename ARCHIVE> void _serializeTree(ARCHIVE & ar, _pbox p)
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
            if (c == 'S')
                {
                Pos center;
                uint64 rad;
                uint32 nospec;
                ar & center;
                ar & rad;
                ar & nospec;
                return _expand_subtree(father, center, rad, nospec);
                }
            throw "";
            }


        /* create the expanded sub-tree with the special object number nbspec*/
        _pbox _expand_subtree(_pbox father, Pos center, uint64 rad, uint32 nospec) const
            {
            /*
            if (rad == 1)
                { // we create leafs
                _pleaf p = _poolLeaf.allocate();
                p->center = center;
                p->rad = rad;
                p->father = father;
                for (size_t i = 0; i < metaprog::power<(2 * R + 1), D>::value; ++i)  { _makeObj(p->data + i, V,str); }
                return p;
                }
            // create a node
            _pnode p = _poolNode.allocate();
            p->center = center;
            p->rad = rad;
            p->father = father;
            for (size_t i = 0; i < metaprog::power<3, D>::value; ++i) { p->tab[i] = _expand_subtree(p, p->subBoxCenterFromIndex(i), (rad-1)/3, V,str); }
            return p;
            */
            return nullptr;
            }


        /* Release all the allocated  memory and reset the tree */
        void _destroyTree(bool callObjDtor)
            {
            if (callObjDtor)
                {
                _poolNode.destroyAll();
                _poolLeaf.destroyAll();
                }
            else
                {
                _poolNode.deallocateAll();
                _poolLeaf.deallocateAll();
                }
            _pcurrent = nullptr;
            _rangemin.clear((int64)1);
            _rangemax.clear((int64)-1);
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


        mutable _pbox _pcurrent;        // pointer to the current box
        mutable Pos   _rangemin;        // the minimal range
        mutable Pos   _rangemax;        // the maximal range

        mutable SingleAllocator<internals_grid::_leaf<D, T, R>,200>  _poolLeaf;       // the two memory pools
        mutable SingleAllocator<internals_grid::_node<D, T, R>,200>  _poolNode;       //

    };


}


/* end of file */




