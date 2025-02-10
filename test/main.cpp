#include <mtools/mtools.hpp>
using namespace mtools;




double f(double x) { return x * x;  }








int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode
	
	auto cons = new Console; 


	//Image im(800, 800);
	//im.clear(RGBc::c_White);
	//tgx::Image<tgx::RGB32> im2(im);
	//im2.clear(tgx::RGB32_Gray);
	//im2.drawThickCircleArcAA({ 400, 400 }, 200,  350, 45, 10, tgx::RGB32_Black);
	//im.draw_thick_circle_arc({ 400,400 }, 200, 190, 200, 4, RGBc::c_Blue);

	auto C = makeFigureCanvas(2); 
	C(mtools::Figure::ThickCircleArc({ 100,100 }, 100, 120, 45, 1, RGBc::c_Red), 0);


	Plotter2D plotter;
	auto P = makePlot2DFigure(C);
	plotter[P];
	plotter.autorangeXY();
	plotter.plot();



	cout << "Hello World\n";
	cout.getKey();
	return 0; 

}


























