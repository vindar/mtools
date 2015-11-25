/***********************************************
 * Project : CMPonZ2
 * date : Sat Aug 15 14:30:08 2015
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

#include "CMPMerger.hpp"


#define NBCOLOR 5


/*********************************************************************************************************
* Torus graph
**********************************************************************************************************/

/**
* A torus of Z2
**/
struct SiteZ2Torus : public CMPHook<SiteZ2Torus, NBCOLOR>
    {

    /* number of neighbour = 4 for a torus */
    inline int nbneighbour() { return 4; }

    /* return the ith neighbour */
    inline SiteZ2Torus* neighbour(int index) 
            {
            switch (index) {
                case 0: if (X == 0)    return(this + (LX-1)); else return (this - 1); // left
                case 1: if (X == LX-1) return(this - (LX-1)); else return (this + 1); // right
                case 2: if (Y == 0)    return(this + LX*(LY - 1)); else return (this - LX); // left
                case 3: if (Y == LY-1) return(this - LX*(LY - 1)); else return (this + LX); // left
                }
            MTOOLS_ERROR("oups...");
            return nullptr;
            }

    /* radius of a site */
    inline double radius() const { return rad; }

    void set(int x, int y, int lx, int ly, double r) { X = x; Y = y; LX = lx; LY = ly; rad = r; }

    double rad; // radius
    int X, Y;   // position                
    int LX, LY; // size of the torus

    };


/**
* Creates a torus with given dimension
**/
template<typename RandomGen> SiteZ2Torus * createTorus(int LX, int LY, double a, RandomGen & rgen)
    {
    SiteZ2Torus * tab = new SiteZ2Torus[LX*LY];
    for (int j = 0;j < LY; j++) 
        { 
        for (int i = 0;i < LX; i++) 
            { 
            tab[i + j*LX].set(i,j,LX,LY, ((rgen.rand_double0() < a) ? 1.0 : 0.0)); 
            } 
        }
    return tab;
    }


/**
* Deletes a torus
**/
void deleteTorus(SiteZ2Torus * root) { delete[] root; }



/*********************************************************************************************************
* Box graph
**********************************************************************************************************/

/**
* A box of Z2
**/
struct SiteZ2Box : public CMPHook<SiteZ2Box, NBCOLOR>
    {

    /* number of neighbour = 4 for a torus */
    inline int nbneighbour() 
        {
        if (X == 0) { if ((Y == 0) || (Y == LY - 1)) { return 2; } return 3; }
        if (X == LX-1) { if ((Y == 0) || (Y == LY - 1)) { return 2; } return 3; }
        return 4;
        }

    /* return the ith neighbour */
    inline SiteZ2Box * neighbour(int index)
        {
        if (X == 0) 
            { 
            if (index == 0)   return (this + 1);
            if (Y == 0)       return (this + LX);
            if (index == 1)   return (this - LX);
            return (this + LX);
            }
        if (X == LX-1)
            {
            if (index == 0)   return (this - 1);
            if (Y == 0)       return (this + LX);
            if (index == 1)   return (this - LX);
            return (this + LX);
            }
        if (Y == 0)
            {
            if (index == 0)   return (this + LX);
            if (X == 0)       return (this + 1);
            if (index == 1)   return (this - 1);
            return (this + 1);
            }
        if (Y == LY-1)
            {
            if (index == 0)   return (this - LX);
            if (X == 0)       return (this + 1);
            if (index == 1)   return (this - 1);
            return (this + 1);
            }
        switch (index) 
            {
            case 0: return (this - 1); // left
            case 1: return (this + 1); // right
            case 2: return (this - LX); // up
            case 3: return (this + LX); // down
            }
        MTOOLS_ERROR("oups...");
        return nullptr;
        }

    /* radius of a site */
    inline double radius() const { return rad; }

    void set(int x, int y, int lx, int ly, double r) { X = x; Y = y; LX = lx; LY = ly; rad = r; }

    double rad; // radius
    int X, Y;   // position                
    int LX, LY; // size of the torus

    };


/**
* Creates a box with given dimension
**/
template<typename RandomGen> SiteZ2Box * createBox(int LX, int LY, double a, RandomGen & rgen)
    {
    SiteZ2Box * tab = new SiteZ2Box[LX*LY];
    for (int j = 0;j < LY; j++)
        {
        for (int i = 0;i < LX; i++)
            {
            tab[i + j*LX].set(i, j, LX, LY, ((rgen.rand_double0() < a) ? 1.0 : 0.0));
            }
        }
    return tab;
    }


/**
* Deletes a boxs
**/
void deleteBox(SiteZ2Box * root) { delete[] root; }






MT2004_64 gen;

SiteZ2Box * Torus;

int LX, LY;




void displayIm()
    {
    Plotter2D P;
    CImg<unsigned char> imz("image8.png");
    auto I = makePlot2DCImg(imz, "CMP");
    P[I];
    I.position(I.TYPECENTER);
    P.plot();
    }



RGBc color0(iVec2 pos)
    {
    if ((pos.X() < 0) || (pos.X() >= LX)) return RGBc::c_Cyan;
    if ((pos.Y() < 0) || (pos.Y() >= LY)) return RGBc::c_Cyan;
    auto site = Torus[pos.X() + pos.Y()*LX];
    return site.CMP_color(0);
    }

