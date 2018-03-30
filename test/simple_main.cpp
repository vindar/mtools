
#include "mtools/mtools.hpp"
using namespace mtools;







class TestImage : public Image
{

public:


	/******************************************************************************************************************************************************
	*																				   																      *
	*                                                           BRESENHAM SEGMENT DRAWING                                                                 *
	*																																					  *
	*******************************************************************************************************************************************************/


	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}






};



int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	RGBc RR = RGBc::c_Red.getMultOpacity(0.5);
	RGBc GG = RGBc::c_Green.getMultOpacity(0.5);
	RGBc BB = RGBc::c_Blue.getMultOpacity(0.5);

	RGBc FF = RGBc::c_Yellow.getMultOpacity(0.5f);

	int64 L = 50;

	TestImage im(L, L);
	im.clear(RGBc(240,240,240));

	using namespace internals_bseg;




	fVec2 P1(10,10);

	fVec2 P2(37.49,25.49);

	fVec2 P3(13,20.99);

	im._bseg_draw(BSeg(P1, P2), true, RR);
	im._bseg_avoid1(BSeg(P1, P3), true, BSeg(P1, P2), true, GG);
	im._bseg_avoid11(BSeg(P2, P3), BSeg(P2, P1), true, BSeg(P3, P1), true, BB);

	im._bseg_fill_triangle(P1, P2, P3, FF);


	/*
	im.blendPixel({ 40, 9 }, BB);
	im.blendPixel({ 10, 9 }, BB);
	im.blendPixel({ 20, 9 }, BB);
	im.blendPixel({ 30, 9 }, BB);
	*/
	Plotter2D plotter;
	auto P = makePlot2DImage(im);
	plotter[P];
	plotter.range().setRange(fBox2{ -0.5, L - 0.5, -0.5, L - 0.5});

	plotter.gridObject(true);
	plotter.gridObject()->setUnitCells();

	plotter.plot();

	return 0;
	}

