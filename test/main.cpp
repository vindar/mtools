/*
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <string>
#include <sstream>

class MyGlWindow : public Fl_Gl_Window {
	void draw() {
		if (!valid()) {
			valid(1);
			glClearColor(0.0, 0.0, 0.0, 0.0);
			gl_font(1, 12);
			glColor3f(1.0f, 1.0f, 1.0f);
			glDisable(GL_DEPTH_TEST);
		}
		glClear(GL_COLOR_BUFFER_BIT);
		for (int j = 0; j < 12; j++)
			for (int i = 0; i < 10; i++)
			{
				std::ostringstream oss;
				oss << " x " << (i + 10 * j); /// <- no bug without the " = "...
				gl_draw(oss.str().c_str(), oss.str().size(), -1.0f + 0.2f*i, -1.0f + +0.15f*j + 0.1);
			}
	}

public:
	MyGlWindow(int X, int Y, int W, int H, const char*L = 0) : Fl_Gl_Window(X, Y, W, H, L) {
	}
};

int main() {
	Fl_Window win(600, 400);
	MyGlWindow mygl(0, 0, win.w(), win.h());
	win.show();
	return(Fl::run());
}


*/

#include <mtools/mtools.hpp>


using namespace mtools;



double f(double x)
{
	return sin(x);
}


void test()
	{
	auto P = makePlot2DFun(f, "sin");
	auto Q = makePlot2DFun(f, "sin");

	Plotter2D plotter;
	plotter[P]; 
	plotter[Q];
	plotter.autorangeXY();

	plotter.plot(); 
	plotter.remove(P);
	}








Image im(500, 600);






void testImageDisplay()
{
	MT2004_64 gen;
	im.clear(RGBc::c_Yellow);
	im.draw_circle(iVec2{ 250,300 }, 200, RGBc::c_Red);
	im.draw_text({ 100,100 }, "Hello World\n", MTOOLS_TEXT_TOPLEFT, RGBc::c_Blue, 80);


	ImageDisplay ID(800, 600, 100, 100, "ImageDisplay",false);

	ID.allowUserSelection(true);
	ID.forceSelectionBeforeClosing(true);
	ID.setSelection(iBox2(0, 50, 100, 200));


	ID(im);

	cout << "START  !\n";


	ID.autoredraw(100);

	ID.startDisplay();

	while (ID.isDisplayOn()) 
		{ 
		double a = Unif(gen) * 500;
		double b = Unif(gen) * 600;
		double c = Unif(gen) * 10;

		int R = Unif(gen) * 255;
		int G = Unif(gen) * 255;
		int B = Unif(gen) * 255;
		RGBc col(R, G, B);

		im.draw_filled_circle({ a,b }, c, col, col);
		}


	cout << ID.getSelection(true) << "\n";

	return;
}





	




void testDelaunayVoronoi()
	{
	mtools::MT2004_64 gen(0);
	mtools::DelaunayVoronoi DV;

	// add 100 points uniformly distributed in [0,1]^2
	for (int i = 0; i < 100; i++) DV.DelaunayVertices.push_back(mtools::fVec2(Unif(gen), Unif(gen)));
		
	// compute the Delaunay trianulation and Voronoi diagram
	DV.compute(); 

	// draw the graphs
	mtools::Plotter2D plotter; 
	auto canvas = mtools::makeFigureCanvas(2);

	// draw the Delaunay triangulation
	int nb_D_e = (int)DV.DelaunayEdgesIndices.size();
	for (int k = 0; k < nb_D_e; k++)
		{
		mtools::iVec2 e = DV.DelaunayEdgesIndices[k];
		canvas(mtools::Figure::Line(DV.DelaunayVertices[e.X()], DV.DelaunayVertices[e.Y()], mtools::RGBc::c_Red), 0);
		}

	// draw the Voronoi diagram
	int nb_V_e = (int)DV.VoronoiEdgesIndices.size();
	for (int k = 0; k < nb_V_e; k++)
		{
		auto e = DV.VoronoiEdgesIndices[k];
		mtools::fVec2 P1 = DV.VoronoiVertices[e.X()];
		if (e.Y() == -1)
			{ // semi-infinite ray
			mtools::fVec2 N = DV.VoronoiNormals[e.X()];
			canvas(mtools::Figure::Line(P1, P1 + N, mtools::RGBc::c_Green), 1);
			}
		else
			{ // regular edge
			mtools::fVec2 P2 = DV.VoronoiVertices[e.Y()];
			canvas(mtools::Figure::Line(P1, P2, mtools::RGBc::c_Black), 1);
			}
		}

	// plot
	auto P = mtools::makePlot2DFigure(canvas, 4, "Delaunay Voronoi");
	plotter[P];
	plotter.range().setRange({ 0,1,0,1 });
	plotter.plot();
	}











