/** @file tgx_ext_Box3.inl */
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



// **** WE ARE INSIDE THE tgx:Box3 class ****



    /**
     * serialise/deserialize the object.
     **/
    template<typename U> void serialize(U& Archive, const int version = 0)
        {
        Archive & minX & minY & minZ & maxX & maxY & maxZ;
        }


    /**
     * Print info about the Box3 object
     **/
    std::string toString() const
        {
        mtools::OSS os;
        os << "tgx::Box3<" << typeid(T).name() << "> [" << minX << "," << maxX << "] x [" << minY << "," << maxY << "] x [" << minZ << "," << maxZ << "]" << (isEmpty() ? " (empty) " : "");
        return os.str();
        }


    /**
    * (explicit) Conversion to mtools::Box3<U> 
    **/
    template<typename U>
    explicit operator mtools::Box<U, 3>() const
        {
        return mtools::Box<U, 3>((U)minX, (U)maxX, (U)minY, (U)maxY, (U)minZ, (U)maxZ);
        }


    /**
    * ctor from a mtools::Box<U,3>
    **/
    template<typename U>
    Box3(const mtools::Box<U, 3> & B) : minX((T)B.min[0]), maxX((T)B.max[0]), minY((T)B.min[1]), maxY((T)B.max[1]), minZ((T)B.min[2]), maxZ((T)B.max[2])
        {
        }




/* end of file */

