/** @file srwonZandZ2.hpp */
//
// Copyright 2015 Arvind Singh
//
// This file is part of the mtools library.
//
// mtools is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mtools  If not, see <http://www.gnu.org/licenses/>.


/**
* Example: IDLA.
* @code{.cpp}
class iDLA
	{
	public:
		void makeWalk(int64 nbParticules) {
			L1 = new GrowingLatticeZ2<char,5>(0,1); L2 = new GrowingLatticeZ2<int64,50>(0,-1);
			N = nbParticules; int64 step = 0;
			Chronometer();
			for(int64 i = 0;i< nbParticules;i++) {
				int64 x=0,y=0;
				while(L1->get(x,y) == 1) {// move until we exit the set of visited sites
					int64 xmin,xmax,ymin,ymax; L1->improvedEnclosingRectFull(x,y,xmin,xmax,ymin,ymax);
					if ((xmax -x > 1)&&(ymax - y > 1)&&(x-xmin > 1)&&(y -  ymin > 1)) {step += random::SRW_Z2_exitRectangle(x,y,xmin,xmax,ymin,ymax,gen);	}
					else {random::SRW_Z2_make1step(x,y,gen()); step++;} }
				L1->set(1,x,y); L2->set(i,x,y); }
			out << N << " particules in " << step << "steps.\nSimulation done in " << (double)(Chronometer())/1000 << " secondes\n";
			LatticePlotter<iDLA,false> Plotter2(*this); Plotter2.startPlot();
			delete L1; delete L2; }

		RGBc getColor(int64 x, int64 y) {if (L1->get(x,y) == 0) {return RGBc::c_White;} return RGBc::jet_palette(L2->get(x,y),0,N);}

	private:
		int64 N;							// number of particles
		MT2004_64  gen;						// the random number generator
		GrowingLatticeZ2<char,5> * L1;		// to check if a site is occupied
		GrowingLatticeZ2<int64,50> * L2;	// time of visit of this site
	};

int main(int argc, char *argv[])
	{
	iDLA O;	O.makeWalk(50000);
	return 0;
	}
*@endcode
**/

#pragma once

#include "srwonZ_cdf.hpp"

#include "../misc/misc.hpp"
#include "../maths/vec.hpp"


#include <algorithm>

namespace mtools
{



        namespace internals_randomgen
            {


            /**
            * Returns a sample of a discrete r.v. X whose CDF is given in tab
            * N    = size of the tab : [0,...N-1]
            * a    = random number in [0,1)
            * tab  = the CDF of X i.e. tab[i] = P( X <= i ) for i = 0.. N-1
            *
            * RQ : the function return N if a >= tab[N-1]
            *      complexity O(log N)
            **/
            inline unsigned int getValueFromDistrTab(const double * tab, unsigned int N, double a)
                {
                if (a < tab[0]) { return 0; }
                if (a >= tab[N - 1]) { return N; }
                unsigned int n1 = 0;	// tab[n1] <= a
                unsigned int n2 = N - 1;	// tab[n2] >  a
                while ((n2 - n1)>1)
                    {
                    unsigned int g = (n1 + n2) / 2;
                    if (a >= tab[g]) { n1 = g; }
                    else { n2 = g; }
                    }
                return n2; // on a necessairement tab[n1] <= a, tab[n2] > a
                }



            /* make 1 step for the SWR on Z */
            inline int64 SRW_Z_make1step(double a)
                {
                if (a < 0.5) { return 1; }
                return -1;
                }

            /* make 1 step for the SWR on Z2 */
            inline void SRW_Z2_make1step(iVec2 & pos, double a)
                {
                if (a < 0.25) { pos.X()++; return; }
                if (a < 0.5) { pos.X()--; return; }
                if (a < 0.75) { pos.Y()++; return; }
                pos.Y()++;
                return;
                }

            /* make 2 steps for the SWR on Z */
            inline int64 SRW_Z_make2steps(double a)
                {
                if (a < 0.25) return -2;
                if (a < 0.5) return 2;
                return 0;
                }

            /* make 2 steps for the SWR on Z2 */
            inline void SRW_Z2_make2steps(iVec2 & pos, double a1, double a2)
                {
                int64 A = SRW_Z_make2steps(a1);
                int64 B = SRW_Z_make2steps(a2);
                pos.X() += (A + B) / 2;
                pos.Y() += (A - B) / 2;
                return;
                }

