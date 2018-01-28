
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:



	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}
	












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






/**
 * Evaluate a quadratic bezier curve at position t.
 **/
fVec2 _quad_bezier_eval(fVec2 P0, fVec2 P1, fVec2 P2, double t)
	{
	return  fVec2(((P0.X() - 2 * P1.X() + P2.X())*t + 2 * (P1.X() - P0.X()))*t + P0.X(),
	           	 ((P0.Y() - 2 * P1.Y() + P2.Y())*t + 2 * (P1.Y() - P0.Y()))*t + P0.Y());
	}


/**
* Find the root inside [0,1] of the derivative of a quad. bezier equation
* if it exists.
**/
double _quad_bezier_solve_deriv(double x0, double x1, double x2)
{
	double dem = x0 + x2 - 2 * x1;
	if (dem == 0) { return -1; }
	double res = (x0 - x1) / dem;
	return ((res > 1) ? -1 : res);
}


fBox2 quadBezierBoundingBox(fVec2 P0, fVec2 P1, fVec2 P2)
	{
	fBox2 B(std::min<double>(P0.X(), P2.X()), std::max<double>(P0.X(), P2.X()), std::min<double>(P0.Y(), P2.Y()), std::max<double>(P0.Y(), P2.Y()));
	
	double tx = _quad_bezier_solve_deriv(P0.X(), P1.X(), P2.X());
	if (tx > 0) { B.swallowPoint(_quad_bezier_eval(P0, P1, P2, tx)); }
	double ty = _quad_bezier_solve_deriv(P0.Y(), P1.Y(), P2.Y());
	if (ty > 0) { B.swallowPoint(_quad_bezier_eval(P0, P1, P2, ty)); }
	return B;
	}


iBox2 quadBezierBoundingBox(iVec2 P0, iVec2 P1, iVec2 P2)
	{
	fBox2 fB = quadBezierBoundingBox((fVec2)P0, (fVec2)P1, (fVec2)P2);
	iBox2 iB = fB.integerEnclosingRect_larger();
	return iB; 
	}




/**
* Evaluate a cubic bezier curve at position t.
**/
fVec2 _cubic_bezier_eval(fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3, double t)
	{
	return  fVec2( (((P3.X() + 3 * (P1.X() - P2.X()) - P0.X())*t + 3 * (P2.X() - 2 * P1.X() + P0.X()))*t + 3 * (P1.X() - P0.X()))*t + P0.X(),
		           (((P3.Y() + 3 * (P1.Y() - P2.Y()) - P0.Y())*t + 3 * (P2.Y() - 2 * P1.Y() + P0.Y()))*t + 3 * (P1.Y() - P0.Y()))*t + P0.Y() );
	}


/**
* Find the roots inside [0,1] of the derivative of a cubic bezier equation
* if it exists.
**/
std::pair<double,double> _cubic_bezier_solve_deriv(double x0, double x1, double x2, double x3)
	{
	double a = 3 * (3 * x1 - 3 * x2 + x3 - x0);
	double b = 6 * (x0 - 2*x1 + x2);
	double c = 3 * (x1 - x0);
	std::pair<double, double> root { -1,-1 };
	int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &root.first, &root.second);
	if (root.first > 1) root.first = -1;
	if (root.second > 1) root.second = -1;
	return root;
	}


fBox2 cubicBezierBoundingBox(fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3)
	{
	fBox2 B(std::min<double>(P0.X(), P3.X()), std::max<double>(P0.X(), P3.X()), std::min<double>(P0.Y(), P3.Y()), std::max<double>(P0.Y(), P3.Y()));

	auto rx = _cubic_bezier_solve_deriv(P0.X(), P1.X(), P2.X(),  P3.X());
	if (rx.first > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, rx.first)); }
	if (rx.second > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, rx.second)); }

	auto ry = _cubic_bezier_solve_deriv(P0.Y(), P1.Y(), P2.Y(), P3.Y());
	if (ry.first > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, ry.first)); }
	if (ry.second > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, ry.second)); }

	return B;
	}


