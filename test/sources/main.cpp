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


void LineBresenham(Image & im, int64 x1, int64 y1, int64 x2, int64 y2, RGBc color)
	{
	int64 dy = y2 - y1;
	int64 dx = x2 - x1;
	int64 stepx, stepy;
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1; dx <<= 1;
	im(x1, y1).blend(color); // start point
	if (dx > dy)
		{
		int64 fraction = dy - (dx >> 1) - ((y2 > y1) ? 1 : 0);
		while (x1 != x2)
			{
			if (fraction >= 0) { y1 += stepy; fraction -= dx; }
			x1 += stepx; fraction += dy;
			im(x1, y1).blend(color);
			}
		}
	else 
		{
		int64 fraction = dx - (dy >> 1) - ((x2 > x1) ? 1 : 0);
		while (y1 != y2) 
			{
			if (fraction >= 0) { x1 += stepx; fraction -= dy; }
			y1 += stepy; fraction += dx;
			im(x1, y1).blend(color);
			}
		}
	}




/* 
*/ 


/**
* THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc)
* Copyright 2001-2, By Po-Han Lin
* c.f. http://www.edepot.com
*
* A little faster than Bressenham (10% increase speed).
**/
void LineEFLA(Image & im, int64 x, int64 y, int64 x2, int64 y2, RGBc color)
	{
	bool yLonger = false;
	int64 shortLen = y2 - y;
	int64 longLen = x2 - x;
	if (abs(shortLen)>abs(longLen)) { int64 swap = shortLen; shortLen = longLen; longLen = swap; yLonger = true; }
	int decInc;
	if (longLen == 0) decInc = 0; else decInc = (shortLen << 16) / longLen;
	if (yLonger)
		{
		if (longLen>0)
			{
			longLen += y;
			for (int j = 0x8000 + (x << 16);y <= longLen;++y) { im(j >> 16, y) = color; j += decInc; }
			return;
			}
		longLen += y;
		for (int j = 0x8000 + (x << 16);y >= longLen;--y) { im(j >> 16, y) = color; j -= decInc; }
		return;
		}
	if (longLen>0)
		{
		longLen += x;
		for (int j = 0x8000 + (y << 16);x <= longLen;++x) { im(x, j >> 16) = color; j += decInc; }
		return;
		}
	longLen += x;
	for (int j = 0x8000 + (y << 16);x >= longLen;--x) { im(x, j >> 16) = color; j -= decInc; }
	}


/**
* THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc)
* Copyright 2001-2, By Po-Han Lin
* c.f. http://www.edepot.com
*
* A little faster than Bressenham (10% increase speed).
**/
void LineEFLA_blend(Image & im, int64 x, int64 y, int64 x2, int64 y2, RGBc color)
	{
	bool yLonger = false;
	int64 shortLen = y2 - y;
	int64 longLen = x2 - x;
	if (abs(shortLen)>abs(longLen)) { int64 swap = shortLen; shortLen = longLen; longLen = swap; yLonger = true; }
	int decInc;
	if (longLen == 0) decInc = 0; else decInc = (shortLen << 16) / longLen;
	if (yLonger)
		{
		if (longLen>0)
			{
			longLen += y;
			for (int j = 0x8000 + (x << 16);y <= longLen;++y) { im(j >> 16, y) = color; j += decInc; }
			return;
			}
		longLen += y;
		for (int j = 0x8000 + (x << 16);y >= longLen;--y) { im(j >> 16, y) = color; j -= decInc; }
		return;
		}
	if (longLen>0)
		{
		longLen += x;
		for (int j = 0x8000 + (y << 16);x <= longLen;++x) { im(x, j >> 16) = color; j += decInc; }
		return;
		}
	longLen += x;
	for (int j = 0x8000 + (y << 16);x >= longLen;--x) { im(x, j >> 16) = color; j -= decInc; }
	}



