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

		// forward declarations
        namespace internals_randomgen
            {
            unsigned int getValueFromDistrTab(const double * tab, unsigned int N, double a);
            }

		int SRW_Z_make1step(double a);
		void SRW_Z2_make1step(int64 & X,int64 & Y,double a);
		int SRW_Z_make10steps(double a1,double a2);
		void SRW_Z2_make10steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make20steps(double a1,double a2);
		void SRW_Z2_make20steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make50steps(double a1,double a2);
		void SRW_Z2_make50steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make100steps(double a1,double a2);
		void SRW_Z2_make100steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make200steps(double a1,double a2);
		void SRW_Z2_make200steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make500steps(double a1,double a2);
		void SRW_Z2_make500steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make2000steps(double a1,double a2);
		void SRW_Z2_make2000steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make5000steps(double a1,double a2);
		void SRW_Z2_make5000steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make20000steps(double a1,double a2);
		void SRW_Z2_make20000steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make200000steps(double a1,double a2);
		void SRW_Z2_make200000steps(int64 & X,int64 & Y,double a1,double a2,double a3);
		int SRW_Z_make2000000steps(double a1,double a2);
		void SRW_Z2_make2000000steps(int64 & X,int64 & Y,double a1,double a2,double a3);


        /**
         * Returns the position of a SRW on Z after N steps. Particularly efficient for large number of
         * steps.
         *
         * @param [in,out]  gen the random number generator.
         * @param   N           number of steps to make.
         *
         * @return the position after N step starting from the origin.
        **/
		template<class random_t> inline int64 SRW_Z_makesteps(random_t & gen,int64 N)
			{
			int64 X = 0;
			while(N>=2000000) {X += SRW_Z_make2000000steps(gen(),gen()); N-= 2000000;}
			while(N>=200000) {X += SRW_Z_make200000steps(gen(),gen()); N-= 200000;}
			while(N>=20000) {X += SRW_Z_make20000steps(gen(),gen()); N-= 20000;}
			while(N>=2000) {X += SRW_Z_make2000steps(gen(),gen()); N-= 2000;}
			while(N>=200) {X += SRW_Z_make200steps(gen(),gen()); N-= 200;}
			while(N>=50) {X += SRW_Z_make50steps(gen(),gen()); N-= 50;}
			while(N>=10) {X += SRW_Z_make10steps(gen(),gen()); N-= 10;}
			while(N>=1) {X += SRW_Z_make1step(gen()); N--;}
			return X;
			}


        /**
         * Returns the position of a SRW on Z2 after N steps. Particularly efficient for large number of
         * steps. Shift  X and Y by the coordinate of the walk after N steps.
         *
         * @param [in,out]  X   The starting X coordinate.
         * @param [in,out]  Y   The starting Y coordinate.
         * @param [in,out]  gen the random number generator.
         * @param   N           number of steps to make.
        **/
        template<class random_t> inline void SRW_Z2_makesteps(int64 & X,int64 & Y,random_t & gen,int64 N)
			{
			while(N>=2000000) {SRW_Z2_make2000000steps(X,Y,gen(),gen(),gen()); N-= 2000000;}
			while(N>=200000) {SRW_Z2_make200000steps(X,Y,gen(),gen(),gen()); N-= 200000;}
			while(N>=20000) {SRW_Z2_make20000steps(X,Y,gen(),gen(),gen()); N-= 20000;}
			while(N>=2000) {SRW_Z2_make2000steps(X,Y,gen(),gen(),gen()); N-= 2000;}
			while(N>=200) {SRW_Z2_make200steps(X,Y,gen(),gen(),gen()); N-= 200;}
			while(N>=50) {SRW_Z2_make50steps(X,Y,gen(),gen(),gen()); N-= 50;}
			while(N>=10) {SRW_Z2_make10steps(X,Y,gen(),gen(),gen()); N-= 10;}
			while(N>=1) {SRW_Z2_make1step(X,Y,gen()); N--;}
			return;
			}

        template<class random_t> inline void SRW_Z2_makesteps(iVec2 & pos, random_t & gen, int64 N)
            {
            SRW_Z2_makesteps(pos.X(), pos.Y(), gen, N);
            }

        /**
         * Srw z coordinate 2 makesteps.
         *
         * @tparam  random_t    Type of the random t.
         * @param [in,out]  gen The generate.
         * @param   N           The int64 to process.
         *
         * @return  the position after N step starting from the origin.
        **/
        template<class random_t> inline iVec2 SRW_Z2_makesteps(random_t & gen, int64 N)
            {
            iVec2 pos(0, 0);
            SRW_Z2_makesteps(pos, gen, N);
            return pos;
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
		template<class random_t> inline int64 SRW_Z2_exitRectangle(int64 & X,int64 & Y,int64 Xmin,int64 Xmax,int64 Ymin,int64 Ymax,random_t & gen)
			{
            using std::min;
			int64 N = 0; // number of steps
			int64 R = min(min(X-Xmin,Y-Ymin),min(Xmax-X,Ymax-Y)) + 1; // R = distance to the boundary of the rectangle
			while(R > 0) // we are inside the rectangle
				{
				// here we use the reflection principle which state that P(sup_{k<=n} |S_k| >= a) \leq 4 P(S_n >= a)  (see my first paper for details)
				// this enables to do many steps at one time while insuring that we do not get out of the rectangle
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
					}}}}}}}}}
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


		/**
		* Returns the position of a SRW on Z after 1 steps
		* a = random number in [0,1)
		**/
		inline int SRW_Z_make1step(double a)
			{
			if (a < 0.5) {return 1;}
			return -1;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 1 steps
		* X,Y are shifted by the new position
		* a = random number in [0,1)
		**/
		inline void SRW_Z2_make1step(int64 & X,int64 & Y,double a)
			{
			if (a < 0.25) {X++; return;}
			if (a < 0.5) {X--; return;}
			if (a < 0.75) {Y++; return;}
			Y--;
			return;
			}


        inline void SRW_Z2_make1step(iVec2 & pos, double a)
            {
            SRW_Z2_make1step(pos.X(), pos.Y(), a);
            }



		/**
		* Returns the position of a SRW on Z after 10 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 10
		**/
		inline int SRW_Z_make10steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_10steps,(internals_randomgen::SRW_CDF_10max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 10 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random numbers in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 10
		**/
		inline void SRW_Z2_make10steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make10steps(a1,a3);
			int B = SRW_Z_make10steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make10steps(iVec2 & pos, double a1, double a2,double a3)
            {
            SRW_Z2_make10steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 20 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 20
		**/
		inline int SRW_Z_make20steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_20steps,(internals_randomgen::SRW_CDF_20max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 20 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random numbers in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 20
		**/
		inline void SRW_Z2_make20steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make20steps(a1,a3);
			int B = SRW_Z_make20steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make20steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make20steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 50 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 50
		**/
		inline int SRW_Z_make50steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_50steps,(internals_randomgen::SRW_CDF_50max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}

		/**
		* Returns the position of a SRW on the Z^2 after 50 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random numbers in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 50
		**/
		inline void SRW_Z2_make50steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make50steps(a1,a3);
			int B = SRW_Z_make50steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}


        inline void SRW_Z2_make50steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make50steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 100 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 76
		**/
		inline int SRW_Z_make100steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_100steps,(internals_randomgen::SRW_CDF_100max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}

		/**
		* Returns the position of a SRW on the Z^2 after 100 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random numbers in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 76
		**/
		inline void SRW_Z2_make100steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make100steps(a1,a3);
			int B = SRW_Z_make100steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make100steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make100steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 200 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 112
		**/
		inline int SRW_Z_make200steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_200steps,(internals_randomgen::SRW_CDF_200max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}

		/**
		* Returns the position of a SRW on the Z^2 after 200 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random numbers in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 112
		**/
		inline void SRW_Z2_make200steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make200steps(a1,a3);
			int B = SRW_Z_make200steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make200steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make200steps(pos.X(), pos.Y(), a1, a2, a3);
            }



		/**
		* Returns the position of a SRW on Z after 500 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 182
		**/
		inline int SRW_Z_make500steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_500steps,(internals_randomgen::SRW_CDF_500max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 500 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random numbers in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 182
		**/
		inline void SRW_Z2_make500steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make500steps(a1,a3);
			int B = SRW_Z_make500steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make500steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make500steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 2000 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 364
		**/
		inline int SRW_Z_make2000steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_2000steps,(internals_randomgen::SRW_CDF_2000max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Return the position of a SRW on the Z^2 after 2000 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random number in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 364
		**/
		inline void SRW_Z2_make2000steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make2000steps(a1,a3);
			int B = SRW_Z_make2000steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make2000steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make2000steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 5000 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 574
		**/
		inline int SRW_Z_make5000steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_5000steps,(internals_randomgen::SRW_CDF_5000max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}

		/**
		* Return the position of a SRW on the Z^2 after 5000 steps
		* X,Y are shifted by the new position
		* a1,a2,a3 = random number in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 574
		**/
		inline void SRW_Z2_make5000steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make5000steps(a1,a3);
			int B = SRW_Z_make5000steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make5000steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make5000steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 20 000 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 1134
		**/
		inline int SRW_Z_make20000steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_20000steps,(internals_randomgen::SRW_CDF_20000max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 20 000 steps
		* X,Y are shifted by the new position
		* a1,a2,a3,a4 = random number in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 1134
		**/
		inline void SRW_Z2_make20000steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make20000steps(a1,a3);
			int B = SRW_Z_make20000steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}


        inline void SRW_Z2_make20000steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make20000steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 200 000 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 3528
		**/
		inline int SRW_Z_make200000steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_200000steps,(internals_randomgen::SRW_CDF_200000max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 200 000 steps
		* X,Y are shifted by the new position
		* a1,a2,a3,a4 = random number in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 3528
		**/
		inline void SRW_Z2_make200000steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make200000steps(a1,a3);
			int B = SRW_Z_make200000steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}

        inline void SRW_Z2_make200000steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make200000steps(pos.X(), pos.Y(), a1, a2, a3);
            }


		/**
		* Returns the position of a SRW on Z after 2 000 000 steps
		* a1,a2 = random number in [0,1)
		* note : the  value x returned is such that |x| <= 10968
		**/
		inline int SRW_Z_make2000000steps(double a1,double a2)
			{
			int X = 2*internals_randomgen::getValueFromDistrTab(internals_randomgen::SRW_CDF_2000000steps,(internals_randomgen::SRW_CDF_2000000max/2)+1,a1);
			if (a2<0.5) return(-X);
			return X;
			}


		/**
		* Returns the position of a SRW on the Z^2 after 2 000 000 steps
		* X,Y are shifted by the new position
		* a1,a2,a3,a4 = random number in [0,1)
		* note : the coordinates (X,Y) returned are such that |X| , |Y| <= 10968
		**/
		inline void SRW_Z2_make2000000steps(int64 & X,int64 & Y,double a1,double a2,double a3)
			{
			double a4 = (((a3 < 0.25)||(a3 >= 0.75)) ? 0.25 : 0.75);
			int A = SRW_Z_make2000000steps(a1,a3);
			int B = SRW_Z_make2000000steps(a2,a4);
			X += (A+B)/2;
			Y += (A-B)/2;
			return;
			}


        inline void SRW_Z2_make2000000steps(iVec2 & pos, double a1, double a2, double a3)
            {
            SRW_Z2_make2000000steps(pos.X(), pos.Y(), a1, a2, a3);
            }


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
            *
            * INTERNAL METHOD, SHOULD NOT BE CALLED FROM OUTSIDE
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

            }

}


/* end of file */




