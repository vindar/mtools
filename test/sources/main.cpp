/*
* @code{.cpp}
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
MT2004_64 gen;

// site are colored w.r.t. the local time of the walk.
RGBc colorLERRW(iVec2 pos)
{
const siteInfo * S = G.peek(pos);
if ((S == nullptr) || (S->V == 0)) return RGBc::c_TransparentWhite;
return RGBc::jetPaletteLog(S->V, 0, S->maxV, 1.2);
}

// Simulate the LERRW with reinforcment parameter delta for steps unit of time.
void makeLERRW(uint64 steps, double delta)
{
Chronometer();
cout << "Simulating " << steps << " steps of the LERRW with reinf. param " << delta << ".\n";
ProgressBar<uint64> PB(steps,"Simulating..");
iVec2 pos = { 0, 0 };
uint64 t = 0;
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
down += delta; pos.Y()--; }}}
}
PB.hide();
cout << "\nSimulation completed in = " << Chronometer() / 1000.0 << " seconds.\n";
std::string fn = std::string("LERRW-N") + mtools::toString(steps) + "-d" + mtools::doubleToStringNice(delta) + ".grid";
G.save(fn); // save the grid in fn
cout << "- saved in file " << fn << "\n";
Plotter2D Plotter;
auto L = makePlot2DLattice(LatticeObj<colorLERRW>::get());
Plotter[L];
Plotter.gridObject(true)->setUnitCells();
Plotter.plot();
return;
}

int main()
{
makeLERRW(100000000, 0.5);
return 0;
}
*/





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
    static cimg_library::CImg<unsigned char> im(1, 1, 1, 4);
    const siteInfo * S = G.peek(pos);
    if ((S == nullptr) || (S->V == 0)) return nullptr;
    ES.site(true);
    ES.siteColor(RGBc::jetPaletteLog(S->V, 0, S->maxV, 1.2));
    ES.text("Azerty");
    ES.bkColor(RGBc::c_TransparentWhite);
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
    auto L = makePlot2DLattice<colorLERRW,imageLERRW>();
    L.domain(iRect(-10, 10, -12, 15));
//    Plotter[L];
//    
    Plotter.gridObject(true)->setUnitCells();

    auto PF = makePlot2DFun(f);
    //Plotter[PF];




    CImg<unsigned char> im;
    im.load("lenna.jpg");

    auto LENA = makePlot2DCImg(im, "Lenna");

    Plotter[LENA];



    auto PT = makePlot2DArray(tab, 1000);
    //Plotter[PT];

    Plotter.plot();
    return;

    
    LENA.position(LENA.TYPECENTER);

    cout.getKey();
    LENA.image(nullptr);

    cout.getKey();
    LENA.position(LENA.TYPEBOTTOMLEFT);

    CImg<unsigned char> im2;
    im2.load("lenna.jpg");
    LENA.image(im2);


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
    auto L = makePlot2DLattice(LatticeObjImage<colorLERRW, imageLERRW>::get());
    Plotter[L];
    Plotter.gridObject(true)->setUnitCells();
    Plotter.plot();
    return;
    }



class A
    {
    public:
        int * getColor(iVec2 & pos) { return 0; }

    };





int nbIter;

/* return the color associated with point pos in the mandelbrot set 
by calculating at most nbIter iteration */
RGBc mandelbrot(fVec2 pos)
    {
    const double cx = pos.X();
    const double cy = pos.Y();
    double X = 0.0;
    double Y = 0.0;
    for(int i = 0; i < nbIter; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2*sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return RGBc::jetPalette(i, 1, nbIter); }
        }
    return RGBc::c_Black;
    }



Grid_factor<2, int, 10> GF(0, 3, true);

Grid_basic<2, int, 2> GS;


RGBc colorGF(iVec2 pos)
    {
    const int * S = GF.peek(pos);
    if (S == nullptr) return RGBc::c_TransparentWhite;
    if (*S == 0) return RGBc::c_Cyan;
    if (*S == 1) return RGBc::c_Blue;
    if (*S == 2) return RGBc::c_Green;
    if (*S == 3) return RGBc::c_Red;
    return RGBc::c_Orange;
    }

void testWalk(int64 N)
    {
    iVec2 pos = { 0,0 };
    for (int64 i = 0;i < N/2;i++)
        {
        GF.set(pos, 1);
        double a = gen.rand_double0();
        if (a < 0.25) pos.X()--; else
        if (a < 0.5) pos.X()++; else
        if (a < 0.75) pos.Y()--; else pos.Y()++;
        }
    for (int64 i = 0;i < N / 2;i++)
        {
        GF.set(pos, 3);
        iRect R;
        GF.findFullBoxiRect(pos, R);

        if (R.lx()*R.ly() >0)
            {
       //     cout << R << "\n";
        //    cout << pos << "\n\n";
            }
        double a = gen.rand_double0();
        if (a < 0.25) pos.X()--; else
        if (a < 0.5) pos.X()++; else
        if (a < 0.75) pos.Y()--; else pos.Y()++;
        }
}



void fillSqr(iRect R, int val)
    {
    for (int64 j = R.ymin; j <= R.ymax; j++)
        for (int64 i = R.xmin; i <= R.xmax; i++)
        GF.set({ i,j }, val);
    }

int main()
    {
    cout.useDefaultInputValue(true);
    cout << "Hello World\n";
    int i = 10;
    cout >> i;

    /*
        fillSqr(iRect(-7, 7, -7, 7), 2);

        GF.set({ 3,3 }, 1);
        GF.set({ 8,0 }, 1);
        GF.set({ 8,0 }, 0);
        GF.set({ 3,3 }, 2);
        */

    Chronometer();

    testWalk(100000);

    void * p = nullptr;
    GS.peek(iVec2(0, 0), p);
    GS.save("hello");
  
    
    cout << "\ntime = " << Chronometer() << "\n"; cout << GF.toString(false);

    //GF.save("walklong2.ar.gz");

    GF.callDtors(false);

    cout << "\ntime = " << Chronometer() << "\n"; cout << GF.toString(false);

    Plotter2D Plotter;

    Plotter.gridObject(true)->setUnitCells();
    auto PGF = makePlot2DLattice<colorGF>("Grid factor");
    Plotter[PGF];
    Plotter.plot();


    return 0;

    /*
    cout << "Mandelbrot set demo.\n";
    cout << "Max number of iteration (1-1024) ? ";
    cout >> nbIter;
    if (nbIter < 1) nbIter = 1; else if (nbIter >104) nbIter = 1024;
    cout << nbIter << "\n"; 
    Plotter2D Plotter;  // create the plotter
    auto PD = makePlot2DPlane<mandelbrot>("Mandelbrot Set");
    
    
    Plotter[PD];
    Plotter.range().setRange(fRect(-2, 2, -2, 2));


    Plotter.startPlot();
    cout.getKey();
    PD.domain(fRect(-2, 0.5, -0.3, 1.2));
    cout.getKey();
    PD.domainEmpty();
    cout.getKey();
    PD.domainFull();
    Plotter.plot();
    return 0;
  */

  //  load(100000000, 0.5);
    makeLERRW(1000000, 0.5);
    return 0;
    }