char bb[10000000];





namespace mtools
{



	/**
	* Class that draws Plotter2Dobj into an image (without
	*
	*
	**/
	class Drawer2D
		{

		std::vector<internals_graphics::Plotter2DObj*> _tabobj;	// vector containing pointers to all object inserted
		Image *	_im;	// the image to draw onto
		int		_nbframe; // number of frame created. 

		internals_graphics::RangeManager _rm; // the range manager 


		Drawer2D(const Drawer2D&) = delete;                  // no copy
		Drawer2D & operator=(const Drawer2D&) = delete;      //

		static const int _DEFAULT_LX = 800;
		static const int _DEFAULT_LY = 600;


		static void objectCB_static(void* data, void* data2, void* obj, int code);
		void objectCB(void* obj, int code);



		void add(internals_graphics::Plotter2DObj* obj);

		void remove(internals_graphics::Plotter2DObj* obj);

		void moveUp(internals_graphics::Plotter2DObj* obj)
			{
			for (int i = 0; i < (int)_tabobj.size(); i++)
				{
				if (_tabobj[i] == obj)
					{
					if (i == 0) return; // nothing to do if already on top
					auto obj2 = _tabobj[i - 1];
					_tabobj[i - 1] = obj;
					_tabobj[i] = obj2;
					return;
					}
				}
			MTOOLS_DEBUG("Plotter2DWindow::moveUp(), object not found.");
			}

		void moveDown(internals_graphics::Plotter2DObj* obj);

		void moveTop(internals_graphics::Plotter2DObj* obj);

		void moveBottom(internals_graphics::Plotter2DObj* obj);

		void removeAll();

		void fixObjectWindow();


		public:


		/**
		* Default ctor. 
		* Create an empty drawer (no image and no object inserted)
        **/
		Drawer2D() : _tabobj(),  _im(nullptr), _nbframe(0), _rm(iVec2(_DEFAULT_LX, _DEFAULT_LY))
			{
			reset();
			}


		/**
		* Reset the object to its initial state. 
		* Remove all objects from the plotter as well as the image.
		**/
		void reset()
			{
			removeAll(); // remove objects
			_im = nullptr; // remove image
			_nbframe = 0;
			_rm.reset(); 
			}


		/**
		* Set the image to draw onto.
		**/
		void setImage(Image& im)
			{			
			_im = &im; 
			if (!(_im->isEmpty()))
				{
				_rm.winSize(im.dimension());
				// change the range ?
				}
			// notify objects ? 
			// ....
			}


		/**
		* Remove the image (if any).
		**/
		void setImage()
			{
			_im = nullptr;			
			}


		/**
		* Return the RangeManager object used to set the
        * range.
		**/
		internals_graphics::RangeManager & range()
			{
			return _rm; 
			}


		/**
		* Fill the image with a uniform color
		**/
		void drawBackground(RGBc color)
			{
			_im->clear(color);
			}


