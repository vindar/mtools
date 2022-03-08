/** @file plot2Darray.hpp */
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

#include "image.hpp"
#include "../misc/internal/mtools_export.hpp"
#include "../maths/vec.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2Dobject.hpp"
#include "internal/rangemanager.hpp"
#include "interpolation.hpp"
#include "internal/plot2Dbasegraph.hpp"


namespace mtools
{

    template<typename F> class Plot2DArray;


    /**
     * Factory function for creating Plot of C-array.  
     * 
     * @code{.cpp}
     * int tab1 = new int[1000];
     * std::array<double, 100> tab2;
     * auto PA1 = makePlot2DArray(tab1, 1000, -50, 50);        // plot from a C aray with 1000 elements and domain [-50,50] i.e. each element has lenght 1/10.
     * auto PA2 = makePlot2DArray(tab2.data(), 100,"tab2");    // here, natural range, each element has length 1, name is "tab2"
     * @endcode.
     *
     * @tparam  T   Type of the elemnt in the aray, must be convertible to double.
     * @param [in,out]  obj pointer to the array.
     * @param   len         The length of the array.
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DArray object encapsulating the array.
     **/
    template<typename T>  Plot2DArray<T>  makePlot2DArray(T  * obj, size_t len, std::string name = "Array") 
        {
        return Plot2DArray<T>(obj,len, name);
        }


    /**
     * Factory function for creating Plot of C-array.  
     * 
     * @code{.cpp}
     * int tab1 = new int[1000];
     * std::array<double, 100> tab2;
     * auto PA1 = makePlot2DArray(tab1, 1000, -50, 50);        // plot from a C aray with 1000 elements and domain [-50,50] i.e. each element has lenght 1/10.
     * auto PA2 = makePlot2DArray(tab2.data(), 100,"tab2");    // here, natural range, each element has length 1, name is "tab2"
     * @endcode.
     *
     * @tparam  T   Type of the elemnt in the aray, must be convertible to double.
     * @param [in,out]  obj pointer to the array.
     * @param   len         The length of the array.
     * @param   minDomain   The minimum of the definition domain.
     * @param   maxDomain   The maximum of the definition domain.
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DArray object encapsulating the array.
     **/
    template<typename T>  Plot2DArray<T>  makePlot2DArray(T  * obj, size_t len, double minDomain, double maxDomain, std::string name = "Array")
        {
        return Plot2DArray<T>(obj, len, minDomain, maxDomain, name);
        }




