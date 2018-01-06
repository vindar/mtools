/****************************************************************************************************
* Classes for simulating a Once Edge Reinforced Random walks on Z^2        Version 1.0, Vindar 2014 *
*                                                                                                   *
*                                                                                                   *
* 3 classes:                                                                                        *
*                                                                                                   *
*   - simpleOERRW     : for making long simulation of OERRW, not much info but do not use much RAM  *
*                                                                                                   *
*   - extendedOERRW   : for making detailled simulation of OERRW, keep more info but uses more RAM  *
*                                                                                                   *
*   - simulationOERRW : master classe using the two class above for creating and displaying         *
*                       different kind of simulation for Once ERRW                                  *
*                                                                                                   *
* implementation (of the classes simulationOERRW) of in OnceERRW.cpp.                               *
****************************************************************************************************/


/********************************************************************************
* Example of simple programm for making simulation and printing them from 
* the command line.
*
#include "mylib_all.h"

int main (int argc, char *argv[])
{
	out.disablelogfile();
	simulationOERRW sim;
	if (argc == 2) 	{
					std::string filename = argv[1]; std::string path = ExtractPath(filename,true);
					if (ExtractExtension(filename) == "onceERRW") {sim.printSimulation(path,ExtractNameWithoutExtension(filename)); return 0;}
					else {out << "parametre de commande [" << filename << "] invalide !\n\n"; input.getkey(); return 0;}
					}
	while(1) {sim.simulationMenu();}
}
*********************************************************************************/

#ifndef _ONCERRRW_H_
#define _ONCEERRW_H_

#include "../mathgraph/GrowingLatticeZ2.h"
#include "../containers/extab.h"
#include "../graphics/LatticePlotter.h"
#include "../graphics/plotter.h"
#include "../graphics/EdgeSiteImage.h"
#include "../randomgen/mt2004_64.h"
#include "../randomgen/SRWonZandZ2.h"

#include <cstdio>
#include <windows.h>


using namespace mylib::mathgraph;
using namespace mylib::containers;
using namespace mylib::graphics;
using namespace mylib::random;

namespace mylib
{
namespace models
{






/****************************************************************************************************************************
 ***************************************************************************************************************************/





/**************************************************************************************
 * simpleOERRW : Class for long simulation of a once ERRW on Z^2 
 *
 * Here, we only save the edges traversed in a GrowingLatticeZ2. This allows to keep 
 * memory space quite low.
 *
 * - This classe should be used to create very long simulation. But does not give much
 *   info about the process. It only
 *   - save the set of traverse edge and visited site (the lattice)
 *   - creates an extab with the times the walk visit a new site
 *   - creates an extab with the number of return to 0 when the range extends
 *
 * RQ: - uses the GrowingLatticeZ2  and search for full region in order to accelerate the
 *     simulation by performing many step of SRW at one time. 
 *
 *     - the object can be saved and loaded from disk (4 files per object)
 *       this permit to interrupt and restart again a simulation
 *
 * As always, d is the reinforcement parameter ie
 * - all edges are initially with value 1 and then delta after being traversed
 *
 **************************************************************************************/
class simpleOERRW
{
private:

	simpleOERRW(const simpleOERRW &);				//	no copy allowed
	simpleOERRW & operator=(const simpleOERRW &);	//

public:

	/* ctor 
	 * can specify the size of the tab for storing the time when the walk vist a new site (rangeTabSize)
	 * and the time the walk returns to the origin (returnTabSize)
	
	*/
	simpleOERRW(double d,size_t rangeTabSize = 1000000,size_t returnTabSize = 1000000)
		{
		L1 = new GrowingLatticeZ2<char,RR>(0,maskup|maskright);	// site is full if up edge and right edge are crossed
		timetab = new ExTab(rangeTabSize,"new site times");		// tab for the range
		returntab = new ExTab(returnTabSize,"return times");	// tab for the return times to the origin
		reset(d);
		}

	/* dtor */
	~simpleOERRW()
		{
		delete returntab;
		delete timetab;
		delete L1;
		}


	/**************************************************************
	* reset the walk 
	* d = reinforcement parameter. 
	***************************************************************/
	void reset(double d)
		{
		delta = d;
		nbreturn = 0;
		N = 0;
		step = 0;
		x = 0; y = 0;
		L1->reset();
		timetab->Reset();
		returntab->Reset();
		}


	/**************************************************************
	* return the number of step performed by the walk 
	***************************************************************/
	int64 steps_done() const {return step;}


	/**************************************************************
	* return the number of distinct sites visited by the walk
	***************************************************************/
	int64 nb_visited() const {return N;}


	/**************************************************************
	* return the number of returns to the origin
	***************************************************************/
	int64 nb_return() const {return nbreturn;}


	/**************************************************************
	* return the range of the walk (useful for plotting the graph)
	***************************************************************/
	void range(fRect & r) const {L1->range(r); return;}


	/**************************************************************
	* return the actual coordinates of the walk 
	***************************************************************/
	void position(int64 & X,int64 & Y) const {X = x; Y = y; return;}


	/**************************************************************
	* return the reinforcment parameter delta
	***************************************************************/
	double delta_param() const {return delta;}


