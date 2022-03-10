/** @file imagewidgetext.hpp */
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


#include "imagewidget.hpp"

#include <FL/fl_ask.H>



namespace mtools
{

    namespace internals_graphics
    {



/** 
 * FLTK Widget to display an Image object. 
 * The widget can be customized for "active" or "passive" display:
 * 
 * - enable/disable resizing of the window.   
 * - enable/disable interactively moving/zooming around the image.  
 * - enable/disable interactively selecting a rectangular region. 
 * 
 * THis widget is not thread safe. All methods should be called from within 
 * the FLTK thread. 
 * 
 * -> THis widget is the foundation of the ImageDisplay class. 
 **/
class ImageWidgetExt : public ImageWidget
	{


	public:

        /**
         * Constructor. No image is associated by default. Draw a gray background.
         *
		 * @param   allow_closing   True to allow the window to be closed and false otherwise. 
		 * @param   allow_resizing  True to make the widow resizable and false to fix its size at creation without further size change.
         * @param   allow_move      True to allow user to move/drag/zoom arounf the image with the mouse and keyboard.
         * @param   allow_select    True to allow user to select a rectangle region with the mouse (by dragging with the left button pressed)
         * @param   X               The X coordinate of the window.
         * @param   Y               The Y coordinate of the window.
         * @param   W               The width of the window.
         * @param   H               The height of the window.
         * @param   l               (Optional) name of the window.
        **/
		ImageWidgetExt(bool allow_closing, bool allow_resizing, bool allow_move, bool allow_select, int X, int Y, int W, int H, const char* l = 0);



		/** Destructor. */
		virtual ~ImageWidgetExt() 
			{
			_fps = 0;
			_arf = false;
			_move_allowed = false;
			_select_allowed = false;
			_close_allowed = true;
			_force_selection = false;
			}



		/**
		 * Decide whether user is allowed to close the window. 
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowClosing(bool status)
			{
			_close_allowed = status;
			}


		/**
		 * Decide whether user must select a region before closing. 
		 * Setting this parameter to true automatically sets allowClosing(true). 
         * 
		 * @param	status True to allow and false to deny.
		 */
		void forceSelectionBeforeClosing(bool status)			
			{
			_force_selection = status;
			if (status)	_close_allowed = status;
			}



		/**
		 * Decide whether user is allowed to move/zoom around the image.
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowUserMove(bool status)
			{
			_move_allowed = status;
			}


		/**
		 * Decide whether user is allowed to modify the selected region
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowUserSelection(bool status)
			{
			_select_allowed = status;
			}


		/**
		 * Sets the current selection rectange.
		 *
		 * @param [in,out]	selectRect The selection rectangle. empty when no region currently selected. 
		 */
		void setSelection(iBox2 selectRect = iBox2())
			{
			_selectR = selectRect;
			}


		/**
		 * Return the currently selected region. Return an empty rectangle when no region is selected.
		 *
		 * @param	clipWithImage (Optional) True to clip the selection rectangle with the image bounding
		 * 						  box.
		 */
		iBox2 getSelection(bool clipWithImage = true)
			{
			if ((_extim == nullptr) || (_extim->isEmpty())) return iBox2(); // no selection without an image
			return (clipWithImage ? (mtools::intersectionRect(_selectR, _extim->imageBox())) : _selectR);
			}


		/**
		* Set the default range (smallest view that encloses the whole image). 
		**/
		void setDefaultRange()
			{
			if ((_extim == nullptr) || (_extim->isEmpty()))
				{
				_viewR = fBox2(0.0, (double)_im.lx(), 0, (double)_im.ly());
				}
			else
				{
				_viewR = fBox2(0.0, (double)_extim->lx(), 0.0 , (double)_extim->ly()).fixedRatioEnclosingRect(((double)_im.lx()) / _im.ly());
				}
			redrawNow();
			}


        /**
         * Set the range to display. 
         *
         * @param   R   the range (empty for default range). 
        **/
		void setRange(fBox2 R)
			{
			if (R.isEmpty())
				{
				setDefaultRange();
				}
			else
				{
				_viewR = R;
				redrawNow();
				}
			}


		/**
		* Redraw the image right away.
		**/
		void redrawNow()
			{
			// recreate the image _im from _extim and the current range
			_updateIm();	
			
			// copy the image in the offscreen buffer
			ImageWidget::setImage(&_im);

			// ask for a redraw immediately
			redraw();
			flush();
			}


        /**
         * Sets the image to display.
         * 
         * The image is NOT copied and must remain available as long as it is displayed ! The image
         * buffer/dimensions must NOT change while the image is being displayed.
         * 
         * If it is needed to change the image buffer/dimensions, first call set(nullptr) and then
         * call again set(&im) after the change have been made. Meanwhile, set(nullptr) the last image 
         * will still be displayed
         *
         * @param   im              (Optional) Pointer to the image to display. nullptr if nothing should
         *                          be displayed.
         * @param   setDefaultRange (Optional) true to use the default range, false to keep the current range
        **/
		void set(const Image* im = nullptr, bool useDefaultRange = true) 
			{
			if (im != _extim)
				{
				_extim = im;
				if (useDefaultRange && (_extim != nullptr) && (_extim->isEmpty() == false)) setDefaultRange();
				redrawNow();
				}
			}


