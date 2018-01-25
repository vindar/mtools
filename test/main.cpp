
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:



	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}
	

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


	int main(int argc, char *argv[])
	{
		MTOOLS_SWAP_THREADS(argc, argv); // required on OSX, does nothing on Linux/Windows
		mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode

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
		TF.drawTreeDebug(im, R, RGBc::c_TransparentWhite);
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
