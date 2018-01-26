/** @file rgbc_flcolor.hpp */
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

#include "../rgbc.hpp"

#include <FL/Fl.H>

namespace mtools
	{

	namespace internals_graphics
		{

		/* convert from Fl_Color to RGBc */
		inline RGBc fromFlColor(Fl_Color c, uint8 a)
			{
			RGBc coul; coul.comp.A = 255;
			Fl::get_color(c, coul.comp.R, coul.comp.G, coul.comp.B);
			return coul.getMultOpacityInt((uint32)a + 1);
			}


		/* convert from RGBc to Fl_Color */
		inline Fl_Color toFlColor(RGBc col)
			{
			return fl_rgb_color(col.comp.R, col.comp.G, col.comp.B);
			}

		}

	}

/* end of file */

