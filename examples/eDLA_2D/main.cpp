/***********************************************
 * Project : eDLA_2D
 * date : Tue Dec 15 17:09:44 2015
 ***********************************************/

#include "stdafx.h"


// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;


MT2004_64 gen; // RNG

Grid_basic<2, int64,2> Grid; // the 2D grid

double maxrad = 1;               // radius of the cluster
volatile int64 N = 1;            // number of particules in the cluster

void* peekhint = nullptr;

bool eight_neighbour = false;


/* return true if there is a neighbour */
inline bool hasNeighbour(const iVec2 & pos)
    {
    { const auto p = Grid.peek(iVec2{ pos.X() + 1,pos.Y() }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
    { const auto p = Grid.peek(iVec2{ pos.X() - 1,pos.Y() }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
    { const auto p = Grid.peek(iVec2{ pos.X(),pos.Y() + 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
    { const auto p = Grid.peek(iVec2{ pos.X(),pos.Y() - 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
	if (eight_neighbour)
		{
		{ const auto p = Grid.peek(iVec2{ pos.X() + 1,pos.Y() + 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
		{ const auto p = Grid.peek(iVec2{ pos.X() + 1,pos.Y() - 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
		{ const auto p = Grid.peek(iVec2{ pos.X() - 1,pos.Y() + 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
		{ const auto p = Grid.peek(iVec2{ pos.X() - 1,pos.Y() - 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
		}
    return false;
    }

/* add particules to the cluster */
inline void addParticules(int64 nb)
    {
    for (int64 i = 0; i < nb;i++)
        {
        double radius = ((maxrad < 1000) ? 1000 : maxrad);
        double a = Unif(gen);
        iVec2 pos = { (int64)round(sin(TWOPI*a)*3*radius) , (int64)round(cos(TWOPI*a)*3*radius) }; // random starting position
        do
            {
            double v;
            while((v = pos.norm()) > radius + 100)
                { // out of the ball that contain the cluster, we can move fast
                if (v > 100 * radius) { pos *= 9; pos /= 10; } // too far away, we drift back...
                else
                    { // make a large step
                    int64 l = (int64)((v - radius - 10)*2.0 / 3.0);
                    SRW_Z2_MoveInRect(pos, iBox2(pos.X() - l, pos.X() + l, pos.Y() - l, pos.Y() + l), 16, gen);
                    }
                }
            iBox2 fullR;
            if (fullR.boundaryDist(pos) == 0)
                { // we only move by a single step
                SRW_Z2_1step(pos, gen);
                }
            else
                { // we are away from the cluster so we can make larger moves
                SRW_Z2_MoveInRect(pos, fullR, 16, gen);
                }
            }
        while(!hasNeighbour(pos));
        Grid(pos) = N;
        N++;
        double r = pos.norm();
        if (r > maxrad) { maxrad = r; }
        }
    }


RGBc colorFct(iVec2 pos)
    {
    auto p = Grid.peek(pos);
    if (p == nullptr) return RGBc::c_TransparentBlack;
    if (*p == 0) return RGBc::c_TransparentBlack;
    return RGBc::jetPalette(*p, 0, N);
    }


int main(int argc, char *argv[]) 
    {
	parseCommandLine(argc,argv);
    int64 maxNN = arg("N", 10000000).info("total number of particles in the simulation");
	int autoredraw = arg('a', 10).info("autoredraw per minutes");
	eight_neighbour = arg('e',false).info("use 8 neighbours adjacency");
    Grid({ 0,0 }) = N; N++; // initial particle 
    Plotter2D P;
    auto L = makePlot2DLattice(LatticeObj<colorFct>::get(), "external DLA 2D"); P[L]; 
    P.autoredraw(autoredraw);
    P.startPlot();
    watch("# of particles",N);
    watch("cluster radius", maxrad);
    while (P.shown())
        {
        if (maxNN - N > 1000) { addParticules(1000); }
        else
            {
            addParticules(maxNN - N);
            cout << "Simulation completed ! \n";
            P.autoredraw(0);
            int64 l = (int64)maxrad + 1;
            P.range().setRange(iBox2(-l, l, -l, l));
            P.redraw();
            P.plot();
            return 0;
            }
        }
    return 0;
	}
	
/* end of file main.cpp */

