/** @file imagewidget.hpp */
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
#include "../image.hpp"
#include "../progressimg.hpp"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>

#include <atomic>
#include <thread>

namespace mtools
{

    namespace internals_graphics
    {


        /**
         * An FLTK widget that display a Image object. The image to display is set via the setImage()
         * method. The image is cached by the widget so that it can be modified/erased after having been
         * set.
         **/
        class ImageWidget : public Fl_Window
        {


            public:

            /**
             * Constructor. Same as every FLTK widget. By default, no image is associated with the window
             * (so that it is drawn with its background color).
             *
             * @param   X   The X coordinate.
             * @param   Y   The Y coordinate.
             * @param   W   The width.
             * @param   H   The height.
             * @param   l   The label.
             **/
			ImageWidget(int X, int Y, int W, int H, const char *l = 0);


            /**
            * Destructor.
            **/
            virtual ~ImageWidget();


        protected:




            /**
             * Set the image to display. The image is cached into an offscreen buffer used when the window
             * must be redrawn. This method is threadsafe.
             *
             * @param   im  the new image to display, nullptr for no image at all.
             **/
            void setImage(const Image * im = nullptr);


            /**
             * Set the image to display. The color of each pixel is 32bit and correspond to the sum of
             * nbRounds 8 bit colors. Useful for dislaying incomplete/stochastic images.
             *
             * !!! All pixels are assumed to have the same normalization (the method uses the value of normData(0,0) for 
             * all pixels !!!!
             * 
             * @param [in,out]  im  the new image to display or nullptr to remove it.
             **/
            void setImage(const ProgressImg * im = nullptr);


            /**
             * Return the width of the offscreen buffer
             **/
            int ox() const { return _ox; }


            /**
             * Return the height of the offscreen buffer
             **/
            int oy() const { return _oy; }



            /**
             * Redraws a part of the screen. Should be called only from the draw method of child widget.
             *
             * @param   r   The portion to redraw
             **/
            void partDraw(const iBox2 & r);


            /* draw the widget */
            virtual void draw();

            private:

            /* callback called when copying the image into the offscreen buffer line by line */
            static void _drawLine_callback(void * data, int x, int y, int w, uchar *buf);

            /* callback called when copying the image into the offscreen buffer line by line */
            static void _drawLine_callback2(void * data, int x, int y, int w, uchar *buf);

            std::recursive_mutex _mutim;        // mutex for synchronization

            std::atomic<Fl_Offscreen>  _offbuf; // offscreen buffer

            std::atomic<int>  _ox;              // size of the offscreen buffer
            std::atomic<int>  _oy;              //

            bool _initdraw;                     // to make sure we call the draw method of the base class at least once.
            Image *       _saved_im;	// used for saving the image prior to the first drawing
            ProgressImg * _saved_im32;	// of the window in order to avoid a seg fault with X11

        };

    }
}


/* end of file */