	/**************************************************************
	* print some info about the walk
	***************************************************************/
	std::string info() const
		{
		std::string s = "Edge Reinforced Random Walk ERRW\n";
		s += "  -> reinforcement parameter delta   = " + tostring(delta) + "\n";
		s += "  -> number of steps done            = " + tostring(step) + "\n";
		s += "  -> number of visited sites         = " + tostring(N) + "\n";
		s += "  -> number of return to origin      = " + tostring(nbreturn) + "\n";
		s += "  -> current position of the walk  X = " + tostring(x) + "   Y = " + tostring(y) + "\n\n";
		return s;
		}


	/**************************************************************
	* Draw the walk on a LatticePlotter
	* - The drawing is monochrome.
	* - when site image are on, the edge set are displayed
	***************************************************************/
	void plotWalk() const
		{
		fRect r;
		L1->range(r);
		LatticePlotter<const simpleOERRW,true> Plotter(*this); 
		Plotter.setRange(r,true);
		Plotter.startPlot();
		}


	/**************************************************************
	* draw the graph of the times of increase of the range
	***************************************************************/
	void PlotRangeIncrease() const
		{
        Plotter FP;
        FP.insert(timetab->PlotMin()); FP.insert(timetab->PlotMax()); FP.insert(timetab->PlotMed());
        FP.setrange();
        FP.plot();
        FP.removeall();
		}


	/**************************************************************
	* return the extab containing the times of increase of the range
	***************************************************************/
	const ExTab * getRangeTab() const {return timetab;}


	/**************************************************************
	* draw the graph of the number of return to 0 when range extends
	***************************************************************/
	void PlotReturn() const
		{
        Plotter FP;
        FP.insert(returntab->PlotMin()); FP.insert(returntab->PlotMax()); FP.insert(returntab->PlotMed());
        FP.setrange();
        FP.plot();
        FP.removeall();
		}


	/**************************************************************
	* return the extab nb of return to (0,0) when range extends
	***************************************************************/
	const ExTab * getReturnTab() const {return returntab;}



	/**************************************************************
	* Save the object into 4 files:
	*
	* - filename.L1.Z2					contain the lattice 
	* - filename.trace.extab			contain the extab with the time the range extends
	* - filename.return.extab			contain the extab with nb of return to (0,0) each time the range extends
	* - filename.onceERRW				contain the info about the walk
	*
	* Do not add the extension to the filename
	* ex : "sim-d5" gives "sim-d5.L1.Z2", "sim-d5.trace.extab" ...
	***************************************************************/
	void save(std::string filename) const
		{
		remove((filename + ".old.L1.Z2").c_str());
		remove((filename + ".old.trace.extab").c_str());
		remove((filename + ".old.return.extab").c_str());
		remove((filename + ".old.onceERRW").c_str());
		rename((filename + ".L1.Z2").c_str(),(filename + ".old.L1.Z2").c_str());
		rename((filename + ".trace.extab").c_str(),(filename + ".old.trace.extab").c_str());
		rename((filename + ".return.extab").c_str(),(filename + ".old.return.extab").c_str());
		rename((filename + ".onceERRW").c_str(),(filename + ".old.onceERRW").c_str());
		L1->save(filename + ".L1.Z2");		// save lattice L1
		timetab->Save(filename + ".trace.extab");	// save the range tab
		returntab->Save(filename + ".return.extab");	// save the return tab
		FILE * hf = fopen((filename + ".onceERRW").c_str(),"wb");
		fwrite(&delta,sizeof(double),1,hf);			// reinforcement parameter delta
		fwrite(&N,sizeof(int64),1,hf);				// size of the range
		fwrite(&nbreturn,sizeof(int64),1,hf);		// number of return to the origin
		fwrite(&step,sizeof(int64),1,hf);			// number of steps
		fwrite(&x,sizeof(int64),1,hf);				// position x
		fwrite(&y,sizeof(int64),1,hf);				// position y
		fclose(hf);
		}


	/**************************************************************
	* Load the object: all 4 files are neeeded !
	*
	* filename : the name of the simulation (files name without extension)
	*
	* Return false it loading fails (in this case, the walk is reset
	***************************************************************/
	bool load(std::string filename)
		{
		reset(delta);
		// load the info
		FILE * hf = fopen((filename + ".onceERRW").c_str(),"rb");
		if (hf==NULL) {return false;}
		fread(&delta,sizeof(double),1,hf);
		fread(&N,sizeof(int64),1,hf);
		fread(&nbreturn,sizeof(int64),1,hf);
		fread(&step,sizeof(int64),1,hf);
		fread(&x,sizeof(int64),1,hf);
		fread(&y,sizeof(int64),1,hf);
		fclose(hf);
		if (L1->load(filename + ".L1.Z2") == false) {reset(delta); return false;}
		delete timetab;   try {timetab  = new ExTab(filename + ".trace.extab");}  catch(...) {timetab   = NULL;} if (timetab==NULL)   {timetab   = new ExTab(1000000,"new site times");} // load the range tab
		delete returntab; try {returntab= new ExTab(filename + ".return.extab");} catch(...) {returntab = NULL;} if (returntab==NULL) {returntab = new ExTab(1000000,"return times");} // load the return tab
		return true;
		}


