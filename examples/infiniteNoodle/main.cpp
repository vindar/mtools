/***********************************************
 * Project : infiniteNoodle
 * date : Wed Nov 22 15:30:20 2017
 *
 * Simulation of the infinite noodle:
 * https://arxiv.org/pdf/1701.01083.pdf
 ***********************************************/

#include "stdafx.h"

// *** Library mtools ***
// Uncomment 'define MTOOLS_BASIC_CONSOLE' in stdafx.h to disable mtools's console
#include "mtools.hpp"  
using namespace mtools;


int L; // percolation set is [|0, L-1|]

std::vector<int> upArc;		// upArc[i] is the neighbour of i along the upper side arc, -1 if it should be <0 and L if it should be >= L. 
std::vector<int> downArc;	// downArc[i] is the neighbour of i along the lower side arc, -1 if it should be <0 and L if it should be >= L. 
std::vector<int> clusterId;	// clusterId[i] give the index if the cluster that ocntain site i.

int maxsize_complete;				// maximum size of any complete cluster. 
int maxsize_incomplete;				// maximum size of any incomplete cluster. 
int nb_complete;					// number of complete clusters
int nb_incomplete;					// number of incomplete clusters
std::vector<int> cluster_size;		// number of site in a given cluster
std::vector<int> cluster_typeleft;	// number of end that exist the cluster to the left
std::vector<int> cluster_typeright; // number of end that exist the cluster to the right 
									// (if both cluster_typeleft and cluster_typeright are 0, the cluster is finite and complete).

mtools::MT2004_64 gen;	// random generator


/* construct an arc vector */
void makeArcTab(std::vector<int> & tab)
	{
	tab.resize(L);
	std::vector<int> stack;
	for (int i = 0; i < L; i++)
		{
		if (mtools::Unif_1(gen))
			{
			tab[i] = L;
			stack.push_back(i);
			}
		else
			{
			if (stack.size() == 0) { tab[i] = -1; } else { const int j = stack.back(); tab[i] = j; tab[j] = i;  stack.pop_back(); }
			}
		}
	}


/* explore the a cluster going down first, return the number of distinct site visited
  (not including the start site and return the final site which may be -1 or L */
std::pair<int,int> follow_cluster_down(int startpos, int label)
	{
	int pos = startpos;
	int nbv = 0;
	if ((pos == -1) || (pos == L)) { return{ nbv, pos }; }
	clusterId[pos] = label;
	pos = downArc[pos];
	while (1)
		{
		if ((pos == -1) || (pos == L) || (pos == startpos)) { return { nbv, pos }; }
		clusterId[pos] = label;
		nbv++;
		pos = upArc[pos];
		if ((pos == -1) || (pos == L) || (pos == startpos)) { return { nbv, pos }; }
		clusterId[pos] = label;
		nbv++;
		pos = downArc[pos];
		}
	}


/* explore the a cluster going up first, return the number of distinct site visited
(not including the start site) and return the final site which may be startsite, -1 or L */
std::pair<int, int> follow_cluster_up(int startpos, int label)
{
	int pos = startpos;
	int nbv = 0;
	if ((pos == -1) || (pos == L)) { return{ nbv, pos }; }
	clusterId[pos] = label;
	pos = upArc[pos];
	while (1)
	{
		if ((pos == -1) || (pos == L) || (pos == startpos)) { return{ nbv, pos }; }
		clusterId[pos] = label;
		nbv++;
		pos = downArc[pos];
		if ((pos == -1) || (pos == L) || (pos == startpos)) { return{ nbv, pos }; }
		clusterId[pos] = label;
		nbv++;
		pos = upArc[pos];
	}
}


