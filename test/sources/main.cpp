#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;



Image im;



typedef char * pchar;




/** Used by CSLineClip to compute the region where the point lies **/ 
int CSLineClipCode(const iVec2 & P, const iBox2 & B)
	{
	int c = 0;
	const int64 x = P.X();
	const int64 y = P.Y();
	if (x < B.min[0]) c |= 1;
	if (x > B.max[0]) c |= 2;
	if (y < B.min[1]) c |= 4;
	if (y > B.max[1]) c |= 8;
	return c;
	}


/**
 * Cohen-Sutherland Line clipping algorithm. 
 *
 * @param [in,out]	P1	The first point.
 * @param [in,out]	P2	The second point.
 * @param	B		  	The rectangle to clip into.
 *
 * @return	true if a line should be drawn and false if it should be discarded.
 **/
bool CSLineClip(iVec2 & P1, iVec2 & P2, const iBox2 & B)
	{
	int c1 = CSLineClipCode(P1, B);
	int c2 = CSLineClipCode(P2, B);
	while (1)
		{
		const double m = ((double)(P2.Y() - P1.Y())) / (P2.X() - P1.X());
		if ((c1 == 0) && (c2 == 0)) { return true; } // both point inside		
		if ((c1 & c2) != 0) { return false; } //AND of both codes != 0.Line is outside. Reject line
		int64 x, y;
		int temp = (c1 == 0) ? c2 : c1; //Decide if point1 is inside, if not, calculate intersection		
		if (temp & 8)
			{ //Line clips top edge
			x = P1.X() + (int64)round((B.max[1] - P1.Y())/m);
			y = B.max[1];
			}
		else if (temp & 4)
			{ 	//Line clips bottom edge
			x = P1.X() + (int64)round((B.min[1] - P1.Y()) / m);
			y = B.min[1];
			}
		else if (temp & 1)
			{ 	//Line clips left edge
			x = B.min[0];
			y = P1.Y() + (int64)round(m*(B.min[0] - P1.X()));
			}
		else if (temp & 2)
			{ 	//Line clips right edge
			x = B.max[0];
			y = P1.Y() + (int64)round(m*(B.max[0] - P1.X()));
			}
		if (temp == c1) //Check which point we had selected earlier as temp, and replace its co-ordinates
			{
			P1.X() = x; P1.Y() = y;
			c1 = CSLineClipCode(P1, B);
			}
		else
			{
			P2.X() = x; P2.Y() = y;
			c2 = CSLineClipCode(P2, B);
			}
		}
	}



/**
 * Draw a line using Bresenham's algorithm. 
 * Optimized. No bound check. 
 * Use blending. 
 **/
void LineBresenham_blend(Image & im, int64 x1, int64 y1, int64 x2, int64 y2, RGBc color)
	{
	int64 dy = y2 - y1;
	int64 dx = x2 - x1;
	int64 stepx, stepy;
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1; dx <<= 1;
	im(x1, y1).blend(color);
	if (dx > dy)
		{
		int64 fraction = dy - (dx >> 1);
		while (x1 != x2)
			{
			if (fraction >= 0) { y1 += stepy; fraction -= dx; }
			x1 += stepx; fraction += dy;
			im(x1, y1).blend(color);
			}
		}
	else 
		{
		int64 fraction = dx - (dy >> 1);
		while (y1 != y2) 
			{
			if (fraction >= 0) { x1 += stepx; fraction -= dy; }
			y1 += stepy; fraction += dx;
			im(x1, y1).blend(color);
			}
		}
	}

/**
* Draw a line using Bresenham's algorithm.
* Optimized. No bound check.
* Use blending.
**/
void LineBresenham(Image & im, int64 x1, int64 y1, int64 x2, int64 y2, RGBc color)
	{
	int64 dy = y2 - y1;
	int64 dx = x2 - x1;
	int64 stepx, stepy;
	if (dy < 0) { dy = -dy;  stepy = -1; }
	else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; }
	else { stepx = 1; }
	dy <<= 1; dx <<= 1;
	im(x1, y1) = color;
	if (dx > dy)
		{
		int64 fraction = dy - (dx >> 1);
		while (x1 != x2)
			{
			if (fraction >= 0) { y1 += stepy; fraction -= dx; }
			x1 += stepx; fraction += dy;
			im(x1, y1) = color;
			}
		}
	else
		{
		int64 fraction = dx - (dy >> 1);
		while (y1 != y2)
			{
			if (fraction >= 0) { x1 += stepx; fraction -= dy; }
			y1 += stepy; fraction += dx;
			im(x1, y1) = color;
			}
		}
	}