	/**************************************************************
	*
	* Perform the walk until the range increases by nbParticules
	*
	***************************************************************/
	void makeWalk(int64 nbParticules)
		{
		nbParticules += N;
		int64 lastcheck = step;
		while(N < nbParticules)
			{
			char v = L1->get(x,y);		// current site
			char vd = L1->get(x,y-1);	// down site
			char vg = L1->get(x-1,y);	// left site
			if ((v == (maskup | maskright))&&(vg & maskright)&&(vd & maskup)) // the four edge around the site are set, do simple rw
				{
				if ((step - lastcheck > 50)&&((abs(x)>2)||(abs(y)>2))) // we try to move fast inside the set of visited sites and awy from the origin
					{ 
					lastcheck = step; // reset the counter
					int64 xmin,xmax,ymin,ymax; 
					bool ok = L1->improvedEnclosingNotZero(x,y,xmin,xmax,ymin,ymax);
					if (ok) {
							xmax--; ymax--;	xmin++; ymin++; // correct the size of the rectangle
							if ((xmax - x > 1)&&(ymax - y > 1)&&(x - xmin > 1)&&(y -  ymin > 1)) {step += random::SRW_Z2_exitRectangle<MT2004_64>(x,y,xmin,xmax,ymin,ymax,gen);	}
							}
					}
				else {random::SRW_Z2_make1step(x,y,gen.rand_double0()); step++;} // make just 1 step
				}
			else 
				{ // not all egdes are set, do ERRW
				double up    = ((v & maskup) ?  delta : 1.0);
				double right = ((v & maskright) ?  delta : 1.0);
				double down  = ((vd & maskup) ?  delta : 1.0);
				double left  = ((vg & maskright) ?  delta : 1.0);
				double a = gen.rand_double0()*(up+right+down+left);
				if (a < up)					{if ((v & maskup) == 0)      {if (is_empty(x,y+1)) {N++; timetab->Add((double)step); returntab->Add((double)nbreturn);} L1->set(v | maskup,x,y);} y++;} else {
				if (a < up + right)			{if ((v & maskright) == 0)   {if (is_empty(x+1,y)) {N++; timetab->Add((double)step); returntab->Add((double)nbreturn);} L1->set(v | maskright,x,y);} x++;} else {
				if (a < up + right + down)	{if ((vd & maskup) == 0)     {if (is_empty(x,y-1)) {N++; timetab->Add((double)step); returntab->Add((double)nbreturn);} L1->set(vd | maskup,x,y-1);} y--;} else 
											{if ((vg & maskright) == 0)  {if (is_empty(x-1,y)) {N++; timetab->Add((double)step); returntab->Add((double)nbreturn);} L1->set(vg | maskright,x-1,y);} x--;}}}				
				step++; 
				}
			if ((x==0)&&(y==0)) {nbreturn++;}
			}
		}


	/* for drawing site images */
	inline const CImg<unsigned char > & getImage(int64 i,int64 j,int lx,int ly) const
		{
		EdgeSiteImage ES; 
		if (is_empty(i,j))  {ES.bkColor(RGBc::c_White); ES.siteColor(RGBc::c_White); return ES.makeImage(im,lx,ly);}
		ES.site(true,RGBc::c_Black);
		char v= L1->get(i,j);
		if (v & maskup)    {ES.up(ES.EDGE,RGBc::c_Black);}						 // whether the up edge is set
		if (v & maskright) {ES.right(ES.EDGE,RGBc::c_Black);}					 // whether the right edge is set
		v = L1->get(i-1,j); if (v & maskright) {ES.left(ES.EDGE,RGBc::c_Black);} // whether edge left is set
		v = L1->get(i,j-1); if (v & maskup)    {ES.down(ES.EDGE,RGBc::c_Black);} // whether edge down is set
		return ES.makeImage(im,lx,ly);
		}


	/* for drawing the color of a site */
	inline RGBc getColor(int64 i, int64 j) const {if (is_empty(i,j)) {return RGBc::c_White;} return RGBc::c_Black;}



	/*****************************************
	* Constant parameter.
	* Changing them make the previous saved
	* files incompatible !
	*****************************************/
	static const int RR = 5;				// size of the subsquare for the edge lattice
	static const char maskup = 16;			// mask for the up edge
	static const char maskright = 32;		// mask for the left edge

	private:

	/* Return true if there is no edge around site (i,j) */
	inline bool is_empty(int64 i,int64 j) const
		{
		if (((L1->get(i,j)) == 0)&&(((L1->get(i,j-1)) & maskup) == 0)&&(((L1->get(i-1,j)) & maskright) == 0)) return true;
		return false;
		}


	double delta;						// reinforcement parameter delta
	int64 N;							// size of the current range
	int64 step;							// number of steps performed
	int64 x,y;							// current position of the walk
	int64 nbreturn;						// number of return to the origin
	GrowingLatticeZ2<char,RR> * L1;		// Lattice for checking if a site is occupied (0 = none 1 = up edge visited , 2 = right edge visited , 3 = both edge visited)
	ExTab * timetab;					// array for the times where the trace increases
	ExTab * returntab;					// array for nb of return to origin when the range extends

