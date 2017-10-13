#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;



Image im;



typedef char * pchar;








void create()
	{

	FontFamily FF;

	FF.insertFont(Font("Open Sans- 8.bff",8));
	FF.insertFont(Font("Open Sans- 9.bff", 9));
	FF.insertFont(Font("Open Sans- 10.bff", 10));
	FF.insertFont(Font("Open Sans- 11.bff", 11));
	FF.insertFont(Font("Open Sans- 12.bff", 12));
	FF.insertFont(Font("Open Sans- 13.bff", 13));
	FF.insertFont(Font("Open Sans- 14.bff", 14));
	FF.insertFont(Font("Open Sans- 16.bff", 16));
	FF.insertFont(Font("Open Sans- 18.bff", 18));
	FF.insertFont(Font("Open Sans- 20.bff", 20));
	FF.insertFont(Font("Open Sans- 22.bff", 22));
	FF.insertFont(Font("Open Sans- 24.bff", 24));
	FF.insertFont(Font("Open Sans- 26.bff", 26));
	FF.insertFont(Font("Open Sans- 28.bff", 28));
	FF.insertFont(Font("Open Sans- 32.bff", 32));
	FF.insertFont(Font("Open Sans- 36.bff", 36));
	FF.insertFont(Font("Open Sans- 40.bff", 40));
	FF.insertFont(Font("Open Sans- 48.bff", 48));
	FF.insertFont(Font("Open Sans- 64.bff", 64));
	FF.insertFont(Font("Open Sans- 72.bff", 72));
	FF.insertFont(Font("Open Sans- 128.bff", 128));
	FF.insertFont(Font("Open Sans- 256.bff", 256));

	OCPPArchive ar("Open_Sans_FontFamily");
	ar & FF;

	mtools::saveStringToFile("OpenSans.txt", ar.get());

	}


void testImg()
	{

	//im.load_png("lenna.png");
	im.resizeRaw(800, 16000);
	im.clear(RGBc::c_Black);


	std::string ss("Ceci est un test\123.99e-788");

	int64 x = 400;
	int64 y = 10;


	for (int i = 2; i < 25; i+=2)
		{

		im.draw_text(x, y, ss, MTOOLS_TEXT_TOPLEFT, RGBc::c_Green,i);

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

	/*
	create();
	cout << "done!\n";
	cout.getKey();
	return 0;
	*/
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

