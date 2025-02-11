#include <mtools/mtools.hpp>
using namespace mtools;




double f(double x) { return x * x;  }








int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode
	
	auto cons = new Console; 


	auto C = makeFigureCanvas(2); 
	C(mtools::Figure::Circle({ 100,100 }, 60, RGBc::c_Red), 0);
	C(mtools::Figure::ThickCircle({ 100,100 }, 120, 3, true, RGBc::c_Blue), 0);
	C(mtools::Figure::ThickCircleSector({ 100,100 }, 59, 45, 100, 5, RGBc::c_Orange, RGBc::c_Green), 0);
	C(mtools::Figure::ThickCircleArc({ 100,100 }, 100, 120, 45, 18, RGBc::c_Purple), 0);

	C.saveSVG("res.svg");

	Plotter2D plotter;
	auto P = makePlot2DFigure(C);
	plotter[P];
	plotter.autorangeXY();
	plotter.plot();



	cout << "Hello World\n";
	cout.getKey();
	return 0; 

}


























