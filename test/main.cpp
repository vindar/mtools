
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:

		void draw_line_new(const iVec2 & P1, const iVec2 & P2, RGBc color, int32 penwidth = 0, bool antialiasing = true, bool blending = true);


	TestImage(int64 lx, int64 ly) : Image(lx, ly) 	
	{

//	draw_line()
	}
	


	void makefromfloat(fVec2 Pf1, fVec2 Pf2, _bdir & linedir, _bpos & linepos)
	{
		fVec2 vd = 1024.0 * (Pf2 - Pf1);
		iVec2 P1 = (iVec2)(Pf1 - vd);
		iVec2 P2 = (iVec2)(Pf2 + vd);

		_init_line(P1, P2, linedir, linepos);

		if (linedir.x_major)
			{

			}

		int64 dx = (int64)fdx; if (dx < 0) { dx = -dx;  linedir.stepx = -1; } else { linedir.stepx = 1; }
		int64 dy = (int64)fdy; if (dy < 0) { dy = -dy;  linedir.stepy = -1; } else { linedir.stepy = 1; }
		MTOOLS_ASSERT((dx >= 2) && (dy >= 2));
		if (dx > dy)
			{
			linedir.x_major = true;
			linedir.rat = (dy == 0) ? 0 : (dx / dy);
			}
		else
			{
			linedir.x_major = false;
			linedir.rat = (dx == 0) ? 0 : (dy / dx);
			}
		linepos.x = (int64)round(Pf1.X());
		linepos.y = (int64)round(Pf1.Y());


		(P1.X() - Pf1.X())*fdy/fdx
		
	}



	};




	MT2004_64 gen;

#define NN 1



	/* fast inverse squere root */





void testCE()
	{
	TestImage imA(1000, 1000);
	TestImage imB(1000, 1000);
	imA.clear(RGBc::c_White);
	imB.clear(RGBc::c_White);
	MT2004_64 gen(0);

	size_t N = 50000;

	
	int64 mult_rx = 10000; 
	int64 mult_ry = 10000;
	int64 mult_pos = 10000; 
	

	std::vector<iVec2> center(N, iVec2());
	std::vector<int64> rx(N, 1);
	std::vector<int64> ry(N, 1);

	for (size_t i = 0; i < N; i++)
		{
		center[i] = { -mult_pos + (int64)(2 * Unif(gen)*mult_pos), -mult_pos + (int64)(2 * Unif(gen)*mult_pos) };
		rx[i] = 1 + (int64)(Unif(gen)*mult_rx);
		ry[i] = 1 + (int64)(Unif(gen)*mult_ry);

		}



	cout << "Simulating A... ";
	Chronometer(); 
	for (size_t i = 0; i < N; i++)
		{
		imA.draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true,true,3);
		}
	auto resA = Chronometer();
	cout << "done in " << durationToString(resA, true) << "\n";


	cout << "Simulating B... ";
	Chronometer();
	for (size_t i = 0; i < N; i++)
		{
		imB.draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true, true,3);
		}
	auto resB = Chronometer();
	cout << "done in " << durationToString(resB, true) << "\n";


	auto PA = makePlot2DImage(imA, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
	auto PB = makePlot2DImage(imB, 1, "Image B");   // Encapsulate the image inside a 'plottable' object.	
	Plotter2D plotter;              // Create a plotter object
	plotter[PA][PB];                // Add the image to the list of objects to draw.  	
	plotter.autorangeXY();          // Set the plotter range to fit the image.
	plotter.plot();                 // start interactive display.

	}




#include "mtools\maths\bezier.hpp"






