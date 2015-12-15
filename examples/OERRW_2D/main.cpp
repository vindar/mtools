/***********************************************
 * Project : OERRW_2D
 * Simulation of a Once Reinforced Random Walk on Z2
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

#include "OERRW.hpp"

int main(int argc, char *argv[])
    {
    parseCommandLine(argc, argv, true);
    cout << "**************************************\n";
    cout << "Simulation of a OnceERRW.\n";
    cout << "**************************************\n";
    double d = arg('d', 5.0).info("reinforcement paramter");
    int64 N = arg('N', 1000000).info("size of trace");
    longOERRW simOERRW(d);
    Chronometer();
    simOERRW.makeWalk(N);
    cout << "Done in : " << Chronometer() << "ms\n";
    cout << simOERRW << "\n";
    simOERRW.plotWalk();
    return 0;
    }
	
/* end of file main.cpp */

