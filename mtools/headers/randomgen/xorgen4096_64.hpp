/** @file xorgen4096_64.hpp */
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

#include "misc/misc.hpp"
#include <ctime>
#include <string>

namespace mtools
{


    /**
    * Xorgen random number generator by Richard Brent
    * 64 bit version for both integers and floating point numbers
    **/
    class XorGen4096_64
    {
    public:

    /**
    * construct the generator with random seed obtained from time function
    **/
    XorGen4096_64() {zero=0; i=-1; init_gen((uint64)time(NULL));}


    /**
    * construct the generator with a given seed
    **/
    XorGen4096_64(uint64 seed) {zero=0; i=-1; init_gen(seed);}


    /**
    * generates a random number on [0,1)-real-interval. Same as rand_double0()
    **/
    inline double operator()() { return rand_double0(); }


    /**
    * generates a random number on [0, 2^32-1]-interval
    **/
    inline uint32 rand_uint32(void) {return((uint32)(randproc64() >> 32));}


    /**
    * generates a random number on [0, 2^64-1]-interval
    **/
    inline uint64 rand_uint64(void) {return randproc64();}


    /**
    *generates a random number on [0,1]-real-interval
    **/
    inline double rand_double01(void) {return (randproc64() >> 11) * (1.0/9007199254740991.0);}


    /**
    * generates a random number on [0,1)-real-interval
    **/
    inline double rand_double0(void) {return (randproc64() >> 11) * (1.0/9007199254740992.0);}


    /**
    * generates a random number on (0,1)-real-interval
    **/
    inline double rand_double(void) {return ((randproc64() >> 12) + 0.5) * (1.0/4503599627370496.0);}


    /**
    * generate a high precision random number on [0,1) interval It means that when the return value
    * is very small (close to zero) the number of significative digit stays roughly the same
    * whereas for rand_double0 the minimal step for each value is 1.0/4503599627370496.0 even for
    * small value. useful for simulationg unbounded RV via their CDF
    *
    * In average, the generation is 1/256 slower than the classic rand_double0()
    **/
    inline double rand_double0_highprecision(void)
		{
		double b = 1.0, a = rand_double0();
		while(a * 256 < 1.0) {b /= 256; a = rand_double0();}
		return a*b;
		}


    /**
    * Test that the implementation is OK.
    **/
    static std::string test()
        {
        std::string s;
        s += "-----------------------------------------------\n";
        s += "Testing the implementation of XorGen4096_64\n";
        s += "Xor random generator by Richard Brent\n";
        s += "version 3.05.\n";
        s += "implementation of the 64 bits version.\n\n";
        XorGen4096_64 gen(1234777);
        uint64 r = gen.rand_uint64();
        s += "Generated [" + std::to_string(r) + "] should be [3381003798738941279]\n";
        for (int i=0; i<995; i++) {gen.rand_uint64();}
        r = gen.rand_uint64();
        s += "Generated [" + std::to_string(r) + "] should be [11234706451175467682]\n";
        gen.rand_uint64(); gen.rand_uint64(); gen.rand_uint64();
        double f = gen.rand_double0();;
        s += "Generated [" + std::to_string((uint64)(f * 1000000000) )+ "] should be [875227214]\n";
        for (int i=0; i<998; i++) {gen.rand_double0();}
        f = gen.rand_double01();
        s += "Generated [" + std::to_string((uint64)(f * 1000000000) )+ "] should be [648574515]\n";
        s += "\nend of test.\n";
        s += "-----------------------------------------------\n";
        return s;
        }

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



