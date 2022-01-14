/** @file drawer2D.cpp */
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


#include "../misc/error.hpp"
#include "../misc/internal/mtools_export.hpp"
#include "../misc/internal/forward_fltk.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "image.hpp"
#include "rgbc.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2DInterface.hpp"
#include "imagedisplay.hpp"
#include "plot2Daxes.hpp"
#include "plot2Dgrid.hpp"



namespace mtools
{



	/**
	* Class that draws Plotter2Dobj into an image (without interaction). 
    * 
    * Useful for creating animation as a sequence of images.
	**/
	class Drawer2D
		{

		public:

		static const int _DEFAULT_LX = 800;							// default image size
		static const int _DEFAULT_LY = 600;							//

		static const int _DEFAULT_DISPLAY_LX = 800;					// default display size
		static const int _DEFAULT_DISPLAY_LY = 600;					//

		static const int _DEFAULT_DISPLAY_POS_X = 0;				// default display position
		static const int _DEFAULT_DISPLAY_POS_Y = 0;				//


		/**
		* Create an empty drawer with given image size
        **/
		Drawer2D(int lx = _DEFAULT_LX, int ly = _DEFAULT_LY, 
				 bool add_axe = false, bool add_grid = false,
				 bool display_image = true, 
			     int display_lx = _DEFAULT_DISPLAY_LX, int display_ly = _DEFAULT_DISPLAY_LY, 
			     int display_x = _DEFAULT_DISPLAY_POS_X, int display_y = _DEFAULT_DISPLAY_POS_Y, 
			     const char * display_title = nullptr) :
			_tabobj(), _im(), _nbframe(0), _rm(iVec2{ lx,ly }), 
			_axe_obj(nullptr), _grid_obj(nullptr), _disp(display_lx,display_ly,display_x, display_y,display_title,true,true,true,true,false)
			{
			imageSize(iVec2{ lx,ly });
			reset();
			axesObject(add_axe);
			gridObject(add_grid);
			if (display_image) _disp.startDisplay();
			}


		/**
		* dtor
		**/
		~Drawer2D()
			{
			axesObject(false);
			gridObject(false);
			removeAll();
			}


		/**
		* Reset the object to its initial state. 
		* - Remove all objects from the plotter.
        * - Reset the range to its initial state (but do not change the current image size). 
		**/
		void reset()
			{
			removeAll();
			_nbframe = 0;
			_rm.reset(); 
			if (axesObject()) insert(*axesObject());
			if (gridObject()) insert(*gridObject());
			}


		/**
		* Enable/disable the windows that display the image in real time. 
		**/
		void showDisplay(bool status)
			{
			if (status)
				{
				_disp.startDisplay();
				_disp.redrawNow();
				}
			else
				{
				_disp.stopDisplay();
				}
			}


		/**
		* Query whether the display is currently showing.
		**/
		bool isDisplayOn()
			{
			return _disp.isDisplayOn();
			}


		/**
		* Wait until the display is closed/hidden.
		* Return immediately if display is not on
		**/
		void waitForClose()
			{
			_disp.waitForClose();
			}


		/**
		* Return the RangeManager object used to set the range.
		**/
		internals_graphics::RangeManager & range()
			{
			return _rm; 
			}


		/**
		 * Try to find the best range horizontal range according to the current range and the prefered
		 * horizontal range of the objects that are currently enabled. We keep (or not) the aspect ratio
		 * depending on whether the flag is currently set in the range() object.
		 **/
		void autorangeX();


		/**
		 *  Same as autorangeX() but we first set/unset the "keep aspect ratio" flag.
		 **/
		void autorangeX(bool keepAspectRatio) 
			{
			range().fixedAspectRatio(keepAspectRatio);
			autorangeX();
			}


		/**
		 * Try to find the best vertical range according to the horizontal range and the prefered range
		 * of the objects that are currently enabled. We keep (or not) the aspect ratio depending on
		 * whether the flag is currently set in the range() object.
		 **/
		void autorangeY();


		/**
		 *  Same as autorangeY() but we first set/unset the "keep aspect ratio" flag.
		 **/
		void autorangeY(bool keepAspectRatio)
			{
			range().fixedAspectRatio(keepAspectRatio);
			autorangeY();
			}



		/**
		 * Try to find the best range (horizontal and vertical) according to the current range and the
		 * prefered range of the object that are currently enabled.We keep (or not) the aspect ratio
		 * depending on whether the flag is currently set in the range() object.
		 **/
		void autorangeXY();


		/**
		 *  Same as autorangeXY() but we first set/unset the "keep aspect ratio" flag.
		 **/
		void autorangeXY(bool keepAspectRatio)
			{
			range().fixedAspectRatio(keepAspectRatio);
			autorangeXY();
			}




		/**
		* Return a const reference to the image. 
		**/
		const Image& image() const
			{
			return _im;
			}


		/**
		* Set the size of the image. 
		**/
		void imageSize(mtools::iVec2 & imsize)
			{			
			_disp.removeImage();
			_im.resizeRaw(imsize);
			drawCheckerBoard();
			_disp.setImage(&_im);
			_rm.winSize(imsize);
			}


		/**
		* Fill the image with a uniform color
		**/
		void drawBackground(RGBc color)
			{
			_im.clear(color);
			}


		/**
		* Fill the image with a checker board pattern
		**/
		void drawCheckerBoard()
			{
			_im.checkerboard();
			}


		/**
		* Draw all the objects onto the image. 
        * 
		* Do not erase the image prior to drawing
        * 
		* Drawing stops when the quality goes over 'min_quality' value.
		**/
		void draw(int min_quality = 100);