RGBc color1(iVec2 pos)
    {
    if ((pos.X() < 0) || (pos.X() >= LX)) return RGBc::c_Cyan;
    if ((pos.Y() < 0) || (pos.Y() >= LY)) return RGBc::c_Cyan;
    auto site = Torus[pos.X() + pos.Y()*LX];
    return site.CMP_color(1);
    }

RGBc color2(iVec2 pos)
    {
    if ((pos.X() < 0) || (pos.X() >= LX)) return RGBc::c_Cyan;
    if ((pos.Y() < 0) || (pos.Y() >= LY)) return RGBc::c_Cyan;
    auto site = Torus[pos.X() + pos.Y()*LX];
    return site.CMP_color(2);
    }

RGBc color3(iVec2 pos)
    {
    if ((pos.X() < 0) || (pos.X() >= LX)) return RGBc::c_Cyan;
    if ((pos.Y() < 0) || (pos.Y() >= LY)) return RGBc::c_Cyan;
    auto site = Torus[pos.X() + pos.Y()*LX];
    return site.CMP_color(3);
    }

RGBc color4(iVec2 pos)
    {
    if ((pos.X() < 0) || (pos.X() >= LX)) return RGBc::c_Cyan;
    if ((pos.Y() < 0) || (pos.Y() >= LY)) return RGBc::c_Cyan;
    auto site = Torus[pos.X() + pos.Y()*LX];
    return site.CMP_color(4);
    }


void test()
    {

    LX = 1000;
    LY = 1000;
    double a = 0.12;
        
    cout << "Creating the torus... ";
    Torus = createBox(LX, LY, a,gen);
    cout << "done in " << Chronometer() << "ms\n\n";
        
    cout << "Computing the CMP... "; Chronometer();    
    CMPMerger<SiteZ2Box> cmpMerger(Torus);
    cout << "done in " << Chronometer() << "ms\n\n";

    cout << "Statistics about the CMP:\n" << cmpMerger << "\n\n";

    cout << "\n\nColoring the clusters..."; Chronometer();
    CMP_CLUSTERLOOP_UP(cmpMerger, it, true, true, true)
        {
        cmpMerger.colorCluster(*it, cmpMerger.rgbHeight(*it), 0);
        }
    cout << "done in " << Chronometer() << "ms\n\n";

    cout << "\n\nColoring the largest clusters..."; Chronometer();
    cmpMerger.colorCluster(cmpMerger.largestCluster(), RGBc::c_Red, 1);
    cout << "done in " << Chronometer() << "ms\n\n";


    cout << "\n\nColoring the Stabilizers..."; Chronometer();
    CMP_CLUSTERLOOP_DOWN(cmpMerger, it, true, true, true)
        {
        cmpMerger.colorStabilizer(*it, opacity(cmpMerger.rgbHeight(*it),0.2f),true,2);
        }
    cout << "done in " << Chronometer() << "ms\n\n";

    cout << "\n\nColoring the top Stabilizers..."; Chronometer();
    CMP_CLUSTERLOOP_UP(cmpMerger, it, true, true, true)
        {
        if (cmpMerger.isMasterCluster())
            {
            if ((*it)->listFathers.size() == 1) cmpMerger.colorStabilizer(*it, opacity(cmpMerger.rgbHeight(*it), 1), true, 3);
            }
        else
            {
            if ((*it)->listFathers.size() == 0) cmpMerger.colorStabilizer(*it, opacity(cmpMerger.rgbHeight(*it), 1), true, 3);
            }
        }
    cout << "done in " << Chronometer() << "ms\n\n";

    cout << "\n\nColoring the largest stabilizer..."; Chronometer();
    cmpMerger.colorStabilizer(cmpMerger.largestCluster(), RGBc::c_Red,true, 4);
    cout << "done in " << Chronometer() << "ms\n\n";

    Plotter2D P;
    auto L4 = makePlot2DLattice(LatticeObj<color4>::get(), "Largest Stabilizer"); P[L4]; L4.opacity(0.8f);
    auto L3 = makePlot2DLattice(LatticeObj<color3>::get(), "Top Stabilizers"); P[L3]; L3.opacity(0.8f);
    auto L2 = makePlot2DLattice(LatticeObj<color2>::get(), "Stabilizers"); P[L2]; L2.opacity(0.8f);
    auto L1 = makePlot2DLattice(LatticeObj<color1>::get(), "Largest cluster"); P[L1];
    auto L0 = makePlot2DLattice(LatticeObj<color0>::get(), "Clusters"); P[L0];

    int m = std::max<int>((LX / 1000), (LY / 1000));
    if (m > 1) { P.setDrawingSize(LX/m, LY/m); P.viewZoomFactor(m); } else { P.setDrawingSize(LX, LY); }
    P.range().setRange(fRect(0, LX, 0, LY));
    P.range().set1to1();
    P.gridObject(true)->setUnitCells();
    P.axesObject(false);
    P.plot();
    }



int main(int argc, char *argv[]) 
    {
  // displayIm();
   test();
    return 0;
	}


/* end of file main.cpp */

