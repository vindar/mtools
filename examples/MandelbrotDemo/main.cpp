/***********************************************
 * Project : MandelbrotDemo
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

int nbIter;

/* return the color associated with point pos in the mandelbrot set 
by calculating at most nbIter iteration */
RGBc mandelbrot(fVec2 pos)
    {
    const double cx = pos.X();
    const double cy = pos.Y();
    double X = 0.0;
    double Y = 0.0;
    for(int i = 0; i < nbIter; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2*sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return RGBc::jetPalette(i, 1, nbIter); }
        }
    return RGBc::c_Black;
    }


int main(int argc, char *argv[]) 
    {
	cout << "Mandelbrot set demo.\n";
    cout << "Maximum number of iterations (1-1024) ? ";
    cout >> nbIter;
    if (nbIter < 1) nbIter = 1; else if (nbIter >104) nbIter = 1024;
    cout << nbIter << "\n"; 
    Plotter2D Plotter;  // create the plotter
    auto PD = makePlot2DPlane<mandelbrot>("Mandelbrot Set");
    Plotter[PD];
    Plotter.range().setRange(fRect(-2, 2, -2, 2));
    Plotter.plot();
    return 0;
	}
	
/* end of file main.cpp */

