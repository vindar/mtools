/** @file tgx_ext.hpp */
//
// Copyright 2022 Arvind Singh
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

// check if tgx must be enabled
#include "../../mtools_config.hpp"


#if (MTOOLS_TGX_EXTENSIONS)

// add the tgx library, which in turn will load the tgx_ext_XXX.h file inside its classes. 
#include <tgx.h>


	/**
	* Conversion from mtools::Image to tgx::Image<RGB32>
    * 
	* Just a view: no color conversion.
	**/
	mtools::Image::operator tgx::Image<tgx::RGB32>() const
		{
		return tgx::Image<tgx::RGB32>(data(), (int)lx(), (int)ly(), (int)stride());
		}


	/**
	* Constructor from a tgx::Image. 
    * 
	* Creates a deep copy !
    * 
	**/
	template<typename color_t>
	mtools::Image::Image(const tgx::Image<color_t>& im) : Image()
		{
		if (im.isValid())
			{
			*this = Image(im.lx(), im.ly());
			for (int y = 0; y < im.ly(); y++)
				{
				for (int x = 0; x < im.lx(); x++)
					{
					setPixel(x, y, (mtools::RGBc)(im.template readPixel<false>(x, y)));
					}
				}
			}
		}



#endif


/* end of file */

