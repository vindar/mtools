/***********************************************
 * Project : OERRW_2D
 * Simulation of a Once Reinforced Random Walk on Z2
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

// *** Library fltk ***
#include "FL/Fl.H"      // add fltk headers this way
#include "GL/glut.h"    // fltk glut
#include "zlib.h"       // fltk zlib
#include "png.h"        // fltk libpng
#include "jpeglib.h"    // fltk libjpeg

#include "OERRW.hpp"

int main(int argc, char *argv[])
    {
    longOERRW simOERRW(5.0);

    simOERRW.makeWalk(10000000);
    cout << simOERRW << "\n";
    simOERRW.plotWalk();
    return 0;
    }
	
/* end of file main.cpp */

