#include <mtools/mtools.hpp>
using namespace mtools;

//#include "buddha.h"



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





inline float _triangleAera(tgx::fVec2 P1, tgx::fVec2 P2, tgx::fVec2 P3)
	{
	return P1.x * (P2.y - P3.y) + P2.x * (P3.y - P1.y) + P3.x * (P1.y - P2.y);
	}



void plotRationalBezier2(tgx::Image<tgx::RGB32>& im, tgx::fVec2 P1, tgx::fVec2 P2, tgx::fVec2 PC, float w, tgx::RGB32 col)
{

	for (int i = 0; i < 10000; i++)
	{
		float t = i / 10000.0;
		tgx::fVec2 Q = (P1 * ((1 - t) * (1 - t)) +
			PC * w * (2 * t * (1 - t)) +
			P2 * (t * t)) / ((1 - t) * (1 - t) + w * 2 * t * (1 - t) + t * t);
		im.drawPixel(tgx::iVec2(Q), col);
	}
	return;
}




void plotRationalBezier(tgx::Image<tgx::RGB32>& im, tgx::fVec2 P1, tgx::fVec2 P2, tgx::fVec2 PC, float w, tgx::RGB32 col)
	{
	const float l = 0.25f; 
	float a = _triangleAera(P1, P2, PC);

	cout << sqrt(a * a / (P1 - P2).norm2()) << "\n";

	if (a * a < l * (P1 - P2).norm2())
		{
		cout << (P1-P2).norm() << "\n";
		im.drawSmoothLine(P1, P2, col);
		}
	else
		{
		float t = 0.5f; 

		float ua = (1-t) + w*t;
		float ub = w*(1-t) + t;
		float u3 = ((1-t)*ua + t*ub);
		
		tgx::fVec2 PA = (P1*(1-t) + PC*(w*t)) / ua;
		tgx::fVec2 PB = (PC*w*(1-t) + P2*t) / ub;
		tgx::fVec2 P3 = (PA*(1-t)*ua + PB*ub*t) / u3;

		float wa = ua/sqrtf(u3);
		float wb = ub/sqrtf(u3);

		plotRationalBezier(im, P1, P3, PA, wa, col);
		plotRationalBezier(im, P2, P3, PB, wb, col);
	}

	return;
	}






bool nextSplitRQB(tgx::fVec2 P1, tgx::fVec2 P2, tgx::fVec2 PC, float w, tgx::fVec2 & Q, tgx::fVec2 & PB, float & wb)
	{
	const int MAX_ITER = 20; 
	const float l = 0.25f;
	const float a = _triangleAera(P1, P2, PC);
	if (a * a < l * (P1 - P2).norm2())
		{ // done !
		return true;
		}
	float t = 0.5f;
	int d = 0; 
	while (1)
		{
		float ua = (1 - t) + w * t;
		float ub = w * (1 - t) + t;
		float u3 = ((1 - t) * ua + t * ub);

		tgx::fVec2 PA = (P1 * (1 - t) + PC * (w * t)) / ua;
		PB = (PC * w * (1 - t) + P2 * t) / ub;
		Q = (PA * (1 - t) * ua + PB * ub * t) / u3;
		
		const float a  = _triangleAera(P1, Q, PA);
		const float n2 = (P1 - Q).norm2();
		if ((n2 < 2)||(a * a < l * n2) || (d++ > MAX_ITER))
			{ // done !
			wb = ub / sqrtf(u3);
			return false;
			}
		t /= 2; 
		}
	}



void plotRationalBezier3(tgx::Image<tgx::RGB32>& im, tgx::fVec2 P1, tgx::fVec2 P2, tgx::fVec2 PC, float w, tgx::RGB32 col)
	{
	while (1)
		{
		tgx::fVec2 Q, PB;
		float wb;
		if (nextSplitRQB(P1, P2, PC, w, Q, PB, wb))
			{
			cout << "last = " << (P2 - P1).norm() << "\n";
			im.drawSmoothLine(P1, P2, col);
			return;
			}
		cout << (Q - P1).norm() << "\n";
		im.drawSmoothLine(P1, Q, col);
		P1 = Q; 
		PC = PB; 
		w = wb; 
		}
	}