/* construct the percolation */
void makeNoodle()
	{
	cout << "-> Generating the percolation... ";
	mtools::Chronometer();
	makeArcTab(upArc);
	makeArcTab(downArc);
	cout << " done in " << Chronometer() << " ms\n";


	cout << "-> Computing the clusters... ";
	mtools::Chronometer();

	clusterId.clear();
	clusterId.resize(L, -1);

	nb_complete = 0;
	nb_incomplete = 0;
	maxsize_complete = 1;
	maxsize_incomplete = 1;
	cluster_size.clear();
	cluster_typeleft.clear();
	cluster_typeright.clear();

	int cl = 0; 
	// create the clusters
	for (int i = 0; i < L; i++)
		{
		if (clusterId[i] == -1)
			{ // i belong to a new cluster
			cluster_size.push_back(0);
			cluster_typeleft.push_back(0);
			cluster_typeright.push_back(0);
			auto resup = follow_cluster_up(i, cl);
			if (resup.second == i)
				{ // complete cluster. 
				nb_complete++;
				const int ns = resup.first + 1;
				cluster_size.back() = ns;
				if (ns > maxsize_complete) maxsize_complete = ns;
				}
			else
				{ // incomplete cluster
				nb_incomplete++; 
				if (resup.second == -1) cluster_typeleft.back()++; else cluster_typeright.back()++;
				auto resdown = follow_cluster_down(i, cl);
				const int ns = resup.first + resdown.first + 1;
				cluster_size.back() = ns;				
				if (ns > maxsize_incomplete) maxsize_incomplete = ns;
				if (resdown.second == -1) cluster_typeleft.back()++; else cluster_typeright.back()++;
				}
			cl++;
			}
		}

	cout << " done in " << Chronometer() << " ms\n";
	}



void makeImage(Image & im, int64 width, int64 height)
	{
	cout << "-> Generating the drawing... ";
	mtools::Chronometer();
	double l = (double)L;
	double lext = l + L / 5;

	im.resizeRaw({ width,height }, true);
	im.clear(RGBc::c_White);

	fBox2 R(-l/5, lext, -l*3/4, l*3/4);

	im.canvas_draw_axes(R);

	bool highquality = (width / L) >= 4;		// use antialiasing and tick lines if ratio is large enough. 
	bool blend = (highquality) ? true : false;
	bool aa = (highquality) ?  true : false;
	int pw = (highquality) ? 1 : 0;

	for (int i = 0; i < L; i++)
		{
		int id = clusterId[i]; // cluster associated with this site
		RGBc color = ((cluster_typeleft[id] != 0) || (cluster_typeright[id] != 0))
					? RGBc::c_Black.getOpacity(0.1 + (0.9f * cluster_size[id])/ maxsize_incomplete) // incomplete clusters in black with opacity according to their size
					: RGBc::jetPalette(cluster_size[id], 2, maxsize_complete); // complete clusters colored by jetPalette according to their size. 
		const int up = upArc[i];
		if ((up == -1) || (up == L))
			{
			im.canvas_draw_line(R, { (double)i, 0.0 }, { (double)i,  lext }, color, true, blend,aa,pw);
			}
		else if (up > i)
			{
			im.canvas_draw_cubic_bezier(R, { (double)i, 0.0 }, { (double)up, 0.0 }, { (double)i, (double)(up-i) }, { (double)up, (double)(up-i) }, color, true, blend, aa, pw);
			}
		const int down = downArc[i];
		if ((down == -1) || (down == L))
			{
			im.canvas_draw_line(R, { (double)i, 0.0 }, { (double)i,  -lext }, color, true, blend, aa, pw);
			}
		else if (down > i)
			{
			im.canvas_draw_cubic_bezier(R, { (double)i, 0.0 }, { (double)down, 0.0 }, { (double)i, (double)(i - down) }, { (double)down, (double)(i - down) }, color, true, blend, aa, pw);
			}
		}
	cout << " done in " << Chronometer() << " ms\n";
	}



int main(int argc, char *argv[]) 
    {
    MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc,argv,true); // parse the command line, interactive mode
	
	L = arg("L", 500).info("Number of site in the percolation");
	int imsize = arg("Image size", 5000).info("Image size");

	makeNoodle();

	Image im;
	makeImage(im, imsize, imsize);

	std::string filename = std::string("noodle-L") + toString(L) + ".png";
	cout << "-> Saving image as : [" << filename << "]... ";
	im.save(filename.c_str());
	cout << "done.\n";

	cout << "\n\n";
	cout << "Number of sites : " << L << "\n\n";
	cout << "Number of clusters : " << cluster_size.size() << "\n";
	cout << "      - complete   : " << nb_complete << "\n";
	cout << "      - incomplete : " << nb_incomplete << "\n\n";
	cout << "Largest complete cluster   : " << maxsize_complete << " sites.\n\n";
	cout << "Largest incomplete cluster : " << maxsize_incomplete << " sites.\n\n";

	Plotter2D plotter; 
	auto P1 = makePlot2DImage(im, 4, "infinite noodle");
	plotter[P1];
	plotter.axesObject()->enable(false);
	plotter.autorangeXY(); 
	plotter.plot();

    return 0;
	}
	
/* end of file main.cpp */


