#include <mtools/mtools.hpp>
using namespace mtools;

//#include "buddha.h"


#include "FreeSansBold12pt7b.h"
#include "FreeSerif9pt7b.h"
#include "Picopixel.h"
#include "TomThumb.h"

#include "font_Courgette_lite.h"
#include "font_Roboto_Bold_AA2.h"
#include "font_FontdinerSwanky_AA4.h"

mtools::MT2004_64 gen(123);

using namespace std::placeholders;


const int LX = 1000; 
const int LY = 1000; 

Image fbim(LX, LY);
tgx::Image<tgx::RGB32> tgxim(fbim);



typedef uint16_t ZBUF_t;

const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD | TGX_SHADER_FLAT;
tgx::Renderer3D<tgx::RGB32, LOADED_SHADERS, ZBUF_t> renderer;

ZBUF_t zbuf[LX * LY];

ImageDisplay ID(LX, LY);


float a = 0;

char cb[1000000];





/* CREATION OF IMAGES AND SUB - IMAGE */
template<typename color_t> void tgx_test_section0()
	{
	tgx::Image<color_t> im1;
	tgx::Image<color_t> im2((tgx::RGB565*)nullptr, 100, 200);
	tgx::Image<color_t> im3((tgx::RGB565*)nullptr, { 100,200 }, 2);
	tgx::Image<color_t> im4(im3, {0, 50, 20, 100});
	tgx::Image<color_t> im5(im4);
	im1 = im5;
	im2.set((tgx::RGB565*)nullptr, 300, 200);
	im3.set((tgx::RGB565*)nullptr, {100, 50});
	im4.crop({5, 6, 10, 20});
	im5 = im1.getCrop({ 6, 7,8, 9 }, true);
	im3 = im2(tgx::iBox2{ 6, 7, 8, 9 });
	im4 = im2(6, 80, 8, 900);
	volatile bool b = im3.isValid();
	im4.setInvalid();
	}


void test_all()
	{
	tgx_test_section0<tgx::RGB32>();

	}














void test_0()
	{
	ImageDisplay ID(320, 240);
	Image dst(320, 240);

	tgx::Image<tgx::RGB32> t(dst);


	auto b = t(10, 30, 40, 70 );

	cout << b << "\n";
	cout << t.lx() << "\n";
	cout << t.ly() << "\n";
	cout << t.width() << "\n";
	cout << t.height() << "\n";
	cout << t.stride() << "\n";
	cout << t.dim() << "\n";
	cout << t.imageBox() << "\n";
	cout << t.data() << "\n";
	
	t.fillScreen(tgx::RGB32_Black);

	t.drawPixel({ 100,100 }, tgx::RGB32_Red);
	t.drawPixel({ 110,110 }, tgx::RGB32_Red, 0.5f);
	t.drawPixel({ 50,100 }, tgx::RGB32_Red);
	t.drawPixel({ 60,110 }, tgx::RGB32_Red, 0.5f);

	cout << t.readPixel({ 100, 100 }) << "\n";

	cout << t({ 50, 100 }) << "\n";
	cout << t({ 60, 11 }) << "\n";

	t.iterate([](tgx::iVec2 pos, tgx::RGB32& col) {cout << pos << "  " << col << "\n";  return true; },{100, 103,100,102});

	ID.setImage(&dst);
	ID.display(); 
	}