/**
* Draw a line using Bresenham's algorithm.
* Optimized. No bound check.
**/
/*
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
*/


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
		im.draw_line(P1, P2, color, true, blend, aa, tick);
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


	iBox2 B(0, 799, 0, 599);
	im.draw_filled_rectangle(B, RGBc(220, 220, 220),false);

	MT2004_64 gen(1);


	int N = 1000000;
	std::vector<iVec2> tabP1, tabP2;

	tabP1.push_back({ 0,0 }); tabP2.push_back({ 799,599 });
	tabP1.push_back({ 799,599 }); tabP2.push_back({ 0,0 });
	tabP1.push_back({ 799,0 }); tabP2.push_back({ 0,599 });
	tabP2.push_back({ 0,599 }); tabP1.push_back({ 799,0 });
	tabP1.push_back({ 0,0 }); tabP2.push_back({ 0,599 });
	tabP2.push_back({ 0,599 }); tabP1.push_back({ 0,0 });
	tabP1.push_back({ 799,0 }); tabP2.push_back({ 799,599 });
	tabP2.push_back({ 799,599 }); tabP1.push_back({ 799,0 });

	for (int i = 0;i < N; i++)
		{
		tabP1.push_back({ Unif_int(-100, 900, gen), Unif_int(-100, 700, gen) });
		tabP2.push_back({ Unif_int(-20000, 20000, gen), Unif_int(-20000, 20000, gen) });
		}


	bool aa = false;
	bool blend = false;
	float tick = 1.0f;





	
	Img<unsigned char> cim(im.lx(),im.ly(),1,4);
	im.clear(RGBc::c_White);

	/*
	{

	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	cout << "\n\nCIMG";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		cim.drawLine(P1, P2, color, 0.5);		
		}
		
	cout << "done in : " << mtools::Chronometer() << "\n";
	}
	*/





	
	{
	RGBc color = RGBc::c_Black;
	cout << "\n\nFUN BRESSENHAM";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		if (CSLineClip(P1, P2, B)) LineBresenham(im, P1.X(), P1.Y(), P2.X(), P2.Y(), color);
		}

	cout << "done in : " << mtools::Chronometer() << "\n";
	}
	


	{
	RGBc color = RGBc::c_Black;
	cout << "\n\nFUN AA";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		if (CSLineClip(P1, P2, B)) LineBresenhamAA(im, P1.X(), P1.Y(), P2.X(), P2.Y(), color);
		}

	cout << "done in : " << mtools::Chronometer() << "\n";
	}




	{
	RGBc color = RGBc::c_Red;
	cout << "\n\nEFLA";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		if (CSLineClip(P1, P2, B)) LineEFLA(im, P1.X(), P1.Y(), P2.X(), P2.Y(), color);
		}

	cout << "done in : " << mtools::Chronometer() << "\n";
	}
	



	{

	RGBc color = RGBc::c_Black;
	cout << "\n\nFUN";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		if (CSLineClip(P1, P2, B)) LineBresenhamAA(im, P1.X(), P1.Y(), P2.X(), P2.Y(), color);
		}

	cout << "done in : " << mtools::Chronometer() << "\n";
	}




	im.clear(RGBc::c_White);
	{
	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	blend = false;
	aa = false;
	cout << "\n\nGG1\n";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		im.draw_line(P1, P2, color, true, blend, aa);
		}
	cout << "done in : " << mtools::Chronometer() << "\n";
	}

	im.clear(RGBc::c_White);
	{
	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	blend = true;
	aa = false;
	cout << "\n\nGG2\n";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		im.draw_line(P1, P2, color, true, blend, aa);
		}
	cout << "done in : " << mtools::Chronometer() << "\n";
	}
	im.clear(RGBc::c_White);
	{
	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	blend = false;
	aa = true;
	cout << "\n\nGG3\n";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		im.draw_line(P1, P2, color, true, blend, aa);
		}
	cout << "done in : " << mtools::Chronometer() << "\n";
	}
	im.clear(RGBc::c_White);
	{
	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	blend = true;
	aa = true;
	cout << "\n\nGG4\n";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		im.draw_line(P1, P2, color, true, blend, aa);
		}
	cout << "done in : " << mtools::Chronometer() << "\n";
	}
	im.clear(RGBc::c_White);
	{
	RGBc color = RGBc::c_Black.getOpacity(0.1f);
	blend = false;
	aa = false;
	cout << "\n\nGG5\n";
	cout << "aa     = " << aa << "\n";
	cout << "blend  = " << blend << "\n";
	cout << "tick   = " << tick << "\n";
	mtools::Chronometer();
	for (int i = 0;i < N; i++)
		{
		iVec2 P1 = tabP1[i];
		iVec2 P2 = tabP2[i];
		im.draw_line(P1, P2, color);
		}
	cout << "done in : " << mtools::Chronometer() << "\n";
	}


	cout << "AAA\n";
	cout.getKey();


	cout << "AAA2\n";
	aa = false;
	blend = true;
	tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	
	cout << "AAA3\n";
	aa = true;
	blend = false;
	tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	cout << "AAA4\n";
	aa = true;
	blend = true;
	tick = 1.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	cout << "AAA5\n";
	aa = true;
	blend = false;
	tick = 3.0f;

	tt(aa, blend, tick, N, tabP1, tabP2);

	cout << "AAA6\n";
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







	void testtick()
	{

		im.resizeRaw(1100, 600);
		im.clear(RGBc::c_White);

		int64 xc = 300;
		int64 yc = 300;

		int64 xd = 800;
		int64 yd = 300;

		double R = 200;
		double r = 20;

		bool draw_p2 = true;
		bool blending = false;
		bool antialiased = false;
		float tick = 1.0f;
		float op = 0.6; 

		for (int a = 0; a < 360; a += 10)
			{
			int64 x1 = xc + r*cos(a*TWOPI / 360);
			int64 y1 = yc + r*sin(a*TWOPI / 360);
			int64 x2 = xc + R*cos(a*TWOPI / 360);
			int64 y2 = yc + R*sin(a*TWOPI / 360);
			im.setPixel({ x2,y2 }, RGBc::c_Red);
			im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);

			int64 x3 = xd + r*cos(a*TWOPI / 360);
			int64 y3 = yd + r*sin(a*TWOPI / 360);
			int64 x4 = xd + R*cos(a*TWOPI / 360);
			int64 y4 = yd + R*sin(a*TWOPI / 360);
			im.setPixel({ x3,y3 }, RGBc::c_Red);
			im.draw_line({ x4,y4 }, { x3,y3 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);

			}


		{
		int64 x1 = 10; int64 y1 = 10;
		int64 x2 = 100; int64 y2 = 10;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}


		{
		int64 x1 = 100; int64 y1 = 20;
		int64 x2 = 10; int64 y2 = 20;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}


		{
		int64 x1 = 10; int64 y1 = 30;
		int64 x2 = 10; int64 y2 = 100;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}

		{
		int64 x1 = 30; int64 y1 = 100;
		int64 x2 = 30; int64 y2 = 30;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}


		{
		int64 x1 = 40; int64 y1 = 40;
		int64 x2 = 80; int64 y2 = 80;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}

		{
		int64 x1 = 90; int64 y1 = 40;
		int64 x2 = 50; int64 y2 = 80;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}

		{
		int64 x1 = 5; int64 y1 = 5;
		int64 x2 = 5; int64 y2 = 5;
		im.setPixel({ x2,y2 }, RGBc::c_Red);
		im.draw_line({ x1,y1 }, { x2,y2 }, RGBc::c_Black.getOpacity(op), draw_p2, blending, antialiased, tick);
		}


		mtools::Plotter2D plotter;
		auto P1 = mtools::makePlot2DImage(im, 4, "Img");
		plotter[P1];
		P1.autorangeXY();
		plotter.plot();
	}