    /**
     * Plot object fixed length C-style arrays. Plot any array of type T which can be cast to a
     * double.
     * 
     * @code{.cpp}
     * int64 function1(double x) { return (int64)sqrt(x >0 ? x : -x); } //integer rounding of abs().
     * 
     * double function2(float x) { return(1 / x); } // hyperbole
     * 
     * int main()
     *    {
     *    double * tab1 = new double[1000]; for (int i = 0; i < 1000; i++) { tab1[i] = cos(sqrt(i)); } // array of double
     *    std::array<int, 100> tab2; for (int i = 0; i < 100; i++) { tab2[i] = i / 10; } // array of int, linear
     *    Plot2DArray<double> PA1(tab1, 1000,-50,50);             // plot for tab1. The range is [-50,50] ie each entry has length 1/10.
     *    Plot2DArray<int> PA2(tab2.data(), 100);                 // natural range, each element has length 1
     *    Plot2DFun<decltype(function1)> PF1(function1);          // function1. Defined on R
     *    Plot2DFun<decltype(function2)> PF2(function2,0.1,120);  // function2. Defined on [0.1,120]
     *    Plotter2D P;                                            // create a plotter object
     *    P.gridObject(true)->setUnitGrid();                      // add a unit grid to the plot
     *    P[PA1][PA2][PF1][PF2];                                  // insert everything
     *    PF1.drawDots(30);                                       // draw with dots instead of line for function 1, max quality
     *    PF1.tickness(3);                                        // and make the drawing ticker
     *    PA1.interpolationCubic();                               // use cubic interpolation for this array
     *    PA2.hypograph(true);                                    // color the hypograph of tab2
     *    PF1.epigraph(true);                                     // and the epigraph of function1
     *    P.autorangeXY(false);                                   // autorange with possible modification of the aspect ratio
     *    P.plot();                                               // make the plot
     *    P.remove(PA1);                                          // not needed but cannot hurt since we are going to delete tab1...
     *    delete [] tab1;
     *    return 0;
     *    }
     * @endcode.
     *
     * @tparam  T   Type of the elements in the array : must be convertible to double.
     **/
    template< typename T > class  Plot2DArray : public internals_graphics::Plot2DBaseGraphWithInterpolation
        {

        public:

        /**
         * Constructor. the interval [minDomain,maxDomain] is divided in len intervals of length
         * (maxDomain-minDomain)/len.
         *
         * @param [in,out]  tab Pointer to the array to plot.
         * @param   len         number of elements in the array.
         * @param   minDomain   The minimum of the definition domain.
         * @param   maxDomain   The maximum of the definition domain.
         * @param   name        The name of the plot .
         **/
        Plot2DArray(const T * tab, size_t len, double minDomain, double maxDomain, std::string name = "Array") : Plot2DBaseGraphWithInterpolation(minDomain, maxDomain, name), _tab(tab), _len(len)
            {
            }


        /**
         * Constructor. The definition domain defaults to [0,len].
         *
         * @param   tab     The function to plot, mist be callable via f(double) and return something
         *                  convertible to double.
         * @param   len     The length of the array.
         * @param   name    The name of the plot .
         **/
        Plot2DArray(const T * tab, size_t len, std::string name = "Array") : Plot2DBaseGraphWithInterpolation(0.0,(double)len,name), _tab(tab), _len(len)
            {
            }


        /**
         * Move Constructor.
         **/
        Plot2DArray(Plot2DArray && obj) : Plot2DBaseGraphWithInterpolation(std::move(obj)), _tab(obj._tab), _len(obj._len)
            {
            }


        /**
         * Destructor. Remove the object if it is still inserted. 
         **/
        virtual ~Plot2DArray()
            {
            detach(); // detach from its owner if there is still one. 
            }


        protected:

        /**
         * Get the value of the function at x, return a quiet NAN if x is not in the definiton domain.
         **/
        virtual double _function(double x) const override
            {
            if ((_tab == nullptr) || (_len == 0)) return std::numeric_limits<double>::quiet_NaN();
            if (!((x >= _minDomain) && (x<=_maxDomain))) return std::numeric_limits<double>::quiet_NaN();
            
			int t = interpolationMethod();
            
			if (t == INTERPOLATION_NONE)
                {
				double _e = (_maxDomain - _minDomain) / _len;
				if (!((_e >= DBL_MIN * 2) && (_e <= DBL_MAX / 2.0))) return std::numeric_limits<double>::quiet_NaN();
				size_t n = (size_t)((x - _minDomain) / _e);
                if (n >= _len) return std::numeric_limits<double>::quiet_NaN();
                double y;
                try { y = (double)_tab[n]; } catch (...) { y = std::numeric_limits<double>::quiet_NaN(); }
                return y;
                }

			if (_len <= 1) return std::numeric_limits<double>::quiet_NaN();
			const double _e = (_maxDomain - _minDomain) / (_len - 1);
			if (!((_e >= DBL_MIN * 2) && (_e <= DBL_MAX / 2.0))) return std::numeric_limits<double>::quiet_NaN();
			size_t n = (size_t)((x - _minDomain) / _e);
			if (n >= _len) return std::numeric_limits<double>::quiet_NaN();

			if (t == INTERPOLATION_LINEAR)
                {
                double x1 = _minDomain + n*_e;
                double x2 = x1 + _e;
                double y1 = std::numeric_limits<double>::quiet_NaN();
                double y2 = std::numeric_limits<double>::quiet_NaN();
                try { y1 = (double)_tab[n]; } catch (...) { y1 = std::numeric_limits<double>::quiet_NaN(); }
                try { y2 = (n+1 >= _len) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n + 1]); } catch (...) { y2 = std::numeric_limits<double>::quiet_NaN(); }
                return linearInterpolation(x, fVec2(x1, y1), fVec2(x2, y2));
                }

			double x1 = _minDomain + n*_e;
            double x0 = x1 - _e;
            double x2 = x1 + _e;
            double x3 = x2 + _e;
            double y0 = std::numeric_limits<double>::quiet_NaN();
            double y1 = std::numeric_limits<double>::quiet_NaN();
            double y2 = std::numeric_limits<double>::quiet_NaN();
            double y3 = std::numeric_limits<double>::quiet_NaN();
            try { y0 = (n == 0) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n - 1]); } catch (...) { y0 = std::numeric_limits<double>::quiet_NaN(); }
            try { y1 = (double)_tab[n]; } catch (...) { y1 = std::numeric_limits<double>::quiet_NaN(); }
            try { y2 = (n + 1 >= _len) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n + 1]); } catch (...) { y2 = std::numeric_limits<double>::quiet_NaN(); }
            try { y3 = (n + 2 >= _len) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n + 2]); } catch (...) { y3 = std::numeric_limits<double>::quiet_NaN(); }
            if (t == INTERPOLATION_CUBIC) return cubicInterpolation(x, fVec2(x0, y0), fVec2(x1, y1), fVec2(x2, y2), fVec2(x3, y3));
            return monotoneCubicInterpolation(x, fVec2(x0, y0), fVec2(x1, y1), fVec2(x2, y2), fVec2(x3, y3));
            }


        protected:

        mutable const T * _tab; // need to be mutable because modified by Plot2DVector in const method _funcion
        mutable size_t _len;    // same here

        };



}


/* end of file */



