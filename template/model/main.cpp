/***********************************************
 * [PROJECT_NAME] project
 ***********************************************/

#include "stdafx.h"

// *** Library mtools ***
// Uncomment 'define MTOOLS_BASIC_CONSOLE' in stdafx.h to disable mtools's console
#include "mtools.hpp"  
using namespace mtools;

// *** Library fltk ***
#include "FL/Fl.H"      // add fltk headers this way
#include "GL/glut.h"    // fltk glut

// *** Library cairo ***
#include "cairo.h"

// *** Library libjpeg ***
#include "jpeglib.h"

// *** Library libpng ***
#include "png.h"

// *** Library pixman ***
#include "pixman.h"

// *** Library zlib ***
#include "zlib.h"



int main(int argc, char *argv[]) 
    {
	parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
	cout << "Hello World\n"; 
	cout.getKey();
	
    return 0;
	}
	
/* end of file main.cpp */

