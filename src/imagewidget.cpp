/** @file imagewidget.cpp */
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


#include "graphics/internal/imagewidget.hpp"


namespace mtools
{

    namespace internals_graphics
    {


        ImageWidget::ImageWidget(int X, int Y, int W, int H, const char *l) : 
			Fl_Window(X, Y, W, H, l), _offbuf((Fl_Offscreen)0), 
			_ox(0), _oy(0), 
			_initdraw(false), _saved_im(nullptr), _saved_im32(nullptr)
        {
        }


        
		ImageWidget::~ImageWidget()
            {
            if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
            delete _saved_im; _saved_im = nullptr;
            delete _saved_im32; _saved_im32 = nullptr;
            }


        void ImageWidget::setImage(const Image * im)
        {
            std::lock_guard<std::recursive_mutex> lock(_mutim);
            if ((im == nullptr) || (im->isEmpty()))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = (Fl_Offscreen)0;
                _ox = 0; _oy = 0;
                redraw();  
                return;
                }
            if (!_initdraw) // prevent FLTK bug on linux by saving the image without displaying it if init is not yet done. 
                {
                delete _saved_im; _saved_im = nullptr;
                delete _saved_im32; _saved_im32 = nullptr;
                _saved_im = new Image(*im, false); // deep copy
                return;
                }
            const int nox = (int)im->width();
            const int noy = (int)im->height();
            if ((nox != _ox) || (noy != _oy))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = fl_create_offscreen(nox, noy);
                _ox = nox; _oy = noy;
                MTOOLS_ASSERT(_offbuf != ((Fl_Offscreen)0));
                }
            fl_begin_offscreen((Fl_Offscreen)(_offbuf));
            fl_draw_image(_drawLine_callback, (void*)im, 0, 0, (int)_ox, (int)_oy, 3);
            fl_end_offscreen();
            return;
        }


        void ImageWidget::_drawLine_callback(void * data, int x, int y, int w, uchar *buf)
            {
            const Image * im = (const Image *)data;
			const RGBc * p = im->offset(x, y);
			uchar * d = buf;
            for (int l = 0; l < w; l++) 
				{ 
				*(d++) = p->comp.R;
				*(d++) = p->comp.G;
				*(d++) = p->comp.B;
				p++;
				}
            }




        void ImageWidget::setImage(const ProgressImg * im)
            {
            std::lock_guard<std::recursive_mutex> lock(_mutim);
            if ((im == nullptr) || (im->isEmpty()))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = (Fl_Offscreen)0;
                _ox = 0; _oy = 0;
                redraw();
                return;
                }
            if (!_initdraw) // prevent FLTK bug on linux
                {
                delete _saved_im; _saved_im = nullptr;
                delete _saved_im32; _saved_im32 = nullptr;
                _saved_im32 = new ProgressImg(*im); // deep copy
                return;
                }
            const int nox = (int)im->width();
            const int noy = (int)im->height();
            if ((nox != _ox) || (noy != _oy))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = fl_create_offscreen(nox, noy);
                _ox = nox; _oy = noy;
                MTOOLS_ASSERT(_offbuf != ((Fl_Offscreen)0));
                }
            fl_begin_offscreen((Fl_Offscreen)(_offbuf));
            fl_draw_image(_drawLine_callback2, (void*)im, 0, 0, (int)_ox, (int)_oy, 3);
            fl_end_offscreen();
            return;
            }


        void ImageWidget::_drawLine_callback2(void * data, int x, int y, int w, uchar *buf)
            {
			const ProgressImg * im = (const ProgressImg *)data;
			const uint32  nb = *(im->normData()) + 1; // all pixels have the same normaliszation
			const RGBc64 * p = im->imData(x, y);
			uchar * d = buf;
			switch (nb)
				{
				case 1   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R);      *(d++) = (uchar)(p->comp.G);      *(d++) = (uchar)(p->comp.B);       p++; } break; }
				case 2   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 1); *(d++) = (uchar)(p->comp.G >> 1); *(d++) = (uchar)(p->comp.B >> 1);  p++; } break; }
				case 4   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 2); *(d++) = (uchar)(p->comp.G >> 2); *(d++) = (uchar)(p->comp.B >> 2);  p++; } break; }
				case 8   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 3); *(d++) = (uchar)(p->comp.G >> 3); *(d++) = (uchar)(p->comp.B >> 3);  p++; } break; }
				case 16  : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 4); *(d++) = (uchar)(p->comp.G >> 4); *(d++) = (uchar)(p->comp.B >> 4);  p++; } break; }
				case 32  : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 5); *(d++) = (uchar)(p->comp.G >> 5); *(d++) = (uchar)(p->comp.B >> 5);  p++; } break; }
				case 64  : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 6); *(d++) = (uchar)(p->comp.G >> 6); *(d++) = (uchar)(p->comp.B >> 6);  p++; } break; }
				case 128 : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 7); *(d++) = (uchar)(p->comp.G >> 7); *(d++) = (uchar)(p->comp.B >> 7);  p++; } break; }
				case 256 : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 8); *(d++) = (uchar)(p->comp.G >> 8); *(d++) = (uchar)(p->comp.B >> 8);  p++; } break; }
				default: 
					{ 
					for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R / nb); *(d++) = (uchar)(p->comp.G / nb); *(d++) = (uchar)(p->comp.B / nb);  p++; }
					break; 
					}
				}
            }



        void ImageWidget::partDraw(const iBox2 & r)
            {
            if (!_initdraw) { draw(); return; }
            else
                {
                std::lock_guard<std::recursive_mutex> lock(_mutim);
                iBox2 rr = mtools::intersectionRect(r, iBox2(0, _ox - 1, 0, _oy - 1));
                if ((_offbuf == ((Fl_Offscreen)0))||(rr.lx() < 0) || (rr.ly() < 0)) return;
                fl_copy_offscreen((int)rr.min[0], (int)rr.min[1], (int)rr.lx() + 1, (int)rr.ly() + 1, (Fl_Offscreen)_offbuf, (int)rr.min[0], (int)rr.min[1]);
                }
            }


        void ImageWidget::draw()
            {
            if ((!_initdraw) || (w() > ((int)_ox)) || (h() > ((int)_oy))) { Fl_Window::draw(); } // first time or base widget showing : redraw it.
                {
                std::lock_guard<std::recursive_mutex> lock(_mutim);

                if (!_initdraw)
                    { 
                    _initdraw = true;
                    if (_saved_im != nullptr) { setImage(_saved_im); delete _saved_im; _saved_im = nullptr; }
                    if (_saved_im32 != nullptr) { setImage(_saved_im32); delete _saved_im32; _saved_im32 = nullptr; }
                    }
                int lx = ((int)_ox > w()) ? w() : (int)_ox;
                int ly = ((int)_oy > h()) ? h() : (int)_oy;
                if ((_offbuf == ((Fl_Offscreen)0)) || (_ox <= 0) || (_oy <= 0)) { return; }

				fl_copy_offscreen(0, 0, lx, ly,(Fl_Offscreen)_offbuf, 0, 0);
                }
            }









    }
}
/* end of file */

