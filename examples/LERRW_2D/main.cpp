/***********************************************
 * Project : LERRW_2D
 * Simulation of a Linearly edge reinforced 
 * random walk on Z^2
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


// structure at each site of Z^2.
struct siteInfo
    {
    siteInfo() : up(1.0), right(1.0), V(0) {}    // ctor, set the intial weights of the edges to 1
    double up, right;   // weights of the up and right edges.
    int64 V;            // number of visits to the site.
    };

int64 maxV = 0;                 // max local time on site
double maxE = 1.0;              // max weight on edges 
int64 range = 1;                // number of distincts site visited. 
Grid_basic<2, siteInfo> G;      // the grid
MT2004_64 gen;                  // random generator
CImg<unsigned char> image;      // image for detailled plot

// site are colored w.r.t. the local time of the walk.
RGBc colorLERRW(iVec2 pos)
    {
    const siteInfo * S = G.peek(pos);
    if ((S == nullptr) || (S->V == 0)) return RGBc::c_TransparentWhite;
    return RGBc::jetPaletteLog(S->V, 0, maxV, 1.2); // light logarithmic scale
    }


/* detail : image associated with a site */
const CImg<unsigned char> * getImage(mtools::iVec2 pos, mtools::iVec2 size)
    {
    const siteInfo * S = G.peek(pos);
    if ((S == nullptr) || (S->V == 0)) return nullptr;
    EdgeSiteImage ES;
    ES.site(true, RGBc::jetPaletteLog(S->V, 0, maxV, 1.2));
    /*
    ES.text(mtools::toString(v->N)).textColor(RGBc::c_White);
    if (v->direction == 1) { ES.up(ES.ARROW_INGOING); }
    if (v->direction == 2) { ES.down(ES.ARROW_INGOING); }
    if (v->direction == 3) { ES.left(ES.ARROW_INGOING); }
    if (v->direction == 4) { ES.right(ES.ARROW_INGOING); }
    auto up = Grid.peek(pos.X(), pos.Y() + 1); if ((up != nullptr) && (up->N >0) && (up->direction == 2)) { ES.up(ES.EDGE); }
    auto down = Grid.peek(pos.X(), pos.Y() - 1); if ((down != nullptr) && (down->N >0) && (down->direction == 1)) { ES.down(ES.EDGE); }
    auto left = Grid.peek(pos.X() - 1, pos.Y()); if ((left != nullptr) && (left->N >0) && (left->direction == 4)) { ES.left(ES.EDGE); }
    auto right = Grid.peek(pos.X() + 1, pos.Y()); if ((right != nullptr) && (right->N >0) && (right->direction == 3)) { ES.right(ES.EDGE); }
    */
    ES.makeImage(image, size);
    return(&image);
    }


// Simulate the LERRW with reinforcment parameter delta for steps unit of time.
void makeLERRW(uint64 steps, double delta)
    {
    Chronometer();
    cout << "Simulating " << steps << " steps of a LERRW with reinf. param " << delta << ".\n";
    maxV = 0;
    maxE = 1.0;
    range = 0;
    G.reset();
    image.resize(1, 1, 1, 3, -1);
    iVec2 pos = { 0, 0 };
    uint64 t = 0;

    ProgressBar<uint64> PB(steps, "Simulating..");
    for (uint64 n = 0; n < steps; n++)
        {
        PB.update(n);
        siteInfo & S = G[pos];                          // info at the current site
        if (S.V == 0) { range++; }                      // increase range if new site
        if ((++S.V) > maxV) { maxV = S.V; }             // save if largest local time

        double & right = S.right;                       // get a reference to the weight
        double & up = S.up;                             // of the 4 adjacent edges of the
        double & left = G(pos.X() - 1, pos.Y()).right;  // current position.
        double & down = G(pos.X(), pos.Y() - 1).up;     //
        double e = gen.rand_double0()*(left + right + up + down);
        if (e < left) 
                { 
                left += delta; if (left > maxE) { maxE = left; }
                pos.X()--; 
                }
        else {
            if (e < (left + right)) 
                { 
                right += delta; if (right > maxE) { maxE = right; }
                pos.X()++; 
                }
            else {
                if (e < (left + right + up)) 
                    { 
                    up += delta; if (up > maxE) { maxE = up; }
                    pos.Y()++; 
                    }
                else 
                    {
                    down += delta; if (down > maxE) { maxE = down; }
                    pos.Y()--;
                    }
                }
            }
        }
    PB.hide();
    cout << "\n\nSimulation completed in = " << Chronometer() / 1000.0 << " seconds.\n";
    cout << "- Reinforcement parameter = " << delta << "\n";
    cout << "- Number of steps = " << steps << "\n";
    cout << "- Range = " << range << "\n";
 //   cout << "- enclosing rectangle = " << R << "\n";
    cout << "- Max site local time = " << maxV << "\n";
    cout << "- Max edge weight = " << maxE << " (" << (maxE - 1)/delta << " visits)\n";
    //std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid";
    //G.save(fn); // save the grid in fn
    //cout << "- saved in file " << fn << "\n";
    Plotter2D Plotter;
    auto L = makePlot2DLattice(LatticeObj<colorLERRW>::get(),std::string("LERRW-d") + mtools::toString(delta));
    Plotter[L];
    Plotter.gridObject(true)->setUnitCells();
    Plotter.plot();
    return;
    }




int main(int argc, char *argv[]) 
    {
    makeLERRW(100000000, 0.5);
    return 0;
	}
	
/* end of file main.cpp */

