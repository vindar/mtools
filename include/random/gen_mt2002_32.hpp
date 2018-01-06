/** @file gen_mt2002_32.hpp */
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

#include "../misc/internal/mtools_export.hpp"
#include "../misc/misc.hpp" 
#include "../misc/timefct.hpp"


namespace mtools
{

    /**
     * Mersenne twister by Makoto Matsumoto et Takuji Nishimura.
     * Version 2002 (with 2004 correction), 32bit.
    **/
    class MT2002_32
    {

    public:
        
        /* type of integer returned by the generator */
        typedef uint32 result_type;

        
        /* min value */
        static constexpr result_type min() { return 0; }


        /* max value */
        static constexpr result_type max() { return 4294967295UL; }


        /* return a random number */
        inline uint32 operator()() {return randproc(); }


        /* discard results */
        void discard(unsigned long long z) { for(unsigned long long i = 0; i < z; i++) operator()(); }


        /* change the seed */
        void seed(result_type s) { mti = N + 1; init_genrand(s); }


        /**
        * Default constructor. Init with a unique random seed.
        **/
        MT2002_32() {seed((int32)randomID());}


        /**
        * Constructor with a given seed
        **/
        MT2002_32(result_type s) { seed(s); }


        /**
        * Constructor with a given seed aray
        *
        * @param   seed_tab    The seed array.
        * @param   tab_length  Length of the array.
        **/
        MT2002_32(result_type seed_tab[], int tab_length) {mti = N+1; init_by_array(seed_tab,tab_length);}



    private:

        /* initializes mt[N] with a seed */
        void init_genrand(uint32 s)
            {
            mt[0]= s & 0xffffffffUL;
            for (mti=1; mti<N; mti++) {mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);  mt[mti] &= 0xffffffffUL;}
            }

        /*  initialize by an array with array-length
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


