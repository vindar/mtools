#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;





MT2004_64 gen(987653); // RNG with 2M vertices.



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







void loadTest(std::string filename)
	{

	fBox2 R;
	std::vector<double> radii;
	std::vector<fVec2> circles;
	std::vector<std::vector<int>> gr;
	std::vector<int> bound;

	mtools::IArchive ar(filename);
	ar & gr; 
	ar & bound; 
	ar & radii; 
	ar & circles; 

	Permutation perm = mtools::getSortPermutation(bound);
	auto invperm = mtools::invertPermutation(perm);
	gr = permuteGraph(gr, perm);
	radii = permute(radii, perm);
	int l = (int)(gr.size() - 3);
	cout << "error L2 = " << internals_circlepacking::errorL2(gr, radii,l) << "\n";
	cout << "error L1 = " << internals_circlepacking::errorL1(gr, radii,l) << "\n";

	gr = permuteGraph(gr, invperm);
	radii = permute(radii, invperm);

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

	double h = 1;
	//drawCirclePacking(fBox2(-h, h, -h, h), radii, circles, gr);
	}



void testBall(int N)
	{
						   
	DyckWord D(N, 3);	// random 3-dyck word.
	D.shuffle(gen);		// with N up. 

	CombinatorialMap CM(D);							// random triangulation with 
	int ea, eb, ec;									// N + 3 vertices and root
	std::tie(ea, eb, ec) = CM.btreeToTriangulation();	// face (ea,eb,ec)

	int nbv	= CM.nbVertices();	// number of vertices

	int v1 = CM.vertice(ea), v2 = CM.vertice(eb), v3 = CM.vertice(ec); // root vertices

	auto gr = CM.toGraph();	// convert the map to a graph. 

	bool connected = false;											// compute distance w.r.t
	int maxd = -1;													// the root vertex v1
	auto dist = computeGraphDistances(gr, v1, maxd, connected);		//

	int w = std::find(dist.begin(), dist.end(), maxd) - dist.begin();// index of a vertex a maximal distance 
	int cutd = maxd/2;	// cut distance

	std::vector<int> vm(gr.size(),0);

	exploreGraph(gr, v1, [&](int vert, int d) -> bool { if (d <= cutd) { vm[vert] = -1; return true; } return false;});    // mark -1 all the vertices at dist <= cutd from v1
	int nbremove = 0;
	exploreGraph(gr, w, [&](int vert, int d) -> bool { if (vm[vert] == 0) { vm[vert] = 1; nbremove++; return true; } return false;});  // mark +1 all the vertices to remove. 

	auto perm = getSortPermutation(vm);             // permutation that put the vertices to remove at the end
	gr = permuteGraph(gr, perm);					// permute the graph
	v1 = perm[v1];	v2 = perm[v2]; v3 = perm[v3];	// and get the new indices for the root face. 

	gr = resizeGraph(gr, (int)gr.size() - nbremove);			// remove al the vertices with +1

	// We have the graph with a simple boundary
	cout << mtools::graphInfo(gr) << "\n\n";	// info about the graph.

	gr = triangulateGraph(gr);					// add a vertex to triangulate the boundary. 

	std::vector<int> boundary(gr.size(), 0);				// set the root face
	boundary[v1] = 1; boundary[v2] = 1; boundary[v3] = 1;	// as the exterior face for packing


	cout << mtools::graphInfo(gr) << "\n\n";	// info about the graph.

	CirclePackingLabelGPU<double> CPTEST(true);		// prepare for packing
	CPTEST.setTriangulation(gr, boundary);			//
	CPTEST.setRadii();								//

	cout << "packing GPU...\n";	
	auto cc = Chrono();
	cout << "ITERATION = " << CPTEST.computeRadii(1.0e-9, 0.03, -1, 1000) << "\n";
	cout << "done in " << cc << "\n";
	cout << "L2 error = " << CPTEST.errorL1() << "\n";
	//cout << "\nL1 error = " << CPTEST.errorL1 << "\n\n";


	cout << "Laying out the circles...\n";
	auto res = computeCirclePackLayout(0, gr, boundary,CPTEST.getRadii());
	auto circleVec = res.first;
	auto R = res.second;

	cout << "done in " << mtools::Chronometer() << "ms\n";

	{
	mtools::OArchive ar(std::string("trig") + mtools::toString(gr.size()) + ".txt");
	ar & gr; ar.newline();
	ar & boundary; ar.newline();
	ar & circleVec; ar.newline();
	}



	auto pos0 = circleVec.back().center;
	double rad0 = circleVec.back().radius;	
	mtools::Mobius<double> M(0.0,1.0,1.0,0.0);

	for (int i = 0; i < circleVec.size(); i++)
		{
		circleVec[i] -= pos0; // center
		circleVec[i] /= rad0; // normalise such that outer boundary circle has size 1
		if (i != circleVec.size() - 1) { circleVec[i] = M*(circleVec[i]); } // invert
		}


	double ratio = (double)R.lx() / ((double)R.ly());
	int LX = 8000;
	int LY = (int)(LX / ratio);

	mtools::Img<unsigned char> imcircle(LX, LY, 1, 4);

	drawCirclePacking(imcircle, R, circleVec, gr, true, true, false, RGBc::c_Blue, 0.1f, (int)gr.size() - 1, (int)gr.size() - 1);
	drawCirclePacking(imcircle, R, circleVec, gr, true, true, false, RGBc::c_Red, 0.2f, 0, (int)gr.size() - 2);
	drawCirclePacking(imcircle, R, circleVec, gr, false, false, true, RGBc::c_Black, 1.0f, 0, (int)gr.size() - 2);

	Plotter2D Plotter;
	auto P2 = makePlot2DCImg(imcircle, "circles");
	Plotter[P2];
	Plotter.autorangeXY();
	Plotter.plot();

	}





int main(int argc, char *argv[])
    {
	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);


	//loadTest("trig1503676.txt");
	//loadTest("trig528.txt");
	//return 0;
	testBall(200000); 
	return 0;



    }

/* end of file main.cpp */






