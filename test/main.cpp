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


fVec2 prot(double a, fVec2 P)
	{
	return { P.X() * cos(a) + P.Y() * sin(a),-P.X() * sin(a) + P.Y() * cos(a) };
	}



void _fillSmoothQuarterCircle(tgx::Image<tgx::RGB32> & im, tgx::fVec2 C, float R, int quarter, bool vertical_center_line, bool horizontal_center_line, tgx::RGB32 color, float opacity = 1.0f)
	{
	// check radius >0  and im valid...
	const int dir_x = (quarter & 1) ? -1 : 1;
	const int dir_y = (quarter & 2) ? -1 : 1;
	auto B = im.imageBox();
	B &= tgx::iBox2(
		((dir_x > 0) ? (int)roundf(C.x - R) : (int)roundf(C.x) + ((vertical_center_line) ? 0 : 1)),
		((dir_x > 0) ? ((int)roundf(C.x) - ((vertical_center_line) ? 0 : 1)) : (int)roundf(C.x + R)),
		((dir_y > 0) ? ((int)roundf(C.y) + ((horizontal_center_line) ? 0 : 1)) : (int)roundf(C.y - R)),
		((dir_y > 0) ? (int)roundf(C.y + R) : (int)roundf(C.y) - ((horizontal_center_line) ? 0 : 1)));
	if (B.isEmpty()) return; 
	if (dir_y < 0) { tgx::swap(B.minY, B.maxY); }
	B.maxY += dir_y;
	if (dir_x < 0) { tgx::swap(B.minX, B.maxX); }
	B.maxX += dir_x;
	const float RT = (R < 0.5f) ? 4*R*R : (R + 0.5f);
	const float RA2 = RT*RT;
	const float RB2 = (R < 0.5f) ? -1 : (R - 0.5f)* (R - 0.5f);
	int i_min = B.minX;
	for (int j = B.minY; j != B.maxY; j += dir_y)
		{
		float dy2 = (j - C.y); dy2 *= dy2;
		for(int i = i_min; i != B.maxX; i += dir_x)
			{
			float dx2 = (i - C.x); dx2 *= dx2;
			const float e2 = dx2 + dy2;
			if (e2 >= RA2) { i_min = i + dir_x; continue; }
			if (e2 <= RB2) 
				{ 
				const int h = B.maxX - dir_x - i;
				if (h >= 0)
					im.drawFastHLine<false>({ i,j }, h + 1, color, opacity);
				else
					im.drawFastHLine<false>({ B.maxX - dir_x,j },  1 - h, color, opacity);
				break; 
				}
			const float alpha = RT - sqrtf(e2);
			im.drawPixel<false>({ i,j }, color, alpha*opacity);
			}
		}
	return;
	}

void _fillSmoothCircle(tgx::Image<tgx::RGB32>& im, tgx::fVec2 C, float R, tgx::RGB32 color, float opacity = 1.0f)
	{
	// check radius >0  and im valid...
	_fillSmoothQuarterCircle(im, C, R, 0, 1, 1, color, opacity);
	_fillSmoothQuarterCircle(im, C, R, 1, 0, 1, color, opacity);
	_fillSmoothQuarterCircle(im, C, R, 2, 1, 0, color, opacity);
	_fillSmoothQuarterCircle(im, C, R, 3, 0, 0, color, opacity);
	}




//
//  2    x=1, y=-1  |  3   x=-1; y=-1
//   ---------------------------------
//  0    x=1, y=1   |  1   x=-1, y=1
void _smoothQuarterCircle(tgx::Image<tgx::RGB32> & im, tgx::fVec2 C, float R, int quarter, bool vertical_center_line, bool horizontal_center_line, tgx::RGB32 color, float opacity = 1.0f)
	{
	// check radius >0  and im valid...
	const int dir_x = (quarter & 1) ? -1 : 1;
	const int dir_y = (quarter & 2) ? -1 : 1;
	auto B = im.imageBox();
	B &= tgx::iBox2(
		((dir_x > 0) ? (int)roundf(C.x - R - 0.5f) : (int)roundf(C.x) + ((vertical_center_line) ? 0 : 1)),
		((dir_x > 0) ? ((int)roundf(C.x) - ((vertical_center_line) ? 0 : 1)) : (int)roundf(C.x + R + 0.5f)),
		((dir_y > 0) ? ((int)roundf(C.y) + ((horizontal_center_line) ? 0 : 1)) : (int)roundf(C.y - R - 0.5f)),
		((dir_y > 0) ? (int)roundf(C.y + R + 0.5f) : (int)roundf(C.y) - ((horizontal_center_line) ? 0 : 1)));
	if (B.isEmpty()) return; 
	if (dir_y < 0) { tgx::swap(B.minY, B.maxY); }
	B.maxY += dir_y;
	if (dir_x < 0) { tgx::swap(B.minX, B.maxX); }
	B.maxX += dir_x;
	const float RA2 = (R < 1) ? (4*R*R) : (R + 1) * (R + 1);
	const float RB2 = (R < 1) ? -1 : (R - 1)* (R - 1);
	if (R < 1) opacity *= R; 
	int i_min = B.minX;
	for (int j = B.minY; j != B.maxY; j += dir_y)
		{
		float dy2 = (j - C.y); dy2 *= dy2;
		for(int i = i_min; i != B.maxX; i += dir_x)
			{
			float dx2 = (i - C.x); dx2 *= dx2;
			const float e2 = dx2 + dy2;
			if (e2 >= RA2) { i_min = i + dir_x; continue; }
			if (e2 <= RB2) break; 
			const float alpha = 1.0f - fabsf(R - sqrtf(e2));
			im.drawPixel<false>({ i,j }, color, alpha*opacity);
			}
		}
	return;
	}


