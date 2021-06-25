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



namespace mtools
{





}

/** 
 *An image widget extent. 
 * 
 **/
class ImageWidgetExt : public internals_graphics::ImageWidget
	{



	private:

		iVec2 _prev_mouse;		// previous mouse position
		iVec2 _current_mouse;	// mouse position

		iVec2 _mouse_sel1;		// mouse position at start of selection
		iVec2 _pos_sel1;		// position of start of selection (in the image space). 
		iBox2 _selectR;			// the currently selected region

		bool _select_on;		// true if when are currently selecting a region by dragging with left button. 


		bool _move_allowed;		// true if the user is allowed to change the range. 
		bool _select_allowed;	// true if the user is allowed to change the range. 

		const Image* _extim;	// the image external source image  
		Image _im;				// internal image that mirror that drawn. 		
		fBox2 _viewR;			// the rectangle dislayed by the window. 

	public:

        /**
         * Constructor. No image is associated by default. Draw a gray background. 
         *
         * @param   X   The X coordinate of the window
         * @param   Y   The Y coordinate of the window
         * @param   W   The width of the window
         * @param   H   The height of the window
         * @param   l   (Optional) name of the window.
        **/
		ImageWidgetExt(int X, int Y, int W, int H, const char* l = 0) : internals_graphics::ImageWidget(X, Y, W, H, l), 			
			_prev_mouse(-1,-1), 
			_current_mouse(-1,-1), 
			_mouse_sel1(-1,-1), 
			_pos_sel1(-1,-1),
			_selectR(),
			_select_on(false), 
			_move_allowed(false), 
			_select_allowed(false), 
			_extim(nullptr), 
			_im(W,H)
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
		 * Decide whether user is allowed to move/zoom around the image.
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowUserMove(bool status)
			{
			_move_allowed = status;
			}


		/**
		 * Decide whether user is allowed to modify the selected region
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowUserSelection(bool status)
			{
			_select_allowed = status;
			}


		/**
		 * Sets the current selection rectange.
		 *
		 * @param [in,out]	selectRect The selection rectangle. empty when no region currently selected. 
		 */
		void setSelection(iVec2 & selectRect = iVec2())
			{
			_selectR = selectRect = iVec2();
			}


		/**
		 * Return the currently selected region. Return an empty rectangle when no region is selected.
		 *
		 * @param	clipWithImage (Optional) True to clip the selection rectangle with the image bounding
		 * 						  box.
		 */
		iBox2 getSelection(bool clipWithImage = true)
			{
			if ((_extim == nullptr) || (_extim->isEmpty())) return iBox2(); // no selection without an image
			return (clipWithImage ? (mtools::intersectionRect(_selectR, _extim->imageBox())) : _selectR);
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
				const bool sel = !(_selectR.isEmpty());
				const RGBc dcol = RGBc::c_Black.getMultOpacity(0.5);
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
						RGBc col = _extim->getPixel(ix, iy, RGBc::c_Gray);
						if ((sel)&&(!_selectR.isInside({ ix,iy })))	col.blend(dcol);
						_im(i, j) = col;
						x += ex;
						}
					y += ey;
					}
				}
			}



		bool _isIn(iVec2 m)
			{
			return ((m.X() >= 0) && (m.Y() >= 0) && (m.X() < w()) && (m.Y() < h()));
			}

        void _saveMouse()
            {
            _current_mouse = iVec2(Fl::event_x(), Fl::event_y());
            }

		bool _canZoomIn()
			{
			if ((_extim == nullptr) || (_extim->isEmpty())) return false;
			if ((_viewR.lx() >= 1.0) && (_viewR.ly() >= 1.0)) return true;
			return false;
			}

		bool _canZoomOut()
			{
			if ((_extim == nullptr) || (_extim->isEmpty())) return false;			
			if ((_viewR.lx() > 5 * _extim->lx()) && (_viewR.ly() > 5 * _extim->ly())) return false;
			return true;
			}

		bool _canSeeImage(const fBox2 & R)
			{
			if (R.max[0] <= 0.0) return false;
			if (R.min[0] >= (double)(_extim->lx())) return false;
			if (R.max[1] <= 0.0) return false;
			if (R.min[1] >= (double)(_extim->ly())) return false;
			return true;
			}

		iVec2 _viewToImage(iVec2 pos)
			{
			int x = (int)(_viewR.min[0] + (_viewR.lx() * ((double)pos.X() + 0.5)) / (_im.lx()) - 0.5);
			int y = (int)(_viewR.min[1] + (_viewR.ly() * ((double)pos.Y() + 0.5)) / (_im.ly()) - 0.5);
			return iVec2(x, y);
			}

		fVec2 _viewToImagef(iVec2 pos)
			{
			double x = _viewR.min[0] + (_viewR.lx() * ((double)pos.X() + 0.5)) / (_im.lx()) ;
			double y = _viewR.min[1] + (_viewR.ly() * ((double)pos.Y() + 0.5)) / (_im.ly()) ;
			return fVec2(x, y);
			}

