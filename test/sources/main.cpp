
#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;




int main(int argc, char *argv[])
	{
	MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc, argv, true); // parse the command line, interactive mode

	Image imm("lenna.jpg");
	Plotter2D plotter;
	auto P = makePlot2DImage(imm);
	plotter[P];
	plotter.autorangeXY();
	plotter.plot();


	return 0;
	}

/* end of file main.cpp */
















