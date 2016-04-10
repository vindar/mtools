/** @file memory.hpp */
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


#include "error.hpp"
#include "metaprog.hpp"

#include <string>
#include <utility>
#include <string>
#include <type_traits>


namespace mtools
{

    /**
    * Macro that convert KB to bytes
    **/
    #define MEM_KB(_nb) (1024*_nb)

    /**
    * Macro that convert MB to bytes
    **/
    #define MEM_MB(_nb) (1048576*_nb)

    /**
    * Macro that convert GB to bytes
    **/
    #define MEM_GB(_nb) (1073741824*_nb)

    /**
    * Macro which compute how many object can fit in a given portion of memory
    * (return at least 1 even if its size is larger than the memory size).
    **/
    #define NB_FOR_SIZE(t,s) ((s/sizeof(t)) + 1)
    
    /**
     * Macro which compute the number of bytes taken by _nb object of type _type
     **/
    #define MEM_FOR_OBJ(_type, _nb) (sizeof(_type)*_nb)


        /**
         * A simple (but pretty fast) memory allocator for serving one element of type T at a time.
         * 
         * - The allocator requests memory from the OS via `malloc()` by chunk of POOLSIZE elements of
         * type T. The memory is returned to the OS with `free()` only when the allocator object itself
         * is destroyed (i.e. deallocating or destroying objects in the allocator does not release the
         * memory: it is kept for a subsequent allocation). Thus, the global memory footprint of the
         * program is non-decreasing in time.
         * 
         * - It is possible to deallocate the memory of one object / every object without calling their
         * destructor(s) via the `deallocate()`/`deallocateAll()` methods. Dumping the whole collection
         * of objects can sometime gives a nice speed-up of the program... Otherwise, traditionnal
         * destruction of one/all objects calling their destructors is performed using the
         * `destroy()`\`destroyAll()` methods. In particular, 'destroyAll' enables to destroy correctly
         * every objects still allocated without having to keep track of them.
         * 
         * - The memory size used for each object is LCM(sizeof(T), T*).  Therefore, for object with
         * size larger than a pointer, the memory overhead null (except for a couple of pointer for each
         * memory block).
         * 
         * - The Allocator is NOT thread safe! The user is responsible for synchronization when sharing
         * an allocator object between different thread (but the allocator contain no static member
         * hence distinct instances will not conflict with each other).  
         *
         * @tparam  T                   Type of objects to allocate.
         * @tparam  POOLSIZE            Number of T object per memory pool (default = such that the memory 
         *                              pool is around 100MB).
         * @tparam  DELETEOBJECTSONEXIT Set this to true (default) to call the destructors of all
         *                              remaining objects when the allocator is deleted. If set to false,
         *                              the memory is released but the objects are simply dumped wihout
         *                              proper destruction. Setting this template parameter flag to false
         *                              enable to allocate objects with private inaccessible destructors
         *                              (the code calling ~T() is not generated as long as the methods
         *                              destroy() and destroyAll() are not called).
         **/
        template<typename T, size_t POOLSIZE = NB_FOR_SIZE(T,MEM_MB(100)), bool DELETEOBJECTSONEXIT = true> class SingleAllocator
        {
        public:

        typedef T* pointer;     ///< A pointer to a T objet.


            /**
             * Default constructor. Create the memory allocator but does not allocate any memory.
             **/
            SingleAllocator() : _m_allocatedobj(0), _m_totmem(0), _m_firstfree(nullptr), _m_currentpool(nullptr), _m_firstpool(nullptr), _m_index(POOLSIZE) {}


            /**
             * Destructor. Delete all remaining objects by calling their destructors (unless the template
             * parameter DELETEOBJECTSONEXIT is set to false) and then finally release all the memory
             * allocated to the OS.
             **/
            ~SingleAllocator()
                { 
                if (_m_firstpool == nullptr) return; 
                _dtorDestroy(metaprog::dummy<DELETEOBJECTSONEXIT>() ); // call, if required the dtors of all object still alive. 
                }


            /**
             * Return a pointer to a memory location for storing an object of type T. throw std::bad_alloc
             * if allocation fails (if so we are doomed and the state of the object is undetermined)
             **/
            inline pointer allocate()
                {
                ++_m_allocatedobj;                    
                if (_m_firstfree != nullptr)
                    {
                    _pfakeT p = _m_firstfree;
                    _m_firstfree = _getnextfake(_m_firstfree);
                    return((pointer)p);
                    }
                if (_m_index == POOLSIZE)
                    {
                    if (_m_currentpool == nullptr)
                        {
                        _m_currentpool = (_pool*)malloc(sizeof(_pool));
                        if (_m_currentpool == nullptr) { MTOOLS_DEBUG("SingleAllocator, bad_alloc"); throw std::bad_alloc(); }
                        MTOOLS_ASSERT(((size_t)_m_currentpool) % 2 == 0); // alignement is at least mod 2
                        _m_totmem += sizeof(_pool);
                        _m_firstpool = _m_currentpool;
                        _m_currentpool->next  = nullptr;
                        }
                    else
                        {
                        if (_m_currentpool->next == nullptr)
                            {
                            _m_currentpool->next = (_pool*)malloc(sizeof(_pool));
                            if (_m_currentpool == nullptr) { MTOOLS_DEBUG("SingleAllocator, bad_alloc"); throw std::bad_alloc(); }
                            MTOOLS_ASSERT(((size_t)_m_currentpool->next) % 2 == 0); // alignement is at least mod 2
                            _m_totmem += sizeof(_pool);
                            _m_currentpool->next->next = nullptr;
                            }
                        _m_currentpool = _m_currentpool->next;
                        }
                    _m_index = 0;
                    }
                pointer r = (pointer)(_m_currentpool->tab + _m_index);
                _m_index++;
                return(r);
                }


            /**
             * Release the memory allocated at adress p WITHOUT calling the destructor of the pointed object.
             **/
            inline void deallocate(pointer p)
                {
                MTOOLS_ASSERT(_m_firstpool != nullptr);
                MTOOLS_ASSERT(_m_allocatedobj > 0);
                --_m_allocatedobj;
                _getnextfake((_pfakeT)p) = _m_firstfree;
                _m_firstfree = (_pfakeT)p;
                }

            /**
             * Delete the object at adress p. Call its destructor and then release it memory to the allocator.
             **/
            inline void destroy(pointer p)
                {
                MTOOLS_ASSERT(_m_firstpool != nullptr);
                MTOOLS_ASSERT(_m_allocatedobj > 0);
                p->~T();
                deallocate(p);
                }


            /**
             * Release all the memory still allocated WITHOUT calling the destructors (fast).
             *
             * @param   releaseMemoryToOS   true to release the malloced memory to the operating system
             *                              (false by default).
             **/
            inline void deallocateAll(bool releaseMemoryToOS = false)
                {
                if (_m_firstpool == nullptr) return;
                _m_firstfree = nullptr;
                _m_currentpool = _m_firstpool;
                _m_allocatedobj = 0;
                _m_index = 0;
                if (releaseMemoryToOS) { _releaseMallocedMemory(); }
                }


            /**
             * Call the destructors of every objects still allocated then release all the memory to the
             * allocator.
             *
             * @param   releaseMemoryToOS   true to release the malloced memory to the operating system
             *                              (false by default).
             **/
            void destroyAll(bool releaseMemoryToOS = false)
                {
                if (_m_firstpool == nullptr) return;
                // we call the dtor for all the sites with lower bit set since they cannot be free sites (memory adresses are aligned mod 2)
                _pool * p = _m_firstpool;
                while (p != _m_currentpool)
                    {
                    for (size_t i = 0; i < POOLSIZE; i++)
                        {
                        _pfakeT f = (p->tab + i); if ((((size_t)_getnextfake(f)) % 2) != 0) { ((pointer)f)->~T(); }
                        }
                    p = p->next;
                    }
                for (size_t i = 0; i < _m_index; i++)
                    {
                    _pfakeT f = (p->tab + i); if ((((size_t)_getnextfake(f)) % 2) != 0) { ((pointer)f)->~T(); }
                    }
                // iterate over the list of free site and put every lowest bit to 1
                while (_m_firstfree != nullptr)
                    {
                    _pfakeT  f = _getnextfake(_m_firstfree); _getnextfake(_m_firstfree) = ((_pfakeT)(1)); _m_firstfree = f;
                    }
                // we call the dtor for all the sites with lower bit = 0, these are exactly the site whose dtor has not yet been called.
                p = _m_firstpool;
                while (p != _m_currentpool)
                    {
                    for (size_t i = 0; i < POOLSIZE; i++)
                        {
                        _pfakeT f = (p->tab + i); if ((((size_t)_getnextfake(f)) % 2) == 0) { ((pointer)f)->~T(); }
                        }
                    p = p->next;
                    }
                for (size_t i = 0; i < _m_index; i++)
                    {
                    _pfakeT f = (p->tab + i); if ((((size_t)_getnextfake(f)) % 2) == 0) { ((pointer)f)->~T(); }
                    }
                // finally, we release the memory to the allocator (and, optionnaly to the OS). 
                deallocateAll(releaseMemoryToOS);
                }


            /**
             * Return the memory size currently allocated by the T objects.
             *
             * @return  the number of bytes currently allocated as T objects (i.e. this value is equal to
             *          sizeof(T) times the number of T objects currently allocated.
             **/
            inline size_t used() const {return(sizeof(T)*_m_allocatedobj);}
            
            /**
             * Return the total memory size "malloc-ed" by the allocator object. This quantity never decrease 
             * since memory is not release until destruction of the allocator.
             *
             * @return  The number of bytes consumed by the allocator
             **/
            size_t footprint() const { return (_m_totmem); }


            /**
             * Print some information about the state of the allocator
             *
             * @return  A std::string that with info about the allocator.
             **/
            std::string toString() const 
                {
                std::string s = std::string("Single Allocator<") + typeid(T).name() + ">\n";
                s += std::string(" - memory allocated : ") + mtools::toString(used() / (1024 * 1024)) + "MB (" + mtools::toString(_m_allocatedobj) + " objects)\n";
                s += std::string(" - memory footprint : ") + mtools::toString(footprint() / (1024 * 1024)) + "MB\n";
                return s;
                }

            private:

            SingleAllocator(const SingleAllocator &) = delete;                  // no copy
            SingleAllocator & operator=(const SingleAllocator &) = delete;      //


            /* a fake T large enough to contain also a pointer, alignement such that T and T* can be safely stored */
            typedef typename std::aligned_storage<metaprog::static_lcm<sizeof(T*), sizeof(T)>::value>::type _fakeT;

            /* pointer to a fake T*/
            typedef _fakeT * _pfakeT;

            /* a memory pool */
            struct _pool
                {
                _fakeT      tab[POOLSIZE];
                _pool *     next;
                };

            size_t      _m_allocatedobj;    // number of object currently allocated.
            size_t      _m_totmem;          // total memory used by the allocator. 

            _pfakeT     _m_firstfree;       // first free elment in the simply chained list of free blocks
            _pool *     _m_currentpool;     // pointer to the current pool
            _pool *     _m_firstpool;       // pointer to the first tpool
            size_t      _m_index;           // index of the first free element in the current pool

            _pfakeT & _getnextfake(_pfakeT f) { return (*((_pfakeT *)f)); } // get the fake T written a the adress of the fake T !

            // should only be called when no object are allocated. 
            inline void _releaseMallocedMemory()
                {                                 
                MTOOLS_ASSERT(_m_allocatedobj == 0);
                while (_m_firstpool != nullptr) { _pool * p = _m_firstpool; _m_firstpool = _m_firstpool->next; free(p); }
                _m_firstfree = nullptr;
                _m_currentpool = nullptr;
                _m_firstpool = nullptr;
                _m_index = POOLSIZE;
                _m_allocatedobj = 0;
                _m_totmem = 0;           
                }

            inline void _dtorDestroy(metaprog::dummy<true> Dum)  { destroyAll(true); }     // here we call the dtors for all object still allocated
            inline void _dtorDestroy(metaprog::dummy<false> Dum) { deallocateAll(true); }  // here, we don't...
        };



