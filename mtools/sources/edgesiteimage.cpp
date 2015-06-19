/** @file edgesiteimage.cpp */
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


#include "graphics/edgesiteimage.hpp"

namespace mtools
{
	
    cimg_library::CImg<unsigned char> & EdgeSiteImage::makeImage(cimg_library::CImg<unsigned char> & im) const
		{
		if ((im.width() <1)||(im.height() <1)) {return(im);}
        return makeImage(im, { im.width(), im.height() });
		}



    cimg_library::CImg<unsigned char> & EdgeSiteImage::makeImage(cimg_library::CImg<unsigned char> & im, iVec2 size) const
		{	
        const int sx = (int)size.X();
        const int sy = (int)size.Y();
		if ((im.width() != sx)||(im.height() != sy)) im.assign(sx,sy,1,im.spectrum());
		if (((sx<5)||(sy<5))||((sx<6)&&(sy<6))) 
			{
			if (_site) {im.clear(_csite);} else {im.clear(_cbk);}
			return im;
			}
        im.clear(_cbk);
		double kx,ky;
		kx = ((double)sx - 1.0)/100;
		ky = ((double)sy - 1.0)/100;
		_drawArrow(_cup,_up,0,im,kx,ky);
		_drawArrow(_cdown,_down,1,im,kx,ky);
		_drawArrow(_cleft,_left,2,im,kx,ky);
		_drawArrow(_cright,_right,3,im,kx,ky);
		if (_site) {_draw_centerSite(_csite,im,kx,ky);} else {_draw_centerNoSite(_csite,im,kx,ky);}
		if ((_text.length()!=0)&&((kx>0.3)&&(ky>0.3))) // draw the center text
			{
            int fs = (int)(((kx + ky) / 2) * 11); 
            fs = im.computeFontSize(_text, mtools::iVec2( (int64)(kx*50), (int64)(ky*40) ), true, 5, fs); 
            im.drawText(_text.c_str(), mtools::iVec2((int64)(kx * 50), (int64)(ky * 50)), 'c', 'c', fs, true, _ctext, 1.0);
			}
		if ((kx>0.5)&&(ky>0.5))
			{
			// collect font size
			int fsup=255,fsdown=255,fsleft=255,fsright=255;
            if (_textup.length() != 0)		{ fsup = (int)(((kx + ky) / 2) * 11);	fsup = im.computeFontSize(_textup, mtools::iVec2((int64)(kx * 28), (int64)(ky * 20)), true, 5, fsup); }
            if (_textdown.length() != 0)    { fsdown = (int)(((kx + ky) / 2) * 11); fsdown = im.computeFontSize(_textdown, mtools::iVec2((int64)(kx * 28), (int64)(ky * 20)), true, 5, fsdown); }
            if (_textleft.length() != 0)	{ fsleft = (int)(((kx + ky) / 2) * 11); fsleft = im.computeFontSize(_textleft, mtools::iVec2((int64)(kx * 24), (int64)(ky * 20)), true, 5, fsleft); }
            if (_textright.length() != 0)	{ fsright = (int)(((kx + ky) / 2) * 11); fsright = im.computeFontSize(_textright, mtools::iVec2((int64)(kx * 24), (int64)(ky * 20)), true, 5, fsright); }
			// normalise so that everyone is written the same
			int fsmin = std::min<int>(std::min<int>(std::min<int>(fsup,fsdown),fsright),fsleft);
			int fsmax = fsmin + (fsmin/5);
			if (fsup>fsmax)	    {fsup = fsmax;}
			if (fsdown>fsmax)	{fsdown = fsmax;}
			if (fsleft>fsmax)	{fsleft = fsmax;}
			if (fsright>fsmax)  {fsright = fsmax;}
			// write the texts
			if (_textup.length()!=0)	
				{
				const int & f = fsup; const int & dir = _up; const std::string & txt = _textup; const RGBc & color = _ctextup;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) { im.drawText(txt, mtools::iVec2((int64)(kx * 67), (int64)(ky * 5)), 'l', 't', f, true, color, 1.0); }
                else if (dir == EDGE) { im.drawText(txt, mtools::iVec2((int64)(kx * 57), (int64)(ky * 0)), 'l', 't', f, true, color, 1.0); }
                else { im.drawText(txt, mtools::iVec2((int64)(kx * 50), (int64)(ky * 5)), 'c', 't', f, true, color, 1.0); }
				}
			if (_textdown.length()!=0)	
				{
				const int & f = fsdown; const int & dir = _down; const std::string & txt = _textdown; const RGBc & color = _ctextdown;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) { im.drawText(txt, mtools::iVec2((int64)(kx * 67), (int64)(ky * 95)), 'l', 'b', f, true, color, 1.0); }
                else if (dir == EDGE) { im.drawText(txt, mtools::iVec2((int64)(kx * 57), (int64)(ky * 100)), 'l', 'b', f, true, color, 1.0); }
                else { im.drawText(txt, mtools::iVec2((int64)(kx * 50), (int64)(ky * 95)), 'c', 'b', f, true, color, 1.0); }
				}
			if (_textleft.length()!=0)	
				{
				const int & f = fsleft; const int & dir = _left; const std::string & txt = _textleft; const RGBc & color = _ctextleft;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) { im.drawText(txt, mtools::iVec2((int64)(kx * 3), (int64)(ky * 67)), 'l', 't', f, true, color, 1.0); }
                else if (dir == EDGE) { im.drawText(txt, mtools::iVec2((int64)(kx * 1), (int64)(ky * 57)), 'l', 't', f, true, color, 1.0); }
                else { im.drawText(txt, mtools::iVec2((int64)(kx * 3), (int64)(ky * 50)), 'l', 'c', f, true, color, 1.0); }
				}
			if (_textright.length()!=0)	
				{
				const int & f = fsright; const int & dir = _right; const std::string & txt = _textright; const RGBc & color = _ctextright;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) { im.drawText(txt, mtools::iVec2((int64)(kx * 97), (int64)(ky * 67)), 'r', 't', f, true, color, 1.0); }
                else if (dir == EDGE) { im.drawText(txt, mtools::iVec2((int64)(kx * 99), (int64)(ky * 57)), 'r', 't', f, true, color, 1.0); }
                else { im.drawText(txt, mtools::iVec2((int64)(kx * 97), (int64)(ky * 50)), 'r', 'c', f, true, color, 1.0); }
				}
			}
		return im;
		}



	/* method for drawing an edge arrow according to specified model */
    void EdgeSiteImage::_drawArrow(RGBc coul, TypeEdge type, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const
		{
		switch(type)
			{
			case NOEDGE: 
				{
				return;
				}
			case EDGE: 
				{
				_draw_extEdge(coul,direction,im,kx,ky);
				_draw_centerEdge(coul,direction,im,kx,ky);
				_draw_insideEdge(coul,direction,im,kx,ky);
				return;
				}
			case ARROW_OUTGOING:
				{
				_draw_extArrow(coul,direction,im,kx,ky);
				_draw_centerEdge(coul,direction,im,kx,ky);
				_draw_insideEdge(coul,direction,im,kx,ky);
				return;
				}
			case ARROW_INGOING:
				{
				_draw_extEdge(coul,direction,im,kx,ky);
				if (_site) {_draw_intArrowSite(coul,direction,im,kx,ky);}
				else {_draw_centerEdge(coul,direction,im,kx,ky); _draw_intArrow(coul,direction,im,kx,ky);}
				return;
				}
			case ARROW_BOTH:
				{
				_draw_extArrow(coul,direction,im,kx,ky);
				if (_site) {_draw_intArrowSite(coul,direction,im,kx,ky);}
				else {_draw_centerEdge(coul,direction,im,kx,ky); _draw_intArrow(coul,direction,im,kx,ky);}
				return;
				}
			}
		return;
		}

    void EdgeSiteImage::_draw_extArrow(RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { _draw_triangle(50, 0, 35, 15, 65, 15, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_intArrowSite(RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { _draw_triangle(35, 10, 50, 25, 65, 10, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_intArrow(RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { _draw_triangle(35, 30, 50, 45, 65, 30, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_extEdge(RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { _draw_rect(45, 0, 55, 15, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_centerEdge(RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { _draw_rect(45, 15, 55, 30, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_insideEdge(RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { _draw_rect(45, 29, 55, 45, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_centerNoSite(RGBc coul, cimg_library::CImg<unsigned char> & im, double kx, double ky) const { im.draw_rectangle((int)(kx * 45), (int)(ky * 45), (int)(kx * 55), (int)(ky * 55), coul.buf(), 1.0); }
    void EdgeSiteImage::_draw_centerSite(RGBc coul, cimg_library::CImg<unsigned char> & im, double kx, double ky) const
		{
        if ((int)(kx * 100) == (int)(ky * 100)) im.draw_circle((int)(kx * 50), (int)(ky * 50), (int)(kx * 25), coul.buf(), 1.0);
        else { im.draw_ellipse((int)(kx * 50), (int)(ky * 50), (float)(kx * 25), (float)(ky * 25), 0.0, coul.buf(), 1.0); }
		}

	/* draw a rectangle */
    void EdgeSiteImage::_draw_rect(double x0, double y0, double x1, double y1, RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky)  const
		{
		switch(direction)
			{
        case 0: {im.draw_rectangle((int)(kx*x0), (int)(ky*y0), (int)(kx*x1), (int)(ky*y1), coul.buf(), 1.0); return; }			// up
        case 1: {im.draw_rectangle((int)(kx*x0), (int)(ky*(100 - y0)), (int)(kx*x1), (int)(ky*(100 - y1)), coul.buf(), 1.0); return; }	// down
        case 2: {im.draw_rectangle((int)(kx*y0), (int)(ky*x0), (int)(kx*y1), (int)(ky*x1), coul.buf(), 1.0); return; }			// left
        case 3: {im.draw_rectangle((int)(kx*(100 - y0)), (int)(ky*x0), (int)(kx*(100 - y1)), (int)(ky*x1), coul.buf(), 1.0); return; }	// right
			}
		return;
		}

	/* draw a triangle */
    void EdgeSiteImage::_draw_triangle(double x0, double y0, double x1, double y1, double x2, double y2, RGBc coul, int direction, cimg_library::CImg<unsigned char> & im, double kx, double ky) const
		{
		switch(direction)
			{
        case 0: {im.draw_triangle((int)(kx*x0), (int)(ky*y0), (int)(kx*x1), (int)(ky*y1), (int)(kx*x2), (int)(ky*y2), coul.buf(), 1.0); return; }				// up
        case 1: {im.draw_triangle((int)(kx*x0), (int)(ky*(100 - y0)), (int)(kx*x1), (int)(ky*(100 - y1)), (int)(kx*x2), (int)(ky*(100 - y2)), coul.buf(), 1.0); return; }	// down
        case 2: {im.draw_triangle((int)(kx*y0), (int)(ky*x0), (int)(kx*y1), (int)(ky*x1), (int)(kx*y2), (int)(ky*x2), coul.buf(), 1.0); return; }				// left
        case 3: {im.draw_triangle((int)(kx*(100 - y0)), (int)(ky*x0), (int)(kx*(100 - y1)), (int)(ky*x1), (int)(kx*(100 - y2)), (int)(ky*x2), coul.buf(), 1.0); return; }	// right
			}
		return;
		}


}

/* end of file */



