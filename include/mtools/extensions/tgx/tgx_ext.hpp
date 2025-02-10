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
	inline mtools::Image::operator tgx::Image<tgx::RGB32>() const
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
					setPixel(x, y, (mtools::RGBc)(im.template readPixel<false>({ x, y })));
					}
				}
			}
		}




	void mtools::Image::draw_circle_arc(mtools::fVec2 center, double radius, double angle_start, double angle_end, mtools::RGBc color, float opacity)
		{
		tgx::Image<tgx::RGB32> im(*this);
		im.drawCircleArcAA(tgx::fVec2(center), (float)radius, (float)angle_start, (float)angle_end, tgx::RGB32(color), opacity);
		}



    void mtools::Image::draw_thick_circle_arc(mtools::fVec2 center, double radius, double angle_start, double angle_end, double thickness, mtools::RGBc color, float opacity)
        {
		tgx::Image<tgx::RGB32> im(*this);
		im.drawThickCircleArcAA(tgx::fVec2(center), (float)radius, (float)angle_start, (float)angle_end, (float)thickness, tgx::RGB32(color), opacity);
        }



    void mtools::Image::draw_circle_sector(mtools::fVec2 center, double r, double angle_start, double angle_end, mtools::RGBc color, float opacity)
        {
		tgx::Image<tgx::RGB32> im(*this);
		im.fillCircleSectorAA(tgx::fVec2(center), (float)r, (float)angle_start, (float)angle_end, tgx::RGB32(color), opacity);
        }


    void mtools::Image::draw_thick_circle_sector(mtools::fVec2 center, double r, double angle_start, double angle_end, double thickness, mtools::RGBc color_interior, mtools::RGBc color_border, float opacity)
        {
		tgx::Image<tgx::RGB32> im(*this);
		im.fillThickCircleSectorAA(tgx::fVec2(center), (float)r, (float)angle_start, (float)angle_end, (float)thickness, tgx::RGB32(color_interior), tgx::RGB32(color_border), opacity);
        }

	
#else

void mtools::Image::draw_circle_arc(mtools::fVec2 center, double radius, double angle_start, double angle_end, mtools::RGBc color, float opacity)
	{
	MTOOLS_ERROR("Image::draw_circle_arc() not implemtented (because tgx is not available) !")
	}



void mtools::Image::draw_thick_circle_arc(mtools::fVec2 center, double radius, double angle_start, double angle_end, double thickness, mtools::RGBc color, float opacity)
	{
	MTOOLS_ERROR("Image::draw_thick_circle_arc() not implemtented (because tgx is not available) !")
	}



void mtools::Image::draw_circle_sector(mtools::fVec2 center, double r, double angle_start, double angle_end, mtools::RGBc color, float opacity)
	{
	MTOOLS_ERROR("Image::draw_circle_sector() not implemtented (because tgx is not available) !")
	}


void mtools::Image::draw_thick_circle_sector(mtools::fVec2 center, double r, double angle_start, double angle_end, double thickness, mtools::RGBc color_interior, mtools::RGBc color_border, float opacity)
	{
	MTOOLS_ERROR("Image::draw_thick_circle_sector not implemtented (because tgx is not available) !")
	}


#endif


/* end of file */

