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
     * Print info about the Box3 object
     **/
    std::string toString() const
        {
        mtools::OSS os;
        os << "tgx::Box3<" << typeid(T).name() << "> [" << minX << "," << maxX << "] x [" << minY << "," << maxY << "] x [" << minZ << "," << maxZ << "]" << (isEmpty() ? " (empty) " : "");
        return os.str();
        }




/* end of file */

