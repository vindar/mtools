
#include "mtools/mtools.hpp"
using namespace mtools;

#include "mtools/misc/internal/threadsafequeue.hpp"



/** Interface class for figure objects. */
class FigureInterface
{

public:


	/** Virtual destructor */
	virtual ~FigureInterface() {}

	/**
	* Draws the figure onto an image with a given range.
	*
	* @param [in,out]	im		    the image to draw onto.
	* @param [in,out]	R		    the range.
	* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
	* 								otherwise.
	*/
	virtual void draw(Image & im, fBox2 & R, bool highQuality = true) = 0;


	/**
	* Return the object's bounding box.
	*
	* @return	A fBox2.
	*/
	virtual fBox2 boundingBox() const = 0;


	/**
	* Print info about the object into an std::string.
	*/
	virtual std::string toString(bool debug = false) const = 0;

	/**
	* Serialize the object.
	*/
	virtual void serialize(OBaseArchive & ar) const = 0;

	/**
	* Deserialize this object.
	*/
	virtual void deserialize(IBaseArchive & ar) = 0;

};




class FigureCircle : public FigureInterface
{

public: 

	/** circle parameters **/

	fVec2	center;			// circle center
	double	radius;			// circle radius
	double	thickness;		// circle thickness
	RGBc	color;			// circle color
	RGBc	fillcolor;		// circle interior color


	/** Constructor. */
	FigureCircle(fVec2 centercircle, double rad, RGBc col) : center(centercircle), radius(rad), color(col)
		{
		//thickness = 0.1;
		fillcolor = RGBc::c_Blue.getMultOpacity(0.5f);
		}



	/**
	* Draws the figure onto an image with a given range.
	*
	* @param [in,out]	im		    the image to draw onto.
	* @param [in,out]	R		    the range.
	* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
	* 								otherwise.
	*/
	virtual void draw(Image & im, fBox2 & R, bool highQuality = true) override
		{
		im.canvas_draw_thick_filled_circle(R, center, radius, thickness, false, color, fillcolor, highQuality);
		}


	/**
	* Return the object's bounding box.
	*
	* @return	A fBox2.
	*/
	virtual fBox2 boundingBox() const override
		{
		return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius);
		}


	/**
	* Print info about the object into an std::string.
	*/
	virtual std::string toString(bool debug = false) const override
		{
		return "todo";
		// TODO
		}

	/**
	* Serialize the object.
	*/
	virtual void serialize(OBaseArchive & ar) const override
		{
		// TODO
		}

	/**
	* Deserialize this object.
	*/
	virtual void deserialize(IBaseArchive & ar) override
		{
		// TODO
		}



};




class FigureBox;

class FigureCircle;

class FigureEllipse;

class FigureDot;

class FigureLine;

class FigurePolyLine;

class FigureTriangle;

class FigureConvexPolygon;

class FigureQuadBezier;

class FigureRatQuadBezier;

class FigureCubicBezier;

class FigureGroup;




/**   
 * Thread that draws figures inside an Image object.
 *    
 * Each thread has its own queue which it polls to query figures to draw. 
 * Instances of this class are created and managed by the FigureDrawerDispatcher class.
 */
class FigureDrawerWorker : public ThreadWorker
{

public:

	/** Constructor. Initially disabled, nothing is drawn. */
	FigureDrawerWorker() : ThreadWorker(), _queue(QUEUE_SIZE), _nb_drawn(0), _im(nullptr), _R(fBox2()), _hq(true)
		{
		}


	/** Stop the thread if active and set the parameters. */
	void set(Image* im, fBox2 R, bool hq)
		{
		stop();
		_queue.clear();
		_nb_drawn = 0;
		_im = im;
		_R = R;
		_hq = hq;
		}

	/** Interrupt any work in progress. */
	void stop()
		{
		signal(CODE_STOP_AND_WAIT);
		sync();
		}

	/* (re)start work (return without waiting for sync(). */
	void restart()
		{
		signal(CODE_RESTART);
		}


	/** push a new figure in the queue */
	MTOOLS_FORCEINLINE bool pushfigure(FigureInterface* fig)
		{
		return _queue.push(fig);
		}

