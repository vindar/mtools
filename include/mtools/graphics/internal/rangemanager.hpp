/** @file rangemanager.hpp */
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

#include "../../mtools_config.hpp"
#include "../../maths/box.hpp"
#include "../../maths/vec.hpp"

#include <cstdint>
#include <mutex>
#include <atomic>


namespace mtools
{
    /* forward declaration */
    // class Plot2DComposer; // not implemented

    namespace internals_graphics
    {
        /* Forward declaration */
        class View2DWidget;


        /**
         * Class for keeping track of the displayed range of a 2D region into a given window.
         * 
         * The object manages an fBox2 rectangle describing the 2D region that should be drawn as well
         * as an iVec2 vector that describe the width/height of the associated window.
         * 
         * When a change is made to the range or the window size, the virtual method rangeNotification()
         * is called to inform of the change and confirm that it accepts it. The method can be overriden
         * in a derived class to capture these events or a callback function can be set.
         * 
         * The object is completely thread safe.
         **/
        class RangeManager
        {
            friend class View2DWidget;
            // friend class Plot2DComposer; // not implemented

            public:

            /**
             * Constructor. Create a range manager object and set the default range/window size. After
             * construction, the range may differ from startRange (due to adjustement for aspect ratio or
             * size incompatibility.
             *
             * @param   startRange          The initial (and thus default) range.
             * @param   winSize             The size of the window.
             * @param   fixedAspectRatio    true to keep a fixed aspect ratio.
             * @param   minValue            The maximum value allowed (default = MINDOUBLE = 1.0e-300).
             * @param   maxValue            The maximum value allowed (default = MAXDOUBLE = 1.0e300).
             * @param   precision           The precision (default = PRECISIONDOUBLE = 1.0e-12).
             **/
            RangeManager(mtools::fBox2 startRange, mtools::iVec2 winSize, bool fixedAspectRatio = true, double minValue = MINDOUBLE, double maxValue = MAXDOUBLE, double precision = PRECISIONDOUBLE);


            /**
             * Constructor. Create a range manager object and set the default range/window size : the range
             * is chosen to have a 1 to 1 correspondence 1 pixel = 1 site and centered around the origin.
             *
             * @param   winSize             The size of the window.
             * @param   fixedAspectRatio    true to keep a fixed aspect ratio.
             * @param   minValue            The maximum value allowed (default = MINDOUBLE = 1.0e-300).
             * @param   maxValue            The maximum value allowed (default = MAXDOUBLE = 1.0e300).
             * @param   precision           The precision (default = PRECISIONDOUBLE = 1.0e-12).
             **/
            RangeManager(mtools::iVec2 winSize, bool fixedAspectRatio = true, double minValue = MINDOUBLE, double maxValue = MAXDOUBLE, double precision = PRECISIONDOUBLE);


            /**
             * Copy constructor. Create an independent (deep) copy if the object, the new object as the same
             * range, same window size, same default value and same callback as the orginal but evolves
             * independently from now on.
             **/
            RangeManager(const RangeManager &);


            /**
             * Assignment operator. Set the range object with the same parameters (range, window size,
             * default value and callback function) as that of R (but the two object still evolve
             * independantly.
             **/
            RangeManager & operator=(const RangeManager & R);


            /**
             * Destructor.
             **/
            virtual ~RangeManager();


            /**
             * Save the current range and current window's size as the new default values.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool saveAsDefault();


            /**
             * Return the current range.
             **/
            mtools::fBox2 getRange() const;


            /**
             * Return the current size of the window.
             **/
            mtools::iVec2 getWinSize() const;


            /**
             * Return the default range.
             **/
            mtools::fBox2 getDefaultRange() const;


            /**
             * Return the default window's size.
             **/
            mtools::iVec2 getDefaultWinSize() const;


            /**
             * Go up.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool up();


            /**
             * Go down.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool down();


            /**
             * Go left.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool left();


            /**
             * Go right.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool right();


            /**
             * Zoom in.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          acquired in time or the notification callback rejected the range).
             **/
            bool zoomIn();


            /**
             * Zoom out.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          acquired in time or the notification callback rejected the range).
             **/
            bool zoomOut();


            /**
             * Zoom in (relative to a given center)
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          acquired in time or the notification callback rejected the range).
             **/
            bool zoomIn(fVec2 center);


            /**
             * Zoom out (relative to a given center)
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          acquired in time or the notification callback rejected the range).
             **/
            bool zoomOut(fVec2 center);


    protected:

