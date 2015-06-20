/***********************************************
 * Project : demoBM
 * Demo of Brownian motion. 
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;


double f1(double x) { return ((x<=1.0) ? 0 : sqrt(2*x*log(log(x)))); }   // upper and lower bound
double f2(double x) { return ((x<=1.0) ? 0 : -sqrt(2*x*log(log(x)))); }  // for the LIL

int main(int argc, char *argv[]) 
    {
    cout << "Simulation of a 1D simple random walk.\n";
    cout << "Do you want to update the plotter's range automatically ?";
    bool autorange; cout >> autorange;
    cout << "\nSimulating...\n";
    MT2004_64 gen;                      // the RNG
    std::vector<int>  tab;              // the vector containing the positions
    int pos = 0;                        // current position
    Plotter2D P;                        // the plotter object
    auto PF1 = makePlot2DFun(f1);       // the first function plot object
    auto PF2 = makePlot2DFun(f2);       // the second function plot object
    auto PV = makePlot2DVector(tab);    // the vector plot, use natural dynamically growing range.
    P[PV][PF1][PF2];                    // insert everything in the plotter
    PV.interpolationLinear();           // use linear interpolation
    PV.hypograph(true);                 // fill the hypograph
    PV.hypographOpacity(0.1f);          // but make it almost transparent
    P.autoredraw(300);                  // the plotter window should redraw itself 5 times per second
    P.range().fixedAspectRatio(false);  // disable the fixed aspect ratio
    P.startPlot();                      // display the plotter
    while (P.shown())                   // loop until the plotter window is closed
        {
        while (tab.size() < tab.capacity()) { pos += ((gen.rand_double0() < 0.5) ? -1 : 1); tab.push_back(pos); } // fill the vector with new steps of the walk until its capacity()
        PV.suspend();      // disable the graph since we must reallocate memory (using suspend() instead of enable(false) remove flickering by not erasing the drawing).
        //cout << tab.size() << "steps.\n";
        tab.reserve(tab.capacity() + 1000000); // reserve additonnal space
        PV.enable(true);   // enable again.
        if (autorange) P.autorangeXY(); // autorange (this cause a redraw)
        }
    return 0;
  	}




/* end of file main.cpp */

