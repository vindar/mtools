#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;


MT2004_64 gen; // RNG

Grid_basic<2, int64, 2> Grid; // the 2D grid

double maxrad = 1;               // radius of the cluster
volatile int64 N = 1;            // number of particules in the cluster

void* peekhint = nullptr;

bool eight_neighbour = false;


/* return true if there is a neighbour */
inline bool hasNeighbour(const iVec2 & pos)
    {
            { const auto p = Grid.peek(iVec2{ pos.X() + 1,pos.Y() }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
            { const auto p = Grid.peek(iVec2{ pos.X() - 1,pos.Y() }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
            { const auto p = Grid.peek(iVec2{ pos.X(),pos.Y() + 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
            { const auto p = Grid.peek(iVec2{ pos.X(),pos.Y() - 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
            if (eight_neighbour)
                {
                        { const auto p = Grid.peek(iVec2{ pos.X() + 1,pos.Y() + 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
                        { const auto p = Grid.peek(iVec2{ pos.X() + 1,pos.Y() - 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
                        { const auto p = Grid.peek(iVec2{ pos.X() - 1,pos.Y() + 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
                        { const auto p = Grid.peek(iVec2{ pos.X() - 1,pos.Y() - 1 }, peekhint); if ((p != nullptr) && (*p != 0)) return true; }
                }
            return false;
    }


/* add particules to the cluster */
inline void addParticules(int64 nb)
    {
    for (int64 i = 0; i < nb;i++)
        {
        double radius = ((maxrad < 1000) ? 1000 : maxrad);
        double a = Unif(gen);
        iVec2 pos = { (int64)round(sin(TWOPI*a) * 3 * radius) , (int64)round(cos(TWOPI*a) * 3 * radius) }; // random starting position
        do
            {
            double v;
            while ((v = pos.norm()) > radius + 100)
                { // out of the ball that contain the cluster, we can move fast
                if (v > 100 * radius) { pos *= 9; pos /= 10; } // too far away, we drift back...
                else
                    { // make a large step
                    int64 l = (int64)((v - radius - 10)*2.0 / 3.0);
                    SRW_Z2_MoveInRect(pos, iBox2(pos.X() - l, pos.X() + l, pos.Y() - l, pos.Y() + l), 16, gen);
                    }
                }
            iBox2 fullR;
            Grid.findFullBoxCentered(pos, fullR);
            if (fullR.boundaryDist(pos) == 0)
                { // we only move by a single step
                SRW_Z2_1step(pos, gen);
                }
            else
                { // we are away from the cluster so we can make larger moves
                SRW_Z2_MoveInRect(pos, fullR, 16, gen);
                }
            }
        while (!hasNeighbour(pos));
            Grid(pos) = N;
            N++;
            double r = pos.norm();
            if (r > maxrad) { maxrad = r; }
        }
    }


RGBc colorFct(iVec2 pos, void* & data)
    {
    auto p = Grid.peek(pos,data);
    if (p == nullptr) return RGBc::c_TransparentBlack;
    if (*p == 0) return RGBc::c_TransparentBlack;
    return RGBc::jetPalette(*p, 0, N);
    }



int sim(int argc, char *argv[])
	{
	int64 maxNN = arg("N", 10000000).info("total number of particles in the simulation");
	int autoredraw = arg('a', 600).info("autoredraw per minutes");
	eight_neighbour = arg('e', false).info("use 8 neighbours adjacency");
	Grid({ 0,0 }) = N; N++; // initial particle 
	Plotter2D P;
	//auto L = makePlot2DLattice(colorFct, "external DLA 2D"); 
	auto L = makePlot2DPixel(colorFct, 7, "external DLA 2D");
	P[L];
	P.autoredraw(autoredraw);
	P.sensibility(10);
	P.startPlot();
	watch("# of particles", N);
	watch("cluster radius", maxrad);
	while (P.shown())
		{
		if (maxNN - N > 1000) { addParticules(1000); }
		else
			{
			addParticules(maxNN - N);
			cout << "Simulation completed ! \n";

			cout << "print\n";
			P.autoredraw(0);
			P.redraw();
			while (P.quality() < 100) { cout << "waiting...\n"; Sleep(1000); }
			Sleep(1000);
			cout << "saving\n";
			mtools::Img<unsigned char> im;
			P.exportImg(im);
			im.save("imtest.png");


			P.autoredraw(0);
			int64 l = (int64)maxrad + 1;
			P.range().setRange(iBox2(-l, l, -l, l));
			P.redraw();
			P.plot();
			return 0;
			}
		}
	return 0;
	}










void drawCirclePacking(fBox2 R, std::vector<double> & radiuses, std::vector<fVec2> & circles)
	{
	cout << "Number of circles: " << circles.size() << "\n";

	double ratio = (double)R.lx() / ((double)R.ly());
	int LX = 15000;
	int LY = (int)(LX / ratio);

	Img<unsigned char> Im(LX, LY, 1, 4);

	for (int i = 0;i < circles.size(); i++)
		{
		Im.fBox2_draw_circle(R, circles[i], radiuses[i], RGBc::c_Red);
		//Im.fBox2_drawPointCirclePen(R,circles[i],2, RGBc::c_Blue);
		Im.fBox2_drawText(R, toString(i + 1), circles[i], 'c', 'c', 10, false, RGBc::c_Blue);
		}
	Im.fBox2_drawAxes(R);


	Plotter2D Plotter;
	auto P = makePlot2DCImg(Im, "circlepacking");
	Plotter[P];
	Plotter.autorangeXY();
	Plotter.plot();
	}




void loadGraph(std::string filename, std::vector<std::vector<int> > & gr, fBox2 & R, std::vector<int> & boundary, std::vector<double> & radiuses, std::vector<fVec2> & circles)
	{

	gr.clear();
	boundary.clear();
	radiuses.clear();
	circles.clear();

	IArchive ar(filename);

	std::string s; // dummy
	int N;	// nombre de vertices
	int alpha, beta, gamma;

	ar & s;
	ar & N;
	ar & s;	ar & s; ar & s;
	ar & alpha; ar & beta; ar & gamma;
	ar & s;

	gr.resize(N);
	boundary.resize(N);
	for (int i = 0; i < N; i++)
		{
		int ind;  ar & ind; ind--;
		int nbchild; ar & nbchild;
		for (int j = 0; j < nbchild; j++)
			{
			int u; ar & u; u--;
			gr[ind].push_back(u);
			}
		int u; ar & u; u--;
		if (u != gr[ind].front()) { gr[ind].push_back(u);  boundary[ind] = 1; }
		}

	ar & s;

	radiuses.resize(N);
	for (int i = 0; i < N; i++)
		{
		double r;
		ar & r;
		radiuses[i] = r;
		}

	ar & s;

	R.min[0] = 0.0;
	R.max[0] = 0.0;
	R.min[1] = 0.0;
	R.max[1] = 0.0;

	circles.resize(N);
	for (int i = 0; i < N; i++)
		{
		double x, y;
		ar & x; ar & y;
		fVec2 p(x, y);
		circles[i] = p;
		double rad = radiuses[i];
		if (p.X() + rad > R.max[0]) { R.max[0] = p.X() + rad; }
		if (p.X() - rad < R.min[0]) { R.min[0] = p.X() - rad; }
		if (p.Y() + rad > R.max[1]) { R.max[1] = p.Y() + rad; }
		if (p.Y() - rad < R.min[1]) { R.min[1] = p.Y() - rad; }
		}

	cout << "Packing with " << N << " vertices\n";
	}



void loadtestgraph(std::vector<std::vector<int> > & gr, std::vector<int> & boundary)
	{
	gr.clear();
	gr.resize(6);

	gr[0].push_back(2);
	gr[0].push_back(5);
	gr[0].push_back(3);
	gr[0].push_back(1);

	gr[1].push_back(0);
	gr[1].push_back(3);
	gr[1].push_back(4);
	gr[1].push_back(2);

	gr[2].push_back(1);
	gr[2].push_back(4);
	gr[2].push_back(5);
	gr[2].push_back(0);

	gr[3].push_back(0);
	gr[3].push_back(5);
	gr[3].push_back(4);
	gr[3].push_back(1);

	gr[4].push_back(3);
	gr[4].push_back(5);
	gr[4].push_back(2);
	gr[4].push_back(1);

	gr[5].push_back(0);
	gr[5].push_back(2);
	gr[5].push_back(4);
	gr[5].push_back(3);

	boundary.clear();
	boundary.resize(gr.size());
	boundary[0] = 1;
	boundary[1] = 1;
	boundary[2] = 1;
	}



int main(int argc, char *argv[])
    {
	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);
	//sim(argc, argv);
	

	std::vector<std::vector<int> > graph;
	fBox2 R;
	std::vector<int> boundary;
	std::vector<double> radii;
	std::vector<fVec2> circles;
	
	loadGraph("color-wheel.p", graph, R, boundary, radii, circles);
	
//	loadtestgraph(graph, boundary);


	CirclePacking CP;

	CP.setTriangulation(graph, boundary);
	CP.setRadii();

	cout << "packing...\n";
	mtools::Chronometer();
	//cout << "ITER = " << CP.computeRadii(1.0e-12) << "\n";
	cout << "ITER = " << CP.computeRadiiFast(1.0e-13) << "\n";
	cout << "done in " << mtools::Chronometer() << "ms\n";



	cout << "layout...\n";
	mtools::Chronometer();
	CP.computeLayout();
	cout << "done in " << mtools::Chronometer() << "ms\n";

	radii = CP.getRadii();
	circles = CP.getLayout();
	R = CP.getEnclosingRect();
	drawCirclePacking(R, radii, circles);


    }

/* end of file main.cpp */






