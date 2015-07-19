/** @file customcimg.hpp */
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

/*
* Header file to include to add cimg with the cimg plugin for mtools.
*/


#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "../maths/rect.hpp"
#include "rgbc.hpp"

#include <thread>
#include <mutex>
#include <memory>
#include <algorithm>

//add the plugin to the cimg library
#define cimg_plugin "graphics/cimg_plugin.hpp"	
// use libpng
#define cimg_use_png
// use libjpeg
#define cimg_use_jpeg

#if defined (_MSC_VER) 
#pragma warning( push )				// disable some warnings
#pragma warning( disable : 4244 )	//
#pragma warning( disable : 4146 )	//
#pragma warning( disable : 4267 )	//
#pragma warning( disable : 4723 )	//
#pragma warning( disable : 4305 )	//
#pragma warning( disable : 4309 )	//
#pragma warning( disable : 4197 )	//
#endif

#include <CImg.h>	    // the header for the cimg library
#undef min
#undef max

using cimg_library::CImg; 
using cimg_library::CImgList;
using cimg_library::CImgDisplay;

#if defined (_MSC_VER) 
#pragma warning( pop )
#endif


/* end of file */