	mutable CImg<unsigned char>  im;	// for drawing an image
	mutable MT2004_64  gen;				// the random number generator
};










/****************************************************************************************************************************
 ***************************************************************************************************************************/











/**************************************************************************************
 * extendedOERRW : Class for detailled simulation of a once ERRW on Z^2 
 *
 * Here, we keep on each edge the number of its visit (le combien ieme visité etait il)
 * We also recall for each edge the direction of first crossing and also, for each site
 * by which edge it was first visited.
 *
 * - This classe should be used to create detailled simulation. But it take much more 
 *   memory space than a simpleOERRW object
 *
 * RQ: - uses the GrowingLatticeZ2  and search for full region in order to accelerate the
 *     simulation by performing many step of SRW at one time. 
 *
 *     - the object can be saved and loaded from disk (4 files per object)
 *       this permit to interrupt and restart again a simulation
 *     - an extendedOERRW is also a simpleOERRW if we remove the .L2.Z2 and .L3.Z2 files !
 *
 * As always, d is the reinforcement parameter ie
 * - all edges are initially with value 1 and then delta after being traversed
 *
 **************************************************************************************/
class extendedOERRW
{

private:

	extendedOERRW(const extendedOERRW &);				//	no copy allowed
	extendedOERRW & operator=(const extendedOERRW &);	//

public:

	/* ctor */
	extendedOERRW(double d,size_t rangeTabSize = 1000000,size_t returnTabSize = 1000000)
		{
		L1 = new GrowingLatticeZ2<char,RR>(0,maskup|maskright);	// site is full if up edge and right edge are crossed
		L2 = new GrowingLatticeZ2<int64,BB>(0,-1);				// contain the first passage time at a site
		L3 = new GrowingLatticeZ2<char,BB>(0,-1);				// contain orientation and order of traversed edges
		timetab = new ExTab(rangeTabSize,"Range times");		// tab for the range
		returntab = new ExTab(returnTabSize,"return times");	// tab for the return time to the origin
		reset(d);
		}

	/* dtor */
	~extendedOERRW()
		{
		delete returntab;
		delete timetab;
		delete L1;
		delete L2;
		delete L3;
		}


	/**************************************************************
	* reset the walk 
	* d= reinforcment parameter
	***************************************************************/
	void reset(double d)
		{
		delta = d;
		nbreturn = 0;
		N = 0;
		step = 0;
		x = 0; y = 0;
		L1->reset();
		L2->reset();
		L3->reset();
		timetab->Reset();
		returntab->Reset();
		}


	/**************************************************************
	* return the range of the walk (useful for plotting)
	***************************************************************/
	void range(fRect & r) const {L1->range(r); return;}


	/**************************************************************
	* return the number of step performed by the walk 
	***************************************************************/
	int64 steps_done() const {return step;}


	/**************************************************************
	* return the number of visited sites 
	***************************************************************/
	int64 nb_visited() const {return N;}


	/**************************************************************
	* return the number of return to the origin
	***************************************************************/
	int64 nb_return() const {return nbreturn;}


	/**************************************************************
	* return the coordinate of the walk 
	***************************************************************/
	void position(int64 & X,int64 & Y) const {X = x; Y = y; return;}


	/**************************************************************
	* return the reinforcement parameter 
	***************************************************************/
	double delta_param() const {return delta;}


	/**************************************************************
	* print some info about the walk in an std::string
	***************************************************************/
	std::string info() const
		{
		std::string s = "Edge Reinforced Random Walk ERRW\n";
		s += "  -> reinforcment parameter delta  = " + tostring(delta) + "\n";
		s += "  -> number of steps done          = " + tostring(step) + "\n";
		s += "  -> number of visited sites       = " + tostring(N) + "\n";
		s += "  -> number of return to origin    = " + tostring(nbreturn) + "\n";
		s += "  -> current position of the walk  X = " + tostring(x) + "   Y = " + tostring(y) + "\n\n";
		return s;
		}


	/**************************************************************
	/* Plot the walk with a LatticePlotter object
	***************************************************************/
	void plotWalk() const
		{
		fRect r;
		L1->range(r);
		LatticePlotter<const extendedOERRW,true> Plotter(*this); 
		Plotter.setRange(r,true);
		Plotter.startPlot();
		}


	/**************************************************************
	* draw the graph of the increase of the range 
	***************************************************************/
	void PlotRangeIncrease() const
		{
        Plotter FP;
        FP.insert(timetab->PlotMin()); FP.insert(timetab->PlotMax()); FP.insert(timetab->PlotMed());
        FP.setrange();
        FP.plot();
        FP.removeall();
		}


	/**************************************************************
	* return the extab containing the times of increase of the range
	***************************************************************/
	const ExTab * getRangeTab() const {return timetab;}


	/**************************************************************
	* draw the graph of the return times (wrt the range) 
	***************************************************************/
	void PlotReturn() const
		{
        Plotter FP;
        FP.insert(returntab->PlotMin()); FP.insert(returntab->PlotMax()); FP.insert(returntab->PlotMed());
        FP.setrange();
        FP.plot();
        FP.removeall();
		}


