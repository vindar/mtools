/***********************************************
 * Project : demoEden
 * date : Tue Jul 21 18:06:26 2015
 * created via Vindar's libwizard.
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

Grid_factor<2, char, 2> Grid(5, 5, false);   // the 2D grid. 0 if no particle, 1-4 = nb of neighour, 5 = in the cluster
RandomUrn<iVec2> Urn;             // list of neighour sites of the current domain
MT2004_64 gen;                  // random number generator

int64 N = 0; // current number of particle in the cluster

             /* color function for the cluster */
RGBc colorFun(iVec2 pos)
    {
    const char * v = Grid.safePeek(pos);
    if ((v == nullptr) || ((*v) == 0)) return RGBc::c_TransparentWhite;
    return RGBc::jetPalette(*v, 1, 5);
    }

/* perfect circle with same volume as the random cluster */
RGBc colorCircle(iVec2 pos)
    {
    const double Pi = 3.14159265358979;
    if (pos.X()*pos.X() + pos.Y()*pos.Y() <= N / Pi) return RGBc::c_Cyan;
    return RGBc::c_TransparentWhite;
    }


int main(int argc, char *argv[])
    {
    cout << "Eden Model (i.e. FPP with exp weights on the edges of Z^2).\n";
    cout << "'infinite simulation' ...\n\n";

    Grid.set({ 0,0 }, 4);
    Urn.insert({ 0,0 }); // the origin will be infected at the first step

    auto P1 = makePlot2DLattice(LatticeObj<colorFun>::get(), "Eden model"); P1.opacity(0.5);
    auto P2 = makePlot2DLattice(LatticeObj<colorCircle>::get(), "Perfect circle"); P2.opacity(0.5);
    Plotter2D Plotter;
    Plotter[P2][P1];
    Plotter.startPlot();
    Plotter.range().setRange(fRect(-10000, 10000, -10000, 10000));
    Plotter.autoredraw(300); // redraw every second

    while (Plotter.shown())
        {
        iVec2 & rpos = Urn(gen.rand_double0()); // pick a neighour site
        char n = Grid(rpos);
        if (gen.rand_double0() * 4 >= (4 - n))
            {// ok, new point in the cluster
            iVec2 pos = rpos;  // save the position 
            Urn.remove(rpos);   // remove it from the urn
            Grid.set(pos, 5);
            iVec2 upPos(pos.X(), pos.Y() + 1); char up = Grid(upPos); if (up == 0) { Urn.insert(upPos); } if (up != 5) { Grid.set(upPos, up + 1); }
            iVec2 downPos(pos.X(), pos.Y() - 1); char down = Grid(downPos); if (down == 0) { Urn.insert(downPos); } if (down != 5) { Grid.set(downPos, down + 1); }
            iVec2 leftPos(pos.X() + 1, pos.Y()); char left = Grid(leftPos); if (left == 0) { Urn.insert(leftPos); } if (left != 5) { Grid.set(leftPos, left + 1); }
            iVec2 rightPos(pos.X() - 1, pos.Y()); char right = Grid(rightPos); if (right == 0) { Urn.insert(rightPos); } if (right != 5) { Grid.set(rightPos, right + 1); }
            N++;
            if (N % 100000000 == 0) // display info every 10e8 steps
                {
                cout << "Number of particles in the cluster: " << N << "\n";
                cout << "Boundary: " << Urn << "\n";
                cout << "Grid: " << Grid << "\n\n";
                }
            }
        }
    return 0;
    }

/* end of file main.cpp */

