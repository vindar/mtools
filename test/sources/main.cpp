#include "stdafx_test.h"


using namespace mtools;


#define NN  14

typedef Vec<int, NN> PermTab;

int64 nperm = 0;
int64 nzero = 0;

void fun(PermTab tab)
    {
    PermTab L(0);
    for (int i = 0;i < NN; i++)
        {
        int p = tab[i];
        L[p] = 2;
        p--;
        while ((p >= 0) && (L[p] == 0)) { p--; }
        if (p >= 0) { (L[p])--; }
        }
    int j = 0;
    while (L[j] == 0) { j++; }
    nperm++;
    nzero += j;
    //mtools::cout << tab << "  \t " << L << " \t " << j << " \t " << nperm << "\n";

    }


void recPermTab(int n, PermTab tab)
    {
    if (n == NN) { fun(tab); return; }
    tab[n] = n;
    recPermTab(n + 1, tab);
    int j = n;
    while (j > 0)
        {
        int t = tab[j - 1];
        tab[j - 1] = tab[j];
        tab[j] = t;
        recPermTab(n + 1, tab);
        j--;
        }
    return;
    }

void simPerm()
    {
    PermTab tab;
    recPermTab(0, tab);

    mtools::cout << "NN = " << NN << "\n";
    mtools::cout << "nombre de permutations = " << nperm << "\n";
    mtools::cout << "nombre de zero = " << nzero << "\n";
    double prob = ((double)nzero) / ((double)nperm);
    mtools::cout << "proba = " << prob << "\n";

    mtools::cout.getKey();
    }



// ***********************************************************
// Simulation of a Linearly Edge Reinforced Random Walk on Z^2.
// ***********************************************************
using namespace mtools;

// structure at each site of Z^2.
struct siteInfo
    {
    siteInfo() : up(1), right(1), V(0) {}    // ctor, set the intial weights of the edges to 1
    double up, right;   // weights of the up and right edges.
    int64 V;            // number of visits to the site.
    static int64 maxV;  // maximum number of visits of any site.
    };

int64 siteInfo::maxV = 0;
Grid_basic<2, siteInfo> G;   // the grid
MT2004_64 gen(0);

// site are colored w.r.t. the local time of the walk.
RGBc colorLERRW(iVec2 pos)
    {
    const siteInfo * S = G.peek(pos);
    if ((S == nullptr) || (S->V == 0)) return RGBc::c_TransparentWhite;
    return RGBc::jetPaletteLog(S->V, 0, S->maxV, 1.2);
    }

// site are colored w.r.t. the local time of the walk.
const cimg_library::CImg<unsigned char> * imageLERRW(mtools::iVec2 pos, mtools::iVec2 size) 
    {
    static EdgeSiteImage ES;
    static cimg_library::CImg<unsigned char> im(1, 1, 1, 3);
    const siteInfo * S = G.peek(pos);
    if ((S == nullptr) || (S->V == 0)) return nullptr;
    ES.site(true);
    ES.siteColor(RGBc::jetPaletteLog(S->V, 0, S->maxV, 1.2));
    ES.text("Azerty");
    ES.makeImage(im, size);
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return &im;
    }



int f(double x)
    {
    return (int)(x*x);
    }

int tab[1000];

// Simulate the LERRW with reinforcment parameter delta for steps unit of time.
void makeLERRW(uint64 steps, double delta)
    {
    Chronometer();
    cout << "Simulating " << steps << " steps of the LERRW with reinf. param " << delta << ".\n";
    ProgressBar<uint64> PB(steps, "Simulating..");
    iVec2 pos = { 0, 0 };
    //uint64 t = 0;
    for (uint64 n = 0; n < steps; n++)
        {
        PB.update(n);
        siteInfo & S = G[pos];                          // info at the current site
        if ((++S.V) > S.maxV) { S.maxV = S.V; }
        double & right = S.right;                       // get a reference to the weight
        double & up = S.up;                             // of the 4 adjacent edges of the
        double & left = G(pos.X() - 1, pos.Y()).right;  // current position.
        double & down = G(pos.X(), pos.Y() - 1).up;     //
        double e = gen.rand_double0()*(left + right + up + down);
        if (e < left) { left += delta; pos.X()--; }
        else {
            if (e < (left + right)) { right += delta; pos.X()++; }
            else {
                if (e < (left + right + up)) { up += delta; pos.Y()++; }
                else {
                    down += delta; pos.Y()--;
                    }
                }
            }
        }
    PB.hide();
    cout << "maxV = " << siteInfo::maxV << "\n";
    cout << "\nSimulation completed in = " << Chronometer() / 1000.0 << " seconds.\n";
    std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid.gz";
  //  cout << "\nsaving... "; Chronometer();
   // G.save(fn); // save the grid in fn
  //  cout << " saved in file " << fn << " in " << (double)Chronometer() / 1000 << "sec.\n";
  

    Plotter2D Plotter;
    auto L = makePlot2DLattice(LatticeObjImage<colorLERRW,imageLERRW>::get());
    Plotter[L];
    Plotter.gridObject(true)->setUnitCells();

    auto PF = makePlot2DFun(f);
    Plotter[PF];

    auto PT = makePlot2DArray(tab, 1000);
    Plotter[PT];

    Plotter.plot();
    return;
    }



void load(uint64 steps, double delta)
    {
    siteInfo::maxV = 45663;
    Chronometer();
    std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid.gz";
    cout << "loading " << fn << " ..";
    G.load(fn);
    cout << " done in " << (double)Chronometer() / 1000 << "sec.\n";
    Plotter2D Plotter;
    auto L = makePlot2DLattice(LatticeObj<colorLERRW>::get());
    Plotter[L];
    Plotter.gridObject(true)->setUnitCells();
    Plotter.plot();
    return;
    }




int main()
    {

    bool b;
    cout << "bool = "; cout >> b; cout << "[" << b << "]\n";
    char c;
    cout << "bool = "; cout >> c; cout << "[" << c << "]\n";

  //  load(100000000, 0.5);
    makeLERRW(1000000, 0.5);
    return 0;
    }
