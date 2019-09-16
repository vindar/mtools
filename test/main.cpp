
#include "mtools/mtools.hpp"
using namespace mtools;










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
	//const double L = 50000;
	
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
		
		std::vector<fVec2> tri = { { -50, 50 } , {200, 127},  {-300,0} };
		std::vector<fVec2> qu = { { 480,677 } , {700, 800},{ 600, 520 } ,{ 500,500 }  };

		canvas(Figure::ThickLine({ 0,100 }, { 200,140 }, 2, true, RGBc::c_Red.getMultOpacity(0.5f)), 0);


		//canvas(Figure::PolyLine(subject, RGBc::c_Green.getMultOpacity(0.5f),1), 1);
		
		canvas(Figure::ThickPolygon(subject, 20, true, RGBc::c_Red.getMultOpacity(0.5f), RGBc::c_Yellow.getMultOpacity(0.5f)), 0);

		canvas(Figure::ThickPolygon(tri, 2 , true, RGBc::c_Green.getMultOpacity(0.5f)), 0);
		canvas(Figure::ThickPolygon(qu, 3, true, RGBc::c_Blue.getMultOpacity(0.5f)), 0);


		canvas(Figure::ThickRectangle(fBox2( -100,-50,0,30 ), 2, 2, false, RGBc::c_Maroon.getMultOpacity(0.5f), RGBc::c_Yellow), 0);
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


		//fVec2 pos2 = { 5000 * Unif(gen),5000 * Unif(gen) };
		//double rad = 10 * Unif(gen);
		//fVec2 pos = { L * Unif(gen),L * Unif(gen) };
		//fVec2 P1 = pos;
		//fVec2 P2 = pos + fVec2(Unif(gen), Unif(gen));
		//fVec2 P3 = pos + fVec2(Unif(gen), Unif(gen));
		//fVec2 P4 = P1 + 3.0*(P3 - P2);

		//canvas(Figure::Quad(P1,P2,P3,P4, RGBc::c_Red.getMultOpacity(0.5f), RGBc::c_Red.getMultOpacity(0.5f)), 1);

		//canvas(Figure::Triangle(pos, pos + fVec2(Unif(gen),Unif(gen)) , pos + fVec2(Unif(gen), Unif(gen)), RGBc::c_Red.getMultOpacity(0.5f), RGBc::c_Red.getMultOpacity(0.5f)), 1);

		//canvas(Figure::Line(pos, pos2, RGBc::c_Red), 1);

//		canvas(Figure::EllipsePart(BOX_SPLIT_UP_RIGHT, pos, 10 * Unif(gen), 10 * Unif(gen), 10, 0, false, RGBc::c_Red.getMultOpacity(1), RGBc::c_Lime.getMultOpacity(0.5)));
		//pos = { 50000 * Unif(gen),50000 * Unif(gen) };
		//rad = 1* Unif(gen);
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
	//plotter.range().setRange(fBox2(199.935142875, 200.050950875, 249.926736625, 250.042544625));
	plotter.plot();
	}











