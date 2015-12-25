/***********************************************
 * Project : TreeEden
 * date : Sat Jul 25 10:39:35 2015
 ***********************************************/


#include "stdafx.h" // pre-compiled header. 

 // *** Library mtools ***
#include "mtools.hpp"
using namespace mtools;



/* information contained at a site of the lattice */
struct siteInfo
    {
    siteInfo() : N(0), direction(0) {}
    int64 N;        // index (time of infection) 0 = no infected yet
    char direction; // direction of the parent site (from which the infection originated) 0 : none (origin), 1 up 2 down 3 left 4 rihht
                    /* serialize/deserialize */
    template<typename Archive> void serialize(Archive & ar) { ar & N & direction; } // serialize/deserialize the object
    };



/* Tree Eden model : Only boundary sites with a single neighour inside the cluster can become infected. */
class TreeEdenCluster
{
public:
    /* construct, initially empty cluster */
    TreeEdenCluster() : N(0), Grid(), Urn(), gen() { clear(); image.resize(1, 1, 1, 3, -1); }

    /* load from a file */
    void load(const std::string & filename) { IArchive ar(filename); ar & (*this); }

    /* save into a file */
    void save(const std::string & filename) { OArchive ar(filename); ar & (*this); }

    /* restart from scratch */
    void clear()
        {
        Grid.reset(); Grid(0, 0).direction = 1; Urn.clear(); Urn.insert({ 0,0 });   // 1 imaginary neighours of the origin so that it is infected at the first step.
        N = 0; // no particle in the cluster yet.
        }

    /* grow the cluster by a given number of particles */
    void simulate(size_t steps)
        {
        int64 finalN = N + steps;
        while (N < finalN)
            {
            iVec2 & rpos = Urn(Unif(gen)); // pick a border site
            iVec2 pos = rpos; // save the position 
            Urn.remove(rpos); // remove it from the urn
            auto & S = Grid(pos);
            if (S.direction == 1)
                { // yes, only one neighour.
                N++;
                S.N = N;
                iVec2 upPos(pos.X(), pos.Y() + 1); auto & up = Grid(upPos); if (up.N > 0) { S.direction = 1; } else { if ((up.direction)++ == 0) Urn.insert(upPos); }
                iVec2 downPos(pos.X(), pos.Y() - 1); auto & down = Grid(downPos); if (down.N > 0) { S.direction = 2; } else { if ((down.direction)++ == 0) Urn.insert(downPos); }
                iVec2 leftPos(pos.X() - 1, pos.Y()); auto & left = Grid(leftPos); if (left.N > 0) { S.direction = 3; } else { if ((left.direction)++ == 0) Urn.insert(leftPos); } 
                iVec2 rightPos(pos.X() + 1, pos.Y()); auto & right = Grid(rightPos); if (right.N > 0) { S.direction = 4; } else { if ((right.direction)++ == 0) Urn.insert(rightPos); } 
                }
            }
        }

    /* size of the cluster */
    inline int64 size() { return N; }

    /* range of the cluster */
    inline fBox2 range() { return fBox2(Grid.getPosRangeiBox2()); }

    /* print some info */
    std::string toString() const { return std::string("Number of particles in the cluster: ") + mtools::toString(N) + "\nBoundary: " + mtools::toString(Urn) + "\nGrid: " + mtools::toString(Grid) + "\n"; }

    /* serialize/deserialize */
    template<typename Archive> void serialize(Archive & ar) { ar << "Tree Eden Model\n"; ar & N & Urn & Grid; }

    /* color function for the cluster */
    RGBc getColor(iVec2 pos)
        {
        auto v = Grid.peek(pos);
        if ((v == nullptr) || ((*v).N == 0)) return RGBc::c_TransparentWhite; else return RGBc::jetPalette((*v).N, 1, N);
        }

    /* detail : image associated with a site */
    const CImg<unsigned char> * getImage(mtools::iVec2 pos, mtools::iVec2 size)
        {
        auto v = Grid.peek(pos);
        if ((v == nullptr) || ((*v).N == 0)) return nullptr;
        EdgeSiteImage ES;
        ES.site(true, RGBc::jetPalette((*v).N, 1, N));
        ES.text(mtools::toString(v->N)).textColor(RGBc::c_White);
        if (v->direction == 1) { ES.up(ES.ARROW_INGOING);}
        if (v->direction == 2) { ES.down(ES.ARROW_INGOING); }
        if (v->direction == 3) { ES.left(ES.ARROW_INGOING); }
        if (v->direction == 4) { ES.right(ES.ARROW_INGOING); }
        auto up = Grid.peek(pos.X(), pos.Y() + 1); if ((up !=nullptr) && (up->N >0) && (up->direction == 2)) { ES.up(ES.EDGE); }
        auto down = Grid.peek(pos.X(), pos.Y() - 1); if ((down != nullptr) && (down->N >0) && (down->direction == 1)) { ES.down(ES.EDGE); }
        auto left = Grid.peek(pos.X() - 1, pos.Y()); if ((left != nullptr) && (left->N >0) && (left->direction == 4)) { ES.left(ES.EDGE); }
        auto right = Grid.peek(pos.X() + 1, pos.Y()); if ((right != nullptr) && (right->N >0) && (right->direction == 3)) { ES.right(ES.EDGE); }
        ES.makeImage(image, size);
        return(&image);
        }

private:
    int64 N;                                     // number of particles in the cluster
    Grid_basic<2,siteInfo> Grid;                 // the 2D grid
    RandomUrn<iVec2> Urn;                        // list of neighour sites of the current domain
    MT2004_64 gen;                               // random number generator
    CImg<unsigned char> image;                   // the cimg image to draw sites
};


TreeEdenCluster EC; // the main TreeEden cluster object

int64 printSize(TreeEdenCluster & EC) { return EC.size(); }

/* run the simulation */
void run()
{
    cout << "\nSimulating : zoom in to view the details of the tree structure...\n";
    auto P = makePlot2DLattice(EC, "Tree Eden model"); P.opacity(0.5);
    P.setImageType(P.TYPEIMAGE);
    Plotter2D Plotter;
    Plotter[P];
    Plotter.startPlot();
    Plotter.range().setRange(unionRect(mtools::zoomOut(EC.range()), fBox2(-5000, 5000, -5000, 5000)));
    Plotter.autoredraw(300);
    cout << EC.toString();
    watch("Cluster size", EC, printSize);
    while (Plotter.shown())
        {
        if (EC.size() % 10000000 == 0) cout << EC.toString();
        EC.simulate(1000000);
        }
    watch.remove("Cluster size");
    return;
}


int main(int argc, char *argv[])
{
    cout << "Tree Eden model\n";
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
        if ((c == 'S') || (c == 's')) { std::string filename = cout.ask("Name of the file to save (.gz to compress)"); try { EC.save(filename); } catch (...) { cout << "*** ERROR ***"; } }
        if ((c == 'R') || (c == 'r')) { run(); }
        if ((c == 'N') || (c == 'n')) { EC.clear(); }
        if ((c == 'Q') || (c == 'q')) { return 0; }
    }
    return 0;
}

/* end of file main.cpp */

