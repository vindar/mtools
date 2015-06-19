/** @file simpleBMP.hpp */
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

#include "misc/misc.hpp"
#include "misc/error.hpp"
#include "rgbc.hpp"

#include <string>
#include <cstdio>

namespace mtools
{
    

#if defined (_MSC_VER) 
#pragma warning (push)
#pragma warning (disable:4996)
#endif



    /**
     * A very simple class for writing BMP image file. (32bit color, no compression).
     * 
     * Call the Add() method lx*ly times (where lx,ly are the dimension of the image). After the last
     * call to Add() the image is saved and the object becomes useless. the point are added in the
     * order : (0,0) (1,0) ... (lx-1,0) (0,1) (1,1) ... (lx-1,1) (0,2) ... (lx-1,ly-1)
     * (0,0) is the bottom left corner and (lx-1,ly-1) the upper right corner.
     **/
    class SimpleBMP
        {
        public:

            /**
             * Constructor. Create the image file. 
             *
             * @param   filename    name of the BMP file to create.
             * @param   lx          width of the image.
             * @param   ly          height of the image.
             **/
            SimpleBMP(const std::string & filename, uint32 lx, uint32 ly);


            /**
             * Destructor.
             **/
            ~SimpleBMP(); 


            /**
             * Add the color of the next point. The order of the points are: (0,0) (1,0) ... (lx-1,0) (0,1)
             * (1,1) ... (lx-1,1) (0,2) ... (lx-1,ly-1) where (0,0) is the bottom left corner and (lx-1,ly-
             * 1) is the upper right corner.
             *
             * @param   coul    The color to add.
             **/
            void Add(const mtools::RGBc & coul);
              

        private:

            SimpleBMP() = delete;
            SimpleBMP(const SimpleBMP & ) = delete;            
            SimpleBMP & operator=(const SimpleBMP &) = delete;


            FILE *  handle;
            uint32   nbo;
            unsigned char * bufline;
            uint32 LX, LXdone;
            uint32 LY, LYdone;

        };

#if defined (_MSC_VER) 
#pragma warning (pop)
#endif

}


/* end of file */

