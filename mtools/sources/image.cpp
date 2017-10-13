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







	}


/* end of file */

