#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;



Image im;



typedef char * pchar;








void testImg()
	{




	Font F = defaultFont(18);

	FontFamily FF;

	FF.insertFont(F);

	Font G; G.createFrom(F, 30);

	FF.insertFont(G);


	//im.load_png("lenna.png");
	im.resizeRaw(800, 2000);
	im.clear(RGBc::c_Black);


	std::string ss("Ceci est un test");


	int64 x = 100;
	int64 y = 10;


	for (int i = 2; i < 50; i+=2)
		{
		Font G = FF(i, MTOOLS_NATIVE_FONT_BELOW);			
		G.drawText(im, x, y, ss, G.TOPLEFT, RGBc::c_White);
		y += i;
		}



	// 	Font F = defaultFont(18);
	// 308450158
	// 308450158
	/*

	std::string txt("The brown fox jumps over the lazy dog\nYEAH!!!!\nThat's nice! Here is a number: 1.2345678999e-678");


	F.drawBackground(im, x , y, txt, F.TOPLEFT, RGBc::c_Black);
	F.drawText(im, x, y, txt, F.TOPLEFT, RGBc::c_White);

	F2.drawBackground(im, x, y + 100, txt, F.TOPLEFT, RGBc::c_Black);
	F2.drawText(im, x, y + 100, txt, F2.TOPLEFT, RGBc::c_White);


	for (int i = 0; i < im.lx(); i++) { im.setPixel(i, y, RGBc::c_Red); }
	for (int j = 0; j < im.ly(); j++) { im.setPixel(x, j, RGBc::c_Red); }



	OCPPArchive ar("_dataFont");
	ar & F;
	cout << ar.get();
	*/


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



	try {
		testImg();
		}
	catch (const char * e)
		{
		cout << e << "\n";;
		}

	cout << "Hello World\n";
	cout.getKey();

	return 0;
	}

/* end of file main.cpp */

