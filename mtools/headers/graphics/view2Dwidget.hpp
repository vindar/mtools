/** @file view2Dwidget.hpp */
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

#include "../maths/box.hpp"
#include "../maths/vec.hpp"
#include "cimgwidget.hpp"
#include "rangemanager.hpp"
#include "../random/gen_fastRNG.hpp"


namespace mtools
{

    namespace internals_graphics
    {
        /**
         * A  FLTK widget used to display a 2 dimensional region.
         * 
         * - The widget display a `CImg` image which can be updated using the `setImage()` method
         * (derived from CImgWidget). The image is cached so it does not need to be shared and can be
         * disposed of (or modified) after a call to `setImage()`.
         * - The widget keep track of the range displayed via a base class `RangeManager`. THe widget is
         * inactive until a Range Manager is inserted using the `setRangeManager()` method.
         * - mouse and key events are use to modify the range manager which in turn call its
         * notification callback where the image displayed can then be updated using `setImage()`.
         * - it is possible to set callbacks to watch for changes of the fixed ratio flag, cross flag and
         * also capture key events to are unused by the widget.
         **/
        class View2DWidget : public CImgWidget
        {

        public:

            /**
             * Constructor. Create a Lattice Widget. To activate the widget, set a range manager using
             * setRangeManager().
             *
             * @param   X   The window's X position.
             * @param   Y   The window's Y position.
             * @param   W   The width of the window.
             * @param   H   The height of the window.
             **/
            View2DWidget(int X, int Y, int W, int H);


            /**
            * Destructor.
            **/
            virtual ~View2DWidget();


            /**
             * Set the zoom factor.
             *
             * @param   z   The new zoom factor (between 1 and 20)
             *
             * @return  The new zoom factor (the previous zoom factor if the new is not admissible)
             **/
            int zoomFactor(int z);


            /**
             * Return the current zoom factor.
             *
             * @return  the zoom factor (between 1 and 20).
             **/
            int zoomFactor() const;


            /**
             * Sets range manager associated with this view. Set it to nullptr to deactive the view. If RM
             * is non nullptr, then the view is active and the range of RM is modified to fit the current
             * window's size.
             * 
             * @warning This method must be called from within FLTK thread.
             *
             * @param [in,out]  RM  The range maanger associated to this view or nullptr to remove the
             *                      current one.
             **/
            void setRangeManager(RangeManager * RM);


            /**
             * Return the current size of the view (multiplied by the zoom factor).
             * 
             * @warning This method must be called from within FLTK thread.
             *
             * @return  the lenght/width of the window (times the zoom factor). 
             **/
            iVec2 viewSizeFactor() const;


            /**
             * signature of the callback used for notification.
             **/
            typedef  void(*pnotcb)(void * data, int key);


            /**
             * Sets the notification callback function. 
             *
             * @param   callback        The callback function to use of nullptr to remove a previous callback. 
             * @param [in,out]  data    The data to pass along.
             **/
            void setNotificationCB(pnotcb callback, void * data);


            /**
            * Set whether the cross should be drawn (thread safe).
            *
            * @param   status  true to draw the cross.
            **/
            void crossOn(bool status);


            /**
            * Check if the cross is displayed (thread safe).
            *
            * @return  true if it is displayed.
            **/
            bool crossOn() const;


            /**
             * signature of the callback when the status of the cross flag changes. The return value is used
             * the status of the cross flag : return newStatus to accept the change and !newstatus to
             * prevent the change.
             **/
            typedef  bool(*pcrosscb)(void * data, bool newStatus);


            /**
             * Set the callback function that is called when the status of the cross on/off changes.
             *
             * @param   callback        The callback function or nullptr to remove a previous callback.
             * @param [in,out]  data    User data to pass along.
             **/
            void setCrossCB(pcrosscb callback, void * data);


            /**
             * Redraw the view.
             * @warning This method must be called exclusively from within the fltk thread !
             **/
            void redrawView();


            /**
             * Inform that the next call to improveImageFactor should completelely overwrite the current
             * buffered image instead of being superposed.
             **/
            void discardImage();


            /**
             * Improve the quality of the image displayed in the CImgWidget object. Every call to this
             * method create a new sampling to the image combines with the previous ones.
             *
             * @param [in,out]  im  the image.
             **/
            void improveImageFactor(Img<unsigned char> * im);


            /**
            * Create shifted image from the current one according to the new range and dimension.  
            **/
            void displayMovedImage(RGBc bkColor = RGBc::c_Gray);


            /**
            * Call this method in order to resize the widget. Re-implemented from the base class.
            * 
            * @warning This method should be called exclusively from the fltk thread.
            **/
            virtual void resize(int X, int Y, int	W, int H);


   
   protected:

            /* set/unset the fixed aspect ratio in the range manager */
            void fixedRatio(bool status);

            /*check whether hte flag is set */
            bool fixedRatio() const;

            /* Return true if m is in the window */
            bool _isIn(iVec2 m);

            /* save the position of the mouse*/
            void _saveMouse();

            /* fltk handle method */
            virtual int handle(int e);
             

            /* fltk method : draw the widget */
            virtual void draw();

        private:

            /* swap the stocIm pointer */
            inline void swapStocIm() {auto tmp = _stocIm; _stocIm = _stocImAlt; _stocImAlt = tmp;}

            View2DWidget(const View2DWidget &) = delete;                // no copy
            View2DWidget& operator=(const View2DWidget &) = delete;     //


            std::atomic<bool> _crossOn; // true if the cross is toggled

            iVec2 _prevMouse;   // previous position of the mouse
            iVec2 _currentMouse;// current position of the mouse

            bool _zoomOn;       // true if we are in the process of selecting a region to zoom in.
            iVec2 _zoom1;
            iVec2 _zoom2;
            iBox2 _encR;


            pcrosscb        _crossCB;       // cross callback
            void *          _crossData;     // cross data for callback function

            pnotcb          _notCB;         // notification callback
            void *          _notData;       // notification data for callback function

            RangeManager * _RM; // range manager associated with the object. 

            std::atomic<int> _zoomFactor; // the zoom factor : the image is ZoomFactor time larger than the view.

            int _nbRounds;  // number of round in _stocIm
            bool _discardIm; // true is the next call to improvImagefactor must overwrite the previous image.

            Img<uint32> * _stocIm;     // double image buffer
            Img<uint32> * _stocImAlt;  //
            fBox2 _stocR;  // range associated with _stocIm

            FastRNG _g_fgen; // fast RNG

        };




    }
}


/* end of file */