            /* make 2^0 = 4 steps for the SWR on Z */
            inline int64 SRW_Z_make4steps(double a)
                {
                if (a < 0.0625) return -4;
                if (a < 0.125) return 4;
                if (a < 0.375) return -2;
                if (a < 0.625) return 2;
                return 0;
                }


            /* make 2^0 = 4 steps for the SWR on Z2 */
            inline void SRW_Z2_make4steps(iVec2 & pos, double a1, double a2)
                {
                int64 A = SRW_Z_make4steps(a1);
                int64 B = SRW_Z_make4steps(a2);
                pos.X() += (A + B) / 2;
                pos.Y() += (A - B) / 2;
                return;
                }


            /* make an arbitrary number of steps of the srw on Z */
            template<class random_t> inline int64 SRW_Z_makesteps(uint64 nbstep, random_t & gen)
                {
                int64 x = 0;
                if ((nbstep & 1) != 0) { x += SRW_Z_make1step(gen());  nbstep -= 1; if (nbstep == 0) return x; }    // one step
                if ((nbstep & 2) != 0) { x += SRW_Z_make2steps(gen()); nbstep -= 2; if (nbstep == 0) return x; }    // 2 steps
                if ((nbstep & 4) != 0) { x += SRW_Z_make4steps(gen()); nbstep -= 4; if (nbstep == 0) return x; }    // 4 steps
                // at least 8 steps must be performed
                if (nbstep >= (1 << 22))
                    { // use gaussian approximation
                    const double s = sqrt((double)nbstep);
                    const double epsilon = std::numeric_limits<double>::min();
                    const double two_pi = 2.0*3.14159265358979323846;
                    double u1; do { u1 = gen(); } while (u1 <= epsilon);
                    double u2 = gen();
                    int64 y = (int64)round(s*sqrt(-2.0 * log(u1)) * cos(two_pi * u2));
                    if ((y & 1) != (nbstep & 1)) { y += ((gen() < 0.5) ? 1 : -1); } // correct parity error
                    return y;
                    }
                // exact sampling for CDF
                int i = 3;
                while(1)
                    {
                    const int64 v = (((int64)1) << i);
                    if (nbstep & v) // bit i set
                        {
                        int64 y = 2 * getValueFromDistrTab(SRW_CDF_TAB[i - 2], (SRW_CDF_LEN[i - 2] / 2) + 1, gen());
                        x += ((gen() < 0.5) ? y : -y);
                        nbstep -= v;
                        if (nbstep == 0) return x;
                        }
                    i++;
                    }
                }


            /* make an arbitrary number of steps of the srw on Z^2 */
            template<class random_t> inline void SRW_Z2_makesteps(iVec2 & pos, uint64 nbstep, random_t & gen)
                {
                if ((nbstep & 1) != 0) { SRW_Z2_make1step(pos,gen());  nbstep -= 1; if (nbstep == 0) return x; }          // one step
                if ((nbstep & 2) != 0) { SRW_Z2_make2steps(pos,gen(),gen()); nbstep -= 2; if (nbstep == 0) return x; }    // 2 steps
                if ((nbstep & 4) != 0) { SRW_Z2_make4steps(pos,gen(), gen()); nbstep -= 4; if (nbstep == 0) return x; }   // 4 steps
                // at least 8 steps must be performed
                if (nbstep >= (1 << 22))
                    { // use gaussian approximation
                    const double s = sqrt((double)nsteps);
                    const double epsilon = std::numeric_limits<double>::min();
                    const double two_pi = 2.0*3.14159265358979323846;
                    double u1; do { u1 = gen(); } while (u1 <= epsilon);
                    double u2 = gen();
                    double c = -1;
                    int64 A = (int64)round(s*sqrt(-2.0 * log(u1)) * cos(two_pi * u2)); if ((A & 1) != (nbstep & 1)) { c = gen(); A += (((c < 0.25)||(c>=0.75)) ? 1 : -1); }
                    int64 B = (int64)round(s*sqrt(-2.0 * log(u1)) * sin(two_pi * u2)); if ((B & 1) != (nbstep & 1)) { if (c < 0) { c = gen(); } B += ((c < 0.5) ? 1 : -1); }
                    pos.X() += (A + B)/2;
                    pos.Y() += (A - B)/2;
                    return ;
                    }
                // exact sampling for CDF
                int i = 3;
                while (1)
                    {
                    const int64 v = (1 << i);
                    if (nbstep & v) // bit i set
                        {
                        int64 A += 2 * getValueFromDistrTab(SRW_CDF_TAB[i - 2], (SRW_CDF_LEN[i - 2] / 2) + 1, gen());
                        int64 B += 2 * getValueFromDistrTab(SRW_CDF_TAB[i - 2], (SRW_CDF_LEN[i - 2] / 2) + 1, gen());
                        double c = gen();
                        if ((c < 0.25) || (c >= 0.75)) {A = -A;}
                        if (c < 0.5) { B = -B };
                        pos.X() += (A + B) / 2;
                        pos.Y() += (A - B) / 2;
                        nbstep -= v;
                        if (nbstep == 0) return x;
                        }
                    i++;
                    }
                }