void test_1()
	{
	ImageDisplay ID(320, 240);
	Image dst(320, 240);
	Image sprite(30, 40);

	tgx::Image<tgx::RGB32> t(dst);
	tgx::Image<tgx::RGB32> s(sprite);

	s.fillScreen(tgx::RGB32_Blue);
	s.drawCircle({ 5,5 }, 20, tgx::RGB32_Red);
	s.drawCircle({ 20,15}, 5, tgx::RGB32_Green);

	t.fillScreen(tgx::RGB32_White);

	t.blit(s, { 300,230 });

	t.blit(s, { -10,20 }, [](tgx::RGB32 A, tgx::RGB32 B) {return A; });

	t.blitMasked(s, tgx::RGB32_Blue, { -10,80 } );

	t.blitMasked(s, tgx::RGB32_Red, { 100,100 }, 0.5f);

	t.fillRect({ 305, 320, 230, 240 }, tgx::RGB32_Yellow);

	t.blitBackward(s, { -0, -0 });

	t.fillRect( { 150, 200, 150, 200 }, tgx::RGB32_Black);

	t.blit(s, { 151,151 });

	t.blitBackward(s, { 150, 150 });


	t.blitScaledRotated(s, { 0,0 }, { 100, 100 }, 10.0f, 45, 0.4f);

	t.blitScaledRotated(s, { 15,20 }, { 100, 100 }, 0.5f, -30, [](tgx::RGB32 A, tgx::RGB32 B) {return tgx::RGB32_Lime; });

	s.fillScreen(tgx::RGB32_Green);
	s.drawRect( { 5, 25, 5, 35 }, tgx::RGB32_Orange, 1.0f);

	t.blitScaledRotatedMasked(s, tgx::RGB32_Green, { 15,20 }, { 40, 40 }, 1, 60, 0.5f);



	t.fillScreen(tgx::RGB32_Black);
	t.copyFrom(s,1.0f); // default blending does not work because drawTexturedQuad need fix !

	cout << t.reduceHalf();
	cout << t.reduceHalf();

	t.drawCircle({ 80,80 }, 100, tgx::RGB32_Orange);

	cout << t.fill({ 85, 85 },tgx::RGB32_Orange, tgx::RGB32_Black);
	ID.setImage(&dst);
	ID.display();

	}



void test_2()
	{
	ImageDisplay ID(320, 240);
	Image dst(320, 240);
	Image sprite(30, 40);

	tgx::Image<tgx::RGB32> t(dst);
	tgx::Image<tgx::RGB32> s(sprite);


	t.fillScreenHGradient(tgx::RGB32_Green, tgx::RGB32_Yellow);
	t.fillScreenVGradient(tgx::RGB32_Blue, tgx::RGB32_Orange);
	t.fillScreen(tgx::RGB32_Black);

	t.drawFastHLine({ 3,3 }, 3, tgx::RGB32_White);
	t.drawFastVLine({ 3,3 }, 4, tgx::RGB32_Blue, 0.5f);
	t.drawFastHLine({ 3,5 }, 3, tgx::RGB32_White,0.5f);
	t.drawFastVLine({ 6,3 }, 5, tgx::RGB32_Orange);


	tgx::iVec2 PA({ 8,3 });
	tgx::iVec2 PB({ 130,30 });
	tgx::iVec2 PC({ 100,110 });
	tgx::iVec2 PD({ 10,130 });
	tgx::iVec2 PE({ 2,60 });



	t.drawPixel(PA, tgx::RGB32_Red);
	t.drawPixel(PB, tgx::RGB32_Red);
	t.drawSegment(PA, false, PB, true, tgx::RGB32_White, 0.5f);



	tgx::iVec2 TT[4] = { PA,PB,PC , PD };
	tgx::iVec2 TT5[5] = { PA,PB,PC , PD, PE };

	t.drawPolyline(4, TT, tgx::RGB32_Green, 0.5f);
	t.drawPolygon(4, TT, tgx::RGB32_Blue, 0.5f);
	t.drawQuadBezier(PA, PC, PB, 1.0f, true, tgx::RGB32_Green, 0.2f);
	t.drawQuadBezier(PA, PC, PB, 6.0f, false, tgx::RGB32_Red);
	t.drawCubicBezier(PA, PD, PB, PC, true, tgx::RGB32_Yellow, 0.5f);
	t.drawQuadSpline(5, TT5, true, tgx::RGB32_Lime, 0.3f);
	t.drawCubicSpline(5, TT5, true, tgx::RGB32_Orange);
	t.drawClosedSpline(5, TT5, tgx::RGB32_Maroon, 0.5f);

	ID.setImage(&dst);
	ID.display();
	}




tgx::fVec2 PP(int j, int N, float C, float R)
	{
	const float a = (float)((2 * M_PI * j) / N);
	return tgx::fVec2(C + R * cosf(a), C + R * sinf(a));
	}



mtools::Image im1(300, 300);
mtools::Image im2(300, 300);
tgx::Image<tgx::RGB32> tim1(im1);
tgx::Image<tgx::RGB32> tim2(im2);









bool collide(tgx::Image<tgx::RGB32>& im)
	{
	bool bug = false;
	im.iterate(
		[&](tgx::iVec2 pos, tgx::RGB32 col)
		{
		if ((col.R > 0) && (col.G > 0))
			{
			bug = true;  return false;
			}
			return true;
		}
		);
	return bug;
	}








