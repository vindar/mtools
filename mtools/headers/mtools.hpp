/** @file mtools.hpp */
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

#if defined (_MSC_VER) 
#pragma warning( push )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4319 )
#endif
#include "graphics/customcimg.hpp"
#if defined (_MSC_VER) 
#pragma warning( pop )
#endif


// containers
#include "containers/grid_basic.hpp"
#include "containers/grid_factor.hpp"
#include "containers/bitgraphZ2.hpp"
#include "containers/randomurn.hpp"
#include "containers/RWtreegraph.hpp"
#include "containers/extab.hpp"


// graphics
#include "graphics/rgbc.hpp"
#include "graphics/progressimg.hpp"
#include "graphics/simpleBMP.hpp"
#include "graphics/edgesiteimage.hpp"
#include "graphics/interpolation.hpp"
#include "graphics/planedrawer.hpp"
#include "graphics/pixeldrawer.hpp"
#include "graphics/sitedrawer.hpp"

#include "graphics/latticedrawer.hpp" // deprecated.

#include "graphics/plotter2D.hpp"
#include "graphics/plot2Darray.hpp"
#include "graphics/plot2Daxes.hpp"
#include "graphics/plot2Dfun.hpp"
#include "graphics/plot2Dgrid.hpp"
#include "graphics/plot2Dlattice.hpp"
#include "graphics/plot2Dplane.hpp"
#include "graphics/plot2Dvector.hpp"
#include "graphics/plot2Dcimg.hpp"

//i.o.
#include "io/console.hpp"
#include "io/fileio.hpp"
#include "io/logfile.hpp"
#include "io/serialization.hpp"
#include "io/commandarg.hpp"
#include "io/watch.hpp"

// maths
#include "maths/vec.hpp"
#include "maths/box.hpp"
#include "maths/sqrmatrix.hpp"
#include "maths/specialFunctions.hpp"
#include "maths/permutation.hpp"
#include "maths/graph.hpp"
#include "maths/dyckword.hpp"
#include "maths/combinatorialmap.hpp"
#include "maths/circlePacking.hpp"

//misc
#include "misc/error.hpp"
#include "misc/stringfct.hpp"
#include "misc/triple.hpp"
#include "misc/indirectcall.hpp"
#include "misc/memory.hpp"
#include "misc/metaprog.hpp"
#include "misc/misc.hpp"
#include "misc/timefct.hpp"

// random
#include "random/gen_mt2002_32.hpp"
#include "random/gen_mt2004_64.hpp"
#include "random/gen_xorgen4096_64.hpp"
#include "random/gen_fastRNG.hpp"
#include "random/classiclaws.hpp"
#include "random/SRW.hpp"
#include "random/peelingUIPT.hpp"
#include "random/krikunlaw.hpp"
#include "random/randomgraph.hpp"


// just in case...
#undef min
#undef max

/* end of file */


