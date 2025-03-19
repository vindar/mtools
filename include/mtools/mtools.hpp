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

// auto-generated configuration file.
#include "mtools_config.hpp"


//i.o.
#include "io/console.hpp"
#include "io/fileio.hpp"
#include "io/logfile.hpp"
#include "io/serialization.hpp"
#include "io/commandarg.hpp"
#include "io/watch.hpp"
#include "io/serialport.hpp"


// maths
#include "maths/rootSolver.hpp"
#include "maths/vec.hpp"
#include "maths/box.hpp"
#include "maths/sqrmatrix.hpp"
#include "maths/circle.hpp"
#include "maths/mobius.hpp"
#include "maths/specialFunctions.hpp"
#include "maths/permutation.hpp"
#include "maths/dyckword.hpp"
#include "maths/combinatorialmap.hpp"
#include "maths/combinatorialmap_random_triangulation.hpp"
#include "maths/graph.hpp"
#include "maths/circlePacking.hpp"
#include "maths/bezier.hpp"
#include "maths/DelaunayVoronoi.hpp"
#include "maths/FunctionExtremas.hpp"

//misc
#include "misc/error.hpp"
#include "misc/stringfct.hpp"
#include "misc/ostringstream.hpp"
#include "misc/tuple.hpp"
#include "misc/indirectcall.hpp"
#include "misc/memory.hpp"
#include "misc/metaprog.hpp"
#include "misc/misc.hpp"
#include "misc/timefct.hpp"
#include "misc/crc16_ccitt.hpp"


// random
#include "random/gen_mt2002_32.hpp"
#include "random/gen_mt2004_64.hpp"
#include "random/gen_xorgen4096_64.hpp"
#include "random/gen_fastRNG.hpp"
#include "random/classiclaws.hpp"
#include "random/SRW.hpp"
#include "random/peelinglaw.hpp"
#include "random/krikunlaw.hpp"


// graphics
#include "graphics/rgbc.hpp"
#include "graphics/palette.hpp"
#include "graphics/image.hpp"
#include "graphics/font.hpp"
#include "graphics/progressimg.hpp"
#include "graphics/simpleBMP.hpp"
#include "graphics/edgesiteimage.hpp"
#include "graphics/interpolation.hpp"
#include "graphics/planedrawer.hpp"
#include "graphics/pixeldrawer.hpp"
#include "graphics/sitedrawer.hpp"
#include "graphics/latticedrawer.hpp" // deprecated.
#include "graphics/plotter2D.hpp"
#include "graphics/plot2Daxes.hpp"
#include "graphics/plot2Dgrid.hpp"
#include "graphics/plot2Dfun.hpp"
#include "graphics/plot2Darray.hpp"
#include "graphics/plot2Dvector.hpp"
#include "graphics/plot2Dmap.hpp"
#include "graphics/plot2Dplane.hpp"
#include "graphics/plot2Dpixel.hpp"
#include "graphics/plot2Dlattice.hpp"
#include "graphics/plot2Dimage.hpp"
#include "graphics/plot2Dcimg.hpp"
#include "graphics/plot2Dbasic.hpp"
#include "graphics/figure.hpp"
#include "graphics/plot2Dfigure.hpp"
#include "graphics/imagedisplay.hpp"
#include "graphics/drawer2D.hpp"


// containers
#include "containers/grid_basic.hpp"
#include "containers/grid_factor.hpp"
#include "containers/bitgraphZ2.hpp"
#include "containers/randomurn.hpp"
#include "containers/RWtreegraph.hpp"
#include "containers/empiricalDistribution.hpp"
#include "containers/extab.hpp"
#include "containers/treefigure.hpp"
#include "containers/pointspace.hpp"


//extensions
#include "extensions/openCL.hpp"
#include "extensions/tgx/tgx_ext.hpp"


/* end of file */