		/**
		* Fill the image with a checker board pattern
		**/
		void drawCheckerBoard()
			{
			_im->checkerboard();
			}


		/**
		* Draw all the objects onto the image. 
		* do not erase the image prior to drawing
		**/
		void draw(mtools::Image im, int min_quality = 100)
			{

			}


		/**
		* save the image with a given name and optional frame number.
        * 
		* filename should be given with the extension (.png for example).
		**/
		void save(const std::string filename, bool add_number = true, int nb_digits = 6)
			{
			if (_im == nullptr) return;
			if (add_number)
				_im->save(filename.c_str(), ++_nbframe, nb_digits);
			else
				_im->save(filename.c_str());
			}


		/**
		* Return the number of frames saved to disk. 
		**/
		int nbFrames() const
			{
			return _nbframe;
			}

		/**
		* Insert an object
		**/
		void insert(mtools::internals_graphics::Plotter2DObj& obj)
			{

			}


		/**
		* Remove an object
		**/
		void remove(mtools::internals_graphics::Plotter2DObj& obj)
			{

			}


		/**
		* Remove all objects
		**/
		void removeAll()
			{

			}


		/**
		* Insert an object.
		**/
		Drawer2D& operator[](mtools::internals_graphics::Plotter2DObj& obj)
			{
			insert(obj);
			return(*this);
			}



		/*
			void test()
				{

				Image img(1000, 800);

				auto P = makePlot2DImage(im, 3, "im");
				((mtools::internals_graphics::Plotter2DObj *)(&P))->resetDrawing();

				}
				*/

		};









        /* called by the inserted object when it want a redraw or when it detaches itself, always called from FLTK */
        void Drawer2D::objectCB_static(void * data, void * data2, void * obj, int code) { MTOOLS_ASSERT(data != nullptr); ((Drawer2D*)data)->objectCB(obj, code); }
        void Drawer2D::objectCB(void * obj, int code)
            {
            MTOOLS_ASSERT(isFltkThread());
            switch (code)
                {
                case internals_graphics::Plotter2DObj::_REQUEST_DETACH:
                    { // the object is detaching itself
                    remove((Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_REFRESH:
                    {
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_YIELDFOCUS:
                    {
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_UP:
                    {
                    moveUp((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_DOWN:
                    {
                    moveDown((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_TOP:
                    {
                    moveTop((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_BOTTOM:
                    {
                    moveBottom((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEX:
                    {
                    useRangeX((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEY:
                    {
                    useRangeY((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEXY:
                    {
                    useRangeXY((internals_graphics::Plotter2DObj*)obj);
                    _PW->take_focus();
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_FIXOBJECTWIN:
                    {
                    fixObjectWindow();
                    return;
                    }
                }
            MTOOLS_ERROR("Plotter2DWindow::objectCB, incorrect code!");
            }












}





double sc(double s)
	{

	auto P = makePlot2DAxes();
	

	if (s == 0) return 1.0;
	return sin(s) / s;
	}


void testdrawer()
	{

	Image im(800, 600);

	im.checkerboard(RGBc::c_Red);
	im.save("hello.png", 123, 6);

	//Display2D disp;

	//disp.test();


	}










int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	//testDelaunayVoronoi();
	
	Plotter2D plot; 


	testdrawer();


	return 0;

	SerialPort sp;

	sp.open("COM18", 2000000);

	while (1)
		{
		//std::this_thread::sleep_for(10);
	//	Sleep(10);

		const int l = 15; 
		cout << "- sending " << l << "\n";

		sp.write(l);
		sp.flush();

		Chrono ch;
		ch.reset();

		int r = 0; 

		while (r < l * 1024)
			{			
			if (sp.available()) sp.read();
			r++;		
			}

		int el = (int)ch.elapsed();
		cout << " received in " << el << "ms\n";
		}





	cout << "done !\n\n";
	cout.getKey(); 
	return 0; 

}


























