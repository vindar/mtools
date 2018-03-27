/** @file clipping.hpp */
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

#include "../../misc/error.hpp"
#include "../../misc/misc.hpp"
#include "../../maths/box.hpp"
#include "../../maths/vec.hpp"


namespace mtools
{



namespace internals_clipping
{


	/**
	* Test if point C in on the left side of the oriented line P1->P2
	*
	* @param	P1	start point of the oriented line.
	* @param	P2	endpoint of the oriented line.
	* @param	C 	Point to test.
	*
	* @return	1 if C is on the left, -1 if C is on the right and 0 if C in on the line itself.
	**/
	MTOOLS_FORCEINLINE int left_of(const fVec2 & P1, const fVec2 & P2, const fVec2 & C)
	{
		double x = crossProduct(P2 - P1, C - P2);
		return ((x < 0) ? -1 : ((x > 0) ? 1 : 0));
	}


	/**
	* Return the approximate winding Windings the given polygon
	*
	* Only works if the polygon is convex !
	*
	* @tparam	POLYGON_T	class such as std::vector<fVec2> that contain the vertices of the polygon.
	* @param	poly	The polygon to test
	*
	* @return	+1 if clockise, -1 if counterclockise, 0 if flat.
	**/
	template<typename POLYGON_T> int winding(const POLYGON_T & poly)
	{
		size_t L = poly.size();
		MTOOLS_ASSERT(poly.size() >= 3);
		for (size_t i = 0; i < L; i++)
		{
			int d = left_of(poly[i], poly[(i + 1) % L], poly[(i + 2) % L]);
			if (d != 0) return d;
		}
		MTOOLS_DEBUG("polygon with 0 winding !!!");
		return 0;
	}


	/**
	* Sub routine used by the Sutherland-Hodgman polygon clipping akgorithm.
	* clip all the vertices of sub against (x0,x1)
	**/
	template<typename POLYGON_T> void Sutherland_Hodgman_clipping_sub(const POLYGON_T & sub, const fVec2 x0, const fVec2 x1, const int left, POLYGON_T & res)
	{
		res.clear();
		const size_t L = sub.size();
		fVec2 v0 = sub[L - 1];
		int side0 = left_of(x0, x1, v0);
		if (side0 != -left) res.push_back(v0);
		for (size_t i = 0; i < L; i++)
		{
			fVec2 v1 = sub[i];
			int side1 = left_of(x0, x1, v1);
			if (side0 + side1 == 0 && side0)
			{
				fVec2 tmp;
				if (intersection(x0, x1, v0, v1, tmp)) res.push_back(tmp);
			}
			if (i == L - 1) break;
			if (side1 != -left) res.push_back(v1);
			v0 = v1;
			side0 = side1;
		}
	}


