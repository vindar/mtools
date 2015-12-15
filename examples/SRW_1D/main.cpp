/***********************************************
 * Project : SRW_1D 
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;


double f1(double x) { return ((x<=1.0) ? 0 : sqrt(2*x*log(log(x)))); }   // upper and lower bound
double f2(double x) { return ((x<=1.0) ? 0 : -sqrt(2*x*log(log(x)))); }  // for the LIL

int main(int argc, char *argv[]) 
    {
    parseCommandLine(argc, argv, true);
    cout << "**************************************\n";
    cout << "Simulation of a 1D simple random walk.\n";
    cout << "**************************************\n";
    bool autorange = arg("auto").info("update the plotter's range automatically"); 
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
    PV.hypographOpacity(0.3f);          // but make it half transparent
    P.autoredraw(60);                   // the plotter window should redraw itself at least every second (more in fact since it redraws after unsuspending)
    P.range().fixedAspectRatio(false);  // disable the fixed aspect ratio
    if (!autorange) P.range().setRange(fRect(-1.0e7, 5.0e8, -60000, 60000)); // set the range (if not automatic adjustment)
    P.startPlot();                      // display the plotter
    while (P.shown())                   // loop until the plotter window is closed
        {
        while (tab.size() < tab.capacity()) { pos += ((Unif(gen) < 0.5) ? -1 : 1); tab.push_back(pos); } // fill the vector with new steps of the walk until its capacity()
        PV.suspend(true);      // suspend access to PV while we reallocate the vector memory.
        tab.reserve(tab.capacity() + 1000000); // reserve more space
        PV.suspend(false);     // resume access to the graph.
        if (autorange) P.autorangeXY(); // autorange (causes a redraw)
        }
    return 0;
  	}




/* end of file main.cpp */

