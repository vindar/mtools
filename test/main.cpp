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

#include "mtools/graphics/internal/view2Dwidget.hpp"

#include "mtools/graphics/internal/imagewidget.hpp"



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



/** 
 *An image widget extent. 
 * 
 **/
class ImageWidgetExt : public internals_graphics::ImageWidget
	{


	public:

        /**
         * Constructor. NO image is associated by default. Draw a gray background. 
         *
         * @param   X   The X coordinate.
         * @param   Y   The Y coordinate.
         * @param   W   The width.
         * @param   H   The height.
         * @param   l   (Optional) name of the window.
        **/
		ImageWidgetExt(int X, int Y, int W, int H, const char* l = 0) : internals_graphics::ImageWidget(X, Y, W, H, l), _extim(nullptr), _im(W,H)
			{
			MTOOLS_INSURE(W > 0);
			MTOOLS_INSURE(H > 0);
			setDefaultRange();
			_im.clear(RGBc::c_Gray);
			internals_graphics::ImageWidget::setImage(&_im);
			}


		/** Destructor. */
		virtual ~ImageWidgetExt()
			{
			}


		/**
		* Set the default range (smallest view that encloses the whole image). 
		**/
		void setDefaultRange()
			{
			if ((_extim == nullptr) || (_extim->isEmpty()))
				{
				_viewR = fBox2(0.0, (double)_im.lx(), 0, (double)_im.ly());
				}
			else
				{
				_viewR = fBox2(0.0, (double)_extim->lx(), 0.0 , (double)_extim->ly()).fixedRatioEnclosingRect(((double)_im.lx()) / _im.ly());
				}
			redrawNow();
			}


        /**
         * Set the range to display. 
         *
         * @param   R   the range (empty for default range). 
        **/
		void setRange(const fBox2& R)
			{
			if (R.isEmpty())
				{
				setDefaultRange();
				}
			else
				{
				_viewR = R;
				redrawNow();
				}
			}


		/**
		* Redraw the image right away.
		**/
		void redrawNow()
			{
			// recreate the image _im from _extim and the current range
			_updateIm();	
			
			// copy the image in the offscreen buffer
			internals_graphics::ImageWidget::setImage(&_im);

			// ask for a redraw immediately
			redraw();
			flush();
			}


        /**
         * Sets the image to display.
         * 
         * The image is NOT copied and must remain available as long as it is displayed ! The image
         * buffer/dimensions must NOT change while the image is being displayed.
         * 
         * If it is needed to change the image buffer/dimensions, first call setImage(nullptr) and then
         * call again setImage(&im) after the change have been made. Meanwhile, setImage(nullptr)
         * the last image will still be displayed
         *
         * @param   im              (Optional) Pointer to the image to display. nullptr if nothing should
         *                          be displayed.
         * @param   setDefaultRange (Optional) true to use the default range, false to keep the current range
        **/
		void set(const Image* im = nullptr, bool useDefaultRange = true) 
			{
			if (im != _extim)
				{
				_extim = im;
				if (useDefaultRange && (_extim != nullptr) && (_extim->isEmpty() == false)) setDefaultRange();
				redrawNow();
				}
			}



	private:



		/** Updates the _im from _extim according to the current range */
		void _updateIm()
			{
			if ((_extim != nullptr) && (_extim->isEmpty() == false))
				{
				const int Lim = (int)_im.ly() - 1;
				const int Lextim = (int)_extim->ly() - 1;
	
				const int W = (int)_im.lx();
				const int H = (int)_im.ly();
				const double ex = _viewR.lx() / W;
				const double ey = _viewR.ly() / H;
				double y = _viewR.min[1] + (ey/2) - 0.5;
				for (int j = 0; j < H; j++)
					{
					double x = _viewR.min[0] + (ex/2) - 0.5;
					for (int i = 0; i < W; i++)
						{
						int ix = (int)x;
						int iy = (int)y;
						_im(i, Lim - j) = _extim->getPixel(ix, Lextim - iy, RGBc::c_Gray);
						x += ex;
						}
					y += ey;
					}
				}
			}


protected:

		/* fltk handle method */
		virtual int handle(int e) override
			{
			switch (e)
			{
			case FL_LEAVE: {  return 1; }
			case FL_ENTER: { take_focus(); return 1; }
			case FL_FOCUS: return 1;
			case FL_UNFOCUS: return 1;
			
			case FL_KEYDOWN:
				{
				take_focus();
				int key = Fl::event_key();
				if (key == FL_Escape)
					{
					if (_move_allowed)
						{
						setDefaultRange();
						redrawNow();
						}
					return 1;
					}
				if (key == FL_Page_Up)
					{
					if (_move_allowed)
						{
						_viewR = mtools::zoomIn(_viewR);
						redrawNow();
						}
					return 1;
					}

				if (key == FL_Page_Down)
					{
					if (_move_allowed)
						{
						_viewR = mtools::zoomOut(_viewR);
						redrawNow();
						}
					return 1;
					}
				if (key == FL_Left)
					{
					if (_move_allowed)
						{
						_viewR = mtools::left(_viewR);
						redrawNow();
						}
					return 1;
					}
				if (key == FL_Right)
					{
					if (_move_allowed)
						{
						_viewR = mtools::right(_viewR);
						redrawNow();
						}
					return 1;
					}
				if (key == FL_Up)
					{
					if (_move_allowed)
						{
						_viewR = mtools::up(_viewR);
						redrawNow();
						}
					return 1;
					}
				if (key == FL_Down)
					{
					if (_move_allowed)
						{
						_viewR = mtools::down(_viewR);
						redrawNow();
						}
					return 1;
					}
				return 1;
				}

			case FL_KEYUP:
				{
				return 1;
				}
				
			}

			return internals_graphics::ImageWidget::handle(e);
			}

		/* fltk method : draw the widget */
		virtual void draw() override
			{
			internals_graphics::ImageWidget::draw();
			}


	private:

		const Image* _extim;	// the image external source image  
		Image _im;				// internal image that mirror that drawn. 		
		fBox2 _viewR;			// the rectangle dislayed by the window. 
		bool _move_allowed;		// true if the user is allowed to change the range. 
	};





	Image im(500, 600);






