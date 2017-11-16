/***********************************************
 * Project : LERRW_2D
 * Simulation of a Linearly edge reinforced 
 * random walk on Z^2
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 


#include "mtools.hpp"
using namespace mtools;



// structure at each site of Z^2.
struct siteInfo
    {
    siteInfo() : up(1.0), right(1.0), V(0) {}    // ctor, set the intial weights of the edges to 1
    double up, right;   // weights of the up and right edges.
    int64 V;            // number of visits to the site.
    };

iVec2 pos;                      // position of the walk
double delta = 1.0;             // reinf param
int64 maxV = 0;                 // max local time on site
double maxE = 1.0;              // max weight on edges 
double logscale = 1.2;          // logarithmic scale
int64 range = 1;                // number of distincts site visited. 
iBox2 R;                        // rectangle enclosing the trace of the walk
Grid_basic<2, siteInfo> G;      // the grid
MT2004_64 gen;                  // random generator
Image image;       // image for detailled plot



struct LERRWPlot
    {
    // site are colored w.r.t. the local time of the walk.
    static RGBc getColor(iVec2 pos)
        {
        const siteInfo * S = G.peek(pos);
        if ((S == nullptr) || (S->V == 0)) return RGBc::c_TransparentWhite;
        return RGBc::jetPaletteLog(S->V, 0, maxV, logscale); // light logarithmic scale
        }


    /* detail : image associated with a site */
    static const Image * getImage(mtools::iVec2 p, mtools::iVec2 size)
        {
        const siteInfo * S = G.peek(p);
        if ((S == nullptr) || (S->V == 0)) return nullptr;
        EdgeSiteImage ES;
        //if (pos == p) { ES.bkColor(RGBc::c_Black.getOpacity(0.5)); }      // uncomment to display the current position
        ES.site(true, RGBc::jetPaletteLog(S->V, 0, maxV, logscale));
        ES.text(mtools::toString(S->V)).textColor(RGBc::c_White);
        double right = S->right;
        double up = S->up;
        double left = G(p.X() - 1, p.Y()).right;
        double down = G(p.X(), p.Y() - 1).up;
        if (up > 1.0) { ES.up(ES.EDGE, RGBc::jetPaletteLog(up / maxE, logscale)); ES.textup(mtools::toString((int64)((up - 1) / delta))); }
        if (down > 1.0) { ES.down(ES.EDGE, RGBc::jetPaletteLog(down / maxE, logscale)); }
        if (left > 1.0) { ES.left(ES.EDGE, RGBc::jetPaletteLog(left / maxE, logscale)); ES.textleft(mtools::toString((int64)((left - 1) / delta))); }
        if (right > 1.0) { ES.right(ES.EDGE, RGBc::jetPaletteLog(right / maxE, logscale)); }
        ES.makeImage(image, size);
        return(&image);
        }

    };

// Simulate the LERRW with reinforcment parameter delta for steps unit of time.
void makeLERRW(uint64 steps, double d)
    {
    delta = d;
    cout << "Simulating ...";
    ProgressBar<uint64> PB(steps, "Simulating...");
    Chronometer();
    maxV = 0;
    maxE = 1.0;
    range = 0;
    R.clear();
    G.reset();
    image.resizeRaw(1, 1);
    pos = { 0, 0 };
    // main loop
    for (uint64 n = 0; n < steps; n++)
        {
        PB.update(n);
        siteInfo & S = G[pos];                          // info at the current site
        if (S.V == 0) { range++; }                      // increase range if new site
        if ((++S.V) > maxV) { maxV = S.V; }             // check if largest local time yet.
        R.swallowPoint(pos);                            // update the rectangle enclosing the trace
        double & right = S.right;                       // get a reference to the weight
        double & up = S.up;                             // of the 4 adjacent edges of the
        double & left = G(pos.X() - 1, pos.Y()).right;  // current position.
        double & down = G(pos.X(), pos.Y() - 1).up;     //
        double e = Unif(gen)*(left + right + up + down);
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
    watch("maxV",maxV);
    watch("maxE", maxE);
    watch("mlogscale", logscale);
    // update one last time for the terminating point
    siteInfo & S = G[pos];
    if (S.V == 0) { range++; }
    if ((++S.V) > maxV) { maxV = S.V; }
    R.swallowPoint(pos);
    // done, print statistic and plot the walk. 
    PB.hide();
    cout << "ok. Completed in " << Chronometer() / 1000.0 << " seconds.\n\nStatistics:\n";
    cout << "  - Reinforcement parameter = " << delta << "\n";
    cout << "  - Number of steps = " << steps << "\n";
    cout << "  - Range = " << range << " site visited inside " << R << "\n";
    cout << "  - Max site local time = " << maxV << "\n";
    cout << "  - Max edge weight = " << maxE << " (" << (int64)((maxE - 1)/delta) << " visits)\n";
    cout << "  - Current position of the walk = (" << pos.X() <<  "," << pos.Y() << ")\n";
    //std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid";
    //G.save(fn); // save the grid in fn
    //cout << "- saved in file " << fn << "\n";
    Plotter2D Plotter;
    auto L = makePlot2DLattice((LERRWPlot*)nullptr,std::string("LERRW-d") + mtools::toString(delta));
    L.setImageType(L.TYPEIMAGE);
    Plotter[L];
    Plotter.gridObject(true)->setUnitCells();
    Plotter.range().setRange(zoomOut(fBox2(R)));
    Plotter.plot();
    return;
    }




int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc, argv);		
    parseCommandLine(argc, argv, true);
    cout << "*******************************************************\n";
    cout << " Simulation of a Linearly Reinforced Random Walk on Z^2\n";
    cout << "*******************************************************\n\n";
    double delta = arg('d', 2.0).info("reinforcement parameter");
    int64 N = arg('N', 1000000000).info("number of steps of the walk");
    makeLERRW(N, delta);
    return 0;
	}
	
/* end of file main.cpp */