            /**
             * Change the window's size and adjust the range accordingly. Only friend can change the window
             * size after construction.
             *
             * @param   newWinSize  Size of the new window.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool winSize(mtools::iVec2 newWinSize);


            /**
             * Set a new range but do not notify rangeNotification()/the callback method.
             *
             * @param   newRange        The new range.
             * @param   keepAspectRatio true to keep aspect ratio (use the centered enclosing rect).
             *
             * @return  true if the operation succeded and false if its failed (the lock could not be aquired
             *          in time).
             **/
            bool setRangeSilently(mtools::fBox2 newRange, bool keepAspectRatio = false);

    public:

            /**
             * Set a new range. Use the enclosing rectangle tio maintain the aspect ratio if the "fixed
             * aspect ratio" flag is on.
             *
             * @param   newRange    The new range.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool setRange(mtools::fBox2 newRange);


            /**
             * Centers the range around a given position.
             *
             * @param   center  The center.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool center(mtools::fVec2 center);


            /**
             * Return the aspect ratio.
             **/
            double ratio() const;


            /**
             * Return whether we are using a fixed aspect ratio
             *
             * @return  true if the ratio is fixed and false otherwise.
             **/
            bool fixedAspectRatio() const;


            /**
             * Set whether we keep a fixed aspect ratio.
             *
             * @param   fix to keep a fixed apsect ratio, false otherwise.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool fixedAspectRatio(bool fix);


            /**
             * Set the range such that 1 pixel = 1 unit
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool set1to1();


            /**
             * Adjust the range to get a 1:1 aspect ratio.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool setRatio1();


            /**
             * Reset the range to its initial value but adapt it to fit the current window size if different
             * from the default window size.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool reset();


            /**
             * Set the canonial range for a given window size : 1 pixel = 1 site and centered around the
             * origin.
             *
             * @return  true if the operation succeded and false if its failed (either the lock could not be
             *          aquired in time or the notification callback rejected the range).
             **/
            bool canonicalRange();


            /**
             * Convert a pixel position in the screen into an absolute position.
             **/
            fVec2 pixelToAbs(iVec2 pixpos) const;


            /**
             * Convert an absolute position into a pixel position in the screen.
             **/
            iVec2 absToPix(fVec2 abspos) const;



            protected:

            /**
             * Virtual method called when the range or the the size of the window is modified (it is not
             * called when the default value are modified ar when the min/max constraint are modified). If
             * the method is not overriden, it will call a callback if set via setNotificationCallback().
             *
             * @param   changedRange            true if the current range was modified.
             * @param   changedWinSize          true if the current size of the window was modified.
             * @param   changedFixAspectRatio   true if the fix aspect ratio flag  was modified.
             *
             * @return  true if we accept the changes and false to prevent theses changes : if the fuction
             *          return false, the range/window size revert back to their previous values.
             **/
            virtual bool rangeNotification(bool changedRange, bool changedWinSize, bool changedFixAspectRatio);


            public:

            /**
             * typedef for the callback notification function (used if rangeNotification is not overridden). 
             **/
            typedef bool(*pnotif)(void * data, void * data2, bool changedRange, bool changedWinSize, bool changedFixAspectRatio);


            /**
             * Set the callback for a notification. This function is called only if the virtual method
             * rangeNotification is not overriden.
             * 
             * @warning the callback is called inside a mutex lock.  
             *
             * @param   cb              data , bool chRange , bool chWinsize)
             *                          set it to nullptr to remove the callback.
             * @param [in,out]  data    additional pointer to passed to the callback.
             * @param [in,out]  data2   second additional pointer passed to the callback.
             **/
            void setNotificationCallback( pnotif cb , void * data, void * data2);


            private:
            
            static const double PRECISIONDOUBLE;
            static const double MAXDOUBLE;
            static const double MINDOUBLE;

            static const int MAXLOCKTIME; // maximum time we can wait for aquiring a lock, otherwise the method fails.

            /* return true if the range is Ok */
            bool _rangeOK(mtools::fBox2 r);

            /* fix the range if the aspect ratio is close but not equal to 1:1 */
            void _fixRange();

            /* set the default range */
            void _defaultrange();

            pnotif _cbfun;              // the callback function
            void * _data;               // the data to pass to the callback
            void * _data2;              // the data to pass to the callback
            mtools::fBox2 _startRange;  // initial starting range
            mtools::fBox2 _range;       // current range
            mtools::iVec2 _startWin;    //initial window size
            mtools::iVec2 _winSize;     // window size
            double _minValue, _maxValue, _precision; // the extremal admissible values
            std::atomic<bool> _fixedAR; // do we keep a fixed aspect ratio
            mutable std::recursive_timed_mutex _mut;    // mutex for sync.

        };



    }
}


/* end of file */