iBox2 cubicBezierBoundingBox(iVec2 P0, iVec2 P1, iVec2 P2, iVec2 P3)
	{
	fBox2 fB = cubicBezierBoundingBox((fVec2)P0, (fVec2)P1, (fVec2)P2, (fVec2)P3);
	iBox2 iB = fB.integerEnclosingRect_larger();
	return iB;
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

		cout << "P0 : " << P0 << "\n";
		cout << "P1 : " << P1 << "\n";
		cout << "P2 : " << P2 << "\n";
		cout << "P3 : " << P3 << "\n";


		P0 = { 226, 803 };
		P1 = { 600, 748};
		P2 = { 665, 154};
		P3 = { 604, 485};

		auto bb = cubicBezierBoundingBox(P0, P1, P2, P3);

		cout << "bb : " << bb << "\n";

		im.draw_box(bb, RGBc::c_Gray, true);

		im.draw_dot(P0, RGBc::c_Green, true, 2);
		im.draw_dot(P1, RGBc::c_Green, true, 2);
		im.draw_dot(P2, RGBc::c_Green, true, 2);
		im.draw_dot(P3, RGBc::c_Green, true, 2);
		/*
		im.draw_line(P0, P1, RGBc::c_Green, false);
		im.draw_line(P1, P2, RGBc::c_Green, false);
		im.draw_line(P2, P3, RGBc::c_Green, false);
		*/
	//	im.draw_cubic_bezier(P0, P3, P1, P2, RGBc::c_Red, true, true,false);
		im.draw_cubic_bezier(P0, P3, P1, P2, RGBc::c_Blue, true, true, true);

		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.
	}
}






	int main(int argc, char *argv[])
	{

		MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
		
		testCF();
		cout.getKey(); 
		return 0;

	


		
		return 0;



/*
		{
			Image im("hello.png");
			im.rescale(10, { im.lx() * 10,im.ly() * 10 });
			auto P = makePlot2DImage(im,6);   // Encapsulate the image inside a 'plottable' object.	
			Plotter2D plotter;              // Create a plotter object
			plotter.axesObject(false);      // Remove the axe system.
			plotter[P];                     // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}
		return 0;
		{
			Image im(800, 600, RGBc(150, 150, 150,150));  // image of size 800x600 with a light gray background

													  // draw on the image
			im.fill_ellipse_in_rect({ 100,400,50,550 }, RGBc::c_Cyan, false);
			im.draw_ellipse_in_rect({ 100,400,50,550 }, RGBc::c_Green, true, true, 4);
			im.draw_text({ 400, 300 }, "Hello\n  World!", MTOOLS_TEXT_CENTER, RGBc::c_Red.getOpacity(0.7f), 200);
			im.draw_cubic_spline({ { 10,10 },{ 100,100 },{ 200,30 },{ 300,100 },{ 600,10 } ,{ 700,300 },
				{ 720, 500 },{ 600, 480 },{ 400,500 } }, RGBc::c_Yellow.getOpacity(0.5f), true, true, true, 3);


			im.draw_line({ 0,0 }, { 800,337 }, RGBc(200, 100, 57, 200), true, true, true, 20);

			im.save("hello2.png");
			im.save("hello2.jpeg");
			im.save("hello2.bmp");

			// display the image
			auto P = makePlot2DImage(im);   // Encapsulate the image inside a 'plottable' object.	
			Plotter2D plotter;              // Create a plotter object
			plotter.axesObject(false);      // Remove the axe system.
			plotter[P];                     // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}
		return 0;
		*/

		TreeFigure<int, NN> TF;

		int n = 1000;


		cout << "inserting...\n";
		mtools::Chronometer();
		/*
		{
			cout << "DEserializing...\n";
			IFileArchive ar("testTreeAR.txt");
			ar & TF;
			cout << "OK...\n";
		}
		*/
		
		for (int i = 0; i < n; i++)
		{
			double xc = Unif(gen) * (Unif(gen) - 0.5) * 20;
			double yc = Unif(gen) * (Unif(gen) - 0.5) * 12;
			double lx = Unif(gen); lx *= lx;
			double ly = Unif(gen); ly *= ly;
			lx = 0.1; ly = 0.1;
			TF.insert({ xc - lx, xc + lx, yc - ly, yc + ly }, 0);
		}


		for (int i = 0; i < n / 10; i++)
		{
			double yc = Unif(gen) * 5;
			double lx = 10 * Unif(gen)* Unif(gen);
			TF.insert({ 0, lx, yc, yc }, 0);
		}

		
		
		cout << TF << "\n";

		//	TF.insert({ 1, 2, 1, 1.6 }, nullptr);

		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";


		fBox2 R = TF.mainBoundingBox();
		R = mtools::zoomOut(R);
		Image im(10000, 10000);
		im.clear(RGBc::c_White);


		cout << "Drawing...\n";
		mtools::Chronometer();
		TF.drawTreeDebug(im, R, RGBc::c_Transparent);
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";


		cout << "Visiting...\n";
		mtools::Chronometer();
		cout << "visited = " << TF.iterate_intersect({ -5,5,0,5 }, [&](const TreeFigure<int, NN>::BoundedObject & bo) -> void { im.canvas_draw_box(R, bo.boundingbox, RGBc::c_Green.getOpacity(0.5f), true);  return; });
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";

		cout << "Visiting...\n";
		mtools::Chronometer();
		cout << "visited = " << TF.iterate_contained_in({ -5,5,0,5 }, [&](const TreeFigure<int, NN>::BoundedObject & bo) -> void { im.canvas_draw_box(R, bo.boundingbox, RGBc::c_Blue.getOpacity(0.5f), true);  return; });
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";

		cout << "Visiting...\n";
		mtools::Chronometer();
		cout << "visited = " << TF.iterate_contain({ 1,1.01,1.5,1.51 }, [&](const TreeFigure<int, NN>::BoundedObject & bo) -> void { im.canvas_draw_box(R, bo.boundingbox, RGBc::c_Yellow.getOpacity(0.2f), true);  return; });
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";

		/*
		{
		cout << "serializing...\n";
		OFileArchive ar("testTreeAR.txt");
		ar & TF;
		cout << "OK...\n";
		}
		*/
		auto P1 = makePlot2DImage(im);
		Plotter2D plotter;
		plotter[P1];
		plotter.autorangeXY();
		plotter.range().zoomOut();
		plotter.plot();

		mtools::cout << "Hello World\n";
		mtools::cout.getKey();
		return 0;
	}
