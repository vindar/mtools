
#include "mtools/mtools.hpp"
using namespace mtools;







void testPalette()
{
	Image im(800, 800);

	im.clear(RGBc::c_White);

	auto PA = ColorPalette().set(RGBc::c_Red, RGBc::c_Gray, 12);


	for(int64 k = 0; k < 600; k++)
		{
		im.draw_horizontal_line(k + 100, 100, 200, PA.getLog(k,0,500, 1.2), true, true);
		im.draw_horizontal_line(k + 100, 300, 400, PA(((double)k) / 500.0, true), true, true);
		}

		Plotter2D plot;

		auto P = makePlot2DImage(im); 
		plot[P];
		plot.autorangeXY();
		plot.plot(); 

}




void testCSCC()
{

	fBox2 B(100, 300, 100, 300);

	std::vector<fVec2> subject = { { 50,150 }, { 200,50 }, { 350,150 }, { 350,300 }, { 250,300 }, { 200,250 }, { 150,350 }, { 100,250 }, { 100,200 } };
	//std::vector<fVec2> subject = { { 0, 100 },{ 100, 100 },{ 99,200 }, };


	cout << winding(subject); 

	fVec2 res[1000];
	size_t res_size = 0;

	Sutherland_Hodgman_clipping(subject.data(), subject.size(), B, res, res_size);

	FigureCanvas<5> canvas(3);

	canvas(Figure::Line( { B.min[0], B.min[1] }, { B.max[0], B.min[1] }, RGBc::c_Black), 0);
	canvas(Figure::Line( { B.min[0], B.max[1] }, { B.max[0], B.max[1] }, RGBc::c_Black), 0);
	canvas(Figure::Line( { B.min[0], B.min[1] }, { B.min[0], B.max[1] }, RGBc::c_Black), 0);
	canvas(Figure::Line( { B.max[0], B.min[1] }, { B.max[0], B.max[1] }, RGBc::c_Black), 0);

	for (size_t i = 0; i < subject.size(); i++)
		{
		canvas(Figure::Line(subject[i], subject[(i + 1) % subject.size()], RGBc::c_Green), 1);
		}

	for (size_t i = 0; i < res_size; i++)
		{
		cout << res[i] << "\n";
		canvas(Figure::Line(res[i], res[(i + 1) % res_size], RGBc::c_Red), 2);
		}

	auto PF = makePlot2DFigure(canvas, 5);
	Plotter2D plotter;
	plotter[PF];
	plotter.autorangeXY();
	plotter.range().setRange(fBox2(199.935142875, 200.050950875, 249.926736625, 250.042544625));
	plotter.plot();

	return;



	}















class BLine
	{



	int64 dx, dy;			// step size in each direction
	int64 stepx, stepy;		// directions (+/-1)
	int64 rat;				// ratio max(dx,dy)/min(dx,dy) to speed up computations
	int64 amul;				// multiplication factor to compute aa values. 
	bool x_major;			// true if the line is xmajor (ie dx > dy) and false if y major (dy >= dx).

	int64 x, y;				// current pos
	int64 frac;				// fractional part


	};












#define HH 5



