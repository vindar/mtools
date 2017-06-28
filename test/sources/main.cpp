#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;






RGBc color(int64 x, int64 y)
	{
	if (x*x + y*y < 10000)
		{
		uint32 c = 1;
		for (int i = 0; i < 100; i++) { c += (x*x) % 100 + (y*y) % 200; }
		c = c % 255;
		return RGBc(c,c,c);
		}
	return RGBc::c_TransparentWhite;
	}

MT2004_64 gen(5679); // RNG with 2M vertices.
int main(int argc, char *argv[])
    {


	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);


	Plotter2D plotter;

	auto P1 = makePlot2DLattice(color, "Lattice");
	auto P2 = makePlot2DPixel(color, 3, "Pixel");

	plotter[P1][P2];
	plotter.plot();
	return 0;



    }

/* end of file main.cpp */