            /* make an arbitrary number of step of the srw on Z */
            template<class random_t> inline void SRW_Z2_make(iVec2 & pos, uint64 nbstep, random_t & gen)
                {
                if ((nbstep & 1) != 0) { SRW_Z2_make0(pos, gen()); if (nbstep < 2) return x; }
                if ((nbstep & 2) != 0) { SRW_Z2_make1(pos, gen(), gen()); if (nbstep < 4) return x; }
                if ((nbstep & 4) != 0) { SRW_Z2_make2(pos, gen(), gen()); if (nbstep < 8) return x; }
                if ((nbstep & 8) != 0) { SRW_Z2_make3(pos, gen(), gen()); if (nbstep < 16) return x; }
                if ((nbstep & 16) != 0) { SRW_Z2_make4(pos, gen(), gen()); if (nbstep < 32) return x; }
                if ((nbstep & 32) != 0) { SRW_Z2_make5(pos, gen(), gen()); if (nbstep < 64) return x; }
                if ((nbstep & 64) != 0) { SRW_Z2_make6(pos, gen(), gen()); if (nbstep < 128) return x; }
                if ((nbstep & 128) != 0) { SRW_Z2_make7(pos, gen(), gen()); if (nbstep < 256) return x; }
                if ((nbstep & 256) != 0) { SRW_Z2_make8(pos, gen(), gen()); if (nbstep < 512) return x; }
                if ((nbstep & 512) != 0) { SRW_Z2_make9(pos, gen(), gen()); if (nbstep < 1024) return x; }
                if ((nbstep & 1024) != 0) { SRW_Z2_make10(pos, gen(), gen()); if (nbstep < 2048) return x; }
                if ((nbstep & 2048) != 0) { SRW_Z2_make11(pos, gen(), gen(), gen()); if (nbstep < 4096) return x; }
                if ((nbstep & 4096) != 0) { SRW_Z2_make12(pos, gen(), gen(), gen()); if (nbstep < 8192) return x; }
                if ((nbstep & 8192) != 0) { SRW_Z2_make13(pos, gen(), gen(), gen()); if (nbstep < 16384) return x; }
                if ((nbstep & 16384) != 0) { SRW_Z2_make14(pos, gen(), gen(), gen()); if (nbstep < 32768) return x; }
                if ((nbstep & 32768) != 0) { SRW_Z2_make15(pos, gen(), gen(), gen()); if (nbstep < 65536) return x; }
                if ((nbstep & 65536) != 0) { SRW_Z2_make16(pos, gen(), gen(), gen()); if (nbstep < 131072) return x; }
                if ((nbstep & 131072) != 0) { SRW_Z2_make17(pos, gen(), gen(), gen()); if (nbstep < 262144) return x; }
                if ((nbstep & 262144) != 0) { SRW_Z2_make18(pos, gen(), gen(), gen()); if (nbstep < 524288) return x; }
                if ((nbstep & 524288) != 0) { SRW_Z2_make19(pos, gen(), gen(), gen()); if (nbstep < 1048576) return x; }
                if ((nbstep & 1048576) != 0) { SRW_Z2_make20(pos, gen(), gen(), gen()); }
                nbstep = nbstep / 2097152;
                for (uint64 i = 0; i < nbstep; i++) { x += SRW_Z2_make21(pos, gen(), gen(), gen()); }
                }