void _smoothCircle(tgx::Image<tgx::RGB32>& im, tgx::fVec2 C, float R, tgx::RGB32 color, float opacity = 1.0f)
	{
	// check radius >0  and im valid...
	_smoothQuarterCircle(im, C, R, 0, 1, 1, color, opacity);
	_smoothQuarterCircle(im, C, R, 1, 0, 1, color, opacity);
	_smoothQuarterCircle(im, C, R, 2, 1, 0, color, opacity);
	_smoothQuarterCircle(im, C, R, 3, 0, 0, color, opacity);
	}












void _fillSmoothRoundedRect(tgx::Image<tgx::RGB32>& im, const tgx::iBox2& B, float corner_radius, tgx::RGB32 color, float opacity = 1.0f)
	{
	// check radius >0 and B not empty and im valid...
	float maxl = (B.maxX - B.minX) / 2.0f;
	float maxh = (B.maxY - B.minY) / 2.0f;
	corner_radius = tgx::min(corner_radius, tgx::min(maxl, maxh));

	const float eps = 0.5f;
	fVec2 P1(B.minX + corner_radius - eps, B.minY + corner_radius - eps);
	_fillSmoothQuarterCircle(im, P1, corner_radius, 2, false, false, color, opacity);

	fVec2 P2(B.maxX - corner_radius + eps, B.minY + corner_radius - eps);
	_fillSmoothQuarterCircle(im, P2, corner_radius, 3, false, false, color, opacity);

	fVec2 P3(B.maxX - corner_radius + eps, B.maxY - corner_radius + eps);
	_fillSmoothQuarterCircle(im, P3, corner_radius, 1, false, false, color, opacity);

	fVec2 P4(B.minX + corner_radius - eps, B.maxY - corner_radius + eps);
	_fillSmoothQuarterCircle(im, P4, corner_radius, 0, false, false, color, opacity);

	const int x1 = (int)roundf(B.minX + corner_radius - eps);
	const int x2 = (int)roundf(B.maxX - corner_radius + eps);
	im.fillRect(iBox2(x1, x2, B.minY, B.maxY), color, opacity);
	const int y1 = (int)roundf(B.minY + corner_radius - eps);
	const int y2 = (int)roundf(B.maxY - corner_radius + eps);
	im.fillRect(iBox2(B.minX, x1-1, y1,y2), color, opacity);
	im.fillRect(iBox2(x2+1, B.maxX, y1,y2), color, opacity);
	}



void _smoothRoundedRect(tgx::Image<tgx::RGB32>& im, const tgx::iBox2& B, float corner_radius, tgx::RGB32 color, float opacity = 1.0f)
	{
	// check radius >0 and B not empty and im valid...
	float maxl = (B.maxX - B.minX) / 2.0f;
	float maxh = (B.maxY - B.minY) / 2.0f;
	corner_radius = tgx::min(corner_radius, tgx::min(maxl, maxh));

	const float eps = 0;

	fVec2 P1(B.minX + corner_radius - eps , B.minY + corner_radius - eps);
	_smoothQuarterCircle(im, P1, corner_radius, 2, false, false, color, opacity);

	fVec2 P2(B.maxX - corner_radius + eps, B.minY + corner_radius - eps);
	_smoothQuarterCircle(im, P2, corner_radius, 3, false, false, color, opacity);

	fVec2 P3(B.maxX - corner_radius + eps, B.maxY - corner_radius + eps);
	_smoothQuarterCircle(im, P3, corner_radius, 1, false, false, color, opacity);

	fVec2 P4(B.minX + corner_radius - eps, B.maxY - corner_radius + eps);
	_smoothQuarterCircle(im, P4, corner_radius, 0, false, false, color, opacity);

	const int x1 = (int)roundf(B.minX + corner_radius - eps);
	const int x2 = (int)roundf(B.maxX - corner_radius + eps);
	const int y1 = (int)roundf(B.minY + corner_radius - eps);
	const int y2 = (int)roundf(B.maxY - corner_radius + eps);
	im.drawFastHLine<true>({ x1,B.minY }, x2 - x1 + 1, color, opacity);
	im.drawFastHLine<true>({ x1, B.maxY }, x2 - x1 + 1, color, opacity);
	im.drawFastVLine<true>({ B.minX,y1 }, y2 - y1 + 1, color, opacity);
	im.drawFastVLine<true>({ B.maxX,y1 }, y2 - y1 + 1, color, opacity);
	}


