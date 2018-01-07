/** @file simpleBMP.cpp */
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


#include "misc/error.hpp"
#include "graphics/simpleBMP.hpp"

namespace mtools
{

#if defined (_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4996)
#endif




    SimpleBMP::SimpleBMP(const std::string & filename, uint32 lx, uint32 ly)
        : LX(lx), LXdone(0), LY(ly), LYdone(0)
        {
        MTOOLS_ASSERT((lx > 0) && (ly > 0));
        nbo = 3 * lx; if ((nbo % 4) != 0) { nbo += (4 - (nbo % 4)); }
        char header[54];
        header[0] = 'B'; header[1] = 'M';
        *((uint32 *)(header + 2)) = (nbo*ly) + 54; *((uint32 *)(header + 6)) = 0; *((uint32 *)(header + 10)) = 54; *((uint32 *)(header + 14)) = 40; *((uint32 *)(header + 18)) = lx; *((uint32 *)(header + 22)) = ly;
        header[26] = 1; header[27] = 0; header[28] = 24; header[29] = 0;
        *((uint32 *)(header + 30)) = 0; *((uint32 *)(header + 34)) = (nbo*ly) + 54; *((uint32 *)(header + 38)) = 10000; *((uint32 *)(header + 42)) = 10000; *((uint32 *)(header + 46)) = 0; *((uint32 *)(header + 50)) = 0;
        handle = fopen(filename.c_str(), "wb");
        if (handle == NULL) { MTOOLS_ERROR("ImageSave::ImageSave(), cannot create file"); }
        if (fwrite(header, 1, 54, handle) != 54) { MTOOLS_ERROR("ImageSave::ImageSave(), write header to file"); }
        bufline = new unsigned char[nbo];
        memset(bufline, 0, nbo);
        }


    SimpleBMP::~SimpleBMP() { MTOOLS_ASSERT(bufline == NULL); }


    /**
     * Add the color of the next point. The order of the points are: (0,0) (1,0) ... (lx-1,0) (0,1)
     * (1,1) ... (lx-1,1) (0,2) ... (lx-1,ly-1) where (0,0) is the bottom left corner and (lx-1,ly-
     * 1) is the upper right corner.
     *
     * @param   coul    The color to add.
     **/
    void SimpleBMP::Add(const mtools::RGBc & coul)
    {
        if (bufline == NULL) { MTOOLS_ERROR("ImageSave::Add(), too many points for file"); }
        bufline[3 * LXdone] = coul.comp.B; bufline[3 * LXdone + 1] = coul.comp.G; bufline[3 * LXdone + 2] = coul.comp.R;
        LXdone++;
        if (LXdone == LX)
        {
            if (fwrite(bufline, 1, nbo, handle) != nbo) { MTOOLS_ERROR("ImageSave::Add(), error writing line in file"); }
            LXdone = 0; LYdone++;
            if (LYdone == LY) { fclose(handle); delete[] bufline; bufline = NULL; }
        }
    }

#if defined (_MSC_VER)
#pragma warning (pop)
#endif

}


/* end of file */



