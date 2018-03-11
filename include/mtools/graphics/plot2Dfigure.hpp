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

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Slider.H>


namespace mtools
{


	/* Forward declarations */
	class FigureDrawerWorker;
	template<int N> class FigureDrawerDispatcher;
	template<int N> class Plot2DFigure;



	/**
	* Factory for creating Plots of FigureCanvas objects.
	*
	* @param			canvas  	FigureCanvas object containing the figures to draw..
	* @param 		  	nbthread	number othread to use (number really used may be different). 
	* @param 		  	name		name of the plot
	* @tparam	N	   Same parameter as for the FigureCanvas class (max number of 'reducible' object per node in the TreeFigure container).
	*
	**/
	template<int N> Plot2DFigure<N> makePlot2DFigure(FigureCanvas<N> & canvas, int nbthread = 2, std::string name = "Figure")
		{
		return Plot2DFigure<N>(canvas, nbthread, name);
		}



	/**
	* Plot Object which encapsulate a FigureCanvas object.
	* 
	* use the factory method makePlot2DFigure() to create it. 
	*
	* @tparam	N	   Same parameter as for the FigureCanvas class (max number of 'reducible' object per node in the TreeFigure container).
	* 				   
	**/
	template<int N> class Plot2DFigure : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
	{


	public:


		/**
		* Constructor.
		* 
		* @param			figcanvas  	FigureCanvas object containing the figures to draw..
		* @param 		  	nbthread	number othread to use (number really used may be different).
		* @param 		  	name		name of the plot
		**/
		Plot2DFigure(FigureCanvas<N> & figcanvas, int nbthread = 2, std::string name = "Figure")
			: internals_graphics::Plotter2DObj(name), _figcanvas(&figcanvas), _figDrawers(nullptr), _ims(nullptr), _R(), _hq(true), _min_thick(Image::DEFAULT_MIN_THICKNESS), _tmpIm(), _win(nullptr)
			{
			size_t nbworkerperlayer = (nbthread / figcanvas.nbLayers());
			nbworkerperlayer = (nbworkerperlayer >= 2) ? (nbworkerperlayer - 1) : 1;
			_figDrawers = new FigureDrawerDispatcher<N>[figcanvas.nbLayers()];	// create drawer dispatcher for each level
			_ims = new std::pair<Image,bool>[figcanvas.nbLayers()];				// create images for each level
			for (size_t i = 0; i < figcanvas.nbLayers(); i++)					// setup 
				{
				_ims[i].second = true;
				_figDrawers[i].set(figcanvas.getTreeLayer(i), nbworkerperlayer,&(_ims[i].first));
				}
			}


		/**
		* Move constructor.
		**/
		Plot2DFigure(Plot2DFigure && o) : internals_graphics::Plotter2DObj(std::move(o)), _figcanvas(o._figcanvas), _figDrawers(o._figDrawers), _ims(o._ims), _R(o._R), _hq(o._hq), _min_thick(o._min_thick), _tmpIm(std::move(o._tmpIm)), _win(nullptr)
			{
			o._figcanvas = nullptr;
			o._figDrawer = nullptr;
			o._ims = nullptr;
			}


		/**
		* Move assignement operator.
		**/
		Plot2DFigure & operator=(Plot2DFigure && o)
			{
			if (&o == this) return *this;
			internals_graphics::Plotter2DObj::operator=(std::move(o));
			_figcanvas = o._figcanvas;
			_figDrawers = o._figDrawers;
			_ims = o._ims;
			_R = o._R;
			_hq = O._hq;
			_min_thick = o._min_thick;
			_tmpIm = std::move(o._tmpIm);
			_win = nullptr;
			o._figcanvas = nullptr;
			o._figDrawer = nullptr;
			o._ims = nullptr;
			return *this;
			}


		/**
		* Destructor. Remove the object if it is still inserted.
		**/
		virtual ~Plot2DFigure()
			{
			const size_t n = nbLayers();
			for (size_t i = 0; i < n; i++)
				{
				_figDrawers[i].stopAll();
				_figDrawers[i].enableAllThreads(false);
				}
			detach();
			delete [] _figDrawers;
			delete [] _ims;
			}


		/**
		* Return the number layers in the underlying canvas object.
		**/
		size_t nbLayers() const
			{
			return (_figcanvas == nullptr) ? 0 : _figcanvas->nbLayers();
			}


		/**
		* Query if we are drawing with high quality.
		**/
		bool highQuality() const { return _hq; }


		/**
		* Set if we are using high quality drawing or not. 
		**/
		void highQuality(bool hq)
			{
			if (hq == _hq) return;
			if (!isFltkThread()) // we need to run the method in FLTK
				{
				IndirectMemberProc<Plot2DFigure<N>, bool> proxy(*this, &Plot2DFigure<N>::highQuality, hq); // registers the call
				runInFltkThread(proxy);
				return;
				}
			_hq = hq;
			resetDrawing();
			}


		/**
		* Query the minimum thickness used for drawing.
		**/
		double minThickness() const { return _min_thick; }


		/**
		* Set the minimum thickness used for drawing.
		**/
		void minThickness(double min_thick) 
			{
			MTOOLS_ASSERT((min_thick >= 0) && (min_thick <= 1));
			if (min_thick < 0) min_thick = 0; else if (min_thick > 1) min_thick = 1;
			if (!isFltkThread()) // we need to run the method in FLTK
				{
				IndirectMemberProc<Plot2DFigure<N>, double> proxy(*this, &Plot2DFigure<N>::minThickness, min_thick); // registers the call
				runInFltkThread(proxy);
				return;
				}
			_min_thick = min_thick;
			resetDrawing();
			}


		/**
		* Query if a given layer is shown. 
		**/
		bool showLayer(size_t layerindex) const
			{
			MTOOLS_INSURE(layerindex < nbLayers());
			return _ims[layerindex].second;
			}


		/**
		* Set whether a given layer is shown.
		**/
		void showLayer(size_t layerindex, bool show)
			{
			MTOOLS_INSURE(layerindex < nbLayers());
			if (show == _ims[layerindex].second) return;
			if (!isFltkThread()) // we need to run the method in FLTK
				{
				IndirectMemberProc<Plot2DFigure<N>, size_t, bool> proxy(*this, &Plot2DFigure<N>::showLayer, layerindex, show); // registers the call
				runInFltkThread(proxy);
				return;
				}
			_ims[layerindex].second = show;
			resetDrawing();
			}


		/**   
		* Query the main bounding box of all layers currently displayed.
		* Return an empty box if no object/layer are displayed. 
		**/
		fBox2 boundingBox() const
			{
			fBox2 R;
			for (size_t i = 0; i < nbLayers(); i++)
				{
				if (_ims[i].second) { R.swallowBox(_figcanvas->getTreeLayer(i)->minBoundingBox()); }
				}
			return R;
			}


		/** Yes, thee is a favourite range X (override from base class) */
		virtual bool hasFavouriteRangeX() override { return true; }


		/** Yes, thee is a favourite range Y (override from base class) */
		virtual bool hasFavouriteRangeY() override { return true; }


		/** Favourite range X (override from base class) */
		virtual fBox2 favouriteRangeX(fBox2 R) override
			{
			fBox2 B = boundingBox();
			if (!B.isEmpty()) return zoomOut(B); else return B;
			}


		/** Favourite range Y (override from base class) */
		virtual fBox2 favouriteRangeY(fBox2 R) override
			{
			fBox2 B = boundingBox();
			if (!B.isEmpty()) return zoomOut(B); else return B;
			}



	protected:


		virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override
			{
			const size_t n = nbLayers();
			_R = range;
			for (size_t i = 0; i < n; i++)
				{
				_figDrawers[i].stopAll();
				_ims[i].first.resizeRaw(imageSize);
				_ims[i].first.clear(RGBc::c_Transparent);
				_figDrawers[i].restart(_R, _hq, _min_thick);
				}
			}


		virtual void resetDrawing() override
			{
			const size_t n = nbLayers();
			for (size_t i = 0; i < n; i++)
				{
				_figDrawers[i].stopAll();
				_ims[i].first.clear(RGBc::c_Transparent);
				_figDrawers[i].restart(_R, _hq, _min_thick);
				}
			Plotter2DObj::refresh();
			}


		virtual int drawOnto(Image & im, float opacity = 1.0f) override
			{
			const size_t n = nbLayers();
			if (n == 0) return 100;
			int totq = 0;
			if ((opacity < 1.0f) && (n > 1))
				{ // use temporary image 
				_tmpIm.resizeRaw(im.dimension());
				_tmpIm.clear(RGBc::c_Transparent);
				for (size_t i = 0; i < n; i++)
					{
					totq += _figDrawers[i].quality();
					if (_ims[i].second) _tmpIm.blend(_ims[i].first, { 0,0 });
					}
				im.blend(_tmpIm, { 0,0 }, opacity);
				}
			else
				{ // direct blending without temp image. 
				for (size_t i = 0; i < n; i++)
					{
					totq += _figDrawers[i].quality();
					if (_ims[i].second) im.blend(_ims[i].first, { 0,0 });
					}
				}
			return (totq / (int)n);
			}


		virtual int quality() const override
			{
			const size_t n = nbLayers();
			if (n == 0) return 100;
			int totq = 0;
			for (size_t i = 0; i < n; i++)  totq += _figDrawers[i].quality();
			return (totq / (int)n);
			}


		virtual void enableThreads(bool status) override
			{
			const size_t n = nbLayers();
			for (size_t i = 0; i < n; i++) _figDrawers[i].enableAllThreads(status);
			}


		virtual bool enableThreads() const override
			{
			return ((nbLayers() == 0) ? true : _figDrawers[0].enableAllThreads());
			}


		virtual int nbThreads() const override
			{
			const size_t n = nbLayers();
			int nbt = 0;
			for (size_t i = 0; i < n; i++) nbt += _figDrawers[i].nbThreads();
			return nbt;
			}


		/**
		* Override of the removed method, nothing to do...
		**/
		virtual void removed(Fl_Group * optionWin) override
			{
			enableThreads(false);
			_win = nullptr;
			Fl::delete_widget(optionWin);
			}


		/**
		* Override of the inserted method. There is no option window for a pixel object...
		**/
		virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
			{
			int lgh = 15 + 20 * (int)nbLayers();

			_win = new Fl_Group(0, 0, reqWidth, 90 + lgh); // create the option group
			optionWin = _win;

			_hqButton = new Fl_Check_Button(5, 10, 150, 15, "Use high quality drawing.");
			_hqButton->labelfont(0);
			_hqButton->labelsize(11);
			_hqButton->color2(FL_RED);
			_hqButton->callback(_toggleHQ_static, this);
			_hqButton->when(FL_WHEN_CHANGED);


			Fl_Box * txtminthick = new Fl_Box(10, 31, 90, 15, "minimum thickness :"); // create the info txt;
			txtminthick->labelfont(0);
			txtminthick->labelsize(11);

			_thickSlider = new Fl_Value_Slider(105, 31, 160, 15);
			_thickSlider->align(Fl_Align(FL_ALIGN_TOP));
			_thickSlider->box(FL_FLAT_BOX);
			_thickSlider->type(FL_HOR_NICE_SLIDER);
			_thickSlider->range(0, 1.0);
			_thickSlider->step(0.01);
			_thickSlider->value(_min_thick);
			_thickSlider->color2(FL_RED);
			_thickSlider->callback(_sliderThick_static, this);

			_infoBox = new Fl_Box(5, 55, reqWidth - 5, 15); // create the info txt;
			_infoBox->labelfont(0);
			_infoBox->labelsize(12);
			_infoBox->labelcolor(FL_RED);

			auto border = new Fl_Box(10, 80, reqWidth - 20, lgh); // create the option group;
			border->box(FL_BORDER_BOX);

			const size_t n = nbLayers();
			_layerButtons.resize(n);
			for (size_t i = 0; i < n; i++)
				{
				std::get<0>(_layerButtons[i]) = this;
				std::get<1>(_layerButtons[i]) = i;
				std::get<2>(_layerButtons[i]) = new Fl_Check_Button(15, 90 + 20*((int)i), 150, 15);
				std::get<2>(_layerButtons[i])->labelfont(0);
				std::get<2>(_layerButtons[i])->labelsize(11);
				std::get<2>(_layerButtons[i])->color2(FL_RED);
				std::get<2>(_layerButtons[i])->callback(_toggleLayer_static, &(_layerButtons[i]));
				std::get<2>(_layerButtons[i])->when(FL_WHEN_CHANGED);
				}

			optionWin->end();	
			_updateWidgets();
			return this;
			}


		private:


			void _updateWidgets()
				{
				_hqButton->value((bool)_hq ? 1 : 0);
				_infoBox->copy_label((mtools::toString(nbLayers()) + " layers, " + mtools::toString(_figcanvas->size()) + " objects.").c_str());
				_thickSlider->value(_min_thick);


				const size_t n = nbLayers();
				for (size_t i = 0; i < n; i++)
					{
					std::get<2>(_layerButtons[i])->copy_label((std::string("Layer ") + mtools::toString(i)  + " \t[" + mtools::toString(_figcanvas->size(i)) + " objects]").c_str());
					std::get<2>(_layerButtons[i])->value((bool)_ims[i].second ? 1 : 0);
					}
				}


			static void _toggleHQ_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DFigure<N>*)data)->_toggleHQ(W); }
			void _toggleHQ(Fl_Widget * W)
				{
				highQuality((bool)((Fl_Check_Button*)W)->value());
				yieldFocus();
				}


			static void _sliderThick_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DFigure<N>*)data)->_sliderThick(W); }
			void _sliderThick(Fl_Widget * W)
				{
				minThickness(((Fl_Value_Slider*)W)->value());
				//yieldFocus(); // better to keep focus when using slider
				}
			

			static void _toggleLayer_static(Fl_Widget * W, void * data) 
				{ 
				MTOOLS_ASSERT(data != nullptr);
				std::tuple<Plot2DFigure<N>*, size_t, Fl_Check_Button*> U = *((std::tuple<Plot2DFigure<N> *, size_t, Fl_Check_Button*> *)data);
				(std::get<0>(U))->_toggleLayer(std::get<2>(U), std::get<1>(U)); 
				}

			void _toggleLayer(Fl_Check_Button * W, size_t index)
				{
				showLayer(index,(bool)(W->value()));
				yieldFocus();
				}



			FigureCanvas<N> *			_figcanvas;		// canvas holding all the figures
			FigureDrawerDispatcher<N> * _figDrawers;	// figure drawer dispatcher objecs (1 per layer). 
			std::pair<Image, bool> *	_ims;			// image to draw onto (one per layer) and their drawing status
			fBox2						_R;				// range to draw
			bool						_hq;			// use high quality.
			double						_min_thick;		// minimum thickness to use
			Image						_tmpIm;			// temporary image in case of partial opacity. 

			Fl_Group *					_win;			// option window
			Fl_Box *					_infoBox;		// info box
			Fl_Check_Button *			_hqButton;		// toggle high quality button
			Fl_Value_Slider *           _thickSlider;	// slider for minimum thickness
			std::vector<std::tuple< Plot2DFigure<N>*, size_t, Fl_Check_Button*> > _layerButtons; // enable/disable a particular layer 

		};







	/**
	* "Worker thread" that draws figures inside an Image object.
	*
	* Each thread has its own queue which is polled to query figures to draw.
	* Instances of this class are created and managed by the FigureDrawerDispatcher class.
	*/
	class FigureDrawerWorker : public ThreadWorker
	{

	public:

		/** Constructor. Initially disabled, and not active: nothing is drawn. */
		FigureDrawerWorker() : ThreadWorker(), _queue(QUEUE_SIZE), _nb_drawn(0), _im(nullptr), _R(fBox2()), _hq(true), _min_thick(Image::DEFAULT_MIN_THICKNESS)
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
		void set(Image* im, fBox2 R, bool hq, double min_thick)
			{
			sync();
			_queue.clear();
			_nb_drawn = 0;
			_im = im;
			_R = R;
			_hq = hq;
			_min_thick = min_thick;
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
			const size_t qs = _queue.size();
			return ((qs == 0) ?  45 : ((int)((45 * _nb_drawn) / (_nb_drawn + qs))));
			}


	protected:


		/**
		* Work method that draws the figures i nthe queue into the _im image.
		**/
		virtual void work() override
			{
			double min_thick = _min_thick;
			bool hq = _hq;
			fBox2 R = _R;
			Image * im = _im;
			MTOOLS_INSURE(im != nullptr);
			_nb_drawn = 0;
			while (1)
				{
				FigureInterface * obj;
				while (!_queue.pop(obj)) { check(); std::this_thread::yield(); }
				obj->draw(*im, R, hq, min_thick);
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

		static constexpr size_t QUEUE_SIZE = 16 * 1024 * 1024;	// max queue size. 					
		static constexpr int64 CODE_STOP_AND_WAIT = 0;
		static constexpr int64 CODE_RESTART = 1;

		SingleProducerSingleConsumerQueue<FigureInterface*> _queue;	// the queue containing the figures to draw
		std::atomic<size_t> _nb_drawn;								// number of figure drawn since the work started 
		std::atomic<Image*> _im;									// the image to draw onto
		std::atomic<fBox2>  _R;										// range to use
		std::atomic<bool>	_hq;									// true for high quality drawing
		std::atomic<double> _min_thick;								// minimum thickness used when drawing
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
		void restart(fBox2 R, bool hq, double min_thick)
			{
			stopAll();
			_nb = 0;
			_phase = 0;
			_R = R;
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].set(_images[i], R, hq,min_thick); } // set parameters for worker threads
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



		/** Return the quality of the image currently drawn. 100 = finished drawing. */
		int quality() const
			{
			if (_figTree->size() == 0) return 100;
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
					} 
				while (!_workers[th].pushfigure(bo.object));
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

		static constexpr int64 CODE_STOP_AND_WAIT = 0;
		static constexpr int64 CODE_RESTART = 1;

		TreeFigure<FigureInterface*, N> *   _figTree;	// container for all figure objects.
		FigureDrawerWorker *				_workers;	// vector containing the worker threads.
		std::vector<Image *>				_images;	// vector containing the images to draw onto
		std::atomic<int64>					_nb;		// number of figure processed
		std::atomic<int>					_phase;		// drawing phase
		std::atomic<fBox2>					_R;			// range

	};



	}




/* end of file */



