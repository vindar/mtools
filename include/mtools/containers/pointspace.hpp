/** @file pointspace.hpp */
//
// Copyright 2021 Arvind Singh
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
#include "../misc/metaprog.hpp"
#include "../misc/misc.hpp"
#include "../maths/box.hpp"
#include "../maths/vec.hpp"
#include "../misc/memory.hpp"
#include "../io/serialization.hpp"


#include <type_traits>
#include <string>
#include <vector>
#include <map>


namespace mtools
{


    /**
    * Functor object to compute the euclidian metric
    *
    * Default metric used when computing distances such as for methods 
    * PointSpace::iterateBall() or PointSpace::findNearest()
    **/
    template<int DIM> struct EuclidianMetric
        {

        /**
        * Compute the distance between two points P1 and P2.
        **/
        double operator()(const mtools::fVec<DIM> & P, const mtools::fVec<DIM>& Q) const
            {
            return dist(P, Q);
            }


        /**
        * Compute a LOWER BOUND on the distance between point P and box B.
        **/
        double operator()(const mtools::fVec<DIM>& P, const mtools::fBox<DIM>& B) const
            {
            const mtools::fVec<DIM> C = B.center(); // distance to center
            const double d = dist(C, P) - dist(C, B.min); // lower bound cannot be closer than that
            return d;
            }

        };



    /**
    * Functor object to compute the euclidian metric on a torus [0,1]^DIM
    *
    * used when computing distances such as for methods 
    * PointSpace::iterateBall() or PointSpace::findNearest()
    **/
    template<int DIM> struct TorusMetric
        {

        /**
        * Compute the distance between two points P1 and P2 on the torus [0,1]^DIM.
        **/
        double operator()(const mtools::fVec<DIM>& P1, const mtools::fVec<DIM>& P2) const
            {
            return distTorus(P1, P2);
            }

        /**
        * Compute a LOWER BOUND on the distance between point P and box B on the torus [0,1]^DIM.
        **/
        double operator()(const mtools::fVec<DIM>& P, const mtools::fBox<DIM>& B) const
            {
            const mtools::fVec<DIM> C = B.center(); // distance to center
            const double d = distTorus(C, P) - distTorus(C, B.min); // lower bound cannot be closer than that
            return d;
            }


        /**
        * Compute the distance between two points P1 and P2 on the torus [0,1]^DIM.
        **/
        double distTorus(const mtools::fVec<DIM>& P1, const mtools::fVec<DIM>& P2) const
            {
            double sum = 0;
            for (int k = 0; k < DIM; k++)
                {
                const double dx = std::abs(P2[k] - P1[k]);
                const double a = (dx > 0.5) ? (1.0 - dx) : dx;
                sum += (a * a);
                }
            return std::sqrt(sum);
            }

        };





    /**
    * Structure that encapsulate a d-dimensional point with a given payload (data). 
    * 
    * This a the basic type "managed" by the PointSpace class. 
    * 
    * - The position is set at construction time and cannot be modified after.  
    *  
    * - The data/payload can be modified at any time.   
    *    
    **/
    template<int DIM, typename T>
    struct PointSpaceObj
        {

        // forward friend declaration        
        template<int, typename, size_t> friend class PointSpaceNode;
        template<int, typename, size_t, size_t> friend class PointSpace;

        private:

            fVec<DIM>   _position;       // point position (puit this first to improve memory layout for 'light' data types 


        public:

            /**
            * Construct the object. 
            * 
            * Set its position and value.
            **/
            PointSpaceObj(const fVec<DIM>& pos, const T& val) : _position(pos), data(val), _child_index(_invalid_index) {}


            /**
            * Construct the object, 
            * 
            * Setting its position and use default ctor T() for the associated data.
            **/
            PointSpaceObj(const fVec<DIM>& pos) : _position(pos), data(), _child_index(_invalid_index) {}


            T           data;           // associated data  (free access)


            /**
            * Query the position of the object (cannot be modified). 
            **/
            const fVec<DIM> & position() const { return _position; }


            /**
            * Print info about the object (position and content)
            * into a string.
            **/
            std::string toString() const
                {
                mtools::ostringstream oss;
                oss << "PointSpaceNode<" << DIM << " , " << typeid(T).name() << ">  ";
                oss << "pos = " << position() << "  ";
                oss << "value = " << mtools::toString(data) << "\n";
                return oss.toString();
                }


        private:

            int32_t    _child_index;     // index of the object index the array (= k < 0 if it does not exist, and it this case the next free child is -k + 1). 


            /** Return a pointer to the node to which this object belong (must be valid) */
            template<size_t SIZE> PointSpaceNode<DIM, T, SIZE> *  _getNode() const
                {
                MTOOLS_INSURE(_child_index >= 0); // if negative, this means that the index object is deleted...
                return (PointSpaceNode<DIM, T, SIZE>*)(this - _child_index);
                }


            /** Return the index of the next free sibling, or -1 if none available. (must be free) **/
            int32_t _nextFree() const
                {
                MTOOLS_INSURE(_child_index < 0);  // if positive, this means that the object is occupied
                return ((_child_index == _invalid_index) ? -1 : (-_child_index - 1));
                }


            /** Set the index of the next free sibling. **/
            void _setNextFree(int index)
                {
                MTOOLS_INSURE(index >= 0);
                _child_index = -index - 1;
                }


            /** Set this obj as the last free obj **/
            void _setLastFree()
                {
                _child_index = _invalid_index;
                }


            static const int32_t _invalid_index = std::numeric_limits<int32_t>::min();   // value for the invalid index. 

        };





    /**
    * Structure for a Node
    * 
    * Private class used by the main PointSpace class. 
    **/
    template<int DIM, typename T, size_t SIZE>
    class PointSpaceNode
        {

        // forward friend declaration        
        template<int, typename, size_t, size_t> friend class PointSpace;



        public:

            /**
            * (public) Destructor
            *
            * Destroy all the T objects that are still alive.
            **/
            ~PointSpaceNode()
                {
                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)obj;
                for (int k = 0; k < SIZE; k++)
                    {
                    if (o[k]._child_index == k)
                        { // object alive so we destroy it
                        (o[k].data).~T();
                        }
                    }
                }

        private:

            static_assert(SIZE >= 2, "template parameter SIZE must be greater or equal to 2");
            static_assert(DIM >= 1, "template parameter DIM must be greater or equal to 1");

            typedef typename std::aligned_storage<sizeof(PointSpaceObj<DIM, T>)>::type _fakePSO; // placeholder 

            _fakePSO obj[SIZE]; // objects contained in this node. * MUST BE AT THE BEGINING OF THE MEMORY OF THE NODE *

            PointSpaceNode<DIM, T, SIZE>* childs[2];  // links to the child nodes in order                                     

            PointSpaceNode<DIM, T, SIZE>* father;     // links to father node                                     

            fBox<DIM> boundaryBox;      // boundary box of this node

            int32_t nb_siblings;        // current number of siblings. 
            int32_t next_free;          // index of the next free object. 
            int32_t splitting_index;    // index of the splitting plane for this node. 


            // no copy
             