		/**
		 * Redraw the image a given number of times per seconds
		 *
		 * @param   fps (Optional) The FPS, set it  <= 0 to disable autoredraw.
		**/
		void  autoredraw(int fps)
			{
			if (fps <= 0)
				{
				_fps = 0;
				Fl::remove_timeout(_static_timeout_cb);
				return;
				}
			_fps = fps;			
			Fl::add_timeout(1.0 / _fps, _static_timeout_cb, this);
			}

		/**
		* Flag to enable/disable autoredraw (prevent for accessing the image when the flag is off). 
		* By default, the flag is off.
		**/
		void autoredrawflag(bool enable)
			{
			_arf = enable;
			}

	private:


		static void _static_timeout_cb(void* p)
			{
			if (p == nullptr) { return; }
			((ImageWidgetExt*)p)->_timeout_cb();
			}


		void _timeout_cb()
			{
			if (_fps <= 0) return;
			if (_arf) { redrawNow(); } // access the image only if arf is set.
			Fl::repeat_timeout(1.0 / _fps, _static_timeout_cb, this);
			}


		static void _static_callback(Fl_Widget* W, void* p) 
			{ 
			if (p == nullptr) { return; } 
			((ImageWidgetExt*)p)->_callback(W);
			}


		void _callback(Fl_Widget* W);


		virtual void resize(int X, int Y, int W, int H) override;


		/** Updates the _im from _extim according to the current range */
		void _updateIm();


		bool _isIn(iVec2 m)
			{
			return ((m.X() >= 0) && (m.Y() >= 0) && (m.X() < w()) && (m.Y() < h()));
			}

        void _saveMouse()
            {
            _current_mouse = iVec2(Fl::event_x(), Fl::event_y());
            }

		bool _canZoomIn()
			{
			if ((_extim == nullptr) || (_extim->isEmpty())) return false;
			if ((_viewR.lx() >= 1.0) && (_viewR.ly() >= 1.0)) return true;
			return false;
			}

		bool _canZoomOut()
			{
			if ((_extim == nullptr) || (_extim->isEmpty())) return false;			
			if ((_viewR.lx() > 5 * _extim->lx()) && (_viewR.ly() > 5 * _extim->ly())) return false;
			return true;
			}

		bool _canSeeImage(const fBox2 & R)
			{
			if (R.max[0] <= 0.0) return false;
			if (R.min[0] >= (double)(_extim->lx())) return false;
			if (R.max[1] <= 0.0) return false;
			if (R.min[1] >= (double)(_extim->ly())) return false;
			return true;
			}

		iVec2 _viewToImage(iVec2 pos)
			{
			int x = (int)(_viewR.min[0] + (_viewR.lx() * ((double)pos.X() + 0.5)) / (_im.lx()) - 0.5);
			int y = (int)(_viewR.min[1] + (_viewR.ly() * ((double)pos.Y() + 0.5)) / (_im.ly()) - 0.5);
			return iVec2(x, y);
			}

		fVec2 _viewToImagef(iVec2 pos)
			{
			double x = _viewR.min[0] + (_viewR.lx() * ((double)pos.X() + 0.5)) / (_im.lx()) ;
			double y = _viewR.min[1] + (_viewR.ly() * ((double)pos.Y() + 0.5)) / (_im.ly()) ;
			return fVec2(x, y);
			}

protected:


		/* fltk handle method */
		virtual int handle(int e) override;


		/* fltk method : draw the widget */
		virtual void draw() override;


	private:

		iVec2 _prev_mouse;		// previous mouse position
		iVec2 _current_mouse;	// mouse position

		iVec2 _mouse_sel1;		// mouse position at start of selection
		iVec2 _pos_sel1;		// position of start of selection (in the image space). 
		iBox2 _selectR;			// the currently selected region

		bool _select_on;		// true if when are currently selecting a region by dragging with left button. 

		bool _translate_on;		// true if when are translating by dragging with right mouse pressed
		fVec2 _translate_pos;	// absolute position of the mouse w.r.t the image. 

		std::atomic<bool>  _move_allowed;		// true if the user is allowed to change the range. 
		std::atomic<bool>  _select_allowed;		// true if the user is allowed to change the range. 
		std::atomic<bool>  _resize_allowed;		// true if window resizing is allowed. 
		std::atomic<bool>  _close_allowed;		// true if the user can close the window himself
		std::atomic<bool>  _force_selection;	// true if a selection must be made before allowing the window to be closed. 

		std::atomic<int> _fps;					// fps for redraw timer (<= 0 to disable). 
		std::atomic<bool> _arf;					// auto redraw enable flag. 

		const Image* _extim;	// the image external source image  
		Image _im;				// internal image that mirror that drawn. 		
		fBox2 _viewR;			// the rectangle dislayed by the window. 


	};









    }

}


/** end of file */



