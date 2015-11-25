/****************************************************************************************************
* Classes for simulating a Once Edge Reinforced Random walks on Z^2   
****************************************************************************************************/

#pragma once

#include "mtools.hpp"
#include <cstdio>
#include <windows.h>



/**   
 * Long simulation of a OERRW on Z2.
 * Save only the set of edges traversed, keeping memory space quite low.
 **/
class longOERRW
    {

    public:

        /* ctor */
        longOERRW(double d = 1.0)
            {
            reset(d);
            }


        /* reset the object, d is the reinf paramter (edge go from 1 to delta) */
        void reset(double d)
            {
            delta = d;
            R.clear();
            N = 0;
            step = 0;
            pos = { 0,0 };
            G.reset(0, 3, false);
            im.resize(1, 1, 1, 3);
            }


        /* return the number of step performed by the walk */
        int64 nbSteps() const { return step; }


        /* return the number of distinct sites visited by the walk */
        int64 nbVisited() const { return N; }


        /* return the range of the walk (useful for plotting the graph) */
        fRect rangeRect() const { return R; }


        /* return the actual coordinates of the walk */
        iVec2 currentPos() const { return pos; }


        /* return the reinforcement parameter */
        double reinfParam() const { return delta; }


        /* print some info about the walk */
        std::string toString() const
            {
            std::string s = "Once Edge Reinforced Random Walk ERRW\n";
            s += "  -> reinf. param. delta    = " + mtools::toString(reinfParam()) + "\n";
            s += "  -> nb of steps            = " + mtools::toString(nbSteps()) + "\n";
            s += "  -> nb of visited sites    = " + mtools::toString(nbVisited()) + "\n";
            s += "  -> current position       = (" + mtools::toString(pos.X()) + "," + mtools::toString(pos.Y()) + ")\n";
            s += "  -> range of the trace     = " + mtools::toString(rangeRect()) + "\n\n";
            return s;
            }


        /**************************************************************
        *
        * Perform the walk until the range increases by nb
        *
        ***************************************************************/
        void makeWalk(int64 nb,bool displayProgress = true)
            {
            ProgressBar<uint64> * PB = nullptr;
            if (displayProgress) { PB = new ProgressBar<uint64>(nb, "Simulating..."); }
            nb += N;
            auto startN = N;
            int64 lastcheck = step;
            while (N < nb)
                {
                char v = G(pos);		         // current site
                char vd = G(pos.X(),pos.Y()-1);	 // down site
                char vg = G(pos.X()-1, pos.Y()); // left site
                if ((v == (maskup | maskright)) && (vg & maskright) && (vd & maskup)) 
                    { // the four edge around the site are set, do simple rw
                    if (step - lastcheck > 50) // we try to move fast inside the set of visited sites and away from the origin
                        { // we make several step at once
                        lastcheck = step;
                        iRect fullR;
                        G.findFullBoxiRect(pos, fullR);
                        fullR.xmin++; fullR.xmax--; fullR.ymin++; fullR.ymax--; // remove the border
                        if ((fullR.xmax - pos.X() > 1) && (fullR.ymax - pos.Y() > 1) && (pos.X() - fullR.xmin > 1) && (pos.Y() - fullR.ymin > 1))
                            {
                            auto a = SRW_Z2_exitRectangle<MT2004_64>(pos, fullR, gen);
                            step += a;
                            if (a == 0)
                                {
                                cout << a << "\n";
                                cout << pos << "\n";
                                cout << fullR << "\n";
                                }
                            }
                        else
                            {
                            SRW_Z2_make1step(pos.X(), pos.Y(), gen.rand_double0());
                            step++;
                            }
                        }
                    else 
                        { // make only one step of the srw
                        SRW_Z2_make1step(pos.X(),pos.Y(), gen.rand_double0()); 
                        step++; 
                        } 
                    }
                else
                    { // not all egdes are set, do ERRW
                    if (PB != nullptr) { PB->update(N - startN); }
                    double up = ((v & maskup) ? delta : 1.0);
                    double right = ((v & maskright) ? delta : 1.0);
                    double down = ((vd & maskup) ? delta : 1.0);
                    double left = ((vg & maskright) ? delta : 1.0);
                    double a = gen.rand_double0()*(up + right + down + left);
                    if (a < up) 
                        { 
                        if ((v & maskup) == 0) { if (is_empty({ pos.X(), pos.Y() + 1 })) { N++; } G.set(pos, v | maskup); } 
                        pos.Y()++; 
                        }
                    else {
                        if (a < up + right) 
                            { 
                            if ((v & maskright) == 0) { if (is_empty({ pos.X() + 1, pos.Y() })) { N++; } G.set(pos, v | maskright); }
                            pos.X()++; 
                            }
                        else {
                            if (a < up + right + down) 
                                {
                                if ((vd & maskup) == 0) { if (is_empty({ pos.X(), pos.Y() - 1 })) { N++; } G.set({pos.X(),pos.Y()-1},vd | maskup); }
                                pos.Y()--; 
                                }
                            else
                                {
                                if ((vg & maskright) == 0) { if (is_empty({ pos.X() - 1, pos.Y() })) { N++; } G.set({ pos.X()-1,pos.Y()},vg | maskright); } 
                                pos.X()--;
                                }
                            }
                        }
                    step++;
                    }
                }
            delete PB;
            }


        /* Plot the trace of the walk */
        void plotWalk() const
            {
            Plotter2D Plotter;
            auto L = makePlot2DLattice(*this, std::string("OERRW-d") + mtools::toString(delta));
            L.setImageType(L.TYPEIMAGE);
            Plotter[L];
            Plotter.gridObject(true)->setUnitCells();
            Plotter.range().setRange(zoomOut(fRect(R)));
            Plotter.plot();
            }


        /* for drawing the color of a site */
        inline RGBc getColor(iVec2 p) const
            {
            if (is_empty(p)) return RGBc::c_TransparentWhite;
            return RGBc::c_Red;
            }

    private:

        /* Return true if there is no edge around site (i,j) */
        inline bool is_empty(iVec2 p) const
            {
            const char * S = G.peek(p); if ((S != nullptr) && ((*S) != 0)) return false;
            const char * Sleft = G.peek(p.X()-1,p.Y()); if ((Sleft != nullptr) && (((*Sleft)& maskright) != 0)) return false;
            const char * Sdown = G.peek(p.X(), p.Y()-1); if ((Sdown != nullptr) && (((*Sdown)& maskup) != 0)) return false;
            return true;
            }


        longOERRW(const  longOERRW &) = delete;             // no copy
        longOERRW & operator=(const  longOERRW &) = delete; //

        static const int RR = 5;				// size of the subsquare for the edge lattice
        static const char maskup = 1;			// mask for the up edge
        static const char maskright = 2;		// mask for the left edge

        double delta;						// reinforcement parameter delta
        iRect R;                            // rectangle containing the trace of the walk
        int64 N;							// size of the current range
        int64 step;							// number of steps performed
        iVec2 pos;							// current position of the walk
        mtools::Grid_factor<2,char,4,RR> G; // The lattice 

        mutable CImg<unsigned char>  im;	// image for drawing
        mutable MT2004_64  gen;				// the random number generator
    };



/* end of file OERRW.hpp */