        /**
         * Perform some steps of the random walk on Z^2 while insuring that it never exists the closed
         * ball of radius R around the starting position. The number of step is not determined and neither is the ending position. 
         * The method is simply optimised to be fast
         *
         * @tparam  random_t    Type of the random t.
         * @param [in,out]  X   The X coordinate.
         * @param [in,out]  Y   The Y coordinate.
         * @param   rad         The xmin.
         * @param [in,out]  gen The generate.
         *
         * @return  The number of step performed.
         **/
        template<class random_t> inline int64 SRW_Z2_walkInside(int64 & X, int64 & Y, int64 rad, random_t & gen)
            {
            }


        template<class random_t> inline int64 SRW_Z2_walkInside(iVec2 & pos, int64 rad, random_t & gen)
            {
            }


        /**
         * Perform a SRW on Z^2 until it exits a given rectangle. The walk is stoped when it is OUTSIDE
         * of the rectangle i.e. (X == Xmin-1)||(X == Xmax+1)||(Y == Ymin-1)||(Y == Ymax+1)
         *
         * @param [in,out]  X   The X coordinate.
         * @param [in,out]  Y   The Y coordinate.
         * @param   Xmin        min X coordinate of the rectangle.
         * @param   Xmax        max X coordinate of the rectangle.
         * @param   Ymin        min Y coordinate of the rectangle.
         * @param   Ymax        max Y coordinate of the rectangle.
         * @param [in,out]  gen random number generator.
         *
         * @return  The number of step it took to exit the rectangle.
        **/
		template<class random_t> inline int64 SRW_Z2_exitRectangle(iVec2 pos, iRect box,random_t & gen)
			{
            using std::min;
			int64 N = 0; // number of steps
			int64 R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1; // R = distance to the boundary of the rectangle
			while(R > 0) // we are inside the rectangle
				{
				// here we use the reflection principle which state that P(sup_{k<=n} |S_k| >= a) \leq 4 P(S_n >= a)  
				// this enables to do many steps at one time while insuring that we do not get out of the rectangle
                //if (R >= SRW_CDF_21maxZ2) { SRW_Z2_make21(pos, gen(), gen(), gen()); N += ; R = min(min(X - Xmin, Y - Ymin), min(Xmax - X, Ymax - Y)) + 1; } else {


                /*
				if (R >= internals_randomgen::SRW_CDF_2000000max) {SRW_Z2_make2000000steps(X,Y,gen(),gen(),gen()); N+= 2000000; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_200000max) {SRW_Z2_make200000steps(X,Y,gen(),gen(),gen()); N+= 200000; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_20000max) {SRW_Z2_make20000steps(X,Y,gen(),gen(),gen()); N+= 20000; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_5000max) {SRW_Z2_make5000steps(X,Y,gen(),gen(),gen()); N+= 5000; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_2000max) {SRW_Z2_make2000steps(X,Y,gen(),gen(),gen()); N+= 2000; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_500max) {SRW_Z2_make500steps(X,Y,gen(),gen(),gen()); N+= 500; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_200max) {SRW_Z2_make200steps(X,Y,gen(),gen(),gen()); N+= 200; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_50max) {SRW_Z2_make50steps(X,Y,gen(),gen(),gen()); N+= 50; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else {
				if (R >= internals_randomgen::SRW_CDF_10max) {SRW_Z2_make10steps(X,Y,gen(),gen(),gen()); N+= 10; R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;} else
					{
					while((R>0)&&(R<10)) // stay in this loop until we exit or we are at distance 10 from the boundary
						{
						for(int64 i=0;i<R;i++) {SRW_Z2_make1step(X,Y,gen());}
						N += R;
						R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1;
						}
					}}}}}}}}}*/
				}
			return N;
			}


        /**
         * Perform a SRW on Z^2 until it exits a given rectangle. The walk is stopped when it is OUTSIDE
         * of the rectangle.
         *
         * @param [in,out]  pos The position of the walk.
         * @param   rect        The rectangle.
         * @param [in,out]  gen The random number generator.
         *
         * @return The number of step it took to exit the rectangle.
        **/
        template<class random_t> inline int64 SRW_Z2_exitRectangle(iVec2 & pos, iRect rect, random_t & gen)
            {
            return SRW_Z2_exitRectangle(pos.X(), pos.Y(), rect.xmin, rect.xmax, rect.ymin, rect.ymax, gen);
            }


        }

}


/* end of file */




