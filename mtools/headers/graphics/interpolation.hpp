/** @file interpolation.hpp */
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
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/error.hpp"


namespace mtools
{

    /**
     * Linear interpolation. Deals with infinity and nans.
     *
     * @param   x   The x coordinate.
     * @param   P1  The first point (x1,y1).
     * @param   P2  The second point (x2,y2).
     *
     * @return  the interpolated value at x. return y1 or y2 if outside of the boundaries.
     **/
    double linearInterpolation(double x, fVec2 P1, fVec2 P2);


    /**
     * Linear interpolation from a map.
     *
     * @param   x   The x coordinate.
     * @param   map The map containing the points.
     *
     * @return  the interpolated value at x. Return Nan if outside or the map is empty.
     **/
    double linearInterpolation(double x, const std::map<double, double> & map);


    /**
     * Compute the cubic inteprolation. The points must be in increasing X order. Takes care of NaNs
     * and infinities.
     *
     * @param   x   The x coordinate to compute the value (between P1.X and P2.X)
     * @param   P0  The "external" left point.
     * @param   P1  The left boundary point.
     * @param   P2  The right boundary point.
     * @param   P3  The "external" right point.
     *
     * @return  the cubic interpolated value.
     **/
    double cubicInterpolation(double x, fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3);


    /**
    * Cubic interpolation from a map.
    *
    * @param   x   The x coordinate.
    * @param   map The map containing the points.
    *
    * @return  the interpolated value at x. Return Nan if outside or the map is empty.
    **/
    double cubicInterpolation(double x, const std::map<double, double> & map);


    /**
     * Compute the monotone cubic inteprolation. The points must be in increasing X order. Takes
     * care of NaNs and infinities.
     *
     * @param   x   The x coordinate to compute the value (between P1.X and P2.X)
     * @param   P0  The "external" left point.
     * @param   P1  The left boundary point.
     * @param   P2  The right boundary point.
     * @param   P3  The "external" right point.
     *
     * @return  the cubic interpolated value.
     **/
    double monotoneCubicInterpolation(double x, fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3);

    
    /**
    * Monotone cubic interpolation from a map.
    *
    * @param   x   The x coordinate.
    * @param   map The map containing the points.
    *
    * @return  the interpolated value at x. Return Nan if outside or the map is empty.
    **/
    double monotoneCubicInterpolation(double x, const std::map<double, double> & map);





}


/* end of file */