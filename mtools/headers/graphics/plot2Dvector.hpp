/** @file plot2Dvector.hpp */
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

#include "plot2Darray.hpp"

#include <vector>

namespace mtools
{

    template<typename T, typename Alloc > class Plot2DVector;


    /**
     * Factory function for creating Plot from a std::vector<T,A>, T must be ocnvertible to double.
     *
     * @code{.cpp}
     * std::vector<int>  tab1;
     * auto PV = makePlot2DVector(tab1);                      // the vector plot, use natural dynamically growing range.
     * std::vector<double>  tab2;
     * auto PV = makePlot2DVector(tab2,-100,100,true,"tab2"); // vector plot object : fixed domain and specific name.
     * @endcode.
     *
     * @tparam  T   type of the elements of the vector, must be convertible to double.
     * @tparam  A   optional allocator class.
     * @param   vec         the std::vector to plot.
     * @param   minDomain   The minimum of the definition domain.
     * @param   maxDomain   The maximum of the definition domain.
     * @param   fixedDomain true (default) to use a fixed domain i.e. the domain does not change even
     *                      if the vector size changes. IF set to false, maxDomain dynamically
     *                      changes to maintain the distance between elements.
     * @param   name        The name of the plot .
     *
     * @return  A Plot2DVector object encapsulating the vector.
     **/
    template<typename T, typename A>  Plot2DVector<T, A>  makePlot2DVector(const std::vector<T, A>  & vec, double minDomain, double maxDomain, bool fixedDomain = true, std::string name = "Vector")
        {
        return Plot2DVector<T, A>(vec, minDomain, maxDomain, fixedDomain, name);
        }


    /**
     * Factory function for creating Plot from a std::vector<T,A>, T must be convertible to double.
     *
     * @code{.cpp}
     * std::vector<int>  tab1;
     * auto PV = makePlot2DVector(tab1);                      // the vector plot, use natural dynamically growing range.
     * std::vector<double>  tab2;
     * auto PV = makePlot2DVector(tab2,-100,100,true,"tab2"); // vector plot object : fixed domain and specific name.
     * @endcode.
     *
     * @tparam  T   type of the elements of the vector, must be convertible to double.
     * @tparam  A   optional allocator class.
     * @param   vec         the std::vector to plot.
     * @param   fixedDomain true (default) to use a fixed domain i.e. the domain does not change even
     *                      if the vector size changes. IF set to false, maxDomain dynamically
     *                      changes to maintain the distance between elements.
     * @param   name        The name of the plot .
     *
     * @return  A Plot2DVector object encapsulating the vector.
     **/
    template<typename T, typename A>  Plot2DVector<T, A>  makePlot2DVector(const std::vector<T, A>  & vec, bool fixedDomain = false, std::string name = "Vector")
        {
        return Plot2DVector<T, A>(vec, fixedDomain, name);
        }



