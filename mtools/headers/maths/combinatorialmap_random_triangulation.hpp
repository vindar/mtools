/** @file combinatorialmap_random_triangulation.hpp */
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


#include "../misc/misc.hpp" 
#include "../misc/stringfct.hpp" 
#include "../misc/error.hpp"
#include "vec.hpp"
#include "box.hpp"
#include "../random/classiclaws.hpp"
#include "permutation.hpp"
#include "dyckword.hpp"
#include "../random/peelinglaw.hpp"
#include "combinatorialmap.hpp"

namespace mtools
	{



	template<typename random_t> void freeBoltzmannPeeling(CombinatorialMap & CM, int peeldart, random_t & gen)
		{

		CM.boltzmannPeelingAlgo(peeldart, [&](int peeledge, int facesize)-> int {
			MTOOLS_INSURE(facesize >= 2);
			if (facesize < 3) { return -2; } // nothing to do 
			int m = facesize - 2;
			int k = (int)freeBoltzmanTriangulationLaw(m, gen);
			if (k == -1) return -1; // new vertex discovered.
			MTOOLS_INSURE((k >= 1) && (k <= m));
			for (int i = 0; i < k; i++) { peeledge = CM.phi(peeledge); }
			return peeledge;
			});

		}

	template<typename random_t> void peelUIPT(CombinatorialMap & CM, int64 nbsteps, int peeldart, random_t & gen)
		{
		peeldart = CM.invphi(peeldart);
		int fsize = CM.faceSize(peeldart); 
		for (int64 n = 0; n < nbsteps; n++)
			{
			int k = UIPTLaw(fsize - 2, gen);
			if (k == -1)
				{
				CM.addTriangle(peeldart);
				fsize++;
				peeldart = CM.invphi(peeldart);
				}
			else
				{
				if (Unif_1(gen))
					{
					int d = peeldart;
					for (int i = 0; i < k + 1; i++) { d = CM.phi(d); }
					CM.addSplittingTriangle(peeldart, d);
					freeBoltzmannPeeling(CM, d, gen);
					peeldart = CM.invphi(peeldart);
					}
				else
					{
					int d = peeldart;
					for (int i = 0; i < k; i++) { d = CM.invphi(d); }
					CM.addSplittingTriangle(peeldart, d);
					freeBoltzmannPeeling(CM, peeldart, gen);
					peeldart = CM.invphi(d);
					}
				fsize -= k;
				}
			MTOOLS_INSURE(fsize == CM.faceSize(peeldart));
			}
		return;
		}


	}


/* end of file */