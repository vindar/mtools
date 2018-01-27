
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:



	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}
	

			/**
			* Draw an ellipse. Alternative method that only draw the portion inside the box B.
			* Used for large ellipse (larger than the image size). 
			* Advantage : can draw with non-integer center and radii !
			**/
			template<bool blend, bool outline, bool fill, bool usepen>  inline  void _draw_ellipse2(iBox2 B, fVec2 P, double rx, double ry, RGBc color, RGBc fillcolor, int32 penwidth)
			{
				/*
				const int64 FALLBACK_MINRADIUS = 5;
				if (r < FALLBACK_MINRADIUS)
				{ // fallback for small value. 
					_draw_circle<blend, true, outline, fill, usepen>(P.X(), P.Y(), r, color, fillcolor, penwidth);
					return;
				}
				*/
				const double rx2 = ((double)rx)*rx;
				const double ry2 = ((double)ry)*ry;
				int64 xmin = B.min[0];
				int64 xmax = B.max[0];
				for (int64 y = B.min[1]; y <= B.max[1]; y++)
				{
					if (xmin > xmax) { xmin = B.min[0]; xmax = B.max[0]; }
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);
					double ly = dy2 - absdy + 0.25;
					double Ly = dy2 + absdy + 0.25;
					while (1)
					{
						double dx = (double)(xmin - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						if ((xmin == B.min[0]) || (lx/rx2 + ly/ry2 > 1.0)) break;
						xmin--;
					}
					while (1)
					{
						const double dx = (double)(xmin - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						const double Lx = dx2 + absdx + 0.25;
						if ((Lx/rx2 + Ly/ry2 <= 1.0) || (xmax < xmin)) break;
						if (outline) { if ((lx/rx2 + Ly/ry2 < 1.0) || (Lx/rx2 + ly/ry2 < 1.0)) { _updatePixel<blend, usepen, false, usepen>(xmin, y, color, 255, penwidth); } }
						xmin++;
					}
					while (1)
					{
						const double dx = (double)(xmax - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						if ((xmax == B.max[0]) || (lx/rx2 + ly/ry2 > 1.0)) break;
						xmax++;
					}
					while (1)
					{
						const double dx = (double)(xmax - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						const double Lx = dx2 + absdx + 0.25;
						if ((Lx/rx2 + Ly/ry2 <= 1.0) || (xmax < xmin)) break;
						if (outline) { if ((lx/rx2 + Ly/ry2 < 1.0) || (Lx/rx2 + ly/ry2 < 1.0)) { _updatePixel<blend, usepen, false, usepen>(xmax, y, color, 255, penwidth); } }
						xmax--;
					}

					if (fill) { if (xmin < xmax) { _hline<blend, false>(xmin, xmax, y, fillcolor); } }
				}
			}




			/**
			* Draw an anti-aliased circle. Alternative method that only draw the portion inside the box B.
			* Used for large circle (larger than the image size).
			**/
			template<bool blend, bool usepen> void _draw_ellipse2_AA(iBox2 B, fVec2 P, double rx, double ry, RGBc color, int32 penwidth)
			{
				const double ex2 = rx * rx;
				const double ey2 = ry * ry;
				const double exy2 = ex2 * ey2;
				const double Rx2 = (rx + 0.5)*(rx + 0.5);
				const double rx2 = (rx - 0.5)*(rx - 0.5);
				const double Ry2 = (ry + 0.5)*(ry + 0.5);
				const double ry2 = (ry - 0.5)*(ry - 0.5);
				const double rxy2 = rx2*ry2;
				const double Rxy2 = Rx2*Ry2;
				int64 xmin = B.min[0];
				int64 xmax = B.max[0];
				for (int64 y = B.min[1]; y <= B.max[1]; y++)
				{
					if (xmin > xmax) { xmin = B.min[0]; xmax = B.max[0]; }
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);
					const double v = ex2 * dy2;
					const double vv = ex2 * v;
					double ly = dy2 - absdy + 0.25;
					double Ly = dy2 + absdy + 0.25;
					const double g1 = (Rxy2 - Rx2 * ly) / Ry2;
					const double g2 = (rxy2 - rx2 * Ly) / ry2;
					while (1)
						{
						double dx = (double)(xmin - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						if ((xmin == B.min[0]) || (lx > g1)) break;
						xmin--;
						}
					while (1)
						{
						const double dx = (double)(xmin - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						const double Lx = dx2 + absdx + 0.25;
						if ((Lx < g2) || (xmax < xmin)) break;
						if  (lx < g1)
							{
							const double u = ey2 * dx2;
							const double uu = ey2 * u;
							double d = (u + v - exy2)/(2*sqrt(uu + vv));
							if (d < 0) d = -d;
							if (d < 1) { _updatePixel<blend, usepen, true, usepen>(xmin, y, color, 256 - (int32)(256 * d), penwidth); }
							}
						xmin++;
						}
					while (1)
						{
						const double dx = (double)(xmax - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						if ((xmax == B.max[0]) || (lx > g1)) break;
						xmax++;
						}
					while (1)
						{
						const double dx = (double)(xmax - P.X());
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx + 0.25;
						const double Lx = dx2 + absdx + 0.25;
						if ((Lx < g2) || (xmax < xmin)) break;
						if  (lx < g1)
							{
							const double u = ey2 * dx2;
							const double uu = ey2 * u;
							double d = (u + v - exy2) / (2 * sqrt(uu + vv));
							if (d < 0) d = -d;
							if (d < 1) { _updatePixel<blend, usepen, true, usepen>(xmax, y, color, 256 - (int32)(256 * d), penwidth); }
							}
						xmax--;
						}
				}
			}




	/**
	* Fill the interior of a circle.
	*
	* The circle border is not drawn, use draw_filled_circle to draw both border and interior simultaneously.
	*
	* @param	P			   position of the center.
	* @param	r			   radius.
	* @param	color_interior color of the interior.
	* @param	blend		   true to use blending.
	*/
	inline void fill_circle_new(iVec2 P, int64 r, RGBc color_interior, bool blend)
		{
		if (isEmpty() || (r < 1)) return;
		iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
		iBox2 imBox = imageBox();
		iBox2 B = intersectionRect(circleBox, imBox);
		if (B.isEmpty()) return; // nothing to draw. 
		if (circleBox.isIncludedIn(imBox))
			{ // circle is completely inside the image
			if (blend) _draw_circle<true, false, false, true, false>(P.X(), P.Y(), r, RGBc::c_White, color_interior, 0); else _draw_circle<false, false, false, true, false>(P.X(), P.Y(), r, RGBc::c_White, color_interior, 0);
			return;
			}
		// partial drawing, use alternative drawing method
		if (blend) _draw_circle2<true, false, true, false>(B, P, r, RGBc::c_White, color_interior, 0); else _draw_circle2<false, false, true, false>(B, P, r, RGBc::c_White, color_interior, 0);
		return;
		}


	/**
	* Draw a filled circle. The border and the interior color may be different.
	*
	* @param	P			  	position of the center.
	* @param	r			  	radius.
	* @param	color_border  	color for the border.
	* @param	color_interior	color of the interior.
	* @param	blend		  	true to use blending.
	**/
	inline void draw_filled_circle_new(iVec2 P, int64 r, RGBc color_border, RGBc color_interior, bool blend)
		{
		if (isEmpty() || (r < 1)) return;
		iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
		iBox2 imBox = imageBox();
		iBox2 B = intersectionRect(circleBox, imBox);
		if (B.isEmpty()) return; // nothing to draw. 
		if (circleBox.isIncludedIn(imBox))
			{ // circle is completely inside the image
			if (blend) _draw_circle<true, false, true, true, false>(P.X(), P.Y(), r, color_border, color_interior, 0); else _draw_circle<false, false, true, true, false>(P.X(), P.Y(), r, color_border, color_interior, 0);
			return;
			}
		// partial drawing, use alternative drawing method
		if (blend) _draw_circle2<true, true, true, false>(B, P, r, color_border, color_interior, 0); else _draw_circle2<false, false, true, false>(B, P, r, color_border, color_interior, 0);
		return;
		}


	/**
	* Draw a circle.
	*
	* @param	P				position of the center.
	* @param	r				radius.
	* @param	color			color to use.
	* @param	blend			true to use blending.
	* @param	antialiasing	true to use antialiasing.
	* @param	penwidth		The pen width (0 = unit width)
	**/
	inline void draw_circle_new(iVec2 P, int64 r, RGBc color, bool blend, bool antialiasing, int32 penwidth = 0)
		{
		if (isEmpty() || (r < 1)) return;
		iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
		iBox2 imBox = imageBox();
		if (penwidth > 0)
			{ // large pen
			_correctPenOpacity(color, penwidth);
			circleBox.enlarge(penwidth);
			iBox2 B = intersectionRect(circleBox, imBox);
			if (B.isEmpty()) return; // nothing to draw.
			if (circleBox.isIncludedIn(imBox))
				{ // included
				if (antialiasing)
					{
					if (blend) _draw_circle_AA<true, false, true>(P.X(), P.Y(), r, color, penwidth); else _draw_circle_AA<false, false, true>(P.X(), P.Y(), r, color, penwidth);
					}
				else
					{
					if (blend) _draw_circle<true, false, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth); else _draw_circle<false, false, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth);
					}
				return;
				}
			// not included
			if (B.area() * 64 > circleBox.area())
				{ // still faster to use draw everything using the first method and checking the range
				if (antialiasing)
					{
					if (blend) _draw_circle_AA<true, true, true>(P.X(), P.Y(), r, color, penwidth); else _draw_circle_AA<false, true, true>(P.X(), P.Y(), r, color, penwidth);
					}
				else
					{
					if (blend) _draw_circle<true, true, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth); else _draw_circle<false, true, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth);
					}
				return;
				}
			// use alternate method
			if (antialiasing)
				{
				if (blend) _draw_circle2_AA<true, true>(B, P, r, color, penwidth); else _draw_circle2_AA<false, true>(B, P, r, color, penwidth);
				}
			else
				{
				if (blend) _draw_circle2<true, true, false, true>(B, P, r, color, RGBc::c_White, penwidth); else _draw_circle2<false, true, false, true>(B, P, r, color, RGBc::c_White, penwidth);
				}			
			return;
			}
		iBox2 B = intersectionRect(circleBox, imBox);
		if (B.isEmpty()) return; // nothing to draw.
		if (circleBox.isIncludedIn(imBox))
			{ // included
			if (antialiasing)
				{
				if (blend) _draw_circle_AA<true, false, false>(P.X(), P.Y(), r, color, 0); else _draw_circle_AA<false, false, false>(P.X(), P.Y(), r, color, 0);
				}
			else
				{
				if (blend) _draw_circle<true, false, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0); else _draw_circle<false, false, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0);
				}
			return;
			}
		// not included
		if (B.area() * 64 > circleBox.area())
			{ // still faster to use draw everything using the first method and checking the range
			if (antialiasing)
				{
				if (blend) _draw_circle_AA<true, true, false>(P.X(), P.Y(), r, color, 0); else _draw_circle_AA<false, true, false>(P.X(), P.Y(), r, color, 0);
				}
			else
				{
				if (blend) _draw_circle<true, true, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0); else _draw_circle<false, true, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0);
				}
			return;
			}
		// use alternate method
		if (antialiasing)
			{
			if (blend) _draw_circle2_AA<true, false>(B, P, r, color, 0); else _draw_circle2_AA<false, false>(B, P, r, color, 0);
			}
		else
			{
			if (blend) _draw_circle2<true, true, false, false>(B, P, r, color, RGBc::c_White, 0); else _draw_circle2<false, true, false, false>(B, P, r, color, RGBc::c_White, 0);
			}
		return;
		}

	};




	MT2004_64 gen;