	/**************************************************************
	* return the extab containing the number of return to the origin
	* wrt the range
	***************************************************************/
	const ExTab * getReturnTab() const {return returntab;}


	/**************************************************************
	* save the object to disk in 6 files (the first 4 correspond to a simpleOERRW object)
	*
	* - filename.L1.Z2					contain the lattice 
	* - filename.trace.extab			contain the extab with the time the range extends
	* - filename.return.extab			contain the extab with the nb of return to (0,0) when range extends
	* - filename.onceERRW				contain the info about the walk
	* - filename.L2.Z2					contain hitting time of each site (very big)
	* - filename.L3.Z3					contain the entry edge and direction of crossing of each edge
	*
	* Do not add the extension to the filename
	* ex : "sim-d5" gives "sim-d5.L1.Z2", "sim-d5.trace.extab" ...
	* **************************************************************/
	void save(std::string filename) const
		{
		remove((filename + ".old.L1.Z2").c_str());
		remove((filename + ".old.L2.Z2").c_str());
		remove((filename + ".old.L3.Z2").c_str());
		remove((filename + ".old.trace.extab").c_str());
		remove((filename + ".old.return.extab").c_str());
		remove((filename + ".old.onceERRW").c_str());
		rename((filename + ".L1.Z2").c_str(),(filename + ".old.L1.Z2").c_str());
		rename((filename + ".L2.Z2").c_str(),(filename + ".old.L2.Z2").c_str());
		rename((filename + ".L3.Z2").c_str(),(filename + ".old.L3.Z2").c_str());
		rename((filename + ".trace.extab").c_str(),(filename + ".old.trace.extab").c_str());
		rename((filename + ".return.extab").c_str(),(filename + ".old.return.extab").c_str());
		rename((filename + ".onceERRW").c_str(),(filename + ".old.onceERRW").c_str());
		L1->save(filename + ".L1.Z2");		// save lattice L1
		L2->save(filename + ".L2.Z2");		// save Lattice L2
		L3->save(filename + ".L3.Z2");		// save Lattice L3
		timetab->Save(filename + ".trace.extab");	// save the range tab
		returntab->Save(filename + ".return.extab");	// save the return tab
		FILE * hf = fopen((filename + ".onceERRW").c_str(),"wb");
		fwrite(&delta,sizeof(double),1,hf);
		fwrite(&N,sizeof(int64),1,hf);
		fwrite(&nbreturn,sizeof(int64),1,hf);
		fwrite(&step,sizeof(int64),1,hf);
		fwrite(&x,sizeof(int64),1,hf);
		fwrite(&y,sizeof(int64),1,hf);
		fclose(hf);
		}


	/**************************************************************
	* Load the object from disk
	* All 6 files are needed !
	*
	* filename : the name of the simulation (files name without extension)
	*
	* Return true is ok
	***************************************************************/
	bool load(std::string filename)
		{
		reset(delta);
		// load the info
		FILE * hf = fopen((filename + ".onceERRW").c_str(),"rb");
		if (hf==NULL) {return false;}
		fread(&delta,sizeof(double),1,hf);
		fread(&N,sizeof(int64),1,hf);
		fread(&nbreturn,sizeof(int64),1,hf);
		fread(&step,sizeof(int64),1,hf);
		fread(&x,sizeof(int64),1,hf);
		fread(&y,sizeof(int64),1,hf);
		fclose(hf);
		if (L1->load(filename + ".L1.Z2") == false) {reset(delta); return false;} // load lattice L1
		if (L2->load(filename + ".L2.Z2") == false) {reset(delta); return false;} // load Lattice L2
		if (L3->load(filename + ".L3.Z2") == false) {reset(delta); return false;} // load Lattice L2
		delete timetab;   try {timetab  = new ExTab(filename + ".trace.extab");}  catch(...) {timetab   = NULL;} if (timetab==NULL)   {timetab   = new ExTab(1000000,"new site times");} // load the range tab
		delete returntab; try {returntab= new ExTab(filename + ".return.extab");} catch(...) {returntab = NULL;} if (returntab==NULL) {returntab = new ExTab(1000000,"return times");} // load the return tab
		return true;
		}



