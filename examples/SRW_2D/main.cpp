/***********************************************
 * project: SRW_2D
 *
 * Simulation of a simple random walk on Z¨2
 *
 * date: 2018-03-23
 ***********************************************/

#include "mtools/mtools.hpp" 

using namespace mtools;

Grid_basic<2, std::pair<int64, RGBc> > grid; // grid, stores last hitting time, color)

MT2004_64 gen;	// RNG


/* return the color of a site */
RGBc getColor(const iVec2 & pos)
	{
	auto p1 = grid.peek(pos);
	if ((p1 == nullptr) || (p1->first == 0)) return RGBc::c_Transparent; // never visited = transparent
	return p1->second; // return the associated color
	}


/* make the walk*/
bool make_walk(int64 N, int64 sqr_rad)
	{
	bool loc = false;
	const int64 L = N - N / 3;
	const float op = 0.5f;
	grid.reset();
		{
		ProgressBar<int64> PB(0, N);
		iVec2 pos(0,0);
		for (int64 i = 0; i < N; i++)
			{
			if ((i > L)&&(pos.norm2() < sqr_rad)) loc = true;
			auto & g = grid.get(pos);
			if (g.first == 0)
				{ // first visit
				g.first = i + 1;
				g.second = RGBc::jetPalette(i, 0, N).getMultOpacity(op);
				}
			else
				{ // subsequent visits
				g.first = i + 1;
				g.second.blend(RGBc::jetPalette(i, 0, N).getMultOpacity(op));
				}

			PB.update(i);
			double a = Unif(gen);
			if (a < 0.25)
				{
				pos.X()++;
				}
			else if (a < 0.5)
				{
				pos.X()--;
				}
			else if (a < 0.75)
				{
				pos.Y()++;
				}
			else
				{
				pos.Y()--;
				}
			}
		}
		return loc;
	}


int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc,argv); // required on OSX, does nothing on Linux/Windows
    mtools::parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
	// parameters
	int64 N = arg("N", 100000000).info("number of steps of the walk");
	bool localised = arg("loc", false).info("make localized walk");
	int64 sqr_rad = N/10;
	
	// do the simulation
	cout << "simulating : ";
	while ((!make_walk(N, sqr_rad)) && (localised)) { cout << "."; }
	cout << " ok\n";

	// display the result
	Plotter2D plotter; 
	auto P = makePlot2DPixel(getColor, 2); 
	plotter[P];
	plotter.axesObject(false);
	plotter.solidBackGroundColor(RGBc::c_Black);
	iBox2 B;
	grid.getPosRange(B);
	plotter.range().setRange(zoomOut(B));
	plotter.plot();
	
	return 0;
    }

	
/* end of file main.cpp */
