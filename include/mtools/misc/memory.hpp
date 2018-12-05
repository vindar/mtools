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

#include "../misc/internal/mtools_export.hpp"
#include "error.hpp"
#include "metaprog.hpp"

#include <cstddef>
#include <cstdlib>
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
	* A simple (but fast) memory pool.
	*
	* Each allocation request returns a continuous memory portion of UNITALLOCSIZE bytes.
	*
	* There is no 'wasted' memory provided that UNITALLOCSIZE > sizeof(T*).
	*
	* @tparam  UNITALLOCSIZE   Size in byte of each unit chunk of memory to allocate.
	* @tparam  POOLSIZE        Number of chunks in each pool. When a pool is full, a new one is
	*                          created (default: each pool uses 50MB).
	**/
	template<size_t UNITALLOCSIZE, size_t POOLSIZE = MEM_MB(50) / UNITALLOCSIZE + 1> class CstSizeMemoryPool
	{

	public:

		/** Default constructor. */
		CstSizeMemoryPool() : _m_allocatedobj(0), _m_totmem(0), _m_firstfree(nullptr), _m_currentpool(nullptr), _m_firstpool(nullptr), _m_index(POOLSIZE) { }


		/** Move constructor **/
		CstSizeMemoryPool(CstSizeMemoryPool && csmp) : _m_allocatedobj(csmp._m_allocatedobj), _m_totmem(csmp._m_totmem), _m_firstfree(csmp._m_firstfree), _m_currentpool(csmp._m_currentpool), _m_firstpool(csmp._m_firstpool), _m_index(csmp._m_index)
			{
			csmp._m_allocatedobj = 0;
			csmp._m_totmem = 0;
			csmp._m_firstfree = nullptr;
			csmp._m_currentpool = nullptr;
			csmp._m_firstpool = nullptr;
			csmp._m_index = POOLSIZE;
			}


		/** Destructor. Release all memory to OS without calling dtors.  **/
		~CstSizeMemoryPool() 
			{
			freeAll(true);
			}


		/** Move assignement operator. Discard current content without calling dtors. */
		CstSizeMemoryPool & operator=(CstSizeMemoryPool && csmp) 			
			{
			if (&csmp == this) return (*this);
			freeAll(true);	// release memory without calling dtors
			_m_allocatedobj = csmp._m_allocatedobj;
			_m_totmem = csmp._m_totmem;
			_m_firstfree = csmp._m_firstfree;
			_m_currentpool = csmp._m_currentpool;
			_m_firstpool = csmp._m_firstpool;
			_m_index = csmp._m_index;
			csmp._m_allocatedobj = 0;
			csmp._m_totmem = 0;
			csmp._m_firstfree = nullptr;
			csmp._m_currentpool = nullptr;
			csmp._m_firstpool = nullptr;
			csmp._m_index = POOLSIZE;
			return (*this);
			}


		/**
		* Allocate UNITALLOCSIZE contiguous bytes.
		*
		* @return  a pointer to the allocated memory
		**/
		inline void * malloc()
			{
			++_m_allocatedobj;
			if (_m_firstfree != nullptr)
				{
				_pfakeT p = _m_firstfree;
				_m_firstfree = _getnextfake(_m_firstfree);
				return p;
				}
			if (_m_index == POOLSIZE) { _nextPool(); }
			auto r = _m_currentpool->tab + _m_index;
			_m_index++;
			return r;
			}


		/**
		* Allocate memory for an element of type T and construct it with the copy ctor.
		*
		* @param	val copy this element
		*
		* @return	a pointer to the newly contructed element in the memory pool.
		*/
		template<typename T> T * allocate(const T& val)
			{
			MTOOLS_ASSERT(sizeof(T) <= UNITALLOCSIZE);
			T * p = (T*)malloc();
			::new(p) T(val);
			return p; 
			}


		/**
		* Allocate memory for an element of type T and construct it with given paramters.
		* 
		* @param	args The constructor paramters.
		*
		* @return	a pointer to the newly contructed element in the memory pool.
		*/
		template<typename T, typename... Args> T * allocate(Args&&... args)
			{
			MTOOLS_ASSERT(sizeof(T) <= UNITALLOCSIZE);
			T * p = (T*)malloc();
			::new(p) T(std::forward<Args>(args)...);
			return p;
			}


		/**
		* Free a previously malloced memory (of UNITALLOCSIZE bytes).
		*
		* @param [in,out]  p   pointer to the memory portion to free.
		**/
		inline void free(void * p)
			{
			MTOOLS_ASSERT(_m_firstpool != nullptr);
			MTOOLS_ASSERT(_m_allocatedobj > 0);
			--_m_allocatedobj;
			_getnextfake((_pfakeT)p) = _m_firstfree;
			_m_firstfree = (_pfakeT)p;
			}


		/**
		* Call the destructor ~T on *p and then free the associatd memory.
		*
		* @param [in,out]	p Pointer to the object in the memeory pool to destroy and free memory.
		*/
		template<typename T> void destroyAndFree(T * p)
			{
			p->~T();
			free(p);
			}


		/**
		* Free all allocated memory.
		*
		* @param   releaseMemoryToOS   true to release malloced memory to the operating system (default
		*                              false).
		**/
		inline void freeAll(bool releaseMemoryToOS = false)
			{
			if (_m_firstpool == nullptr) return;
			_m_firstfree = nullptr;
			_m_currentpool = _m_firstpool;
			_m_allocatedobj = 0;
			_m_index = 0;
			if (releaseMemoryToOS)
				{
				while (_m_firstpool != nullptr) { _pool * p = _m_firstpool; _m_firstpool = _m_firstpool->next; std::free(p); }
				_m_firstpool = nullptr;
				_m_index = POOLSIZE;
				_m_totmem = 0;
				}
			}	


		/**
		* Free all allocated memory but call destructor ~T() on all allocated memory before releasing
		* it.
		*
		* @tparam  T   Type of object stored in memory (i.e. type of dtor called).
		* @param   releaseMemoryToOS   true to release malloced memory to the operating system (default
		*                              false).
		*                              
		* @return the number of destructor called (= number of object that were allocated). 
		**/
		template<typename T> size_t destroyAndFreeAll(bool releaseMemoryToOS = false)
			{
			if (_m_firstpool == nullptr) return 0;
			// we call the dtor for all the sites with lower bit set since they cannot be free sites (memory adresses are aligned mod 2)
			_pool * p = _m_firstpool;
			size_t destroyed = 0;
			while (p != _m_currentpool)
			{
				for (size_t i = 0; i < POOLSIZE; i++)
					{
					_pfakeT f = (p->tab + i); 
					if ((((size_t)_getnextfake(f)) & 1) != 0) { ((T*)f)->~T();  _getnextfakevalue(f) = 1;  destroyed++; }
					}
				p = p->next;
			}
			for (size_t i = 0; i < _m_index; i++)
				{
				_pfakeT f = (p->tab + i); 
				if ((((size_t)_getnextfake(f)) & 1) != 0) { ((T*)f)->~T(); _getnextfakevalue(f) = 1; destroyed++; }
				}
			// iterate over the list of free site and put every lowest bit to 1
			while (_m_firstfree != nullptr)
				{
				_pfakeT  f = _getnextfake(_m_firstfree); _getnextfakevalue(_m_firstfree) = 1;  _m_firstfree = f;
				}
			// we call the dtor for all the sites with lower bit = 0, these are exactly the site whose dtor has not yet been called.
			p = _m_firstpool;
			while (p != _m_currentpool)
				{
				for (size_t i = 0; i < POOLSIZE; i++)
					{
					_pfakeT f = (p->tab + i); 
					if ((_getnextfakevalue(f) & 1) == 0) { ((T*)f)->~T(); destroyed++; }
					}
				p = p->next;
				}
			for (size_t i = 0; i < _m_index; i++)
				{
				_pfakeT f = (p->tab + i); 
				if ((_getnextfakevalue(f) & 1) == 0) { ((T*)f)->~T(); destroyed++; }
				}
			MTOOLS_ASSERT(destroyed == _m_allocatedobj); // normally, all destructors have been called
			// finally, we release the memory to the allocator (and, optionnaly to the OS). 			 
			freeAll(releaseMemoryToOS);
			return destroyed; 
			}


		/**
		 * Iterate over all currently allocated objects.
		 * 
		 * !!! THE LOWER BIT OF EACH CHUNK/OBJECT MUST NOT BE MODIFIED INSIDE fun() !!!
		 *
		 * @tparam	FUNCTION Type of the function.
		 * @param	fun function to call on each object currently allocated, must accept call of the form 'fun(void * p)'. 
		 *
		 * @return	the number of call to fun() = number of object currently allocated = size().
		 */
		template<typename FUNCTION> size_t iterateOver(FUNCTION fun)
			{
			if (_m_firstpool == nullptr) return 0;
			// we call fun with all object whose lowest bit is set since they cannot represent free site (memory adresses are aligned mod 2)
			_pool * p = _m_firstpool;
			size_t called = 0;
			while (p != _m_currentpool)
				{
				for (size_t i = 0; i < POOLSIZE; i++)
					{
					_pfakeT f = (p->tab + i);
					if ((_getnextfakevalue(f) & 1) != 0) { fun((void *)f); MTOOLS_INSURE((_getnextfakevalue(f) & 1) != 0);  called++; } // if this insure fails, this means that fun() changed the lowest bit of the object: FORBIDDEN !
					}
				p = p->next;
				}
			for (size_t i = 0; i < _m_index; i++)
				{
				_pfakeT f = (p->tab + i);
				if ((_getnextfakevalue(f) & 1) != 0) { fun((void *)f); MTOOLS_INSURE((_getnextfakevalue(f) & 1) != 0);  called++; } // if this insure fails, this means that fun() changed the lowest bit of the object: FORBIDDEN !
				}
			// iterate over the list of free site and set the lowest bit to 1.			
			_pfakeT q = _m_firstfree;
			while (q != nullptr)
				{
				_pfakeT  f = _getnextfake(q);  MTOOLS_ASSERT((_getnextfakevalue(q) & 1) == 0);  (_getnextfakevalue(q))++; q = f;
				}
			// we call the dtor for all the sites with lower bit = 0, these are exactly the site whose dtor has not yet been called.
			p = _m_firstpool;
			while (p != _m_currentpool)
				{
				for (size_t i = 0; i < POOLSIZE; i++)
					{
					_pfakeT f = (p->tab + i);
					if ((_getnextfakevalue(f) & 1) == 0) { fun((void *)f); MTOOLS_INSURE((_getnextfakevalue(f) & 1) == 0);  called++; } // if this insure fails, this means that fun() changed the lowest bit of the object: FORBIDDEN !
					}
				p = p->next;
				}
			for (size_t i = 0; i < _m_index; i++)
				{
				_pfakeT f = (p->tab + i);
				if ((_getnextfakevalue(f) & 1) == 0) { fun((void *)f); MTOOLS_INSURE((_getnextfakevalue(f) & 1) == 0);  called++; } // if this insure fails, this means that fun() changed the lowest bit of the object: FORBIDDEN !
				}
			// iterate over the list of free site and restore the lowest bit to 0. 
			q = _m_firstfree;
			while (q != nullptr)
				{
				MTOOLS_ASSERT((_getnextfakevalue(q) & 1) != 0);  (_getnextfakevalue(q))--; q = _getnextfake(q);
				}
			MTOOLS_ASSERT(called == _m_allocatedobj); // normally, all destructor have been called
			return called;
			}


		/**
		* Return the number of objects/chunks allocated in the memory pool.
		*
		* @return  the number of object currently allocated. 
		**/
		inline size_t size() const { return _m_allocatedobj; }


		/**
		* Return the memory size currently allocated.
		*
		* @return  the number of bytes currently allocated.
		**/
		inline size_t used() const { return(UNITALLOCSIZE*_m_allocatedobj); }


		/**
		* Return the total memory size malloced by the allocator object. This quantity never decrease
		* unless freeAll() or destoyAndFreeAll() is called with flag releaseMemoryToOS set to true.
		*
		* @return  The number of bytes consumed by the allocator.
		**/
		inline size_t footprint() const { return (_m_totmem); }


		/**
		* Print some information about the state of the memory pool.
		*
		* @return  A std::string that with info about the current object state.
		**/
		std::string toString() const
			{
			std::string s = std::string("CstSizeMemoryPool<") + mtools::toString(UNITALLOCSIZE) + ", " + mtools::toString(POOLSIZE) + ">\n";
			s += std::string(" - number of chunks : ") + mtools::toString(_m_allocatedobj) + " (in " + mtools::toString(footprint() / sizeof(_pool)) + " pools)\n";
			s += std::string(" - memory allocated : ") + toStringMemSize(used()) + "\n";
			s += std::string(" - memory footprint : ") + toStringMemSize(footprint()) + "\n";
			return s;
			}


		/**
		* Query if a pointer belong to the memory pool.
		*
		* Does not check whether the adress is allocated or not, just whether it resides inside the memory pool.
		* Does not check if the adress is correctly aligned.
		*
		* @param [in,out]	p	the pointer to check
		*
		* @return	true if it points to some memory inside the pool and false otherwise.
		**/
		bool isInPool(void * p) const
			{
			const uintptr_t uip = (uintptr_t)p;
			_pool * cpool = _m_firstpool;
			while (cpool != nullptr)
				{
				const uintptr_t uimin = (uintptr_t)(cpool->tab);
				const uintptr_t uimax = (uintptr_t)(cpool->tab + POOLSIZE);
				if ((uip >= uimin) && (uip < uimax)) return true;
				cpool = cpool->next;
				}
			return false;
			}

	private:


		/** get/create the next memory pool */
		void _nextPool()
			{
			if (_m_currentpool == nullptr)
				{
				_m_currentpool = (_pool*)std::malloc(sizeof(_pool));
				if (_m_currentpool == nullptr) { MTOOLS_DEBUG("SingleObjectAllocator, bad_alloc"); throw std::bad_alloc(); }
				MTOOLS_ASSERT(((size_t)_m_currentpool) % 2 == 0); // alignement is at least mod 2
				_m_totmem += sizeof(_pool);
				_m_firstpool = _m_currentpool;
				_m_currentpool->next = nullptr;
				}
			else
				{
				if (_m_currentpool->next == nullptr)
					{
					_m_currentpool->next = (_pool*)std::malloc(sizeof(_pool));
					if (_m_currentpool == nullptr) { MTOOLS_DEBUG("SingleObjectAllocator, bad_alloc"); throw std::bad_alloc(); }
					MTOOLS_ASSERT(((size_t)_m_currentpool->next) % 2 == 0); // alignement is at least mod 2
					_m_totmem += sizeof(_pool);
					_m_currentpool->next->next = nullptr;
					}
				_m_currentpool = _m_currentpool->next;
				}
			_m_index = 0;
			}


		CstSizeMemoryPool(const CstSizeMemoryPool &) = delete;                  // no copy
		CstSizeMemoryPool & operator=(const CstSizeMemoryPool &) = delete;      //


		typedef typename std::aligned_storage<((UNITALLOCSIZE > sizeof(int*)) ? UNITALLOCSIZE : sizeof(int*))>::type _fakeT; // placeholder 
		typedef _fakeT * _pfakeT; // pointer of placeholder

		struct _pool // memory pool
			{
			_fakeT      tab[POOLSIZE];
			_pool *     next;
			};

		size_t      _m_allocatedobj;    // number of object currently allocated.
		size_t      _m_totmem;          // total memory used by the allocator. 

		_pfakeT     _m_firstfree;       // first free element in the simply chained list of free blocks
		_pool *     _m_currentpool;     // pointer to the current pool
		_pool *     _m_firstpool;       // pointer to the first tpool
		size_t      _m_index;           // index of the first free element in the current pool

		_pfakeT & _getnextfake(_pfakeT f) { return (*((_pfakeT *)f)); } // get the fake T written a the adress of the fake T !

		size_t & _getnextfakevalue(_pfakeT f) { return(*((size_t *)f)); } // get the fake T written a the adress of the fake T casted as a size_t !


		
	};










	/**
	* Simple STL compliant allocator
	*
	* The allocator can only serve chunks of memory whose lenght is no more than AllocSize bytes
	* (rely on CstSizeMemoryPool for memory allocation).
	*
	* Allocator have a state. This means that two distinct instances have separate memory pool. Yet,
	* Allocators constructed via copy constructors share the same memory pool hence compare equal.
	*
	* To use the allocator with an stl container such as list/set/map..., choose AllocSize large enough
	* so that it can allocate all the required node structs.
	*
	* For example:
	*
	* std::set<double, std::less<double>, mtools::SingleObjectAllocator<double, 40> > mySet; // double + 4 pointers for nodes.
	*
	* @tparam  T           Type of object to allocate. Must be such that sizeof(T) <= AllocSize.
	* @tparam  AllocSize   Maximum number of bytes of any allocation.
	* @tparam  PoolSize    Size of each memory pool i.e. number of chunks of lenght AllocSize per
	*                      pool. (default = such that each pool uses 50MB).
	**/
	template<typename T, size_t AllocSize = sizeof(T), size_t PoolSize = ((MEM_MB(50)) / AllocSize) + 1> class SingleObjectAllocator
	{

		static_assert((sizeof(T) <= AllocSize), "Type T is larger than the size of a block. Try increasing the AllocSize template parameter");

	public:

		typedef size_t      size_type;              // required to comply with stl allocators
		typedef ptrdiff_t   difference_type;        // 
		typedef T*          pointer;                //
		typedef const T*    const_pointer;          //
		typedef T&          reference;              //
		typedef const T&    const_reference;        //
		typedef T           value_type;             //

		template<typename U> struct rebind { typedef SingleObjectAllocator<U, AllocSize, PoolSize> other; }; // rebind: obtain the allocator for a new type


																											 /**
																											 * Default constructor.
																											 **/
		SingleObjectAllocator() throw() : _memPool(new MemPoolType()), _count(new size_t(1))
		{
			MTOOLS_DEBUG(std::string("SingleObjectAllocator ctor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize) + " poolSize = " + mtools::toString(PoolSize));
		}


		/**
		* Copy constructor.
		**/
		SingleObjectAllocator(const SingleObjectAllocator & alloc) throw() : _memPool((MemPoolType*)alloc._memPool), _count(alloc._count)
		{
			(*_count)++;
			MTOOLS_DEBUG(std::string("SingleObjectAllocator copy ctor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize) + " poolSize = " + mtools::toString(PoolSize));
		}


		/**
		*  Copy constructor with another type.
		**/
		template<typename U> SingleObjectAllocator(const SingleObjectAllocator<U, AllocSize, PoolSize> & alloc) throw() : _memPool((MemPoolType*)alloc._memPool), _count(alloc._count)
		{
			static_assert(sizeof(U) < AllocSize, "Copy constructor to a type U which is larger than AllocSize. Try increasing the AllocSize template parameter");
			(*_count)++;
			MTOOLS_DEBUG(std::string("SingleObjectAllocator copy ctor from T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " to " + typeid(U).name() + "] size " + mtools::toString(sizeof(U)) + " AllocSize = " + mtools::toString(AllocSize) + " poolSize = " + mtools::toString(PoolSize));
		}


		/**
		* Move constructor.
		**/
		SingleObjectAllocator(SingleObjectAllocator && alloc) throw() : _memPool((MemPoolType*)alloc._memPool), _count(alloc._count)
		{
			alloc._count = nullptr;
			alloc._memPool = nullptr;
			MTOOLS_DEBUG(std::string("SingleObjectAllocator move ctor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize) + " poolSize = " + mtools::toString(PoolSize));
		}


		/**
		* Destructor.
		**/
		~SingleObjectAllocator()
		{
			MTOOLS_DEBUG(std::string("SingleObjectAllocator destructor with T=[") + typeid(T).name() + "] size " + mtools::toString(sizeof(T)) + " AllocSize = " + mtools::toString(AllocSize) + " poolSize = " + mtools::toString(PoolSize));
			if (_count == nullptr) return; // empty object, do nothing
			(*_count)--;
			if ((*_count) == 0)
			{
				delete _memPool;
				delete _count;
				MTOOLS_DEBUG(std::string("Last instance of SingleObjectAllocator, deleting also the memory pool"));
			}
		}


		/**
		*  Get address of reference.
		**/
		pointer address(reference x) const { return &x; }


		/**
		*  Get const address of const reference.
		**/
		const_pointer address(const_reference x) const { return &x; }


		/**
		* Allocates memory for one object (do not call the constructor).
		*
		* @param   n       Must be 1.
		* @param   hint    Unused.
		*
		* @return  A pointer to the allocated memory.
		**/
		pointer allocate(size_type n = 1, const void* hint = 0)
		{
			if (n != 1) { MTOOLS_ERROR(std::string("SingleObjectAllocator<") + typeid(T).name() + ", " + mtools::toString(AllocSize) + ", " + mtools::toString(PoolSize) + ">::allocate. Trying to allocate " + mtools::toString(n) + " objects simultaneously (must be 1)."); }
			return (pointer)(_memPool->malloc());
		}


		/**
		* Deallocate the memory of the object (do not call the destructor).
		*
		* @param [in,out]  p   Pointer to the object to deallocate.
		* @param   n           Must be 1 (single block allocation).
		**/
		void deallocate(void* p, size_type n = 1)
		{
			if (n != 1) { MTOOLS_ERROR(std::string("SingleObjectAllocator<") + typeid(T).name() + ", " + mtools::toString(AllocSize) + ", " + mtools::toString(PoolSize) + ">::deallocate. Trying to deallocate " + mtools::toString(n) + " objects simultaneously (should be 1)"); }
			if (_count == nullptr) return; // empty object, do nothing
			_memPool->free(p);
		}


		/**
		* Deallocate all objects in the associated memory pool.
		*
		* @param   releaseMemoryToOS   true to release the malloced memory to the operating system.
		**/
		void deallocateAll(bool releaseMemoryToOS = false)
		{
			if (_count == nullptr) return; // empty object, do nothing
			_memPool->freeAll(releaseMemoryToOS);
		}


		/**
		*  Call the constructor.
		**/
		void construct(pointer p, const T& val) { ::new((T*)p) T(val); }


		/**
		*  Call the constructor with additional arguments.
		**/
		template<typename U, typename... Args> void construct(U* p, Args&&... args)
		{
			static_assert(sizeof(U) < AllocSize, "Trying to construct an object larger than AllocSize !");
			::new((U*)p) U(std::forward<Args>(args)...);
		}


		/**
		*  Call the destructor (do not release memeory).
		**/
		void destroy(pointer p) { p->~T(); }


		/**
		*  Call the destructor of type U * (do not release memory).
		**/
		template<typename U> void destroy(U* p)
		{
			static_assert(sizeof(U) < AllocSize, "Trying to destroy an object larger then AllocSize !");
			p->~U();
		}


		/**
		* Call the destructors and then deallocate all objects in the memory pool.
		*
		* @param   releaseMemoryToOS   true to release the malloced memory to the operating system.
		**/
		void destroyAndDeallocateAll(bool releaseMemoryToOS = false)
		{
			if (_count == nullptr) return; // empty object, do nothing
			_memPool->template destroyAndFreeAll<T>(releaseMemoryToOS);
		}


		/**
		* Call the destructors of given type U and then deallocate all objects in the memory pool.
		*
		* @tparam  U   Type of destructor to call.
		* @param   releaseMemoryToOS   true to release the malloced memory to the operating system.
		**/
		template<typename U> void destroyAndDeallocateAll(bool releaseMemoryToOS = false)
		{
			static_assert(sizeof(U) < AllocSize, "Trying to destroy objects larger then AllocSize !");
			if (_count == nullptr) return; // empty object, do nothing
			_memPool->template destroyAndFreeAll<U>(releaseMemoryToOS);
		}


		/**
		*  Query the max allocation size *.
		**/
		size_type max_size() const { return size_type(-1); } // DO NOT CHANGE : putting some other value here seems to mess with the STL containers...


		/**
		 * Return the memory size currently allocated.
		 *
		 * @return  the number of bytes currently allocated.
		 **/
		inline size_t used() const { return (_memPool->used()); }


		/**
		* Return the total memory size malloced by the memory pool. This quantity never decrease
		* since memory is not release until destruction of the allocator.
		*
		* @return  The number of bytes consumed by the memory pool.
		**/
		inline size_t footprint() const { return (_memPool->footprint()); }


		/**
		* Print some information about the state of the memory pool.
		*
		* @return  A std::string that with info about the current object state.
		**/
		std::string toString() const
		{
			std::string s = std::string("SingleObjectAllocator<") + typeid(T).name() + ", " + mtools::toString(AllocSize) + ", " + mtools::toString(PoolSize) + ">\n";
			s += std::string(" - object count : ") + mtools::toString(_count) + "\n";
			s += std::string(" - memory pool adress : ") + mtools::toString(_memPool) + "\n --- Memory pool info ---\n";
			s += _memPool->toString() + "---\n";
			return s;
		}



		/**
		* Query if a pointer belong to the memory pool of the allocator.
		*
		* Does not check whether the adress is allocated or not, just whether it resides inside the memory pool.
		* Does not check if the adress is correctly aligned.
		*
		* @param [in,out]	p	the pointer to check
		*
		* @return	true if it points to some memory inside the pool and false otherwise.
		**/
		bool isInPool(void * p) const { return _memPool->isInPool(p); }



	private:

		SingleObjectAllocator & operator=(const SingleObjectAllocator & alloc) = delete; // operator= forbidden

		template<typename T2, size_t AllocSize2, size_t PoolSize2> friend class SingleObjectAllocator; // friend with its template variant

		typedef CstSizeMemoryPool<AllocSize, PoolSize> MemPoolType;

		MemPoolType *   _memPool;   // memory Pool
		size_t *        _count;     // object count
	};


	/** Operator== overload : instances with same template parameters AllocSize and PoolSize compare equal if they have the same meory pool **/
	template<typename T1, typename T2, size_t AllocSize, size_t PoolSize> inline bool operator==(const SingleObjectAllocator<T1, AllocSize, PoolSize>& alloc1, const SingleObjectAllocator<T2, AllocSize, PoolSize>& alloc2) { return (alloc1._memPool == alloc2._memPool); }

	/** Operator!= overload : instances with same template parameters AllocSize and PoolSize compare equal if they have the same meory pool **/
	template<typename T1, typename T2, size_t AllocSize, size_t PoolSize> inline bool operator!=(const SingleObjectAllocator<T1, AllocSize, PoolSize>& alloc1, const SingleObjectAllocator<T2, AllocSize, PoolSize>& alloc2) { return (alloc1._memPool != alloc2._memPool); }



}


/* end of file */

