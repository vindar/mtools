/** @file imagewidgetext.cpp */
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


#include "mtools_config.hpp"
#include "graphics/internal/imagewidgetext.hpp"

#include <FL/fl_ask.H>

namespace mtools
{

    namespace internals_graphics
    {

		
		ImageWidgetExt::ImageWidgetExt(bool allow_closing, bool allow_resizing, bool allow_move, bool allow_select, int X, int Y, int W, int H, const char* l) : ImageWidget(X, Y, W, H, l),
			_prev_mouse(-1, -1), _current_mouse(-1, -1), _mouse_sel1(-1, -1), _pos_sel1(-1, -1), _selectR(), _select_on(false), _translate_on(false), _translate_pos(0, 0),
			_move_allowed(allow_move), _select_allowed(allow_select), _resize_allowed(allow_resizing),
			_close_allowed(allow_closing), _force_selection(true), _fps(0), _arf(false),
			_extim(nullptr), _im(W, H)
			{
			MTOOLS_INSURE(W > 0);
			MTOOLS_INSURE(H > 0);
			setDefaultRange();
			_im.clear(RGBc::c_Gray);
			ImageWidget::setImage(&_im);
			if (allow_resizing) resizable(this);
			callback(_static_callback, this);
			}
       

		void ImageWidgetExt::_callback(Fl_Widget* W)
			{
			if (!((bool)_close_allowed)) return; // cannot close the window. 

			if (((bool)_force_selection) && ((bool)_select_allowed) && (_selectR.isEmpty()))
				{
				if (_move_allowed)
					{
					fl_message("You must select a region before closing the window\n\n - Press the left mouse button while dragging the mouse to select a region.\n - Use the arrow keys or drag the mouse while pressing the right button to move around the image.\n - Use the mouse wheel or [PgUp]/[PgDown] to zoom in/out of the image.");
					}
				else
					{
					fl_message("You must select a region before closing the window\n\n - Press the left mouse button while dragging the mouse to select a region.");
					}
				return;
				}
			_arf = false;
			hide(); // pretend to close
			return;
			}



		void ImageWidgetExt::resize(int X, int Y, int W, int H)
			{
			ImageWidget::resize(X, Y, W, H);
			if ((W != _im.lx()) || (H != _im.ly()))
				{
				_viewR.max[0] = _viewR.min[0] + (_viewR.lx() * W) / _im.lx();
				_viewR.max[1] = _viewR.min[1] + (_viewR.ly() * H) / _im.ly();
				_im.resizeRaw(W, H);
				_im.clear(RGBc::c_Gray);
				ImageWidget::setImage(&_im);
				redrawNow();
				}
			}


		/** Updates the _im from _extim according to the current range */
		void ImageWidgetExt::_updateIm()
			{
			if ((_extim != nullptr) && (_extim->isEmpty() == false))
				{
				const bool sel = !(_selectR.isEmpty());
				const RGBc dcol = RGBc::c_Black.getMultOpacity(0.5);
				const int W = (int)_im.lx();
				const int H = (int)_im.ly();
				const double ex = _viewR.lx() / W;
				const double ey = _viewR.ly() / H;
				double y = _viewR.min[1] + (ey / 2) - 0.5;
				for (int j = 0; j < H; j++)
					{
					double x = _viewR.min[0] + (ex / 2) - 0.5;
					for (int i = 0; i < W; i++)
						{
						int ix = (int)x;
						int iy = (int)y;
						RGBc col = _extim->getPixel(ix, iy, RGBc::c_Gray);
						if ((sel) && (!_selectR.isInside({ ix,iy })))	col.blend(dcol);
						_im(i, j) = col;
						x += ex;
						}
					y += ey;
					}
				}
			}


