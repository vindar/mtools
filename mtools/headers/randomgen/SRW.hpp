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
#include "../maths/rect.hpp"
#include "exitGridSRWZ2.hpp"
#include "classiclaws.hpp"

#include <algorithm>

namespace mtools
    {


        /**
         * Make a given number of step for the simple random walk in Z 
         *
         * @param   n   number of step to make
         * @param   gen random generator
         *
         * @return  A rv distributed as Bin(n,1/2).
         **/
        template<class random_t> inline int64 SRW_Z(uint64 n, random_t & gen)
            { // compute a Binomial(n,1/2)
           
            return 0;
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
            { // use 45 degree rotation of two indep. SRW on Z
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
         * @tparam  random_t    Type of the random t.
         * @param [in,out]  pos The position of the walk.
         * @param   R           The rectangle.
         * @param [in,out]  gen The random number generator.
         * @param   ratio       The ratio by which the distance to the (inner) boundary has to decrease
         *                      before we stop (set to a negative value for infinite ratio = stop at the
         *                      boundary).
         *
         * @return  The new distance to the inner boundary.
         **/
        template<class random_t> int64 SRW_Z2_MoveInRect(iVec2 & pos, iRect R, random_t & gen, uint64 ratio = 8)
            {
            MTOOLS_ASSERT((!R.isEmpty()) && (R.isInside(pos)));
            int64 min_d = ((ratio <= 0) ? 0 : R.boundaryDist(pos)/ratio);
            while((d = R.boundaryDist(pos)) >  min_d)
                { // keep looping while we are striclty inside the rectangle.
                if (d == 1)
                    { // one step
                    switch (Unif_int(0,3,gen))
                        {
                        case  0: pos.X()++; break;
                        case  1: pos.X()--; break;
                        case  2: pos.Y()++; break;
                        case  3: pos.Y()--; break;
                        }
                    continue;
                    }
                if (d < 4)
                    { //  square of radius 2
                    switch(Unif_int(0, 15, gen))
                        {
                        case  0: pos.X() += 2; break;
                        case  1: pos.X() += 2; break;
                        case  2: pos.X() -= 2; break;
                        case  3: pos.X() -= 2; break;
                        case  4: pos.Y() += 2; break;
                        case  5: pos.Y() += 2; break;
                        case  6: pos.Y() -= 2; break;
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
                if (d > 2047)
                    { // use a uniformly distributed point on the circle of radius d
                    double a = Unif(gen)*TWOPI;
                    pos.X() += (int64)round(d*sin(a));
                    pos.Y() += (int64)round(d*cos(a));
                    continue;
                    }
                // d is in [4,2048[, use precomputed tables.
                d >>= 2;
                int64 l = 4;
                size_t index = 0;
                while (d > 1) { index++; l <<= 1; d >>= 1; }
                int 64 off = sampleDiscreteRVfromCDF(SRWexitGrid_CDF[i], l - 1, gen);
                switch(Unif_int(0, 7, gen))
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
                }
            MTOOLS_ASSERT(d >= 0);
            return d;
            }


        /**
         * Move the SRW starting from pos until it reaches the INNER boudary of the rectangle R.
         * 
         * More precisely, if R = [a,b]x[c,d] and the returned position pos = (x,y) then
         * either (x is a or b and y in ]c,d[) or (y is c or d and x in ]a,b[) (assuming the 
         * starting position is indeed in the interior of R
         * 
         * @param [in,out]  pos The position of the walk
         * @param   R           The rectangle
         * @param [in,out]  gen The random number generator
         **/
        template<class random_t> inline int64 SRW_Z2_ExitRect(iVec2 & pos, iRect R, random_t & gen)
            {
            int64 d = SRW_Z2_MoveInRect(pos, R, gen,-1); // set ratio to infinity
            MTOOLS_ASSERT(d == 0);
            return;
            }



    }


/* end of file */




