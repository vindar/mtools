/***********************************************
* Random Poisson-Voronoi tesselation
* 
* Draw the point process, the Delaunay triangulation
* and the associated Vornoi cell inside the unit square.
 ***********************************************/

#include "mtools/mtools.hpp" 

using namespace mtools;

DelaunayVoronoi DV; 

MT2004_64 gen;

int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc,argv); // required on OSX, does nothing on Linux/Windows
    mtools::parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
    int N = arg("Number of points in [0,1]x[0,1]", 1000);

    // add vertices to the delauney triangulation.
    for (int n = 0; n < N; n++)
        {
        DV.DelaunayVertices.push_back(fVec2(Unif(gen), Unif(gen)));
        }

    // compute the triangulation and the associated Voronoi diagram. 
    DV.compute();

    //draw on screen
	mtools::Plotter2D plotter;
	auto canvas = mtools::makeFigureCanvas(3);

	// draw dots at vertices positions
	for (int k = 0; k < N; k++)
		{
		canvas(mtools::Figure::CircleDot(DV.DelaunayVertices[k], 2,RGBc::c_Red), 2);
		}

	// draw the delaunay triangulation in red. 
	int nb_D_e = (int)DV.DelaunayEdgesIndices.size();
	for (int k = 0; k < nb_D_e; k++)
		{
		mtools::iVec2 e = DV.DelaunayEdgesIndices[k];
		canvas(mtools::Figure::Line(DV.DelaunayVertices[e.X()], DV.DelaunayVertices[e.Y()], mtools::RGBc::c_Orange), 1);
		}

	// draw the voronoi diagram
	int nb_V_e = (int)DV.VoronoiEdgesIndices.size();
	for (int k = 0; k < nb_V_e; k++)
		{
		mtools::iVec2 e = DV.VoronoiEdgesIndices[k];
		mtools::fVec2 P1 = DV.VoronoiVertices[e.X()];
		if (e.Y() == -1)
			{ // semi-infinite ray
			mtools::fVec2 N = DV.VoronoiNormals[e.X()];
			canvas(mtools::Figure::Line(P1, P1 + N, mtools::RGBc::c_Gray), 0);
			}
		else
			{ // regular edge
			mtools::fVec2 P2 = DV.VoronoiVertices[e.Y()];
			canvas(mtools::Figure::Line(P1, P2, mtools::RGBc::c_Black), 0);
			}
		}

	// display
	auto P = mtools::makePlot2DFigure(canvas, 4, "Poisson-Delaunay-Voronoi");
	plotter[P];
	plotter.range().setRange({ 0,1,0,1 });
	plotter.plot();

    return 0;
    }
	
/* end of file main.cpp */