	/**************************************************************
	*
	* Perform the walk until the range increases by nbParticules
	*
	***************************************************************/
	void makeWalk(int64 nbParticules)
		{
		nbParticules += N;
		int64 lastcheck = step;
		while(N < nbParticules)
			{
			char v = L1->get(x,y);		// current site
			char vd = L1->get(x,y-1);	// down site
			char vg = L1->get(x-1,y);	// left site
			if ((v == (maskup | maskright))&&(vg & maskright)&&(vd & maskup)) // the four edge around the site are set, do simple rw
				{
				if ((step - lastcheck > 50)&&((abs(x)>2)||(abs(y)>2))) // we try to move fast inside the set of visited sites
					{ 
					lastcheck = step; // reset the counter
					int64 xmin,xmax,ymin,ymax; 
					bool ok = L1->improvedEnclosingNotZero(x,y,xmin,xmax,ymin,ymax);
					if (ok) {
							xmax--; ymax--;	xmin++; ymin++; // correct the size of the full rectangle
							if ((xmax - x > 1)&&(ymax - y > 1)&&(x - xmin > 1)&&(y -  ymin > 1)) {step += random::SRW_Z2_exitRectangle<MT2004_64>(x,y,xmin,xmax,ymin,ymax,gen);	}
							}
					}
				else {random::SRW_Z2_make1step(x,y,gen.rand_double0()); step++;} // make just 1 step
				}
			else 
				{ // not all egdes are set, do ERRW
				char dir = 0;
				double up    = ((v & maskup) ?  delta : 1.0);
				double right = ((v & maskright) ?  delta : 1.0);
				double down  = ((vd & maskup) ?  delta : 1.0);
				double left  = ((vg & maskright) ?  delta : 1.0);
				double a = gen.rand_double0()*(up+right+down+left);
				if (a < up)					{if ((v & maskup) == 0)      {if (is_empty(x,y+1)) {L3->set(e_down,x,y+1); N++; L2->set(N,x,y+1); timetab->Add((double)step);  returntab->Add((double)nbreturn);} L1->set(v | maskup,x,y); L3->set(L3->get(x,y) | maskup,x,y);       } y++;} else {
				if (a < up + right)			{if ((v & maskright) == 0)   {if (is_empty(x+1,y)) {L3->set(e_left,x+1,y); N++; L2->set(N,x+1,y); timetab->Add((double)step);  returntab->Add((double)nbreturn);} L1->set(v | maskright,x,y); L3->set(L3->get(x,y) | maskright,x,y); } x++;} else {
				if (a < up + right + down)	{if ((vd & maskup) == 0)     {if (is_empty(x,y-1)) {L3->set(e_up,x,y-1); N++; L2->set(N,x,y-1); timetab->Add((double)step);    returntab->Add((double)nbreturn);} L1->set(vd | maskup,x,y-1);                                        } y--;} else 
											{if ((vg & maskright) == 0)  {if (is_empty(x-1,y)) {L3->set(e_right,x-1,y); N++; L2->set(N,x-1,y); timetab->Add((double)step); returntab->Add((double)nbreturn);} L1->set(vg | maskright,x-1,y);                                     } x--;}}}
				step++; // next step	
				}
			if ((x==0)&&(y==0)) {nbreturn++;} // note if we are at the origin
			}
		}



	/* for drawing site images */
	inline const CImg<unsigned char > & getImage(int64 i,int64 j,int lx,int ly) const
		{
		EdgeSiteImage ES; 
		if (is_empty(i,j))  {ES.bkColor(RGBc::c_White); ES.siteColor(RGBc::c_White); return ES.makeImage(im,lx,ly);}
		int64 T = L2->get(i,j);
		ES.site(true,RGBc::jet_palette(T,0,N));
		ES.text(tostring(T));
		char v= L1->get(i,j); // whether edge up and right are set
		char w= L3->get(i,j); // if so, in which direction they were traversed
		if (v & maskup)    {if (w & maskup)    {ES.up(ES.EDGE,RGBc::c_Black);}    else {ES.up(ES.ARROW_INGOING,RGBc::c_Black);}}
		if (v & maskright) {if (w & maskright) {ES.right(ES.EDGE,RGBc::c_Black);} else {ES.right(ES.ARROW_INGOING,RGBc::c_Black);}}
		v = L1->get(i-1,j); // whether edge left is set
		w=  L3->get(i-1,j); // if so, in which direction
		if (v & maskright) {if (w & maskright) {ES.left(ES.ARROW_INGOING,RGBc::c_Black);} else {ES.left(ES.EDGE,RGBc::c_Black);}}
		v = L1->get(i,j-1); // whether edge down is set
		w=  L3->get(i,j-1); // if so, in which direction
		if (v & maskup) {if (w & maskup)    {ES.down(ES.ARROW_INGOING,RGBc::c_Black);} else {ES.down(ES.EDGE,RGBc::c_Black);}}
		char e = L3->get(i,j);
		if ((e & maskfirstdir) == e_up)   {ES.up(ES.ARROW_INGOING,RGBc::c_Red);}
		if ((e & maskfirstdir) == e_down) {ES.down(ES.ARROW_INGOING,RGBc::c_Red);}
		if ((e & maskfirstdir) == e_left) {ES.left(ES.ARROW_INGOING,RGBc::c_Red);}
		if ((e & maskfirstdir) == e_right){ES.right(ES.ARROW_INGOING,RGBc::c_Red);}	
		return ES.makeImage(im,lx,ly);
		}


	/* return the color of a site */
	inline RGBc getColor(int64 i, int64 j) const
		{
		if (is_empty(i,j)) {return RGBc::c_White;} 
		int64 v = L2->get(i,j);
		return RGBc::jet_palette(L2->get(i,j),0,N);
		}



