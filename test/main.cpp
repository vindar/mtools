
#include "mtools.hpp"
using namespace mtools;



// structure at each site of Z^2.
struct siteInfo
{
	siteInfo() : up(1.0), right(1.0), V(0) {}    // ctor, set the intial weights of the edges to 1
	double up, right;   // weights of the up and right edges.
	int64 V;            // number of visits to the site.
};

iVec2 pos;                      // position of the walk
double delta = 1.0;             // reinf param
int64 maxV = 0;                 // max local time on site
double maxE = 1.0;              // max weight on edges 
double logscale = 1.2;          // logarithmic scale
int64 range = 1;                // number of distincts site visited. 
iBox2 R;                        // rectangle enclosing the trace of the walk
Grid_basic<2, siteInfo> G;      // the grid
MT2004_64 gen;                  // random generator
Image image;       // image for detailled plot



struct LERRWPlot
{
	// site are colored w.r.t. the local time of the walk.
	static RGBc getColor(iVec2 pos)
	{
		const siteInfo * S = G.peek(pos);
		if ((S == nullptr) || (S->V == 0)) return RGBc::c_TransparentWhite;
		return RGBc::jetPaletteLog(S->V, 0, maxV, logscale); // light logarithmic scale
	}


	/* detail : image associated with a site */
	static const Image * getImage(mtools::iVec2 p, mtools::iVec2 size)
	{
		const siteInfo * S = G.peek(p);
		if ((S == nullptr) || (S->V == 0)) return nullptr;
		EdgeSiteImage ES;

		
		//if (pos == p) { ES.bkColor(RGBc::c_Black.getOpacity(0.5)); }      // uncomment to display the current position
		ES.site(true, RGBc::jetPaletteLog(S->V, 0, maxV, logscale));
		ES.text(mtools::toString(S->V)).textColor(RGBc::c_White);
		double right = S->right;
		double up = S->up;
		double left = G(p.X() - 1, p.Y()).right;
		double down = G(p.X(), p.Y() - 1).up;
		if (up > 1.0) { ES.up(ES.EDGE, RGBc::jetPaletteLog(up / maxE, logscale)); ES.textup(mtools::toString((int64)((up - 1) / delta))); }
		if (down > 1.0) { ES.down(ES.EDGE, RGBc::jetPaletteLog(down / maxE, logscale)); }
		if (left > 1.0) { ES.left(ES.EDGE, RGBc::jetPaletteLog(left / maxE, logscale)); ES.textleft(mtools::toString((int64)((left - 1) / delta))); }
		if (right > 1.0) { ES.right(ES.EDGE, RGBc::jetPaletteLog(right / maxE, logscale)); }
		

		/*
		ES.siteColor(RGBc::c_Orange);
		//ES.up(ES.EDGE, RGBc::c_Red);
		ES.down(ES.EDGE, RGBc::c_Blue);
		//ES.left(ES.EDGE, RGBc::c_Green);
		ES.right(ES.EDGE, RGBc::c_Yellow);
		*/
		ES.makeImage(image, size);
		return(&image);
	}

};

// Simulate the LERRW with reinforcment parameter delta for steps unit of time.
void makeLERRW(uint64 steps, double d)
{
	delta = d;
	cout << "Simulating ...";
	ProgressBar<uint64> PB(steps, "Simulating...");
	Chronometer();
	maxV = 0;
	maxE = 1.0;
	range = 0;
	R.clear();
	G.reset();
	image.resizeRaw(1, 1);
	pos = { 0, 0 };
	// main loop
	for (uint64 n = 0; n < steps; n++)
	{
		PB.update(n);
		siteInfo & S = G[pos];                          // info at the current site
		if (S.V == 0) { range++; }                      // increase range if new site
		if ((++S.V) > maxV) { maxV = S.V; }             // check if largest local time yet.
		R.swallowPoint(pos);                            // update the rectangle enclosing the trace
		double & right = S.right;                       // get a reference to the weight
		double & up = S.up;                             // of the 4 adjacent edges of the
		double & left = G(pos.X() - 1, pos.Y()).right;  // current position.
		double & down = G(pos.X(), pos.Y() - 1).up;     //
		double e = Unif(gen)*(left + right + up + down);
		if (e < left)
		{
			left += delta; if (left > maxE) { maxE = left; }
			pos.X()--;
		}
		else {
			if (e < (left + right))
			{
				right += delta; if (right > maxE) { maxE = right; }
				pos.X()++;
			}
			else {
				if (e < (left + right + up))
				{
					up += delta; if (up > maxE) { maxE = up; }
					pos.Y()++;
				}
				else
				{
					down += delta; if (down > maxE) { maxE = down; }
					pos.Y()--;
				}
			}
		}
	}
	watch("maxV", maxV);
	watch("maxE", maxE);
	watch("mlogscale", logscale);
	// update one last time for the terminating point
	siteInfo & S = G[pos];
	if (S.V == 0) { range++; }
	if ((++S.V) > maxV) { maxV = S.V; }
	R.swallowPoint(pos);
	// done, print statistic and plot the walk. 
	PB.hide();
	cout << "ok. Completed in " << Chronometer() / 1000.0 << " seconds.\n\nStatistics:\n";
	cout << "  - Reinforcement parameter = " << delta << "\n";
	cout << "  - Number of steps = " << steps << "\n";
	cout << "  - Range = " << range << " site visited inside " << R << "\n";
	cout << "  - Max site local time = " << maxV << "\n";
	cout << "  - Max edge weight = " << maxE << " (" << (int64)((maxE - 1) / delta) << " visits)\n";
	cout << "  - Current position of the walk = (" << pos.X() << "," << pos.Y() << ")\n";
	//std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid";
	//G.save(fn); // save the grid in fn
	//cout << "- saved in file " << fn << "\n";
	Plotter2D Plotter;
	auto L = makePlot2DLattice((LERRWPlot*)nullptr, std::string("LERRW-d") + mtools::toString(delta));
	L.setImageType(L.TYPEIMAGE);
	Plotter[L];
	Plotter.gridObject(true)->setUnitCells();
	Plotter.range().setRange(zoomOut(fBox2(R)));
	Plotter.plot();
	return;
}