#define NN 1



	/* fast inverse squere root */


	int main(int argc, char *argv[])
	{

		MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


		int64 NS = 100000000;
			{
			double tot = 0.0;
			Chronometer();
			for (int64 n = 1; n < NS; n++)
				{
				tot += 1 / sqrt((double)n);
				}
			auto res = Chronometer();
			mtools::cout << "tot = " << tot << " in " << durationToString(res,true) << "\n\n";
			}

			{
				double tot = 0.0;
				Chronometer();
				for (int64 n = 0; n < NS; n++)
				{
					tot += fast_invsqrt((float)n);
				}
				auto res = Chronometer();
				mtools::cout << "tot = " << tot << " in " << durationToString(res,true) << "\n\n";
			}


			cout.getKey();
			return 0;

		{
			for (int i = 0; i < 20; i++)
				{
				double a = Unif(gen) * 100; 
				cout << a << "\t - \t" << 1 / sqrt(a) << "\t - \t" << Q_rsqrt(a) << "\n";
				}
			cout.getKey();
			return 0;
		}


		{
			TestImage im(1000, 600);
			im.clear(RGBc::c_White);


			iBox2 B(100, 800, 100, 500);
			fVec2 Pa(300, 250);
			fVec2 Pb(400, 250);
			fVec2 Pc(500, 250);
			fVec2 Pd(600, 350);
			double rx = 50;
			double ry = 50;

			//im.draw_box(B, RGBc::c_Gray, true);
			//im._draw_ellipse2<true, true, false, false>(B, Pa, rx, ry, RGBc::c_Red, RGBc::c_Blue, 0);
			im._draw_ellipse2_AA<true, false>(B, Pa, rx, ry, RGBc::c_Red, 0);
			//im._draw_ellipse2<true, true, false, false>(B, Pb, rx, ry, RGBc::c_Red, RGBc::c_Blue, 0);

			//im.draw_ellipse(Pc, rx, ry, RGBc::c_Red, true, false);
			im.draw_ellipse(Pd, rx, ry, RGBc::c_Red, true, true);

			//im.draw_dot({ 304, 259 }, RGBc::c_Blue, true, 0);

			auto P = makePlot2DImage(im, 6);   // Encapsulate the image inside a 'plottable' object.	
			Plotter2D plotter;              // Create a plotter object
			plotter.axesObject(false);      // Remove the axe system.
			plotter[P];                     // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}


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