void testriangle()
	{

	im.resizeRaw(1100, 800);
	im.clear(RGBc::c_White);


/*	iVec2 P1 = { 300, 500 };
	iVec2 P2 = { 500, 500 };*/
	iVec2 P1 = { 300, 100 };
	iVec2 P2 = { 500, 100 };

	iVec2 P3 = { 200, 150 };


//	im.draw_triangle(P3, P2, P1, RGBc::c_Red.getOpacity(0.5), true, false);
	im.draw_triangle(P3, P2, P1, RGBc::c_Red.getOpacity(1),false,false);
	//im._fill_interior_angle(P1, P2, P3, RGBc::c_Black, false);


	mtools::Plotter2D plotter;
	auto Plot1 = mtools::makePlot2DImage(im, 4, "Img");
	plotter[Plot1];
	Plot1.autorangeXY();
	plotter.plot();

	}




void test_b()
	{
	int LX = 80;
	int LY = 60;
	Image im(LX, LY);

	mtools::Plotter2D plotter;
	auto Plot1 = mtools::makePlot2DImage(im, 4, "Img");
	plotter[Plot1];
	Plot1.autorangeXY();
	plotter.startPlot();


	MT2004_64 gen(1);


	int e = 20;

	while (1)
		{
		im.clear(RGBc::c_White);
		iVec2 P1(Unif_int(0 - e, LX - 1 + e, gen), Unif_int(0 - e, LY - 1 + e, gen));
		iVec2 P2(Unif_int(0 - e, LX - 1 + e, gen), Unif_int(0 - e, LY - 1 + e, gen));
		iVec2 P3(Unif_int(0 - e, LX - 1 + e, gen), Unif_int(0 - e, LY - 1 + e, gen));

		im.draw_triangle(P1, P2, P3, RGBc::c_Green.getOpacity(0.5),true,false);
		im.draw_triangle_interior(P1, P2, P3, RGBc::c_Red.getOpacity(0.5), true);

		plotter.redraw();
		cout.getKey();
		}


	}