struct A
	{

	char a[5];
	};









/**
* A Functor class to create a sort for fixed sized arrays/containers with a
* compile time generated Bose-Nelson sorting network.
* \tparam NumElements  The number of elements in the array or container to sort.
* \tparam T            The element type.
* \tparam Compare      A comparator functor class that returns true if lhs < rhs.
*/



template <unsigned NumElements, class Compare = void> class StaticSort
	{

	template <class A, class C> struct Swap
		{
		template <class T> inline void s(T &v0, T &v1)
			{
			T t = Compare()(v0, v1) ? v0 : v1; // Min
			v1 = Compare()(v0, v1) ? v1 : v0; // Max
			v0 = t;
			}

		inline Swap(A &a, const int &i0, const int &i1) { s(a[i0], a[i1]); }
		};

	template <class A> struct Swap <A, void>
		{
		template <class T> inline void s(T &v0, T &v1)
			{
			// Explicitly code out the Min and Max to nudge the compiler
			// to generate branchless code.
			T t = v0 < v1 ? v0 : v1; // Min
			v1 = v0 < v1 ? v1 : v0; // Max
			v0 = t;
			}

		inline Swap(A &a, const int &i0, const int &i1) { s(a[i0], a[i1]); }
		};

	template <class A, class C, int I, int J, int X, int Y> struct PB
		{
		inline PB(A &a)
			{
			enum { L = X >> 1, M = (X & 1 ? Y : Y + 1) >> 1, IAddL = I + L, XSubL = X - L };
			PB<A, C, I, J, L, M> p0(a);
			PB<A, C, IAddL, J + M, XSubL, Y - M> p1(a);
			PB<A, C, IAddL, J, XSubL, M> p2(a);
			}
		};

	template <class A, class C, int I, int J> struct PB <A, C, I, J, 1, 1>
		{
		inline PB(A &a) { Swap<A, C> s(a, I - 1, J - 1); }
		};

	template <class A, class C, int I, int J> struct PB <A, C, I, J, 1, 2>
		{
		inline PB(A &a) { Swap<A, C> s0(a, I - 1, J); Swap<A, C> s1(a, I - 1, J - 1); }
		};

	template <class A, class C, int I, int J> struct PB <A, C, I, J, 2, 1>
		{
		inline PB(A &a) { Swap<A, C> s0(a, I - 1, J - 1); Swap<A, C> s1(a, I, J - 1); }
		};

	template <class A, class C, int I, int M, bool Stop = false> struct PS
		{
		inline PS(A &a)
			{
			enum { L = M >> 1, IAddL = I + L, MSubL = M - L };
			PS<A, C, I, L, (L <= 1)> ps0(a);
			PS<A, C, IAddL, MSubL, (MSubL <= 1)> ps1(a);
			PB<A, C, I, IAddL, L, MSubL> pb(a);
			}
		};

	template <class A, class C, int I, int M> struct PS <A, C, I, M, true>
		{
		inline PS(A &a) {}
		};

	public:

		/**
		* Sorts the array/container arr.
		* \param  arr  The array/container to be sorted.
		*/
		template <class Container> inline void operator() (Container &arr) const
			{
			PS<Container, Compare, 1, NumElements, (NumElements <= 1)> ps(arr);
			};

		/**
		* Sorts the array arr.
		* \param  arr  The array to be sorted.
		*/
		template <class T> inline void operator() (T *arr) const
			{
			PS<T*, Compare, 1, NumElements, (NumElements <= 1)> ps(arr);
			};
};

