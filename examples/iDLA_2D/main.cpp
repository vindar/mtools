/***********************************************
 * Project : iDLA_2D
 * date : Wed Dec 16 00:01:14 2015
 ***********************************************/

#include "mtools.hpp"  
using namespace mtools;

MT2004_64  gen;	// the random number generator

Grid_factor<2, char, 2,5> Grid;   // the grid

volatile int64 N; //number of walkers sent


/* send a given number of walkers */
void makeCluster(int nb)
    {
    for (int i = 0;i < nb;i++) 
        {
        iVec2 pos(0, 0);
        int k = 101;
        while(Grid.get(pos) == 1) 
            {
            if (k > 100) {
                iBox2 fullR;
                Grid.findFullBox(pos, fullR);
                if (fullR.min[0] == fullR.max[0]) { SRW_Z2_1step(pos, gen); k = 0; }
                else 
                    {
                    fullR.min[0]--; fullR.max[0]++; fullR.min[1]--; fullR.max[1]++;
                    SRW_Z2_MoveInRect(pos, fullR, 16, gen);
                    }
                }
            else  { 
                SRW_Z2_1step(pos, gen); k++; 
                }
            }
        Grid.set(pos, 1);
        N++;
        }
    }


/* DLA cluster colored in red */
RGBc colorCluster(iVec2 pos)
    {
    auto p = Grid.peek(pos);
    if ((p == nullptr)||(*p == 0)) return RGBc::c_TransparentWhite;
    return RGBc::c_Red;
    }


/* circle of similar volume colored in blue */
RGBc colorCircle(iVec2 pos)
    {
    if (PI*(pos.X()*pos.X() + pos.Y()*pos.Y()) <= N) return RGBc::c_Blue;
    return RGBc::c_TransparentWhite;
    }


int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc, argv);		
	parseCommandLine(argc,argv,false);
    cout << "******************************\n";
    cout << "internal DLA on Z2\n";
    cout << "******************************\n";
    int autoredraw = arg('a', 600).info("autoredraw rate");
    Grid.reset(0, 1, false);
    Grid.set(0, 0, 1); // initial cluster. 
    Plotter2D P;
    auto Cl = makePlot2DLattice(colorCluster, "iDLA cluster"); P[Cl]; Cl.opacity(0.5);
    auto Ci = makePlot2DLattice(colorCircle, "Circle"); P[Ci]; Ci.opacity(0.5);
    P.autoredraw(autoredraw);
    P.startPlot();
    Chronometer();
    watch("# of particles", N);
    while (P.shown())
        {
        makeCluster(1000);
        }
    return 0;
	}
	
/* end of file main.cpp */

