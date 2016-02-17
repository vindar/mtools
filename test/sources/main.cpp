#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;


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
    
    stupidTest();

    return 0;
    }

/* end of file main.cpp */

