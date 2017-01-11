/** @file gen_fastRNG.hpp */
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

namespace mtools
    {

    /**
    * Very fast RNG. Should only be used for test purpose and speed when the number need not have
    * good statistical properties. The generator always has the same deterministic seed.
    **/
    class FastRNG
        {

        public:

            /* type of integer returned by the generator */
            typedef uint32 result_type;


            /* min value */
            static constexpr result_type min() { return 0; }


            /* max value */
            static constexpr result_type max() { return 4294967295UL; }


            /* return a random number */
            inline uint32 operator()() 
                {
                uint32 t;
                _gen_x ^= _gen_x << 16; _gen_x ^= _gen_x >> 5; _gen_x ^= _gen_x << 1;
                t = _gen_x; _gen_x = _gen_y; _gen_y = _gen_z; _gen_z = t ^ _gen_x ^ _gen_y;
                return _gen_z;
                }


            /* return a uniform on [0,1).  */
            inline double unif() { return ((double)(operator()())) / (4294967296.0); }


            /* discard results */
            void discard(unsigned long long z) { for (unsigned long long i = 0; i < z; i++) operator()(); }


            /**
             * Change the seed. This does nothing here (kept for compatibility purposes).
             **/
            void seed(result_type s) { }


            /**
            * Default constructor. Always initialize with the same seed.
            **/
            FastRNG() : _gen_x(123456789), _gen_y(362436069), _gen_z(521288629) { }


            /**
            * Constructor with a given seed. Same as default constructor since the seed cannot change (kept for compatibility purposes).
            **/
            FastRNG(result_type s) : _gen_x(123456789), _gen_y(362436069), _gen_z(521288629)  { }



        private:


            uint32 _gen_x, _gen_y, _gen_z;		// state of the generator


        };


    }



/* end of file */