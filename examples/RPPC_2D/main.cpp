/***********************************************
 * RPPC : Random Partition via Poissonian Coloring
 * 
 * Compute the partition and draw the cluster
 * and the voronoi region on the torus [0,1]^2. 
 * 
 * c.f. Aldous: https://arxiv.org/abs/1701.00131
 * and also 
 * date: 2021-10-07
 ***********************************************/

#include "mtools/mtools.hpp" 

using namespace mtools;



/**
 * 
 * Class to compute distance in the torus [0,1]^DIM.
 * 
 * This class implements the METRIC interface used by the PointSpace class
 * for nearest neighbour searches.

**/
template<int DIM> struct Metric
    {
   

    /**
    * Compute the distance between two points P1 and P2 on the torus [0,1]^DIM.
    * 
    * 
    * Functor method used by the PointSpace class for search (METRIC interface).
    **/
    double operator()(const mtools::fVec<DIM> & P1, const mtools::fVec<DIM> & P2) const
        {
        return distTorus(P1, P2);
        }


    /**
    * Compute a LOWER BOUND on the distance between point P and box B on the torus [0,1]^DIM.
    *
    * Functor method used by the PointSpace class for search (METRIC interface).
    **/
    double operator()(const mtools::fVec<DIM> & P, const mtools::fBox<DIM> & B) const
       {
       const mtools::fVec<DIM> C = B.center(); // distance to center
       const double d = distTorus(C, P) - distTorus(C, B.min); // lower bound cannot be closer than that
       return d;
       }


    /**
    * Compute the distance between two points P1 and P2 on the torus [0,1]^DIM.
    **/
    double distTorus(const mtools::fVec<DIM> & P1, const mtools::fVec<DIM> & P2) const
        {
        double sum = 0;
        for(int k = 0; k < DIM; k++)
            {
            const double dx = std::abs(P2[k] - P1[k]);
            const double a = (dx > 0.5) ? (1.0 - dx) : dx;
            sum += (a * a);
            }
        return std::sqrt(sum);
        }


    /**
    * Change point P2 to the representant (possibly outside [0,1]^DIM) that minimize
    * the euclidian distance d(P1,P2).
    **/
    void changeP2(const mtools::fVec<DIM> & P1, mtools::fVec<DIM> & P2) const
        {
        for (int k = 0; k < DIM; k++)
            {
            const double dx = P2[k] - P1[k];
            const double adx = std::abs(dx);
            if (std::abs(dx + 1) < adx) { P2[k] += 1; }
            else if (std::abs(dx - 1) < adx) { P2[k] -= 1; }
            }
        }
        


    };



/**
* Data payload for the PointSpace container.
**/
template<int DIM> struct Data
    {
    mtools::PointSpaceObj<DIM, Data>* father;   // pointer to the father of this atom
    int color;                                  // color of the atom
    int nb_childs;                              // number of childrens
    };




// here, we are in dimension 2
static const int DIMENSION = 2;


// RNG
mtools::MT2004_64 gen;


// points of the PPP.
PointSpace<DIMENSION, Data<DIMENSION>, 5> psp;


// keep a list pointer to all objects in psp
std::vector< PointSpaceObj<DIMENSION, Data<DIMENSION> > * > vec;


// metric functor for torus distance
Metric<DIMENSION> torus_distance;


// total number of color used
int nb_colors = 0;



/**
* Create the RPPC : Random partition via Poisson coloring of the plane
**/
template<int DIM> void createRPPC(int64_t nb_points, const char * filename = nullptr)
    {
    mtools::cout << "\nComputing the RPPC for " << nb_points << " points... ";
    mtools::Chrono ch;

    psp.callDtors(false);    // no need to call dtors
    psp.clear(); // reset

    // first point (the root) is centered in the torus. 
    auto r = psp.insert(psp.initialBoundingBox().center());
    r->data.color = 0;
    r->data.father = nullptr;
    r->data.nb_childs = 0; 
    vec.push_back(r);

    // add the points
    for (int i = 0; i < nb_points - 1; i++)
        {
        mtools::fVec<DIM> pos; 
        for (int k = 0; k < DIM; k++) pos[k] = Unif(gen);

        auto cl = psp.findNearest(pos, torus_distance); // find father

        int color;
        if (cl->data.color == 0)
            {
            nb_colors++; 
            color = nb_colors;
            }
        else
            {
            color = cl->data.color;
            }

        auto o = psp.insert(pos, cl);
        o->data.color = color;
        o->data.father = cl;
        o->data.nb_childs = 0;
        cl->data.nb_childs++;

        vec.push_back(o);
        }
    mtools::cout << " done in " << ch << "\n\n";
    mtools::cout << psp << "\n";
    
    mtools::cout << "number of colors : " << nb_colors << "\n";

    // compute mean number of children and mean number of siblings
    double E = 0; 
    double E2 = 0;
    for (auto v : vec)
        {
        E += v->data.nb_childs;
        E2 += ((v->data.father != nullptr) ? (v->data.father->data.nb_childs) : 1);
        }
    E /= vec.size();
    E2 /= vec.size();
    mtools::cout << "mean number of children : " << E << "\n";
    mtools::cout << "mean number of siblings : " << E2 << "\n\n";
    
    //export to csv file
    if (filename)
        {
        mtools::cout << "Exporting a [" << filename << "]... ";
        mtools::LogFile out("rnntpoints.txt", false, false);
        for (auto v : vec)
            {
            for (int k = 0; k < DIM; k++)
                {
                auto P = v->position();
                out << P[k] << ",";
                }
            out << v->data.color << "\n";
            }
        mtools::cout << "done !\n\n";
        }

    return;
    }



