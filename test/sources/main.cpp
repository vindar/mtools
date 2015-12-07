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
        double e = Unif(gen)*(left + right + up + down);
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








const int Rs = 2;
Grid_factor<2, int, 4, Rs> testG;
//Grid_basic<2, int,  Rs> testG;

int mx = 0;


inline int depth(iRect & r)
    {
    int64 rad = (r.xmax - r.xmin) / 2;
    if (rad == 0) return 0;
    int i = 1;
    while (rad > Rs) { i++; rad = (rad - 1) / 3; }
    return i;
    }


RGBc colorTest1A(iVec2 pos)
    {
    iRect r;
    const int * p = testG.findFullBox(pos, r);
    int d = depth(r);
    if (p == nullptr) { return RGBc::jetPalette(20 - d, 0, 21); }
    if (d != 0) { return RGBc::jetPalette(d, 0, 21); }
    if (*p == 0) return RGBc::c_TransparentWhite;
    return RGBc::c_Black;
    }

RGBc colorTest1B(iVec2 pos)
    {
    iRect r;
    const int * p = testG.findFullBoxCentered(pos, r);
    int d = depth(r);
    if (p == nullptr) { return RGBc::jetPalette(20 - d, 0, 21); }
    if (d != 0) { return RGBc::jetPalette(d, 0, 21); }
    if (*p == 0) return RGBc::c_TransparentWhite;
    return RGBc::c_Black;
    }



RGBc colorTest2A(iVec2 pos)
    {
    iRect r;
    const int * p = testG.findFullBox(pos, r);
    if (r.lx() == 0)
        { // nothing
        if (*p == 0) return RGBc::c_TransparentWhite;
        return RGBc::c_Black;
        }
    int64 v = r.boundaryDist(pos);
    if (v < 0) MTOOLS_ERROR("oups");
    if (p == nullptr)
        { 
        if (v == 0) return RGBc::c_Salmon;
        return RGBc::jetPalette(v, 0, 100); 
        }
    if (v == 0) return RGBc::c_Salmon;
    return RGBc::jetPalette(v, 0, 100);
    }

RGBc colorTest2B(iVec2 pos)
    {
    iRect r;
    const int * p = testG.findFullBoxCentered(pos, r);
    if (r.lx() == 0)
        { // nothing
        if (*p == 0) return RGBc::c_TransparentWhite;
        return RGBc::c_Black;
        }
    int64 v = r.boundaryDist(pos);


    if (v < 0) MTOOLS_ERROR("oups");

    /*
    if (p != nullptr)
        {
        return RGBc::jetPaletteLog(v, 0, 100, 1.1);
        }
        */

    if (v<6) return RGBc::c_Salmon;

    return RGBc::jetPaletteLog(v, 0, 7000, 1.1);

    if (p == nullptr)
        {
        if (v == 0) return RGBc::c_Salmon;
        return RGBc::jetPaletteLog(v, 0, mx);
        }

    if (v == 0) return RGBc::c_Salmon;
    if (v > 100) return RGBc::c_Red;
    if (v > 50) return RGBc::c_Yellow;
    if (v > 20) return RGBc::c_Green;
    if (v > 10) return RGBc::c_Purple;
    if (v > 5) return RGBc::c_Olive;
    return RGBc::c_Blue;
    }


RGBc colorTest2(iVec2 pos)
    {
    iRect r, r2;
    const int * p = testG.findFullBox({ pos.X(), pos.Y() }, r);
    testG.findFullBox({ pos.X(), pos.Y() + 1 }, r2);  if (r2 != r) return RGBc::c_Red;
    testG.findFullBox({ pos.X(), pos.Y() - 1 }, r2);  if (r2 != r) return RGBc::c_Red;
    testG.findFullBox({ pos.X() + 1, pos.Y() }, r2);  if (r2 != r) return RGBc::c_Red;
    testG.findFullBox({ pos.X() - 1, pos.Y() }, r2);  if (r2 != r) return RGBc::c_Red;
    return RGBc::c_TransparentWhite;
    if (p == nullptr) return RGBc::c_Cyan;
    if (*p == 0) return RGBc::c_TransparentWhite;
    return RGBc::c_Black;
    }







	
int main(int argc, char* argv[])
{

    testG.reset(0,3,false);
    //testG.reset();

    const int N = 1000;

    for(int i = -N;i < N;i++)
        for (int j = -N;j < N;j++)
            {
            if ((i*i + j*j <= N*N) 
                && (i*i + j*j >= 3 * N*N / 4)
                ) testG.set({ i, j }, 1);
            }


    testG.set({ 0, 0 }, 2);
    iRect rr;

    testG.findFullBoxCentered({ 13,13 }, rr);
    cout << rr << "\n";
    mx =  rr.boundaryDist({ 13,13 });
    cout << mx << "\n";


    //return 0;

    Plotter2D Plotter;
    auto L1A = makePlot2DLattice<colorTest1A>();
    auto L1B = makePlot2DLattice<colorTest1B>();

    auto L2A = makePlot2DLattice<colorTest2A>("normal");
    auto L2B = makePlot2DLattice<colorTest2B>("centered");
    //    auto L3 = makePlot2DLattice<colorTest3>();
//    auto L4 = makePlot2DLattice<colorTest4>();
    Plotter[L2A];
    Plotter[L2B];
//    Plotter[L2];
//    Plotter[L3];
//    Plotter[L4];
    Plotter.gridObject(true)->setUnitCells();
    Plotter.plot();

	return 0;
}