/**
* Draw an antialiased line using Bresenham's algorithm.
* No bound check.
**/
void LineBresenhamAA(Image & im, int64 x0, int64 y0, int64 x1, int64 y1, RGBc color)
	{
	int64 sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, x2;
	int64 dx = abs(x1 - x0), dy = abs(y1 - y0), err = dx*dx + dy*dy;
	int64 e2 = err == 0 ? 1 : (int64)(0xffff7fl / sqrt(err));
	dx *= e2; dy *= e2; err = dx - dy;
	int64 op = mtools::convertAlpha_0xFF_to_0x100(color.comp.A);
	if (op == 256)
		{
		for (; ; )
			{
			color.comp.A = (uint8)((255 - (abs(err - dx + dy) >> 16)));
			im(x0, y0) = color;
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx) {
				if (x0 == x1) break;
				if (e2 + dy < 0xff0000l)
					{
					color.comp.A = (uint8)((255 - (int)((e2 + dy) >> 16)));
					im(x0, y0 + sy) = color;
					}
				err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy) {
				if (y0 == y1) break;
				if (dx - e2 < 0xff0000l)
					{
					color.comp.A = (uint8)((255 - (int)((dx - e2) >> 16)));
					im(x2 + sx, y0) = color;
					}
				err += dx; y0 += sy;
				}
			}
		return;
		}
	else
		{
		for (; ; )
			{
			color.comp.A = (uint8)(((255 - (abs(err - dx + dy) >> 16))*op) >> 8);
			im(x0, y0) = color;
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx) {
				if (x0 == x1) break;
				if (e2 + dy < 0xff0000l)
					{
					color.comp.A = (uint8)(((255 - (int)((e2 + dy) >> 16))*op) >> 8);
					im(x0, y0 + sy) = color;
					}
				err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy) {
				if (y0 == y1) break;
				if (dx - e2 < 0xff0000l)
					{
					color.comp.A = (uint8)(((255 - (int)((dx - e2) >> 16))*op) >> 8);
					im(x2 + sx, y0) = color;
					}
				err += dx; y0 += sy;
				}
			}
		return;
		}
	}



/**
* Draw an antialiased line using Bresenham's algorithm.
* No bound check.
* Use blending.
**/
void LineBresenhamAA_blend(Image & im, int64 x0, int64 y0, int64 x1, int64 y1, RGBc color)
	{
	int64 sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, x2;
	int64 dx = abs(x1 - x0), dy = abs(y1 - y0), err = dx*dx + dy*dy;
	int64 e2 = err == 0 ? 1 : (int64)(0xffff7fl / sqrt(err));
	dx *= e2; dy *= e2; err = dx - dy;
	int64 op = mtools::convertAlpha_0xFF_to_0x100(color.comp.A);
	if (op == 256)
		{
		for (; ; )
			{
			color.comp.A = (uint8)((255 - (abs(err - dx + dy) >> 16)));
			im(x0, y0).blend(color);
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx) {
				if (x0 == x1) break;
				if (e2 + dy < 0xff0000l)
					{
					color.comp.A = (uint8)((255 - (int)((e2 + dy) >> 16)));
					im(x0, y0 + sy).blend(color);
					}
				err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy) {
				if (y0 == y1) break;
				if (dx - e2 < 0xff0000l)
					{
					color.comp.A = (uint8)((255 - (int)((dx - e2) >> 16)));
					im(x2 + sx, y0).blend(color);
					}
				err += dx; y0 += sy;
				}
			}
		return;
		}
	else
		{
		for (; ; )
			{
			color.comp.A = (uint8)(((255 - (abs(err - dx + dy) >> 16))*op) >> 8);
			im(x0, y0).blend(color);
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx) {
				if (x0 == x1) break;
				if (e2 + dy < 0xff0000l)
					{
					color.comp.A = (uint8)(((255 - (int)((e2 + dy) >> 16))*op) >> 8);
					im(x0, y0 + sy).blend(color);
					}
				err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy) {
				if (y0 == y1) break;
				if (dx - e2 < 0xff0000l)
					{
					color.comp.A = (uint8)(((255 - (int)((dx - e2) >> 16))*op) >> 8);
					im(x2 + sx, y0).blend(color);
					}
				err += dx; y0 += sy;
				}
			}
		return;
		}
	}




/**
* Draw an tick antialiased line using Bresenham's algorithm.
**/
void TickLineBresenhamAA(Image & im, int64 x0, int64 y0, int64 x1, int64 y1, float wd, RGBc color)
	{
	int64 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int64 dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int64 err = dx - dy, e2, x2, y2;
	float ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);
	int64 op = mtools::convertAlpha_0xFF_to_0x100(color.comp.A);
	if (op == 256)
		{
		for (wd = (wd + 1) / 2; ; )
			{
			color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(err - dx + dy) / ed - wd + 1)));
			im.setPixel(x0, y0, color);
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx)
				{
				for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
					{
					color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1)));
					im.setPixel(x0, y2 += sy, color);
					}
				if (x0 == x1) break;
				e2 = err; err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy)
				{
				for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
					{
					color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1)));
					im.setPixel(x2 += sx, y0, color);
					}
				if (y0 == y1) break;
				err += dx; y0 += sy;
				}
			}
		return;
		}
	else
		{
		for (wd = (wd + 1) / 2; ; )
			{
			color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(err - dx + dy) / ed - wd + 1)))) * op) >> 8);
			im.setPixel(x0, y0, color);
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx)
				{
				for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
					{
					color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))))*op) >> 8);
					im.setPixel(x0, y2 += sy, color);
					}
				if (x0 == x1) break;
				e2 = err; err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy)
				{
				for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
					{
					color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))))*op) >> 8);
					im.setPixel(x2 += sx, y0, color);
					}
				if (y0 == y1) break;
				err += dx; y0 += sy;
				}
			}
		return;
		}
	}


