/** @file tgx_ext_Color_Mat4.inl */
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



// **** WE ARE INSIDE THE tgx::Mat4 class ****

    /**
     * serialise/deserialize the object.
     **/
    template<typename U> void serialize(U& Archive, const int version = 0)
        {
        Archive & M;
        }



    /**
     * Print info about the Vec2 object
     **/
    std::string toString() const
        {
        const size_t l = 19;
        mtools::OSS os;
        os << "tgx::Mat4<" << typeid(T).name() << ">\n";
        os << mtools::justify_left(mtools::toString(M[0]),l,' ') << " " << mtools::justify_left(mtools::toString(M[4]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[8]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[12]), l, ' ') << "\n";
        os << mtools::justify_left(mtools::toString(M[1]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[5]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[9]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[13]), l, ' ') << "\n";
        os << mtools::justify_left(mtools::toString(M[2]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[6]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[10]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[14]), l, ' ') << "\n";
        os << mtools::justify_left(mtools::toString(M[3]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[7]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[11]), l, ' ') << " " << mtools::justify_left(mtools::toString(M[15]), l, ' ') << "\n";
        return os.str();
        }



/* end of file */