void testQuad(const fBox2 & B, BezierRationalQuadratic BQ, Image & im)
{
	RGBc color;
	auto C = B;
	C.enlarge(2);
	double res[12];
	int nb = BQ.intersect_rect(C, res);
	for (int i = (nb - 1); i > 0; i--)
		{
		res[i] = (res[i] - res[i - 1]) / (1.0 - res[i - 1]);
		}

	for (int i = 0; i < nb; i++)
		{
		auto sp = BQ.split(res[i]);
		BQ = sp.second;
		color = (C.isInside(sp.first(0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
		sp.first.normalize();
		im.draw_quad_bezier(sp.first.P0, sp.first.P2, sp.first.P1, sp.first.w1, color, true, true, true);
		}

	color = (C.isInside(BQ(0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
	BQ.normalize();
	im.draw_quad_bezier(BQ.P0, BQ.P2, BQ.P1, BQ.w1, color, true, true, true);
}




void draw(BezierQuadratic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_quad_bezier(sp.P0, sp.P2, sp.P1, 1, color, true, true, true, penwidth);
	}

void draw(BezierRationalQuadratic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_quad_bezier(sp.P0, sp.P2, sp.P1, sp.w1, color, true, true, true, penwidth);
	}

void draw(BezierCubic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_cubic_bezier(sp.P0, sp.P3, sp.P1, sp.P2, color, true, true, true, penwidth);
	}


template<typename BezierClass> void testBezier(fBox2 B, BezierClass curve, Image & im)
	{
	draw(curve, im, RGBc::c_Black, 1);
	B.enlarge(2);
	BezierClass subcurves[5];
	int tot = splitBezierInsideBox(B, curve, subcurves);
	for (int i = 0; i < tot; i++) { draw(subcurves[i], im, RGBc::c_Red, 2); }
	}







void testCF()
{
	size_t N = 50000;
	int64 LX = 1000;
	int64 LY = 1000;

	TestImage im(LX, LY);
	im.clear(RGBc::RGBc(240,240,200));
	MT2004_64 gen(0);


	while (1)
	{
		im.clear(RGBc::RGBc(240, 240, 200));

		iVec2 P0 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P1 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P2 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P3 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		double w = Unif(gen) * 10;

		cout << "P0 : " << P0 << "\n";
		cout << "P1 : " << P1 << "\n";
		cout << "P2 : " << P2 << "\n";
		cout << "P3 : " << P3 << "\n";
		cout << "w : " << w << "\n";

		BezierQuadratic curve(P0, P1,P2);
		//BezierRationalQuadratic curve(P0, 1.0, P1, w, P2,1.0);
		//BezierCubic curve(P0, P1, P2, P3);

		auto bb = curve.integerBoundingBox();
		im.draw_box(bb, RGBc::c_Gray, true);
		im.draw_dot(P0, RGBc::c_Green, true, 2);
		im.draw_dot(P1, RGBc::c_Green, true, 2);
		im.draw_dot(P2, RGBc::c_Green, true, 2);
		im.draw_dot(P3, RGBc::c_Green, true, 2);

		iBox2 TB{ 100,900,200,800 };
		im.draw_box(TB, RGBc::c_Yellow.getMultOpacity(0.5), true);
		im.draw_rectangle(TB, RGBc::c_Yellow, true);

		testBezier(TB, curve, im);
			


		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.
	}
}






void LineBresenham(iVec2 P1, iVec2 P2, Image & im, RGBc color)
{
	int64 x1 = P1.X(); 
	int64 y1 = P1.Y();
	int64 x2 = P2.X();
	int64 y2 = P2.Y();

	int64 dy = y2 - y1;
	int64 dx = x2 - x1;
	int64 stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;        // dy is now 2*dy
	dx <<= 1;        // dx is now 2*dx

	im.operator()(x1, y1).blend(color);


	if (stepx == 1) 
		{
		if (stepy == 1)
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1++;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1++;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1++;
						fraction -= dy;
					}
					y1++;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		else
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1--;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1++;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1++;
						fraction -= dy;
					}
					y1--;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		}
	else
		{
		if (stepy == 1)
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1++;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1--;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1--;
						fraction -= dy;
					}
					y1++;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		else
			{
			if (dx > dy)
			{
				int fraction = dy - (dx >> 1);  // same as 2*dy - dx
				while (x1 != x2)
				{
					if (fraction >= 0)
					{
						y1--;
						fraction -= dx;          // same as fraction -= 2*dx
					}
					x1--;
					fraction += dy;              // same as fraction -= 2*dy
					im.operator()(x1, y1).blend(color);
				}
			}
			else {
				int fraction = dx - (dy >> 1);
				while (y1 != y2) {
					if (fraction >= 0) {
						x1--;
						fraction -= dy;
					}
					y1--;
					fraction += dx;
					im.operator()(x1, y1).blend(color);
				}
			}
			}
		}

}


int main(int argc, char *argv[])
{

	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	TestImage im(800, 800);

	RGBc color = RGBc::c_Red.getMultOpacity(0.5);;
	RGBc colorfill = RGBc::c_Red.getMultOpacity(0.5);;



	iVec2 P1 = { 50,50 };
	iVec2 P2 = { 350,100 };
	iVec2 P3 = { 300,400 };
	iVec2 P4 = { 100,500 };

	int64 N = 1;// 0000;

	Chronometer();


	for (int i = 0; i < N; i++)
		{
		im.fill_triangle(P1, P2, P3, colorfill, true);
		//im.draw_triangle(P1, P2, P3, color, true, false, 0);

		LineBresenham(P1, P2, im, color);
		LineBresenham(P2, P3, im, color);
		LineBresenham(P3, P1, im, color);
	}
	cout << "1) done in " << mtools::durationToString(Chronometer(),true) << "\n";

	iVec2 T = { 350, 0 };
	P1 += T;
	P2 += T;
	P3 += T;
	P4 += T;

	Chronometer();
	for (int i = 0; i < N; i++)
	{
	//im.draw_triangle(P1, P2, P3, color, true, false, 0);
	im.fill_triangle(P1, P2, P3, colorfill, true);
	//im.fill_triangle(P1, P4, P3, colorfill, true);
	im._lineBresenham<true, false, false, true, true>(P1, P2, color, false, 0, 0);
	im._lineBresenham_avoid<true, false, false, true, true>(P2, P3, P1, color, 0,0);
	im._lineBresenham_avoid_both_sides<true, false, false, true, true>(P3, P1, P2,color,0);
	}
	cout << "1) done in " << mtools::durationToString(Chronometer(),true) << "\n";
	

	/*
	/* 
	
	- dot / pixel access
	- line
	- quadratic bezier
	- rational quad bezier
	- cubic bezier
	

	
	
	
	
	*/

	/*
	iVec2 P[10];

	P[0] = { 350,100 };
	P[1] = { 550,150 };
	P[2] = { 600,350 };
	P[3] = { 550,550 };
	P[4] = { 350,600 };
	P[5] = { 150,550 };
	P[6] = { 100,350 };
	P[7] = { 150,150 };


	iVec2 PC = { 350,350 };
	
	
	im._lineBresenham_AA<true, true, false>(P[0], P[1], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[1], P[2], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[2], P[3], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[3], P[4], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[4], P[5], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[5], P[6], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[6], P[7], color, false, 0);
	im._lineBresenham_AA<true, true, false>(P[7], P[0], color, false, 0);

	im.fill_convex_polygon(8, P, colorfill, true);
	*/
	 

		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.

		
		return 0;
	}







