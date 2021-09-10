/***********************************************
 * Project : offlattice-eDLA
 ***********************************************/

#include "mtools/mtools.hpp"  
using namespace mtools;


/* The two constants below must be chosen such that there can be at most NBPARTICLESPERBOX
 * non interesting circles of radius RAD whose center lies in a square of side lenght 1.
 * (see circle packing problem: https://en.wikipedia.org/wiki/Circle_packing_in_a_square).
 *
 * Number of circles                 max Radius               
 *        2                     1/sqrt(2)                  = 0.7071... (too large)
 *        3                     1/(sqrt(2)/2 + sqrt(6)/2)  = 0.5176... (too large) 
 *        4                     1/2                        = 0.5       (too large)
 *        5                     1/(2sqrt(2))               = 0.3535...
 *        6                     sqrt(13)/12                = 0.3004...
 *        7                     1/(2+sqrt(3))              = 0.2679...
 *        8                     1/(sqrt(2)+sqrt(6))        = 0.2588...
 *        9                     1/4                        = 0.25
 */

 // radius of a particle
//#define RAD 0.3536        // slower but uses less memory
#define RAD 0.2501

 // max number of particles par unit box 
//#define NBPARTICLESPERBOX 4 // slower but uses less memory
#define NBPARTICLESPERBOX 8

/* structure at an integer site, at most 2 balls of radius R inside */
struct siteInfo
    {
    siteInfo() { memset(this, 0, sizeof(siteInfo)); }       // ctor, set everything to zero
    int64 N[NBPARTICLESPERBOX];                             // particle index (0 if no particle)
    fVec2 pos[NBPARTICLESPERBOX];                           // position of the center of the particle
    };


double eps;                         // distance under which particles stick together
MT2004_64 gen;                      // random number generator
Grid_basic<2, siteInfo, 2> Grid;    // the main grid 
int64 NN = 0;                       // current number of particles
Image im;                           // image for drawing
void * hintpeek = nullptr;          // used for safe peeking of the grid.
double maxd = 0.0;                  // current maximal distance from the origin of any particule center.


struct eDLAPLot
    {

    
    /* return the color of a site : color of the largest index of any particle
       whose center lie inside that unit square */
    inline RGBc getColor(iVec2 pos)
        {
        auto p = Grid.peek(pos, hintpeek);
        if (p == nullptr) return RGBc::c_Transparent;
        int64 maxV = p->N[0];
        if (maxV == 0) return RGBc::c_Transparent;
        for (size_t t = 1; t < NBPARTICLESPERBOX; t++)
            {
            auto V = p->N[t];
            if (V == 0) return RGBc::jetPalette(maxV, 1, NN); else if (V > maxV) { maxV = V; }
            }
        return RGBc::jetPalette(maxV, 1, NN);
        }


    inline void _drawBalls(int64 i, int64 j, const fBox2 & R, const iVec2 & size, bool & b)
        {
        auto p = Grid.peek({ i,j }, hintpeek);
        if ((p == nullptr) || (p->N[0] == 0)) return;
        if (!b) { b = true; im.resizeRaw((int64)size.X(), (int64)size.Y(), true); im.clear(RGBc::c_Transparent); }
		RGBc cc = RGBc::jetPalette(p->N[0], 1, NN);
		im.canvas_draw_filled_circle(R, p->pos[0], RAD, cc, cc, true);
        for (size_t t = 1; t < NBPARTICLESPERBOX; t++)
            {
            if (p->N[t] == 0) return;
			RGBc cc2 = RGBc::jetPalette(p->N[t], 1, NN);
			im.canvas_draw_filled_circle(R, p->pos[t], RAD, cc2, cc2, true);
            }
        }

    /* draw a site */
    inline const Image * getImage(mtools::iVec2 pos, mtools::iVec2 size)
        {
        bool b = false;
        fBox2 R((double)pos.X() - 0.5, (double)pos.X() + 0.5, (double)pos.Y() - 0.5, (double)pos.Y() + 0.5);
        const int64 i = pos.X();
        const int64 j = pos.Y();
        _drawBalls(i, j - 1, R, size, b);
        _drawBalls(i, j, R, size, b);
        _drawBalls(i, j + 1, R, size, b);
        _drawBalls(i - 1, j - 1, R, size, b);
        _drawBalls(i - 1, j, R, size, b);
        _drawBalls(i - 1, j + 1, R, size, b);
        _drawBalls(i + 1, j - 1, R, size, b);
        _drawBalls(i + 1, j, R, size, b);
        _drawBalls(i + 1, j + 1, R, size, b);
        if (b) return &im;
        return nullptr;
        }

    };

/* move uniformly on a circle of radius r around of pos */
inline void move(fVec2 & pos, double r)
    {
    double a = Unif(gen)*TWOPI;
    pos.X() += sin(a)*r;
    pos.Y() += cos(a)*r;
    }