void test_3()
{

	//	testline(); 
	ImageDisplay ID(320, 240);
	Image dst(320, 240);
	Image sprite(30, 40);

	tgx::Image<tgx::RGB32> t(dst);
	tgx::Image<tgx::RGB32> s(sprite);

	ID.setImage(&dst);
	ID.startDisplay();


	tgx::fVec2 C(100 + 10*Unif(gen), 100.5 + 10 * Unif(gen));
	float R = 100 * (Unif(gen) + 0.1f);

	/*
	float a = M_PI*2*Unif(gen);
	while (1)
		{
		a += 0.0001f;
		tgx::fVec2 A(C.x + R * cos(a), C.y + R * sin(a));
		tgx::BSeg seg(C, A);

		t.fillScreen(tgx::RGB32_Black);
		t._bseg_draw(seg, true, true, tgx::RGB32_Red, 0, 128, true);

		int kx, ky, mino, maxo;
		seg.equation(kx, ky, mino, maxo, 1);

		t.iterate(
			[&](tgx::iVec2 pos, tgx::RGB32 & col)
			{
				int o = kx * pos.x + ky * pos.y;
				if ((o < mino) || (o > maxo))
					col.blend(tgx::RGB32_Green, 0.5f);
				return true;
			}
			);

		if (collide(t))
			{
			cout << "Found !\n";
			ID.redrawNow();
			}

		}

		*/

	t.drawSmoothCircle({ 100,100 }, 100, tgx::RGB32_Red);

	float a1 = 332;
	float a2 = 45.001;


	while (1)
		{

		a2 += 0.14578f;
		a1 += 0.1f;


		t.fillScreen(tgx::RGB32_Black);
		//t.fillSmoothCirclePie(C, R, a1, a2, tgx::RGB32_Orange, 0.5f);
		//t.fillSmoothCirclePie(C, R, a2, a1, tgx::RGB32_Cyan, 0.5f);
		

		t.fillSmoothThickCircle(C, 100, 5.5, tgx::RGB32_White, tgx::RGB32_Red, 0.5f);

		ID.redrawNow();
		}


	t.fillSmoothCirclePie(C, R, a1, a2, tgx::RGB32_Green, 0.5f);


	/*
	//t._rectifyAngles(a1, a2); 

	const float rad1 = a1*M_2PI / 360;
	const float rad2 = a2*M_2PI / 360;
	fVec2 A1(C.x + R * sin(rad1), C.y - R * cos(rad1));
	fVec2 A2(C.x + 2*R * sin(rad2), C.y - 2*R * cos(rad2));

	t.drawSmoothLine(C, A1, tgx::RGB32_White);
	t.drawSmoothLine(C, A2, tgx::RGB32_White);
	t.drawSmoothCircle(C, R, tgx::RGB32_White, 0.5f);

	
	for(int i=0; i<4; i++)
		{
		float m, M;
		switch(i)
			{
			case 0: m = 180; M = 270; break;
			case 1: m = 90; M = 180; break;
			case 2: m = 270; M = 360; break;
			default: m = 0; M = 90; break;
			}
		bool b1 = (a1 >= m) && (a1 <= M);
		bool b2 = (a2 >= m) && (a2 <= M);
		if (b1)
			{
			if (b2)
				t._fillSmoothQuarterCircleInterHP2(i, C, R, tgx::RGB32_Blue, 0.5f, tgx::BSeg(C, A1), 1, tgx::BSeg(C, A2), -1);
			else
				t._fillSmoothQuarterCircleInterHP1(i, C, R, tgx::RGB32_Blue, 0.5f, tgx::BSeg(C, A1), 1);
			}
		else 
			{
			if (b2)
				t._fillSmoothQuarterCircleInterHP1(i, C, R, tgx::RGB32_Blue, 0.5f, tgx::BSeg(C, A2), -1);
			else
				{
				switch (i)
					{
					case 0:
						{
						if (a1 > 270) continue;
						if ((a2 > 90) && (a2 < 180)) continue;
						break;
						}
					case 1:
						{
						if ((a1 > 180) && (a1 < 270)) continue;
						if (a2 < 90) continue;
						break;
						}

					case 2:
						{
						if (a1 < 90) continue;
						if ((a2 > 180) && (a2 < 270)) continue;
						break;
						}

					default:
						{
						if ((a1 > 90) && (a1 < 180)) continue;
						if (a2 > 270) continue;
						break;
						}
					}
				t._fillSmoothQuarterCircleInterHP2(i, C, R, tgx::RGB32_Blue, 0.5f, tgx::BSeg(C, A1), 1, tgx::BSeg(C, A2), -1);
				}
			}
		}

		*/
/*
	t._fillSmoothQuarterCircleInterHP2(0, C, R, tgx::RGB32_Blue, 0.5f, tgx::BSeg(C, A1), 1, tgx::BSeg(C, A2), 1);
	t._fillSmoothQuarterCircleInterHP2(1, C, R, tgx::RGB32_Green, 0.5f, tgx::BSeg(C, A1), 1, tgx::BSeg(C, A2),1);
	t._fillSmoothQuarterCircleInterHP2(2, C, R, tgx::RGB32_Red, 0.5f, tgx::BSeg(C, A1), 1, tgx::BSeg(C, A2), 1);
	t._fillSmoothQuarterCircleInterHP2(3, C, R, tgx::RGB32_Yellow, 0.5f, tgx::BSeg(C, A1), 1, tgx::BSeg(C, A2), 1);
	*/



	/*
	{
		auto font = font_Roboto_Bold_AA2_12;
		iVec2 pos = { 50, 50 };
		auto B = t.measureText(text, pos, font, false);
		t.fillRect(B, tgx::RGB32_Green, 0.3f);
		t.drawText(text, pos , tgx::RGB32_White, font, false);
	}
	
	*/

	tgx::iVec2 tabI[6] = { tgx::iVec2(10, 40), tgx::iVec2(40, 5), tgx::iVec2(110, 30), tgx::iVec2(150,70), tgx::iVec2(100, 150), tgx::iVec2(20, 130) };
	tgx::fVec2 tabF[6];
	for (int k = 0; k < 6; k++) 
		{ 
		tabF[k] = tgx::fVec2(tabI[k]); 
	//	t.fillSmoothCircle(tabF[k], 2, tgx::RGB32_Yellow);
		}



//	t.drawSmoothWedgeLine(tabF[0], tabF[2], 0, tgx::END_ROUNDED, 20, tgx::END_ROUNDED, tgx::RGB32_Red, 0.5f);

	

//t.drawQuadSpline(6, tabI, true, tgx::RGB32_Green, 1.0f);

	//t.drawSmoothThickPolyline(6, tabF, 8, tgx::END_ARROW_SKEWED_2, tgx::END_STRAIGHT, tgx::RGB32_Red, 0.5f);



	//t.fillSmoothThickClosedSpline(6, tabF, 8, tgx::RGB32_Blue, tgx::RGB32_Red, 0.5f);

	//t.drawSmoothThickQuadSpline(6, tabF, 6, tgx::END_ROUNDED, tgx::END_STRAIGHT, tgx::RGB32_Red, 0.5f);
	//t.drawSmoothThickCubicSpline(6, tabF, 6, false, true, tgx::RGB32_Red, 0.5f);
	//t.drawCubicSpline(6, tabI,true, tgx::RGB32_Red, 0.1f);


	//t.drawClosedSpline(6, tabI, tgx::RGB32_Green, 1.0f);

	//t.drawSmoothThickClosedSpline(6, tabF, 3, tgx::RGB32_Red, 0.5f);
	
	//t.fillSmoothClosedSpline(6, tabF, tgx::RGB32_Red, 0.5f);
//	t.drawSmoothThickQuadSpline(6, tabF, 6, tgx::END_STRAIGHT,  tgx::END_ROUNDED, tgx::RGB32_Red, 0.5f);

//	t.drawSmoothThickCubicBezier(tabF[0], tabF[4], tabF[3], tabF[5], 8, tgx::END_ROUNDED, tgx::END_ARROW_1, tgx::RGB32_Red, 0.5f);
	//t.drawSmoothThickCubicBezier(tabF[0], tabF[4], tabF[3], tabF[5], 8, true, false, tgx::RGB32_Red, 0.5f);


//	t.drawSmoothWedgeLine(tabF[0], tabF[1], 25, true, 10, false, tgx::RGB32_Red, 0.5f);


	ID.setImage(&dst);
	ID.display();
	return;
	}
	



int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows




	test_all();


	//test_0();
    //test_1();
	//test_2();
	test_3();

//	testblend(); 
	return 0; 




	return 0; 

}


























