/** @file rgbc.cpp */
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

#include "graphics/rgbc.hpp"

namespace mtools
{

    const RGBc RGBc::c_Black = RGBc(0, 0, 0);           
    const RGBc RGBc::c_White = RGBc(255, 255, 255);     
    const RGBc RGBc::c_Red = RGBc(255, 0, 0);           
    const RGBc RGBc::c_Blue = RGBc(0, 0, 255);          
    const RGBc RGBc::c_Green = RGBc(0, 128, 0);         
    const RGBc RGBc::c_Purple = RGBc(128, 0, 128);      
    const RGBc RGBc::c_Orange = RGBc(255, 135, 0);      
    const RGBc RGBc::c_Cyan = RGBc(0, 255, 255);        
    const RGBc RGBc::c_Lime = RGBc(0, 255, 0);          
    const RGBc RGBc::c_Salmon = RGBc(250, 128, 114);    
    const RGBc RGBc::c_Maroon = RGBc(128, 0, 0);        
    const RGBc RGBc::c_Yellow = RGBc(255, 255, 0);      
    const RGBc RGBc::c_Magenta = RGBc(255, 0, 255);     
    const RGBc RGBc::c_Olive = RGBc(128, 128, 0);       
    const RGBc RGBc::c_Teal = RGBc(0, 128, 128);        
    const RGBc RGBc::c_Gray = RGBc(128, 128, 128);      
    const RGBc RGBc::c_Silver = RGBc(192, 192, 192);    
    const RGBc RGBc::c_Navy = RGBc(0, 0, 128);          
    const RGBc RGBc::c_TransparentWhite = RGBc(255, 255, 255,0);
    const RGBc RGBc::c_TransparentBlack = RGBc(0, 0, 0, 0);
    const RGBc RGBc::c_TransparentRed   = RGBc(255, 0, 0, 0);
    const RGBc RGBc::c_TransparentGreen = RGBc(0, 255, 0, 0);
    const RGBc RGBc::c_TransparentBlue  = RGBc(0, 0 ,255, 0);

}

/* end of file */