class ImageCropper : public ImageWidgetExt
{

public:


	ImageCropper(int X, int Y, int W, int H, const char* name) : ImageWidgetExt(X, Y, W, H, name)
		{
		im.clear(RGBc::c_Yellow);
		im.draw_circle(iVec2{ 250,300 }, 200, RGBc::c_Red);
		im.draw_text({ 100,100 }, "Hello World\n", MTOOLS_TEXT_TOPLEFT, RGBc::c_Blue, 80);
		set(&im);
		}

protected:


};





/*

Class de base

class FlImageDisplay
{

	setImage();
	setRange();



}


/*
*
* Image with size (ix, iy)
* VIew with size (lx,ly)
* 
* 
* 
* 
* 
* 
* 
* 
* 
* INTERACTION
* 
*  - no interaction.
*  - allow panning ?
*  - allow zooming ?
*  - allow selection ? 
*  - allow closing windows. 
* 
* 
* MISC :
* 
* - Add info at begining. 
* - add confirmation at closing. 
* 
* 

*/



ImageCropper* IW;


int fltk_fun()
{
	IW = new ImageCropper(100, 100, 800, 600, "test");
	IW->show();
	IW->take_focus();
	return 0;
}

int showo()	
	{
//	IW->show();
	return 0;
	}

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	cout << "Hello ";

	mtools::IndirectFun<int> proxy(&fltk_fun);
	mtools::runInFltkThread(proxy); 
	

	cout << " ....\n";
	while(1) 
		{ 
		Sleep(1000);
		mtools::IndirectFun<int> proxyshow(&showo);
		mtools::runInFltkThread(proxyshow);
		}
	cout << "done !\n\n";
	cout.getKey(); 
	return 0; 

}
