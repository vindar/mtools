#include <mtools/mtools.hpp>
using namespace mtools;

#include "buddha.h"



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

	Image dst(320, 240);
	tgx::Image<tgx::RGB32> tgx_dst(dst);

	tgx_dst.fillScreen(tgx::RGB32_Black);

	/*
	tgx_dst.drawQuadBezier({10,10}, {100, 10}, {200,100}, 1.0f, true, tgx::RGB32_White, 0.5f);

	tgx_dst.drawQuadBezier({ 10,10 }, { 100, 10 }, { 200,100 }, 100.0f, true, tgx::RGB32_Red);


	tgx_dst.drawCubicBezier({ 10,10 }, { 100, 10 }, { 200,100 }, { 0,200 }, true, tgx::RGB32_Green);


	*/
	tgx::iVec2 tabP[4] = { {20, 20}, {100, 20}, {100, 100}, {20, 100}  };


	tgx::iVec2 tabP2[6] = { {20, 20}, {60, 20}, {100, 20}, {100, 100}, {60,100}, {20, 100} };

	
	//tgx_dst.drawQuadSpline(4, tabP, true, tgx::RGB32_Blue, 0.4f);
	
	tgx_dst.drawClosedSpline(4, tabP, tgx::RGB32_Orange, 0.4f);

	tgx_dst.drawClosedSpline(6, tabP2, tgx::RGB32_Red, 0.4f);



	tgx_dst.drawCubicSpline(4, tabP, true, tgx::RGB32_Yellow, 0.4f);


	//tgx_dst.drawPolygon(4, tabP,  tgx::RGB32_Red, 0.5f);
	
	//tgx_dst.drawPolygon(4, tabP,  tgx::RGB32_Yellow, 0.5f);


	int k = 5000; 
	for (int u = 0; u < k; u++)
		{
		tgx_dst.drawPixel((int)(Unif(gen) * 100), (int)(Unif(gen) * 100), tgx::RGB32_Olive);
		}


	tgx_dst.drawPixel(70,70, tgx::RGB32_Black);

	int st;
	/*
	mtools::Chrono ch;
	for (int i = 0; i < 1; i++)
		{
		st = tgx_dst.fill<100000>({ 70,70 }, tgx::RGB32_Blue);
		}
	cout << ch << "\n";
	*/
	cout << "stack used = " << tgx_dst.fill<100000>({ 70,70 }, tgx::RGB32_Blue);
	cout << "stack used = " << tgx_dst.fill<100000>({70,70}, tgx::RGB32_Blue);


	ID.setImage(&dst);
	ID.display(); 
	return;
	}









int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	 

	testblend(); 
	return 0; 




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



		ID.redrawNow();

		a += 0.1f;
		Sleep(100);
		}


	//cout << "done !\n\n";
	//cout.getKey(); 
	return 0; 

}


























