
#include "mtools/mtools.hpp"
using namespace mtools;



double f(double x) { return sin(x); }


int main(int argc, char *argv[])
	{
	MTOOLS_SWAP_THREADS(argc, argv);


	Plotter2D plotter;

	std::cout << "HELLO WORLD3\n";

	auto P = makePlot2DFun(f, "sin");

	std::cout << "HELLO WORLD4\n";

	plotter[P];

	std::cout << "HELLO WORLD5\n";

	plotter.plot();

	std::cout << "HELLO WORLD6\n";

	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	return 0;
	}



