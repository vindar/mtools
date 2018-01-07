/****************************************************************************************************
* Classes for simulating a Once Edge Reinforced Random walks on Z^2   
****************************************************************************************************/

#pragma once

#include "mtools/mtools.hpp"
#include <cstdio>



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
            pos = { 0,0 };
            G.reset(maskfull, maskfull, false);
            im.resizeRaw(1, 1, true);
            }


        /* return the number of distinct sites visited by the walk */
        int64 nbVisited() const { return N; }


        /* return the range of the walk (useful for plotting the graph) */
        fBox2 rangeRect() const { return R; }


        /* return the actual coordinates of the walk */
        iVec2 currentPos() const { return pos; }


        /* return the reinforcement parameter */
        double reinfParam() const { return delta; }


        /* print some info about the walk */
        std::string toString() const
            {
            std::string s = "Once Edge Reinforced Random Walk ERRW\n";
            s += "  -> reinf. param. delta    = " + mtools::toString(reinfParam()) + "\n";
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
            int64 lastb = 0;
            char v = G(pos);		         // current site
            while (N < nb)
                {
                if (isFull(v)) 
                    { // the four edge around the site are set, do simple rw
                    if (lastb < 100) 
                        { // boundary not a long time ago, just one step
                        SRW_Z2_1step(pos, gen);
                        lastb++;
                        }
                    else
                        {
                        int64 d;
                        do
                            {
                            iBox2 fullR;
                            G.findFullBoxCentered(pos, fullR);  // find a full box
                            fullR.min[0]--; fullR.max[0]++; fullR.min[1]--; fullR.max[1]++;
                            d = SRW_Z2_MoveInRect(pos, fullR, 8, gen); // move in the rectangle 
                            }
                        while (d > 0);
                        }
                    v = G(pos);
                    } 
                else 
                    { // not all egdes are set, do ERRW
                    lastb = 0; // reset the last visit to boundary
                    const double up    = ((v & maskup) ? delta : 1.0);
                    const double right = ((v & maskright) ? delta : 1.0);
                    const double down  = ((v & maskdown) ? delta : 1.0);
                    const double left  = ((v & maskleft) ? delta : 1.0);
                    const double a = Unif(gen)*(up + right + down + left);
                    if (a < up + right) 
                        { 
                        if (a < up)
                            {
                            if ((v & maskup) == 0) { G.set(pos, v | maskup); pos.Y()++; v = G(pos); if (isEmpty(v)) { N++; if (PB != nullptr) { PB->update(N - startN); } } v |= maskdown; G.set(pos, v); }
                            else { pos.Y()++; v = G(pos); }
                            }
                        else
                            {
                            if ((v & maskright) == 0) { G.set(pos, v | maskright); pos.X()++; v = G(pos); if (isEmpty(v)) { N++; if (PB != nullptr) { PB->update(N - startN); } } v |= maskleft; G.set(pos, v); }
                            else { pos.X()++; v = G(pos); }
                            }
                        }
                    else
                        {
                        if (a < up + right + down)
                            {
                            if ((v & maskdown) == 0) { G.set(pos, v | maskdown); pos.Y()--; v = G(pos); if (isEmpty(v)) { N++; if (PB != nullptr) { PB->update(N - startN); } }  v |= maskup; G.set(pos, v); }
                            else { pos.Y()--;  v = G(pos); }
                            }
                        else
                            {
                            if ((v & maskleft) == 0) { G.set(pos, v | maskleft); pos.X()--; v = G(pos); if (isEmpty(v)) { N++; if (PB != nullptr) { PB->update(N - startN); } }  v |= maskright; G.set(pos, v); }
                            else { pos.X()--;  v = G(pos); }
                            }
                        }
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
            Plotter.range().setRange(zoomOut(fBox2(R)));
            Plotter.plot();
            }


        /* for drawing the color of a site */
        inline RGBc getColor(iVec2 p) const
            {
            const char * s = G.peek(p);
            if ((s == nullptr)||(isEmpty(*s))) return RGBc::c_TransparentWhite;
            return RGBc::c_Red;
            }

    private:

        inline bool isEmpty(char val) const { return (val == 0); }
        inline bool isFull(char val) const { return (val == maskfull); }


        longOERRW(const  longOERRW &) = delete;             // no copy
        longOERRW & operator=(const  longOERRW &) = delete; //

        static const int RR = 5;				// size of the subsquare for the edge lattice

        static const char maskup = 1;			// mask for the up edge
        static const char maskright = 2;		// mask for the left edge
        static const char maskdown = 4;			// mask for the up edge
        static const char maskleft = 8;		    // mask for the left edge
        static const char maskfull = (maskup | maskdown | maskleft | maskright); // full mask

        double delta;						// reinforcement parameter delta
        iBox2 R;                            // rectangle containing the trace of the walk
        int64 N;							// size of the current range
        iVec2 pos;							// current position of the walk
        mtools::Grid_factor<2,char,1,RR> G; // The lattice 

        mutable Image  im;					// image for drawing
        mutable MT2004_64  gen;				// the random number generator
    };



/* end of file OERRW.hpp */





