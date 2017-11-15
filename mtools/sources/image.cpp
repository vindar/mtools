/** @file image.cpp */
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

#include "graphics/image.hpp"
#include "graphics/font.hpp"


namespace mtools
	{




	void Image::draw_text_background(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc bkcolor, const Font * font)
		{
		font->drawBackground(*this, x, y, txt, txt_pos, bkcolor);
		}


	void Image::draw_text_background(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc bkcolor, int fontsize)
		{
		gFont(fontsize, MTOOLS_EXACT_FONT).drawBackground(*this, x, y, txt, txt_pos, bkcolor);
		}


	void Image::draw_text(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc color, const Font * font)
		{
		font->drawText(*this, x, y, txt, txt_pos, color);
		}


	void Image::draw_text(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc color, int fontsize)
		{
		gFont(fontsize, MTOOLS_EXACT_FONT).drawText(*this, x, y, txt, txt_pos, color);
		}



	void Image::canvas_draw_numbers(const mtools::fBox2 & R, float scaling, mtools::RGBc color, float opacity)
		{
		scaling = scaling*((float)(std::sqrt(_lx*_ly) / 1000.0));
		int64 gradsize = 1 +(int64)(3 * scaling);
		int fontsize = 10 + (int)(10 * scaling);
		const int64 winx = this->_lx, winy = this->_ly;
		int64 py = winy - 1 - (int64)ceil(((-R.min[1]) / (R.max[1] - R.min[1]))*winy - ((double)1.0 / 2.0));
		int64 px = (int64)ceil(((-R.min[0]) / (R.max[0] - R.min[0]))*winx - ((double)1.0 / 2.0));
		if ((px > -1) && (px < winx))
			{
			int64 l, zz; double k, xx, kk, pp, xx2, op, v1, v2;
			op = ::log10(R.ly());
			if (op<0) { l = ((int64)(op)) - 1; }
			else { l = ((int64)(op)); }
			k = ::pow(10.0, (double)(l));
			v1 = floor(R.min[1] / k); v1 = v1 - 1; v2 = floor(R.max[1] / k);
			v2 = v2 + 1;
			kk = k; pp = kk / 5;
			if ((v2 - v1) < 5) { kk = k / 2; pp = kk / 5; }
			else { if ((v2 - v1) > 8) { kk = k * 2; pp = kk / 2; v1 = ((v1 / 2) * 2) - 2; } }
			xx = k*v1; xx2 = k*v1;
			while (xx2 <= (R.max[1] + 2 * k))
				{
				xx = xx + kk; xx2 = xx2 + pp;
				zz = (int64)R.absToPixel(mtools::fVec2(0, xx), mtools::iVec2(winx, winy)).Y();
				if ((zz >= -10) && (zz < winy + 10))
					{
					if (xx != 0)
						{
						std::string tt = mtools::doubleToStringNice(xx);
						if ((zz < py - 3) || (zz > py + 3)) 
							{
							draw_text({ px - 4 * gradsize, zz }, tt, MTOOLS_TEXT_CENTERRIGHT, color, fontsize);
							}
						}
					}
				}
			}
		if ((py > -1) && (py < winy))
			{
			int64 l, zz; double k, xx, kk, pp, xx2, op, v1, v2;
			op = ::log10(R.lx()); if (op<0) { l = ((int64)op) - 1; }
			else { l = (int64)op; }
			k = ::pow(10.0, (double)(l));
			v1 = floor(R.min[0] / k);  v1 = v1 - 1; v2 = floor(R.max[0] / k);  v2 = v2 + 1;
			kk = k; pp = kk / 5;
			if ((v2 - v1) < 5) { kk = k / 2; pp = kk / 5; }
			else { if ((v2 - v1) > 8) { kk = k * 2; pp = kk / 2; v1 = ((v1 / 2) * 2) - 2; } }
			xx = k*v1; xx2 = k*v1;
			while (xx2 <= (R.max[0] + 2 * k))
				{
				xx = xx + kk; xx2 = xx2 + pp;
				zz = (int)R.absToPixel(mtools::fVec2(xx, 0), mtools::iVec2(winx, winy)).X();
				if ((zz >= -30) && (zz < winx + 30))
					{
					if (xx != 0)
						{
						std::string tt = mtools::doubleToStringNice(xx);
						if ((zz < px - 3) || (zz> px + 3))
							{
							draw_text({ zz, py + 3 * gradsize }, tt, MTOOLS_TEXT_CENTERTOP, color, fontsize);
							}
						}
					}
				}
			}
		}





	}


/* end of file */

