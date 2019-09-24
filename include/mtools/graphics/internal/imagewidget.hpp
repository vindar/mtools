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

#include <FL/Fl_Gl_Window.H>


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
        class ImageWidgetFL : public Fl_Window
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
			ImageWidgetFL(int X, int Y, int W, int H, const char *l = 0);


            /**
            * Destructor.
            **/
            virtual ~ImageWidgetFL();


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




            /** Query if partdraw() is implemented (yes on the FLTK version). */
            bool hasPartDraw() const { return true; }
 

            /**
             * Redraws a part of the screen. Should be called only from the draw method of child widget.
             *
             * @param   r   The portion to redraw
             **/
            void partDraw(const iBox2 & r);


			/** only to be called from the draw method of a child widget. After having called draw() on this object */
			void set_color(RGBc col);


			/** only to be called from the draw method of a child widget. After having called draw() on this object */
			void draw_line(iVec2 P1, iVec2 P2);


			/** only to be called from the draw method of a child widget. After having called draw() on this object */
			void draw_rect(iBox2 B);


			/** only to be called from the draw method of a child widget. After having called draw() on this object 
			    text is draw from botom left on the box, offset move the text up and left w.r.t.  the box */
			void draw_text(const std::string & text, iBox2 B, int fontsize, int text_offx, int text_offy, RGBc color, RGBc bkcolor);


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





        /**
         * OpenGL version. 
		 *
         * An FLTK widget that display a Image object. The image to display is set via the setImage().
         **/
		 
		 
        class ImageWidgetGL : public  Fl_Gl_Window
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
			ImageWidgetGL(int X, int Y, int W, int H, const char *l = 0);


            /**
            * Destructor.
            **/
			virtual ~ImageWidgetGL();


			// OPENGL 3
            /*
			virtual int handle(int event);
			*/


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
			void setImage(const ProgressImg * im);

			
            /**
             * Return the width of the cached image
             **/
            int ox() const { return _ox; }


            /**
             * Return the height of the cached image
             **/
            int oy() const { return _oy; }


            /** Query if partdraw() is implemented (not on the OpenGL version). */
            bool hasPartDraw() const { return false; }

            /**
             * not implemented for the OpenGL version
             **/
			void partDraw(const iBox2 & r);


			/** only to be called from the draw method of a child widget. After having called draw() on this object */
			void set_color(RGBc col);


			/** only to be called from the draw method of a child widget. After having called draw() on this object */
			void draw_line(iVec2 P1, iVec2 P2);


			/** only to be called from the draw method of a child widget. After having called draw() on this object */
			void draw_rect(iBox2 B);


			/** only to be called from the draw method of a child widget. After having called draw() on this object
				text is draw from botom left on the box, offset move the text up and left w.r.t.  the box */
			void draw_text(std::string text, iBox2 B, int fontsize, int text_offx, int text_offy, RGBc color, RGBc bkcolor);


			/** Draws the window, override from Fl_Gl_Window */
			void draw();


            private:


			/* initialisation for opengl */
			void init();

            std::atomic<int>  _ox, _oy;         // size of image. 

			std::atomic<int>  _tex_lx, _tex_ly; // size of the textures 

			std::atomic<uint32>  _texID[2];		// id for the 2 texture;
			std::atomic<int>  _current_tex;     // indexed of the currently displayed texture

			std::atomic<bool>  _init_done;      // true if init is complete

			Image				_tmp_im;		//  temp image for progress image

			bool				_missed_draw;	// true if a drawing was missed before opengl initialization


		};






#if defined(__APPLE__)
    // On MacOS, use OPenGL because direct FLTK drawing method are VERY slow...
	using ImageWidget = ImageWidgetGL;

#else
    // use FLTK drawing
	using ImageWidget = ImageWidgetFL; 
#endif














    }
}



/* end of file */



