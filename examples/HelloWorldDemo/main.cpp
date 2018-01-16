/***********************************************
 * project: HelloWorldDemo
 * date: 2018-01-16
 ***********************************************/

#include "mtools/mtools.hpp" 
using namespace mtools;

int main(int argc, char *argv[])
  {	
  MTOOLS_SWAP_THREADS(argc,argv);         // required on OSX, does nothing on Linux/Windows
  cout << "Hello from the console !";     // print on mtools::cout console (saved in cout.text)
    
  Image im(800, 600, RGBc(220,220,220));  // image of size 800x600 with a light gray background
    
  // draw on the image
  im.fill_ellipse_in_rect({ 100,400,50,550 }, RGBc::c_Cyan, false);
  im.draw_ellipse_in_rect({ 100,400,50,550 }, RGBc::c_Green, true, true, 4);
  im.draw_text({400, 300}, "Hello\n  World!",MTOOLS_TEXT_CENTER,RGBc::c_Red.getOpacity(0.7), 200);
  im.draw_cubic_spline({ {10,10},{100,100},{200,30},{300,100}, {600,10} , {700,300},
      {720, 500}, {600, 480}, {400,500} }, RGBc::c_Yellow.getOpacity(0.5), true, true, true, 3);
	
  // display the image
  auto P = makePlot2DImage(im);   // Encapsulate the image inside a 'plottable' object.	
  Plotter2D plotter;              // Create a plotter object
  plotter.axesObject(false);      // Remove the axe system.
  plotter[P];                     // Add the image to the list of objects to draw.  	
  plotter.autorangeXY();          // Set the plotter range to fit the image.
  plotter.plot();                 // start interactive display.
  
  return 0;
  }

	
/* end of file main.cpp */
