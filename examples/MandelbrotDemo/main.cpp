/***********************************************
 * Project : MandelbrotDemo
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

int nbIter = 64; // number of iteration to use when compting the fractal sets. 

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


RGBc rabbit(fVec2 pos)
    {
    const double cx = -0.122561;
    const double cy = 0.744862;
    double X = pos.X();
    double Y = pos.Y();
    for (int i = 0; i < nbIter; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2 * sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return RGBc::jetPalette(i, 1, nbIter); }
        }
    return RGBc::c_Black;
    }


int main(int argc, char *argv[]) 
    {
	cout << "Drawing Mandelbrot + Douady's rabbit.\n";
    cout << "Maximum number of iterations (1-1024) ? "; 
    cout.useDefaultInputValue(true); // so that we use the current value of nbIter (ie 64) as the default value. 
    cout >> nbIter;
    if (nbIter < 1) nbIter = 1; else if (nbIter >1024) nbIter = 1024;
    cout << nbIter << "\n"; 
    Plotter2D Plotter;  // create the plotter
    auto M = makePlot2DPlane<mandelbrot>("Mandelbrot Set"); // the mandelbrot set
    auto D = makePlot2DPlane<rabbit>("Douady's rabbit"); // the mandelbrot set
    Plotter[M][D];
    M.opacity(0.5);
    D.opacity(0.5);
    Plotter.range().setRange(fRect(-0.65,-0.15,0.4,0.8));
    Plotter.plot();
    return 0;
	}
	
/* end of file main.cpp */