		/**
		* save the image with a given name and optional frame number.
		**/
		void save(const std::string filename, bool add_number = true, int nb_digits = 6)
			{
			if (add_number)
				_im.save(filename.c_str(), ++_nbframe, nb_digits);
			else
				_im.save(filename.c_str());
			}


		/**
		* Draw all the objects onto the image and then save the image to file.
		*
		* Do not erase the image prior to drawing
		*
		* Drawing stops when the quality goes over 'min_quality' value.
		*
		* Same as combining the 'draw()' and save() methods above.
		**/
		void drawAndSave(const std::string filename, bool add_number = true, int nb_digits = 6, int min_quality = 100)
			{
			draw(min_quality);
			save(filename, add_number, nb_digits);
			}


		/**
		* Return the number of frames saved to disk.
		**/
		int nbFrames() const
			{
			return _nbframe;
			}


		/**
		 * Return whether an Plot2DAxes object is added on top
		 *
		 * return  a pointer to the axes Object if there is one and nullptr otherwise.
		 **/
		Plot2DAxes* axesObject() const
			{
			return _axe_obj;
			}


		/**
		 * Decides whether we add a Plot2DAxes object on top.
		 *
		 * @param   status  true to add and false to remove.
		 *
		 * @return  nullptr is status is false, a pointer to the axe object otherwise.
		 **/
		Plot2DAxes* axesObject(bool status)
			{
			if (status)
				{
				if (_axe_obj == nullptr)
					{
					_axe_obj = new mtools::Plot2DAxes;
					insert(*_axe_obj);
					}
				}
			else
				{
				if (_axe_obj != nullptr)
					{
					remove(*_axe_obj);
					delete _axe_obj;
					_axe_obj = nullptr;
					}
				}
			return _axe_obj;
			}


		/**
		 * Return whether an Plot2DGrid object is added on top.
		 *
		 * @return  a pointer to the grid Object if there is one and nullptr otherwise.
		 **/
		Plot2DGrid* gridObject() const
			{
			return _grid_obj;
			}


		/**
		 * Decides whether we add a Plot2DGrid object on top.
		 *
		 * @param   status  true to add and false to remove.
		 *
		 * @return  nullptr is status is false, a pointer to the grid object otherwise.
		 **/
		Plot2DGrid* gridObject(bool status)
			{
			if (status)
				{
				if (_grid_obj == nullptr)
					{
					_grid_obj = new mtools::Plot2DGrid;
					insert(*_grid_obj);
					}
				}
			else
				{
				if (_grid_obj != nullptr)
					{
					remove(*_grid_obj);
					delete _grid_obj;
					_grid_obj = nullptr;
					}
				}
			return _grid_obj;
			}



		/**
		* Insert an object
		**/
		void insert(internals_graphics::Plotter2DObj& obj)
			{
			_insert(&obj);
			}


		/**
		* Insert an object.
		**/
		Drawer2D& operator[](internals_graphics::Plotter2DObj& obj)
			{
			insert(obj);
			return(*this);
			}


		/**
		* Remove an object
		**/
		void remove(internals_graphics::Plotter2DObj& obj)
			{
			_remove(&obj);
			}
			

		/**
		* Remove all objects
		**/
		void removeAll();


		/**
        * Move an object up in the list of object to plot.
		**/
		void moveUp(internals_graphics::Plotter2DObj & obj)
			{
			_moveUp(&obj);
			}


		/**
		* Move an object down in the list of object to plot.
		**/
		void moveDown(internals_graphics::Plotter2DObj & obj) 
			{
			_moveDown(&obj);
			}


		/**
		* Move an object at the top of the list of object to plot (draw it first).
		**/
		void moveTop(internals_graphics::Plotter2DObj & obj)
			{
			_moveTop(&obj);
			}


		/**
		* Move an object at the bottom of the list of object to plot (draw it last).
		**/
		void moveBottom(internals_graphics::Plotter2DObj & obj)
			{
			_moveBottom(&obj);
			}



		private:


			static void objectCB_static(void* data, void* data2, void* obj, int code);

			void objectCB(void* obj, int code);

			void _insert(internals_graphics::Plotter2DObj* obj);

			void _remove(internals_graphics::Plotter2DObj* obj);

			void _moveUp(internals_graphics::Plotter2DObj* obj);
				
			void _moveDown(internals_graphics::Plotter2DObj* obj);

			void _moveTop(internals_graphics::Plotter2DObj* obj);

			void _moveBottom(internals_graphics::Plotter2DObj* obj);

			fBox2 _findRangeX(internals_graphics::Plotter2DObj* obj, fBox2 CR, bool keepAR);

			fBox2 _findRangeY(internals_graphics::Plotter2DObj* obj, fBox2 CR, bool keepAR);

			void _useRangeX(internals_graphics::Plotter2DObj* obj);

			void _useRangeY(internals_graphics::Plotter2DObj* obj);

			void _useRangeXY(internals_graphics::Plotter2DObj* obj);

			fBox2 _getAutoRangeX(fBox2 CR, bool keepAR);
			
			fBox2 _getAutoRangeY(fBox2 CR, bool keepAR);

			int _findIndex(internals_graphics::Plotter2DObj* obj);

			Drawer2D(const Drawer2D&) = delete;							// no copy
			Drawer2D& operator=(const Drawer2D&) = delete;				//

			std::vector<internals_graphics::Plotter2DObj*>	_tabobj;	// vector containing pointers to all object inserted
			Image 		_im;											// the image to draw onto
			int			_nbframe;										// number of frame created. 

			internals_graphics::RangeManager _rm;						// the range manager 

			mtools::Plot2DAxes * _axe_obj;								// optional axe object
			mtools::Plot2DGrid * _grid_obj;								// optional grid object

			ImageDisplay _disp;											// object to display the image

		};



}



/* end of file */



