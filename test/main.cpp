
#include <mtools/mtools.hpp>

using namespace mtools;




double f(double x)
{
	return sin(x);
}

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	auto P = makePlot2DFun(f, "sin");

	Plotter2D plotter;
	plotter[P]; 
	plotter.autorangeXY(); 

	plotter.plot(); 



}

