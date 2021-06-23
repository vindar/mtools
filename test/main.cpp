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









class ImageExtWidget : public internals_graphics::ImageWidget
	{



	virtual void setImage(const Image* im = nullptr) override;


	virtual void setImage(const ProgressImg* im = nullptr) override;




	};




mtools::internals_graphics::ImageWidget * IW;


Image im(800, 600);

class ImageCropper : public internals_graphics::ImageWidget
{

	

public:


	ImageCropper(int X, int Y, int W, int H, const char* name) : internals_graphics::ImageWidget(X, Y, W, H, name)
	{
		im.clear(RGBc::c_Yellow);
		im.draw_circle(iVec2{ 250,300 }, 200, RGBc::c_Red);
		im.draw_text({ 100,100 }, "Hello World\n", MTOOLS_TEXT_TOPLEFT, RGBc::c_Blue, 80);

		im = im.get_rotate270();

		setImage(&im);
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
			/*
		case FL_KEYDOWN:
			{
			take_focus();
			int key = Fl::event_key();
			if (key == FL_Page_Up)
				{
				_RM->zoomIn();
				redrawView();
				return 1;
				}

			if (key == FL_Page_Down)
				{
				_RM->zoomOut();
				redrawView();
				return 1;
				}
			if (key == FL_Left)
				{
				_RM->left();
				redrawView();
				return 1;
				}
			if (key == FL_Right)
				{
				_RM->right();
				redrawView();
				return 1;
				}
			if (key == FL_Up)
				{
				_RM->up();
				redrawView();
				return 1;
				}
			if (key == FL_Down)
				{
				_RM->down();
				redrawView();
				return 1;
				}
			return 1;
			}
		case FL_KEYUP:
			{
			return 1;
			}
			*/
		}



		return internals_graphics::ImageWidget::handle(e);
	}

	/* fltk method : draw the widget */
	virtual void draw() override
	{
		internals_graphics::ImageWidget::draw();
	}

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





int fltk_fun()
{
	IW = new ImageCropper(100, 100, 800, 600, "test");
	IW->show();
	return 0;
}

int showo()	
	{
	IW->show();
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
	//	mtools::IndirectFun<int> proxyshow(&showo);
	//	mtools::runInFltkThread(proxyshow);
		}
	cout << "done !\n\n";
	cout.getKey(); 
	return 0; 

}
