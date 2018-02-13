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


#include "graphics/edgesiteimage.hpp"

namespace mtools
{
	
	Image & EdgeSiteImage::makeImage(Image & im) const
		{
		if (im.isEmpty()) { return(im);}
        return makeImage(im, { im.width(), im.height() });
		}



	Image & EdgeSiteImage::makeImage(Image & im, iVec2 size) const
		{	
        const int sx = (int)size.X();
        const int sy = (int)size.Y();
		if ((im.width() != sx) || (im.height() != sy)) im.resizeRaw(sx, sy, true);
		if (im.isEmpty()) return im;
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
			fs = gFontFindSize(_text, { (int64)(kx * 50), (int64)(ky * 40) }, MTOOLS_NATIVE_FONT_BELOW, 5, fs);
			im.draw_text( { (int64)(kx * 50), (int64)(ky * 50) }, _text, MTOOLS_TEXT_CENTER, _ctext, fs);
			}
		if ((kx>0.5)&&(ky>0.5))
			{
			// collect font size
			int fsup=255,fsdown=255,fsleft=255,fsright=255;
			if (_textup.length() != 0)		{ fsup = (int)(((kx + ky) / 2) * 11);       fsup = gFontFindSize(_textup  , { (int64)(kx * 28), (int64)(ky * 20) }, MTOOLS_NATIVE_FONT_BELOW, 5, fsup); }			
			if (_textdown.length() != 0)    { fsdown = (int)(((kx + ky) / 2) * 11);   fsdown = gFontFindSize(_textdown, { (int64)(kx * 28), (int64)(ky * 20) }, MTOOLS_NATIVE_FONT_BELOW, 5, fsdown); }
			if (_textleft.length() != 0)    { fsleft = (int)(((kx + ky) / 2) * 11);   fsleft = gFontFindSize(_textleft, { (int64)(kx * 24), (int64)(ky * 20) }, MTOOLS_NATIVE_FONT_BELOW, 5, fsleft); }
			if (_textright.length() != 0)   { fsright = (int)(((kx + ky) / 2) * 11); fsright = gFontFindSize(_textright,{ (int64)(kx * 24), (int64)(ky * 20) }, MTOOLS_NATIVE_FONT_BELOW, 5, fsright); }
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
				if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH))
					{
					im.draw_text({ (int64)(kx * 67), (int64)(ky * 5) }, txt, MTOOLS_TEXT_TOPLEFT, color, f);
					}
                else if (dir == EDGE) 
					{ 
					im.draw_text({ (int64)(kx * 57), (int64)(ky * 0) }, txt, MTOOLS_TEXT_TOPLEFT, color, f);
					}
                else 
					{
					im.draw_text({ (int64)(kx * 50), (int64)(ky * 5) }, txt, MTOOLS_TEXT_CENTERTOP, color, f);
					}
				}
			if (_textdown.length()!=0)	
				{
				const int & f = fsdown; const int & dir = _down; const std::string & txt = _textdown; const RGBc & color = _ctextdown;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) 
					{ 
					im.draw_text({ (int64)(kx * 67), (int64)(ky * 95) }, txt, MTOOLS_TEXT_BOTTOMLEFT, color, f);
					}
                else if (dir == EDGE) 
					{ 
					im.draw_text({ (int64)(kx * 57), (int64)(ky * 100) }, txt, MTOOLS_TEXT_BOTTOMLEFT, color, f);
					}
                else 
					{ 
					im.draw_text({ (int64)(kx * 50), (int64)(ky * 95) }, txt, MTOOLS_TEXT_CENTERBOTTOM, color, f);
					}
				}
			if (_textleft.length()!=0)	
				{
				const int & f = fsleft; const int & dir = _left; const std::string & txt = _textleft; const RGBc & color = _ctextleft;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) 
					{ 
					im.draw_text({ (int64)(kx * 3), (int64)(ky * 67) }, txt, MTOOLS_TEXT_TOPLEFT, color, f);
					}
                else if (dir == EDGE) 
					{ 
					im.draw_text({ (int64)(kx * 1), (int64)(ky * 57) }, txt, MTOOLS_TEXT_TOPLEFT, color, f);
					}
                else 
					{ 
					im.draw_text({ (int64)(kx * 3), (int64)(ky * 50) }, txt, MTOOLS_TEXT_CENTERLEFT, color, f);
					}
				}
			if (_textright.length()!=0)	
				{
				const int & f = fsright; const int & dir = _right; const std::string & txt = _textright; const RGBc & color = _ctextright;
                if ((dir == ARROW_OUTGOING) || (dir == ARROW_INGOING) || (dir == ARROW_BOTH)) 
					{ 
					im.draw_text({ (int64)(kx * 97), (int64)(ky * 67) }, txt, MTOOLS_TEXT_TOPRIGHT, color, f);
					}
                else if (dir == EDGE) 
					{ 
					im.draw_text({ (int64)(kx * 99), (int64)(ky * 57) }, txt, MTOOLS_TEXT_TOPRIGHT, color, f);
					}
                else 
					{ 
					im.draw_text({ (int64)(kx * 97), (int64)(ky * 50) }, txt, MTOOLS_TEXT_CENTERRIGHT, color, f);
					}
				}
			}
		return im;
		}



	/* method for drawing an edge arrow according to specified model */
    void EdgeSiteImage::_drawArrow(RGBc coul, TypeEdge type, int direction, Image & im, double kx, double ky) const
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

    void EdgeSiteImage::_draw_extArrow(RGBc coul, int direction, Image & im, double kx, double ky) const { _draw_triangle(50, 0, 35, 15, 65, 15, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_intArrowSite(RGBc coul, int direction, Image & im, double kx, double ky) const { _draw_triangle(35, 10, 50, 25, 65, 10, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_intArrow(RGBc coul, int direction, Image & im, double kx, double ky) const { _draw_triangle(35, 30, 50, 45, 65, 30, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_extEdge(RGBc coul, int direction, Image & im, double kx, double ky) const { _draw_rect(45, 0, 55, 15, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_centerEdge(RGBc coul, int direction, Image & im, double kx, double ky) const { _draw_rect(45, 15, 55, 30, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_insideEdge(RGBc coul, int direction, Image & im, double kx, double ky) const { _draw_rect(45, 29, 55, 45, coul, direction, im, kx, ky); }
    void EdgeSiteImage::_draw_centerNoSite(RGBc coul, Image & im, double kx, double ky) const  
		{  
		im.draw_rectangle(iBox2((int64)(kx * 45), (int64)(kx * 55), (int64)(ky * 45), (int64)(ky * 55),true), coul, false); 
		im.fill_rectangle(iBox2((int64)(kx * 45), (int64)(kx * 55), (int64)(ky * 45), (int64)(ky * 55), true), coul, false);
		}

    void EdgeSiteImage::_draw_centerSite(RGBc coul, Image & im, double kx, double ky) const
		{
		if ((int)(kx * 100) == (int)(ky * 100))
			{
			im.draw_filled_circle({ (int64)(kx * 50), (int64)(ky * 50) }, (int64)(kx * 25), coul, coul);
			}
        else 
			{ 
			im.draw_filled_ellipse({ (int64)(kx * 50), (int64)(ky * 50) }, (int64)(kx * 25), (int64)(ky * 25), coul, coul);
			}
		}

	/* draw a rectangle */
    void EdgeSiteImage::_draw_rect(double x0, double y0, double x1, double y1, RGBc coul, int direction, Image & im, double kx, double ky)  const
		{
		switch (direction)
			{
			case 0:
				{	// up
				im.draw_rectangle(iBox2((int64)(kx*x0), (int64)(kx*x1), (int64)(ky*y0), (int64)(ky*y1), true), coul, false);
				im.fill_rectangle(iBox2((int64)(kx*x0), (int64)(kx*x1), (int64)(ky*y0), (int64)(ky*y1), true), coul, false);
				return;
				}
			case 1:
				{	// down
				im.draw_rectangle(iBox2((int64)(kx*x0), (int64)(kx*x1), (int64)(ky*(100 - y0)), (int64)(ky*(100 - y1)), true), coul, false);
				im.fill_rectangle(iBox2((int64)(kx*x0), (int64)(kx*x1), (int64)(ky*(100 - y0)), (int64)(ky*(100 - y1)), true), coul, false);
				return;
				}
			case 2:
				{   // left
				im.draw_rectangle(iBox2((int64)(kx*y0), (int64)(kx*y1), (int64)(ky*x0), (int64)(ky*x1), true), coul, false);
				im.fill_rectangle(iBox2((int64)(kx*y0), (int64)(kx*y1), (int64)(ky*x0), (int64)(ky*x1), true), coul, false);
				return;
				}
			case 3:
				{  // right
				im.draw_rectangle(iBox2((int64)(kx*(100 - y0)), (int64)(kx*(100 - y1)), (int64)(ky*x0), (int64)(ky*x1), true), coul, false);
				im.fill_rectangle(iBox2((int64)(kx*(100 - y0)), (int64)(kx*(100 - y1)), (int64)(ky*x0), (int64)(ky*x1), true), coul, false);
				return;
				}
			}
		return;
		}


	/* draw a triangle */
    void EdgeSiteImage::_draw_triangle(double x0, double y0, double x1, double y1, double x2, double y2, RGBc coul, int direction, Image & im, double kx, double ky) const
		{
		switch(direction)
			{
			case 0: 
				{// up
				im.draw_triangle({ (int64)(kx*x0), (int64)(ky*y0) }, { (int64)(kx*x1), (int64)(ky*y1) }, { (int64)(kx*x2), (int64)(ky*y2) }, coul, false, false);
				im.fill_triangle({ (int64)(kx*x0), (int64)(ky*y0) }, { (int64)(kx*x1), (int64)(ky*y1) }, { (int64)(kx*x2), (int64)(ky*y2) }, coul, false);
				return;
				}
			case 1: 
				{// down
				im.draw_triangle({ (int64)(kx*x0), (int64)(ky*(100 - y0)) }, { (int64)(kx*x1), (int64)(ky*(100 - y1)) }, { (int64)(kx*x2), (int64)(ky*(100 - y2)) }, coul, false, false);
				im.fill_triangle({ (int64)(kx*x0), (int64)(ky*(100 - y0)) }, { (int64)(kx*x1), (int64)(ky*(100 - y1)) }, { (int64)(kx*x2), (int64)(ky*(100 - y2)) }, coul, false);
				return;
				}
			case 2: 
				{// left
				im.draw_triangle({ (int64)(kx*y0), (int64)(ky*x0) }, { (int64)(kx*y1), (int64)(ky*x1) }, { (int64)(kx*y2), (int64)(ky*x2) }, coul, false, false);
				im.fill_triangle({ (int64)(kx*y0), (int64)(ky*x0) }, { (int64)(kx*y1), (int64)(ky*x1) }, { (int64)(kx*y2), (int64)(ky*x2) }, coul, false);
				return;
				}
			case 3: 
				{// right
				im.draw_triangle({ (int64)(kx*(100 - y0)), (int64)(ky*x0) }, { (int64)(kx*(100 - y1)), (int64)(ky*x1) }, { (int64)(kx*(100 - y2)), (int64)(ky*x2) }, coul, false, false);
				im.fill_triangle({ (int64)(kx*(100 - y0)), (int64)(ky*x0) }, { (int64)(kx*(100 - y1)), (int64)(ky*x1) }, { (int64)(kx*(100 - y2)), (int64)(ky*x2) }, coul, false);
				return;
				}
			}
		return;
		}


}

/* end of file */