        /**
        * Simple STL compliant allocator
        *
        * The allocator can only serve chunks of memory whose lenght is no more than AllocSize bytes
        * (uses mtools::SingleAllocator to manage memory allocation).
        *
        * - Allocator created by the default ctor use their personal memory pool.
        *
        * - Allocators created via copy constructor share the pool of their father. The commun pool
        * is destroyed (without calling the destructor) when the last allocator is destroyed.
        *
        * To use the allocator with an stl container, choose AllocSize large enough so that it can
        * allocate all required node structures. For example:
        *
        * std::set<double, std::less<double>, mtools::FixedSizeAllocator<double, 40> > mySet; // double + 4 pointers for nodes
        *
        * @tparam  T               Type of object to allocate. Must be such that sizeof(T) <= AllocSize.
        * @tparam  AllocSize       Maximum number of bytes of any allocation request.
        * @tparam  PoolSize        Size of the pool i.e. number of chunks of lenght AllocSize per pool.
        *                          (default = such that each pool uses 100MB).
        **/
        template<typename T, size_t AllocSize, size_t PoolSize = NB_FOR_SIZE(AllocSize, MEM_MB(100))> class FixedSizeAllocator
            {

            public:

                typedef size_t      size_type;              // required to comply with stl allocators
                typedef ptrdiff_t   difference_type;        // 
                typedef T*          pointer;                //
                typedef const T*    const_pointer;          //
                typedef T&          reference;              //
                typedef const T&    const_reference;        //
                typedef T           value_type;             //

                template<typename U> struct rebind { typedef FixedSizeAllocator<U, AllocSize, PoolSize> other; }; // obtain the allocator for a new type


                                                                                                                  /** Default constructor. */
                FixedSizeAllocator() throw() : _memPool(new MemPoolType()), _count(new size_t(1))
                    {
                    MTOOLS_DEBUG(std::string("FixedSizeAllocator ctor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize));
                    }


                /** Copy constructor. **/
                FixedSizeAllocator(const FixedSizeAllocator & alloc) throw() : _memPool((MemPoolType*)alloc._memPool), _count(alloc._count)
                    {
                    (*_count)++;
                    MTOOLS_DEBUG(std::string("FixedSizeAllocator copy ctor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize));
                    }


                /** Copy constructor with another type. **/
                template<typename U> FixedSizeAllocator(const FixedSizeAllocator<U, AllocSize, PoolSize> & alloc) throw() : _memPool((MemPoolType*)alloc._memPool), _count(alloc._count)
                    {
                    (*_count)++;
                    MTOOLS_DEBUG(std::string("FixedSizeAllocator copy ctor from T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " to " + typeid(U).name() + "] size " + mtools::toString(sizeof(U)) + " AllocSize = " + mtools::toString(AllocSize));
                    }


                /** Destructor. */
                ~FixedSizeAllocator()
                    {
                    MTOOLS_DEBUG(std::string("FixedSizeAllocator destructor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize));
                    (*_count)--;
                    if ((*_count) == 0)
                        {
                        delete _memPool;
                        MTOOLS_DEBUG(std::string("Last instance of FixedSizeAllocator, deleting also the memory pool"));
                        }
                    }


                /** Copy. **/
                FixedSizeAllocator & operator=(const FixedSizeAllocator & alloc) = delete;


                /** Copy with another type **/
                template<typename U> FixedSizeAllocator& operator=(const FixedSizeAllocator<U, AllocSize, PoolSize> & alloc) = delete;


                /** Get address of reference. **/
                pointer address(reference x) const { return &x; }


                /** Get const address of const reference **/
                const_pointer address(const_reference x) const { return &x; }


                /** Allocate memory **/
                pointer allocate(size_type n, const void* hint = 0)
                    {
                    if ((n*sizeof(value_type)) > AllocSize) { MTOOLS_ERROR(std::string("FixedSizeAllocator<") + typeid(T).name() + ", " + mtools::toString(AllocSize) + ", " + mtools::toString(PoolSize) + ">::allocate. Trying to allocate " + mtools::toString(n) + " objects of size " + mtools::toString(sizeof(value_type))); }
                    return (pointer)(_memPool->allocate());
                    }


                /** Deallocate memory **/
                void deallocate(void* p, size_type n)
                    {
                    if ((n*sizeof(value_type)) > AllocSize) { MTOOLS_ERROR(std::string("FixedSizeAllocator<") + typeid(T).name() + ", " + mtools::toString(AllocSize) + ", " + mtools::toString(PoolSize) + ">::deallocate. Trying to deallocate " + mtools::toString(n) + " objects of size " + mtools::toString(sizeof(value_type))); }
                    _memPool->deallocate((TT*)p);
                    }


                /** deallocate all object in the associated memory pool **/
                void deallocateAll()
                    {
                    _memPool->deallocateAll();
                    }


                /** Call the constructor **/
                void construct(pointer p, const T& val) { ::new((T*)p) T(val); }


                /** Call the constructor with additional arguments **/
                template<typename U, typename... Args> void construct(U* p, Args&&... args) { ::new((U*)p) U(std::forward<Args>(args)...); }


                /** Call the destructor **/
                void destroy(pointer p) { p->~T(); }


                /** Call the destructor of type U **/
                template<typename U> void destroy(U* p) { p->~U(); }


                /** Query the max allocation size **/
                size_type max_size() const { return size_type(-1); } // DO NOT CHANGE : putting some other value here seems to mess with the STL containers...


            private:

                template<typename T2, size_t AllocSize2, size_t PoolSize2> friend class FixedSizeAllocator;

                static_assert((sizeof(T) <= AllocSize), "Type T is larger than the size of a block. Try increasing the AllocSize template parameter");

                typedef typename std::aligned_storage<metaprog::static_lcm<sizeof(int*), AllocSize>::value>::type TT;

                typedef typename SingleAllocator<TT, PoolSize, false> MemPoolType;

                MemPoolType *   _memPool;   // memory Pool
                size_t *        _count;     // object count


            };


        /** Operator== overload : instances with same template parameters AllocSize and PoolSize compare equal if they have the same meory pool **/
        template<typename T1, typename T2, size_t AllocSize, size_t PoolSize> inline bool operator==(const FixedSizeAllocator<T1, AllocSize, PoolSize>& alloc1, const FixedSizeAllocator<T2, AllocSize, PoolSize>& alloc2) { return (alloc1._memPool == alloc2._memPool); }

        /** Operator!= overload : instances with same template parameters AllocSize and PoolSize compare equal if they have the same meory pool **/
        template<typename T1, typename T2, size_t AllocSize, size_t PoolSize> inline bool operator!=(const FixedSizeAllocator<T1, AllocSize, PoolSize>& alloc1, const FixedSizeAllocator<T2, AllocSize, PoolSize>& alloc2) { return (alloc1._memPool != alloc2._memPool); }



}


/* end of file */

