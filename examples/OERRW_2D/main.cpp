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
    longOERRW simOERRW(5.0);
    Chronometer();
    simOERRW.makeWalk(30000000);
    cout << "Done in : " << Chronometer() << "ms\n";
    cout << simOERRW << "\n";
    simOERRW.plotWalk();
    return 0;
    }
	
/* end of file main.cpp */