/**
* Draw an tick antialiased line using Bresenham's algorithm.
* use blending.
**/
void TickLineBresenhamAA_blend(Image & im, int64 x0, int64 y0, int64 x1, int64 y1, float wd, RGBc color)
	{
	int64 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int64 dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int64 err = dx - dy, e2, x2, y2;
	float ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);
	int64 op = mtools::convertAlpha_0xFF_to_0x100(color.comp.A);
	if (op == 256)
		{
		for (wd = (wd + 1) / 2; ; )
			{
			color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(err - dx + dy) / ed - wd + 1)));
			im.blendPixel(x0, y0, color);
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx)
				{
				for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
					{
					color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1)));
					im.blendPixel(x0, y2 += sy, color);
					}
				if (x0 == x1) break;
				e2 = err; err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy)
				{
				for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
					{
					color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1)));
					im.blendPixel(x2 += sx, y0, color);
					}
				if (y0 == y1) break;
				err += dx; y0 += sy;
				}
			}
		return;
		}
	else
		{
		for (wd = (wd + 1) / 2; ; )
			{
			color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(err - dx + dy) / ed - wd + 1)))) * op) >> 8);
			im.blendPixel(x0, y0, color);
			e2 = err; x2 = x0;
			if (2 * e2 >= -dx)
				{
				for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
					{
					color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))))*op) >> 8);
					im.blendPixel(x0, y2 += sy, color);
					}
				if (x0 == x1) break;
				e2 = err; err -= dy; x0 += sx;
				}
			if (2 * e2 <= dy)
				{
				for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
					{
					color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))))*op) >> 8);
					im.blendPixel(x2 += sx, y0, color);
					}
				if (y0 == y1) break;
				err += dx; y0 += sy;
				}
			}
		return;
		}
	}







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








void tt(bool aa, bool blend, float tick, int N, std::vector<iVec2> & tabP1, std::vector<iVec2> & tabP2)
	{
	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	cout << "\n\n";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		im.draw_line(P1, P2, color, aa, blend, tick);
		}

	cout << "done in : " << mtools::Chronometer() << "\n";
	}



void testImg()
	{

	//im.load_png("lenna.png");
	im.resizeRaw(800, 600);
	im.clear(RGBc::c_White);

	int x = 400;
	int y = 300;


	iBox2 B(200, 503, 118, 451);
	im.draw_filled_rectangle(B, RGBc(220, 220, 220));

	MT2004_64 gen(1);


	int N = 2000000;
	std::vector<iVec2> tabP1, tabP2;
	for (int i = 0;i < N; i++)
		{
		tabP1.push_back({ Unif_int(-100, 900, gen), Unif_int(-100, 700, gen) });
		tabP2.push_back({ Unif_int(-20000, 20000, gen), Unif_int(-20000, 20000, gen) });
		}


	bool aa = false;
	bool blend = false;
	float tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	aa = false;
	blend = true;
	tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	
	aa = true;
	blend = false;
	tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	aa = true;
	blend = true;
	tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	aa = true;
	blend = false;
	tick = 3.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	aa = true;
	blend = true;
	tick = 3.0f;
	
	tt(aa, blend, tick, N, tabP1, tabP2);

	/*
	LineBresenham(im, x, y, x + 100, y, RGBc::c_Red);
	LineBresenham(im, x, y, x - 100, y, RGBc::c_Red);
	LineBresenham(im, x, y , x, y + 100, RGBc::c_Red);
	LineBresenham(im, x, y, x , y - 100, RGBc::c_Red);


	LineBresenham(im, x, y, x + 100, y + 100, RGBc::c_Red);
	LineBresenham(im, x, y, x - 100, y - 100, RGBc::c_Red);
	LineBresenham(im, x, y, x - 100, y + 100, RGBc::c_Red);
	LineBresenham(im, x, y, x + 100, y - 100, RGBc::c_Red);


	LineBresenham(im, x, y, x + 100, y + 50, RGBc::c_Red);
	LineBresenham(im, x, y, x - 100, y - 50, RGBc::c_Red);
	LineBresenham(im, x, y, x - 100, y + 50, RGBc::c_Red);
	LineBresenham(im, x, y, x + 100, y - 50, RGBc::c_Red);

	LineBresenham(im, x, y, x + 50, y + 100, RGBc::c_Red);
	LineBresenham(im, x, y, x - 50, y - 100, RGBc::c_Red);
	LineBresenham(im, x, y, x - 50, y + 100, RGBc::c_Red);
	LineBresenham(im, x, y, x + 50, y - 100, RGBc::c_Red);


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

