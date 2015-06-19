/** @file plot2Dfun.hpp */
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



#include "plot2Dbasegraph.hpp"

#include <string>
#include <limits>

namespace mtools
{

    template<typename F > class Plot2DFun;


    /**
     * Factory for creating Plot of functions.  
     * 
     * @code{.cpp}
     *    auto PF1 = makePlot2DFun(function1,"fonction 1");       // function defined on R with name "funtion 1"
     *    auto PF2 = makePlot2DFun(function2,-5,12);              // function defined on [0.1,120].
     * @endcode.
     *
     * @tparam  F   The function type, must be of the form T f(U) where U and T are both convertible
     *              to double.
     * @param [in,out]  obj the function itself.
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DFun object containing the function.
     **/
    template<typename F>  Plot2DFun<F>  makePlot2DFun(F  & obj, std::string name = "Function") 
        {
        return Plot2DFun<F>(obj,name);
        }


    /**
     * Factory function for creating Plot of functions.  
     * 
     * @code{.cpp}
     *    auto PF1 = makePlot2DFun(function1,"fonction 1");       // function defined on R with name "funtion 1"
     *    auto PF2 = makePlot2DFun(function2,-5,12);              // function defined on [0.1,120].
     * @endcode.
     *
     * @tparam  F   The function type, must be of the form T f(U) where U and T are both convertible
     *              to double.
     * @param [in,out]  obj the function itself.
     * @param   minDomain   The minimum of the definition domain.
     * @param   maxDomain   The maximum of the definition domain.
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DFun object containing the function.
     **/
    template<typename F>  Plot2DFun<F>  makePlot2DFun(F  & obj, double minDomain, double maxDomain, std::string name = "Function")
        {
        return Plot2DFun<F>(obj, minDomain, maxDomain, name);
        }




    /**
     * Plot object for a generic function/functor. Encapsulate any function/object that overload
     * operator()(double) and return a type which can be cast to a double.
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
     *    auto PF1 = make2DFun(function1);                        // same as "Plot2DFun<decltype(function1)> PF1(function1);" (function defined on R)
     *    auto PF2 = make2DFun(function2,0.1,120);                // same as "Plot2DFun<decltype(function2)> PF2(function2,0.1,120)" (function defined on [0.1,120])
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
     * @tparam  F   Type of the function/functor. The object must be such that F(double) is well
     *              formed and the returned value must be convertible to double.
     **/
    template< typename F > class  Plot2DFun : public internals_graphics::Plot2DBaseGraph
        {

        public:

        /**
         * Constructor.
         *
         * @param [in,out]  fun The function/functor to plot, must be callable via fun(double) the return
         *                      type must be convertible to double.
         * @param   minDomain   The minimum of the definition domain.
         * @param   maxDomain   The maximum of the definition domain.
         * @param   name        The name of the plot.
         **/
        Plot2DFun(F & fun, double minDomain, double maxDomain, std::string name = "Function") : Plot2DBaseGraph(minDomain, maxDomain, name), _fun(fun)
            {
            }


        /**
         * Constructor. The definition domain is the whole line.
         *
         * @param [in,out]  fun The function/functor to plot, must be callable via fun(double) the return
         *                      type must be convertible to double.
         * @param   name        The name of the plot.
         **/
        Plot2DFun(F & fun, std::string name = "Function") : Plot2DBaseGraph(name), _fun(fun)
            {
            }


        /**
         * Move Constructor.
         **/
        Plot2DFun(Plot2DFun && obj) : Plot2DBaseGraph(std::move(obj)), _fun(obj._fun)
            {
            }



        /**
         * Destructor. Remove the object if it is still inserted. 
         **/
        virtual ~Plot2DFun()
            {
            detach(); // detach from its owner if there is still one. 
            }


        protected:

        /**
         * Override : return the value of the function at x or a quiet NAN if fun(x) is not defined.
         **/
        virtual double _function(double x) const override
            {
            if ((x < _minDomain) || (x>_maxDomain)) return std::numeric_limits<double>::quiet_NaN();
            double y;
            try { y = (double)_fun(x); } catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
            return y;
            }


        private:

             F & _fun;

        };


}


/* end of file */



