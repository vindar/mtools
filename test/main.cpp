#include <mtools/mtools.hpp>
using namespace mtools;

#include "buddha.h"


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




float test(const tgx::RGBf & col)
	{
	return col.B;
	}


tgx::RGBf mult_op(tgx::RGBf colA, tgx::RGBf colB)
	{
	return tgx::RGBf(colA.R * colB.R, colA.G * colB.G, colA.B * colB.B);
	}


void testblend()
	{

	ImageDisplay ID(LX, LY);

	Image src(200, 200);
	tgx::Image<tgx::RGB32> tgx_src(src);
	tgx_src.fillScreenHGradient(tgx::RGB32_Purple, tgx::RGB32_Orange);
	tgx_src.fillCircle({ 100, 100 }, 80, tgx::RGB32_Salmon, tgx::RGB32_Black);

	Image dst(320, 240);
	tgx::Image<tgx::RGB32> tgx_dst(dst);
	tgx_dst.fillScreenVGradient(tgx::RGB32_Green, tgx::RGB32_White);

	//tgx_dst.blit(tgx_src, { 60 , 20 }, mult_op);


	tgx_dst.blitScaledRotated(tgx_src, tgx_src.dim()/2, tgx_dst.dim()/2, 1.0f, 45, [](tgx::RGB32 src, tgx::RGB32 dst) { return tgx::RGB32(src.G, src.R, src.B); });

	//tgx_dst.copyFrom(tgx_dst, [](tgx::RGB32 src, tgx::RGB32 dst) {return tgx::RGB32(src.G, src.R, src.B); });


	ID.setImage(&dst);
	ID.display(); 
	return;
	}



template<typename color_t> 
void drawBottomRightCorner(tgx::Image<color_t> & im, int x, int y, int r, color_t color)
	{
	tgx::Image<color_t> sub_im = im.getCrop({ x, x + r + 1, y, y + r + 1}); // create a sub image for clipping
	sub_im.drawCircle({ 0,0 }, r, color); // draw the circle on the sub image 
	}

class TT
{
public:


	TT(double c) : a(c) {}


	double ff(double x)
		{
		return a * x;
		}


	RGBc getcol(int64 x, int64 y)
		{
		return (x * x + y * y < 100*a) ? RGBc::c_Green : RGBc::c_Transparent;
		}


private:
	double a; 
};


RGBc getcol(int64 x, int64 y)
	{
	return (x * x + y * y < 100) ? RGBc::c_Red : RGBc::c_Transparent;
	}


        template<typename T> class GetC
            {
            static void * dumptr;
            typedef typename std::decay<mtools::RGBc>::type decayrgb;

			/*
            template<typename U> static decltype((*(U*)(0)).getColor(iVec2{ 0,0 }, dumptr)) vers1(int);
            template<typename> static metaprog::no vers1(...);
            static const bool version1 = std::is_same<typename std::decay<decltype(vers1<T>(0))>::type, decayrgb>::value;


			
            template<typename U> static decltype((*(U*)(0)).getColor(iVec2{ 0,0 })) vers2(int);
            template<typename> static metaprog::no vers2(...);
            static const bool version2 = std::is_same<typename std::decay<decltype(vers2<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0)).getColor(0, 0, dumptr)) vers3(int);
            template<typename> static metaprog::no vers3(...);
            static const bool version3 = std::is_same<typename std::decay<decltype(vers3<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0)).getColor(0, 0)) vers4(int);
            template<typename> static metaprog::no vers4(...);
            static const bool version4 = std::is_same<typename std::decay<decltype(vers4<T>(0))>::type, decayrgb >::value;

			
            template<typename U> static decltype((*(U*)(0))(iVec2{ 0,0 }, dumptr)) vers5(int);
            template<typename> static metaprog::no vers5(...);
            static const bool version5 = std::is_same<typename std::decay<decltype(vers5<T>(0))>::type, decayrgb >::value;

			*/
			
            template<typename U> static decltype((*(U*)(0))(iVec2{ 0,0 })) vers6(int);
            template<typename> static metaprog::no vers6(...);
            static const bool version6 = std::is_same<typename std::decay<decltype(vers6<T>(0))>::type, decayrgb >::value;

			/*
            template<typename U> static decltype((*(U*)(0))(0, 0, dumptr)) vers7(int);
            template<typename> static metaprog::no vers7(...);
            static const bool version7 = std::is_same<typename std::decay<decltype(vers7<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0))(0, 0)) vers8(int);
            template<typename> static metaprog::no vers8(...);
            static const bool version8 = std::is_same<typename std::decay<decltype(vers8<T>(0))>::type, decayrgb >::value;
			*/


            public:

				static const bool has_getColor = version1; // | version2 | version3 | version4 | version5 | version6 | version7 | version8;

            };

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	//testblend(); 
	//return 0; 


	TT tt(0.5);

	{


		auto hh = std::bind(&TT::ff, &tt, _1);
		cout << hh(3.0);

		auto P1 = makePlot2DFun([&](double x) {return x * x; }, std::string("lambda"));
		auto P2 = makePlot2DFun(&TT::ff, tt, 0, 1, "lambda");

		//auto P3 = makePlot2DLattice(&TT::getcol, &tt, "zz");	
		
		auto bb = std::bind(&TT::getcol, tt, std::placeholders::_1, std::placeholders::_2);
		

		cout << "has color : " << GetC<decltype(bb)>::has_getColor << "\n";


		//auto P3 = makePlot2DLattice(bb);
			

		//cout << bb(2, 4);

		Plotter2D plotter;
		plotter[P1];
		plotter[P2];
		//plotter[P3];
		plotter.range().setRange({ -20,20, -20, 20 });
		plotter.plot();
	}



	// setup the 3D renderer.
	renderer.setViewportSize(LX, LY);
	renderer.setOffset(0, 0);
	renderer.setImage(&tgxim);
	renderer.setZbuffer(zbuf);
	renderer.setPerspective(45, ((float)LX) / LY, 1.0f, 100.0f);  // set the perspective projection matrix.     
	renderer.setMaterial(tgx::RGBf(0.85f, 0.55f, 0.25f), 0.2f, 0.7f, 0.8f, 64); // bronze color with a lot of specular reflexion. 
	renderer.setShaders(TGX_SHADER_GOURAUD);
	
	size_t ram1u = 17;
	size_t ram2u = 18;
	auto bb = tgx::cacheMesh(&buddha, (void*)cb, 240000, nullptr, 0, "VNTIF", &ram1u, &ram2u);
	
	
	cout.getKey();

	ID.setImage(&fbim);
	ID.startDisplay();
	while (ID.isDisplayOn())
		{
		tgxim.fillScreen(tgx::RGB32_White);

		renderer.clearZbuffer();

		renderer.setMaterialColor(tgx::RGBf(0, 1, 0));
		renderer.setModelPosScaleRot({ 0, a, -35 }, { 10,10,10 }, 0);
		renderer.drawMesh(bb, false);
		
		//tgxim._drawCircleHelper<true>(100, 100, 50, 15, tgx::RGB32_Black);

		tgxim.drawLine({ 100, 100 },{200, 100},tgx::RGB32_Green);
		tgxim.drawLine({ 100, 100 }, { 100, 200 }, tgx::RGB32_Green);

		drawBottomRightCorner(tgxim, 100, 100, 50, tgx::RGB32_Black);


		ID.redrawNow();

		a += 0.1f;
		Sleep(100);
		}


	//cout << "done !\n\n";
	//cout.getKey(); 
	return 0; 

}


























