/***********************************************
 * Project : eDLA_2D
 * date : Tue Dec 15 17:09:44 2015
 ***********************************************/

#include "stdafx.h"


// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;


MT2004_64 gen(1); // RNG

Grid_basic<2, int64,2> Grid; // the 2D grid

double radius = 1000.0; // radius of the cluster
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
inline void addParticule(int64 nb)
    {
    int64 L = (int64)(radius*radius) * 1000;
    for (int64 i = 0; i < nb;i++)
        {
        double a = Unif(gen);
        iVec2 pos = { (int64)round(sin(TWOPI*a)*(2*radius + 1000)) , (int64)round(cos(TWOPI*a)*(2*radius + 1000)) }; // random starting position
        label:
        const int64 * p = Grid.peek(pos, peekhint); //value at the position 
        if (p == nullptr)
            { // we are away from the cluster
            int64 d;
            do
                {
                if (pos.norm2() > L) { pos /= 2; }
                iBox2 fullR;
                Grid.findFullBoxCentered(pos, fullR);
                d = SRW_Z2_MoveInRect(pos, fullR, 16, gen);
                }
            while (d > 0);
            }
        if (!hasNeighbour(pos)) { SRW_Z2_1step(pos,gen); goto label; }
        Grid(pos) = N;
        N++;
        double r = pos.norm();
        if (r > radius) { radius = r; }
        }
    }


RGBc colorFct(iVec2 pos)
    {
    auto p = Grid.peek(pos);
    if (p == nullptr) return RGBc::c_TransparentWhite;
    if (*p == 0) return RGBc::c_TransparentWhite;
    //return RGBc::c_Black;
    return RGBc::jetPalette(*p, 0, N);
    }


int main(int argc, char *argv[]) 
    {
	parseCommandLine(argc,argv,false,true);
	
	int64 stepBetweenInfo = arg('S',10000).info("steps between information");
	int autoredraw = arg('a', 180).info("autoredraw per minutes");
	eight_neighbour = arg('e',false).info("use 8 neighbour adjacency");
	
    Grid({ 0,0 }) = N; N++; // initial particle 
    Plotter2D P;
    auto L = makePlot2DLattice(LatticeObj<colorFct>::get(), "external DLA 2D"); P[L]; 
    P.autoredraw(autoredraw);
    P.solidBackGroundColor(RGBc::c_Black);
    P.startPlot();

    watch("# of particles",N);
    while (P.shown())
        {
        addParticule(stepBetweenInfo);
        }

    return 0;
	}
	
/* end of file main.cpp */

