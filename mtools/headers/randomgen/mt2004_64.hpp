/** @file mt2004_64.hpp */
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
//Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
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


namespace mtools
{


    /**
    * Mersenne twister by Makoto Matsumoto et Takuji Nishimura.
    * Version 2004 (with 2008 correction), 64bit.
    **/
    class MT2004_64
    {


    public:


        /* type of integer returned by the generator */
        typedef uint64 result_type;


        /* min value */
        static constexpr result_type min() { return 0; }


        /* max value */
        static constexpr result_type max() { return 18446744073709551615; }


        /* return a random number */
        inline uint64 operator()() { return randproc64(); }


        /* discard results */
        void discard(unsigned long long z) { for (unsigned long long i = 0; i < z; i++) operator()(); }


        /* change the seed */
        void seed(result_type s) { mti = NN + 1; init_genrand64(s); }


        /**
        * Default constructor. Init with a seed obtained from the internal clock.
        **/
        MT2004_64() { seed(time(NULL)); }


        /**
        * Constructor with a given seed
        **/
        MT2004_64(result_type s) { seed(s); }


        /**
        * Constructor with a given seed aray
        *
        * @param   seed_tab    The seed array.
        * @param   tab_length  Length of the array.
        **/
        MT2004_64(result_type seed_tab[], int tab_length) { mti = NN + 1; init_by_array64(seed_tab, tab_length); }


    private:


    /* initializes mt[NN] with a seed */
    void init_genrand64(unsigned long long seed)
        {
        mt[0] = seed;
        for (mti=1; mti<NN; mti++) mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
        }


    /* initialize by an array with array-length */
    /* init_key is the array for initializing keys */
    /* key_length is its length */
    void init_by_array64(uint64 init_key[], unsigned long long key_length)
    {
        uint64 i, j, k;
        init_genrand64(19650218ULL);
        i=1; j=0;
        k = (((uint64)NN)>key_length ? NN : key_length);
        for (; k; k--)
            {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 3935559000370003845ULL))+ init_key[j] + j;
            i++; j++;
            if (i>=((uint64)NN)) { mt[0] = mt[NN-1]; i=1; }
            if (j>=key_length) j=0;
            }
        for (k=NN-1; k; k--)
            {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 2862933555777941757ULL)) - i;
            i++;
            if (i>=((uint64)NN)) { mt[0] = mt[NN-1]; i=1; }
            }
        mt[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */
    }


    /* generates a random number on [0, 2^64-1]-interval */
    inline uint64 randproc64(void)
    {
        int i;
        uint64 x;
        const static uint64 mag01[2]={0ULL, MATRIX_A};
        if (mti >= NN)
            {
            for (i=0;i<NN-MM;i++) {x = (mt[i]&UM)|(mt[i+1]&LM); mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];}
            for (;i<NN-1;i++) { x = (mt[i]&UM)|(mt[i+1]&LM); mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];}
            x = (mt[NN-1]&UM)|(mt[0]&LM);
            mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
            mti = 0;
            }
        x = mt[mti++];
        x ^= (x >> 29) & 0x5555555555555555ULL;
        x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
        x ^= (x << 37) & 0xFFF7EEE000000000ULL;
        x ^= (x >> 43);
        return x;
    }


    /* some constants */
    static const int NN = 312;
    static const int MM = 156;
    static const uint64 MATRIX_A = 0xB5026F5AA96619E9ULL;
    static const uint64 UM = 0xFFFFFFFF80000000ULL;
    static const uint64 LM = 0x7FFFFFFFULL;

    /* state of the generator */
    uint64 mt[NN];
    int mti;
    };

}


/* end of file */


