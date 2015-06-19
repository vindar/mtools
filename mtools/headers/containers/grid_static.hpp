/** @file grid_static.hpp */
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



namespace mtools
{


    /**
     * An D-dimensional grid Z^d where each site contain an object of type T.
     *
     * Version with factorization and static initialization: there are "special object" which are
     * unique. The object T() obtain by default construction is special and, initially, the grid is
     * such that every site point to this special default constructed object.
     *
     * By comparison with Grid_basic and Grid_factor, reading the object at a site does not
     * construct it since it return a reference to the default constructed special object T(). Only
     * sites which are modified are really constructed.
     *
     * GUARANTEE: every object which is NOT a special object is unique and is never deleted, moved
     * around or copied during the whole life of the grid. Thus, pointer to non-special elements are
     * never invalidated. On the other hand, special objects may be created, deleted and multiple
     * sites with the same special objet will point to the same reference. Every destructor is
     * called when the object is detroyed or when reset() is called.
     *
     * The objet T must satisfy the following properties.
     *
     * | Requir.  | Property.
     * |----------|--------------------------------------------------------------------
     * | REQUIRED | Constructor `T()` which must return the same equivalent object at  each call. [If `T(const Pos &)` also exist, it is ignored]  it is used instead of `T()`.
     * | REQUIRED | Copy constructor `T(const T&)`. [The assignement `operator=()` is not needed].
     * | OPTIONAL | Comparison `operator==()`. If T is comparable with `==`, then this operator is used for testing whether an object is special. Otherwise, basic memcmp() comparison is used.
     * | OPTIONAL | Serialization via `serialize(OArchive &)` and `deserialize(IArchive &)` or constructor `T(IArchive &)`. Otherwise, basic memcpy is used when saving to/loading from file.
     *
     *
     * @tparam  D   Dimension of the grid (e.g. Z^D).
     *
     * @tparam  T   Type of objects stored in the sites of the grid. Can be any type satisfying the requirement above.
     *
     * @tparam  R   Radius of an elementary box i.e. each elementary box contain (2R+1)^D objects.
     *              | Dimension | default value for R
     *              |-----------|-------------------
     *              | 1         | 10000
     *              | 2         | 100
     *              | 3         | 20
     *              | 4         | 6
     *              | >= 5      | 1
     *
     * @sa Grid_basic, Grid_factor
     **/
     /*
    template<size_t D, typename T, size_t R = internals_grid::defRadius<D>::val > class Grid_static
    {
    // TODDO
    };
*/



}


/* end of file */

