/** @file threadsafequeue.hpp */
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

#include "../misc.hpp"
#include "../error.hpp"

#include <ctime>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mtools
{


/**
 * Very simple thread-safe FIFO queue with circular buffer for
 * a single producer thread and a single consumer thread.
 **/
template<typename T> class SingleProducerSingleConsumerQueue
	{

	public:

	/** Constructor. */
	SingleProducerSingleConsumerQueue(size_t buffer_size) : _N((int64)(buffer_size+1)), _queue(buffer_size + 1), _readpos(0), _writepos(0)
		{
		MTOOLS_INSURE(_N > 1);
		}


	/**   
	 * pop an element from the queue, return false if none available.  
	 * Should only be called by the (unique) consumer thread. 
	 **/
	MTOOLS_FORCEINLINE bool pop(T & obj)
		{
		const int64 rp = _readpos;
		if (rp == _writepos) return false; 
		obj = _queue[rp];
		std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
		_readpos = (rp + 1) % _N;
		return true;
		}

	/**
	* push an element in the queue, return false if the queue is full.
	* Should only be called by the (unique) producer thread.
	**/
	MTOOLS_FORCEINLINE bool push(const T & obj)
		{
		const int64 wp = _writepos;
		const int64 nwp = (wp + 1) % _N;
		if (nwp == _readpos) return false;
		_queue[wp] = obj;
		std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
		_writepos = nwp;
		return true;
		}


	/** Return the number of elements in the queue */
	MTOOLS_FORCEINLINE size_t size() const
		{
		const int64 l = _writepos - _readpos;
		return (size_t)((l >= 0) ? l : (_N + l));
		}


	/** Clear the queue (this method is not threadsafe) */
	MTOOLS_FORCEINLINE void clear()
		{
		_readpos.store(_writepos);
		}


	private:

		const int64				_N;			// buffer size
		std::vector<T>			_queue;		// buffer
		std::atomic<int64>		_readpos;	// position to read
		std::atomic<int64>      _writepos;	// position to write

	};




}

/* end of file */