#include <mtools/mtools.hpp>
using namespace mtools;



double f(double x)
	{
	return sin(x);
	}


void test()
	{
	auto P = makePlot2DFun(f, "sin");
	auto Q = makePlot2DFun(f, "sin");

	Plotter2D plotter;
	plotter[P]; 
	plotter[Q];
	plotter.autorangeXY();

	plotter.plot(); 
	plotter.remove(P);
	}








Image im(500, 600);






void testImageDisplay()
{
	MT2004_64 gen;
	im.clear(RGBc::c_Yellow);
	im.draw_circle(iVec2{ 250,300 }, 200, RGBc::c_Red);
	im.draw_text({ 100,100 }, "Hello World\n", MTOOLS_TEXT_TOPLEFT, RGBc::c_Blue, 80);


	ImageDisplay ID(800, 600, 100, 100, "ImageDisplay",false);

	ID.allowUserSelection(true);
	ID.forceSelectionBeforeClosing(true);
	ID.setSelection(iBox2(0, 50, 100, 200));


	ID(im);

	cout << "START  !\n";


	ID.autoredraw(100);

	ID.startDisplay();

	while (ID.isDisplayOn()) 
		{ 
		double a = Unif(gen) * 500;
		double b = Unif(gen) * 600;
		double c = Unif(gen) * 10;

		int R = Unif(gen) * 255;
		int G = Unif(gen) * 255;
		int B = Unif(gen) * 255;
		RGBc col(R, G, B);

		im.draw_filled_circle({ a,b }, c, col, col);
		}


	cout << ID.getSelection(true) << "\n";

	return;
}





	




void testDelaunayVoronoi()
	{
	mtools::MT2004_64 gen(0);
	mtools::DelaunayVoronoi DV;

	// add 100 points uniformly distributed in [0,1]^2
	for (int i = 0; i < 100; i++) DV.DelaunayVertices.push_back(mtools::fVec2(Unif(gen), Unif(gen)));
		
	// compute the Delaunay trianulation and Voronoi diagram
	DV.compute(); 

	// draw the graphs
	mtools::Plotter2D plotter; 
	auto canvas = mtools::makeFigureCanvas(2);

	// draw the Delaunay triangulation
	int nb_D_e = (int)DV.DelaunayEdgesIndices.size();
	for (int k = 0; k < nb_D_e; k++)
		{
		mtools::iVec2 e = DV.DelaunayEdgesIndices[k];
		canvas(mtools::Figure::Line(DV.DelaunayVertices[e.X()], DV.DelaunayVertices[e.Y()], mtools::RGBc::c_Red), 0);
		}

	// draw the Voronoi diagram
	int nb_V_e = (int)DV.VoronoiEdgesIndices.size();
	for (int k = 0; k < nb_V_e; k++)
		{
		auto e = DV.VoronoiEdgesIndices[k];
		mtools::fVec2 P1 = DV.VoronoiVertices[e.X()];
		if (e.Y() == -1)
			{ // semi-infinite ray
			mtools::fVec2 N = DV.VoronoiNormals[e.X()];
			canvas(mtools::Figure::Line(P1, P1 + N, mtools::RGBc::c_Green), 1);
			}
		else
			{ // regular edge
			mtools::fVec2 P2 = DV.VoronoiVertices[e.Y()];
			canvas(mtools::Figure::Line(P1, P2, mtools::RGBc::c_Black), 1);
			}
		}

	// plot


	
	Drawer2D drawer(1000,1000,true,true);

	auto P = mtools::makePlot2DFigure(canvas, 4, "Delaunay Voronoi");

	drawer[P];
	drawer.range().setRange({ 0,1,0,1 });
	drawer.range().zoomOut(); 

	drawer.drawBackground(RGBc::c_White);
	drawer.drawAndSave("im.png");

	drawer.waitForClose();
	
	}











char bb[10000000];





namespace mtools
{


}





double sc(double s)
	{

	auto P = makePlot2DAxes();
	

	if (s == 0) return 1.0;
	return sin(s) / s;
	}


void testdrawer()
	{

	//Display2D disp;

	//disp.test();


	}








int a[100];

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows


	tgx::fVec4 V(1, 2,3, 5);

	mtools::fVec4 VV(V);

	auto W = (tgx::iVec4)VV;

	mtools::cout << W << "\n";





	tgx::Image<tgx::RGB565> im((void*)a, 320,240);

	cout << im.toString() << "\n";
	cout.getKey();
	return 0;

	testDelaunayVoronoi();
	
	return 0;

	Plotter2D plot; 


	testdrawer();


	return 0;

	SerialPort sp;

	sp.open("COM18", 2000000);

	while (1)
		{
		//std::this_thread::sleep_for(10);
	//	Sleep(10);

		const int l = 15; 
		cout << "- sending " << l << "\n";

		sp.write(l);
		sp.flush();

		Chrono ch;
		ch.reset();

		int r = 0; 

		while (r < l * 1024)
			{			
			if (sp.available()) sp.read();
			r++;		
			}

		int el = (int)ch.elapsed();
		cout << " received in " << el << "ms\n";
		}





	cout << "done !\n\n";
	cout.getKey(); 
	return 0; 

}


























