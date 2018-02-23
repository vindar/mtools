/** @file plot2Dfigure.hpp */
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

#include "image.hpp"
#include "figure.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2DInterface.hpp"
#include "internal/rangemanager.hpp"
#include "../misc/indirectcall.hpp"
#include "../io/internal/fltkSupervisor.hpp"
#include "../misc/internal/forward_fltk.hpp"
#include "../misc/internal/threadworker.hpp"
#include "../misc/internal/threadsafequeue.hpp"

#include <atomic>



namespace mtools
{




	/**
	* "Worker thread" that draws figures inside an Image object.
	*
	* Each thread has its own queue which is polled to query figures to draw.
	* Instances of this class are created and managed by the FigureDrawerDispatcher class.
	*/
	class FigureDrawerWorker : public ThreadWorker
	{

	public:

		/** Constructor. Initially disabled, and not not active: nothing is drawn. */
		FigureDrawerWorker() : ThreadWorker(), _queue(QUEUE_SIZE), _nb_drawn(0), _im(nullptr), _R(fBox2()), _hq(true)
		{
		}


		/** dtor. */
		virtual ~FigureDrawerWorker()
		{
			requestStop();
			sync();
			_im = nullptr;
		}


		/** Set the parameters. requestStop() must have been called previously ! */
		void set(Image* im, fBox2 R, bool hq)
		{
			sync();
			_queue.clear();
			_nb_drawn = 0;
			_im = im;
			_R = R;
			_hq = hq;
		}


		/** Request stop for any work in progress. Use sync() to wait for completion. */
		MTOOLS_FORCEINLINE void requestStop()
		{
			signal(CODE_STOP_AND_WAIT);
		}


		/* (re)start work. Return without waiting for sync(). */
		MTOOLS_FORCEINLINE void restart()
		{
			signal(CODE_RESTART);
		}


		/** push a new figure in the queue */
		MTOOLS_FORCEINLINE bool pushfigure(FigureInterface* fig)
		{
			return _queue.push(fig);
		}


		/* current progress w.r.t. the queue size, between 0 and 45 (queue empty) */
		MTOOLS_FORCEINLINE int current_prog() const
		{
			if (_nb_drawn == 0) return 0;
			return (int)((45 * _nb_drawn) / (_nb_drawn + _queue.size()));
		}

	protected:


		/**
		* Work method that draws the figures i nthe queue into the _im image.
		**/
		virtual void work() override
		{
			bool hq = _hq;
			fBox2 R = _R;
			Image * im = _im;
			MTOOLS_INSURE(im != nullptr);
			_nb_drawn = 0;
			while (1)
			{
				FigureInterface * obj;
				while (!_queue.pop(obj)) { check(); std::this_thread::yield(); }
				obj->draw(*im, R, hq);
				_nb_drawn++;
				check();
			}
		}


		/**
		* Process incomming messages
		**/
		virtual int message(int64 code)
		{
			switch (code)
			{
			case CODE_RESTART:
			{ // start drawing operations
				return THREAD_RESET;
			}
			case CODE_STOP_AND_WAIT:
			{ // stop all drawing operation and wait until new messages arrive
				return THREAD_RESET_AND_WAIT;
			}
			default:
			{
				MTOOLS_ERROR("should not be possible...");
			}
			}
			return THREAD_RESET_AND_WAIT;
		}


	private:

		static const size_t QUEUE_SIZE = 16 * 1024 * 1024;	// max queue size. 					
		static const int64 CODE_STOP_AND_WAIT = 0;
		static const int64 CODE_RESTART = 1;

		SingleProducerSingleConsumerQueue<FigureInterface*> _queue;	// the queue containing the figures to draw
		std::atomic<size_t> _nb_drawn;								// number of figure drawn since the work started 
		std::atomic<Image*> _im;									// the image to draw onto
		std::atomic<fBox2>  _R;										// range to use
		std::atomic<bool>	_hq;									// true for high quality drawing

	};



