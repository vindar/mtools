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

	auto canvas = makeFigureCanvas();

	Figure::Circle C({ 0.0,0.0 }, 50, RGBc::c_Black, RGBc::c_Red);
	C.setAnchorGlobal({ 100.0, 200.0 });

	C.setClipGlobal(fBox2(100.0,150.0, 200.0, 400.0));
	C.setClipLocal(fBox2(-30.0, 30.0, -20.0, 45.0));

	Figure::Rectangle RR(fBox2(100.0, 130.0, 200.0, 245.0), RGBc::c_Black, RGBc::c_Green.getMultOpacity(0.3));

	canvas(RR, 0);
	canvas(C, 0);

	auto PC = makePlot2DFigure(canvas);

	canvas.saveSVG("test.svg");

	Plotter2D plotter;
	plotter[PC];
	plotter.autorangeXY();
	plotter.plot(); 
	}

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	test();


}
