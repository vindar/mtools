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



    /** Used by argMaxFunction()
        return the position of the maximum of a function inside a given box discretized in a mesh with mesh_points in each direction */
    template<int D, typename FUN> fVec<D> _argMaxFunction(FUN& f, const fBox<D>& boundary, size_t mesh_points)
        {
        size_t tot = 1; // total number of points to sample
        for (int i = 0; i < D; i++) { tot *= mesh_points; }
        iVec<D> I; // index of the current point
        for (int i = 0; i < D; i++) { I[i] = 0; }
        double maxv = -mtools::INF; // maximum value found
        fVec<D> maxP; // position of the maximum
        for (size_t n = 0; n < tot; n++)
            {
            fVec<D> P; // current point
            for (int d = 0; d < D; d++)
                { // compute the point P according to the current index I
                P[d] = boundary.min[d] + (boundary.max[d] - boundary.min[d]) * (((double)I[d]) / (mesh_points - 1));
                }
            const double v = f(P);
            if (!std::isnan(v))
                {
                if (v > maxv) { maxv = v; maxP = P; }
                }
            for (int d = 0; d < D; d++)
                { //compute the next index I
                I[d]++;
                if (I[d] < (int64_t)mesh_points) { break; }
                I[d] = 0;
                }
            }
        return maxP; 
        }



    /**
     * Estimate the position of the maximum of a d-dimensional function f inside a box of R^d.
     *
     * @param [in,out]  f           the function with signature fVec<D> -> double.
     * @param           boundary    The boundary box
     * @param           mesh_points number of sampling point in each direction (0 for automatic choice)
     *
     * @returns the position of the largest value found
     */
    template<int D, typename FUN> fVec<D> argMaxFunction(FUN& f, fBox<D> boundary, size_t mesh_points = 0)
        {
        if (mesh_points == 0) 
            {
            switch (D)
                {
                case 1: mesh_points = 1001; break;
                default: mesh_points = 101; break;
                }
            }
        mesh_points = std::max(mesh_points, (size_t)7); // at least 7 point in each direction
        if (mesh_points % 2 == 0) { mesh_points++; } // odd number of points
        const int max_depth = 60; // maximum depth of the search
        fVec<D> bestP = boundary.center();
        double bestv = f(bestP);
        for (int depth = 0; depth < max_depth; depth++)
            {
            fVec<D> P = _argMaxFunction(f, boundary, mesh_points); // find the maximum
            double v = f(P);
            if (v > bestv) { bestv = v; bestP = P; }
            fBox<D> B;
            int nbn = 0; 
            for (int i = 0; i < D; i++) 
                { 
                double l = (boundary.max[i] - boundary.min[i]) / 3;
                B.min[i] = P[i] - l;
                B.max[i] = P[i] + l;
                if (l <= 0) { nbn++; }
                }
            if (nbn == D) 
                { // no more space to search
                //cout << "break at depth=" << depth << "\n";
                break; 
                } 
            boundary = intersectionRect(boundary, B); // reduce the search area
            }
        //cout << "best value = " << bestv << "\n";
        //cout << "best pos = " << bestP << "\n";
        return bestP; 
        }


    /**
     * Estimate the position of the maximum of a 1-dimensional function f inside an interval [xmin, xmax]
     *
     * @param [in,out]  f           the function with signature double -> double.
     * @param           xmin        the min value of the interval
     * @oaram           xmax        the max value of the interval
     * @param           mesh_points number of sampling point (0 for automatic choice)
     *
     * @returns the position of the largest value found
     */
    template<typename FUN> double argMaxFunction_1D(FUN& f, double xmin, double xmax, size_t mesh_points = 0)
        {
        // TODO: improve algo in 1D using Newton...
        if (xmax > xmin) { std::swap(xmin, xmax); }
        fBox1 B; 
        B.min[0] = xmin;
        B.max[0] = xmax;
        return (argMaxFunction([&f](fVec1 P) {return f(P[0]); }, B, mesh_points))[0];
        }


    /**
     * Estimate the maximum of a d-dimensional function f inside a box of R^d.
     *
     * @param [in,out]  f           the function with signature fVec<D> -> double.
     * @param           boundary    The boundary box
     * @param           mesh_points number of sampling point in each direction (0 for automatic choice)
     *
     * @returns the largest value found.
     */
    template<int D, typename FUN> double maxFunction(FUN& f, fBox<D> boundary, size_t mesh_points= 0)
        {
        return f(argMaxFunction(f,boundary,mesh_points));
        }


    /**
     * Estimate the maximum of a 1-dimensional function f inside an interval [minx, maxx]
     *
     * @param [in,out]  f           the function with signature double -> double.
     * @param           xmin        the min value of the interval
     * @oaram           xmax        the max value of the interval
     * @param           mesh_points number of sampling point (0 for automatic choice)
     *
     * @returns the maximum value found.
     */
    template<typename FUN> double maxFunction_1D(FUN& f, double xmin, double xmax, size_t mesh_points = 0)
        {
        return f(argMaxFunction_1D(f, xmin, xmax, mesh_points));
        }











    /** Used by argMinFunction()
        return the position of the minimum of a function inside a given box discretized in a mesh with mesh_points in each direction */
    template<int D, typename FUN> fVec<D> _argMinFunction(FUN& f, const fBox<D>& boundary, size_t mesh_points)
        {
        size_t tot = 1; // total number of points to sample
        for (int i = 0; i < D; i++) { tot *= mesh_points; }
        iVec<D> I; // index of the current point
        for (int i = 0; i < D; i++) { I[i] = 0; }
        double minv = mtools::INF; // minimum value found
        fVec<D> minP; // position of the minimum
        for (size_t n = 0; n < tot; n++)
            {
            fVec<D> P; // current point
            for (int d = 0; d < D; d++)
                { // compute the point P according to the current index I
                P[d] = boundary.min[d] + (boundary.max[d] - boundary.min[d]) * (((double)I[d]) / (mesh_points - 1));
                }
            const double v = f(P);
            if (!std::isnan(v))
                {
                if (v < minv) { minv = v; minP = P; }
                }
            for (int d = 0; d < D; d++)
                { //compute the next index I
                I[d]++;
                if (I[d] < (int64_t)mesh_points) { break; }
                I[d] = 0;
                }
            }
        return minP;
        }



    /**
     * Estimate the position of the minimum of a d-dimensional function f inside a box of R^d.
     *
     * @param [in,out]  f           the function with signature fVec<D> -> double.
     * @param           boundary    The boundary box
     * @param           mesh_points number of sampling point in each direction (0 for automatic choice)
     *
     * @returns the position of the smallest value found
     */
    template<int D, typename FUN> fVec<D> argMinFunction(FUN& f, fBox<D> boundary, size_t mesh_points = 0)
        {
        if (mesh_points == 0)
            {
            switch (D)
                {
                case 1: mesh_points = 1001; break;
                default: mesh_points = 101; break;
                }
            }
        mesh_points = std::max(mesh_points, (size_t)7); // at least 7 point in each direction
        if (mesh_points % 2 == 0) { mesh_points++; } // odd number of points
        const int max_depth = 60; // maximum depth of the search
        fVec<D> bestP = boundary.center();
        double bestv = f(bestP);
        for (int depth = 0; depth < max_depth; depth++)
            {
            fVec<D> P = _argMinFunction(f, boundary, mesh_points); // find the minimum
            double v = f(P);
            if (v < bestv) { bestv = v; bestP = P; }
            fBox<D> B;
            for (int i = 0; i < D; i++)
                {
                double l = (boundary.max[i] - boundary.min[i]) * 2.0 / (mesh_points - 1);
                B.min[i] = P[i] - l;
                B.max[i] = P[i] + l;
                /* if (l <= 0) {
                    cout << "break at " << depth << "\n";
                    break;
                    }*/
                }            
            boundary = intersectionRect(boundary, B); // reduce the search area            
            if (boundary.area() <= 0) {
                //cout << "break volume at " << depth << "\n";
                break;
                }

            }
        return bestP;
        }


    /**
     * Estimate the position of the maximum of a 1-dimensional function f inside an interval [xmin, xmax]
     *
     * @param [in,out]  f           the function with signature double -> double.
     * @param           xmin        the min value of the interval
     * @oaram           xmax        the max value of the interval
     * @param           mesh_points number of sampling point (0 for automatic choice)
     *
     * @returns the position of the smallest value found
     */
    template<typename FUN> double argMinFunction_1D(FUN& f, double xmin, double xmax, size_t mesh_points = 0)
        {
        // TODO: improve algo in 1D using Newton...
        if (xmax > xmin) { std::swap(xmin, xmax); }
        fBox1 B; 
        B.min[0] = xmin;
        B.max[0] = xmax;
        return (argMinFunction([&f](fVec1 P) {return f(P[0]); }, B, mesh_points))[0];
        }


    /**
     * Estimate the minimum of a d-dimensional function f inside a box of R^d.
     *
     * @param [in,out]  f           the function with signature fVec<D> -> double.
     * @param           boundary    The boundary box
     * @param           mesh_points number of sampling point in each direction (0 for automatic choice)
     *
     * @returns the minimum value found.
     */
    template<int D, typename FUN> double minFunction(FUN& f, fBox<D> boundary, size_t mesh_points = 0)
        {
        return f(argMinFunction(f, boundary, mesh_points));
        }



    /**
     * Estimate the minimum of a 1-dimensional function f inside an interval [minx, maxx]
     *
     * @param [in,out]  f           the function with signature double -> double.
     * @param           xmin        the min value of the interval   
     * @oaram           xmax        the max value of the interval                              
     * @param           mesh_points number of sampling point (0 for automatic choice)
     *
     * @returns the minimum value found.
     */
    template<typename FUN> double minFunction_1D(FUN& f, double xmin, double xmax, size_t mesh_points = 0)
        {
        return f(argMinFunction_1D(f, xmin, xmax, mesh_points));
        }


	}


/** end of file FunctionExtremas.hpp */

