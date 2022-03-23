#include <mtools/mtools.hpp>
using namespace mtools;

#include "buddha.h"



const int LX = 1000; 
const int LY = 1000; 

Image fbim(LX, LY);
tgx::Image<tgx::RGB32> tgxim(fbim);



const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD;
tgx::Renderer3D<tgx::RGB32, LX, LY, LOADED_SHADERS> renderer; 

float zbuf[LX * LY]; 

ImageDisplay ID(LX, LY);


float a = 0;

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	// setup the 3D renderer.
	renderer.setOffset(0, 0);
	renderer.setImage(&tgxim);
	renderer.setZbuffer(zbuf);
	renderer.setPerspective(45, ((float)LX) / LY, 0.1f, 1000.0f);  // set the perspective projection matrix.     
	renderer.setMaterial(tgx::RGBf(0.85f, 0.55f, 0.25f), 0.2f, 0.7f, 0.8f, 64); // bronze color with a lot of specular reflexion. 
	renderer.setShaders(TGX_SHADER_GOURAUD);



	ID.setImage(&fbim);
	ID.startDisplay();
	while (ID.isDisplayOn())
		{
		tgxim.fillScreen(tgx::RGB32_Blue);

		renderer.clearZbuffer();

		renderer.setModelPosScaleRot({ 0, 0.5f, -35 }, { 13,13,13 }, a);

		renderer.drawMesh(&buddha, false);

		ID.redrawNow();

		a += 0.5f;
		}


	//cout << "done !\n\n";
	//cout.getKey(); 
	return 0; 

}


