	/*****************************************
	* Constant parameter.
	* Changing them make the previous saved
	* files incompatible !
	*****************************************/
	static const int RR = simpleOERRW::RR;
	static const int BB = 50;
	static const char maskup = simpleOERRW::maskup;			// mask for the up edge
	static const char maskright = simpleOERRW::maskright;	// mask for the left edge
	static const char maskfirstdir = 7;						// mask for the direction of first edge crossed
	static const char e_up = 1;								// the first edge crossed (when entering the site) was the up edge
	static const char e_down = 2;							// the first edge crossed (when entering the site)  was the down edge
	static const char e_left = 3;							// the first edge crossed (when entering the site)  was the left edge
	static const char e_right = 4;							// the first edge crossed (when entering the site)  was the right edge

private:

	/* Return true if there is no edge around site (i,j) */
	inline bool is_empty(int64 i,int64 j) const
		{
		if (((L1->get(i,j)) == 0)&&(((L1->get(i,j-1)) & maskup) == 0)&&(((L1->get(i-1,j)) & maskright) == 0)) return true;
		return false;
		}


	double delta;						// reinforcement parameter delta
	int64 N;							// size of the current range
	int64 step;							// number of steps performed
	int64 x,y;							// current position of the walk
	int64 nbreturn;						// number of return to the origin
	GrowingLatticeZ2<char,RR> * L1;		// Lattice for checking if a site is occupied (0 = none 1 = up edge visited , 2 = right edge visited , 3 = both edge visited)
	GrowingLatticeZ2<int64,BB> * L2;	// Lattice containing the time of first visit of a site
	GrowingLatticeZ2<char,BB> * L3;		// Lattice containing orientation and order of the traversed edges
	ExTab * timetab;					// array for the times where the trace increases
	ExTab * returntab;					// array number of returns to origin when range extends

	mutable CImg<unsigned char>  im;	// for drawing a site image
	mutable MT2004_64  gen;				// the random number generator

};








/****************************************************************************************************************************
 ***************************************************************************************************************************/








/**************************************************************************************
 * simulationOERRW : Master class for easily making different kind of simulation of 
 *                   OERRW on Z^2. 
 *
 * Uses simpleOERRW and extendedOERRW.
 *
 * - printSimulation() for printing simulation file from disk
 *
 * - simulationMenu()  for chosing the type of simulation and making a it
 *
 * - detailledSimulation()  make a detailled simulation
 * - longSimulation() 	    make a long simulation
 * - packSimulation()	    make a pack of simulation
 *
 * - fusionPack()	   create the average .extab for a pack of simulation
 *
 **************************************************************************************/
class simulationOERRW
{
public:

	simulationOERRW() {return;}
	~simulationOERRW() {clearVecLat();}

	/*************************************************************************************
	* Affichage d'une simulation d'une onceERRW a partir de fichiers sur le disque
	*
	* - filename : Nom des fichiers de simulation, sans les extensions
	*              par example "sim-d35" pour les fichiers "sim-d35.*.*"
	*              
	* - path :     le repertoire ou se trouvent les fichiers de la simulation, par defaut .\
	*
	* Essaie de determiner le type de simulation pour afficher le plus de details possible.
	*
	*   1 - regarde si il y a des fichiers .L2.Z2. Si oui, load une simulation detaillée
	*       (objet de type extendedOERRW)
	*
	*   2 - Sinon, regarde si si on a une simulation longue : Liste tout les fichiers
	*       [filename*.L1.Z2] et les superposent dans l'affichage pour avoir de la couleur.
	*
	*   3 - Sinon, load simplement l'objet comme un simulation simple (simpleOERRW)
	*       et realise un affichage simple
	*************************************************************************************/
	void printSimulation(std::string path,std::string filename);


	/*************************************************************************************
	* Lance un menu pour realiser plusieurs type de simulations
	*
	* Simulation detaillé : Simulation dans un objet extendedOERRW. On choisit jusqu'ou l'on
	*                       simul et on fait une sauvegarde reguliere (ecrasée au ur a a mesure)
	*                       -> Detaille mais utilise beaucoup de RAM.
	*                       -> la simulation peut etre interrompue et reprise ensuite.
	*
	* Simulation longue : Simulation dans un simpleOERRW. On sauve regulierement dans des fichiers
	*                     de nom differents pour pouvoir ensuite reconstituer et coloriser selon 
	*                     les temps de visite. 
	*                     -> peu de memoire utilise, peu durer tres longtemps ! 
	*					  -> la simulation peut etre interrompue et reprise ensuite.
	*
	* Pack de simulation : Simul en permanence des simpleOERRW (jusqu'a une distance raisonable, pas
	*                      de sauvegarde intermediaire des marches) puis numerote chaque marche et 
	*                      sauve les fichier .extab decrivant les temps d'increment de la trace et
	*                      les temps temps de retour (wrt la trace).
	*                      -> Ne sauve pas la marche elle meme ! Prend peu de place en memoire et
	*                         sur le disque donc peut tourner tres longtemps !!!
	*                      -> la simulation peut etre interrompue et reprise ensuite. (continue
	*                         la numeratation des .extab la ou on s'etait areté) 
	*
	* Fusion des extab   : associer a un pack de simulation, ceci permet de fusionner les .extabs
	*                      des temps de nouvelle visite ensemble (idem pour les return to 0) en creant
	*                      a chaque fois un extab qui correspond a la moyenne element par element.
	*
	* Afficher simulation : Appel a printSimulation qui essaie d'afficher de la meilleure maniere
	*                       chaque tpye de simulation 
	***************************************************************************************/
	void simulationMenu();



