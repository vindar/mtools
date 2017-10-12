#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;



Image readBFF(const std::string & filename)
	{
	std::string bff = mtools::loadStringFromFile(filename);
	if (bff.size() == 0) { MTOOLS_ERROR("readBFF() : Cannot read file."); }
	if (bff.size() <= 276) { MTOOLS_ERROR("readBFF() : File too small."); }
	if (((uint8)bff[0] != 0xBF) || ((uint8)bff[1] != 0xF2)) { MTOOLS_ERROR("incorrect BFF tag."); }
	uint32 im_lx = *((uint32*)(bff.data() + 2));
	uint32 im_ly = *((uint32*)(bff.data() + 6));
	uint32 cell_lx = *((uint32*)(bff.data() + 10));
	uint32 cell_ly = *((uint32*)(bff.data() + 14));
	uint8 bpp = (uint8)bff[18];
	if (bpp != 32) { MTOOLS_ERROR("readBFF() : Error, image format must be 32bit."); }
	uint8 char_offset = (uint8)bff[19];
	uint8 width[256];
	for (int i = 0;i < 256;i++) { width[i] = (uint8)bff[20 + i]; }
	if (bff.size() < 276 + 4 * im_lx*im_ly) { MTOOLS_ERROR("readBFF() : Error, file to small to contain the whole image."); }
	Image im(im_lx, im_ly);
	for (uint32 j = 0; j < im_ly; j++)
		{
		for (uint32 i = 0; i < im_lx; i++)
			{
			uint8 A = bff[276 + 4 * (i + j*im_lx) + 3];
			//if (A >= 0xF0) { A = 0xFF; } else if (A <= 0x10) { A = 0x00; }
			im(i, j) = RGBc(0, 0, 0, A);
			}
		}

	for (uint32 j = 0; j < im_ly / 4; j++)
		{
		im(j, j) = RGBc(255, 0, 0, 255);
		}
	return im;
	}






using namespace mtools;

Image im;


RGBc fimg(mtools::int64 x, mtools::int64 y)
	{
	if ((x < im.lx()) && (x >= 0) && (y < im.ly()) && (y >= 0))
		{
		return im((int32)x, im.ly() - 1 - (int32)y);
		}
	return RGBc::c_Cyan;
	}


typedef char * pchar;



void testImg()
	{


	//im = readBFF("TNR6.bff");
	im = Image("lenna.png");
	//im.rescale(10, im.lx() / 10, im.ly() / 10);

	/*
	{
	cimg_library::CImg<unsigned char> im1("lenna.png");

	cimg_library::CImg<unsigned char>  im2(10000, 3000, 1, 3);

	Chronometer();
	for (int i = 0;i < 100; i++)
	{
	im2 = im1.get_resize(10000, 3000, 1, 3, 1);
	//im.blit_rescaled(im.UPSCALE_FASTEST, im.DOWNSCALE_FASTEST, im1, 0, 0, 10000, 3000);
	}

	cout << "Elapsed CIMG = " << Chronometer() << "\n";

	}
	*/


	/*
	Image im2(10000, 5000, RGBc::c_Blue);

	MT2004_64 gen(0);

	int N = 100;

	Chronometer();

	int64 x1 = 10000 * Unif(gen);
	int64 y1 = 5000 * Unif(gen);

	for (int i = 0;i < N; i++)
	{
	x1 = (x1 + 7 ) % 10000;
	y1 = (y1 + 13)  % 5000;

	//im2.blitSprite(im1, x1, y1);

	im2.clear(RGBc::c_Red);
	}
	*/




	/*
	Chronometer();
	Image im1("lenna.png");


	for (int hx = 0;hx < im1.lx(); hx++)
	{
	for (int hy = 0; hy < im1.ly(); hy++)
	{
	im1(hx, hy).comp.A = im1(hx, hy).comp.R;
	}
	}

	cout << "Image loaded = " << Chronometer() << "\n";

	int lx = im1.lx() * 2;
	int ly = im1.ly();

	im = Image(lx, ly);
	im.clear(RGBc::c_White);

	cout.getKey();

	int N = 0;

	Chronometer();
	for (int i = 0; i <= N; i++)
	{
	im.mask(im1, i * 5, 0, opacity(RGBc::c_Red,1));
	}

	cout << "done in " << Chronometer() << "\n";
	*/

	//PixelDrawer

	/*
	{
	Chronometer();
	cimg_library::CImg<unsigned char> im1("world.png");
	cout << "CIMG loaded = " << Chronometer() << "\n";
	cimg_library::CImg<unsigned char> im2;
	Chronometer();
	for (int i = 0;i < N; i++)
	{
	im2 = im1.get_resize(lx, ly, 1, 3, 2);
	}
	cout << "Elapsed CIMG = " << Chronometer() << "\n";

	for (int j = 0;j < im2.height(); j++)
	{
	for (int i = 0;i < im2.width(); i++)
	{
	im(lx + (lx - 1 - i), ly-1-j) = RGBc(im2(i, j, 0, 0), im2(i, j, 0, 1), im2(i, j, 0, 2));
	}
	}
	}
	*/
	/*
	Chronometer();
	for (int i = 0;i < N;i++)
	{
	im.blit_rescaled(0, 0, im1, lx + 10, 0, lx, ly);
	}
	cout << "done = " << Chronometer() << "\n";
	*/



	mtools::Plotter2D plotter;
	auto P1 = mtools::makePlot2DImage(im, 4, "Img");
	plotter[P1];
	plotter.plot();


	return;
	mtools::cout << "Hello !\n";
	mtools::cout.getKey();
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

