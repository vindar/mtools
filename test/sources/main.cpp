#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;

/*
MT2004_64 gen;
double sinus(double x) { return sin(x); }

void stupidTest()
    {
    cout << "Hello World\n";
    int64 v = cout.ask("Give me a number", 0);
    watch("your number", v);
    cout << "Terminate the program gracefully by closing the plotter window\n";
    cout << "or forcefully by closing the cout or watch window\n";
    Plotter2D PL;
    auto P1 = makePlot2DFun(sinus, -10, 10, "sinus");
    PL[P1];
    PL.autorangeXY();
    PL.plot();

    //mtools::exit(5);


    while (PL.shown())
        {
        v += Unif_1(gen);
        }
    }



int main(int argc, char *argv[])
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv, false, false);


    //cout << GetColorSelector<decltype(fcoul)>::call(fcoul, iVec2{ 1, 2 }, nullptr);
    mtools::cout.getKey();

    stupidTest();

    return 0;
    }
    */
/* end of file main.cpp */


int inIt;

/* Mandelbrot
simple RGBc return type : multiple call for the same pixel are blended together
*/
RGBc mandelbrot(const fVec2 & pos, const fBox2 & R, int32 nbiter)
    {
    int nbi = inIt + nbiter * (inIt/10);
    const double cx = pos.X();
    const double cy = pos.Y();
    double X = 0.0;
    double Y = 0.0;
    for (int i = 0; i < nbi; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2 * sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return RGBc::jetPalette(i, 1, nbi); }
        }
    return RGBc::c_Black;
    }


/* Douady's rabbit
return type std::pair<RGBc,bool> : setting the bool to true force color returned to overwrite previous color at the same pixel. 
*/
std::pair<RGBc,bool> rabbit(const fVec2 & pos, const fBox2 & R, int32 nbiter)
    {
    const int nbi = inIt + nbiter * (inIt/10);
    const double cx = -0.122561;
    const double cy = 0.744862;
    double X = pos.X();
    double Y = pos.Y();
    for (int i = 0; i < nbi; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2 * sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return std::pair<RGBc,bool>(RGBc::jetPalette(i, 1,64),true); }
        }
    return std::pair<RGBc, bool>(RGBc::c_Black,true);
    }


int main(int argc, char *argv[])
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv, true);
    cout << "**************************************\n";
    cout << "Drawing Mandelbrot + Douady's rabbit.\n";
    cout << "**************************************\n";
    inIt = arg('n', 64).info("initial number of iterations");
    Plotter2D Plotter;  // create the plotter
    auto M = makePlot2DPlane(mandelbrot, "Mandelbrot Set"); // the mandelbrot set
    auto D = makePlot2DPlane(rabbit, "Douady's rabbit"); // the mandelbrot set
    Plotter[M][D];
    Plotter.sensibility(1);
    M.opacity(1.0);
    D.opacity(0.5);
    Plotter.range().setRange(fBox2(-0.65, -0.15, 0.4, 0.8));
    watch("Nb of iterations", inIt);
    Plotter.plot();
    watch.remove("Nb of iterations");
    return 0;
    }

/* end of file */