            PointSpaceNode(const PointSpaceNode<DIM, T, SIZE>&) = delete; 
            PointSpaceNode<DIM, T, SIZE> & operator=(const PointSpaceNode<DIM, T, SIZE>&) = delete;



            /**
            * private ctor, create the root node.
            * No T object is created. 
            **/
            PointSpaceNode(const fBox<DIM>& B, int32_t split_index) : childs{ nullptr,nullptr }, father(nullptr), boundaryBox(B), nb_siblings(0), next_free(0), splitting_index(split_index)
                {
                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)obj;                
                for (int k = 0; k < SIZE - 1; k++) { o[k]._setNextFree(k + 1); }
                o[SIZE - 1]._setLastFree();
                }


            /**
            * private ctor, create the node with a first object
            * (use copy constructor for T)
            **/
            PointSpaceNode(PointSpaceNode<DIM,T,SIZE> * father_node, const fBox<DIM> & B, const fVec<DIM> & pos, int32_t split_index, const T & val) : childs{nullptr,nullptr}, father(father_node), boundaryBox(B), nb_siblings(1), next_free(1), splitting_index(split_index)
                {
                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)obj;
                new (&(o[0].data)) T(val);
                o[0]._position = pos;
                o[0]._child_index = 0;
                for(int k = 1; k < SIZE - 1; k++) { o[k]._setNextFree(k + 1); }
                o[SIZE - 1]._setLastFree();
                }


            /**
            * private ctor, create the node with a first object
            * (use default constructor for T)
            **/
            PointSpaceNode(PointSpaceNode<DIM,T,SIZE> * father_node, const fBox<DIM> & B, const fVec<DIM> & pos, int32_t split_index) : childs{nullptr,nullptr}, father(father_node), boundaryBox(B), nb_siblings(1), next_free(1), splitting_index(split_index)
                {
                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)obj;
                new (&(o[0].data)) T;
                o[0]._position = pos;
                o[0]._child_index = 0;
                for(int k = 1; k < SIZE - 1; k++) { o[k]._setNextFree(k + 1); }
                o[SIZE - 1]._setLastFree();
                }



            /**
            * Return the sub-box corresponding to a given index in [0 , 1]
            **/
            mtools::fBox<DIM> subBox(int i) const
                {
                const double mi = boundaryBox.min[splitting_index];
                const double ma = boundaryBox.max[splitting_index];
                fBox<DIM> B = boundaryBox;
                ((i == 0) ? B.max[splitting_index] : B.min[splitting_index]) = (mi + ma) / 2;
                return B; 
                }


            /**
            * Add an object
            * 
            * Use copy ctor for T
            **/
            template<size_t SIZE_NODE, size_t POOL_SIZE>
            PointSpaceObj<DIM, T>* addObj(const mtools::fVec<DIM>& pos, mtools::CstSizeMemoryPool<SIZE_NODE, POOL_SIZE>* nodePool, const T & val)
                {
                MTOOLS_INSURE(boundaryBox.isInside(pos));
                if (nb_siblings < SIZE)
                    { // we can add it here
                    nb_siblings++;
                    const int32_t nf = next_free;
                    MTOOLS_INSURE((nf >= 0) && (nf < SIZE)); // should be valid !
                    PointSpaceObj<DIM, T> * o = ((PointSpaceObj<DIM, T>*)obj) + nf;
                    new (&(o->data)) T(val); // copy ctor
                    next_free = o->_nextFree();
                    o->_child_index = nf;
                    o->_position = pos;
                    return o;
                    }
                else
                    {
                    const double mi = boundaryBox.min[splitting_index];
                    const double ma = boundaryBox.max[splitting_index];
                    const double mid = (mi + ma) / 2; 
                    const int i = (pos[splitting_index] <= mid) ? 0 : 1; 
                    if (childs[i])
                        { // sub-box already exists so we recurse inside
                        return (childs[i])->addObj(pos, nodePool, val);
                        }
                    // node does not exist so we create it
                    fBox<DIM> B = boundaryBox;
                    ((i == 0) ? B.max[splitting_index] : B.min[splitting_index]) = mid;
                    PointSpaceNode<DIM, T, SIZE>* n = (PointSpaceNode<DIM, T, SIZE>*)nodePool->malloc();
                    new (n) PointSpaceNode<DIM, T, SIZE>(this, B, pos, (splitting_index + 1) % DIM, val);
                    childs[i] = n;
                    return ((PointSpaceObj<DIM, T>*)(n->obj));
                    }
                }



            /**
            * Add an object
            * 
            * Use default ctor for T
            **/
            template<size_t SIZE_NODE, size_t POOL_SIZE>
            PointSpaceObj<DIM, T>* addObj(const mtools::fVec<DIM>& pos, mtools::CstSizeMemoryPool<SIZE_NODE, POOL_SIZE>* nodePool)
                {
                MTOOLS_INSURE(boundaryBox.isInside(pos));
                if (nb_siblings < SIZE)
                    { // we can add it here
                    nb_siblings++;
                    const int32_t nf = next_free;
                    MTOOLS_INSURE((nf >= 0) && (nf < SIZE)); // should be valid !
                    PointSpaceObj<DIM, T> * o = ((PointSpaceObj<DIM, T>*)obj) + nf;
                    new (&(o->data)) T; // copy ctor
                    next_free = o->_nextFree();
                    o->_child_index = nf;
                    o->_position = pos;
                    return o;
                    }
                else
                    {
                    const double mi = boundaryBox.min[splitting_index];
                    const double ma = boundaryBox.max[splitting_index];
                    const double mid = (mi + ma) / 2; 
                    const int i = (pos[splitting_index] <= mid) ? 0 : 1; 
                    if (childs[i])
                        { // sub-box already exists so we recurse inside
                        return (childs[i])->addObj(pos, nodePool);
                        }
                    // node does not exist so we create it
                    fBox<DIM> B = boundaryBox;
                    ((i == 0) ? B.max[splitting_index] : B.min[splitting_index]) = mid;                    
                    PointSpaceNode<DIM, T, SIZE>* n = (PointSpaceNode<DIM, T, SIZE> * )nodePool->malloc();
                    new (n) PointSpaceNode<DIM, T, SIZE>(this, B, pos, (splitting_index + 1) % DIM);
                    childs[i] = n;
                    return ((PointSpaceObj<DIM, T>*)(n->obj));
                    }
                }
            


            /**   
             * Remove object with a given index (call the dtor for T). 
             * 
             * Warning : This node may be destroyed in the process ! 
             **/
            template<bool CALLDTOR, size_t SIZE_NODE, size_t POOL_SIZE>
            void remove(int index, mtools::CstSizeMemoryPool<SIZE_NODE, POOL_SIZE>* nodePool)
                {
                MTOOLS_INSURE((index >= 0) && (index < SIZE));
                PointSpaceObj<DIM, T>* o = ((PointSpaceObj<DIM, T>*)obj);
                if (CALLDTOR) { (o[index].data).~T(); } // call the destructor.
                MTOOLS_INSURE((nb_siblings >= 1) && (nb_siblings <= SIZE));
                if (nb_siblings == SIZE)
                    {
                    MTOOLS_INSURE(next_free < 0);
                    o[index]._setLastFree();
                    }
                else
                    {
                    MTOOLS_INSURE((next_free >= 0) && (next_free < SIZE));
                    o[index]._setNextFree(next_free);
                    }
                next_free = index;
                nb_siblings--;
                prune(nodePool);
                return;
                }


            /** call this to prune the tree, may destroy this node !  */
            template<size_t SIZE_NODE, size_t POOL_SIZE>
            void prune(mtools::CstSizeMemoryPool<SIZE_NODE, POOL_SIZE>* nodePool)
                {
                if ((nb_siblings == 0) && (childs[0] == nullptr) && (childs[1] == nullptr))
                    {
                    PointSpaceNode<DIM,T,SIZE> * const f = father;
                    if (f != nullptr)
                        {
                        f->childs[((father->childs[0] == this) ? 0 : 1)] = nullptr;
                        nodePool->free(this); // do not call dstructor because there is no more T object in it...
                        f->prune(nodePool); // recurse
                        }
                    }
                }


            /** for debug purposes */
            std::string toString(int sp = 0)
                {
                mtools::ostringstream os;
                os << _spaces(sp) << "Node: " << (size_t)(this) << "\n";
                os << _spaces(sp) << " - boundary box : " << boundaryBox << "\n";
                os << _spaces(sp) << " - nb siblings  : " << nb_siblings << " / " << SIZE << "\n";
                PointSpaceObj<DIM, T>* o = ((PointSpaceObj<DIM, T>*)obj);
                for (int k = 0; k < SIZE; k++)
                    {
                    os << _spaces(sp) << "   |-> [" << k << "] ";
                    if (o[k]._child_index >= 0)
                        {
                        os << "(USED) pos = " << o[k]._position <<  " - value: " << mtools::toString(o[k].data) << "\n";
                        }
                    else
                        {
                        os << "(FREE) next = " << o[k]._nextFree() << "\n";
                        }                                               
                    }
                os << _spaces(sp) << " - sub Nodes \n\n";
                for (int i = 0; i < 2; i++)
                    {
                    if (childs[i] == nullptr)
                        {
                        os << _spaces(sp) << "    |-> NULLPTR\n\n";
                        }
                    else
                        {
                        os << _spaces(sp) << "    |-> created.\n";
                        os << (childs[i])->toString(sp + 4);
                        }
                    }
                os << "\n";
                return os.toString();
                }


            /** for debugging purpose */
            std::string _spaces(int n)
                {
                return std::string(n, ' ');
                }




        };






    /**
    * PointSpace. d-dimensional spatial container for points. 
    * 
    * The container is a variation of the usual k-d tree, where each box is split exactly in half along a direction at each level.
    * 
    * To each point is associated a data/payload of template type T (which can be any type, even a complicated class). 
    * 
    * Operations:
    * 
    * - adding a point (with payload) : O(log n)
    * - removing a point O(1)
    * - finding a point / nearest neighbour (log n).   
    * - iterating inside a region (log(n)). 
    * 
    * NOTE:
    * 
    * - the template type T must have either a public default constructor or a public copy constructor.   
    * - Every PointSpaceObj pointer remains indefinitely (i.e. until it is removed from the container).  
    * - deleting will try to free up space but may be inefficient in some cases. 
    * 
    **/
    template<int DIM, typename T, size_t NB_OBJ_PER_NODE = 10, size_t NB_NODE_PER_MEM_POOL = 1000000 / NB_OBJ_PER_NODE>
    class PointSpace
        {

        template<int, typename, size_t, size_t> friend class PointSpace; // friend all classes with different template paramters

        public:

            // short name
            typedef PointSpaceObj<DIM, T> PSO; 


            /****************************************************************************************************************
            *
            * creation / destruction 
            *
            *****************************************************************************************************************/

            /**
             * Create an empty container with a given main bounding box.
             *
             * @param   mainBoundingBox Initial bounding box of the container. the main bounding box gets
             *                          enlarged as needed (but doing so is inefficient so it is better to
             *                          give the correct bounding box on start).
             * @param   calldtorOnExit  (Optional) True to call the destructors of the T elements when they
             *                          are removed/destroyed.
            **/
            PointSpace(const mtools::fBox<DIM>& mainBoundingBox, bool callDtor = false) : _bounding_box(mainBoundingBox), _nb_obj(0), _calldtor(callDtor)
                {
                _root = (PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>*)_nodePool.malloc();
                new (_root) PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>(_bounding_box, 0);
                }


            /**
            * Create an empty container.
            * 
            * Same as above but with default bounding box [0,1]^DIM.
            **/
            PointSpace(bool callDtor = false) : _nb_obj(0), _calldtor(callDtor)
                {
                for (int k = 0; k < DIM; k++)
                    {
                    _bounding_box.min[k] = 0;
                    _bounding_box.max[k] = 1;
                    }
                _root = (PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>*)_nodePool.malloc();
                new (_root) PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>(_bounding_box, 0);
                }


            /**
             * Dtor.
             * 
             * If callDtor is set to true, all the destructors of the remaining T objects are call.
            **/
            ~PointSpace()
                {
                if (_calldtor)
                    {
                    _nodePool.destroyAndFreeAll<PointSpaceNode<DIM, T, NB_OBJ_PER_NODE> >(true);
                    }
                }


            /**
             * Copy ctor (deep copy).
             *
             * - Use the copy constructor for the T data type.
             * - The container created mirrors psp but its layout may differ.
            **/
            PointSpace(const PointSpace & psp) : PointSpace(psp._bounding_box, psp._calldtor)
                {
                append(psp);
                }


            /**
             * Copy ctor (deep copy) with type conversion.
             *
             * - Use the copy constructor for the T data type (for construction and type conversion).
             * - The container created mirrors psp but its layout may differ. 
            **/
            template<typename T2, size_t NB_OBJ_PER_NODE2, size_t NB_NODE_PER_MEM_POOL2> 
            PointSpace(const PointSpace<DIM, T2, NB_OBJ_PER_NODE2, NB_NODE_PER_MEM_POOL2> & psp) : PointSpace(psp._bounding_box, psp._calldtor)
                {
                append(psp);
                }


            /**
             * Assignment operator (deep copy). 
             *
             * - Use the copy constructor for the T data type.
             * - The container created mirrors psp but its layout may differ.
            **/
            PointSpace& operator=(const PointSpace & psp)
                {
                clear();
                _bounding_box = psp._bounding_box;
                append(psp);
                return *this;
                }


            /**
             * Assignment operator (deep copy) with type conversion.
             *
             * - Use the copy constructor for the T data type (for construction and type conversion).
             * - The container created mirrors psp but its layout may differ.
            **/
            template<typename T2, size_t NB_OBJ_PER_NODE2, size_t NB_NODE_PER_MEM_POOL2>
            PointSpace& operator=(const PointSpace<DIM, T2, NB_OBJ_PER_NODE2, NB_NODE_PER_MEM_POOL2> & psp)
                {
                clear();
                _bounding_box = psp._bounding_box;
                append(psp);
                return *this;
                }



            /****************************************************************************************************************
            *
            * Attributes / Infos / Stats
            *
            *****************************************************************************************************************/


            /**
            * Return the memory currently allocated (in bytes).
            **/
            size_t memoryAllocated() const { return sizeof(*this) + _nodePool.footprint(); }


            /**
            * Return the memory currently used by the grid (in bytes).
            **/
            size_t memoryUsed() const { return sizeof(*this) + _nodePool.used(); }


            /**
             * Query whether destructors are called when a T object is deleted/removed.
            **/
            void callDtors() const
                {
                return _calldtor;
                }


            /**
             * Set whether destructors are called when a T object is deleted/removed.
            **/
            void callDtors(bool callDtor)
                {
                _calldtor = callDtor;
                }


            /**
            * Return the number of object currently inserted in the container.
            **/
            int64 size() const
                {
                return _nb_obj;
                }


            /**
            * Return the current master bounding box. 
            * 
            * May be larger than the initial bounding box if points outside were added. 
            **/
            mtools::fBox<DIM> currentBoundingBox() const
                {
                return _root->boundaryBox;
                }


            /**
            * Return the initial bounding bounding box set at creation time.
            **/
            mtools::fBox<DIM> initialBoundingBox() const
                {
                return _bounding_box;
                }


            /**
            * Returns a string with some information concerning the object.
            *
            * @param   debug   Set this flag to true to enable the debug mode where the whole tree structure
            *                  of the container is written into the string [should not be used for large objects].
            *
            * @return  an info string.
            **/
            std::string toString(bool debug = false) const
                {
                mtools::ostringstream os;
                os << "PointSpace<" << DIM << " , " << typeid(T).name() << " , " << NB_OBJ_PER_NODE << " , " << NB_NODE_PER_MEM_POOL  << ">\n";
                os << " - Memory : " << mtools::toStringMemSize(memoryUsed()) << " / " << mtools::toStringMemSize(memoryAllocated()) << "\n";
                os << " - nb obj  :" << _nb_obj << "\n";
                os << " - initial bounding box : " << _bounding_box << "\n";
                os << " - current bounding box : " << _root->boundaryBox << "\n";
                if (debug) 
                    {
                    os << "\n";
                    os << _root->toString(0);
                    }
                return os.str();
                }


            /****************************************************************************************************************
            *
            * Load / save / reset / copy / append
            *
            *****************************************************************************************************************/


            /**
            * Empty the container. 
            * 
            * - If callDtor is set to true, the destructors of the remaining T objects are call.
            * - Reset the bounding box to its initial value.
            **/
            void clear()
                {
                if (_calldtor)
                    {
                    _nodePool.destroyAndFreeAll<PointSpaceNode<DIM, T, NB_OBJ_PER_NODE> >(false);
                    }
                else
                    {
                    _nodePool.freeAll(false);
                    }
                _nb_obj = 0;
                _root = (PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>*)_nodePool.malloc();
                new (_root) PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>(_bounding_box, 0);
                }


            /**
            * Empty the container. Same as clear().
            **/
            void reset()
                {
                clear();
                }


            /**
            * Save the container to a file.
            * 
            * The data type T must be serializable. 
            **/
            void save(const std::string& filename) const
                {
                OFileArchive ar(filename);
                serialize(ar);
                }


            /**
            * Load the container from a file. 
            * 
            * Remove the current content.
            *
            * The data type T must be deserializable and default constructible (via T())
            **/
            void load(const std::string& filename)
                {
                IFileArchive ar(filename);
                deserialize(ar);
                }


            /**
            * Serialize the object into an archive. 
            * 
            * The data type T must be serializable.
            **/
            template<typename OARCHIVE> void serialize(OARCHIVE & ar) const
                {
                ar << "PointSpace<" << DIM << "," << typeid(A).name() << ">\n";
                ar & DIM; ar << "\n";
                ar & _bounding_box; ar << "\n";
                ar & _nb_obj; ar << "\n";
                iterate_const([&](PointSpaceObj<DIM, T>& obj)
                        {
                        ar& obj.position();
                        ar& obj.data;
                        return true; // continue iteration
                        });
                }


            /**
            * Serialize the object into an archive. 
            * 
            * Remove the current content.
            *
            * The data type T must be deserializable and default constructible (via T())
            **/
            template<typename IARCHIVE> void deserialize(IARCHIVE & ar) 
                {
                int dim;
                ar & dim; 
                MTOOLS_INSURE(dim == DIM);
                ar & _bounding_box;
                clear(); // clear and set the new bounding box
                int nbo;
                ar & nbo;
                for (int k = 0; k < nbo; k++)
                    {
                    fVec<DIM> pos;
                    ar & pos;
                    ar & ((insert(pos))->data); 
                    }
                }


            /**
            * Add the element from another container into this one (and convert type). deep copy. 
            * 
            * Element already present are unchanged.
            * This is the same as the append() method.
            **/
            template<typename T2, size_t NB_OBJ_PER_NODE2, size_t NB_NODE_PER_MEM_POOL2>
            void operator+=(const PointSpace<DIM, T2, NB_OBJ_PER_NODE2, NB_NODE_PER_MEM_POOL2>& psp)
                {
                append(psp);
                }


            /**
            * Add the elements from another container into this one. deep copy.
            *
            * Element already present are unchanged.
            * This is the same as the operator+=() method.
            **/
            template<size_t NB_OBJ_PER_NODE2, size_t NB_NODE_PER_MEM_POOL2>
            void append(const PointSpace<DIM, T, NB_OBJ_PER_NODE2, NB_NODE_PER_MEM_POOL2>& psp)
                {
                psp.iterate_const([&](PointSpaceObj<DIM, T>& obj)
                    {
                        insert(obj.position(), obj.data);
                        return true; // continue iteration
                    });
                }


            /**
            * Add the elements from another container into this one (and convert type). deep copy.
            *
            * Element already present are unchanged.
            * This is the same as the operator+=() method.
            **/
            template<typename T2, size_t NB_OBJ_PER_NODE2, size_t NB_NODE_PER_MEM_POOL2>
            void append(const PointSpace<DIM, T2, NB_OBJ_PER_NODE2, NB_NODE_PER_MEM_POOL2>& psp)
                {
                psp.iterate_const([&](PointSpaceObj<DIM, T2>& obj)
                    {
                        insert(obj.position(), (T)(obj.data));
                        return true; // continue iteration
                    });
                }



            /****************************************************************************************************************
            *
            * Adding / removing / accessing elements
            *
            *****************************************************************************************************************/


            /**
             * Add an object inside the container. 
             * This method always insert a new object, even if there is already an object at that position.
             * 
             * This method return a pointer to a PointSpaceObj object whose .data member is created with
             * the default T() constructor.
            **/
            PointSpaceObj<DIM, T>* insert(const mtools::fVec<DIM> & pos)
                {
                if (!(_root->boundaryBox.isInside(pos))) _rootUp(pos);
                _nb_obj++;
                return _root->addObj(pos, &_nodePool);                  
                }



            /**
             * Add an object inside the container.
             * This method always insert a new object, even if there is already an object at that position.
             *
             * This method return a pointer to a PointSpaceObj object whose .data member is created with
             * the default T() constructor.
             * 
             * The hint object is used to improved the search in the tree. the hinted object should be 'near' 
             * pos for optimal results. 
             **/
            PointSpaceObj<DIM, T>* insert(const mtools::fVec<DIM>& pos, const PointSpaceObj<DIM, T>* hint)
                {
                if (hint != nullptr)
                    {
                    auto node = hint->_getNode<NB_OBJ_PER_NODE>();
                    if (node->boundaryBox.isInside(pos))
                        {
                        _nb_obj++;
                        return node->addObj(pos, &_nodePool);
                        }
                    }
                // fallback
                return insert(pos);
                }


            /**
             * Add an object inside the container.
             * This method always insert a new object, even if there is already an object at that position.
             * 
             * This method return a reference to a PointSpaceObj object whose .data member is created with
             * the default T() constructor.
            **/
            PointSpaceObj<DIM, T> & operator[](const mtools::fVec<DIM> & pos)
                {
                return (*insert(pos));
                }


            /**
             * Add an object inside the container with a given payload.
             * This method always insert a new object, even if there is already an object at that position.
             *
             * This method return a pointer to a PointSpaceObj object whose .data member is created with
             * the copy ctor using val. 
            **/
            PointSpaceObj<DIM, T>* insert(const mtools::fVec<DIM> & pos, const T & val)
                {
                if (!(_root->boundaryBox.isInside(pos))) _rootUp(pos);
                _nb_obj++;
                return _root->addObj(pos, &_nodePool,val);
                }


            /**
             * Add an object inside the container with a given payload.
             * This method always insert a new object, even if there is already an object at that position.
             *
             * This method return a pointer to a PointSpaceObj object whose .data member is created with
             * the copy ctor using val. 
             *
             * The hint object is used to improved the search in the tree. the hinted object should be 'near'
             * pos for optimal results.
            **/
            PointSpaceObj<DIM, T>* insert(const mtools::fVec<DIM> & pos, const T & val, const PointSpaceObj<DIM, T>* hint)
                {
                if (hint != nullptr)
                    {
                    auto node = hint->_getNode<NB_OBJ_PER_NODE>();
                    if (node->boundaryBox.isInside(pos))
                        {
                        _nb_obj++;
                        return node->addObj(pos, &_nodePool, val);
                        }
                    }
                // fallback
                return insert(pos, val);
                }


            /**
             * Add an object inside the container.
             * This method always insert a new object, even if there is already an object at that position.
             *
             * This method return a reference to a PointSpaceObj object whose .data member is created with
             * the copy ctor using val.
            **/
            PointSpaceObj<DIM, T> & operator()(const mtools::fVec<DIM> & pos, const T& val)
                {
                return (*insert(pos, val));
                }


            /**
            * Remove an object from the container.
            * 
            * This calls the destructor ~T() on the .data member. 
            **/
            void remove(PointSpaceObj<DIM, T> * obj)
                {
                PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>* node = obj->_getNode< NB_OBJ_PER_NODE>();
                if (_calldtor) 
                    { 
                    node->remove<true, sizeof(PointSpaceNode < DIM, T, NB_OBJ_PER_NODE>), NB_NODE_PER_MEM_POOL >(obj->_child_index, &_nodePool); 
                    }
                else 
                    {
                    node->remove<false, sizeof(PointSpaceNode < DIM, T, NB_OBJ_PER_NODE>), NB_NODE_PER_MEM_POOL >(obj->_child_index, &_nodePool);
                    }
                _nb_obj--;
                }



            /****************************************************************************************************************
            *
            * Iterations 
            *
            *****************************************************************************************************************/



            /**
            * Add into a vector a pointer for each element in the container which are inside a given (closed) box B.
            * 
            * - The vector is not cleared. 
            * - Return the number of element found and added into the vector.
            **/
            size_t vector(std::vector<PointSpaceObj<DIM, T>*>& vec, fBox<DIM>& B) const
                {
                size_t nb = 0;
                iterate_const(B, [&](const PointSpaceObj & obj) {
                    vec.push_back((PointSpaceObj<DIM, T>*)&obj);
                    });
                return nb;
                }


            /**
             * Iterate over all entries in the container which are in the (colsed) box B.
             *
             * Return true if iteration completed and false if it was interrupted.
             * 
             * @param   box The (closed) box inside which we iterate.
             * @param   fun method called for each entry found. The signature of fun must be compatible with
             *              'bool fun(PointSpaceObj<DIM, T> & obj)'
             *              the function must return true to continu iteration and falseto stop the iteration. 
            **/
            template<typename FUN> bool iterate(const fBox<DIM>& box, FUN fun)
                {
                return _iterate(_root, box, fun);
                }


            /**
             * Iterate over all entries in the container which are in the box B (const version).
             *
             * Return true if iteration completed and false if it was interrupted.
             *
             * @param   box The (closed) box inside which we iterate.
             * @param   fun method called for each entry found. The signature of fun must be compatible with
             *              'bool fun(const PointSpaceObj<DIM, T> & obj)'
             *              the function must return true to continu iteration and falseto stop the iteration. 
            **/
            template<typename FUN> bool iterate_const(const fBox<DIM>& box, FUN fun) const
                {
                return _iterate_const(_root, box, fun);
                }


            /**
             * Iterate over all entries in the container.
             *
             * Return true if iteration completed and false if it was interrupted.
             *
             * @param   fun method called for each entry found. The signature of fun must be compatible with
             *              'bool fun(PointSpaceObj<DIM, T> & obj)'
             *              the function must return true to continu iteration and falseto stop the iteration. 
            **/
            template<typename FUN> bool iterate(FUN fun)
                {
                return _iterate(_root, _root->boundaryBox, fun);
                }


            /**
             * Iterate over all entries in the container (const version).
             *
             * Return true if iteration completed and false if it was interrupted.
             *
             * @param   fun method called for each entry found. The signature of fun must be compatible with
             *              'bool fun(const PointSpaceObj<DIM, T> & obj)'
             *              the function must return true to continu iteration and falseto stop the iteration. 
            **/
            template<typename FUN> bool iterate_const(FUN fun) const
                {
                return _iterate_const(_root, _root->boundaryBox, fun);
                }


            
            /**
             * Iterate over the entries of the container (advanced version).
             *
             * Return true if iteration completed and false if it was interrupted.
             *
             * @param   target  'target point' used to decide the order in which the boxes/nodes are traversed. 
             *                  the algorithm tries to choose the node closest to target point first. 
             * 
             * @param   fun_obj Function called for each entry found. Signature must be compatible with
             *                  'bool fun_obj(PointSpaceObj<DIM, T> & obj)'
             *                  the function must return true to continue iteration and false to abort.
             * 
             * @param   fun_box Function called to decide whether to examine a new box. Signature must be compatible with
             *                  'bool fun_box(const fBox<DIM> & box)'
             *                  the function must return true to iterate inside the box and false to skip this box.
            **/
            template<typename FUN_OBJ, typename FUN_BOX> bool iterate(const fVec<DIM> & target, FUN_OBJ fun_obj, FUN_BOX fun_box)
                {
                return _iterate(_root, target, fun_obj, fun_box);
                }


            /**
             * Iterate over the entries of the container (advanced version). Const version
             *
             * Return true if iteration completed and false if it was interrupted.
             *
             * @param   target  'target point' used to decide the order in which the boxes/nodes are traversed.
             *                  the algorithm tries to choose the node closest to target point first.
             *
             * @param   fun_obj Function called for each entry found. Signature must be compatible with
             *                  'bool fun_obj(const PointSpaceObj<DIM, T> & obj)'
             *                  the function must return true to continue iteration and false to abort.
             *
             * @param   fun_box Function called to decide whether to examine a new box. Signature must be compatible with
             *                  'bool fun_box(const fBox<DIM> & box)'
             *                  the function must return true to iterate inside the box and false to skip this box.
            **/
            template<typename FUN_OBJ, typename FUN_BOX> bool iterate_const(const fVec<DIM> & target, FUN_OBJ fun_obj, FUN_BOX fun_box) const
                {
                return _iterate_const(_root, target, fun_obj, fun_box);
                }



            /****************************************************************************************************************
            *
            * Neighbour search
            *
            *****************************************************************************************************************/


            /**
            * Return the first object found at a given position or nullptr if there is no
            * object at position pos in the container. 
            *
            * Note: there may be several object at position pos. This method return only the 
            *       first object found. use findAll() to find all object at a given position.
            **/
            PointSpaceObj<DIM, T>* find(const mtools::fVec<DIM>& pos) const
                {
                PointSpaceObj<DIM, T> * po = nullptr;
                iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T> & obj)
                        {
                        if (obj.position() == pos)
                            {
                            po = (PointSpaceObj<DIM, T>*)&obj;
                            return false;
                            }
                        return true; // continue iteration
                        },
                    [&](const fBox<DIM>& box)
                        {
                        return box.isInside(pos);
                        });
                return po;
                }


            /**
            * Return a vector containing pointers to all the object at position pos.
            **/
            std::vector<PointSpaceObj<DIM, T>*> findAll(const mtools::fVec<DIM>& pos) const
                {
                std::vector<PointSpaceObj<DIM, T>*> vec;
                iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj)
                        {
                        if (obj.position() == pos)
                            {
                            vec.push_back((PointSpaceObj<DIM, T>*) & obj);
                            }
                        return true; // continue iteration
                        },
                    [&](const fBox<DIM>& box)
                        {
                        return box.isInside(pos);
                        });
                return vec;
                }



            /**
            * Return (one of) the object closest to a position pos. 
            *
            * Uses the usual euclidian metric
            **/
            PointSpaceObj<DIM, T>* findNearest(const fVec<DIM> & pos) const
                {
                return findNearest(pos, EuclidianMetric<DIM>());
                }


            /**
            * Return (one of) the object closest to obj (but excluding obj). 
            *
            * Uses the usual euclidian metric.
            **/
            PointSpaceObj<DIM, T>* findNearest(const PointSpaceObj<DIM, T> * obj) const
                {
                return findNearest(obj, EuclidianMetric<DIM>());
                }


            /**
            * Return the k objects closest to position pos.
            * 
            * Result is returned in the form (distance, object) in the result map (emptied first).
            *
            * Uses the usual euclidian metric.
            **/
            void findKNearest(std::multimap<double, PointSpaceObj<DIM, T>*>& result, int k, const mtools::fVec<DIM> & pos) const
                {
                findKNearest(result, k, pos, EuclidianMetric<DIM>());
                }


            /**
            * Return the k objects closest to obj (but excluding obj). 
            *
            * Result is returned in the form (distance, object) in the result map (emptied first).
            *
            * Uses the usual euclidian metric.
            **/            
            void findKNearest(std::multimap<double, PointSpaceObj<DIM, T>*> & result, int k, const PointSpaceObj<DIM, T> * obj) const
                {
                findKNearest(result, k, obj, EuclidianMetric<DIM>());
                }
            

            /**
             * Iterate over all objects located inside the closed euclidian ball with given radius.
             *
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(const PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to pos. the function must
             * return true to continue iteration and false to abort.
             * 
             * The method return false if iteration was aborted early and true if iteration completed normally.
            **/
            template<typename FUN>
            bool iterateBall(const fVec<DIM>& pos, double radius, FUN fun)
                {
                return iterateBall(pos, radius, fun, EuclidianMetric<DIM>());;
                }


            /**
             * Iterate over all objects located inside the closed euclidian ball with given radius.
             *
             * obj itself is not passed to the callback.
             *
             * The callback function fun is called for each entry found. Signature must be compatible with 
             * 'bool fun(const PointSpaceOb<DIM,T> & o, double d)' 
             * where 'o' is the object found and 'd' its distance to obj. the function must
             * return true to continue iteration and false to abort.
             * 
             * The method return false if iteration was aborted early and true if iteration completed normally.
            **/
            template<typename FUN>
            bool iterateBall(const PointSpaceObj<DIM, T> * obj, double radius, FUN fun)
                {
                return iterateBall(obj, radius, fun, EuclidianMetric<DIM>());
                }


            /**
             * Iterate over all objects located inside the closed euclidian ball with given radius.
             * (const version)
             * 
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(const PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to pos. the function must
             * return true to continue iteration and false to abort.
             *
             * The method return false if iteration was aborted early and true if iteration completed normally.
            **/
            template<typename FUN>
            bool iterateBall_const(const fVec<DIM> & pos, double radius, FUN fun) const
                {
                return iterateBall_const(pos, radius, fun, EuclidianMetric<DIM>());
                }


            /**
             * Iterate over all objects located inside the closed euclidian ball with given radius.
             * (const version).
             *
             * obj itself is not passed to the callback.
             *
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(const PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to obj. the function must
             * return true to continue iteration and false to abort.
             *
             * The method return false if iteration was aborted early and true if iteration completed normally.
            **/
            template<typename FUN>
            bool iterateBall_const(const PointSpaceObj<DIM, T> * obj, double radius, FUN fun) const
                {
                return iterateBall_const(obj, radius, fun, EuclidianMetric<DIM>());
                }





            /****************************************************************************************************************
            *
            * Neighbour search (with given metric)
            *
            *****************************************************************************************************************/




            /**
            * Return (one of) the object closest to a given position for a given metric. 
            * 
            * Use a custom metric, ie a functor that satisfies:
            * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
            * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC>
            PointSpaceObj<DIM, T>* findNearest(const fVec<DIM> & pos, const  METRIC metric) const
                {
                PointSpaceObj<DIM, T>* co = nullptr; // current closest
                double d = mtools::INF; // and associated distance
                iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj)
                        {
                        auto objpos = obj.position();
                        double nd = metric(pos, objpos);
                        if (nd < d)
                            {
                            d = nd;
                            co = (PointSpaceObj<DIM, T>*)&obj;
                            if (d <= 0) return false; // cannot do better so we stop
                            }
                        return true; // continue iteration
                        },
                    [&](const fBox<DIM>& box)
                        {
                        return (metric(pos, box) <= d); // only look at boxes whose distance is at most d
                        });
                return co;
                }


            /**
            * Return (one of) the object closest to obj (but excluding obj) for a given metric.
            *
            * Use a custom metric, ie a functor that satisfies:
            * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
            * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC>
            PointSpaceObj<DIM, T>* findNearest(const PointSpaceObj<DIM, T> * o, const METRIC metric) const
                {
                PointSpaceObj<DIM, T>* co = nullptr; // current closest
                double d = mtools::INF; // and associated distance
                const auto pos = o->position();
                iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj)
                        {
                        if (&obj == o) return true; // skip 
                        auto objpos = obj.position();
                        double nd = metric(pos, objpos);
                        if (nd < d)
                            {
                            d = nd;
                            co = (PointSpaceObj<DIM, T>*) &obj;
                            if (d <= 0) return false; // cannot do better so we stop
                            }
                        return true; // continue iteration
                        },
                    [&](const fBox<DIM>& box)
                        {
                        return (metric(pos, box) <= d); // only look at boxes whose distance is at most d
                        });
                return co;
                }


            /**
            * Return the k objects closest to a given position for a given metric.
            * 
            * Result is returned in the form(distance, object) in the result map(emptied first).
            *
            * Use a custom metric, ie a functor that satisfies:
            * 
            * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).            
            * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC>
            void findKNearest(std::multimap<double, PointSpaceObj<DIM, T>*> & result, int k, const mtools::fVec<DIM> & pos, const METRIC metric) const
                {
                result.clear();                
                double d = mtools::INF; // worst of the k distances, infinity if not yet k value in multimap
                iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj)
                        {
                        const double nd = metric(pos, obj.position());
                        if (nd <= d)
                            { // we must add obj to the map
                            result.insert({ nd , ((PointSpaceObj<DIM, T>*) & obj) }); // add to the map.
                            if (result.size() > k) { result.erase(std::prev(result.end())); d = (std::prev(result.end()))->first; }
                            }
                        return true; // continue iteration
                        },
                    [&](const fBox<DIM>& box)
                        {
                        return (metric(pos, box) <= d); // only look at boxes whose distance is at most d
                        });                
                }


            /**
            * Return the k objects closest to obj (but excluding o) for a given metric. 
            *
            * Result is returned in the form(distance, object) in the result map(emptied first).
            *
            * Use a custom metric, ie a functor that satisfies:
            * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
            * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC>
            void findKNearest(std::multimap<double, PointSpaceObj<DIM, T>*>& result, int k, const PointSpaceObj<DIM, T>* o, const METRIC metric) const
                {
                result.clear();                
                double d = mtools::INF; // worst of the k distances, infinity if not yet k value in multimap
                const auto pos = o->position();
                iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj)
                        {
                        const double nd = metric(pos, obj.position());
                        if (((&obj) != o)&&(nd <= d))
                            { // we must add obj to the map
                            result.insert({ nd , ((PointSpaceObj<DIM, T>*) & obj) }); // add to the map.
                            if (result.size() > k) { result.erase(std::prev(result.end())); d = (std::prev(result.end()))->first; }
                            }
                        return true; // continue iteration
                        },
                    [&](const fBox<DIM>& box)
                        {
                        return (metric(pos, box) <= d); // only look at boxes whose distance is at most d
                        });                
                }


            /**
             * Iterate over all objects located inside a closed ball with given radius (for a given metric)
             *
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to pos. the function must
             * return true to continue iteration and false to abort.
             *
             * The method return false if iteration was aborted early and true if iteration completed normally.
             *
             * Use a custom metric, ie a functor that satisfies:
             * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
             * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC, typename FUN>
            bool iterateBall(const fVec<DIM>& pos, double radius, FUN fun, const METRIC metric)
                {
                return iterate(pos,
                    [&](PointSpaceObj<DIM, T>& obj) -> bool
                        {
                        const double d = metric(pos, obj.position());
                        if (d <= radius) { return fun(obj, d); }
                        return true;
                        },
                    [&](const fBox<DIM>& box) -> bool
                        {
                        return (metric(pos, box) <= radius); // only look at boxes whose distance is at most d
                        });
                }


            /**
             * Iterate over all objects located inside a closed ball with given radius (for a given metric)
             *
             * obj itself is not passed to the callback.
             *
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to obj. the function must
             * return true to continue iteration and false to abort.
             *
             * The method return false if iteration was aborted early and true if iteration completed normally.
             *
             * Use a custom metric, ie a functor that satisfies:
             * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
             * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC, typename FUN>
            bool iterateBall(const PointSpaceObj<DIM, T>* o, double radius, FUN fun, const METRIC metric)
                {
                const auto pos = o->position();
                return iterate(pos,
                    [&](PointSpaceObj<DIM, T>& obj) -> bool
                        {
                        const double d = metric(pos, obj.position());
                        if ((&obj != o) && (d <= radius)) { return fun(obj, d); }
                        return true;
                        },
                    [&](const fBox<DIM>& box) -> bool
                        {
                        return (metric(pos, box) <= radius); // only look at boxes whose distance is at most d
                        });
                }


            /**
             * Iterate over all objects located inside a closed ball with given radius (for a given metric)
             * (const version)
             *
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(const PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to pos. the function must
             * return true to continue iteration and false to abort.
             *
             * The method return false if iteration was aborted early and true if iteration completed normally.
             *
             * Use a custom metric, ie a functor that satisfies:
             * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
             * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC, typename FUN>
            bool iterateBall_const(const fVec<DIM>& pos, double radius, FUN fun, const METRIC metric) const
                {
                return iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj) -> bool
                        {
                        const double d = metric(pos, obj.position());
                        if (d <= radius) { return fun(obj, d); }
                        return true;
                        },
                    [&](const fBox<DIM>& box) -> bool
                        {
                        return (metric(pos, box) <= radius); // only look at boxes whose distance is at most d
                        });
                }


            /**
             * Iterate over all objects located inside a closed ball with given radius (for a given metric)
             * (const version).
             *
             * obj itself is not passed to the callback. 
             * 
             * The callback function fun is called for each entry found. Signature must be compatible with
             * 'bool fun(const PointSpaceOb<DIM,T> & o, double d)'
             * where 'o' is the object found and 'd' its distance to obj. the function must
             * return true to continue iteration and false to abort.
             *
             * The method return false if iteration was aborted early and true if iteration completed normally.
             *
             * Use a custom metric, ie a functor that satisfies:
             * - double metric(const fVec<DIM> & P, const fVec<DIM> & Q) -> return dist(P,Q).
             * - double metric(const fVec<DIM> & P, const fBox<DIM> & B) -> return a LOWER BOUND on dist(P,B).
            **/
            template<typename METRIC, typename FUN>
            bool iterateBall_const(const PointSpaceObj<DIM, T>* o, double radius, FUN fun, const METRIC metric) const
                {
                const auto pos = o->position();
                return iterate_const(pos,
                    [&](const PointSpaceObj<DIM, T>& obj) -> bool
                        {
                        const double d = metric(pos, obj.position());
                        if ((&obj != o) &&  (d <= radius)) { return fun(obj, d); }
                        return true;
                        },
                    [&](const fBox<DIM>& box) -> bool
                        {
                        return (metric(pos, box) <= radius); // only look at boxes whose distance is at most d
                        });
                }




        private:




            /****************************************************************************************************************
            *
            * You shall not pass !
            *
            *****************************************************************************************************************/



            /** change the root so that the bounding box contains point pos. */
            void _rootUp(const mtools::fVec<DIM>& pos)
                {
                do 
                    { // we know we must enlarge bounding box
                    const int ind = (_root->splitting_index + DIM - 1) % DIM; // splitting direction. 
                    fBox<DIM> B = _root->boundaryBox;
                    int ni;
                    if (pos[ind] < B.min[ind])
                        {
                        B.min[ind] -= (B.max[ind] - B.min[ind]);
                        ni = 1;
                        }
                    else
                        {
                        B.max[ind] += (B.max[ind] - B.min[ind]);
                        ni = 0;
                        }
                    PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>* newroot = (PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>*)_nodePool.malloc();
                    new (newroot) PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>(B, ind);
                    if ((_nb_obj > 0)||(_root->childs[0] != nullptr)||(_root->childs[1] != nullptr))
                        {
                        newroot->childs[ni] = _root;
                        _root->father = newroot;
                        }
                    else
                        {
                        _nodePool.free(_root);
                        }
                    _root = newroot;
                    } 
                while (!(_root->boundaryBox.isInside(pos)));
                }


            /** Recursively iterate over the node and subnodes */
            template<typename FUN> 
            bool _iterate(PointSpaceNode<DIM, T, NB_OBJ_PER_NODE> * node, const fBox<DIM>& box, FUN fun)
                {
                if (intersectionRect(node->boundaryBox, box).isEmpty()) return true; // done

                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)node->obj;
                for (int k = 0; k < NB_OBJ_PER_NODE; k++)
                    {
                    if (o[k]._child_index == k)
                        { 
                        if (box.isInside(o[k].position()))
                            {
                            bool r = fun(o[k]);
                            if (!r) return false;
                            }
                        }
                    }
                if (node->childs[0] != nullptr)
                    {
                    bool r = _iterate(node->childs[0], box, fun);
                    if (!r) return false;
                    }
                if (node->childs[1] != nullptr)
                    {
                    bool r = _iterate(node->childs[1], box, fun);
                    if (!r) return false;
                    }
                return true;
                }


            /** Recursively iterate over the node and subnodes (const version) */
            template<typename FUN> 
            bool _iterate_const(const PointSpaceNode<DIM, T, NB_OBJ_PER_NODE> * node, const fBox<DIM>& box, FUN fun) const
                {
                if (intersectionRect(node->boundaryBox, box).isEmpty()) return true; // done

                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)node->obj;
                for (int k = 0; k < NB_OBJ_PER_NODE; k++)
                    {
                    if (o[k]._child_index == k)
                        { 
                        if (box.isInside(o[k].position()))
                            {
                            bool r = fun(o[k]);
                            if (!r) return false;
                            }
                        }
                    }
                if (node->childs[0] != nullptr)
                    {
                    bool r = _iterate_const(node->childs[0], box, fun);
                    if (!r) return false;
                    }
                if (node->childs[1] != nullptr)
                    {
                    bool r = _iterate_const(node->childs[1], box, fun);
                    if (!r) return false;
                    }
                return true;
                }



            /** Recursively iterate over the node and subnodes */
            template<typename FUN_OBJ, typename FUN_BOX> 
            bool _iterate(PointSpaceNode<DIM, T, NB_OBJ_PER_NODE> * node, const fVec<DIM>& target, FUN_OBJ fun_obj, FUN_BOX fun_box)
                {
                if (!fun_box(node->boundaryBox)) return true; // do not explore this node

                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)node->obj;
                for (int k = 0; k < NB_OBJ_PER_NODE; k++)
                    {
                    if (o[k]._child_index == k)
                        {
                        if (!fun_obj(o[k])) return false;
                        }
                    }

                const int ind = node->splitting_index;
                const double mid = (node->boundaryBox.min[ind] + node->boundaryBox.max[ind]) / 2;
                if (target[ind] <= mid)
                    {
                    if (node->childs[0] != nullptr)
                        {
                        if (!_iterate(node->childs[0], target, fun_obj, fun_box)) return false;
                        }
                    if (node->childs[1] != nullptr)
                        {
                        if (!_iterate(node->childs[1], target, fun_obj, fun_box)) return false;
                        }
                    }
                else
                    {
                    if (node->childs[1] != nullptr)
                        {
                        if (!_iterate(node->childs[1], target, fun_obj, fun_box)) return false;
                        }
                    if (node->childs[0] != nullptr)
                        {
                        if (!_iterate(node->childs[0], target, fun_obj, fun_box)) return false;
                        }
                    }
                return true;
                }



            /** Recursively iterate over the node and subnodes (const version) */
            template<typename FUN_OBJ, typename FUN_BOX> 
            bool _iterate_const(const PointSpaceNode<DIM, T, NB_OBJ_PER_NODE> * node, const fVec<DIM>& target, FUN_OBJ fun_obj, FUN_BOX fun_box) const
                {
                if (!fun_box(node->boundaryBox)) return true; // do not explore this node

                PointSpaceObj<DIM, T>* o = (PointSpaceObj<DIM, T>*)node->obj;
                for (int k = 0; k < NB_OBJ_PER_NODE; k++)
                    {
                    if (o[k]._child_index == k)
                        {
                        if (!fun_obj(o[k])) return false;
                        }
                    }

                const int ind = node->splitting_index;
                const double mid = (node->boundaryBox.min[ind] + node->boundaryBox.max[ind]) / 2;
                if (target[ind] <= mid)
                    {
                    if (node->childs[0] != nullptr)
                        {
                        if (!_iterate_const(node->childs[0], target, fun_obj, fun_box)) return false;
                        }
                    if (node->childs[1] != nullptr)
                        {
                        if (!_iterate_const(node->childs[1], target, fun_obj, fun_box)) return false;
                        }
                    }
                else
                    {
                    if (node->childs[1] != nullptr)
                        {
                        if (!_iterate_const(node->childs[1], target, fun_obj, fun_box)) return false;
                        }
                    if (node->childs[0] != nullptr)
                        {
                        if (!_iterate_const(node->childs[0], target, fun_obj, fun_box)) return false;
                        }
                    }
                return true;
                }





            fBox<DIM>                                   _bounding_box;
            PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>*    _root;
            int64                                       _nb_obj;
            bool                                        _calldtor;

            mtools::CstSizeMemoryPool<sizeof(PointSpaceNode<DIM, T, NB_OBJ_PER_NODE>), NB_NODE_PER_MEM_POOL> _nodePool;   // memory pool for nodes

    };



        /** specialization in dimension 1,2,3,4,5 */

        template<typename T> using PointSpace1D = PointSpace<1, T>;
        template<typename T> using PointSpace2D = PointSpace<2, T>;
        template<typename T> using PointSpace3D = PointSpace<3, T>;
        template<typename T> using PointSpace4D = PointSpace<4, T>;
        template<typename T> using PointSpace5D = PointSpace<5, T>;

        template<typename T> using PointSpaceObj1D = PointSpaceObj<1, T>;
        template<typename T> using PointSpaceObj2D = PointSpaceObj<2, T>;
        template<typename T> using PointSpaceObj3D = PointSpaceObj<3, T>;
        template<typename T> using PointSpaceObj4D = PointSpaceObj<4, T>;
        template<typename T> using PointSpaceObj5D = PointSpaceObj<5, T>;


}



/** end of file */