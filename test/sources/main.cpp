#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;


MT2004_64 gen;

double sinus(double x)
    {
    return sin(x);
    }



int main(int argc, char *argv[])
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv, false, false);

    Plotter2D PL;
    auto P1 = makePlot2DFun(sinus, -1,5, "sinus");
    PL[P1];
    PL.autorangeXY();
    PL.plot();

    return 0;

    }

/* end of file main.cpp */