protected:




		/* fltk handle method */
		virtual int handle(int e) override
			{

			if ((_extim == nullptr) || (_extim->isEmpty()))
				{ // no interaction when image not set. 
				return internals_graphics::ImageWidget::handle(e);
				}

			switch (e)
			{

			case FL_LEAVE: 
				{  
				_saveMouse();
				return 1; 
				}

			case FL_ENTER: 
				{ 
				_saveMouse();
				take_focus(); 
				return 1; 
				}

			case FL_FOCUS: return 1;

			case FL_UNFOCUS: return 1;
			
			case FL_MOVE:
				{
				_saveMouse();
				if (!_isIn(_current_mouse)) { return 1; } else { take_focus(); }
				return 1;
				}

			case FL_PUSH:
				{
				take_focus();
				_saveMouse();
				if (!_isIn(_current_mouse)) { return 1; }

				int button = Fl::event_button();
				if (button == FL_LEFT_MOUSE)
					{ // start selection
					if (_select_allowed)
						{
						_select_on = true;
						_mouse_sel1 = _current_mouse;
						_pos_sel1 = _viewToImage(_mouse_sel1);
						redrawNow();
						}
					return 1;
					}

				return 1;
				}

			case FL_RELEASE:
				{
				_saveMouse();
				int button = Fl::event_button();
				if (button == FL_LEFT_MOUSE)
					{ 
					if (_select_allowed)
						{
						if (_mouse_sel1 == _current_mouse)
							{ // mouse did not move, remove selection
							_selectR = iBox2(); 
							}
						_select_on = false;
						}
					redrawNow();
					return 1;
					}
				return 1;
				}

			case FL_DRAG:
				{
				_saveMouse();

				if (_select_on)
					{
					auto pos_sel2 = _viewToImage(_current_mouse);
					_selectR = iBox2(_pos_sel1, pos_sel2, true);
					redrawNow();
					return 1;
					}

				return 1;
				}

			case FL_MOUSEWHEEL:
				{
				//_saveMouse();
				take_focus();
				if ((_isIn(_current_mouse)) && (_move_allowed))
					{					
					int d = Fl::event_dy();
					if ((d < 0) && (_canZoomIn()))
						{
						fBox2 R = mtools::zoomIn(_viewR, _viewToImagef(_current_mouse));
						if (_canSeeImage(R)) _viewR = R;
						_select_on = false;
						redrawNow();
						return 1; 
						}
					if ((d > 0) && (_canZoomOut()))
						{
						fBox2 R = mtools::zoomOut(_viewR, _viewToImagef(_current_mouse));
						if (_canSeeImage(R)) _viewR = R;
						_select_on = false;
						redrawNow();
						return 1;
						}
					}
				return 1;
				}

			case FL_KEYDOWN:
				{
				_select_on = false;
				take_focus();
				int key = Fl::event_key();
				switch (key)
					{
					case FL_Delete:
					case FL_BackSpace:
						if (_select_allowed)
							{
							_selectR = iBox2();
							redrawNow();
							}
						return 1;

					case FL_Escape:
						if (_move_allowed)
							{
							setDefaultRange();
							redrawNow();
							}
						return 1;

					case FL_Page_Up:
						if ((_move_allowed)&&(_canZoomIn()))
							{
							fBox2 R = mtools::zoomIn(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Page_Down:
						if ((_move_allowed)&&(_canZoomOut()))
							{
							fBox2 R = mtools::zoomOut(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Left:
						if (_move_allowed)
							{
							fBox2 R = mtools::left(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Right:
						if (_move_allowed)
							{
							fBox2 R = mtools::right(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Down:
						if (_move_allowed)
							{
							fBox2 R = mtools::up(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Up:
						if (_move_allowed)
							{
							fBox2 R = mtools::down(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;
					}
				// key not handled
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

			if (_select_on)
				{
				if (_mouse_sel1 != _current_mouse)
					{
					draw_rect(iBox2(_mouse_sel1, _current_mouse, true));
					}
				}

			}


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
		allowUserMove(true);
		allowUserSelection(true);
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




double ss(double x)
	{
	if (x <= 1) return x;
	if (x > 1) return 2 - x;

	return (2 * sin(x)) / (x * x + 1);
	}


int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	{
		Plotter2D plotter;
		auto P = makePlot2DFun(ss);
		plotter[P];
		plotter.autorangeXY();
		plotter.plot();
	}



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
