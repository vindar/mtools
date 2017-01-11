/** @file gen_xorgen4096_64.hpp */
//
// Copyright 2004, 2006, 2008 R. P. Brent. for the xorgen generator.
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
#include "../misc/timefct.hpp"


namespace mtools
{


    /**
    * Xorgen random number generator by Richard Brent
    * 64 bit version for both integers and floating point numbers
    **/
    class XorGen4096_64
    {
    public:


    public:


        /* type of integer returned by the generator */
        typedef uint64 result_type;


        /* min value */
        static constexpr result_type min() { return 0; }


        /* max value */
        static constexpr result_type max() { return 18446744073709551615ULL; }


        /* return a random number */
        inline uint64 operator()() { return randproc64(); }


        /* discard results */
        void discard(unsigned long long z) { for (unsigned long long i = 0; i < z; i++) operator()(); }


        /* change the seed */
        void seed(result_type s) { zero = 0; i = -1; init_gen(s); }


        /**
        * Default constructor. Init with a unique random seed.
        **/
        XorGen4096_64() { seed((uint64)randomID()); }


        /**
        * Constructor with a given seed
        **/
        XorGen4096_64(result_type s) { seed(s); }



    private:

    /* initialize the generator with a seed */
    void init_gen(uint64 seed)
        {
        uint64 t, v;
        int k;
        weyl = ((((uint64)0x61c88646)<<16)<<16) + (uint64)0x80b583eb;
        v = (seed!=zero)? seed:~seed;
        for (k = wlen; k > 0; k--) { v ^= v<<10; v ^= v>>15; v ^= v<<4;  v ^= v>>13;}
        for (w = v, k = 0; k < r; k++) { v ^= v<<10; v ^= v>>15; v ^= v<<4;  v ^= v>>13; x[k] = v + (w+=weyl); }
        for (i = r-1, k = 4*r; k > 0; k--) { t = x[i = (i+1)&(r-1)];   t ^= t<<a;  t ^= t>>b; v = x[(i+(r-s))&(r-1)];   v ^= v<<c;  v ^= v>>d; x[i] = t^v;}
        }


    /* return a new 64bit random number */
    inline uint64 randproc64()
        {
        uint64 t, v;
        t = x[i = (i+1)&(r-1)];
        v = x[(i+(r-s))&(r-1)];
        t ^= t<<a;  t ^= t>>b;
        v ^= v<<c;  v ^= v>>d;
        x[i] = (v ^= t);
        w += weyl;
        return (v + (w^(w>>ws)));
        }


    static const uint64 wlen = 64;
    static const int r = 64;
    static const int64 s = 53;
    static const int a = 33;
    static const int b = 26;
    static const int c = 27;
    static const int d = 29;
    static const int ws = 27;


    /* state of the generator */
    uint64 w;
    uint64 weyl;
    uint64 zero;
    uint64 x[r];
    int i;

    };


}


/* end of file */