/**************************************************************
 * Testing bezier class. To use in image class later for drawing
 * bezier curve with clipping 
**************************************************************/




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
		im.draw_quad_bezier((iVec2)sp.first.P0, (iVec2)sp.first.P2, (iVec2)sp.first.P1, (float)sp.first.w1, color, true, true, true);
		}

	color = (C.isInside(BQ(0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
	BQ.normalize();
	im.draw_quad_bezier((iVec2)BQ.P0, (iVec2)BQ.P2, (iVec2)BQ.P1, (float)BQ.w1, color, true, true, true);
}

void draw(BezierQuadratic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_quad_bezier((iVec2)sp.P0, (iVec2)sp.P2, (iVec2)sp.P1, 1, color, true, true, true, penwidth);
	}

void draw(BezierRationalQuadratic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_quad_bezier((iVec2)sp.P0, (iVec2)sp.P2, (iVec2)sp.P1, (float)sp.w1, color, true, true, true, penwidth);
	}

void draw(BezierCubic sp, Image & im, RGBc color, int penwidth)
	{
	im.draw_cubic_bezier((iVec2)sp.P0, (iVec2)sp.P3, (iVec2)sp.P1, (iVec2)sp.P2, color, true, true, true, penwidth);
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
	//size_t N = 50000;
	int64 LX = 1000;
	int64 LY = 1000;

	Image im(LX, LY);
	im.clear(RGBc(240,240,200));
	MT2004_64 gen(0);

	while (1)
	{
		im.clear(RGBc(240, 240, 200));

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

		BezierQuadratic curve((fVec2)P0, (fVec2)P1, (fVec2)P2);
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
//		im.draw_rectangle(TB, RGBc::c_Yellow, true);

		testBezier((fBox2)TB, curve, im);

		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.
	}
}






void error_cb(const std::string & title, const std::string & msg)
	{
	cout << title << "\n" << msg;
	}









/*


*/


void testSVG()
	{
	
		{
			auto canvas = makeFigureCanvas(2);

			/*
			canvas(Figure::CircleDot(fVec2{ 50, 50 }, RGBc::c_Green), 1);
			canvas(Figure::SquareDot(fVec2{ 60, 50 }, RGBc::c_Green.getMultOpacity(0.5f)), 1);


			canvas(Figure::VerticalLine(0, 0, 100, RGBc::c_Black), 0);
			canvas(Figure::HorizontalLine(100, 0, 200, RGBc::c_Red.getOpacity(0.2f)), 0);
			canvas(Figure::Line({ 200, 100 }, { 150,50 }, RGBc::c_Red), 0);

			std::vector<fVec2> tabPL = { fVec2(70, 50), fVec2(80,50), fVec2(75,60) };
			canvas(Figure::PolyLine(tabPL, RGBc::c_Black, 2), 1);


			canvas(Figure::ThickHorizontalLine(-20,-40,50,RGBc::c_Yellow, 20), 0);
			canvas(Figure::ThickVerticalLine(-20, -50, 50, RGBc::c_Orange, 20), 1);

			canvas(Figure::ThickLine({-50, 30}, {50,-30}, 20, true, RGBc::c_Purple.getMultOpacity(0.5)), 1);


			canvas(Figure::ThickPolyLine(tabPL, 5, true, RGBc::c_Salmon), 0);

			canvas(Figure::Rectangle(fBox2(2,5,2,10), RGBc::c_Red, RGBc::c_Salmon.getMultOpacity(0.5)), 0);
			
			canvas(Figure::Triangle({ 2,2 }, { 5 ,10 }, {10, 10}, RGBc::c_Green.getMultOpacity(0.5)), 1);


			canvas(Figure::Quad({-100,-100}, {-80, -80}, {-50,-90}, {-30,-130}, RGBc::c_Red, RGBc::c_Salmon.getMultOpacity(0.2f)), 0);

			std::vector<fVec2> tabPO = { { -100,-100 }, { -200, -200 }, { -100,-150 }, { -80,-200 }, { -50,-100 } };

			canvas(Figure::Polygon(tabPO, RGBc::c_Green, RGBc::c_Salmon.getMultOpacity(0.0f)), 0);



			canvas(Figure::Rectangle(fBox2(300, 310, 100, 110), RGBc::c_Black), 1);

			canvas(Figure::ThickTriangle({ 300,100 }, { 305 ,110 }, { 310, 103 }, 2.0, true, RGBc::c_Green.getMultOpacity(0.5), RGBc::c_Cyan.getMultOpacity(0.5)), 0);



			canvas(Figure::Rectangle(fBox2(200, 220, 100, 130), RGBc::c_Black), 1);
			canvas(Figure::ThickRectangle(fBox2(200, 220, 100, 130), 5 , 5, true, RGBc::c_Cyan.getMultOpacity(0.5f), RGBc::c_Yellow.getMultOpacity(0.5f)), 0);



			canvas(Figure::Rectangle(fBox2(-60, 00, 90, 110), RGBc::c_Black), 1);

			//canvas(Figure::ThickEllipse({ -30, 100 },30,10,5,5,true,RGBc::c_Blue.getMultOpacity(0.5), RGBc::c_Gray), 1);

			canvas(Figure::ThickCirclePart(BOX_SPLIT_LEFT, { -30, 100 } , 10, 3, true, RGBc::c_Gray, RGBc::c_Blue.getMultOpacity(0.5)), 1);
			canvas(Figure::ThickCirclePart(BOX_SPLIT_UP_RIGHT, { -30, 100 }, 10, 5,  true, RGBc::c_Gray, RGBc::c_Red.getMultOpacity(0.5)), 1);
			canvas(Figure::ThickCirclePart(BOX_SPLIT_DOWN_RIGHT, { -30, 100 },  10, 5,  false, RGBc::c_Gray,  RGBc::c_Green.getMultOpacity(0.5)), 1);
			*/

			std::string s = "Lorem Ipsum";

			canvas(Figure::Text(s, { 50, 100 }, { 200, 0 }, MTOOLS_TEXT_CENTERTOP, RGBc::c_Black, RGBc::c_Red.getMultOpacity(0.2f), 1.0f), 0);
			canvas(Figure::ThickRectangle(fBox2(0,50,0,100),1,1, true, RGBc::c_Red), 0);


			canvas.saveSVG("azerty.svg", true, {0, 500});

			/*Petit texte\n  qui ne semble\n\tpas long  ...*/

			canvas.saveSVG("fig.svg");

			auto PA = makePlot2DFigure(canvas);

			Plotter2D plotter;              // Create a plotter object
			plotter[PA];	                // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}
		



			{
			auto canvas = makeFigureCanvas();
			canvas.loadSVG("fig.svg");

			auto PA = makePlot2DFigure(canvas);

			Plotter2D plotter;              // Create a plotter object
			plotter[PA];	                // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
			}

	return;

	}




/*

-> decoupage en ligne
-> expension du texte en supprimant les \t et en remplacant par 4 space
-> calcul de la longueur d'unr ligne en police 44
-> calcul de la hauteur d'une ligne


*/

void testfont()
{
//	std::string s = "Ceciestuntestpourvoirsicamarche."; 
	std::string s = "b  b  b  b  b  b";
	cout << gFont(64, MTOOLS_EXACT_FONT).textDimension(s) << "\n";
	cout.getKey();

}


int main(int argc, char *argv[])
	{

	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


//	testfont();

	testSVG(); 
	return 0; 
	auto canvas = makeFigureCanvas();

	canvas(Figure::Text("Hello World !", { 0.0, 0.0 }, { -20.0, 20.0 }, MTOOLS_TEXT_RIGHT | MTOOLS_TEXT_YCENTER, RGBc::c_Red, RGBc::c_Blue.getMultOpacity(0.5) ),0); 
	canvas(Figure::Text("A", { 0.0, 0.0 }, { 10.0, 0.0 }, MTOOLS_TEXT_LEFT | MTOOLS_TEXT_TOP, RGBc::c_Red, RGBc::c_Blue.getMultOpacity(0.5),1.0f), 0);
	canvas(Figure::Text("B", { 0.0, 0.0 }, { 10.0, 0.0 }, MTOOLS_TEXT_LEFT | MTOOLS_TEXT_BOTTOM, RGBc::c_Red, RGBc::c_Blue.getMultOpacity(0.5), 1.0f), 0);

	canvas(Figure::Text("ceci est un text\navec plusieurs lignes\n... yopla :)", { 50.0, 20.0 }, { 130.0, 0.0 }, MTOOLS_TEXT_CENTER, RGBc::c_Black, RGBc::c_Blue.getMultOpacity(0.1f)), 0);

	auto PA = makePlot2DFigure(canvas);

	Plotter2D plotter;              // Create a plotter object
	plotter[PA];	                // Add the image to the list of objects to draw.  	
	plotter.autorangeXY();          // Set the plotter range to fit the image.
	plotter.plot();                 // start interactive display.

	return 0; 





	mtools::ostringstream os; 



	fVec2 d(1, 2);

	os << "Hello wrold : " << 123 << " - " << d << "\n" << std::tuple<double, int, char>(12.3, 4, 'c') << "\n";

	cout << os; 


	cout << "azerty";
	cout.getKey(); 

	testplotfigure();
	return 0;
	}

