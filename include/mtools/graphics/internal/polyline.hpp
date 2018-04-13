/** @file polyline.hpp */
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

#include "../../mtools_config.hpp"
#include "../../misc/error.hpp"
#include "../../misc/misc.hpp"
#include "../../maths/box.hpp"
#include "../../maths/vec.hpp"


namespace mtools
{



namespace internals_polyline
	{


	/** sub routine */
	MTOOLS_FORCEINLINE void _enlargeline(const fVec2 & P1, const fVec2 & P2, const fVec2 & P3, double thickleft, double thickright, fVec2 & A1, fVec2 & A2, fVec2 & A3, fVec2 & B1, fVec2 & B2, fVec2 & B3)
		{
		fVec2 H1 = (P2 - P1).get_rotate90(); H1.normalize();
		fVec2 H2 = (P3 - P2).get_rotate90(); H2.normalize();
		A1 = P1 + H1 * thickleft; const fVec2 A1b = A1 + (P2 - P1);
		A3 = P3 + H2 * thickleft; const fVec2 A3b = A3 + (P2 - P3);
		if (!intersection(A1, A1b, A3, A3b, A2)) { MTOOLS_DEBUG("no intersection for A2");  A2 = 0.5*((A1b - P2) + (A3b - P2)); } // approximation...
		B1 = P1 - H1 * thickright; const fVec2 B1b = B1 - (P2 - P1);
		B3 = P3 - H2 * thickright; const fVec2 B3b = B3 - (P2 - P3);
		if (!intersection(B1, B1b, B3, B3b, B2)) { MTOOLS_DEBUG("no intersection for B2");  B2 = 0.5*((B1b - P2) + (B3b - P2)); } // approximation...
		}


	/**
	 * Enlarge a polyline and return the corresponding clockwise polygon
	 *
	 * @param 		  	tabPoints 	points that constitute the polyline.
	 * @param 		  	thickleft 	left extension.
	 * @param 		  	thickright	right extension
	 * @param [in,out]	tabout	  	vector to store the polygon vertices.
	**/
	inline void polylinetoPolygon(const std::vector<fVec2> & tabPoints, double thickleft, double thickright,  std::vector<fVec2> & tabout)
		{
		const size_t l = tabPoints.size();
		MTOOLS_INSURE(l >= 3);
		const size_t N = 2*l - 1;
		tabout.resize(N + 1);
		for (size_t i = 1; i < l - 1; i++)
			{
			fVec2 A1, A3, B1, B3;
			_enlargeline(tabPoints[i - 1], tabPoints[i], tabPoints[i + 1], thickleft, thickright, A1, tabout[i], A3, B1, tabout[N-i], B3);
			if (i == 1) { tabout[0] = A1; tabout[N] = B1; }
			if (i == l - 2) { tabout[l - 1] = A3; tabout[N - (l - 1)] = B3; }
			}
		}


	/**
	 * Enlarge a polyline and return both side in separate arrays.
	 *
	 * @param 		  	tabPoints points that constitute the polyline
	 * @param 		  	thickleft 	left extension.
	 * @param 		  	thickright	right extension
	 * @param [in,out]	tabA	  left side extension
	 * @param [in,out]	tabB	  right side extension.
	 */	
	inline void enlargePolyline(const std::vector<fVec2> & tabPoints, double thickleft, double thickright, std::vector<fVec2> & tabA, std::vector<fVec2> & tabB)
		{
		const size_t l = tabPoints.size();
		if (l < 3) { MTOOLS_DEBUG("enlargePolyline called with less than 3 points");  tabA = tabPoints; tabB = tabPoints;  return; }
		tabA.resize(l); tabB.resize(l);
		for (size_t i = 1; i < l - 1; i++)
			{
			fVec2 A1, A3, B1, B3;
			_enlargeline(tabPoints[i - 1], tabPoints[i], tabPoints[i + 1], thickleft, thickright, A1, tabA[i], A3, B1, tabB[i], B3);
			if (i == 1) { tabA[0] = A1; tabB[0] = B1; }
			if (i == l - 2) { tabA[l - 1] = A3; tabB[l - 1] = B3; }
			}
		}


	/**
	* Enlarge a polygone by a given thickness. return the two resulting polygon. 
	*
	* @param 		  	tabPoints points that constitute the polyline
	* @param 		  	thickleft 	left extension.
	* @param 		  	thickright	right extension
	* @param [in,out]	tabA	  left side polygon
	* @param [in,out]	tabB	  right side polygon.
	*/
	inline void enlargePolygon(const std::vector<fVec2> & tabPoints, double thickleft, double thickright, std::vector<fVec2> & tabA, std::vector<fVec2> & tabB)
		{
		const size_t l = tabPoints.size();
		if (l < 3) { MTOOLS_DEBUG("enlargePolygon called with less than 3 points");  tabA = tabPoints; tabB = tabPoints;  return; }
		tabA.resize(l); tabB.resize(l);
		for (size_t i = 1; i < l - 1; i++)
			{
			fVec2 A1, A3, B1, B3;
			_enlargeline(tabPoints[i - 1], tabPoints[i], tabPoints[i + 1], thickleft, thickright, A1, tabA[i], A3, B1, tabB[i], B3);
			}
		fVec2 A1, A3, B1, B3;
		_enlargeline(tabPoints[l - 1], tabPoints[0], tabPoints[1], thickleft, thickright, A1, tabA[0], A3, B1, tabB[0], B3);
		_enlargeline(tabPoints[l - 2], tabPoints[l - 1], tabPoints[0], thickleft, thickright, A1, tabA[l - 1], A3, B1, tabB[l - 1], B3);
		}






	}

}


/* end of file */








