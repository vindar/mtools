/***********************************************
 * Project : demoImage
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;


/* demo on how to use the Plot2DCimg class to display an 
   image inside a Plotter2D */
int main(int argc, char *argv[]) 
    {	
	Plotter2D Plotter;	// the plotter
    Plotter.useSolidBackground(false);  // remove the default white background so that we see a checkboard. 
	
    CImg<unsigned char> im;	// a CImg image
    im.load("lenna.jpg");	// load the image from a file
	
    auto L = makePlot2DCImg(im, "Lenna");   // create a Plot2DCimg object associated with im
    L.position(L.TYPECENTER);               // center the image around the origin
    L.opacity(0.6f);                         // add some transparency for fun.
	Plotter[L]; // insert the object in the plotter.
	
    Plotter.autorangeXY(); // set the plotter range to fit the image (with fixed aspect ratio 1)
    
	Plotter.plot(); // display the plotter.
	
    return 0;
	}
	
/* end of file main.cpp */

