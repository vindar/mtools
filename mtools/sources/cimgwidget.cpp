/** @file cimgwidget.cpp */
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


#include "stdafx_mtools.h"

#include "graphics/internal/cimgwidget.hpp"


namespace mtools
{

    namespace internals_graphics
    {


        CImgWidget::CImgWidget(int X, int Y, int W, int H, const char *l) : Fl_Window(X, Y, W, H, l), _offbuf((Fl_Offscreen)0), _ox(0), _oy(0), _initdraw(false), _saved_im(nullptr), _saved_im32(nullptr), _saved_nbRounds(0)
        {
        }


        

        CImgWidget::~CImgWidget()
            {
            if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
            delete _saved_im; _saved_im = nullptr;
            delete _saved_im32; _saved_im32 = nullptr;
            }


        void CImgWidget::setImage(Img<unsigned char> * im)
        {
            std::lock_guard<std::recursive_mutex> lock(_mutim);
            if ((im == nullptr) || (im->width() == 0) || (im->height() == 0)||(im->spectrum()<3))
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
                _saved_im = new Img<unsigned char>(*im, false);
                return;
                }
            const int nox = im->width();
            const int noy = im->height();
            if ((nox != _ox) || (noy != _oy))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = fl_create_offscreen(nox, noy);
                _ox = nox; _oy = noy;
                MTOOLS_ASSERT(_offbuf != ((Fl_Offscreen)0));
                }
            fl_begin_offscreen((Fl_Offscreen)(_offbuf));
            fl_draw_image(_drawLine_callback, im, 0, 0, (int)_ox, (int)_oy, 3);
            fl_end_offscreen();
            return;
        }

        void CImgWidget::_drawLine_callback(void * data, int x, int y, int w, uchar *buf)
            {
            Img<unsigned char> * im = (Img<unsigned char>*)data;
            unsigned char *pR = im->data(x, y, 0, 0), *pG = im->data(x, y, 0, 1), *pB = im->data(x, y, 0, 2);
            for (int l = 0; l < w; l++) { buf[3 * l + 0] = *pR; ++pR; buf[3 * l + 1] = *pG; ++pG; buf[3 * l + 2] = *pB; ++pB; }
            }


        void CImgWidget::setImage32(Img<uint32> * im,int nbRounds)
            {
            std::lock_guard<std::recursive_mutex> lock(_mutim);
            if ((nbRounds<=0)||(im == nullptr) || (im->width() == 0) || (im->height() == 0) || (im->spectrum()<3))
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
                _saved_im32 = new Img<uint32>(*im, false);
                _saved_nbRounds = nbRounds;
                return;
                }
            const int nox = im->width();
            const int noy = im->height();
            if ((nox != _ox) || (noy != _oy))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = fl_create_offscreen(nox, noy);
                _ox = nox; _oy = noy;
                MTOOLS_ASSERT(_offbuf != ((Fl_Offscreen)0));
                }
            fl_begin_offscreen((Fl_Offscreen)(_offbuf));
            auto data = std::pair<Img<uint32>*, int>(im, nbRounds);
            fl_draw_image(_drawLine_callback32, &data, 0, 0, (int)_ox, (int)_oy, 3);
            fl_end_offscreen();
            return;
            }


        void CImgWidget::_drawLine_callback32(void * data, int x, int y, int w, uchar *buf)
            {
            std::pair<Img<uint32>*, int> * p = (std::pair<Img<uint32>*, int> *)data;
            Img<uint32> * im = p->first;
            int nb = p->second;
            uint32 *pR = im->data(x, y, 0, 0), *pG = im->data(x, y, 0, 1), *pB = im->data(x, y, 0, 2);
            for (int l = 0; l < w; l++) 
                { 
                buf[3 * l + 0] = (unsigned char)((*pR) / nb); ++pR; 
                buf[3 * l + 1] = (unsigned char)((*pG) / nb); ++pG;
                buf[3 * l + 2] = (unsigned char)((*pB) / nb); ++pB; 
                }
            }



        void CImgWidget::partDraw(iBox2 r)
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


        void CImgWidget::draw()
            {
            if ((!_initdraw) || (w() > ((int)_ox)) || (h() > ((int)_oy))) { Fl_Window::draw(); } // first time or base widget showing : redraw it.
                {
                std::lock_guard<std::recursive_mutex> lock(_mutim);
                if (!_initdraw)
                    { 
                    _initdraw = true;
                    if (_saved_im != nullptr) { setImage(_saved_im); delete _saved_im; _saved_im = nullptr; }
                    if (_saved_im32 != nullptr) { setImage32(_saved_im32,_saved_nbRounds); delete _saved_im32; _saved_im32 = nullptr; }
                    }
                int lx = ((int)_ox > w()) ? w() : (int)_ox;
                int ly = ((int)_oy > h()) ? h() : (int)_oy;
                if ((_offbuf == ((Fl_Offscreen)0)) || (_ox <= 0) || (_oy <= 0)) { return; }
                fl_copy_offscreen(0, 0, lx, ly, (Fl_Offscreen)_offbuf, 0, 0);
                }
            }


    }
}
/* end of file */

