/** @file mt2002_32.hpp */
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
//
// 
// Licence from the original mersenne twister source file upon
// which this file is based. 
// 
//Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//1. Redistributions of source code must retain the above copyright
//notice, this list of conditions and the following disclaimer.
//
//2. Redistributions in binary form must reproduce the above copyright
//notice, this list of conditions and the following disclaimer in the
//documentation and/or other materials provided with the distribution.
//
//3. The names of its contributors may not be used to endorse or promote
//products derived from this software without specific prior written
//permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#pragma once



#include "../misc/misc.hpp" 
#include <ctime>
#include <string>

namespace mtools
{

    /**
     * Mersenne twister by Makoto Matsumoto et Takuji Nishimura.
     * Version 2002 (with 2004 correction), 32bit.
    **/
    class MT2002_32
    {
    public:


    /**
     * Default constructor. Init with a seed obtained from the internal clock.
    **/
    MT2002_32() {mti = N+1; init_genrand(randtime32());}


    /**
     * Constructor with a given seed
     *
     * @param   seed    The seed.
    **/
    MT2002_32(uint32 seed) {mti = N+1; init_genrand(seed);}


    /**
     * Constructor with a given seed aray
     *
     * @param   seed_tab    The seed array.
     * @param   tab_length  Length of the array.
    **/
    MT2002_32(uint32 seed_tab[], int tab_length) {mti = N+1; init_by_array(seed_tab,tab_length);}


    /**
    * generates a random number on [0,1)-real-interval. Same as rand_double0()
    **/
    inline double operator()() { return rand_double0(); }


    /**
     * generates a random number on [0,0xffffffff]-interval
    **/
    inline uint32 rand_uint32(void) {return randproc();}


    /** 
    *generates a random number on [0, 2^64-1]-interval 
    **/
    inline uint64 rand_uint64(void) {uint64 a = randproc(); a+=  ( ((uint64)randproc()) << 32); return a;}
     

    /**  
    * generates a random number on [0,1]-real-interval 
    **/
    inline double rand_double01(void) {return (rand_uint64() >> 11) * (1.0/9007199254740991.0);}


    /** 
    * generates a random number on [0,1)-real-interval 
    **/
    inline double rand_double0(void) {return (rand_uint64() >> 11) * (1.0/9007199254740992.0);}


    /**  
    * generates a random number on (0,1)-real-interval 
    **/
    inline double rand_double(void) {return ((rand_uint64() >> 12) + 0.5) * (1.0/4503599627370496.0);}


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
        s += "Testing the implementation of MT2002_32\n";
        s += "Mersenne twister by Matsumoto/Nishimura\n";
        s += "version 2002 (with 2004 corrections)\n";
        s += "this is the original 32 bits version.\n\n";
        uint32 init[4]={0x123, 0x234, 0x345, 0x456};
        MT2002_32 gen(init,4);
        uint32 r = gen.rand_uint32();
        s += "Generated [" + std::to_string(r) + "] should be [1067595299]\n";
        for (int i=0; i<998; i++) {gen.rand_uint32();}
        r = gen.rand_uint32();
        s += "Generated [" + std::to_string(r) + "] should be [3460025646]\n";
        double f = gen.rand_double0();
        s+= "Generated [" + std::to_string((uint32)(f*1000000000)) + "] should be [990006440]\n";
        for (int i=0; i<998; i++) {gen.rand_double0();}
        f = gen.rand_double01();
        s +=  "Generated [" + std::to_string( (uint32)(f * 1000000000)) + "] should be [736128311]\n";
        s+= "\nend of test.\n";
        s+= "-----------------------------------------------\n";
        return s;
        }


    private:

    /* initializes mt[N] with a seed */
    void init_genrand(uint32 s)
        {
        mt[0]= s & 0xffffffffUL;
        for (mti=1; mti<N; mti++) {mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);  mt[mti] &= 0xffffffffUL;}
        }

    /* initialize by an array with array-length
       init_key is the array for initializing keys 
       key_length is its length
       slight change for C++, 2004/2/26 */
    void init_by_array(uint32 init_key[], int key_length)
        {
        int i, j, k;
        init_genrand(19650218UL);
        i=1; j=0;
        k = (N>key_length ? N : key_length);
        for (; k; k--)
            {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL)) + init_key[j] + j;
            mt[i] &= 0xffffffffUL;
            i++; j++;
            if (i>=N) { mt[0] = mt[N-1]; i=1;}
            if (j>=key_length) j=0;
            }
        for (k=N-1; k; k--)
            {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL)) - i;
            mt[i] &= 0xffffffffUL;
            i++;
            if (i>=N) { mt[0] = mt[N-1]; i=1; }
            }
        mt[0] = 0x80000000UL;
        }

    inline uint32 randproc(void)
        {
        uint32 y;
        const static uint32 mag01[2]={0x0UL, MATRIX_A};
        if (mti >= N)
            {
            int kk;
            for (kk=0;kk<N-M;kk++)
                {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
            for (;kk<N-1;kk++)
                {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
            y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
            mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
            mti = 0;
            }
        y = mt[mti++];
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680UL;
        y ^= (y << 15) & 0xefc60000UL;
        y ^= (y >> 18);
        return y;
        }

    uint32 randtime32()
        {
        time_t a = time(NULL);
        if (sizeof(time_t)==4) {return((uint32)a);}
        uint32 r = (uint32)(((uint64)a) >> 32);
        r+= a & 0xffffffffULL;
        return r;
        }


    /* some constants */
    static const int N = 624;
    static const int M = 397;
    static const uint32 MATRIX_A = 0x9908b0dfUL;
    static const uint32 UPPER_MASK = 0x80000000UL;
    static const uint32 LOWER_MASK = 0x7fffffffUL;

    /* state of the generator */
    uint32 mt[N]; /* the array for the state vector  */
    int mti; /* mti==N+1 means mt[N] is not initialized */

    };


}


/* end of file  */


