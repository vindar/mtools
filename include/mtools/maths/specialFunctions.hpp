/** @file specialFunctions.hpp */
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

#include "../misc/internal/mtools_export.hpp"
#include "../misc/misc.hpp"
#include "../misc/error.hpp"

namespace mtools
    {



	/**
	 * Fast inverse square root.
	 * Taken from wikipedia : https://en.wikipedia.org/wiki/Fast_inverse_square_root
	 * 
	 * @param	number	input value
	 *
	 * @return	an approximation of 1/sqrt(x). 
	 **/
	inline float fast_invsqrt(float x)
		{
		#ifdef __GNUC__
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstrict-aliasing"
		#endif
		long i;
		float x2, y;
		const float threehalfs = 1.5F;
		x2 = x * 0.5F;
		y = x;
		i = *(long *)&y;							// evil floating point bit level hacking
		i = 0x5f3759df - (i >> 1);					// what the fuck? 
		y = *(float *)&i;
		y = y * (threehalfs - (x2 * y * y));		// 1st iteration
   //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
		return y;
		#ifdef __GNUC__
		#pragma GCC diagnostic pop
		#endif
		}


    /**
     * Compute the logarithm of the gamma function.
     * Taken from numerical recipes.
     **/
    inline double gammln(const double xx)
        {
        int j;
        double x, tmp, y, ser;
        static const double cof[14] = { 57.1562356658629235,-59.5979603554754912,14.1360979747417471,-0.491913816097620199,.339946499848118887e-4,.465236289270485756e-4,-.983744753048795646e-4,.158088703224912494e-3,-.210264441724104883e-3,.217439618115212643e-3,-.164318106536763890e-3,.844182239838527433e-4,-.261908384015814087e-4,.368991826595316234e-5 };
        MTOOLS_ASSERT(xx > 0);
        y = x = xx;
        tmp = x + 5.24218750000000000;
        tmp = (x + 0.5)*log(tmp) - tmp;
        ser = 0.999999999999997092;
        for (j = 0;j < 14;j++) ser += cof[j] / ++y;
        return tmp + log(2.5066282746310005*ser / x);
        }


    /**
     * Compute the factorial for n in [0,170]
     * Taken from numerical recipes, (well, this one I could have written myself :-))
     **/
    inline double factrl(int64 n)
        {
        static double a[171];
        static bool init = true;
        if (init) { init = false; a[0] = 1.; for (int i = 1;i < 171;i++) a[i] = i*a[i - 1]; }
        MTOOLS_ASSERT((n >= 0 || n <= 170));
        return a[n];
        }


    /**
     * Compute the logarithm of a factorial.
     * Taken from numerical recipes.
     **/
    inline double factln(int64 n)
        {
        static const int64 NTOP = 2000;
        static double a[NTOP];
        static bool init = true;
        if (init) { init = false; for (int64 i = 0;i < NTOP;i++) a[i] = gammln(i + 1.); }
        MTOOLS_ASSERT(n >= 0);
        if (n < NTOP) return a[n];
        return gammln(n + 1.);
        }


    /**
     * Compute the binomial coefficient (n,k)
     * Taken from numerical recipes.
     **/
    inline double bico(const int64 n, const int64 k)
        {
        MTOOLS_ASSERT((n >= 0) || (k >= 0) || (k <= n));
        if (n < 171) return floor(0.5 + factrl(n) / (factrl(k)*factrl(n - k)));
        return floor(0.5 + exp(factln(n) - factln(k) - factln(n - k)));
        }


    /**
     * Compute the value of the Beta(z,w) function
     * Taken from numerical recipes.
     **/
    inline double beta(const double z, const double w)
        {
        return exp(gammln(z) + gammln(w) - gammln(z + w));
        }


    }

/* end of file */

