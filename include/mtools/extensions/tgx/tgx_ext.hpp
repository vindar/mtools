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

#endif
/* end of file */