void testblend()
	{

//	ImageDisplay ID(LX, LY);
	ImageDisplay ID(320,240);

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
	
	/*
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
	//cout << "stack used = " << tgx_dst.fill<100000>({ 70,70 }, tgx::RGB32_Blue);
	//cout << "stack used = " << tgx_dst.fill<100000>({70,70}, tgx::RGB32_Blue);


	RGBc cc = RGBc::c_White.getMultOpacity(0.5f);
	RGBc cc2 = RGBc::c_Green.getMultOpacity(0.5f);

	double h = 0.5; 
	double l = 100; 
	double a = 30; 
	fVec2 C = { 100,100 };

	fVec2 P1(-l/2,h);
	fVec2 P2(l/2, h);
	fVec2 P3(l/2, -h);
	fVec2 P4(-l/2, -h);


	tgx::BSeg bs(tgx::fVec2{ (float)P1.X(), (float)P1.Y()}, tgx::fVec2{ (float)P2.X(), (float)P2.Y() });

	cout << bs.len(); 
//	dst.draw_filled_quad(P1 + C, P2 + C, P3 + C, P4 + C, cc2, cc2, true, true, 0);


	P1 = prot(a, P1) + C;
	P2 = prot(a, P2) + C;
	P3 = prot(a, P3) + C;
	P4 = prot(a, P4) + C;


//	dst.draw_filled_quad(P1, P2, P3, P4, cc  , cc, true, true, 0);

//	tgx_dst.drawWedgeLine(P1, P3, 10, 10, tgx::RGB32_White, 1.0f);
	
	auto ccc = tgx::RGB32_White;
	ccc = ccc.getMultOpacity(1.0f);

	auto ccc2 = tgx::RGB32_Red;
	ccc2 = ccc2.getMultOpacity(1.0f);


	float rr = 4; 
/*
	_quarterCircle(tgx_dst, { 100,100 }, rr, 1, 1, 1, 1, ccc);
	_quarterCircle(tgx_dst, { 100,100 }, rr, -1, 0, 1, 1, ccc);
	_quarterCircle(tgx_dst, { 100,100 }, rr, 1, 1, -1, 0, ccc);
	_quarterCircle(tgx_dst, { 100,100 }, rr, -1, 0, -1, 0, ccc);
*/

	//_fillSmoothCircle(tgx_dst, { 100,100 }, rr, ccc);
	//_smoothCircle(tgx_dst, { 150,100 }, rr, ccc);

	//tgx_dst.fillRect(tgx::iBox2(100, 200, 130, 180), tgx::RGB32_Blue);




		/*
	tgx_dst.fillCircle({ 100, 150 }, rr, ccc, ccc);

	tgx_dst.drawCircle({ 150, 150 }, rr, ccc);
	*/

//	dst.draw_circle({ 150,100 }, rr, RGBc::c_White);
//	tgx_dst.drawCircle({ 100, 100 }, 2, tgx::RGB32_White);


	float r = 0; 
	ID.setImage(&dst);
	ID.startDisplay();
	float eps = 0.1f;




	while (ID.isDisplayOn())
		{

		tgx_dst.fillScreen(tgx::RGB32_Black);
		_fillSmoothRoundedRect(tgx_dst, tgx::iBox2(100, 200, 130, 230), r, ccc2, 0.5f);
		_smoothRoundedRect(tgx_dst, tgx::iBox2(100, 200, 130, 230), r, ccc, 1.0f);
		dst.draw_thick_filled_ellipse({ 100, 40 }, 100, 30, 10, 10, RGBc::c_Red, RGBc::c_White, true, true, false);

		ID.redrawNow();

		r += eps;
		if (r > 50) { r = 50; eps = -eps; }
		if (r < 0) { r = 0; eps = -eps; }
		}
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


























