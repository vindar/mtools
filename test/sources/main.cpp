

#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;

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
CImg<unsigned char> im;             // image for drawing
void * hintpeek = nullptr;          // used for safe peeking of the grid.
double maxd = 0.0;                  // current maximal distance from the origin of any particule center.


                                    /* return the color of a site : color of the largest index of any particle
                                    whose center lie inside that unit square */
inline RGBc getColor(iVec2 pos)
    {
    auto p = Grid.peek(pos, hintpeek);
    if (p == nullptr) return RGBc(0, 0, 0, 0);
    int64 maxV = p->N[0];
    if (maxV == 0) return RGBc(0, 0, 0, 0);
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
    if (!b) { b = true; im.resize((int)size.X(), (int)size.Y(), 1, 4); im.clear(RGBc::c_TransparentWhite); }
    im.fBox2_draw_circle(R, p->pos[0], RAD, RGBc::jetPalette(p->N[0], 1, NN), 1.0, true);
    for (size_t t = 1; t < NBPARTICLESPERBOX; t++)
        {
        if (p->N[t] == 0) return;
        im.fBox2_draw_circle(R, p->pos[t], RAD, RGBc::jetPalette(p->N[t], 1, NN), 1.0, true);
        }
    }

/* draw a site */
inline const CImg<unsigned char> * getImage(mtools::iVec2 pos, mtools::iVec2 size)
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
    const double r1 = 0.5 - abs(pos.X() - (double)i);
    const double r2 = 0.5 - abs(pos.Y() - (double)j);
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
            while ((d = pos.norm()) > 2 * maxd + 3)
                {
                if (pos.norm() > 5000 + (500 * maxd)) { pos /= 1.2; }
                else { move(pos, d - maxd + 2); }
                }

            int64 i = (int64)floor(pos.X() + 0.5);  // center of the integer square containing
            int64 j = (int64)floor(pos.Y() + 0.5);  // the current position
            iBox2 R;
            const siteInfo * p = Grid.findFullBoxCentered({ i,j }, R); // look for an empty box
            if ((p != nullptr) || (R.boundaryDist({ i,j }) == 0))
                { // no empty box
                e = neighbour(pos) - (2 * RAD); // upper bound on the distance we can move, (exact bound if smaller than 1.0 - 2*RAD).
                }
            else
                { // yes, there is an empty box
                fBox2 fR((double)(R.min[0]) - 0.5, (double)(R.max[0]) + 0.5, (double)(R.min[1]) - 0.5, (double)(R.max[1]) + 0.5); // rectangle known to be empty. 
                e = fR.boundaryDist(pos) - (2 * RAD); // upper bound on the distance we can move,
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








double test0(int64 v)
    {
    return 0.0;
    }

double test1(int64 v)
    {
    return 1.0;
    }


void espp(int64 N)
    {
    Chronometer();
    int64 tot = 0;
    double var = 0.0;
    for (int i = 0;i < N; i++)
        {
        int64 h = UIHPTpeelLaw(gen);
        tot += h;
        var += h*h;
        }
    cout << "esp = " << ((double)tot) / N << " in " << Chronometer() << "\n";
    cout << "var = " << ((double)var) / N << "\n\n";
    }


int main(int argc, char *argv[])
    {
    espp(100);
    espp(1000);
    espp(10000);
    espp(100000);
    espp(1000000);
    espp(10000000);
    espp(100000000);
    espp(1000000000);

    cout.getKey();
    return 0;

    }

/* end of file main.cpp */

