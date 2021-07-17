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





	















char bb[10000000];

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


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


























