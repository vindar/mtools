/** @file tgx_ext_Color_HSV.inl */
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



// **** WE ARE INSIDE THE tgx::HSV class ****



    /**
     * serialise/deserialize the object.
     **/
    template<typename U> void serialize(U& Archive, const int version = 0)
        {
        Archive & H & S & V;
        }


    /**
     * Print info about the HSV color object
     **/
    std::string toString() const
        {
        mtools::OSS os;
        os << "tgx::HSV(" << H << " , " << S << " , " << V << ")";
        return os.str();
        }


    /**
    * Conversion to a mtools::RGBc
    **/
    explicit operator mtools::RGBc() const
        { 
        tgx::RGB24 c(*this);
        return mtools::RGBc(c.R, c.G, c.B); 
        }


    /**
    * ctor from a mtools::RGBc
    **/
    HSV(const mtools::RGBc col) : HSV(tgx::RGB24((int)col.comp.R, (int)col.comp.G, (int)col.comp.B))
        {
        }



/* end of file */