		/* fltk handle method */
		int ImageWidgetExt::handle(int e) 
			{

			if ((_extim == nullptr) || (_extim->isEmpty()))
				{ // no interaction when image not set. 
				return ImageWidget::handle(e);
				}

			switch (e)
			{

			case FL_LEAVE: 
				{ 
				_translate_on = false;
				_saveMouse();
				return 1; 
				}

			case FL_ENTER: 
				{ 
				_translate_on = false;
				_saveMouse();
				take_focus(); 
				return 1; 
				}

			case FL_FOCUS: return 1;

			case FL_UNFOCUS: return 1;
			
			case FL_MOVE:
				{
				_translate_on = false;
				_saveMouse();
				if (!_isIn(_current_mouse)) { return 1; } else { take_focus(); }
				return 1;
				}

			case FL_PUSH:
				{
				take_focus();
				_saveMouse();
				if (!_isIn(_current_mouse)) { return 1; }

				int button = Fl::event_button();
				if (button == FL_LEFT_MOUSE)
					{ // start selection
					_translate_on = false;
					if (_select_allowed)
						{
						_select_on = true;
						_mouse_sel1 = _current_mouse;
						_pos_sel1 = _viewToImage(_mouse_sel1);
						redrawNow();
						}
					return 1;
					}
				if (button == FL_RIGHT_MOUSE)
					{ // start selection
					if (_move_allowed)
						{
						_translate_on = true;
						_translate_pos = _viewToImagef(_current_mouse);
						}
					return 1;
					}
				return 1;
				}

			case FL_RELEASE:
				{
				_saveMouse();
				int button = Fl::event_button();
				if (button == FL_LEFT_MOUSE)
					{ 
					if (_select_allowed)
						{
						if (_mouse_sel1 == _current_mouse)
							{ // mouse did not move, remove selection
							_selectR = iBox2(); 
							}
						_select_on = false;
						}
					redrawNow();
					return 1;
					}

				if (button == FL_LEFT_MOUSE)
					{ // start selection
					_translate_on = false;					
					return 1;
					}

				return 1;
				}

			case FL_DRAG:
				{
				_saveMouse();

				if (_select_on)
					{
					auto pos_sel2 = _viewToImage(_current_mouse);
					_selectR = iBox2(_pos_sel1, pos_sel2, true);
					redrawNow();
					return 1;
					}

				if (_translate_on)
					{
					if (_move_allowed)
						{
						auto pos = _viewToImagef(_current_mouse);
						_viewR = _viewR + (_translate_pos - pos);
						redrawNow();
						return 1;
						}
					}
				return 1;
				}

			case FL_MOUSEWHEEL:
				{
				_translate_on = false;
				//_saveMouse();
				take_focus();
				if ((_isIn(_current_mouse)) && (_move_allowed))
					{									
					int d = Fl::event_dy();
					if ((d < 0) && (_canZoomIn()))
						{
						fBox2 R = mtools::zoomIn(_viewR, _viewToImagef(_current_mouse));
						if (_canSeeImage(R)) _viewR = R;
						_select_on = false;
						redrawNow();
						return 1; 
						}
					if ((d > 0) && (_canZoomOut()))
						{
						fBox2 R = mtools::zoomOut(_viewR, _viewToImagef(_current_mouse));
						if (_canSeeImage(R)) _viewR = R;
						_select_on = false;
						redrawNow();
						return 1;
						}
					}
				return 1;
				}

			case FL_KEYDOWN:
				{
				_translate_on = false;
				_select_on = false;
				take_focus();
				int key = Fl::event_key();
				switch (key)
					{
					case FL_Delete:
					case FL_BackSpace:
						if (_select_allowed)
							{
							_selectR = iBox2();
							redrawNow();
							}
						return 1;

					case FL_Escape:
						if (_move_allowed)
							{
							setDefaultRange();
							redrawNow();
							}
						return 1;

					case FL_Page_Up:
						if ((_move_allowed)&&(_canZoomIn()))
							{
							fBox2 R = mtools::zoomIn(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Page_Down:
						if ((_move_allowed)&&(_canZoomOut()))
							{
							fBox2 R = mtools::zoomOut(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Left:
						if (_move_allowed)
							{
							fBox2 R = mtools::left(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Right:
						if (_move_allowed)
							{
							fBox2 R = mtools::right(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Down:
						if (_move_allowed)
							{
							fBox2 R = mtools::up(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;

					case FL_Up:
						if (_move_allowed)
							{
							fBox2 R = mtools::down(_viewR);
							if (_canSeeImage(R)) _viewR = R;
							redrawNow();
							}
						return 1;
					}
				// key not handled
				return 1;
				}

			case FL_KEYUP:
				{
				return 1;
				}
				
			}

			return ImageWidget::handle(e);
			}


		/* fltk method : draw the widget */
		void ImageWidgetExt::draw()
			{
			ImageWidget::draw();

			if (_select_on)
				{
				if (_mouse_sel1 != _current_mouse)
					{
					draw_rect(iBox2(_mouse_sel1, _current_mouse, true));
					}
				}

			}





    }

}

/** end of file */

