#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;









using namespace mtools;

Image im;



typedef char * pchar;



void testImg()
	{


	int64 x = 0;
	int64 y = 400;


	Font F("SUI3.bff");

	Font F2(F,102);


	std::string txt("The brown fox jumps over the lazy dog\nYEAH!!!!\nThat's nice! Here is a number: 1.2345678999e-678");

	im.load_png("lenna.png");
	
	im.clear(RGBc::c_White);

	F.drawBackground(im, x , y, txt, F.TOPLEFT, RGBc::c_Black);
	F.drawText(im, x, y, txt, F.TOPLEFT, RGBc::c_White);

	F2.drawBackground(im, x, y + 100, txt, F.TOPLEFT, RGBc::c_Black);
	F2.drawText(im, x, y + 100, txt, F2.TOPLEFT, RGBc::c_White);


	for (int i = 0; i < im.lx(); i++) { im.setPixel(i, y, RGBc::c_Red); }
	for (int j = 0; j < im.ly(); j++) { im.setPixel(x, j, RGBc::c_Red); }



	mtools::Plotter2D plotter;
	auto P1 = mtools::makePlot2DImage(im, 4, "Img");
	plotter[P1];
	P1.autorangeXY();
	plotter.plot();
	}




int main(int argc, char *argv[])
	{
	MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc, argv, true); // parse the command line, interactive mode



	testImg();


	cout << "Hello World\n";
	cout.getKey();

	return 0;
	}

/* end of file main.cpp */