inline void _neighbour(double & r2, int64 i, int64 j, const fVec2 & pos)
    {
    const siteInfo * si = Grid.peek(i, j);
    if (si == nullptr) return;
    for (size_t t = 0; t < NBPARTICLESPERBOX; t++)
        {
        if (si->N[t] == 0) return;
        const double rr2 = dist2(pos, si->pos[t]);
        if (rr2 < r2) { r2 = rr2; }
        }
    }


/* return a lower bound on the minimal distance between pos
and the centers of other particles */
inline double neighbour(const fVec2 & pos)
    {
    int64 i = (int64)floor(pos.X() + 0.5);
    int64 j = (int64)floor(pos.Y() + 0.5);
    const double r1 = 0.5 - std::abs(pos.X() - (double)i);
    const double r2 = 0.5 - std::abs(pos.Y() - (double)j);
    double r = 1.0 + std::min(r1, r2); r *= r;
    _neighbour(r, i, j, pos);
    _neighbour(r, i, j - 1, pos);
    _neighbour(r, i, j + 1, pos);
    _neighbour(r, i - 1, j, pos);
    _neighbour(r, i - 1, j - 1, pos);
    _neighbour(r, i - 1, j + 1, pos);
    _neighbour(r, i + 1, j, pos);
    _neighbour(r, i + 1, j - 1, pos);
    _neighbour(r, i + 1, j + 1, pos);
    return(sqrt(r));
    }


/* add a given number of particles to the cluster */
void addParticules(int64 nb)
    {
    for (int64 n = 0; n < nb; n++)
        {
        fVec2 pos(0.0, 0.0); // start from the origine but directly
        double e = maxd + 2; // go at distance  maxd + 2
        do
            {
            move(pos, e);
            double d;
            while((d = pos.norm()) > 2*maxd + 3)
                {
                if (pos.norm() > 5000 + (500*maxd)) { pos /= 1.2; } else { move(pos, d - maxd - 2); }
                }

            int64 i = (int64)floor(pos.X() + 0.5);  // center of the integer square containing
            int64 j = (int64)floor(pos.Y() + 0.5);  // the current position
            iBox2 R;
            const siteInfo * p = Grid.findFullBoxCentered({ i,j }, R); // look for an empty box
            if ((p != nullptr) || (R.boundaryDist({ i,j }) == 0))
                { // no empty box
                e = neighbour(pos) - (2*RAD); // upper bound on the distance we can move, (exact bound if smaller than 1.0 - 2*RAD).
                }
            else
                { // yes, there is an empty box
                fBox2 fR((double)(R.min[0]) - 0.5, (double)(R.max[0]) + 0.5, (double)(R.min[1]) - 0.5, (double)(R.max[1]) + 0.5); // rectangle known to be empty. 
                e = fR.boundaryDist(pos) - (2*RAD); // upper bound on the distance we can move,
                }
            }
        while (e > eps);
        double D = pos.norm(); if (D > maxd) { maxd = D; } // update maxd if needed
        siteInfo & S = Grid((int64)floor(pos.X() + 0.5), (int64)floor(pos.Y() + 0.5));
        size_t t = 0;
        while (S.N[t] != 0) { t++; }
        MTOOLS_INSURE(t < NBPARTICLESPERBOX); // it this fails, this means that NBPARTICLESPERBOX and RAD where not correctly chosen...
        NN++;
        S.N[t] = NN;
        S.pos[t] = pos;
        }
    }


int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc, argv);		
	parseCommandLine(argc,argv); // parse the command line, interactive mode
    int64 maxNN = arg("N", 10000000).info("total number of particles in the simulation");
    eps = arg("eps", 0.00001).info("distance under which particles stick together");
    int autoredraw = arg('a', 600).info("autoredraw per minutes");
    cout << "Radius of a particle : " << RAD << "\n";
    NN = 1;
    Grid(0, 0).N[0] = 1;  Grid(0, 0).pos[0] = { 0.0,0.0 }; // initial particle in the cluster
    Plotter2D P;
    auto L = makePlot2DLattice((eDLAPLot*)nullptr, "non-Lattice eDLA");
    P[L];
    P.autoredraw(autoredraw);
    P.startPlot();
    watch("# of particles", NN);
    watch("cluster radius", maxd);
    while (P.shown())
        {
        if (maxNN - NN > 1000) { addParticules(1000); }
        else
            {
            addParticules(maxNN - NN);
            cout << "Simulation completed ! \n";
            P.autoredraw(0);
            double l = (double)maxd + 1;
            P.range().setRange(fBox2(-l, l, -l, l));
            P.redraw();
            P.plot();
            return 0;
            }
        }
    return 0;
	}
	
/* end of file main.cpp */

