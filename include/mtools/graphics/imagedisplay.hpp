/** @file imagedisplay.hpp */
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

#include "internal/imagewidgetext.hpp"



namespace mtools
{




	/**
	* Class use to display an Image object. 
    * 
	* Display can be configured to be either "passive" or "interactive":
    * 
	* - enable/disable the resizing of the window.
	* - enable/disable interactively moving/zooming around the image with mouse and keyboard.
	* - enable/disable interactively selecting a rectangular region.
	**/
	class ImageDisplay
	{


	public:


        /**
         * Constructor
         *
         * @param   W                               The width of the window (interior)
         * @param   H                               The height of the window (interior)
         * @param   X                               position of the window (x coord)
         * @param   Y                               position of the window (y coord)
         * @param   title                           title of the window
         * @param   allow_resizing                  true to allow the user to resize the widow
         * @param   allow_move                      true to allow the user to move/drag the image displayed around the window
         * @param   allow_close                     true to allow the user the user to close the window
         * @param   allow_select                    true to allow the user to select a rectangular region
         * @param   force_selection_before_closing  true to force user to select a region before closing the window
        **/
		ImageDisplay(int W, int H, int X, int Y, const char* title = nullptr,
					bool allow_resizing = true,
					bool allow_move = true,
					bool allow_close = true,
					bool allow_select = false,
					bool force_selection_before_closing = false
					);


		/** dtor */
		~ImageDisplay();


		/**
		 * Decide whether user is allowed to close the window. 
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowClosing(bool status);


		/**
		 * Decide whether user must select a region before closing. 
		 * Setting this parameter to true will automatically sets allowClosing(true). 
         * 
		 * @param	status True to allow and false to deny.
		 */
		void forceSelectionBeforeClosing(bool status);


		/**
		 * Decide whether user is allowed to move/zoom around the image.
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowUserMove(bool status);


		/**
		 * Decide whether user is allowed select a region (or modify a selected region).
		 *
		 * @param	status True to allow and false to deny.
		 */
		void allowUserSelection(bool status);


		/**
		 * Sets the current selected region. 
		 *
		 * @param [in,out]	selectRect The selection rectangle. Empty when no region currently selected. 
		 */
		void setSelection(iBox2 selectRect = iBox2());


		/**
		 * Return the currently selected region. Return an empty rectangle when no region is selected.
		 *
		 * @param	clipWithImage (Optional) True to clip the selection rectangle with the image bounding
		 * 						  box.
		 */
		iBox2 getSelection(bool clipWithImage = true);


		/**
		* Set the range as the default one (ie the smallest view that encloses the whole image). 
		**/
		void setDefaultRange();


        /**
         * Set the range to display. 
         * Note the image always start at (0,0) which is the top left corner of the window. 
         *
         * @param   R   the range (empty for default range). 
        **/
		void setRange(fBox2 R);


		/**
		* Redraw the image right away.
        * 
		* Call this will the image changes to update the screen. 
		**/
		void redrawNow();


        /**
         * Sets the image to display.
         * 
         * The image is NOT copied and must remain available as long as it is displayed ! The image
         * buffer/dimensions must NOT change while the image is being displayed.
         * 
         * If it is needed to change the image buffer/dimensions, first call setImage(nullptr) and then
         * call again setImage(&im) after the change have been made. Meanwhile setImage(nullptr) the last image 
         * will still be displayed...
         *
         * @param   im              (Optional) Pointer to the image to display. nullptr if nothing should
         *                          be displayed.
         * @param   setDefaultRange (Optional) true to use the default range, false to keep the current range
        **/
		void setImage(const Image* im = nullptr, bool useDefaultRange = true);


        /**
		 * Sets the image to display. Same as setImage().
		 *
		 * The image is NOT copied and must remain available as long as it is displayed ! The image
		 * buffer/dimensions must NOT change while the image is being displayed.
		 *
		 * If it is needed to change the image buffer/dimensions, first call setImage(nullptr) and then
		 * call again setImage(&im) after the change have been made. Meanwhile setImage(nullptr) the last image
		 * will still be displayed...
		 *
		 * @param   im              (Optional) Pointer to the image to display. nullptr if nothing should
		 *                          be displayed.
		 * @param   setDefaultRange (Optional) true to use the default range, false to keep the current range
		**/
		void operator()(const Image & im, bool useDefaultRange = true)
			{
			setImage(&im, useDefaultRange);
			}



		/**
		* Remove the image currently inserted. Same as setImage(). 
		**/
		void removeImage()
			{
			setImage();
			}


		/**
		* Start displaying the image and return only when the user closes the window 
        * 
		* calling this method will automatically set allowClosing(true);
		**/
		void display();


		/**
		* Start displaying the image and return automatically
		**/
		void startDisplay();


        /**
         * Query whether the window is currently being shown. 
         * The windows is being shown after display() or startDisplay() is called and until 
         * is is closed by stopDisplay() or if the user closes it (if he is allowed to do so). 
         *
         * @returns True if the window is being shown and false if it is hidden. 
        **/
		bool isDisplayOn(); 


		/**
		* Stop displaying (i.e.) hide the window. 
		**/
		void stopDisplay();


        /**
         * Redraw the image a given number of times per seconds
         *
         * @param   fps (Optional) The FPS, set it  <= 0 to disable autoredraw.  
        **/
		void autoredraw(int fps = 0);




	private:


		void _createWindow(int W, int H, int X, int Y, const char* title, bool allow_resizing, bool allow_move, bool allow_close, bool allow_select, bool force_selection_before_closing);


		void _destroyWindow();


		internals_graphics::ImageWidgetExt * _iwe; // the main fltk object. 


	};











}



/** end of file */