// object used to compute the Voronoi diagram
mtools::DelaunayVoronoi DV;


/**
* Compute the Voronoi diagram of the RPPC
**/
void makeDelaunay()
    {
    mtools::cout << "Computing the Voronoi diagram... ";
    mtools::Chrono ch;
    DV.DelaunayVertices.clear();
    for (size_t k = 0; k < (size_t)vec.size(); k++)
        {
        DV.DelaunayVertices.push_back(vec[k]->position());
        }
    DV.compute();
    mtools::cout << "done in " << ch << "\n\n";
    return;
    }




/**
* Display the RPPC. 
**/
void drawRNNT()
    {
    mtools::cout << "Drawing the RNNT ... ";
    mtools::Chrono ch;

    mtools::FigureCanvas canvas(3);

    for (auto o : vec)
        {
        auto rgb = mtools::Palette::mix_32[o->data.color];

        canvas(mtools::Figure::CircleDot(o->position(), 2, rgb),0); // dot for points (on layer 0)

        if (o->data.father != nullptr)
            {
            mtools::fVec2 P2 = o->data.father->position();
            torus_distance.changeP2(o->position(), P2);

            mtools::fVec2 C = (o->position() + P2) / 2.0;
            canvas(mtools::Figure::Line(o->position(), C, rgb), 1);  // arrow toward father (on layer 1)
            canvas(mtools::Figure::Line(C, P2, rgb.getMultOpacity(0.2f)), 1);  // arrow toward father (on layer 1)
            }
        }

    for (int k = 0; k < (int)DV.VoronoiEdgesIndices.size(); k++)
        {
        auto e = DV.VoronoiEdgesIndices[k];
        auto f = DV.DelaunayEdgesIndices[k];

        if (vec[f.X()]->data.color != vec[f.Y()]->data.color)
            {
            if (e.Y() == -1)
                {
                mtools::fVec2 P1 = DV.VoronoiVertices[e.X()];
                canvas(mtools::Figure::CircleDot(P1, 5, mtools::RGBc::c_Red), 0);
                mtools::fVec2 N = DV.VoronoiNormals[e.X()];
                N.normalize();
                mtools::fVec2 P2 = P1 + N;
                canvas(mtools::Figure::Line(P1, P2, mtools::RGBc::c_Red), 2);
                }
            else
                {
                mtools::fVec2 P1 = DV.VoronoiVertices[e.X()];
                mtools::fVec2 P2 = DV.VoronoiVertices[e.Y()];
                //if (dist(P1, P2) < 0.1)
                 canvas(mtools::Figure::Line(P1, P2, mtools::RGBc::c_Black), 2);
                }
            }
        }

    mtools::cout << " done in " << ch << "\n\n";

    mtools::Plotter2D plotter;
    plotter.setDrawingSize(1000, 1000);
    auto P = mtools::makePlot2DFigure(canvas, 8, "RPPC (2D)");

    P.showLayer(0, true);
    P.showLayer(1, true);
    P.showLayer(2, true);
    plotter[P];
    plotter.range().setRange(mtools::fBox2(0,1,0,1));
    plotter.range().zoomOut();
    plotter.plot();
    }





int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc,argv); // required on OSX, does nothing on Linux/Windows
    mtools::parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
    int64_t n = mtools::arg("number of points on the torus ?", 1000);

    createRPPC<DIMENSION>(n);
    makeDelaunay(); 
    drawRNNT();

    mtools::cout.getKey();	
    return 0;
    }


/* end of file main.cpp */
