
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:

		void draw_line_new(const iVec2 & P1, const iVec2 & P2, RGBc color, int32 penwidth = 0, bool antialiasing = true, bool blending = true);


	TestImage(int64 lx, int64 ly) : Image(lx, ly) 	
	{

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

	double y = 1.5; 
	fVec2 Pf1{ 0.3, 0 };
	fVec2 Pf2{ 100, 30};

	fVec2 Pf3{ 0.3, 0  + y};
	fVec2 Pf4{ 100, 30 + y};


	iVec2 P1, P2, P3, P4; 

	Image::_bdir dira, dirb;
	Image::_bpos posa, posb;

	int64 lena = im._init_line(Pf1, Pf2, dira, posa, P1, P2);
	int64 lenb = im._init_line(Pf3, Pf4, dirb, posb, P3, P4);

	cout << "Pf1 = " << Pf1 << " \t P1 = " << P1 << "\n";
	cout << "Pf2 = " << Pf2 << " \t P2 = " << P2 << "\n";
	cout << "Pf3 = " << Pf3 << " \t P3 = " << P3 << "\n";
	cout << "Pf4 = " << Pf4 << " \t P4 = " << P4 << "\n";

	im._lineBresenham<true, true, false, false, true, true>(dira, posa, lena, color, 0, 0);
	im._lineBresenham<true, true, false, false, true, false>(dirb, posb, lenb, color, 0, 0);


	//	im.draw_triangle(P1, P2, P3, RGBc::c_Red.getMultOpacity(0.5), true, true);
	 

		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.

		
		return 0;
	}







