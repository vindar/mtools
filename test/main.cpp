/*

// OpenGL example showing text on a rotating 3D object.
// erco 03/03/06
//
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#  ifdef __APPLE__
 #    include <OpenGL/glu.h>
 #  else
 #    include <GL/glu.h>
 #  endif
#include <string.h>
#include <stdio.h>
// Tetrahedron points
#define TOP    0,  1,  0
#define RIGHT  1, -1,  1
#define LEFT  -1, -1,  1
#define BACK   0, -1, -1
class MyGlWindow : public Fl_Gl_Window {
	float rotangle;
	void draw() {
		// First time? init viewport, etc.
		if (!valid()) {
			valid(1);
			// Initialize GL
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClearDepth(1.0);
			glDepthFunc(GL_LESS);
			glEnable(GL_DEPTH_TEST);
			glShadeModel(GL_FLAT);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Position camera/viewport init
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, w(), h());
		gluPerspective(45.0, (float)w() / (float)h(), 1.0, 10.0);
		glTranslatef(0.0, 0.0, -5.0);
		// Position object
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(rotangle, 1, 0, 1);
		glRotatef(rotangle, 0, 1, 0);
		glRotatef(rotangle, 1, 1, 1);
		// Draw tetrahedron
		glColor3f(1.0, 0.0, 0.0); glBegin(GL_POLYGON); glVertex3f(TOP);   glVertex3f(RIGHT);  glVertex3f(LEFT);  glEnd();
		glColor3f(0.0, 1.0, 0.0); glBegin(GL_POLYGON); glVertex3f(TOP);   glVertex3f(BACK);   glVertex3f(RIGHT); glEnd();
		glColor3f(0.0, 0.0, 1.0); glBegin(GL_POLYGON); glVertex3f(TOP);   glVertex3f(LEFT);   glVertex3f(BACK);  glEnd();
		glColor3f(0.5, 0.5, 0.5); glBegin(GL_POLYGON); glVertex3f(RIGHT); glVertex3f(BACK);   glVertex3f(LEFT);  glEnd();
		// Print tetrahedron's points on object
		//     Disable depth buffer while drawing text,
		//     so text draws /over/ object.
		//
		glDisable(GL_DEPTH_TEST);
		{
			const char *p;
			gl_font(1, 12);
			glColor3f(1.0, 0.0, 1.0);
			glRasterPos3f(TOP);   p = "+ top";   gl_draw(p, strlen(p));
			glRasterPos3f(LEFT);  p = "+ left";  gl_draw(p, strlen(p));
			glRasterPos3f(RIGHT); p = "+ right"; gl_draw(p, strlen(p));
			glRasterPos3f(BACK);  p = "+ back";  gl_draw(p, strlen(p));
		}
		glEnable(GL_DEPTH_TEST);
		// Print rotangle value at fixed position at lower left
		char s[40];
		sprintf(s, "ROT=%.2f", rotangle);
		glLoadIdentity(); glRasterPos2f(-3, -2); gl_draw(s, strlen(s));
	}
	static void Timer_CB(void *userdata) {
		MyGlWindow *o = (MyGlWindow*)userdata;
		o->rotangle += 1.0;
		o->redraw();
		Fl::repeat_timeout(1.0 / 24.0, Timer_CB, userdata);       // 24fps
	}
public:
	// CONSTRUCTOR
	MyGlWindow(int X, int Y, int W, int H, const char*L = 0) : Fl_Gl_Window(X, Y, W, H, L) {
		rotangle = 0;
		Fl::add_timeout(3.0, Timer_CB, (void*)this);       // wait 3 secs before animation begins
	}
};
// MAIN
int main() {
	Fl_Window win(500, 300);
	MyGlWindow mygl(10, 10, win.w() - 20, win.h() - 20);
	win.show();
	return(Fl::run());
}

*/


#include <mtools/mtools.hpp>

using namespace mtools;



double f(double x)
{
	return sin(x);
}


void test()
	{
	auto P = makePlot2DFun(f, "sin");
	auto Q = makePlot2DFun(f, "sin");

	Plotter2D plotter;
	plotter[P]; 
	plotter[Q];
	plotter.autorangeXY();

	plotter.plot(); 
	plotter.remove(P);
	}

int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	test();


}

