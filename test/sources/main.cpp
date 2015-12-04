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








const int Rs = 3;
Grid_factor<2, int, 4, Rs> testG;

inline int depth(iRect & r)
    {
    int64 rad = (r.xmax - r.xmin) / 2;
    if (rad == 0) return 0;
    int i = 1;
    while (rad > Rs) { i++; rad = (rad - 1) / 3; }
    return i;
    }


RGBc colorTest(iVec2 pos)
    {
    iRect r;
    const int * p = testG.findFullBox(pos, r);
    int d = depth(r);
    if (p == nullptr) { return RGBc::jetPalette(20 - d, 0, 21); }
    if (d != 0) { return RGBc::jetPalette(d, 0, 21); }
    if (*p == 0) return RGBc::c_TransparentWhite;
    return RGBc::c_Black;
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






template<class random_t> void testRG()
    {
    random_t mte;

    cout << "type return  : " << typeid(random_t::result_type).name() << "\n";
    cout << "type size : " << sizeof(random_t::result_type) << "\n";
    cout << "min  : " << mte.min() << "\n";
    cout << "max  : " << mte.max() << "\n";
    cout << "random value " << Unif(mte) << "\n\n";

    Chronometer();
    double s = 0.0;
    double v = 0.0;
    int GN = 100000000;

    double mu = 7;
    double s2 = 12;

    double E = mu;
    double V = s2;

    NormalLaw B(mu,s2);

    for (int i = 0;i < GN;i++)
        {
        double  a = B(mte);
        s += a; v += (a - E)*(a - E);
        }

    double mean = ((double)s) / GN;
    double vari = ((double)v) / GN;

    cout << "Mean     = " << mean   << "\n";
    cout << "variance = " << vari << "\n";
    cout << "Finished in " << Chronometer() << "\n";
    cout << "----------------------\n\n";

    }



	
int main(int argc, char* argv[])
{


    //testRG<std::mt19937>();
    testRG<std::mt19937_64>();
    //testRG<MT2002_32>();
    //testRG<MT2004_64>();
    //testRG<XorGen4096_64>();
    //testRG<FastRNG>();


    cout.getKey();
    return 0;





        {
        const int M = 20000;             // taille du tableau
        const uint64 NBSIM = 1000000; // nombre de simulation
        const uint64 Npas = 1000006;    // nombre de pas de la marche
        const int64 Ndiv = 1;          // nombre de pas de la marche

        int64 tabF[2 * M + 1]; memset(tabF, 0, sizeof(tabF));
        double repF[2 * M + 1]; memset(tabF, 0, sizeof(repF));

        mtools::MT2004_64 g;

        ProgressBar<uint64> PB(NBSIM, "Simulating..");
        for (uint64 i = 0;i < NBSIM; i++)
            {
            PB.update(i);
            int64 x = 0; //mtools::internals_randomgen::SRW_Z_makesteps(Npas, g);
          //  cout << x << "\n";
            if ((x % 1) != (Npas % 1)) cout << "Erreur parite!\n";
            int64 index = 0;// M + (x / Ndiv) + (g() * 2);

            if ((index < 0) || (index > 2 * M)) cout << "OUT " << index << "\n";;
            tabF[index]++;
            }
        PB.hide();

        for (uint64 i = 0;i < 2 * M + 1; i++) { repF[i] = (((double)tabF[i]) / ((double)NBSIM)); }

        Plotter2D P;
        auto T = makePlot2DArray(repF, 2 * M + 1,-1.0,1.0);
        P[T];
        T.hypograph(true);
        P.range().fixedAspectRatio(false);
        T.autorangeXY();
        P.range().zoomOut();
        P.plot();
        }
    return 0;


    testG.reset(0,3,false);

    const int N = 1000;

    for(int i = -N;i < N;i++)
        for (int j = -N;j < N;j++)
            {
            if (i*i + j*j <= N*N) testG.set(i, j, 1);

            }


    Plotter2D Plotter;
    auto L = makePlot2DLattice<colorTest>();
    auto L2 = makePlot2DLattice<colorTest2>();
    Plotter[L];
    Plotter[L2];
    Plotter.gridObject(true)->setUnitCells();

    Plotter.plot();


	return 0;
}
