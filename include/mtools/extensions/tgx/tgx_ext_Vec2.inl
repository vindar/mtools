/** @file tgx_ext_Color_Vec2.inl */
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



// **** WE ARE INSIDE THE tgx::Vec2 class ****

    /**
     * serialise/deserialize the object.
     **/
    template<typename U> void serialize(U& Archive, const int version = 0)
        {
        Archive & x & y;
        }


    /**
     * Print info about the Vec2 object
     **/
    std::string toString() const
        {
        mtools::OSS os;
        os << "tgx::Vec2<" << typeid(T).name() <<  ">(" << x << " , " << y << ")";
        return os.str();
        }


    /**
    * Explicit conversion to a mtools::Vec<U,2>
    **/
    template<typename U>
    explicit operator mtools::Vec<U,2>() 
        { 
        return mtools::Vec<U, 2>((U)x, (U)y); 
        }


    /**
    * ctor from a mtools::Vec<U,2>
    **/
    template<typename U>
    Vec2(const mtools::Vec<U, 2> & V) : x((T)V.X()), y((T)V.Y())
        {
        }


/* end of file */