	/**
	* Class that manage a TreeFigure object and draws it onto an Image using
	* one or more FigureDrawerWorker instances.
	*/
	template<int N> class FigureDrawerDispatcher : protected ThreadWorker
	{

	public:

		/** Constructor. Set the object in an empty state that does nothing. */
		FigureDrawerDispatcher() : _figTree(nullptr), _workers(nullptr), _images(), _nb(0), _phase(0), _R(fBox2())
		{
		}

		/** Constructor. Set the object in an empty state that does nothing. */
		virtual ~FigureDrawerDispatcher()
		{
			delete[] _workers;
			_workers = nullptr;
		}


		/**
		* Set the main parameters.
		* Set the TreeFigure object to draw, number of threads and corresponding images.
		**/
		void set(TreeFigure<FigureInterface*, N> * figtree, const std::vector<Image*> & images)
		{
			MTOOLS_INSURE(figtree != nullptr);
			MTOOLS_INSURE(images.size() > 0);
			stopAll();						// interrupt any work in progress
			_figTree = figtree;				// save the tree figure object
			delete[] _workers;				// delete previous threads (if any)
			_workers = new FigureDrawerWorker[images.size()]; // create the worker threads
			_images = images;				// save the images. 
			_phase = 0;						// nothing done...
			_nb = 0;						// yet...
		}


		/** Same as above but set all threads to draw on the same image.*/
		void set(TreeFigure<FigureInterface*, N> * figtree, const size_t nb_worker_threads, Image * image)
		{
			MTOOLS_INSURE(figtree != nullptr);
			MTOOLS_INSURE(image != nullptr);
			MTOOLS_INSURE(nb_worker_threads > 0);
			std::vector<Image*> images(nb_worker_threads, image);
			set(figtree, images);
		}


		/** restart the drawing */
		void restart(fBox2 R, bool hq)
		{
			stopAll();
			_nb = 0;
			_phase = 0;
			_R = R;
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].set(_images[i], R, hq); } // set parameters for worker threads
			signal(CODE_RESTART); // start the dispatcher thread
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].restart(); } // start the worker threads. 
		}


		/** Stop everything */
		void stopAll()
		{
			requestStopAll();
			syncAll();
		}


		/** Request stop from all threads (dispatcher and workers).*/
		void requestStopAll()
		{
			signal(CODE_STOP_AND_WAIT); //request stop the dispatcher thread
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].requestStop(); } // stop the worker threads
		}


		/** Wait for synchronization of all threads (dispatcher and workers).*/
		void syncAll()
		{
			sync();
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].sync(); } // stop the worker threads
		}


		/** Query if the thread are currently enabled. */
		bool enableAllThreads() const
		{
			return enable();
		}


		/** Enable/disable all the threads (wait for completion) */
		void enableAllThreads(bool status)
		{
			if (enable() == status) return;
			enable(status);
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].enable(status); }
			sync();
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].sync(); }
		}


		/** return the total number of thread (1 + number of worker thread) */
		int nbThreads() const
		{
			return (int)(1 + _images.size());
		}



		/** Return the quality of the image currently drawn. 100 = finshed drawing. */
		int quality() const
		{
			if (_phase == 0)
			{
				int64 u = _nb;
				if (u == 0) return 0;
				u *= u;
				return (1 + mtools::highestBit((uint64)u));
			}
			else
			{
				const size_t Nth = _images.size();
				int tot = 0;
				for (size_t i = 0; i < Nth; i++) { tot += _workers[i].current_prog(); }
				tot /= ((int)Nth);
				return 55 + tot;
			}
		}


	protected:


		/**
		* Work method. draws the figures.
		**/
		virtual void work() override
		{
			_phase = 0; // iterating
			const int64 Nth = _images.size();	// number of worker threads.
			int64 th = 0;						// index of the thread to use. 
												// iterate over the figures to draw
			fBox2 oR = zoomOut((fBox2)_R);
			_figTree->iterate_intersect(oR,
				[&](mtools::TreeFigure<FigureInterface *, N, double>::BoundedObject & bo) -> void
			{
				do
				{
					check(); // check if we should interrupt 
					th++;
					if (th >= Nth) th = 0;
				} while (!_workers[th].pushfigure(bo.object));
				_nb++;
			});
			_phase = 1; // finished iterating.
		}


		/**
		* Process incomming messages
		**/
		virtual int message(int64 code)
		{
			switch (code)
			{
			case CODE_RESTART:
			{ // start drawing operations
				return THREAD_RESET;
			}
			case CODE_STOP_AND_WAIT:
			{ // stop all drawing operation and wait until new messages arrive
				return THREAD_RESET_AND_WAIT;
			}
			default:
			{
				MTOOLS_ERROR("should not be possible...");
			}
			}
			return THREAD_RESET_AND_WAIT;
		}


	private:

		static const int64 CODE_STOP_AND_WAIT = 0;
		static const int64 CODE_RESTART = 1;

		TreeFigure<FigureInterface*, N> *   _figTree;	// container for all figure objects.
		FigureDrawerWorker *  _workers;					// vector containing the worker threads.
		std::vector<Image *> _images;					// vector containing the images to draw onto
		std::atomic<int64>	_nb;						// number of figure processed
		std::atomic<int>	_phase;						// drawing phase
		std::atomic<fBox2>	_R;							// range

	};







	/**
	* Plot Object which encapsulate a TreeFigure object.
	**/
	template<int N > class Plot2DFigure : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
	{

	public:

		/**
		* Constructor.
		**/
		Plot2DFigure(TreeFigure<FigureInterface*, N> * figtree, int nbthread = 2, std::string name = "Figure") : internals_graphics::Plotter2DObj(name), _figDrawer(nullptr), _im(), _R(), _hq(true)
		{
			_figDrawer = new FigureDrawerDispatcher<N>;
			_figDrawer->set(figtree, nbthread - 1, &_im);
		}


		/**
		* Constructor. Reference verison
		**/
		Plot2DFigure(TreeFigure<FigureInterface*, N> & figtree, int nbthread = 2, std::string name = "Figure") : internals_graphics::Plotter2DObj(name), _figDrawer(nullptr), _im(), _R(), _hq(true)
		{
			_figDrawer = new FigureDrawerDispatcher<N>;
			_figDrawer->set(&figtree, nbthread - 1, &_im);
		}


		/**
		* Move constructor.
		**/
		Plot2DFigure(Plot2DFigure && o) : internals_graphics::Plotter2DObj(std::move(o)), _figDrawer(o._figDrawer), _im(std::move(o._im)), _R(o._R), _hq(O._hq)
		{
			o._figDrawer = nullptr;
		}


		/**
		* Destructor. Remove the object if it is still inserted.
		**/
		virtual ~Plot2DFigure()
		{
			_figDrawer->stopAll();
			_figDrawer->enableAllThreads(false);
			detach();
			delete _figDrawer;
		}


	protected:


		virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override
		{
			_figDrawer->stopAll();
			_im.resizeRaw(imageSize);
			_im.clear(RGBc::c_Transparent);
			_R = range;
			_figDrawer->restart(_R, _hq);
		}


		virtual void resetDrawing() override
		{
			_figDrawer->stopAll();
			_im.clear(RGBc::c_Transparent);
			_figDrawer->restart(_R, _hq);
		}


		virtual int drawOnto(Image & im, float opacity = 1.0) override
		{
			auto q = _figDrawer->quality();
			im.blend(_im, { 0,0 }, opacity);
			return q;
		}


		virtual int quality() const override
		{
			return _figDrawer->quality();
		}


		virtual void enableThreads(bool status) override
		{
			_figDrawer->enableAllThreads(status);
		}


		virtual bool enableThreads() const override
		{
			return _figDrawer->enableAllThreads();
		}


		virtual int nbThreads() const override
		{
			return _figDrawer->nbThreads();
		}


		/**
		* Override of the removed method, nothing to do...
		**/
		virtual void removed(Fl_Group * optionWin) override
		{
			_figDrawer->enableAllThreads(false);
		}


		/**
		* Override of the inserted method. There is no option window for a pixel object...
		**/
		virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
		{
			optionWin = nullptr;
			return this;
		}

	private:


		FigureDrawerDispatcher<N> * _figDrawer;  // figure drawer dispatcher object. 
		Image						_im;		 // image to draw onto
		fBox2						_R;			 // range to draw
		bool						_hq;		 // use high quality.

	};






}


/* end of file */