void test_3()
{

//	mtools::cout << _triangleAera({ 0,5 }, { 100,5 }, {9, 6}) << "\n";
//	mtools::cout.getKey(); 


	//	testline(); 
	ImageDisplay ID(320, 240);
	Image dst(320, 240);
	Image sprite(30, 40);

	tgx::Image<tgx::RGB32> t(dst);
	tgx::Image<tgx::RGB32> s(sprite);

	t.fillScreen(tgx::RGB32_Black);


	{
	tgx::fVec2 P1(10, 10);
	tgx::fVec2 PC(200, 10);
	tgx::fVec2 PC2(100, 230);
	tgx::fVec2 P2(40, 130);
	float w = 2;

	t.fillSmoothCircle(P1, 2, tgx::RGB32_Green);
	t.fillSmoothCircle(P2, 2, tgx::RGB32_Green);
	t.fillSmoothCircle(PC, 2, tgx::RGB32_Green);
	t.fillSmoothCircle(PC2, 2, tgx::RGB32_Green);

	const float dd = 0; 



	t.drawSmoothThickCubicBezier(P1, P2, PC2, PC, 2.5, true, tgx::RGB32_Red);

//	t.drawCubicBezier(tgx::iVec2(P1), tgx::iVec2(P2), tgx::iVec2(PC2), tgx::iVec2(PC), true, tgx::RGB32_Blue, 0.5f);


	ID.setImage(&dst);
	ID.display();
	return;
	}
	







	tgx::iVec2 PA({ 8,3 });
	tgx::iVec2 PB({ 130,30 });
	tgx::iVec2 PC({ 70,110 });
	tgx::iVec2 PC2({ 70,110 });

	tgx::iVec2 PD({ 10,130 });
	tgx::iVec2 PE({ 2,60 });




	/*
	{
		float R = 50.0f;
		float C = 60.5;
		float rr = 4.0f;
		int o = 100;

		int N =  (int)( 3*R);

		tgx::fVec2 O = tgx::fVec2(2 + R, 2 + R);

		for (int j = 0; j < N; j++)
		{
			auto PA = PP(j - 1, N, C, R);
			auto PB = PP(j, N, C, R);
			auto PC = PP(j + 1, N, C, R);

			auto QA = PP(j - 1, N, C, R - rr);
			auto QB = PP(j, N, C, R - rr);
			auto QC = PP(j + 1, N, C, R - rr);


			t._bseg_avoid1(tgx::BSeg(PB,PC), tgx::BSeg(PB,PA), true, false, true, tgx::RGB32_Red, 1, o, true);
			t._bseg_avoid1(tgx::BSeg(QB, QC), tgx::BSeg(QB, QA), true, false, true, tgx::RGB32_Red, -1, o, true);
			t._bseg_avoid22(tgx::BSeg(QB, PB), tgx::BSeg(QB, QA), tgx::BSeg(QB, QC), tgx::BSeg(PB, PA), tgx::BSeg(PB, PC), true, true, true, true, tgx::RGB32_Red, 0, o, true);
			t._bseg_avoid22(tgx::BSeg(QB, PC), tgx::BSeg(QB, PB), tgx::BSeg(QB, QC), tgx::BSeg(PC, PB), tgx::BSeg(PC, QC), true, true, true, true, tgx::RGB32_Red, 0, o, true);


			t._bseg_fill_triangle(QB, PB, PC, tgx::RGB32_Red, o / 256.0f);
			t._bseg_fill_triangle(QB, PC, QC, tgx::RGB32_Red, o / 256.0f);


			//		t._bseg_avoid11(O, PB, PA, PA, true, true, tgx::RGB32_Red, 0, o, true);

					//t.drawPixel(tgx::iVec2(floorf(PB.x), floorf(PB.y)), tgx::RGB32_Lime);
		}
	}
	*/





	/*

		{
		tgx::fVec2 P2(10.2, 10.3);
		tgx::fVec2 P1(60, 70.3);

		t.drawSmoothLine(P1, P2, tgx::RGB32_Red, 1.0f);

			{
			mtools::Chrono ch;
			ch.reset();
			for (int i = 0; i < 10000; i++)
				{
				t.drawSmoothWedgeLine(P1, P2, 4, 15.2, true, tgx::RGB32_Green, 0.5f);
				}
			int el = ch.elapsed();
			cout << ch.elapsed() << "\n";
			}

			{
			mtools::Chrono ch;
			ch.reset();
			for (int i = 0; i < 10000; i++)
				{
				t.drawSmoothWedgeLine(P1, P2, 4, 15.2, false, tgx::RGB32_Red, 0.5f);
				}
			int el = ch.elapsed();
			cout << ch.elapsed() << "\n";
			}


			t._fillSmoothTriangle({ 50.0f, 50.5f } , { 123.0f, 100.0f } , {400.0f, 201.0f}, tgx::RGB32_Red, 0.5f);

		}

		*/

	tgx::fBox2 B(5.5, 40.5, 4.5, 30.5);
	//tgx::fBox2 B(-5, 40, 4, 30);

//	t.drawSmoothThickRoundRect(B, 15, 5, tgx::RGB32_Red, 0.3f);

//	t.drawTriangle({ 10, 5 }, { 80, 70 }, { 30, 120 }, tgx::RGB32_Red,1.0f);

	iVec2 P1(5, 5);
	iVec2 P2(50, 25);
	iVec2 P3(50, 100);
	iVec2 P4(10, 120);


	/*
	{
		mtools::Chrono ch;
		ch.reset();
		for (int i = 0; i < 40000; i++)
			{
			t.fillTriangle(P1, P2, P3, tgx::RGB32_Red, tgx::RGB32_Green);
			}
		int e = ch.elapsed();
		cout << "elapsed triangle old = " << e << "\n";
	}
	*/

//	t.drawSmoothTriangle(P1, P2, P3, tgx::RGB32_Red, 0.5f);


	//t.fillSmoothQuad({ -10, 5 }, { 80.5, 70.5 }, { 130, 120 }, { 30, 200 }, tgx::RGB32_Red);

//	dst.draw_thick_triangle({ 1,1 }, { 100, 3 }, { 30,90 }, 10, RGBc::c_Red, true, true);

//	t.fillSmoothTriangle({ 1,1 }, { 100, 3 }, { 30,90 }, tgx::RGB32_Red);

	tgx::RGB32 c = tgx::RGB32_Red;

	tgx::fVec2 tabP[4] =
		{
		{10, 10}, {150, 50}, {170,200}, {20,230}
		};

	tgx::iVec2 tabPi[4] =
		{
		{10, 10}, {150, 50}, {170,200}, {20,230}
		};


	//t.fillSmoothPolygon(4, tabP, c);

	//t.drawSmoothThickPolyline(4, tabP, 3.6, false, tgx::RGB32_Green, 1.0f);
//	t.drawPolyline(4, tabPi, tgx::RGB32_Green, 1.0f);
//	t.drawSmoothThickPolygon(4, tabP, 5, tgx::RGB32_Green, 0.5f);





	{
		tgx::fVec2 A(30, 30);
		tgx::fVec2 B(30, 70);
		tgx::fVec2 C(320, 50);

		//t.drawQuadBezier(tgx::iVec2(A), tgx::iVec2(B), tgx::iVec2(C), 1.0f, true, tgx::RGB32_Red, 0.5f);

		const int N = 1000;

		tgx::fVec2 tab[N];


		tab[0] = A;
		int nb = 1;

		const float LEN = 3;
		float delta = LEN / ((C - A).norm() + (C - B).norm()); 
		delta *= LEN * (((1 - delta) * (1 - delta)) * A + (delta * delta) * B + 2 * delta * (1 - delta) * C - A).invnorm_fast();
		float tt = 0; 

		while(1)
			{
			float uu = tt + delta; 
			tgx::fVec2 P = tab[nb-1];
			tgx::fVec2 Q = ((1 - uu) * (1 - uu)) * A + (uu * uu) * B + 2 * uu * (1 - uu) * C;
			float d2 = (P - Q).norm2();
			if (d2 > (LEN + 1)*(LEN+1))
				{ // too large
				delta /= 2; 
				continue;
				}			
			tt += delta;
			delta *= LEN*fast_invsqrt(d2);			

			if (tt >= 1)
				{
				if ((B - tab[nb - 1]).norm() <= 1)
					{
					tab[nb- 1] = B;
					cout << "ERASE PREVIOUS \n";
					}
				else
					{
					tab[nb++] = B;
					}
				cout << 1.0f << "   " << (tab[nb - 1] - tab[nb - 2]).norm() << " ERASE PREVIOUS\n";
				break;
				}
			tab[nb++] = Q;			
			cout << tt - delta << "    " << (tab[nb - 1] - tab[nb - 2]).norm() << "\n";
			}
		cout << "nb = " << nb << "\n";



		//t.fillSmoothThickPolygon(nb-1, tab, 3, tgx::RGB32_Green, tgx::RGB32_Red, 0.5f);

		//t.drawSmoothThickPolyline(nb, tab, 4, true, tgx::RGB32_Green, 0.5f);
		//t.drawSmoothThickPolygon(nb, tab, 4, tgx::RGB32_Red, 0.5f);
			{
			tgx::iVec2 tabi[1000];
			for (int i = 0; i < nb; i++)
				{
				tabi[i] = iVec2(tab[i]);
				}
		//t.drawPolygon(nb, tabi, tgx::RGB32_Green, 0.5f);
		//t.fillPolygon(nb, tabi, tgx::RGB32_Green, 0.5f);
			}

		//t.fillSmoothPolygon(nb,tab, tgx::RGB32_Red, 0.5f);
	
			/*
			const int nbb = 3;
			int n = 0; 
			t.fillSmoothThickPolygon(
				[&](tgx::fVec2 & P)
					{
					if (n >= nbb) n = 0;
					P.x = 100 + 60*cosf(2 * PI * n / nbb);
					P.y = 100 + 50*sinf(2 * PI * n / nbb);
					n++;
					return (n < nbb);
					},
			10 , tgx::RGB32_Green, tgx::RGB32_Red, 0.5f);
			*/


		//t.fillSmoothThickQuad({ 50, 150 }, { 200, 100 }, {100, 80},  {30, 30}, 5, tgx::RGB32_Green, tgx::RGB32_Red, 0.5f);



	    //t.fillSmoothThickRoundRect(tgx::fBox2(10.0, 100, 20, 50), 15, 1.51f, tgx::RGB32_Blue, tgx::RGB32_Red, 0.5f);
	    
	    
		t.fillSmoothThickEllipse({ 50, 50 }, { 30, 20 }, 5, tgx::RGB32_Green, tgx::RGB32_Red, 0.5f);


		//t.drawSmoothThickRoundRect(tgx::fBox2(10.5f, 100.5f, 20.5f, 50.5f), 8, 3, tgx::RGB32_Red, 0.5f);

		//t.fillSmoothRoundRect(tgx::fBox2(10.5f, 100.5f, 20.5f, 50.5f), 8, tgx::RGB32_Blue, tgx::RGB32_Red, 0.5f);


//			t.fillThickRect(tgx::iBox2(10, 100, 20, 50), 10, tgx::RGB32_Red, tgx::RGB32_Blue, 0.5f);
//			t.drawCircle({ 13,20 }, 5, tgx::RGB32_Green);
	}		
	


	ID.setImage(&dst);
	ID.display();
	}



	void printcolor(tgx::RGB565 col)
	{
		std::cout << "0x" << std::hex << col.val << "\n";
	}


int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	printcolor(tgx::RGB565_Black);
	printcolor(tgx::RGB565_White);
	printcolor(tgx::RGB565_Red);
	printcolor(tgx::RGB565_Blue);
	printcolor(tgx::RGB565_Green);
	printcolor(tgx::RGB565_Purple);
	printcolor(tgx::RGB565_Orange);
	printcolor(tgx::RGB565_Cyan);
	printcolor(tgx::RGB565_Lime);
	printcolor(tgx::RGB565_Salmon);
	printcolor(tgx::RGB565_Maroon);
	printcolor(tgx::RGB565_Yellow);
	printcolor(tgx::RGB565_Magenta);
	printcolor(tgx::RGB565_Olive);
	printcolor(tgx::RGB565_Teal);
	printcolor(tgx::RGB565_Gray);
	printcolor(tgx::RGB565_Silver);
	printcolor(tgx::RGB565_Navy);



	test_all();


	//test_0();
    //test_1();
	//test_2();
	test_3();

//	testblend(); 
	return 0; 




	return 0; 

}


























