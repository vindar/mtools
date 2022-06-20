#include <mtools/mtools.hpp>
using namespace mtools;

#include "buddha.h"



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



int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	//testblend(); 
	//return 0; 






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
		


		ID.redrawNow();

		a += 0.1f;
		Sleep(100);
		}


	//cout << "done !\n\n";
	//cout.getKey(); 
	return 0; 

}


























