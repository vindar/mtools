#include <mtools/mtools.hpp>
using namespace mtools;




double f(double x) { return x * x;  }




MT2004_64 gen;



double ff(mtools::fVec2 V) 
	{ 
	return (V.X()* V.X()* V.X() + (V.Y() * V.Y()));
	}


int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode
	

	auto V = mtools::PoissonPointProcess_fast(gen, ff, fBox2(-5, 7, -15, 10));
	//auto V = mtools::PoissonPointProcess(gen, ff, fBox2(-5, 7, -15, 10), -1, 1000);


	auto C = makeFigureCanvas(2);
	for(auto &v : V) 
        {
        C(mtools::Figure::CircleDot(v, 0.1, RGBc::c_Red), 0);
        }
	Plotter2D plotter;
	auto P = makePlot2DFigure(C);
	plotter[P];
	plotter.autorangeXY();
	plotter.plot();


	cout << V; 
	cout.getKey();


	return 0; 

}


