    /**
     * Plot object acting as a wrapper for a std::vector<T> object.
     *
     * - The type  T must be convertible to double.
     * - The vector can grow dynamically while being inserted into a plotter object for interactive
     * display (see the example below). The only thing to do is to disable the object whenever the
     * vector reallocates its storage space and then re-enable it afterwards. Appart from that, the
     * vector can be modified and its length can be changes while being displayed in the plotter.
     * - It is possible to decide whether a change in the vector size changes the domain range or
     * changes the lenght of each element of the vector instead. See the fixedDomain parameter.
     *
     * @code{.cpp}
     * double f1(double x) { return sqrt(x); }
     * double f2(double x) { return -sqrt(x); }
     *
     * int main()
     * {
     * MT2004_64 gen;                      // random number generator
     * std::vector<int>  tab;              // the vector
     * int pos = 0;                        // current position of the walk
     * Plotter2D P;                        // the plotter
     * auto PF1 = makePlot2DFun(f1);       // insert the first function
     * auto PF2 = makePlot2DFun(f2);       // insert the second function
     * auto PV = makePlot2DVector(tab);    // the vector plot, use natural dynamically growing range.
     * P[PV][PF1][PF2];                    // insert everything in the plotter
     * PV.interpolationLinear();           // linear interpolation
     * PV.hypograph(true);                 // draw the hypograph
     * PV.hypographOpacity(0.1);           // but make it almost transparent
     * P.autoredraw(300);
     * P.startPlot();                      // start the plot
     * P.range().fixedAspectRatio(false);  // disable the fixed aspect ratio
     * while (P.shown())                   // loop until the plotter window is closed
     *    {
     *    while(tab.size() < tab.capacity()) { pos += ((Unif(gen) < 0.5) ? -1 : 1); tab.push_back(pos); } // fill the vector with new steps of the walk until its capacity()
     *    PV.suspend();      // disable the graph since we must reallocate (use suspend() instead of enable(false) flickering by not erasing the drawing).
     *    tab.reserve(tab.capacity() + 100000); // reserve additonnal space
     *    PV.enable(true);   // enable again.
     *    //P.autorangeXY(); // autorange (this cause a redraw so we can turn off autoredraw if we uncomment this line)
     *    }
     * return 0;
     * }
     * @endcode.
     *
     * @tparam  T       Generic type parameter.
     * @tparam  T       Generic type parameter.
     * @tparam  Alloc   The allocator class, defaults to std::allocator<T>
     **/
    template<typename T, typename Alloc = std::allocator<T> > class Plot2DVector :public Plot2DArray < T >
    {

        public:

        /**
         * Constructor. the interval [minDomain,maxDomain] is divided in vec.size() intervals of length
         * (maxDomain-minDomain)/vec.size(). It is OK, to construct this object even if the vector is
         * empty.
         *
         * @param   vec         the std::vector to plot.
         * @param   minDomain   The minimum of the definition domain.
         * @param   maxDomain   The maximum of the definition domain.
         * @param   fixedDomain true (default) to use a fixed domain i.e. the domain does not change even
         *                      if the vector size changes. IF set to false, maxDomain dynamically
         *                      changes to maintain the distance between elements.
         * @param   name        The name of the plot .
         **/
        Plot2DVector(const std::vector<T, Alloc>  & vec, double minDomain, double maxDomain, bool fixedDomain = true, std::string name = "Vector") : Plot2DArray<T>(vec.data(), vec.size(), minDomain, maxDomain, name), _fd(fixedDomain), _vec(&vec), _e(1.0)
            {
            if (this->_len == 0) { return; }
            _e = ((this->_maxDomain - this->_minDomain) / this->_len);
            if ((_e <= DBL_MIN * 2) || (_e <= DBL_MIN * 2) || (std::isnan(_e))) _e = 1.0;
            }


        /**
         * Constructor. The definition domain defaults to [0,vec.size()[. It is ok to construct the
         * object even if the vector is empty.
         *
         * @param   vec         the std::vector to plot.
         * @param   fixedDomain False (default) to dynamically change maxDomain in such way that the
         *                      distance between elements is maintained. Set it to true to maintain the
         *                      domain even if the vector's size changes.
         * @param   name        The name of the plot .
         **/
        Plot2DVector(const std::vector<T, Alloc>  & vec, bool fixedDomain = false, std::string name = "Vector") : Plot2DArray<T>(vec.data(), vec.size(), name), _fd(fixedDomain), _vec(&vec), _e(1.0)
            {
            if (this->_len == 0) { return; }
            _e = ((this->_maxDomain - this->_minDomain) / this->_len);
            if ((_e <= DBL_MIN * 2) || (_e <= DBL_MIN * 2) || (std::isnan(_e))) _e = 1.0;
            }


        /**
         * Move Constructor.
         **/
        Plot2DVector(Plot2DVector && o) : Plot2DArray<T>(std::move(o)), _fd(o._fd), _vec(o._vec), _e(o._e)
            {
            }


        /**
         * Destructor. Remove the object if it is still inserted.
         **/
        virtual ~Plot2DVector()
            {
            this->detach(); // detach from its owner if there is still one.
            }


        /**
         * Determines if we are keeping the definiton domain fixed even when the size of the vector
         * changes.
         *
         * @return  true if we never change the domain and false if it is automatically changed to
         *          maintain the distance beetween elements.
         **/
        bool fixedDomain() const { return _fd; }


        /**
         * Set/unset whether we are using a fixed domain.
         *
         * @param   status  true to fix the domain and false to let it grow/schrink depending on the size
         *                  of the vector.
         **/
        void fixedDomain(bool status)
            {
            if (!status)
                {
                double e = ((this->_maxDomain - this->_minDomain) / this->_len);
                if ((e <= DBL_MIN * 2) || (e <= DBL_MIN * 2) || (std::isnan(e))) e = 1.0;
                _e = e;
                }
            _fd = status;
            }



        protected:

        /**
         * Get the value of the function at x, return a quiet NAN if x is not in the definiton domain.
         **/
        virtual double _function(double x) const override
            {
            this->_tab = _vec->data(); // update the pointer in case it changed
            if (this->_len != _vec->size())
                {
                if (!_fd)
                    { // dynamic domain, update it
                    this->_maxDomain = this->_minDomain + _e*_vec->size();
                    }
                this->_len = _vec->size(); // save the new size
                }
            return Plot2DArray<T>::_function(x); // call the base method
            }


        private:

        bool _fd;
        const std::vector<T, Alloc> * _vec;
        double _e;

        };



}


/* end of file */