	/* current progress wrt the queue size, between 0 and 50 */
	MTOOLS_FORCEINLINE int current_prog() const
		{
		if (_nb_drawn == 0) return 0; 
		return (int)((50 * _nb_drawn) / (_nb_drawn + _queue.size()));
		}

protected:


	/**
	* Work method. draws the figures.
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

	static const size_t QUEUE_SIZE = 1024*1024;
	static const int64 CODE_STOP_AND_WAIT = 0;
	static const int64 CODE_RESTART = 1;

	SingleProducerSingleConsumerQueue<FigureInterface*> _queue;	// the queue containing the figures to draw
	std::atomic<size_t> _nb_drawn;								// number of figure drawn since the work started 
	std::atomic<Image*> _im;									// the image to draw onto
	std::atomic<fBox2>  _R;										// range to use
	std::atomic<bool>	_hq;									// true for high quality drawing

};



/**
 * Class that manage a TreeFigure class and draws it onto an Image using
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
			stop();							// interrupt any work in progress
			_figTree = figtree;				// save the tree figure object
			delete [] _workers;				// delete previous threads
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
			stop(); // stop everything
			_nb = 0;
			_phase = 0;
			_R = R;
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].set(_images[i], R, hq); } // set parameters for worker threads
			signal(CODE_RESTART); // start the dispatcher thread
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].restart(); } // start the worker threads. 
			}


		/** Stop all threads (dispatcher and workers).*/
		void stop()
			{
			signal(CODE_STOP_AND_WAIT); //stop the dispatcher thread
			for(size_t i = 0; i < _images.size(); i++) { _workers[i].stop(); } // stop the worker threads
			sync(); 
			}


		/** Query if the thread are currently enabled. */ 
		bool enableAllThreads() const
			{
			return enable();
			}