	/** Used by Colin-SutherLand Line clipping algorithm **/
	template<typename T> MTOOLS_FORCEINLINE int csLineClipCode(const Vec<T,2> & P, const Box<T,2> & B)
		{
		int c = 0;
		const T xx = P.X();
		const T yy = P.Y();
		if (xx < B.min[0]) c |= 1;
		if (xx > B.max[0]) c |= 2;
		if (yy < B.min[1]) c |= 4;
		if (yy > B.max[1]) c |= 8;
		return c;
		}

}




/**
 * Sutherland hodgman clipping algorithm. Clip a given polygon against another (convex) polygon.
 *
 * @tparam	POLYGON_T	Type of the polygon t, compatible with std::vector<fVec2>
 * @param 		  	sub 	The polygon to clip.
 * @param 		  	clip	The clipping region delimited by a convex polygon.
 * @param [in,out]	res 	Polygon to store the result (may be empty).
 **/
template<typename POLYGON_T> void Sutherland_Hodgman_clipping(const POLYGON_T &  sub, const POLYGON_T & clip, POLYGON_T & res)
	{
	res.clear();
	const size_t L = clip.size();
	POLYGON_T tmp;
	POLYGON_T * p1 = (L & 1) ? &tmp : &res;
	POLYGON_T * p2 = (L & 1) ? &res : &tmp;
	int dir = internals_clipping::winding(clip);
	internals_clipping::Sutherland_Hodgman_clipping_sub(sub, clip[L-1], clip[0], dir, *p2);
	for (size_t i = 0; i < L-1; i++) 		
		{
		mtools::swap<POLYGON_T*>(p1, p2);
		if (p1->size() == 0) { res.clear(); return; }
		internals_clipping::Sutherland_Hodgman_clipping_sub(*p1, clip[i], clip[i + 1], dir, *p2);
		}
	MTOOLS_ASSERT(p2 == &res);
	}


/**
 * Sutherland hodgman clipping algorithm. Clip a given polygon against a rectangle box.
 *
 * @tparam	POLYGON_T	Type of the polygon t, compatible with std::vector<fVec2>
 * @param 		  	sub		   	The polygon to clip.
 * @param 		  	clippingBox	The box to clip into.
 * @param [in,out]	res		   	Polygon to store the result (may be empty).
 **/
template<typename POLYGON_T> void Sutherland_Hodgman_clipping(const POLYGON_T &  sub, const fBox2 & clippingBox , POLYGON_T & res)
	{
	POLYGON_T clip;
	clip.reserve(4); 
	clip.push_back({ clippingBox.min[0], clippingBox.min[1] });
	clip.push_back({ clippingBox.max[0], clippingBox.min[1] });
	clip.push_back({ clippingBox.max[0], clippingBox.max[1] });
	clip.push_back({ clippingBox.min[0], clippingBox.max[1] });
	Sutherland_Hodgman_clipping(sub, clip, res);
	}





/**
* Cohen-Sutherland Line clipping algorithm (real valued lines)
*
* @param [in,out]	P1	The first point of the segment.
* @param [in,out]	P2	The second point of the segment.
* @param	B		  	The rectangle to clip into.
*
* @return	true if a line should be drawn and false if it should be discarded. If true, P1 and
* 			P2 are now inside the closed rectangle B and delimit the line to draw.
**/
bool Colin_SutherLand_lineclip(fVec2 & P1, fVec2 & P2, const fBox2 & B)
{
	int c1 = internals_clipping::csLineClipCode<double>(P1, B);
	int c2 = internals_clipping::csLineClipCode<double>(P2, B);
	while (1)
	{
		if ((c1 == 0) && (c2 == 0)) { return true; }		
		if ((c1 & c2) != 0) { return false; }
		int temp = (c1 == 0) ? c2 : c1;
		{
			double x = 0, y = 0;
			const double m = (P2.Y() - P1.Y()) / (P2.X() - P1.X());
			if (temp & 8)
			{ //Line clips top edge
				x = P1.X() + (B.max[1] - P1.Y()) / m;
				y = B.max[1];
			}
			else if (temp & 4)
			{ 	//Line clips bottom edge
				x = P1.X() + (B.min[1] - P1.Y()) / m;
				y = B.min[1];
			}
			else if (temp & 1)
			{ 	//Line clips left edge
				x = B.min[0];
				y = P1.Y() + (B.min[0] - P1.X()) * m;
			}
			else if (temp & 2)
			{ 	//Line clips right edge
				x = B.max[0];
				y = P1.Y() + (B.max[0] - P1.X()) * m;
			}
			if (temp == c1)
			{
				P1.X() = x; P1.Y() = y;
				c1 = internals_clipping::csLineClipCode<double>(P1, B);
			}
			else
			{
				P2.X() = x; P2.Y() = y;
				c2 = internals_clipping::csLineClipCode<double>(P2, B);
			}
		}
	}
}


/**
* Cohen-Sutherland Line clipping algorithm (integer valued lines)
*
* @param [in,out]	P1	The first point of the segment.
* @param [in,out]	P2	The second point of the segment.
* @param	B		  	The rectangle to clip into.
*
* @return	true if a line should be drawn and false if it should be discarded. If true, P1 and
* 			P2 are now inside the closed rectangle B and delimit the line to draw.
**/
bool Colin_SutherLand_lineclip(iVec2 & P1, iVec2 & P2, const iBox2 & B)
{
	int c1 = internals_clipping::csLineClipCode<int64>(P1, B);
	int c2 = internals_clipping::csLineClipCode<int64>(P2, B);
	while (1)
	{
		if ((c1 == 0) && (c2 == 0)) { return true; }		
		if ((c1 & c2) != 0) { return false; }
		int temp = (c1 == 0) ? c2 : c1;
		{
			int64 x = 0, y = 0;
			const double m = ((double)(P2.Y() - P1.Y())) / (P2.X() - P1.X());
			if (temp & 8)
			{ //Line clips top edge
				x = P1.X() + (int64)round((B.max[1] - P1.Y()) / m);
				y = B.max[1];
			}
			else if (temp & 4)
			{ 	//Line clips bottom edge
				x = P1.X() + (int64)round((B.min[1] - P1.Y()) / m);
				y = B.min[1];
			}
			else if (temp & 1)
			{ 	//Line clips left edge
				x = B.min[0];
				y = P1.Y() + (int64)round(m*(B.min[0] - P1.X()));
			}
			else if (temp & 2)
			{ 	//Line clips right edge
				x = B.max[0];
				y = P1.Y() + (int64)round(m*(B.max[0] - P1.X()));
			}
			if (temp == c1)
			{
				P1.X() = x; P1.Y() = y;
				c1 = internals_clipping::csLineClipCode<int64>(P1, B);
			}
			else
			{
				P2.X() = x; P2.Y() = y;
				c2 = internals_clipping::csLineClipCode<int64>(P2, B);
			}
		}
	}
}

}


/* end of file */








