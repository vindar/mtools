
#include "mtools/mtools.hpp"
using namespace mtools;













void testCSCC()
{

	fBox2 B(100, 300, 100, 300);

	std::vector<fVec2> subject = { { 50,150 }, { 200,50 }, { 350,150 }, { 350,300 }, { 250,300 }, { 200,250 }, { 150,350 }, { 100,250 }, { 100,200 } };
	//std::vector<fVec2> subject = { { 0, 100 },{ 100, 100 },{ 99,200 }, };

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
	plotter.range().setRange(fBox2(0,1000,0,1000));
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


	MT2004_64 gen(0);

	FigureCanvas<5> canvas(20);

	cout << "Creating... ";

	int nb = 1000000;
	const double L = 50000;
	
	for (int k = 0; k < nb; k++)
		{

		/*
			{ // CircleDot
				fVec2 pos = { L * Unif(gen),L * Unif(gen) };
				canvas(Figure::CircleDot(pos, 10, RGBc::c_Red, RGBc::c_Blue), 0);
			}

			{ // SquareDot
				fVec2 pos = { L * Unif(gen),L * Unif(gen) };
				canvas(Figure::SquareDot(pos, RGBc::c_Red, 5), 1);
			}
		*/
		 
			{ // Thick line
			fVec2 P1 = { L * Unif(gen),L * Unif(gen) };
			fVec2 P2 = { L * Unif(gen),L * Unif(gen) };
			canvas(Figure::ThickLine(P1, P2, Unif(gen), RGBc::c_Red.getMultOpacity(0.5f)), 1);
			}


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

	auto PF = makePlot2DFigure(canvas, 5);
	//PF.highQuality(false);
	Plotter2D plotter; 
	plotter[PF];
	plotter.autorangeXY();
	plotter.range().setRange(fBox2(296.023991392, 296.023991668, 3772.45928744, 3772.45928772));
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


inline void assert(int nb, fVec2 Pf1, fVec2 Pf2, bool sta)
	{
	if (!sta)
		{
		cout << "Error " << nb << " at " << Pf1 << " , " << Pf2 << "\n";
		cout.getKey();
		}
	}

void test_lines(int L, double epsilon)
{
	Image im((int)L*epsilon + 2, (int)L*epsilon + 2);
	im.clear(RGBc::c_White);

	for (int x1 = 0; x1 < L; x1++)
		{
		for (int y1 = 0; y1 < L; y1++)
			{
			for (int x2 = 0; x2 < L; x2++)
				{
				for (int y2 = 0; y2 < L; y2++)
					{

					fVec2 Pf1 = { x1*epsilon + 1, y1*epsilon + 1 };
					fVec2 Pf2 = { x2*epsilon + 1, y2*epsilon + 1 };

					Image::_bdir dira,dirb;
					Image::_bpos posa,posb;
					iVec2 P1a, P1b, P2a, P2b;

					int64 lena = im._init_line(Pf1, Pf2, dira, posa, P1a, P2a); 
					int64 lenb = im._init_line(Pf2, Pf1, dirb, posb, P1b, P2b);

					
					assert(0, Pf1, Pf2, lena == lenb);
					assert(1, Pf1, Pf2, P1a == P2b);
					assert(2, Pf1, Pf2, P2a == P1b);
					assert(3, Pf1, Pf2, posa.x = P1a.X());
					assert(4, Pf1, Pf2, posa.y = P1a.Y());
					assert(5, Pf1, Pf2, posb.x = P1b.X());
					assert(6, Pf1, Pf2, posb.y = P1b.Y());
					
					for (int64 i = 0; i < lena; i++)
						{
						im(posa.x, posa.y) = RGBc::c_Black;
						im._move_line(dira, posa,1);
						}
					
					im(posa.x, posa.y) = RGBc::c_Black;
					assert(7, Pf1, Pf2, posa.x = P2a.X());
					assert(8, Pf1, Pf2, posa.y = P2a.Y());

					for (int64 i = 0; i < lenb; i++)
						{
						assert(9, Pf1, Pf2, im(posb.x, posb.y) == RGBc::c_Black);
						im(posb.x, posb.y) = RGBc::c_White;
						im._move_line(dirb, posb, 1);
						}
					assert(10, Pf1, Pf2, im(posb.x, posb.y) == RGBc::c_Black);
					im(posb.x, posb.y) = RGBc::c_White;

					assert(11, Pf1, Pf2, posb.x = P2b.X());
					assert(12, Pf1, Pf2, posb.y = P2b.Y());
					
					}
				}
			}
		cout << ".";
		}



}






inline void nextpoint(double l, Image & im, fVec2 & A, fVec2 & B, fVec2 & C, fVec2 D, RGBc color)
	{
	fVec2 M = 0.5*(A + B);
	fVec2 U = C - M;

	fVec2 Al = A + U;
	fVec2 Bl = B + U;

	fVec2 V = D - C; 

	fVec2 H = { V.Y(), -V.X() };
	H.normalize(); H *= l; 

	fVec2 UU = (Al - C - H); UU.normalize();  UU *= l; 
	iVec2 AA = C + UU;
	fVec2 VV = (Bl - C + H); VV.normalize();  VV *= l; 
	iVec2 BB = C + VV;

	fVec2 A1 = A;
	fVec2 A2 = AA;
	fVec2 A3 = BB; 
	fVec2 A4 = B;

	iVec2 AP1, AP2, AP3, AP4;

	Image::_bdir dir12, dir21;
	Image::_bpos pos12, pos21;
	int64 len12 = im._init_line(A1, A2, dir12, pos12, AP1, AP2);
	pos21 = pos12;
	dir21 = dir12;
	im._reverse_line(dir21, pos21, len12);

	Image::_bdir dir23, dir32;
	Image::_bpos pos23, pos32;
	int64 len23 = im._init_line(A2, A3, dir23, pos23, AP2, AP3);
	pos32 = pos23;
	dir32 = dir23;
	im._reverse_line(dir32, pos32, len23);

	Image::_bdir dir34, dir43;
	Image::_bpos pos34, pos43;
	int64 len34 = im._init_line(A3, A4, dir34, pos34, AP3, AP4);
	pos43 = pos34;
	dir43 = dir34;
	im._reverse_line(dir43, pos43, len34);

	Image::_bdir dir41, dir14;
	Image::_bpos pos41, pos14;
	int64 len41 = im._init_line(A4, A1, dir41, pos41, AP4, AP1);
	pos14 = pos41;
	dir14 = dir41;
	im._reverse_line(dir14, pos14, len41);

	Image::_bdir dir13;
	Image::_bpos pos13;
	int64 len13 = im._init_line(A1, A3, dir13, pos13, AP1, AP3);


	static const bool caa = true;

	im._lineBresenham_avoid<true, true, false, caa, false>(dir12, pos12, len12+1, dir14, pos14, len41 + 1, color, 0);
	im._lineBresenham_avoid<true, true, false, caa, true>(dir43, pos43, len34+1, dir41, pos41, len41 + 1, color, 0);

	/*
	im(AP2) = RGBc::c_Black.getMultOpacity(0.5);
	im(AP3) = RGBc::c_Black.getMultOpacity(0.5);
	*/
	

	im._lineBresenham_avoid_both_sides_triangle<true, true, false, false, true>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir34, pos34, len34 + 1, color, 0);
	
	im._lineBresenham_avoid_both_sides<true, true, false, false, true>
		(dir13, pos13, len13,
			dir12, pos12, len12,
			dir14, pos14, len41,
			dir32, pos32, len23,
			dir34, pos34, len34, color, 0);
			
	
	im._draw_triangle_interior<true, true>(A1, A2, A3, color);
	im._draw_triangle_interior<true, true>(A1, A3, A4, color);
	
	
	A = AA; 
	B = BB; 
	C = D; 
	return;
	}


void rot(fVec2 & V, double alpha)
	{
	double b = alpha * TWOPI / 360;
	V = { V.X() * cos(b) + V.Y() * sin(b), -V.X() * sin(b) + V.Y() * cos(b) };
	}


















int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	//testCSCC();
	testplotfigure();
	return 0;

	double lx = 800.0;
	double ly = 600.0;


	TestImage im((int)(lx), (int)(ly));

	RGBc color = RGBc::c_Red.getMultOpacity(0.5);;
	RGBc color2 = RGBc::c_Green.getMultOpacity(0.5);;
	RGBc colorfill = RGBc::c_Blue.getMultOpacity(0.5);;


	colorfill = color;

	fVec2 Pf1, Pf2, Pf3; 
	iVec2 P1, P2, P3;

	MT2004_64 gen(0);

	/*
	while (1)
		{
		im.clear(RGBc::c_White);
		Pf1 = { Unif(gen)*lx, Unif(gen)*ly };
		Pf2 = { Unif(gen)*lx, Unif(gen)*ly };
		Pf3 = { Unif(gen)*lx, Unif(gen)*ly };
		im._draw_triangle_interior<true, true>(Pf1, Pf2, Pf3, colorfill);

		Image::_bdir dir12, dir21, dir13, dir31, dir23;
		Image::_bpos pos12, pos21, pos13, pos31, pos23;

		int64 len12 = im._init_line(Pf1, Pf2, dir12, pos12, P1, P2);
		dir21 = dir12; pos21 = pos12; im._reverse_line(dir21, pos21, len12);

		int64 len13 = im._init_line(Pf1, Pf3, dir13, pos13, P1, P3);
		dir31 = dir13; pos31 = pos13; im._reverse_line(dir31, pos31, len13);

		int64 len23 = im._init_line(Pf2, Pf3, dir23, pos23, P2, P3);


		fVec2 vA = (Pf3 - Pf1), vB = (Pf2 - Pf1);
		double det = vA.X()*vB.Y() - vB.X()*vA.Y();

		if (det > 0)
			{
			im._lineBresenham<true, true, false, false, true, false>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, true>(dir13, pos13, len13 + 1, dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, false>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
			}
		else
			{
			im._lineBresenham<true, true, false, false, true, true>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, false>(dir13, pos13, len13 + 1 , dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, true>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
			}
		
		cout << "Pf1 = " << Pf1 << " \t P1 = " << P1 << "\n";
		cout << "Pf2 = " << Pf2 << " \t P2 = " << P2 << "\n";
		cout << "Pf3 = " << Pf3 << " \t P3 = " << P3 << "\n";

		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.		
		}
		*/


	im.clear(RGBc::c_White);


	{

		double l = 35; 
		double r = 5; 

		fVec2 O = { 200.5,200.5 };

		fVec2 A = { O.X() - l, O.Y() };
		fVec2 B = { O.X() + l, O.Y() };
		fVec2 C = { O.X(), O.Y() + r };


		fVec2 R = { 0, r };


		fVec2 D;

		for (int i = 0; i < 100; i++)
		{
			D = C + R;
			nextpoint(l, im, A, B, C, D, color);
			rot(R, 2);
		}


		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
	//	plotter.plot();                 // start interactive display.		


	}


	Chronometer();
	int NSN = 100;
	double l = 0.75;

	for (int i = 0; i < NSN; i++)
	{

		fVec2 Pfa = { Unif(gen)*lx, Unif(gen)*ly };
		fVec2 Pfb = { Unif(gen)*lx, Unif(gen)*ly };

		iVec2 JA = { (int64)round(Pfa.X()), (int64)round(Pfa.Y()) };
		iVec2 JB = { (int64)round(Pfb.X()), (int64)round(Pfb.Y()) };

//		if (i & 1)
	//		im.draw_line(JA, JB, color2, true, true, true, l);
	//	else
		{
			fVec2 U = Pfa - Pfb;
			fVec2 V = { U.Y(), -U.X() };
			V.normalize();
			V *= l;

			fVec2 A1 = Pfa + V;
			fVec2 A2 = Pfb + V;
			fVec2 A3 = Pfb - V;
			fVec2 A4 = Pfa - V;

			im._draw_triangle_interior<true, true>(A1, A2, A3, colorfill);
			im._draw_triangle_interior<true, true>(A1, A3, A4, colorfill);

			iVec2 AP1, AP2, AP3, AP4;

			Image::_bdir dir12, dir21;
			Image::_bpos pos12, pos21;
			int64 len12 = im._init_line(A1, A2, dir12, pos12, AP1, AP2);
			pos21 = pos12;
			dir21 = dir12;
			im._reverse_line(dir21, pos21, len12);

			Image::_bdir dir23, dir32;
			Image::_bpos pos23, pos32;
			int64 len23 = im._init_line(A2, A3, dir23, pos23, AP2, AP3);
			pos32 = pos23;
			dir32 = dir23;
			im._reverse_line(dir32, pos32, len23);

			Image::_bdir dir34, dir43;
			Image::_bpos pos34, pos43;
			int64 len34 = im._init_line(A3, A4, dir34, pos34, AP3, AP4);
			pos43 = pos34;
			dir43 = dir34;
			im._reverse_line(dir43, pos43, len34);

			Image::_bdir dir41, dir14;
			Image::_bpos pos41, pos14;
			int64 len41 = im._init_line(A4, A1, dir41, pos41, AP4, AP1);
			pos14 = pos41;
			dir14 = dir41;
			im._reverse_line(dir14, pos14, len41);

			Image::_bdir dir13;
			Image::_bpos pos13;
			int64 len13 = im._init_line(A1, A3, dir13, pos13, AP1, AP3);

			
			static const int caa = true;
			im._lineBresenham<true, true, false, false, caa, false>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, caa, false>(dir23, pos23, len23 + 1, dir21, pos21, len12 + 1, color, 0);
			im._lineBresenham_avoid<true, true, false, caa, false>(dir34, pos34, len34 + 1, dir32, pos32, len23 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, caa, false>(dir41, pos41, len41, dir43, pos43, len34 + 1, dir12, pos12, len12 + 1, color, 0);
			

			im._lineBresenham_avoid_both_sides<true, true, false, false, true>
				(dir13, pos13, len13,
					dir12, pos12, len12,
					dir14, pos14, len41,
					dir32, pos32, len23,
					dir34, pos34, len34, color, 0);
		}

			 
			 

		/*

		Image::_bdir dir12, dir21, dir13, dir31, dir23;
		Image::_bpos pos12, pos21, pos13, pos31, pos23;

		int64 len12 = im._init_line(Pf1, Pf2, dir12, pos12, P1, P2);
		dir21 = dir12; pos21 = pos12; im._reverse_line(dir21, pos21, len12);

		int64 len13 = im._init_line(Pf1, Pf3, dir13, pos13, P1, P3);
		dir31 = dir13; pos31 = pos13; im._reverse_line(dir31, pos31, len13);

		int64 len23 = im._init_line(Pf2, Pf3, dir23, pos23, P2, P3);

		fVec2 vA = (Pf3 - Pf1), vB = (Pf2 - Pf1);
		double det = vA.X()*vB.Y() - vB.X()*vA.Y();

		if (det > 0)
		{
			im._lineBresenham<true, true, false, false, true, false>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, true>(dir13, pos13, len13 + 1, dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, false>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
		}
		else
		{
			im._lineBresenham<true, true, false, false, true, true>(dir12, pos12, len12 + 1, color, 0, 0);
			im._lineBresenham_avoid<true, true, false, true, false>(dir13, pos13, len13 + 1, dir12, pos12, len12 + 1, color, 0);
			im._lineBresenham_avoid_both_sides_triangle<true, true, false, true, true>(dir23, pos23, len23, dir21, pos21, len12 + 1, dir31, pos31, len13 + 1, color, 0);
		}


		*/

	}


	int64 xs = 120;
	int64 ys = 0;

	iBox2 BB(100 + xs, 200 + xs, 100 + ys, 200 + ys);





	colorfill = RGBc::c_Blue.getMultOpacity(0.5);

	im.clear(RGBc::c_White);
	im.draw_box(BB, RGBc::c_Gray, false);

	cout << mtools::durationToString(Chronometer(), true);


	//im._draw_ellipse4_AA<true,true,false>(im.imageBox(), { 300,300 }, 1, 2, color, colorfill, 0);

	double R = 100;
	fVec2 P = {300,300};

	//im.draw_thick_ellipse(P, R, R + 50, 3, 5, color, true, true);


	fBox2 SR(0.0, 4.0, 0.0, 3.0);

	im.canvas_draw_thick_filled_circle(SR, { 0.5,0.5 }, 3, 3, false, color, colorfill);

	im(300, 300) = RGBc::c_Black;

	//im.draw_thick_circle(P, 10 + R, 2.0, color, colorfill, true, true);
	//im.draw_thick_circle(P, 20 + R, 2.0, color, colorfill, true, true);

	/*
	int qqL = 10000;
	Chronometer();
	for (int qq = 0; qq < qqL; qq++)
		{
			im._draw_ellipse_thick_AA<true, false, false>(im.imageBox(), { 250.0 , 190.0 }, 120, 170, 121, 171, color, colorfill, 0);
		}
	cout << "done in " << mtools::durationToString(Chronometer(), true) << "\n";
	
	for (int qq = 0; qq < qqL; qq++)
	{
		im._draw_ellipse2_AA<true, false, false>(im.imageBox(), { 455.0 , 450.0 }, 150, 200, color, colorfill, 0);
	}
	cout << "done in " << mtools::durationToString(Chronometer(), true) << "\n";

	for (int qq = 0; qq < qqL; qq++)
	{
		im.draw_ellipse({ 250,550 }, 150, 200, color,true, false, 0);
	}
	cout << "done in " << mtools::durationToString(Chronometer(), true) << "\n";


	*/
	cout << "zzzz"; 

	//auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	


	//auto PTF = makePlotTestFig(); 


		return 0;
	}