		/** Enable/disable all the threads */
		void enableAllThreads(bool status)
			{
			if (enable() == status) return;
			enable(status);
			for (size_t i = 0; i < _images.size(); i++) { _workers[i].enable(status); }
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
				tot /= Nth;
				return 50 + tot;
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
				[&](mtools::TreeFigure<FigureInterface *, N, double>::BoundedObject & bo ) -> void
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
		Plot2DFigure(Plot2DFigure && o) : internals_graphics::Plotter2DObj(std::move(o)), _figDrawer(o._figDrawer), _im(std::move(o._im)), _R(o._R),_hq(O._hq)
			{
			o._figDrawer = nullptr;
			}


		/**
		* Destructor. Remove the object if it is still inserted.
		**/
		virtual ~Plot2DFigure()
			{
			_figDrawer->stop();
			_figDrawer->enableAllThreads(false);
			detach();
			delete _figDrawer;
			}


	protected:


		virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override
			{
			_figDrawer->stop();
			_im.resizeRaw(imageSize);
			_im.clear(RGBc::c_Transparent);
			_R = range;
			_figDrawer->restart(_R,_hq);
			}


		virtual void resetDrawing() override
			{
			_figDrawer->stop();
			_im.clear(RGBc::c_Transparent);
			_figDrawer->restart(_R,_hq);
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












#define HH 5



class PlotTestF : public Plot2DBasic
{
	static const int N = 10;

	const double LX = 100;
	const double LY = 100;
	const double R = 1;


	std::vector<TreeFigure<int, HH>::BoundedObject *> v;

public:


	PlotTestF() : Plot2DBasic() , gen(0)
		{
		for (int k = 0; k < N; k++)
			{
			fVec2 center(Unif(gen)*LX, Unif(gen)*LY);
			double rad = Unif(gen)*Unif(gen)*Unif(gen)*Unif(gen)*R;
			TF.insert(fBox2(center.X() - rad, center.X() + rad, center.Y() - rad, center.Y() + rad), k);
			}
		v.resize(N);
		}




	virtual void draw(const fBox2 & R, Image & im, float opacity) override
	{
		RGBc color = RGBc::c_Red.getMultOpacity(0.5f);
		RGBc fillcolor = RGBc::c_Blue.getMultOpacity(0.1f);



		Chronometer();
		size_t count = 0;

		try
		{
			TF.iterate_intersect(
				zoomOut(R),
				[&](TreeFigure<int, HH>::BoundedObject & bo) -> void
				{
				v[count] = &bo;
				count++;
				if (count > 100000) throw "";
				});
		}
		catch (...) {}
		cout << "listed in  " << durationToString(Chronometer(), true) << "\n";
		cout << "nb = " << count << "\n";

		Chronometer();
		for (size_t i = 0; i < count; i++)
			{
			im.canvas_draw_ellipse_in_box(R, v[i]->boundingbox, color);		
			}
		cout << "drawn in  " << durationToString(Chronometer(), true) << "\n";

	
		return;
		}


private:


	TreeFigure<int, HH> TF;

	std::vector<std::pair<fVec2, int> >  cl;

	MT2004_64 gen;

};







void testplotfigure()
	{
	MT2004_64 gen;

	static const int NNN = 5; 

	TreeFigure<FigureInterface*, NNN> figtree;

	int nb = 10000000;

	cout << "Creating... ";
	for (int k = 0; k < nb; k++)
		{
		fVec2 pos = { 10000 * Unif(gen),10000 * Unif(gen) };
		double rad = Unif(gen);
		FigureCircle * C = new FigureCircle(pos, rad, RGBc::c_Red.getMultOpacity(0.5));
		figtree.insert(C->boundingBox(), C);
		}
	cout << "ok !\n\n";
	Plot2DFigure<NNN> PF(figtree,4);

	Plotter2D plotter; 
	plotter[PF];
	plotter.autorangeXY();
	plotter.plot();
	}













/*

drawing parameters

 antialiased    (bool)
 blend			(bool)
 tickness		(double)
 tickscale		(double)
 
 figures

 outline 

 - lines
 - multi broken lines
 - closed multi  broken lines
 - open bezier curves
 - circle ellipse

 
 - triangle
 - square
 - convex polygon
 - circle
 - ellipse


 */







class TestImage : public Image
	{

	public:

	//	void draw_line_new(const iVec2 & P1, const iVec2 & P2, RGBc color, int32 penwidth = 0, bool antialiasing = true, bool blending = true);


	TestImage(int64 lx, int64 ly) : Image(lx, ly) 	
	{

	}
	




	};




	MT2004_64 gen;

#define NN 1



	/* fast inverse squere root */





void testCE()
	{
	TestImage imA(1000, 1000);
	TestImage imB(1000, 1000);
	imA.clear(RGBc::c_White);
	imB.clear(RGBc::c_White);
	MT2004_64 gen(0);

	size_t N = 50000;

	
	int64 mult_rx = 10000; 
	int64 mult_ry = 10000;
	int64 mult_pos = 10000; 
	

	std::vector<iVec2> center(N, iVec2());
	std::vector<int64> rx(N, 1);
	std::vector<int64> ry(N, 1);

	for (size_t i = 0; i < N; i++)
		{
		center[i] = { -mult_pos + (int64)(2 * Unif(gen)*mult_pos), -mult_pos + (int64)(2 * Unif(gen)*mult_pos) };
		rx[i] = 1 + (int64)(Unif(gen)*mult_rx);
		ry[i] = 1 + (int64)(Unif(gen)*mult_ry);

		}



	cout << "Simulating A... ";
	Chronometer(); 
	for (size_t i = 0; i < N; i++)
		{
		imA.draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true,true,3);
		}
	auto resA = Chronometer();
	cout << "done in " << durationToString(resA, true) << "\n";


	cout << "Simulating B... ";
	Chronometer();
	for (size_t i = 0; i < N; i++)
		{
		imB.draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true, true,3);
		}
	auto resB = Chronometer();
	cout << "done in " << durationToString(resB, true) << "\n";


	auto PA = makePlot2DImage(imA, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
	auto PB = makePlot2DImage(imB, 1, "Image B");   // Encapsulate the image inside a 'plottable' object.	
	Plotter2D plotter;              // Create a plotter object
	plotter[PA][PB];                // Add the image to the list of objects to draw.  	
	plotter.autorangeXY();          // Set the plotter range to fit the image.
	plotter.plot();                 // start interactive display.

	}




#include "mtools\maths\bezier.hpp"






void testQuad(const fBox2 & B, BezierRationalQuadratic BQ, Image & im)
{
	RGBc color;
	auto C = B;
	C.enlarge(2);
	double res[12];
	int nb = BQ.intersect_rect(C, res);
	for (int i = (nb - 1); i > 0; i--)
		{
		res[i] = (res[i] - res[i - 1]) / (1.0 - res[i - 1]);
		}

	for (int i = 0; i < nb; i++)
		{
		auto sp = BQ.split(res[i]);
		BQ = sp.second;
		color = (C.isInside(sp.first(0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
		sp.first.normalize();
		im.draw_quad_bezier(sp.first.P0, sp.first.P2, sp.first.P1, sp.first.w1, color, true, true, true);
		}

	color = (C.isInside(BQ(0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
	BQ.normalize();
	im.draw_quad_bezier(BQ.P0, BQ.P2, BQ.P1, BQ.w1, color, true, true, true);
}




void draw(BezierQuadratic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_quad_bezier(sp.P0, sp.P2, sp.P1, 1, color, true, true, true, penwidth);
	}

void draw(BezierRationalQuadratic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_quad_bezier(sp.P0, sp.P2, sp.P1, sp.w1, color, true, true, true, penwidth);
	}

void draw(BezierCubic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_cubic_bezier(sp.P0, sp.P3, sp.P1, sp.P2, color, true, true, true, penwidth);
	}


template<typename BezierClass> void testBezier(fBox2 B, BezierClass curve, Image & im)
	{
	draw(curve, im, RGBc::c_Black, 1);
	B.enlarge(2);
	BezierClass subcurves[5];
	int tot = splitBezierInsideBox(B, curve, subcurves);
	for (int i = 0; i < tot; i++) { draw(subcurves[i], im, RGBc::c_Red, 2); }
	}







void testCF()
{
	size_t N = 50000;
	int64 LX = 1000;
	int64 LY = 1000;

	TestImage im(LX, LY);
	im.clear(RGBc::RGBc(240,240,200));
	MT2004_64 gen(0);


	while (1)
	{
		im.clear(RGBc::RGBc(240, 240, 200));

		iVec2 P0 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P1 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P2 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P3 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		double w = Unif(gen) * 10;

		cout << "P0 : " << P0 << "\n";
		cout << "P1 : " << P1 << "\n";
		cout << "P2 : " << P2 << "\n";
		cout << "P3 : " << P3 << "\n";
		cout << "w : " << w << "\n";

		BezierQuadratic curve(P0, P1,P2);
		//BezierRationalQuadratic curve(P0, 1.0, P1, w, P2,1.0);
		//BezierCubic curve(P0, P1, P2, P3);

		auto bb = curve.integerBoundingBox();
		im.draw_box(bb, RGBc::c_Gray, true);
		im.draw_dot(P0, RGBc::c_Green, true, 2);
		im.draw_dot(P1, RGBc::c_Green, true, 2);
		im.draw_dot(P2, RGBc::c_Green, true, 2);
		im.draw_dot(P3, RGBc::c_Green, true, 2);

		iBox2 TB{ 100,900,200,800 };
		im.draw_box(TB, RGBc::c_Yellow.getMultOpacity(0.5), true);
		im.draw_rectangle(TB, RGBc::c_Yellow, true);

		testBezier(TB, curve, im);
			


		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.
	}
}






void LineBresenham(iVec2 P1, iVec2 P2, Image & im, RGBc color)
{
	int64 x1 = P1.X(); 
	int64 y1 = P1.Y();
	int64 x2 = P2.X();
	int64 y2 = P2.Y();

	int64 dy = y2 - y1;
	int64 dx = x2 - x1;
	int64 stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;        // dy is now 2*dy
	dx <<= 1;        // dx is now 2*dx

	im.operator()(x1, y1).blend(color);


	if (stepx == 1) 
		{
		if (stepy == 1)
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1++;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1++;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1++;
						fraction -= dy;
					}
					y1++;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		else
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1--;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1++;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1++;
						fraction -= dy;
					}
					y1--;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		}
	else
		{
		if (stepy == 1)
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1++;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1--;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1--;
						fraction -= dy;
					}
					y1++;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		else
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1--;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1--;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1--;
						fraction -= dy;
					}
					y1--;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		}

}


inline void assert(int nb, fVec2 Pf1, fVec2 Pf2, bool sta)
	{
	if (!sta)
		{
		cout << "Error " << nb << " at " << Pf1 << " , " << Pf2 << "\n";
		cout.getKey();
		}
	}

void test_lines(int L, double epsilon)
{
	Image im((int)L*epsilon + 2, (int)L*epsilon + 2);
	im.clear(RGBc::c_White);

	for (int x1 = 0; x1 < L; x1++)
		{
		for (int y1 = 0; y1 < L; y1++)
			{
			for (int x2 = 0; x2 < L; x2++)
				{
				for (int y2 = 0; y2 < L; y2++)
					{

					fVec2 Pf1 = { x1*epsilon + 1, y1*epsilon + 1 };
					fVec2 Pf2 = { x2*epsilon + 1, y2*epsilon + 1 };

					Image::_bdir dira,dirb;
					Image::_bpos posa,posb;
					iVec2 P1a, P1b, P2a, P2b;

					int64 lena = im._init_line(Pf1, Pf2, dira, posa, P1a, P2a); 
					int64 lenb = im._init_line(Pf2, Pf1, dirb, posb, P1b, P2b);

					
					assert(0, Pf1, Pf2, lena == lenb);
					assert(1, Pf1, Pf2, P1a == P2b);
					assert(2, Pf1, Pf2, P2a == P1b);
					assert(3, Pf1, Pf2, posa.x = P1a.X());
					assert(4, Pf1, Pf2, posa.y = P1a.Y());
					assert(5, Pf1, Pf2, posb.x = P1b.X());
					assert(6, Pf1, Pf2, posb.y = P1b.Y());
					
					for (int64 i = 0; i < lena; i++)
						{
						im(posa.x, posa.y) = RGBc::c_Black;
						im._move_line(dira, posa,1);
						}
					
					im(posa.x, posa.y) = RGBc::c_Black;
					assert(7, Pf1, Pf2, posa.x = P2a.X());
					assert(8, Pf1, Pf2, posa.y = P2a.Y());

					for (int64 i = 0; i < lenb; i++)
						{
						assert(9, Pf1, Pf2, im(posb.x, posb.y) == RGBc::c_Black);
						im(posb.x, posb.y) = RGBc::c_White;
						im._move_line(dirb, posb, 1);
						}
					assert(10, Pf1, Pf2, im(posb.x, posb.y) == RGBc::c_Black);
					im(posb.x, posb.y) = RGBc::c_White;

					assert(11, Pf1, Pf2, posb.x = P2b.X());
					assert(12, Pf1, Pf2, posb.y = P2b.Y());
					
					}
				}
			}
		cout << ".";
		}



}






inline void nextpoint(double l, Image & im, fVec2 & A, fVec2 & B, fVec2 & C, fVec2 D, RGBc color)
	{
	fVec2 M = 0.5*(A + B);
	fVec2 U = C - M;

	fVec2 Al = A + U;
	fVec2 Bl = B + U;

	fVec2 V = D - C; 

	fVec2 H = { V.Y(), -V.X() };
	H.normalize(); H *= l; 

	fVec2 UU = (Al - C - H); UU.normalize();  UU *= l; 
	iVec2 AA = C + UU;
	fVec2 VV = (Bl - C + H); VV.normalize();  VV *= l; 
	iVec2 BB = C + VV;

	fVec2 A1 = A;
	fVec2 A2 = AA;
	fVec2 A3 = BB; 
	fVec2 A4 = B;

	iVec2 AP1, AP2, AP3, AP4;

	Image::_bdir dir12, dir21;
	Image::_bpos pos12, pos21;
	int64 len12 = im._init_line(A1, A2, dir12, pos12, AP1, AP2);
	pos21 = pos12;
	dir21 = dir12;
	im._reverse_line(dir21, pos21, len12);

	Image::_bdir dir23, dir32;
	Image::_bpos pos23, pos32;
	int64 len23 = im._init_line(A2, A3, dir23, pos23, AP2, AP3);
	pos32 = pos23;
	dir32 = dir23;
	im._reverse_line(dir32, pos32, len23);

	Image::_bdir dir34, dir43;
	Image::_bpos pos34, pos43;
	int64 len34 = im._init_line(A3, A4, dir34, pos34, AP3, AP4);
	pos43 = pos34;
	dir43 = dir34;
	im._reverse_line(dir43, pos43, len34);

	Image::_bdir dir41, dir14;
	Image::_bpos pos41, pos14;
	int64 len41 = im._init_line(A4, A1, dir41, pos41, AP4, AP1);
	pos14 = pos41;
	dir14 = dir41;
	im._reverse_line(dir14, pos14, len41);

	Image::_bdir dir13;
	Image::_bpos pos13;
	int64 len13 = im._init_line(A1, A3, dir13, pos13, AP1, AP3);


	static const bool caa = true;

	im._lineBresenham_avoid<true, true, false, caa, false>(dir12, pos12, len12+1, dir14, pos14, len41 + 1, color, 0);
	im._lineBresenham_avoid<true, true, false, caa, true>(dir43, pos43, len34+1, dir41, pos41, len41 + 1, color, 0);

	/*
	im(AP2) = RGBc::c_Black.getMultOpacity(0.5);
	im(AP3) = RGBc::c_Black.getMultOpacity(0.5);
	*/
	

	im._lineBresenham_avoid_both_sides_triangle<true, true, false, false, true>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir34, pos34, len34 + 1, color, 0);
	
	im._lineBresenham_avoid_both_sides<true, true, false, false, true>
		(dir13, pos13, len13,
			dir12, pos12, len12,
			dir14, pos14, len41,
			dir32, pos32, len23,
			dir34, pos34, len34, color, 0);
			
	
	im._draw_triangle_interior<true, true>(A1, A2, A3, color);
	im._draw_triangle_interior<true, true>(A1, A3, A4, color);
	
	
	A = AA; 
	B = BB; 
	C = D; 
	return;
	}

void rot(fVec2 & V, double alpha)
	{
	double b = alpha * TWOPI / 360;
	V = { V.X() * cos(b) + V.Y() * sin(b), -V.X() * sin(b) + V.Y() * cos(b) };
	}




int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	testplotfigure();
	return 0;

	double lx = 800.0;
	double ly = 600.0;


	TestImage im((int)(lx), (int)(ly));

	RGBc color = RGBc::c_Red.getMultOpacity(0.5);;
	RGBc color2 = RGBc::c_Green.getMultOpacity(0.5);;
	RGBc colorfill = RGBc::c_Blue.getMultOpacity(0.5);;


	colorfill = color;

	fVec2 Pf1, Pf2, Pf3; 
	iVec2 P1, P2, P3;

	MT2004_64 gen(0);

	/*
	while (1)
		{
		im.clear(RGBc::c_White);
		Pf1 = { Unif(gen)*lx, Unif(gen)*ly };
		Pf2 = { Unif(gen)*lx, Unif(gen)*ly };
		Pf3 = { Unif(gen)*lx, Unif(gen)*ly };
		im._draw_triangle_interior<true, true>(Pf1, Pf2, Pf3, colorfill);

		Image::_bdir dir12, dir21, dir13, dir31, dir23;
		Image::_bpos pos12, pos21, pos13, pos31, pos23;

		int64 len12 = im._init_line(Pf1, Pf2, dir12, pos12, P1, P2);
		dir21 = dir12; pos21 = pos12; im._reverse_line(dir21, pos21, len12);

		int64 len13 = im._init_line(Pf1, Pf3, dir13, pos13, P1, P3);
		dir31 = dir13; pos31 = pos13; im._reverse_line(dir31, pos31, len13);

		int64 len23 = im._init_line(Pf2, Pf3, dir23, pos23, P2, P3);


		fVec2 vA = (Pf3 - Pf1), vB = (Pf2 - Pf1);
		double det = vA.X()*vB.Y() - vB.X()*vA.Y();

		if (det > 0)
			{
			im._lineBresenham<true, true, false, false, true, false>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, true>(dir13, pos13, len13 + 1, dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, false>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
			}
		else
			{
			im._lineBresenham<true, true, false, false, true, true>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, false>(dir13, pos13, len13 + 1 , dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, true>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
			}
		
		cout << "Pf1 = " << Pf1 << " \t P1 = " << P1 << "\n";
		cout << "Pf2 = " << Pf2 << " \t P2 = " << P2 << "\n";
		cout << "Pf3 = " << Pf3 << " \t P3 = " << P3 << "\n";

		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.		
		}
		*/


	im.clear(RGBc::c_White);


	{

		double l = 35; 
		double r = 5; 

		fVec2 O = { 200.5,200.5 };

		fVec2 A = { O.X() - l, O.Y() };
		fVec2 B = { O.X() + l, O.Y() };
		fVec2 C = { O.X(), O.Y() + r };


		fVec2 R = { 0, r };


		fVec2 D;

		for (int i = 0; i < 100; i++)
		{
			D = C + R;
			nextpoint(l, im, A, B, C, D, color);
			rot(R, 2);
		}


		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
	//	plotter.plot();                 // start interactive display.		


	}


	Chronometer();
	int NSN = 100;
	double l = 0.75;

	for (int i = 0; i < NSN; i++)
	{

		fVec2 Pfa = { Unif(gen)*lx, Unif(gen)*ly };
		fVec2 Pfb = { Unif(gen)*lx, Unif(gen)*ly };

		iVec2 JA = { (int64)round(Pfa.X()), (int64)round(Pfa.Y()) };
		iVec2 JB = { (int64)round(Pfb.X()), (int64)round(Pfb.Y()) };

//		if (i & 1)
	//		im.draw_line(JA, JB, color2, true, true, true, l);
	//	else
		{
			fVec2 U = Pfa - Pfb;
			fVec2 V = { U.Y(), -U.X() };
			V.normalize();
			V *= l;

			fVec2 A1 = Pfa + V;
			fVec2 A2 = Pfb + V;
			fVec2 A3 = Pfb - V;
			fVec2 A4 = Pfa - V;

			im._draw_triangle_interior<true, true>(A1, A2, A3, colorfill);
			im._draw_triangle_interior<true, true>(A1, A3, A4, colorfill);

			iVec2 AP1, AP2, AP3, AP4;

			Image::_bdir dir12, dir21;
			Image::_bpos pos12, pos21;
			int64 len12 = im._init_line(A1, A2, dir12, pos12, AP1, AP2);
			pos21 = pos12;
			dir21 = dir12;
			im._reverse_line(dir21, pos21, len12);

			Image::_bdir dir23, dir32;
			Image::_bpos pos23, pos32;
			int64 len23 = im._init_line(A2, A3, dir23, pos23, AP2, AP3);
			pos32 = pos23;
			dir32 = dir23;
			im._reverse_line(dir32, pos32, len23);

			Image::_bdir dir34, dir43;
			Image::_bpos pos34, pos43;
			int64 len34 = im._init_line(A3, A4, dir34, pos34, AP3, AP4);
			pos43 = pos34;
			dir43 = dir34;
			im._reverse_line(dir43, pos43, len34);

			Image::_bdir dir41, dir14;
			Image::_bpos pos41, pos14;
			int64 len41 = im._init_line(A4, A1, dir41, pos41, AP4, AP1);
			pos14 = pos41;
			dir14 = dir41;
			im._reverse_line(dir14, pos14, len41);

			Image::_bdir dir13;
			Image::_bpos pos13;
			int64 len13 = im._init_line(A1, A3, dir13, pos13, AP1, AP3);

			
			static const int caa = true;
			im._lineBresenham<true, true, false, false, caa, false>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, caa, false>(dir23, pos23, len23 + 1, dir21, pos21, len12 + 1, color, 0);
			im._lineBresenham_avoid<true, true, false, caa, false>(dir34, pos34, len34 + 1, dir32, pos32, len23 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, caa, false>(dir41, pos41, len41, dir43, pos43, len34 + 1, dir12, pos12, len12 + 1, color, 0);
			

			im._lineBresenham_avoid_both_sides<true, true, false, false, true>
				(dir13, pos13, len13,
					dir12, pos12, len12,
					dir14, pos14, len41,
					dir32, pos32, len23,
					dir34, pos34, len34, color, 0);
		}

			 
			 

		/*

		Image::_bdir dir12, dir21, dir13, dir31, dir23;
		Image::_bpos pos12, pos21, pos13, pos31, pos23;

		int64 len12 = im._init_line(Pf1, Pf2, dir12, pos12, P1, P2);
		dir21 = dir12; pos21 = pos12; im._reverse_line(dir21, pos21, len12);

		int64 len13 = im._init_line(Pf1, Pf3, dir13, pos13, P1, P3);
		dir31 = dir13; pos31 = pos13; im._reverse_line(dir31, pos31, len13);

		int64 len23 = im._init_line(Pf2, Pf3, dir23, pos23, P2, P3);

		fVec2 vA = (Pf3 - Pf1), vB = (Pf2 - Pf1);
		double det = vA.X()*vB.Y() - vB.X()*vA.Y();

		if (det > 0)
		{
			im._lineBresenham<true, true, false, false, true, false>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, true>(dir13, pos13, len13 + 1, dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, false>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
		}
		else
		{
			im._lineBresenham<true, true, false, false, true, true>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, false>(dir13, pos13, len13 + 1, dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, true>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
		}


		*/

	}


	int64 xs = 120;
	int64 ys = 0;

	iBox2 BB(100 + xs, 200 + xs, 100 + ys, 200 + ys);





	colorfill = RGBc::c_Blue.getMultOpacity(0.5);

	im.clear(RGBc::c_White);
	im.draw_box(BB, RGBc::c_Gray, false);

	cout << mtools::durationToString(Chronometer(), true);


	//im._draw_ellipse4_AA<true,true,false>(im.imageBox(), { 300,300 }, 1, 2, color, colorfill, 0);

	double R = 100;
	fVec2 P = {300,300};

	//im.draw_thick_ellipse(P, R, R + 50, 3, 5, color, true, true);


	fBox2 SR(0.0, 4.0, 0.0, 3.0);

	im.canvas_draw_thick_filled_circle(SR, { 0.5,0.5 }, 3, 3, false, color, colorfill);

	im(300, 300) = RGBc::c_Black;

	//im.draw_thick_circle(P, 10 + R, 2.0, color, colorfill, true, true);
	//im.draw_thick_circle(P, 20 + R, 2.0, color, colorfill, true, true);

	/*
	int qqL = 10000;
	Chronometer();
	for (int qq = 0; qq < qqL; qq++)
		{
			im._draw_ellipse_thick_AA<true, false, false>(im.imageBox(), { 250.0 , 190.0 }, 120, 170, 121, 171, color, colorfill, 0);
		}
	cout << "done in " << mtools::durationToString(Chronometer(), true) << "\n";
	
	for (int qq = 0; qq < qqL; qq++)
	{
		im._draw_ellipse2_AA<true, false, false>(im.imageBox(), { 455.0 , 450.0 }, 150, 200, color, colorfill, 0);
	}
	cout << "done in " << mtools::durationToString(Chronometer(), true) << "\n";

	for (int qq = 0; qq < qqL; qq++)
	{
		im.draw_ellipse({ 250,550 }, 150, 200, color,true, false, 0);
	}
	cout << "done in " << mtools::durationToString(Chronometer(), true) << "\n";


	*/
	cout << "zzzz"; 

	//auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	


	//auto PTF = makePlotTestFig(); 


	PlotTestF PTF; 

	Plotter2D plotter;              // Create a plotter object
	plotter[PTF];	                // Add the image to the list of objects to draw.  	
	plotter.autorangeXY();          // Set the plotter range to fit the image.

	//plotter.range().setRange(zoomIn(fBox2(66.201, 66.217, 53.530, 53.545)));
	plotter.plot();                 // start interactive display.		


		return 0;
	}

