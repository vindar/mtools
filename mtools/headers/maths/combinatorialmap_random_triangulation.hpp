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

	/**
	 * Insert a Free Boltzmann Triangulation (of type II) inside a given face of a map using the
	 * peeling algorithm.
	 * 
	 * This method only add new darts/vertices/faces inside the map but does not change the numbering
	 * of the vertices/darts/other faces already present. 
	 *
	 * 
	 * @param [in,out]	CM			The map.
	 * @param	peeldart            a dart that identifies the face inside which the FBT should be constructed.
	 * @param	avoiddoubleedges	true to avoid double edges whenever possible. This means that when the peeling
	 * 								algorithm encounters a face of size two, it collapse the two edges together.
	 * 								This is useful for creating a type III free Boltzman triangulation BUT even 
	 * 								if avoiddoubleedges = true, the resulting map may still contain double edges!
	 * 								Use collapseToTypeII() after this method to create a 'real' type III map.
	 *
	 * @param [in,out]	gen			random number generator.
	 */
	template<typename random_t> void freeBoltzmannTriangulation(CombinatorialMap & CM, int peeldart, bool avoiddoubleedges, random_t & gen)
		{
		CM.boltzmannPeelingAlgo(peeldart, [&](int peeledge, int facesize)-> int {
			MTOOLS_ASSERT((facesize >= 2)); // face must have size at least 2
			MTOOLS_ASSERT((facesize >= 3)||(!avoiddoubleedges)); // if we avoid double edges, then all faces must have size >= 3.
			int m = facesize - 2;
			int k = (int)freeBoltzmanTriangulationLaw(m, gen);
			if (k == -1) return -1; // new vertex discovered.
			if ((m == 0) && (k == 0)) return -2; // stop peeling this face of size 2
			MTOOLS_ASSERT((k >= 1)&&(k <= m));
			for (int i = 0; i < k+1; i++) { peeledge = CM.phi(peeledge); }
			return peeledge;
			},true); 
		}



	/**
	* Insert a generalized Boltzmann Triangulation (of type II) inside a given face of a map using the
	* peeling algorithm.
	*
	* This method only add new darts/vertices/faces inside the map but does not change the numbering
	* of the vertices/darts/other faces already present.
	*
	*
	* @param [in,out]	CM			The map.
	* @param	peeldart            a dart that identifies the face inside which the FBT should be constructed.
	* 0param    theta               Parameter of the boltzman in (0,1/6] (cf peelinglaw.hpp for details).		
	* @param	avoiddoubleedges	true to avoid double edges whenever possible. This means that when the peeling
	* 								algorithm encounters a face of size two, it collapse the two edges together.
	* 								This is useful for creating a type III free Boltzman triangulation BUT even
	* 								if avoiddoubleedges = true, the resulting map may still contain double edges!
	* 								Use collapseToTypeII() after this method to create a 'real' type III map.
	*
	* @param [in,out]	gen			random number generator.
	*/
	template<typename random_t> void generalBoltzmannTriangulation(CombinatorialMap & CM, int peeldart, double theta, bool avoiddoubleedges, random_t & gen)
		{
		CM.boltzmannPeelingAlgo(peeldart, [&](int peeledge, int facesize)-> int {
			MTOOLS_ASSERT((facesize >= 2)); // face must have size at least 2
			MTOOLS_ASSERT((facesize >= 3) || (!avoiddoubleedges)); // if we avoid double edges, then all faces must have size >= 3.
			int m = facesize - 2;
			int k = (int)generalBoltzmanTriangulationLaw(m, theta, gen);
			if (k == -1) return -1; // new vertex discovered.
			if ((m == 0) && (k == 0)) return -2; // stop peeling this face of size 2
			MTOOLS_ASSERT((k >= 1) && (k <= m));
			for (int i = 0; i < k + 1; i++) { peeledge = CM.phi(peeledge); }
			return peeledge;
			}, true);
		}



	/**
	 * Peel a given number of steps of the UIPT type II. Use the peeling 'by layer'.
	 *
	 * @param [in,out]	CM 	The map to peel
	 * @param	nbsteps	   	The nbsteps.
	 * @param	peeldart   	The peeldart.
	 * @param [in,out]	gen	The generate.
	 */
	template<typename random_t> int peelUIPT(CombinatorialMap & CM, int64 nbsteps, int predart, bool avoiddoubleddges, random_t & gen)
		{
		int fsize = CM.faceSize(predart); 
		for (int64 n = 0; n < nbsteps; n++)
			{
			int k = (int)UIPTLaw(fsize - 2, gen);
			if (avoiddoubleddges) { if (fsize == 3) { k = -1; } else if (fsize - k == 2) { k--; } } // avoid problematic cases...
			if (k == -1)
				{
				CM.addTriangle(predart);
				fsize++;
				predart = CM.invphi(predart);
				}
			else
				{
				if (Unif_1(gen)) // direction
					{
					int d = predart;
					for (int i = 0; i < k + 1; i++) { d = CM.phi(d); }		
					auto fs2 = CM.addSplittingTriangle(predart, d, avoiddoubleddges); 
					MTOOLS_INSURE(fs2 = k + 1);
					if ((!avoiddoubleddges) || (fs2 > 2)) { freeBoltzmannTriangulation(CM, d, avoiddoubleddges, gen); }
					predart = CM.invphi(predart);
					}
				else
					{
					int d = predart;
					for (int i = 0; i < k; i++) { d = CM.invphi(d); }
					auto fs2 = CM.addSplittingTriangle(predart, d, avoiddoubleddges);
					int fs1 = fsize - fs2 + 1;
					MTOOLS_INSURE(fs2 = k + 1);
					if ((!avoiddoubleddges) || (fs1 > 2)) { freeBoltzmannTriangulation(CM, predart, avoiddoubleddges, gen); }
					predart = CM.invphi(d);
					}
				fsize -= k;
				}
			MTOOLS_INSURE(fsize == CM.faceSize(predart));
			}
		return predart;
		}



	/**
	* Peel a given number of steps of an hyperbolic infinite planar triangulation (cf peelinglaw.hpp).
	* Use the peeling 'by layer'.
	*
	* @param [in,out]	CM 	The map to peel
	* @param	nbsteps	   	The nbsteps.
	* @param	peeldart   	The peeldart.
	* 0param    theta       Parameter of hyperbolicity in (0,1/6] (use <= 1/8 otherwise its very slow). 						
	* @param [in,out]	gen	The generate.
	*/
	template<typename random_t> int peelHyperbolicIPT(CombinatorialMap & CM, int64 nbsteps, int predart, double theta, bool avoiddoubleddges, random_t & gen)
		{
		hyperbolicIPTLaw HL(theta);
		int fsize = CM.faceSize(predart);
		for (int64 n = 0; n < nbsteps; n++)
			{
			int k = (int)HL(fsize - 2, gen);
			if (avoiddoubleddges) { if (fsize == 3) { k = -1;} else if (fsize - k == 2) { k--; } } // avoid problematic cases...
			if (k == -1)
				{
				CM.addTriangle(predart);
				fsize++;
				predart = CM.invphi(predart);
				}
			else
				{
				if (Unif_1(gen)) // direction
					{
					int d = predart;
					for (int i = 0; i < k + 1; i++) { d = CM.phi(d); }
					auto fs2 = CM.addSplittingTriangle(predart, d, avoiddoubleddges);
					MTOOLS_INSURE(fs2 = k + 1);
					if ((!avoiddoubleddges) || (fs2 > 2)) { generalBoltzmannTriangulation(CM, d, theta, avoiddoubleddges, gen); }
					predart = CM.invphi(predart);
					}
				else
					{
					int d = predart;
					for (int i = 0; i < k; i++) { d = CM.invphi(d); }
					auto fs2 = CM.addSplittingTriangle(predart, d, avoiddoubleddges);
					int fs1 = fsize - fs2 + 1;
					MTOOLS_INSURE(fs2 = k + 1);
					if ((!avoiddoubleddges) || (fs1 > 2)) { generalBoltzmannTriangulation(CM, predart, theta, avoiddoubleddges, gen); }
					predart = CM.invphi(d);
					}
				fsize -= k;
				}
			MTOOLS_INSURE(fsize == CM.faceSize(predart));
			}
		return predart;
		}


	}


/* end of file */