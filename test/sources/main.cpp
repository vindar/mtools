#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;








MT2004_64 gen(31135); // RNG
//MT2004_64 gen(44); // RNG

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










void drawCirclePacking(fBox2 R, std::vector<double> & radiuses, std::vector<fVec2> & circles, std::vector<std::vector<int> > & gr)
	{
	cout << "Number of circles: " << circles.size() << "\n";

	double ratio = (double)R.lx() / ((double)R.ly());
	int LX = 15000;
	int LY = (int)(LX / ratio);

	mtools::Img<unsigned char> Im(LX, LY, 1, 4);

	for (int i = 0;i < circles.size(); i++)
		{
		if (i != circles.size()-1)
			{
			Im.fBox2_draw_circle(R, circles[i], radiuses[i], RGBc::c_Red, 0.7f);
			}
		else
			{
			Im.fBox2_draw_circle(R, circles[i], radiuses[i], RGBc::c_Green, 0.1f);
			}
		//Im.fBox2_drawPointCirclePen(R,circles[i],2, RGBc::c_Blue);
		//Im.fBox2_drawText(R, toString(i + 1), circles[i], 'c', 'c', 20, false, RGBc::c_Blue);
		}

	for (int i = 0;i < circles.size() - 1; i++)
		{
		for (int j = 0;j < gr[i].size(); j++)
			{
			if (gr[i][j] != circles.size() - 1)
				{
				Im.fBox2_drawLine(R, circles[i], circles[gr[i][j]], RGBc::c_Black);
				}
			}
		}


	Im.fBox2_drawAxes(R,RGBc::c_Green);


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






std::vector<int> markToRemove(std::vector<std::vector<int> > & gr, std::vector<int> & dist, int dmin, int dmax, std::vector<int> & bound)
	{
	bound.clear();
	bound.resize(gr.size(), 0);
	std::vector<int> marked(gr.size(), 0);
	int o = -1;
	for (int i = 0; i < gr.size(); i++) { if (dist[i] == dmax) { o = i; } }
	marked[o] = 1;
	std::set<int> nei1;
	std::set<int> nei2;
	std::set<int> * cnei = &nei1;
	std::set<int> * cneialt = &nei2;

	for (int i = 0; i < gr[o].size(); i++) 
		{ 
		if (dist[gr[o][i]] == dmin) { bound[gr[o][i]] = 1; } else
		if ((dist[gr[o][i]] > dmin) && (marked[gr[o][i]] == 0)) { cnei->insert(gr[o][i]); } 
		}

	while (cnei->size() != 0)
		{
		cneialt->clear();
		for (auto it = cnei->begin(); it != cnei->end(); it++) { marked[*it] = 1;  }
		for (auto it = cnei->begin(); it != cnei->end(); it++)
			{
			int k = (*it);
			for (int j = 0; j < gr[k].size(); j++)
				{
				int n = gr[k][j];
				if (dist[n] == dmin) { bound[n] = 1; } 
				if ((dist[n] > dmin) && (marked[n] == 0)) { cneialt->insert(n); }
				}
			}
		if (cnei == &nei1) { cnei = &nei2; cneialt = &nei1; } else { cnei = &nei1; cneialt = &nei2; }
		}
	return marked;
	}




void loadtestgraph(std::vector<std::vector<int> > & gr, std::vector<int> & boundary)
	{
	gr.clear();
	gr.resize(6);

	gr[0].push_back(2);
	gr[0].push_back(5);
	gr[0].push_back(3);
	//gr[0].push_back(1);

	//gr[1].push_back(0);
	//gr[1].push_back(3);
	gr[1].push_back(4);
	gr[1].push_back(2);

	gr[2].push_back(1);
	gr[2].push_back(4);
	gr[2].push_back(5);
	gr[2].push_back(0);

	gr[3].push_back(0);
	gr[3].push_back(5);
	gr[3].push_back(4);
	//gr[3].push_back(1);

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




void testTriangulation()
	{
	int sizeTrig = 10000;

	cout << "\n\n\n\n" << Unif(gen) << "\n";

	mtools::Chronometer();
	DyckWord D(sizeTrig, 3);
	D.shuffle(gen,false);

	cout << "tree created in " << mtools::Chronometer() << " ms\n";

	CombinatorialMap CM(D);

	int a, b, c, nbv;

	std::tie(a,b,c) = CM.btreeToTriangulation();
	cout << "triangulation created in " << mtools::Chronometer() << " ms\n";

	auto gr = CM.toGraph();
	cout << "converted in graph in " << mtools::Chronometer() << " ms\n";

	nbv = CM.nbVertices();
	auto V = CM.getVerticeVector();
	int v1 = V[a];
	int v2 = V[b];
	int v3 = V[c];
	cout << v1 << " " << v2 << " " << v3 << "\n\n";

	fBox2 R;
	std::vector<int> boundary;
	boundary.resize(gr.size());
	boundary[v1] = 1;
	boundary[v2] = 1;
	boundary[v3] = 1;

	std::vector<double> radii;
	std::vector<fVec2> circles;

	CirclePackingLabelGPU<double> CP;

	CP.setTriangulation(gr, boundary);
	CP.setRadii();

	cout << "packing...\n";
	mtools::Chronometer();
	cout << "ITER = " << CP.computeRadii(1.0e-5) << "\n";
	cout << "done in " << mtools::Chronometer() << "ms\n";

	CirclePacking CP2;
	CP2.setTriangulation(gr, boundary);
	CP2.setRadii(CP.getRadii());
	cout << "layout...\n";
	mtools::Chronometer();
	CP2.computeLayout();
	cout << "done in " << mtools::Chronometer() << "ms\n";

	radii = CP2.getRadii();
	circles = CP2.getLayout();
	R = CP2.getEnclosingRect();
	drawCirclePacking(R, radii, circles, gr);

	return;
	}


/**
 * Rotate the vectro such that the ith element is now in first place
 *
 * @param	i		   	Zero-based index of the.
 * @param [in,out]	vec	The vector.
 */
void rotate(int i, std::vector<int> & vec)
	{
	const int l = (int)vec.size();
	std::vector<int> vec2(l);
	if ((i % l) != 0)
		{
		for (int k = 0; k < l; k++) { vec2[k] = vec[(k + i) % l]; }
		vec = vec2;
		}
	}

void closeBoundary(std::vector< std::vector<int> > & gr, std::vector<int> & bound)
	{
	int l = (int)gr.size();
	int nbb = 0;
	int bv = -1;
	for (int i = 0; i < l; i++)
		{
		if (bound[i] > 0)
			{
			nbb++;
			bv = i;
			const int m = (int)gr[i].size();
			int k;
			for (k = 0; k < m; k++)
				{
				if ((bound[gr[i][k]] > 0) && (bound[gr[i][(k + 1) % m]] > 0)) 
					{ 
					rotate(k + 1, gr[i]); 
					gr[i].push_back(l); k = m + 2;
					}
				}
			MTOOLS_INSURE(k == (m + 3));
			}
		}
	MTOOLS_INSURE(bv >= 0);

	gr.resize(gr.size() + 1);
	gr.back().reserve(nbb);
	gr.back().push_back(bv);
	int k = gr[bv][gr[bv].size() - 2];
	while (k != bv) { gr.back().push_back(k); k = gr[k][gr[k].size() - 2];  }
	MTOOLS_INSURE(gr.back().size() == nbb);
	}



void remove_last_vertex(std::vector< std::vector<int> > & gr)
	{
	const int l = (int)gr.size() - 1;
	gr.resize(l);
	for (int i = 0; i < l; i++)
		{
		for (int k = 0; k < gr[i].size(); k++)
			{
			if (gr[i][k] == l) { gr[i].erase(gr[i].begin() + k); }
			}
		}
	}



void loadTest(std::string filename)
	{

	fBox2 R;
	std::vector<double> radii;
	std::vector<fVec2> circles;
	std::vector<std::vector<int>> gr;
	std::vector<int> bound;


	{
	mtools::IArchive ar(filename);
	ar & gr; 
	ar & bound; 
	ar & radii; 
	ar & circles; 
	}

	CirclePackingLabel<double> CPTEST(true);
	CPTEST.setTriangulation(gr, bound);

	std::vector<double> rr(gr.size());
	for (int i = 0;i < gr.size(); i++) { rr[i] = radii[i]; }

	std::sort(rr.begin(), rr.end());

	cout << "\n";
	cout << "min radius = "<< rr[3] << "\n";
	cout << "max radius = " << rr.back() << "\n\n";

	CPTEST.setRadii(radii);

	cout << "repacking...\n";

	auto cc = chrono();
	cout << "ITER = " << CPTEST.computeRadii(1.0e-10,0.05,100,1000) << "\n";
	cout << "done in " << cc << "\n";


	CirclePacking CP;
	CP.setTriangulation(gr, bound);

	auto R1 = CPTEST.getRadii();
	std::vector<double> R2(gr.size());
	for (int i = 0;i < gr.size(); i++)
		{
		R2[i] = (double)R1[i];
		}

	CP.setRadii(R2);

	CP.computeLayout();
	cout << "done in " << mtools::Chronometer() << "ms\n";


	radii = CP.getRadii();
	circles = CP.getLayout();


	fVec2 pos0 = circles.back();
	double rad0 = radii.back();
	mtools::Mobius<double> M(0.0, 1.0, 1.0, 0.0);

	for (int i = 0; i < circles.size(); i++)
		{
		circles[i] -= pos0;
		circles[i] /= rad0;
		radii[i] /= rad0;

		auto rr = M.imageCircle((mtools::complex<double>)(circles[i]), radii[i]);
		circles[i] = rr.first;
		radii[i] = rr.second;
		}


	drawCirclePacking(fBox2(-2, 2, -2, 2), radii, circles, gr);

	}



void testBall()
	{
	int sizeTrig = 2000000;//10e6 200K error

	DyckWord D(sizeTrig, 3);
	D.shuffle(gen);

	CombinatorialMap CM(D);
	int a, b, c;
	std::tie(a, b, c) = CM.btreeToTriangulation();

	auto gr = CM.toGraph();

	cout << "TRIANGULATION CREATED\n";

	int nbv	= CM.nbVertices();
	auto V = CM.getVerticeVector();

	int v1 = V[a];	// root face
	int v2 = V[b];	//
	int v3 = V[c];	//
	std::vector<int> oldbound(nbv);
	oldbound[v1] = 1;
	oldbound[v2] = 1;
	oldbound[v3] = 1;


	bool connected = false;
	int maxd = -1;
	auto dist = computeGraphDistances(gr, v1, maxd,connected);
	int cutd = maxd/2;

	std::vector<int> bound;
	auto marked = markToRemove(gr, dist, cutd, maxd, bound);

	dist = mtools::permute(dist, getSortPermutation(marked));
	bound = mtools::permute(bound, getSortPermutation(marked));
	oldbound = mtools::permute(oldbound, getSortPermutation(marked));
	gr = mtools::permuteGraph(gr, getSortPermutation(marked));
	marked = mtools::permute(marked, getSortPermutation(marked));

	int L = 0;
	while (marked[L] == 0) { L++; }
	gr.resize(L);
	bound.resize(L);

	gr = resizeGraph(gr, L);
	cout << mtools::graphInfo(gr) << "\n\n";
	// ok, we have the graph with boundary bound
	
	gr = triangulateGraph(gr);
	cout << mtools::graphInfo(gr) << "\n\n";
	// ok the graph is triangulated
	
	oldbound.resize(gr.size()); // the orignal root face
	int f = 0;
	for (int i = 0;i < gr.size();i++)
		{
		if (oldbound[i] > 0) f++;
		}
	cout << f << "\n";

	fBox2 R;
	std::vector<double> radii;
	std::vector<fVec2> circles;
	

	CirclePackingLabelGPU<double> CPTEST(true);
	CPTEST.setTriangulation(gr, oldbound);
	CPTEST.setRadii();
	
	cout << "packing GPU...\n";
	auto cc = Chrono();
	cout << "ITER = " << CPTEST.computeRadii(1.0e-10,0.03,-1,1000) << "\n";
	cout << "done in " << cc << "\n";
	cout << CPTEST.errorL1() << "\n";
	cout << CPTEST.errorL2() << "\n\n";

	/*
	CirclePackingLabel<double> CP2;
	CP2.setTriangulation(gr, oldbound);
	CP2.setRadii();
	
	cout << "packing classique CPU...\n";
	mtools::Chronometer();
	cout << "ITER = " << CP2.computeRadii(1.0e-7) << "\n";
	cout << "done in " << mtools::Chronometer() << "ms\n";
	cout << CP2.errorL1() << "\n";
	cout << CP2.errorL2() << "\n\n";
	*/


	/*
	CirclePackingLabel<double> CP3;
	CP3.setTriangulation(gr, oldbound);
	CP3.setRadii();

	cout << "packing classique CPU slow...\n";
	mtools::Chronometer();
	cout << "ITER = " << CP3.computeRadiiSlow(1.0e-7) << "\n";
	cout << "done in " << mtools::Chronometer() << "ms\n";
	cout << CP3.errorL1() << "\n";
	cout << CP3.errorL2() << "\n";
	*/



	CirclePacking CP;
	CP.setTriangulation(gr, oldbound);

	auto R1 = CPTEST.getRadii();
	std::vector<double> R2(gr.size());
	for (int i = 0;i < gr.size(); i++)
		{
		R2[i] = (double)R1[i];
		}

	CP.setRadii(R2);

	CP.computeLayout();
	cout << "done in " << mtools::Chronometer() << "ms\n";


	radii = CP.getRadii();
	circles = CP.getLayout();


	{
	mtools::OArchive ar(std::string("trig") + mtools::toString(gr.size()) + ".txt");
	ar & gr; ar.newline();
	ar & oldbound; ar.newline();
	ar & radii; ar.newline();
	ar & circles; ar.newline();
	}


	fVec2 pos0 = circles.back();
	double rad0 = radii.back();
	mtools::Mobius<double> M(0.0,1.0,1.0,0.0);

	for (int i = 0; i < circles.size(); i++)
		{
		circles[i] -= pos0;
		circles[i] /= rad0;
		radii[i] /= rad0;

		auto rr = M.imageCircle((mtools::complex<double>)(circles[i]), radii[i]);
		circles[i] = rr.first;
		radii[i] = rr.second;

		}


	//cout.getKey();


	drawCirclePacking(fBox2(-2,2,-2,2), radii, circles, gr);




	}





	void testR(CombinatorialMap & cm)
		{
		cout << "MAP\n";
		cout << "Is tree : " << cm.isTree() << "\n";
		cout << "nb edges: " << cm.nbEdges() << "\n";
		cout << "nb faces: " << cm.nbFaces() << "\n";
		cout << "nb verti: " << cm.nbVertices() << "\n\n";
		cout << "euler   : " << cm.nbVertices() - cm.nbEdges() + cm.nbFaces() << "\n";
		cout << "DUAL\n";
		auto cm2 = cm.getDual();
		cout << "Is tree : " << cm2.isTree() << "\n";
		cout << "nb edges: " << cm2.nbEdges() << "\n";
		cout << "nb faces: " << cm2.nbFaces() << "\n";
		cout << "nb verti: " << cm2.nbVertices() << "\n\n";
		cout << "euler   : " << cm.nbVertices() - cm.nbEdges() + cm.nbFaces() << "\n";
		auto cm3 = cm2.getDual();
		cout << "DUAL(DUAL) = MAP :" << (cm3 == cm) << "\n\n\n";
		}



int main(int argc, char *argv[])
    {
	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);

	//loadTest("trig1421883.txt");
	//return 0;
	testBall(); 
	return 0;



    }

/* end of file main.cpp */