	/**********************************************************************************************
	* Make a long simulation
	*
	* delta		= reinforcement parameter (edge non crossed = 1 and delta afterward)
	* step		= number of new visited site between each save
	* end       = we stop when when exceed end visited site in total (set to -1 for never stopping)
	*
	*   Create the 4 file of a simpleOERRW containing the state of the simulation and, at each new 
	*   save, copy the L1.Z2 file with the number of step done. Thus, after a while, there is the
	*   4 files of the current simpleOERRW object plus a bunch of files .L1.Z2 decribing the range
	*   of the walk at each save done that can be used to reconstruct the wxalk and colorize.
	*   -> Everything is saved on the current directory
	*   -> If a long simulation on the current directory already exist with the same parameter delta. 
	*      then it is continued (in particular, nothing is done if the simulation already exceed end)
	*   -> Simulation is saved in files of name "long_OERRW-d(delta)" 
	*
	* This simulation can be make extremely long since it only used one simpleOERRW in RAM. However, 
	* make sure you have enough space on disk to store all the .L1.Z2 (the lattices) or choose a
	* value of step large so that there is not to much saving done. 
	**********************************************************************************************/
	void longSimulation(double delta,int64 step,int64 end = -1);



	/**********************************************************************************************
	* Make a detailled simulation using a extendedOERRW class
	*
	* delta		= reinforcement parameter (edge non crossed = 1 and delta afterward)
	* step		= number of new visited site between each save
	* end       = we stop when when exceed end visited site in total (set to -1 for never stopping)
	* 
	*   -> Everything is saved on the current directory
	*   -> If an extended simulation on the current directory already exist with the same parameter 
	*      delta then it is continued (in particular, nothing is done if the simulation already exceed end)
	*   -> Simulation is saved in files of name "extended_OERRW-d(delta)" 
	*
	* This simulation is more detailled but used more RAM. 
	**********************************************************************************************/
	void detailledSimulation(double delta,int64 step,int64 end);



	/**********************************************************************************************
	* Make a pack of simple simulation saving only the .extab for each simulation. Then one can
	* use the fusinPack method for creating average extab file. 
	*
	* delta		= reinforcement parameter (edge non crossed = 1 and delta afterward)
	* end       = each simulation if done until the range of the walk is equal to end
	* 
	*   -> Everything is saved on the current directory
	*   -> If an pack simulation on the current directory already exist with the same parameter 
	*      delta then it is continued (the numerotation of the extab file continue without
	*      overwrtitng the previous ones
	*   -> Simulation is saved in files of name "pack_OERRW-d(delta)" 
	*
	* This can be very long : does not use much RAM (simpleOERRW object not going to far) and
	* also does not use much disk (only the .extab of each simulation are kept)
	**********************************************************************************************/
	void packSimulation(double delta,int64 end);



	/**********************************************************************************************
	* Create the average .extab for the range and return times from a pack of simulation previously
	* done. 
	*
	* delta		= reinforcement parameter for identifying the pack files
	* end		= lenght of each simulation for identifying the pack files
	* 
	*   -> Look into the current directory
	*   -> make the average for all the "pack_OERRW-d(delta)-R(end).no*.trace.extab" found
	*   -> do the same for all the "pack_OERRW-d(delta)-R(end).no*.return.extab" found
	*
	* the extab are saved as "pack_OERRW-d(delta)-R(end).average(N).trace.extab"
	*                    and "pack_OERRW-d(delta)-R(end).average(M).return.extab"
	* where N and M are the number of files found for each type. 
	**********************************************************************************************/
	void fusionPack(double delta,int64 end);




	/* for drawing site images */
	//inline const CImg<unsigned char > & getImage(int64 i,int64 j,int lx,int ly) const

	/* for drawing the color of a site for long simulation*/
	inline RGBc getColor(int64 x, int64 y) const 
		{
		int nb=0; size_t N = vecLat.size(); for(size_t i=0;i<N;i++) {if (isVisited(vecLat[i],x,y)) {nb++;}}
		if (nb == 0) {return RGBc::c_White;};
		return RGBc::jet_palette(N-nb,0,N);;
		}


private:


	// used for printing long simulations
	std::vector< GrowingLatticeZ2<char,simpleOERRW::RR>* > vecLat;

	// vlear the vecLat vector
	inline void clearVecLat() {for(size_t k=0; k<vecLat.size();k++) {delete vecLat[k];} vecLat.clear();}

	// used for printing a Simulation
	inline bool isVisited(const GrowingLatticeZ2<char,simpleOERRW::RR> * L,int64 i,int64 j) const
		{
		char v = L->get(i,j);		// current site
		char vd = L->get(i,j-1);	// down site
		char vg = L->get(i-1,j);	// left site		
		if ((v == 0)&&((vd & simpleOERRW::maskup) == 0)&&((vg & simpleOERRW::maskright) == 0)) return false;
		return true;
		}

};





}


}

#endif

/* end of file OnceERRW.h */




