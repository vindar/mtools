#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;





MT2004_64 gen; // RNG with 2M vertices.



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

	/*
	{
	mtools::OArchive ar(std::string("trig") + mtools::toString(gr.size()) + ".txt");
	ar & gr; ar.newline();
	ar & boundary; ar.newline();
	ar & circleVec; ar.newline();
	}
	*/



	Permutation perm(bound);
	gr = permuteGraph(gr, perm);
	radii = perm.getPermute(radii);
	int l = (int)(gr.size() - 3);
	cout << "error L2 = " << internals_circlepacking::errorL2euclidian(gr, radii,l) << "\n";
	cout << "error L1 = " << internals_circlepacking::errorL1euclidian(gr, radii,l) << "\n";

	gr = permuteGraph(gr, perm.getInverse());
	radii = perm.getAntiPermute(radii);

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
	
	}





std::vector<double> computeHyperbolicRadii(const std::vector<Circle<double> > & circle)
	{
	std::vector<double> srad(circle.size());
	for (size_t i = 0; i < circle.size(); i++)
		{
		double s = circle[i].euclidianToHyperbolic().radius;
		if (s < 0) { s = 0; }
		srad[i] = s;
		if (s >= 1)
			{
			cout << "UN = " << circle[i] << "\n";
			}
		}
	return srad;
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

	int w = (int)(std::find(dist.begin(), dist.end(), maxd) - dist.begin());// index of a vertex a maximal distance 
	int cutd = maxd/2;	// cut distance

	std::vector<int> vm(gr.size(),0);

	exploreGraph(gr, v1, [&](int vert, int d) -> bool { if (d <= cutd) { vm[vert] = -1; return true; } return false;});    // mark -1 all the vertices at dist <= cutd from v1
	int nbremove = 0;
	exploreGraph(gr, w, [&](int vert, int d) -> bool { if (vm[vert] == 0) { vm[vert] = 1; nbremove++; return true; } return false;});  // mark +1 all the vertices to remove. 

	Permutation perm(vm);							// permutation that put the vertices to remove at the end
	gr = permuteGraph(gr, perm);					// permute the graph
	v1 = perm.inv(v1);	v2 = perm.inv(v2); v3 = perm.inv(v3);	// and get the new indices for the root face. 

	gr = resizeGraph(gr, (int)gr.size() - nbremove);			// remove all the vertices with +1

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
	cout << "L2 error = " << CPTEST.errorL2() << "\n";
	cout << "\nL1 error = " << CPTEST.errorL1() << "\n\n";

	cout << "Laying out the circles...\n";
	auto circleVec = computeCirclePackLayout(gr, boundary, CPTEST.getRadii(), false, (int)gr.size() - 1);

	cout << "done in " << mtools::Chronometer() << "ms\n";

	auto pos0 = circleVec.back().center;
	double rad0 = circleVec.back().radius;	
	mtools::Mobius<double> M(0.0,1.0,1.0,0.0);

	for (int i = 0; i < circleVec.size(); i++)
		{ 
		circleVec[i] -= pos0; // center
		circleVec[i] /= rad0; // normalise such that outer boundary circle has size 1
		if (i != circleVec.size() - 1) { circleVec[i] = M*(circleVec[i]); } // invert */
		}


	int LX = 4000;
	int LY = 4000;
	fBox2 R(-2, 2, -2, 2);
	mtools::Img<unsigned char> im(LX, LY, 1, 4);
	im.clear(RGBc::c_White);

	drawCirclePacking_Circles(im, R, circleVec, gr, true, RGBc::c_Red, 0.2f, (int)gr.size() - 1, (int)gr.size());
	drawCirclePacking_Circles(im, R, circleVec, gr, true, RGBc::c_Red, 0.2f, 0, (int)gr.size()- 1);
	drawCirclePacking_Graph(im, R, circleVec, gr, RGBc::c_Black, 1.0f, 0, (int)gr.size() - 1);
	drawCirclePacking_Labels(im, R, circleVec, gr, 13, RGBc::c_Green, 1.0f, 0, (int)gr.size() - 1);


	boundary.clear();
	boundary.resize(gr.size(),0);
	for (int i = 0;i < gr.back().size(); i++) { boundary[gr.back()[i]] = 1; }

	gr = resizeGraph(gr, (int)gr.size() - 1);
	boundary.resize(gr.size());
	circleVec.resize(gr.size());

	mtools::saveCirclePacking(std::string("trig") + mtools::toString(gr.size()) + ".p", gr, boundary, circleVec, v1);
		
	Plotter2D Plotter;
	auto P2 = makePlot2DCImg(im, "circles");
	Plotter[P2];
	Plotter.autorangeXY();
	Plotter.plot();
	

	/* compute hyperbolic radii */
	auto srad = computeHyperbolicRadii(circleVec);

	cout << "Laying out the circles in hyperbolic space...\n";
	auto circleVec2 = computeCirclePackLayoutHyperbolic(gr, boundary, srad, true);

	{
	im.clear(RGBc::c_White);
	drawCirclePacking_Circles(im, R, circleVec2, gr, true, RGBc::c_Red, 0.2f, 0, (int)gr.size());
	drawCirclePacking_Graph(im, R, circleVec2, gr, RGBc::c_Black, 1.0f, 0, (int)gr.size());
	drawCirclePacking_Labels(im, R, circleVec2, gr, 13, RGBc::c_Green, 1.0f, 0, (int)gr.size());

	Plotter2D Plotter;
	auto P2 = makePlot2DCImg(im, "circles");
	Plotter[P2];
	Plotter.autorangeXY();
	Plotter.plot();
	}




	}



	void loadPack(std::string filename)
		{
		std::vector<std::vector<int> > gr;
		std::vector<int> bound;
		std::vector<mtools::Circle<double> > circles;
		int alpha, beta, gamma;
		mtools::loadCirclePacking(filename, gr, bound, circles, alpha, beta, gamma);

		cout << graphInfo(gr); 

		cout << "L2 error = " << circlePackErrorL2euclidian(gr, bound, circles) << "\n";
		cout << "\nL1 error = " << circlePackErrorL1euclidian(gr, bound, circles) << "\n\n";

		/*
		CirclePackingLabelGPU<double> CPTEST(true);	// prepare for packing
		CPTEST.setTriangulation(gr, bound);			//

		std::vector<double> rad(circles.size());
		for (size_t i = 0;i < circles.size(); i++) { rad[i] = circles[i].radius; }
		CPTEST.setRadii(rad);

		cout << rad.size() << "\n";
		cout << rad[0] << "\n";
		
		cout << "packing GPU...\n";
		auto cc = Chrono();
		cout << "ITERATION = " << CPTEST.computeRadii(1.0e-10, 0.03, -1, 1000) << "\n";
		cout << "done in " << cc << "\n";
		cout << "L2 error = " << CPTEST.errorL2() << "\n";
		cout << "\nL1 error = " << CPTEST.errorL1() << "\n\n";


		cout << "Laying out the circles...\n";
		circles = computeCirclePackLayout(gr, bound, CPTEST.getRadii(),false,alpha);
		*/

		fBox2 R(-1, 1, -1, 1);
		int LX = 12000;
		int LY = 12000;
		mtools::Img<unsigned char> im(LX, LY, 1, 4);

		im.clear(RGBc::c_White);
		im.fBox2_draw_circle(R, { 0,0 }, 1.0, RGBc::c_Blue, 0.1f, true);
		drawCirclePacking_Graph(im, R, circles, gr, RGBc::c_Black, 1.0f);


		Plotter2D Plotter;
		auto P2 = makePlot2DCImg(im, "circles");
		Plotter[P2];
		Plotter.autorangeXY();
		Plotter.plot();

		im.clear(RGBc::c_White);
		im.fBox2_draw_circle(R, { 0,0 }, 1.0, RGBc::c_Blue, 0.2f, true);
		drawCirclePacking_Circles(im, R, circles, gr, true, RGBc::c_Red, 0.2f);
		drawCirclePacking_Graph(im, R, circles, gr, RGBc::c_Black, 1.0f);
		//drawCirclePacking_Labels(im, R, circles, gr, 13, RGBc::c_Green, 1.0f);


		Plotter.redraw();
		Plotter.autorangeXY();
		Plotter.plot();



		}




	void testFBT(int n)
		{

		CombinatorialMap CM;
		CM.makeNgon(n);

		cout << graphInfo(CM.toGraph()) << "\n";

		CM.boltzmannPeelingAlgo(0, [&](int peeledge, int facesize)-> int {
			MTOOLS_INSURE(facesize >= 2);
			if (facesize < 3) { return -2; } // nothing to do 
			int m = facesize - 2;
			int k = (int)generalBoltzmanTriangulationLaw(m, 1.0/8.0, gen);
			if (k == -1) return -1; // new vertex discovered.
			MTOOLS_INSURE((k >= 1)&&(k <= m));
			//k = (m + 1 / 2);
			for (int i = 0; i < k; i++) { peeledge = CM.phi(peeledge); }
			return peeledge;
			});
			
		cout.getKey();
		cout << "\n\nOK, finished peeling\n\n";
		cout << graphInfo(CM.toGraph());
		//cout << CM.toGraph() << "\n\n";	// info about the graph.

		cout.getKey();
		cout << "\n\nMake type III\n\n";
		CM.reroot(CM.alpha(CM.root()));
		CM.collapsetoTypeIII();

		cout << graphInfo(CM.toGraph());
		//cout << CM.toGraph() << "\n\n";	// info about the graph.
		cout.getKey();

		cout << "done !\n";


		// OK, 
		std::vector<std::vector<int> > gr = CM.toGraph();

		cout << graphInfo(gr) << "\n";
		cout.getKey(); 


		std::vector<int> bbv(n);
		int maxd = 0;
		int maxv = -1;
		for (int i = 0;i < n;i++) { bbv[i] = i; }
		exploreGraph(gr, bbv, [&](int v, int d)->bool { maxd = d; maxv = v; return true;});

		int ee = -1;
		for (int i = 0;i < CM.nbDarts(); i++) { if (CM.vertice(i) == maxv) { ee = i; break; } }
		MTOOLS_INSURE(ee != -1);

		cout << "max distance from boundary = " << maxd << "\n";
		cout << "index of vertex that realizes it = " << maxv << "\n";
		cout << "index of vertex that realizes it = " << CM.vertice(ee) << "\n";

		int e1 = ee;
		int e2 = CM.phi(e1);
		int e3 = CM.phi(e2);
		MTOOLS_INSURE(CM.phi(e3) == e1);
		int v1 = CM.vertice(e1);
		int v2 = CM.vertice(e2);
		int v3 = CM.vertice(e3);

		gr = triangulateGraph(gr);
		std::vector<int> boundary(gr.size(), 0);				// set the root face
		boundary[v1] = 1; boundary[v2] = 1; boundary[v3] = 1;	// as the exterior face for packing


		cout << mtools::graphInfo(gr) << "\n\n";	// info about the graph.

		CirclePackingLabel<double> CPTEST(true);		// prepare for packing
		CPTEST.setTriangulation(gr, boundary);			//
		CPTEST.setRadii();								//

		cout << "ITERATION = " << CPTEST.computeRadii(9.9e-4, 0.03, -1, 1000) << "\n";
		cout << "Laying out the circles...\n";
		auto circleVec = computeCirclePackLayout(gr, boundary, CPTEST.getRadii(), false, (int)gr.size() - 1);

		cout << "done in " << mtools::Chronometer() << "ms\n";

		auto pos0 = circleVec.back().center;
		double rad0 = circleVec.back().radius;
		mtools::Mobius<double> M(0.0, 1.0, 1.0, 0.0);

		for (int i = 0; i < circleVec.size(); i++)
			{
			circleVec[i] -= pos0; // center
			circleVec[i] /= rad0; // normalise such that outer boundary circle has size 1
			if (i != circleVec.size() - 1) { circleVec[i] = M*(circleVec[i]); } // invert */
			}


		int LX = 2000;
		int LY = 2000;
		fBox2 R(-2, 2, -2, 2);
		mtools::Img<unsigned char> im(LX, LY, 1, 4);
		im.clear(RGBc::c_White);

		drawCirclePacking_Circles(im, R, circleVec, gr, true, RGBc::c_Red, 0.2f, (int)gr.size() - 1, (int)gr.size());
		drawCirclePacking_Circles(im, R, circleVec, gr, true, RGBc::c_Red, 0.2f, 0, (int)gr.size() - 1);
		drawCirclePacking_Graph(im, R, circleVec, gr, RGBc::c_Black, 1.0f, 0, (int)gr.size() - 1);
		//drawCirclePacking_Labels(im, R, circleVec, gr, 40, RGBc::c_Green, 1.0f, 0, (int)gr.size() - 1);

		Plotter2D Plotter;
		auto P2 = makePlot2DCImg(im, "circles");
		Plotter[P2];
		Plotter.autorangeXY();
		Plotter.plot();

		}



int main(int argc, char *argv[])
    {
	/*
	double q = generalBoltzmanTriangulation_CDF(0, 100, 1.0 / 6.0);

	cout << "GBT = " << 1.0 - generalBoltzmanTriangulation_CDF(100 - 37, 100, 1.0 / 6.0) << "\n";
	cout << "GBT = " << generalBoltzmanTriangulation_CDF(37, 100, 1.0 / 6.0) - q << "\n";



	cout << "GBT = " << generalBoltzmanTriangulation_CDF(99, 100, 1.0 / 6.0) << "\n";
	cout << "FBT = " << freeBoltzmanTriangulation_CDF(99, 100) << "\n\n";
*/

	cout.getKey();
	//	MTOOLS_SWAP_THREADS(argc, argv);
//	parseCommandLine(argc, argv);

	testFBT(200);
	//loadTest("trig1503676.txt");
	//loadTest("trig1503676.txt");
	
	//loadPack("trig1125072.p");

//	testBall(20000); 
	return 0;
	/*
	testBall(1100001);
	testBall(1200002);
	testBall(1400003);
	testBall(1600004);
	testBall(1800005);
	testBall(2000006);
	testBall(2200007);
	//	
	*/
	return 0;



    }

/* end of file main.cpp */






