/** @file interpolation.cpp */
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


#include "stdafx_mtools.h"


#include "graphics/interpolation.hpp"

namespace mtools
{


    double linearInterpolation(double x, fVec2 P1, fVec2 P2)
        {
        if (std::isnan(P1.X()) || std::isnan(P1.Y()))  return mtools::NaN;
        if (std::isnan(P2.X()) || std::isnan(P2.Y()))  return mtools::NaN;
        if (P2.X() < P1.X()) { fVec2 temp = P1; P1 = P2; P2 = temp; }
        double delta = P2.X() - P1.X();
        if (delta <= 0.0) 
            {
            if (x < P1.X()) return P1.Y();
            if (x > P2.X()) return P2.Y();
            return (P1.Y() + P2.Y()) / 2; 
            }
        if ((P1.Y() > DBL_MAX) && (P2.Y() < -DBL_MAX))  return mtools::NaN;
        if ((P2.Y() > DBL_MAX) && (P1.Y() < -DBL_MAX))  return mtools::NaN;
        if ((P1.Y() > DBL_MAX) || (P2.Y() > DBL_MAX))   return mtools::INF; 
        if ((P1.Y() < -DBL_MAX) || (P2.Y() < -DBL_MAX)) return -mtools::INF;
        return P1.Y() + ((x - P1.X())/delta)*(P2.Y() - P1.Y());
        }


    double linearInterpolation(double x, const std::map<double, double> & map)
        {
        auto it = map.lower_bound(x);
        if (it == map.end()) return mtools::NaN;
        fVec2 P2(it->first,it->second);
        if (P2.X() == x) return P2.Y();
        if (it == map.begin()) return mtools::NaN;
        it--;
        fVec2 P1(it->first, it->second);
        return linearInterpolation(x, P1, P2);
        }



    namespace internals_graphics
    {

        inline double _hermiteSplineH00(const double x)
            {
            return (1 + 2 * x)*(1 - x)*(1 - x);
            }

        inline double _hermiteSplineH10(const double x)
            {
            return x*(1 - x)*(1 - x);
            }

        inline double _hermiteSplineH01(const double x)
            {
            return x*x*(3 - 2 * x);
            }

        inline double _hermiteSplineH11(const double x)
            {
            return x*x*(x - 1);
            }

        inline double _cubicInterpolation(const double x, const fVec2 P1, const fVec2 P2, const double m1, const double m2)
           {
            const double h = P2.X() - P1.X();
            const double t = (x - P1.X()) / h;
            return P1.Y()*_hermiteSplineH00(t) + h*m1*_hermiteSplineH10(t) + P2.Y()*_hermiteSplineH01(t) + h*m2*_hermiteSplineH11(t);
            }

    }



    double cubicInterpolation(double x, fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3)
        {
        if (std::isnan(P1.X()) || std::isnan(P1.Y()))  return mtools::NaN;
        if (std::isnan(P2.X()) || std::isnan(P2.Y()))  return mtools::NaN;
        if (std::isnan(P0.X()))  return mtools::NaN;
        if (std::isnan(P3.X()))  return mtools::NaN;

        const double D1 = (P2.Y() - P1.Y()) / (P2.X() - P1.X());
        const double D0 = (std::isnan(P0.Y())) ? D1 : (P1.Y() - P0.Y()) / (P1.X() - P0.X());
        const double D2 = (std::isnan(P3.Y())) ? D1 : (P3.Y() - P2.Y()) / (P3.X() - P2.X());

        const double m1 = (D0 + D1) / 2;
        const double m2 = (D1 + D2) / 2;

        return internals_graphics::_cubicInterpolation(x, P1, P2, m1, m2);
        }


    double cubicInterpolation(double x, const std::map<double, double> & fmap)
        {
        auto it = fmap.lower_bound(x);
        if (it == fmap.end()) return mtools::NaN;
        fVec2 P2(it->first, it->second); 
        if (P2.X() == x) return P2.Y();
        fVec2 P3(mtools::NaN,mtools::NaN); 
        auto pit = it; pit++;
        if (pit != fmap.end()) { P3.X() = pit->first; P3.Y() = pit->second; }
        if (it == fmap.begin()) return mtools::NaN;
        it--;
        fVec2 P1(it->first, it->second);
        fVec2 P0(mtools::NaN, mtools::NaN);
        if (it != fmap.begin()) { it--; P0.X() = it->first; P0.Y() = it->second; }
        return cubicInterpolation(x,P0, P1, P2, P3);
        }


    double monotoneCubicInterpolation(double x, fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3)
    {
        if (std::isnan(P1.X()) || std::isnan(P1.Y()))  return mtools::NaN;
        if (std::isnan(P2.X()) || std::isnan(P2.Y()))  return mtools::NaN;
        if (std::isnan(P0.X()))  return mtools::NaN;
        if (std::isnan(P3.X()))  return mtools::NaN;

        const double D1 = (P2.Y() - P1.Y()) / (P2.X() - P1.X());
        const double D0 = (std::isnan(P0.Y())) ? D1 : (P1.Y() - P0.Y()) / (P1.X() - P0.X());
        const double D2 = (std::isnan(P3.Y())) ? D1 : (P3.Y() - P2.Y()) / (P3.X() - P2.X());

        double m1 = (D0 + D1) / 2;
        double m2 = (D1 + D2) / 2;

        if ((D0 == 0.0) || (D1 == 0.0)) { m1 = 0.0; } 
        else
            {
            double a1 = (D1 != 0) ? (m1 / D1) : -1.0;
            if (a1 <= 0.0) { m1 = 0.0; } else { if (a1 > 3.0) { m1 = 3.0 * D1; } }
            double b0 = (D0 != 0) ? (m1 / D0) : -1.0;
            if (b0 <= 0.0) { m1 = 0.0; } else { if (b0 > 3.0) { m1 = 3.0 * D0; } }
            }

        if ((D1 == 0.0) || (D2 == 0.0)) { m2 = 0.0; }
        else
            {
            double a2 = (D2 != 0) ? (m2 / D2) : -1.0;
            if (a2 <= 0.0) { m2 = 0.0; } else { if (a2 > 3.0) { m2 = 3.0 * D2; } }
            double b1 = (D1 != 0) ? (m2 / D1) : -1.0;
            if (b1 <= 0.0) { m2 = 0.0; } else { if (b1 > 3.0) { m2 = 3.0 * D1; } }
            }

        return internals_graphics::_cubicInterpolation(x, P1, P2, m1, m2);
    }


    double monotoneCubicInterpolation(double x, const std::map<double, double> & fmap)
        {
        auto it = fmap.lower_bound(x);
        if (it == fmap.end()) return mtools::NaN;
        fVec2 P2(it->first, it->second);
        if (P2.X() == x) return P2.Y();
        fVec2 P3(mtools::NaN, mtools::NaN);
        auto pit = it; pit++;
        if (pit != fmap.end()) { P3.X() = pit->first; P3.Y() = pit->second; }
        if (it == fmap.begin()) return mtools::NaN;
        it--;
        fVec2 P1(it->first, it->second);
        fVec2 P0(mtools::NaN, mtools::NaN);
        if (it != fmap.begin()) { it--; P0.X() = it->first; P0.Y() = it->second; }
        return monotoneCubicInterpolation(x, P0, P1, P2, P3);
        }



}

/* end of file */

