
#include "mtools/mtools.hpp"
using namespace mtools;



double f(double x) { return sin(x); }


int main(int argc, char *argv[])
	{


	Plotter2D plotter;

	auto P = makePlot2DFun(f, "sin");

	plotter[P];
	plotter.plot();

	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	return 0;
	}