int main(int argc, char *argv[])
	{
	MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc, argv, true); // parse the command line, interactive mode

	test_b();
	/*
	//testtick();
	testriangle();
	cout << "Hello World\n";
	cout.getKey();
	return 0;
	*/

	/*
	create();
	cout << "done!\n";
	cout.getKey();
	return 0;*
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


















/*


template<typename tc> Img<T> & draw_triangle(const int x0, const int y0, const int x1, const int y1, const int x2, const int y2, const tc *const color, const float opacity = 1)

template<typename tc> Img<T>& draw_rectangle(const int x0, const int y0, const int x1, const int y1, const tc *const color, const float opacity = 1)

template<typename tc> Img<T>& draw_circle(const int x0, const int y0, int radius, const tc *const color, const float opacity = 1)

template<typename tc> Img<T>& draw_ellipse(const int x0, const int y0, const float r1, const float r2, const float angle, const tc *const color, const float opacity = 1)

template<typename tc> Img<T>& draw_ellipse(const int x0, const int y0, const float r1, const float r2, const float angle, const tc *const color, const float opacity, const unsigned int pattern)

const Img<T>& save(const char *const filename, const int number = -1, const unsigned int digits = 6) const

Img<T>& load(const char *const filename)





Img<T>& drawPoint(mtools::iVec2 P, mtools::RGBc color, const float opacity = 1);

Img<T>& drawPointCirclePen(mtools::iVec2 P, int rad, mtools::RGBc color, const float opacity = 1);

Img<T>& drawPointSquarePen(mtools::iVec2 P, int rad, mtools::RGBc color, const float opacity = 1)

Img<T>& drawLine(mtools::iVec2 P1, mtools::iVec2 P2, mtools::RGBc color, float opacity = 1)

Img<T>& drawHorizontalLine(int y, mtools::RGBc color, float opacity = 1)

Img<T>& drawVerticalLine(int x, mtools::RGBc color, float opacity = 1)

Img<T>& drawLineCirclePen(mtools::iVec2 P1, mtools::iVec2 P2, int rad, mtools::RGBc color, float opacity = 1)

Img<T>& drawLineSquarePen(mtools::iVec2 P1, mtools::iVec2 P2, int rad, mtools::RGBc color, float opacity = 1)

static const cimg_library::CImgList<floatT> & getFont(unsigned int font_height, bool variable_width = true)

static unsigned int computeFontSize(const std::string & text, mtools::iVec2 boxsize, bool variable_width = true, unsigned int minheight = 5, unsigned int maxheight = 256)

static mtools::iVec2 getTextDimensions(const std::string & text, unsigned int font_height, bool variable_width = true)

Img<T>& drawText(const std::string & text, mtools::iVec2 Pos, char xcentering, char ycentering, int fontsize, bool variable_width, mtools::RGBc color, double opacity = 1.0)

Img<T>& fBox2_drawText(const mtools::fBox2 & R, const std::string & text, mtools::fVec2 Pos, char xcentering, char ycentering, int fontsize, bool variable_width, mtools::RGBc color, double opacity = 1.0)

unsigned int fBox2_computeFontSize(const mtools::fBox2 & R, const std::string & text, mtools::fVec2 boxsize, bool variable_width = true, unsigned int minheight = 5, unsigned int maxheight = 256)

Img<T>& fBox2_floodFill(const mtools::fBox2 & R, mtools::fVec2 Pos, mtools::RGBc color, const float opacity = 1, const float sigma = 0, const bool is_high_connexity = false)

Img<T>& fBox2_drawPoint(const mtools::fBox2 & R, mtools::fVec2 P, mtools::RGBc color, const float opacity = 1)

Img<T>& fBox2_drawPointCirclePen(const mtools::fBox2 & R, mtools::fVec2 P, int rad, mtools::RGBc color, const float opacity = 1)

Img<T>& fBox2_drawPointSquarePen(const mtools::fBox2 & R, mtools::fVec2 P, int rad, mtools::RGBc color, const float opacity = 1)

Img<T>& fBox2_drawLine(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, mtools::RGBc color, float opacity = 1)

Img<T>& fBox2_drawHorizontalLine(const mtools::fBox2 & R, double y, mtools::RGBc color, float opacity = 1)

Img<T>& fBox2_drawVerticalLine(const mtools::fBox2 & R, double x, mtools::RGBc color, float opacity = 1)

Img<T>& fBox2_drawLineCirclePen(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, int rad, mtools::RGBc color, float opacity = 1)

Img<T>& fBox2_drawLineSquarePen(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, int rad, mtools::RGBc color, float opacity = 1)

Img<T>& fBox2_draw_spline(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 PA, mtools::fVec2 PB, mtools::fVec2 P2, mtools::RGBc color, float opacity = 1, float precision = 0.25)

Img<T>& fBox2_draw_triangle(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, mtools::fVec2 P3, mtools::RGBc color, float opacity = 1.0, bool filled = true)

Img<T>& fBox2_draw_rectangle(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, mtools::RGBc color, float opacity = 1.0, bool filled = true)

Img<T>& fBox2_draw_circle(const mtools::fBox2 & R, mtools::fVec2 C, double rad, mtools::RGBc color, float opacity = 1, bool filled = true)

Img<T>& fBox2_drawAxes(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 1.0)

Img<T>& fBox2_drawGrid(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Gray, float opacity = 0.5)

Img<T>& fBox2_drawCells(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Gray, float opacity = 0.5)

Img<T>& fBox2_drawGraduations(const mtools::fBox2 & R, float scaling = 1.0, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 0.7)

Img<T>& fBox2_drawNumbers(const mtools::fBox2 & R, float scaling = 1.0, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 0.7)


*/