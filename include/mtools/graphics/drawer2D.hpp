/** @file plotter2D.cpp */
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


#include "graphics/image.hpp"
#include "../misc/error.hpp"
#include "../misc/internal/mtools_export.hpp"
#include "../misc/internal/forward_fltk.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "rgbc.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2DInterface.hpp"




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


		/**
		* Default ctor. 
		* Create an empty drawer with default image size
        **/
		Drawer2D() : _tabobj(),  _im(_DEFAULT_LX, _DEFAULT_LY), _nbframe(0), _rm(iVec2(_DEFAULT_LX, _DEFAULT_LY))
			{
			reset();
			}


		/**
		* Create an empty drawer with given image size
        **/
		Drawer2D(iVec2 imSize) : _tabobj(),  _im(imSize), _nbframe(0), _rm(imSize)
			{
			reset();
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
			}


		/**
		* Return the RangeManager object used to set the range.
		**/
		internals_graphics::RangeManager & range()
			{
			return _rm; 
			}


		/**
		* Return a const reference to the image. 
		**/
		const Image& image() const
			{
			return _im;
			}


		/**
		* Resize the image
		**/
		void resizeImage(mtools::iVec2 & imsize)
			{
			_im.resizeRaw(imsize);
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
		* Return the number of frames saved to disk.
		**/
		int nbFrames() const
			{
			return _nbframe;
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
		void _remove(internals_graphics::Plotter2DObj& obj)
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


		/*
			void test()
				{

				Image img(1000, 800);

				auto P = makePlot2DImage(im, 3, "im");
				((mtools::internals_graphics::Plotter2DObj *)(&P))->resetDrawing();

				}
				*/

		private:


			static void objectCB_static(void* data, void* data2, void* obj, int code);

			void objectCB(void* obj, int code);

			void _insert(internals_graphics::Plotter2DObj* obj);

			void _remove(internals_graphics::Plotter2DObj* obj);

			void _moveUp(internals_graphics::Plotter2DObj* obj);
				
			void _moveDown(internals_graphics::Plotter2DObj* obj);

			void _moveTop(internals_graphics::Plotter2DObj* obj);

			void _moveBottom(internals_graphics::Plotter2DObj* obj);



			Drawer2D(const Drawer2D&) = delete;							// no copy
			Drawer2D& operator=(const Drawer2D&) = delete;				//


			static const int _DEFAULT_LX = 800;							// default image size
			static const int _DEFAULT_LY = 600;							//


			std::vector<internals_graphics::Plotter2DObj*>	_tabobj;	// vector containing pointers to all object inserted
			Image 		_im;											// the image to draw onto
			int			_nbframe;										// number of frame created. 

			internals_graphics::RangeManager _rm;						// the range manager 


		};




		void Drawer2D::draw(int min_quality = 100)
			{

			}




		void Drawer2D::_insert(internals_graphics::Plotter2DObj* obj)
			{

			}


		void Drawer2D::_remove(internals_graphics::Plotter2DObj * obj)
			{
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if (_tabobj[i] == obj)
                    {
                    obj->_removed(); // inform than we are removing the object from the plotter
                    while ((i+1) < ((int)_tabobj.size()))
                        {
                        _tabobj[i] = _tabobj[i + 1];
                        i++;
                        }
                    _tabobj.resize(_tabobj.size() - 1); // diminish the vector size by one.
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::remove(), object not found.");
			}


		void Drawer2D::_moveUp(internals_graphics::Plotter2DObj* obj)
			{
			for (int i = 0; i < (int)_tabobj.size(); i++)
				{
				if (_tabobj[i] == obj)
					{
					if (i == 0) return; // nothing to do if already on top
					auto obj2 = _tabobj[i - 1];
					_tabobj[i - 1] = obj;
					_tabobj[i] = obj2;
					return;
					}
				}
			MTOOLS_DEBUG("Plotter2DWindow::moveUp(), object not found.");
			}


		void Drawer2D::_moveDown(internals_graphics::Plotter2DObj* obj)
			{
			for (int i = 0; i < (int)_tabobj.size(); i++)
				{
				if (_tabobj[i] == obj)
					{
					i++;
					if (i == (int)_tabobj.size()) return; // nothing to do if already at bottom
					_moveUp(_tabobj[i]);
					return;
					}
				}
			MTOOLS_DEBUG("Plotter2DWindow::moveDown(), object not found.");
			}


		void Drawer2D::_moveTop(internals_graphics::Plotter2DObj* obj)
			{	
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if (_tabobj[i] == obj)
                    {
                    if (i == 0) return; // nothing to do if already on top
                    for (int k = i; k > 0; k--) { _tabobj[k] = _tabobj[k - 1];}
                    _tabobj[0] = obj;
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveTop(), object not found.");
			}


		void Drawer2D::_moveBottom(internals_graphics::Plotter2DObj* obj)
			{
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if (_tabobj[i] == obj)
                    {
                    if (i == (((int)_tabobj.size())-1)) return; // nothing to do if already on bottom
                    for (int k = i+1; k < (int)_tabobj.size(); k++) { _tabobj[k - 1] = _tabobj[k];  }
                    _tabobj[_tabobj.size() - 1] = obj;
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveBottom(), object not found.");
			}


        void Drawer2D::objectCB_static(void * data, void * data2, void * obj, int code) { MTOOLS_ASSERT(data != nullptr); ((Drawer2D*)data)->objectCB(obj, code); }

		void Drawer2D::objectCB(void * obj, int code)
            {
            MTOOLS_ASSERT(isFltkThread());
            switch (code)
                {
                case internals_graphics::Plotter2DObj::_REQUEST_DETACH:
                    { // the object is detaching itself
                    _remove((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_REFRESH:
                    {
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_YIELDFOCUS:
                    {
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_UP:
                    {
                    _moveUp((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_DOWN:
                    {
                    _moveDown((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_TOP:
                    {
                    _moveTop((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_BOTTOM:
                    {
                    _moveBottom((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEX:
                    {
                    useRangeX((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEY:
                    {
                    useRangeY((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEXY:
                    {
                    useRangeXY((internals_graphics::Plotter2DObj*)obj);
                    _PW->take_focus();
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_FIXOBJECTWIN:
                    {
                    return;
                    }
                }
            MTOOLS_ERROR("Plotter2DWindow::objectCB, incorrect code!");
            }


}



/* end of file */



