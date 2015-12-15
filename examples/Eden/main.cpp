/***********************************************
 * Project : Eden
 * date : Tue Jul 21 18:06:26 2015
 ***********************************************/
#include "stdafx.h" // pre-compiled header. 

// *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;

/* Eden model : FPP with exp. weights */
class EdenCluster
    {
    public:
    /* construct, initially empty cluster */
    EdenCluster() : N(0), Grid(5, 5, false), Urn(), gen() { clear(); }

    /* load from a file */
    void load(const std::string & filename) { IArchive ar(filename); ar & (*this); }

    /* save into a file */
    void save(const std::string & filename) { OArchive ar(filename); ar & (*this); }

    /* restart from scratch */
    void clear()
        {
        Grid.reset(); Grid.set({ 0,0 }, 4); Urn.clear(); Urn.insert({ 0,0 });   // 4 imaginary neighours of the origin so that it is infected at the first step.
        N = 0; // no particle in the cluster yet.
        }
        
    /* grow the cluster by a given number of particles */
    void simulate(size_t steps)
        {
        int64 finalN = N + steps;
        while (N < finalN)
            {
            iVec2 & rpos = Urn(Unif(gen)); // pick a border site
            if (Unif(gen) * 4 >= (4 - Grid(rpos)))
                { // ok, new point in the cluster
                iVec2 pos = rpos; // save the position 
                Urn.remove(rpos); // remove it from the urn
                Grid.set(pos, 5); // mark as belonging to the cluster
                iVec2 upPos(pos.X(), pos.Y() + 1); char up = Grid(upPos); if (up == 0) { Urn.insert(upPos); } if (up != 5) { Grid.set(upPos, up + 1); }
                iVec2 downPos(pos.X(), pos.Y() - 1); char down = Grid(downPos); if (down == 0) { Urn.insert(downPos); } if (down != 5) { Grid.set(downPos, down + 1); }
                iVec2 leftPos(pos.X() + 1, pos.Y()); char left = Grid(leftPos); if (left == 0) { Urn.insert(leftPos); } if (left != 5) { Grid.set(leftPos, left + 1); }
                iVec2 rightPos(pos.X() - 1, pos.Y()); char right = Grid(rightPos); if (right == 0) { Urn.insert(rightPos); } if (right != 5) { Grid.set(rightPos, right + 1); }
                N++;
                }
            }
        }

    /* size of the cluster */
    inline int64 size() { return N; }

    /* range of the cluster */
    inline fRect range() { return fRect(Grid.getPosRangeiRect()); }

    /* print some info */
    std::string toString() const { return std::string("Number of particles in the cluster: ") + mtools::toString(N) + "\nBoundary: " + mtools::toString(Urn) + "\nGrid: " + mtools::toString(Grid) + "\n"; }

    /* serialize/deserialize */
    template<typename Archive> void serialize(Archive & ar) { ar << "Eden Model\n"; ar & N & Urn & Grid; }

    /* color function for the cluster */
    RGBc getColor(iVec2 pos)
        {
        auto v = Grid.safePeek(pos);
        if ((v == nullptr) || ((*v) == 0)) return RGBc::c_TransparentWhite; else return RGBc::jetPalette(*v, 1, 5);
        }

    private:
        int64 N;                                     // number of particles in the cluster
        Grid_factor<2, char, 2> Grid;                // the 2D grid. 0 if no particle, 1-4 = nb of neighour, 5 = in the cluster
        RandomUrn<iVec2> Urn;                        // list of neighour sites of the current domain
        MT2004_64 gen;                               // random number generator
    };


EdenCluster EC; // the Eden cluster object

/* Circle with same volume as the random cluster */
RGBc colorCircle(iVec2 pos)
    {
    const double Pi = 3.14159265358979;
    if (pos.X()*pos.X() + pos.Y()*pos.Y() <= EC.size() / Pi) return RGBc::c_Cyan; else return RGBc::c_TransparentWhite;
    }


/* run the simulation */
void run()
    {
    cout << "\nSimulating (close the plotter window to stop)...\n";
    auto P1 = makePlot2DLattice(EC, "Eden model"); P1.opacity(0.5);
    auto P2 = makePlot2DLattice(LatticeObj<colorCircle>::get(), "Perfect circle"); P2.opacity(0.5);
    Plotter2D Plotter;
    Plotter[P2][P1];
    Plotter.startPlot();
    Plotter.range().setRange(unionRect(mtools::zoomOut(EC.range()), fRect(-5000, 5000, -5000, 5000)));
    Plotter.autoredraw(300);
    cout << EC.toString();
    while (Plotter.shown())
        {
        if (EC.size() % 10000000 == 0) cout << EC.toString();
        EC.simulate(1000000);
        }
    return;
    }


int main(int argc, char *argv[])
    {
    cout << "Eden model (FPP with exp weights on edges). 'Infinite simulation'\n\n";
    while (1)
        {
        cout << "\n\n-----------------------------\n";
        cout << "Number of particles in the cluster : " << EC.size() << "\n";
        cout << "(L) Load a simulation.\n";
        cout << "(S) Save the simulation.\n";
        cout << "(N) New simulation.\n";
        cout << "(R) Run the simulation.\n";
        cout << "(Q) Quit.\n";
        char c = cout.getKey();
        if ((c == 'L') || (c == 'l')) { std::string filename = cout.ask("Name of the file to load"); try { EC.load(filename); } catch (...) { cout << "*** ERROR ***"; EC.clear(); } }
        if ((c == 'S') || (c == 's')) { std::string filename = cout.ask("Name of the file to save (.gz to compress)");  try { EC.save(filename); } catch (...) { cout << "*** ERROR ***"; } }
        if ((c == 'R') || (c == 'r')) { run(); }
        if ((c == 'N') || (c == 'n')) { EC.clear(); }
        if ((c == 'Q') || (c == 'q')) { return 0; }
        }
    return 0;
    }

/* end of file main.cpp */

