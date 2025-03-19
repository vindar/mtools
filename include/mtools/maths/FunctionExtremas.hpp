/** @file FunctionExtremas.hpp */
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

#include "../misc/internal/mtools_export.hpp"
#include "../misc/misc.hpp" 
#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"


namespace mtools
	{



    /**
     * Estimate the maximum of a d-dimensional function f inside a box of R^d using a grid of mesh_points^d points.
     *
     * @tparam  D       dimension of the space
     * @param           f                  the function with signature fVec<D> -> double.
     * @param [in,out]  boundary           The boundary.
     * @param           mesh_points        The number of sampling pointfor each directions (total sampled points is mesh^D).
     *
     * @returns A double.
     */
    template<int D, typename FUN> double maxFunctionValue(FUN& f, const fBox<D>& boundary, size_t mesh_points)
        {
        size_t tot = 1; // total number of points to sample
        for (int i = 0; i < D; i++) { tot *= mesh_points; }
        iVec<D> I; // index of the current point
        for (int i = 0; i < D; i++) { I[i] = 0; }
        double maxv = -mtools::INF; // maximum value found
        for (size_t n = 0; n < tot; n++)
            {
            fVec<D> P; // current point
            for (int d = 0; d < D; d++)
                { // compute the point P according to the current index I
                P[d] = boundary.min[d] + (boundary.max[d] - boundary.min[d]) * (((double)I[d]) / (mesh_points - 1));
                }
            const double v = f(P);
            if (v > maxv) { maxv = v; }

            for (int d = 0; d < D; d++)
                { //compute the next index I
                I[d]++;
                if (I[d] < (int64_t)mesh_points) { break; }
                I[d] = 0;
                }
            }
        return maxv;
        }



	}


/** end of file FunctionExtremas.hpp */