void testplotfigure()
	{



	std::vector<fVec2> tr = { {0,0} , {0, 100}, {49,50} , {100, 0} };

	cout << "wind = " << winding(tr) << "\n";
	cout << "convex = " << convex(tr) << "\n";
	cout << "left_of = " << left_of({ 1,0 }, { 0,100 }, {0,50}) << "\n";


	MT2004_64 gen(0);

	FigureCanvas<5> canvas(2);

	cout << "Creating... ";

	int nb = 1;
	const double L = 50000;
	
	for (int k = 0; k < nb; k++)
		{

		std::vector<fVec2> subject = { { 50,150 },{ 200,50 },{ 350,150 },{ 350,300 },{ 250,300 },{ 200,250 },{ 150,350 },{ 100,250 },{ 100,200 } };

		/*
		std::vector<fVec2> enlarged;
		mtools::internals_polyline::polylinetoPolygon(subject, 10, enlarged);

		cout << enlarged << "\n";
		for (size_t l = 0; l < enlarged.size()/2; l++)
			{
			mtools::swap(enlarged[l], enlarged[enlarged.size() - 1 - l]);
			}

		cout << enlarged << "\n\n\n";
		*/
		
		std::vector<fVec2> tri = { {-300,0}, {200, 127}, {-50, 50} };
		std::vector<fVec2> qu = { {500,500} , {600, 520} , {700, 800}, {480,677} };

//		canvas(Figure::PolyLine(subject, RGBc::c_Green.getMultOpacity(0.5f),1), 0);
		canvas(Figure::ThickPolyLine(subject, 50, RGBc::c_Red.getMultOpacity(0.5f)), 0);

		canvas(Figure::Polygon(tri, RGBc::c_Green.getMultOpacity(0.5f), RGBc::c_Green.getMultOpacity(0.5f)), 0);
		canvas(Figure::Polygon(qu, RGBc::c_Blue.getMultOpacity(0.5f), RGBc::c_Blue.getMultOpacity(0.5f)), 0);

	//	canvas(Figure::Polygon(subject, RGBc::c_Black, RGBc::c_Yellow.getMultOpacity(0.5f)), 1);


		//canvas(Figure::Polygon(subject, RGBc::c_Red, RGBc::c_Red), 0);


		/*
			{ // CircleDot
				fVec2 pos = { L * Unif(gen),L * Unif(gen) };
				canvas(Figure::CircleDot(pos, 10, RGBc::c_Red, RGBc::c_Blue), 0);
			}

			{ // SquareDot
				fVec2 pos = { L * Unif(gen),L * Unif(gen) };
				canvas(Figure::SquareDot(pos, RGBc::c_Red, 5), 1);
			}
		 
			{ // Thick line
			fVec2 P1 = { L * Unif(gen),L * Unif(gen) };
			fVec2 P2 = { L * Unif(gen),L * Unif(gen) };
			canvas(Figure::ThickLine(P1, P2, Unif(gen), RGBc::c_Red.getMultOpacity(0.5f)), 1);
			}
		*/


		fVec2 pos2 = { 5000 * Unif(gen),5000 * Unif(gen) };
		double rad = 10 * Unif(gen);
		fVec2 pos = { L * Unif(gen),L * Unif(gen) };
		fVec2 P1 = pos;
		fVec2 P2 = pos + fVec2(Unif(gen), Unif(gen));
		fVec2 P3 = pos + fVec2(Unif(gen), Unif(gen));
		fVec2 P4 = P1 + 3.0*(P3 - P2);

		//canvas(Figure::Quad(P1,P2,P3,P4, RGBc::c_Red.getMultOpacity(0.5f), RGBc::c_Red.getMultOpacity(0.5f)), 1);

		//canvas(Figure::Triangle(pos, pos + fVec2(Unif(gen),Unif(gen)) , pos + fVec2(Unif(gen), Unif(gen)), RGBc::c_Red.getMultOpacity(0.5f), RGBc::c_Red.getMultOpacity(0.5f)), 1);

		//canvas(Figure::Line(pos, pos2, RGBc::c_Red), 1);

//		canvas(Figure::EllipsePart(BOX_SPLIT_UP_RIGHT, pos, 10 * Unif(gen), 10 * Unif(gen), 10, 0, false, RGBc::c_Red.getMultOpacity(1), RGBc::c_Lime.getMultOpacity(0.5)));
		
		pos = { 50000 * Unif(gen),50000 * Unif(gen) };
		rad = 1* Unif(gen);
//		canvas(Figure::VerticalLine(pos.Y(), pos.X() - rad, pos.X() + rad, 5 , true, RGBc::c_Blue.getMultOpacity(1)),1);
		}
	

	/*
	canvas(Figure::CirclePart(BOX_SPLIT_UP, { (double)nb , 0 }, 0, 0.5, true, RGBc::c_Red.getMultOpacity(1)), 0);
	for (int k = 0; k < nb; k++)
		{
		canvas(Figure::CirclePart(BOX_SPLIT_UP, { (double)nb , 0}, k + 1, 0.5, true, RGBc::c_Red.getMultOpacity(1)), 0);
		}
	*/
	cout << "ok !\n\n";

	auto PF = makePlot2DFigure(canvas, 1);
	//PF.highQuality(false);
	Plotter2D plotter; 
	plotter[PF];
	plotter.autorangeXY();
	plotter.range().setRange(fBox2(199.935142875, 200.050950875, 249.926736625, 250.042544625));
	plotter.plot();
	}













/*

drawing parameters

 antialiased    (bool)
 blend			(bool)
 tickness		(double)
 tickscale		(double)
 
 figures

 outline 

 - lines
 - multi broken lines
 - closed multi  broken lines
 - open bezier curves
 - circle ellipse

 
 - triangle
 - square
 - convex polygon
 - circle
 - ellipse


 */







class TestImage : public Image
	{

	public:

	//	void draw_line_new(const iVec2 & P1, const iVec2 & P2, RGBc color, int32 penwidth = 0, bool antialiasing = true, bool blending = true);


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
		im.draw_square_dot(P0, RGBc::c_Green, true, 2);
		im.draw_square_dot(P1, RGBc::c_Green, true, 2);
		im.draw_square_dot(P2, RGBc::c_Green, true, 2);
		im.draw_square_dot(P3, RGBc::c_Green, true, 2);

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


	
											//testCSCC();
	testplotfigure();
	return 0;
		return 0;
	}

