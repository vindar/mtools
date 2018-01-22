
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



	MT2004_64 gen(0);

	struct TA
	{
		TA(uint64 v) : val(v)
			{
		//	cout << "ctor = " << val << "\n";
			}

		~TA()
			{
			val = gen();
	//		cout << "DTOR = " << val << "\n";
			}


		void aa() { cout << " - " << val << " - \n"; }

	uint64 val;

	};

std::set<TA*> tset;


CstSizeMemoryPool<sizeof(TA), 100> mempool;



void addAndDecimate(int64 N)
	{
	// add new element
	for (int i = 0; i < N; i++)
		{
		uint64 v = gen();
		TA * p = mempool.allocate<TA>(v);
		tset.insert(p);
		}

	auto it = tset.begin();
	while(it != tset.end())
		{
		auto pit = it;
		it++; 
		if (Unif(gen) < 0.1)
			{
			mempool.destroyAndFree(*pit);
			tset.erase(pit);
			}
		}
	}


size_t nbi = 0;

void fun(void * q)
	{
	TA * p = (TA*)q;
	//p->val = gen();
	nbi++; 
	}





void lol()
{
	nbi = 0;
	int64 N = 1000;

	addAndDecimate(N);
	addAndDecimate(N);
	addAndDecimate(N);
	addAndDecimate(N);
	addAndDecimate(N);
	addAndDecimate(N);
	addAndDecimate(N);
	addAndDecimate(N);


	cout << "size of tset : " << tset.size() << "\n";
	cout << "allocated    : " << mempool.size() << "\n";
	cout << "iterated : " << mempool.iterateOver(fun) << "\n";
	cout << "count iterated = " << nbi << "\n\n";

	cout.getKey();

}



	int main(int argc, char *argv[])
	{

	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv, true);

	lol();
	lol();
	lol();
	lol();
	lol();
	lol();
	lol();
	lol();
	lol();

	cout << "number destroyed = " << mempool.destroyAndFreeAll<TA>(false) << "\n";
	tset.clear();
	cout << "size of tset : " << tset.size() << "\n";
	cout << "allocated    : " << mempool.size() << "\n";

	cout.getKey();
	cout.getKey();
	cout.getKey();


	}