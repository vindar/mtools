/** @file timefct.hpp */
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


#include "../io/fltkSupervisor.hpp"
#include "misc.hpp"

#include <chrono>
#include <string>

namespace mtools
{

    /* forward declaration */
    namespace internals_timefct
        {
            class ProgressWidget;
            ProgressWidget * makeProgressWidget(bool, const std::string &);
            void setProgressWidgetValue(ProgressWidget *, double);
            void deleteProgressWidget(ProgressWidget *);
            void hideProgressWidget(ProgressWidget *);
            void showProgressWidget(ProgressWidget *);
        }


    /**
     * Return a unique identifier based on the current time, thread id and process id and other
     * randomness if possible. the value returned is suited for initialization of an RNG.
     *
     * @return  A "random" uint32 that should be truly unique even among different thread/processes.
     **/
    size_t randomID();


    /**
     * Simple Chronometer. Each call return the number of milliseconds elapsed since the previous
     * call to the Chronometer.
     *
     * @return  the time elapsed since the last call in milliseconds.
     **/
    uint64 Chronometer();


    /**
     * Convert a time length in millisecond into string with format  "XXX days YYY hours ZZZ min.
     * TTT sec. [MMM ms.]".
     *
     * @param   milliseconds        the time length in milliseconds.
     * @param   printMilliseconds   true to print milliseconds accuracy (otherwise, stop at second
     *                              accuracy).
     *
     * @return  A std::string containing the time length in human readable format.
     **/
    std::string durationToString(uint64 milliseconds, bool printMilliseconds);
     

    /**
     * Class creating a windows that shows a progress bar with an indication of the remaining time.
     *
     * @tparam  T   (scalar) type of the progress counter.
     **/
    template<class T> class ProgressBar
        {

        public:

            /**
             * Create the progress bar and initialize its range.
             *
             * @param   minval              The minimum value.
             * @param   maxval              The maximum value.
             * @param   name                The name (default "Progress")
             * @param   showRemainingTime   True (default) to show an estimation of the remaining time.
             **/
            ProgressBar(T minval, T maxval, const std::string & name = "Progress",  bool showRemainingTime = true) : _minval(minval), _maxval(maxval), _val(minval)
                {
                _PW = internals_timefct::makeProgressWidget(showRemainingTime,name);
                }


            /**
             * Create the progress bar and initialize its range. Ctor with minval = 0;
             *
             * @param   maxval              The maximum value (minval = 0).
             * @param   name                The name (default "Progress").
             * @param   showRemainingTime   True (default) to show an estimation of the remaining time.
             *
             * ### param    minval  The minimum value.
             **/
            ProgressBar(T maxval, const std::string & name = "Progress", bool showRemainingTime = true) : _minval(0), _maxval(maxval), _val(0)
                {
                _PW = internals_timefct::makeProgressWidget(showRemainingTime, name);
                }


            /**
             * Destructor. Remove the progress windows
             **/
            ~ProgressBar()
                {
                deleteProgressWidget(_PW);
                }


            /**
             * Updates the position of the progress bar. If val > maxval, the windows is hidden. If val
             * < minval, val is set to minval.
             *
             * @param   val The new value.
             **/
            inline void update(T val) { if (((T)_val) != val) { _val = val; internals_timefct::setProgressWidgetValue(_PW, _rescale(val)); } }


            /**
             * Add st to the value of the counter
             *
             * @param   st  The step to add.
             **/
            inline void step(T st)  { if (st != 0) { _val += st; internals_timefct::setProgressWidgetValue(_PW, _rescale(_val)); } }


            /**
            * Set the value to maxval+1 hence hiding the progress bar.
            **/
            inline void hide() { update(_maxval+1); }

        private:

            inline double _rescale(T v) { return(((double)(v - _minval)) / ((double)(_maxval - _minval))); }

            ProgressBar(const ProgressBar &) = delete;              // no copy
            ProgressBar & operator=(const ProgressBar &) = delete;  // 

            internals_timefct::ProgressWidget * _PW;
            int64 _minval;
            int64 _maxval;
            int64 _val;

        };



}


/* end of file */




