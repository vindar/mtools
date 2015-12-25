/** @file SRW.hpp */
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

#pragma once

#include "../misc/misc.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "exitGridSRWZ2.hpp"
#include "classiclaws.hpp"

#include <algorithm>

namespace mtools
    {


        /**
         * Make a single step for the SRW on Z.
         *
         * @param [in,out]  gen random generator
         *
         * @return  +/-1 with equal probability. 
         **/
        template<class random_t> inline int SRW_Z_1step(random_t & gen)
            {
            return ((Unif_1(gen) == 0) ? -1 : 1);
            }


        /**
         * Make a given number of step for the simple random walk in Z 
         *
         * @param   n   number of step to make
         * @param   gen random generator
         *
         * @return  A rv distributed as 2 * Bin(n,1/2) - n.
         **/
        template<class random_t> inline int SRW_Z(int n, random_t & gen)
            { 
            MTOOLS_ASSERT(n > 0);
            if (n <= 320)
                { // code the walk from the bits of an uniform uint64.
                int k = 0;
                do
                    {
                    int m = ((n >= 64) ? 64 : n); n -= 64; k -= m;
                    uint64 u = Unif_64(gen);
                    for (int j = 0; j < m; j++) { k += 2 * (u & 1); u >>= 1; }
                    }
                while (n > 0);
                return k;
                }
           static const double plog = log(0.5); 
           int k;
           for (;;)
                {
                double u = 0.645*Unif(gen);
                double v = -0.63 + 1.25*Unif(gen);
                double v2 = v*v;
                if (v >= 0.) { if (v2 > 6.5*u*(0.645 - u)*(u + 0.2)) continue; } else { if (v2 > 8.4*u*(0.645 - u)*(u + 0.1)) continue; }
                k = int(floor(sqrt(n*0.25)*(v / u) + n*0.5 + 0.5));
                if ((k < 0) || (k > n)) continue;
                double u2 = u*u;
                if (v >= 0.) { if (v2 < 12.25*u2*(0.615 - u)*(0.92 - u)) break; } else { if (v2 < 7.84*u2*(0.615 - u)*(1.2 - u)) break; }
                double b = sqrt(n*0.25)*exp( gammln(n + 1.) + n*plog - (gammln(k + 1.) + gammln(n - k + 1.)) );
                if (u2 < b) break;
                }
            return (2*k - n);
            }


        /**
         * Make a single step for the SRW on Z^2.
         *
         * @param [in,out]  gen random generator.
         *
         **/
        template<class random_t> inline void SRW_Z2_1step(iVec2 & pos, random_t & gen)
            {
            switch(Unif_2(gen))
                {
                case 0: pos.X()--; return;
                case 1: pos.X()++; return;
                case 2: pos.Y()--; return;
                case 3: pos.Y()++; return;
                }
            MTOOLS_ERROR("wtf...");
            }


        /**
         * Make a given number of step for the simple random walk in Z starting from position pos.
         *
         * @param [in,out]  pos the position of the walk.
         * @param   n   number of step to make
         * @param   gen random generator
         *
         **/
        template<class random_t> inline void SRW_Z2(iVec2 & pos, uint64 n, random_t & gen)
            { // 45 degree rotation of two indep. SRW on Z
            int64 A = SRW_Z(n,gen);
            int64 B = SRW_Z(n,gen);
            pos.X() += (A + B) / 2;
            pos.Y() += (A - B) / 2;
            return;
            }


        /**
         * Move the SRW while staying inside the rectangle R. Contrarily to SRW_ExitRect(), the position
         * pos when the method returns need not be on the boundary of R. However, When the method
         * returns, the distance to the (inner) boundary of the rectangle has been divided by at least
         * the parameter 'ratio' compared to the initial distance from the boundary.
         *
         * @param [in,out]  pos The position of the walk.
         * @param   R           The rectangle.
         * @param   ratio       The ratio by which the distance to the (inner) boundary has to decrease
         *                      before we stop (set to <=0 for infinite ratio = stop at the
         *                      boundary). 8 is a good choice usually.
         * @param [in,out]  gen The random number generator.
         *
         * @return  The new distance to the inner boundary.
         **/
        template<class random_t> int64 SRW_Z2_MoveInRect(iVec2 & pos, iBox2 R, uint64 ratio, random_t & gen)
            {
            MTOOLS_ASSERT((!R.isEmpty()) && (R.isInside(pos)));
            int64 min_d = ((ratio <= 0) ? 0 : R.boundaryDist(pos)/ratio);
            int64 d;
            while((d = R.boundaryDist(pos)) >  min_d)
                { // keep looping while we are striclty inside the rectangle.
                if (d == 1)
                    { // make one step
                    switch (Unif_2(gen))
                        {
                        case  0: pos.X()++; break;
                        case  1: pos.X()--; break;
                        case  2: pos.Y()++; break;
                        case  3: pos.Y()--; break;
                        }
                    continue;
                    }
                if (d == 2)
                    { //  square of radius 2
                    switch(Unif_4(gen))
                        {
                        case  0: 
                        case  1: pos.X() += 2; break;
                        case  2: 
                        case  3: pos.X() -= 2; break;
                        case  4: 
                        case  5: pos.Y() += 2; break;
                        case  6: 
                        case  7: pos.Y() -= 2; break;
                        case  8: pos.X() += 2; pos.Y()++; break;
                        case  9: pos.X() += 2; pos.Y()--; break;
                        case 10: pos.X() -= 2; pos.Y()++; break;
                        case 11: pos.X() -= 2; pos.Y()--; break;
                        case 12: pos.Y() += 2; pos.X()++; break;
                        case 13: pos.Y() += 2; pos.X()--; break;
                        case 14: pos.Y() -= 2; pos.X()++; break;
                        case 15: pos.Y() -= 2; pos.X()--; break;
                        }
                    continue;
                    }
                if (d < 128)
                    { // 2 < d < 128 : we use the exact CDF from the small grid array
                    int64 off = sampleDiscreteRVfromCDF(internals_random::_srwExitGridSmallR[d], d-1, gen);
                    switch (Unif_3(gen))
                        {
                        case  0: pos.X() += d; pos.Y() += off; break;
                        case  1: pos.X() += d; pos.Y() -= off; break;
                        case  2: pos.X() -= d; pos.Y() += off; break;
                        case  3: pos.X() -= d; pos.Y() -= off; break;
                        case  4: pos.Y() += d; pos.X() += off; break;
                        case  5: pos.Y() += d; pos.X() -= off; break;
                        case  6: pos.Y() -= d; pos.X() += off; break;
                        case  7: pos.Y() -= d; pos.X() -= off; break;
                        }
                    continue;
                    }
                if (d < 1152)
                    { // 128 <= d < 1152 : use the neareset 128 multiple and exact CDF from the large grid array
                    int64 b = (d >> 7);  // divide by 128
                    int64 l = (b << 7); // get the nearest lower multiple of 128
                    int64 off = sampleDiscreteRVfromCDF(internals_random::_srwExitGridLargeR[b], l-1, gen);
                    switch (Unif_3(gen))
                        {
                        case  0: pos.X() += l; pos.Y() += off; break;
                        case  1: pos.X() += l; pos.Y() -= off; break;
                        case  2: pos.X() -= l; pos.Y() += off; break;
                        case  3: pos.X() -= l; pos.Y() -= off; break;
                        case  4: pos.Y() += l; pos.X() += off; break;
                        case  5: pos.Y() += l; pos.X() -= off; break;
                        case  6: pos.Y() -= l; pos.X() += off; break;
                        case  7: pos.Y() -= l; pos.X() -= off; break;
                        }
                    continue;
                    }
                // d >= 1152 We choose a point uniformly on the circle of radius d
                double a = Unif(gen)*TWOPI;
                pos.X() += (int64)round(d*sin(a));
                pos.Y() += (int64)round(d*cos(a));
                }
            MTOOLS_ASSERT(d >= 0);
            return d;
            }


        /**
         * Move the SRW starting from pos until it reaches the INNER boudary of the rectangle R.
         * 
         * More precisely, if R = [a,b]x[c,d] and the returned position pos = (x,y) then
         * either x = a or b and y in ]c,d[) or y = c or d and x in ]a,b[) (assuming the 
         * starting position is indeed in the interior of R).
         * 
         * @param [in,out]  pos The position of the walk
         * @param   R           The rectangle
         * @param [in,out]  gen The random number generator
         **/
        template<class random_t> inline void SRW_Z2_ExitRect(iVec2 & pos, iBox2 R, random_t & gen)
            {
            int64 d = SRW_Z2_MoveInRect(pos, R, -1,gen); // set ratio to infinity
            MTOOLS_ASSERT(d == 0);
            return;
            }



    }


/* end of file */




