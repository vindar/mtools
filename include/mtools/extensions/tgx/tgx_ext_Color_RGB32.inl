/** @file tgx_ext_Color_RGB32.inl */
//
// Copyright 2022 Arvind Singh
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



// **** WE ARE INSIDE THE tgx::RGB32 class ****


    /**
     * serialise/deserialize the object.
     **/
    template<typename U> void serialize(U& Archive, const int version = 0)
        {
        Archive & R & G & B & A;
        }


    /**
     * Print info about the RGB32 color object
     **/
    std::string toString() const
        {
        mtools::OSS os;
        os << "tgx::RGB32(" << R << " , " << G << " , " << B << " , " << A << ")";
        return os.str();
        }


    /**
    * Conversion to a mtools::RGBc
    **/
    explicit operator mtools::RGBc() const
        { 
        return mtools::RGBc(R, G, B, A); 
        }


    /**
    * ctor from a mtools::RGBc
    **/
    RGB32(const mtools::RGBc col) : RGB32((int)col.comp.R, (int)col.comp.G, (int)col.comp.B, (int)col.comp.A)
        {
        }


/* end of file */