#include <iostream>
#include <vector>
/*
int main(int argc, const char * argv[])
{
	enum { NumValues = 32 };

	// Arrays
	{
		int rands[NumValues];
		for (int i = 0; i < NumValues; ++i) rands[i] = rand() % 100;
		std::cout << "Before Sort: \t";
		for (int i = 0; i < NumValues; ++i) std::cout << rands[i] << " ";
		std::cout << "\n";
		StaticSort<NumValues> staticSort;
		staticSort(rands);
		std::cout << "After Sort: \t";
		for (int i = 0; i < NumValues; ++i) std::cout << rands[i] << " ";
		std::cout << "\n";
	}

	std::cout << "\n";

	// STL Vector
	{
		std::vector<int> rands(NumValues);
		for (int i = 0; i < NumValues; ++i) rands[i] = rand() % 100;
		std::cout << "Before Sort: \t";
		for (int i = 0; i < NumValues; ++i) std::cout << rands[i] << " ";
		std::cout << "\n";
		StaticSort<NumValues> staticSort;
		staticSort(rands);
		std::cout << "After Sort: \t";
		for (int i = 0; i < NumValues; ++i) std::cout << rands[i] << " ";
		std::cout << "\n";
	}

	return 0;
}
*/






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




	




int main(int argc, char *argv[])
{

	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv, true);



	TestImage im(1000, 1000);

	im.clear(RGBc::c_White);

	
	int64 NS = 10000; 
	int64 m = 1000000;

	int64 x0 = 200000;
	Chronometer();
	for (int64 n = 0; n < NS; n++)
		{
		im.draw_circle({ -x0 ,300 }, x0 + 300, RGBc::c_Cyan, true,true,0);
		//im.draw_circle({ -x0 ,320 }, x0 + 300, RGBc::c_Cyan, true, true, 0);
		//		im.fill_triangle({ -m*n - 1000, -m*n - 1000 }, { m*n + 1000, m*n + 1000 }, { 400,300 }, RGBc::c_Blue,false);
		}

	cout << " done in << " << Chronometer() << "\n";
	
	


	//im.draw_circle({ 300,300 }, 5, RGBc::c_Blue.getOpacity(0.5), true, false, 0);
	//im.draw_circle({ 311,300 }, 5, RGBc::c_Red.getOpacity(0.5), true, false, 0);
	// 
	
	int64 rr = 12;


	//im.fill_circle_new({ 400,300 }, 100, RGBc::c_White, false);
	//im.draw_filled_circle_new({ 400,300 }, 600, RGBc::c_Red.getOpacity(0.1), RGBc::c_Blue, true);
	//im.draw_filled_circle({ 380,300 }, 100, RGBc::c_Red.getOpacity(0.1), RGBc::c_Blue, true);
	//im.draw_circle({ 400,300 }, 100, RGBc::c_Green.getOpacity(0.1), true, false);
	//im.draw_filled_circle_new({ 5,7 }, 256, RGBc::c_Green.getOpacity(0.5), RGBc::c_Red.getOpacity(0.5), true);

	auto P1 = makePlot2DImage(im, 1, "Image");

	Plotter2D plotter; 

	plotter[P1];
	plotter.autorangeXY();
	plotter.plot();

	return 0;

	/*
	A TT[3];

	cout << "sizeof(A) = " << sizeof(A) << "\n";
	cout << "sizeof(A[3]) = " << sizeof(TT) << "\n";
	cout << "align = " << metaprog::reqAlign<A>() << "\n";
	cout.getKey();
	*/
	
	return 0;





	cout << "hello\n"; 
	IntegerEmpiricalDistribution ED; 
	
	ED[-65539];
	ED[-65540];
	ED[-65537];
	ED[-65536];
	ED[-65535];
	ED[-65534];
	ED[-65533];

	ED[-3];
	ED[-3];
	ED[-1];


	ED[0];	ED[0]; ED[0]; ED[0];

	ED[1];	ED[1];
	ED[3];

	ED[123456];
	ED[123457];

	ED[222222223];

	ED.insert_plus_infinity();
	ED.insert_plus_infinity();
	ED.insert_minus_infinity();

	ED.save_csv_format("test.txt");

	cout << "yop\n";
	cout.getKey();

	return 0; 

	/*
	cimg_library::CImg<unsigned char> imm("lenna.jpg");
	auto P3 = makePlot2DCImg(imm);

	Plotter2D plotter;
	plotter[P3];

	plotter.autorangeXY();
	plotter.plot();
	*/

	
	cout << "*******************************************************\n";
	cout << " Simulation of a Linearly Reinforced Random Walk on Z^2\n";
	cout << "*******************************************************\n\n";
	double delta = arg('d', 2.0).info("reinforcement parameter");
	int64 N = arg('N', 1000000000).info("number of steps of the walk");
	makeLERRW(N, delta);
	return 0;

	
	}

/* end of file main.cpp */




/*
#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;






double sinfun(double x)
{
	return 2 * sin(x);
}


std::vector<double> tab;


int main(int argc, char *argv[])
	{
	MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc, argv, true); // parse the command line, interactive mode


	tab.resize(100);
	for (int i = 0; i < 100; i++)
		{
		tab[i] = cos((double)i / 10);
		}


	auto P1 = makePlot2DFun(sinfun, "sin");
	auto P2 = makePlot2DVector(tab, 0, 10, true, "vec");


	Image imm("lenna.jpg");
	auto P3 = makePlot2DImage(imm);


	Plotter2D plotter;


	plotter[P1];
	plotter[P2];
	plotter[P3];


	plotter.autorangeXY();
	plotter.plot();

	
	return 0;
	}

	*/

/* end of file main.cpp */
















