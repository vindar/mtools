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
	* Sub routine used by the Sutherland-Hodgman polygon clipping akgorithm.
	* clip all the vertices of sub against (x0,x1)
	**/
	inline void Sutherland_Hodgman_clipping_sub(const fVec2 * sub_tab, const size_t sub_len, const fVec2 x0, const fVec2 x1, const int left, fVec2 * res_tab, size_t & res_len)
		{
		res_len = 0;
		fVec2 v0 = sub_tab[sub_len - 1];
		int side0 = left_of(x0, x1, v0);
		if (side0 != -left) { res_tab[res_len++] = v0; }
		for (size_t i = 0; i < sub_len; i++)
			{
			fVec2 v1 = sub_tab[i];
			int side1 = left_of(x0, x1, v1);
			if (side0 + side1 == 0 && side0)
				{
				fVec2 tmp;
				if (intersection(x0, x1, v0, v1, tmp)) res_tab[res_len++] = tmp;
				}
			if (i == sub_len - 1) break;
			if (side1 != -left) res_tab[res_len++] = v1;
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
 * Return the winding direction of a convex polygon
 *
 * @param	poly_tab	The polygon vertices
 * @param	poly_len	number of vertices
 *
 * @return	+1 if clockise, -1 if counterclockise, 0 if not convex or flat
**/
inline int winding(const fVec2 * poly_tab, const size_t poly_len)
	{
	if (poly_len < 3) return 0;
	size_t a = 0, b = 0;
	for (size_t i = 0; i < poly_len; i++)
		{
		const int d = internals_clipping::left_of(poly_tab[i], poly_tab[(i + 1) % poly_len], poly_tab[(i + 2) % poly_len]);
		switch (d)
			{
			case +1: { a++; break; }
			case -1: { b++; break; }
			}
		}
	if ((a > 0) && (b == 0)) return 1;
	if ((b > 0) && (a == 0)) return -1;
	return 0;
	}


/**
 * Return the winding direction of a convex polygon.
 *
 * @param	polygon	The polygon to compute the winding direction.
 *
 * @return	+1 if clockise, -1 if counterclockise, 0 if not convex or flat.
**/
inline int winding(const std::vector<fVec2> & polygon)	
	{
	return winding(polygon.data(), polygon.size());
	}


/**
 * Sutherland Hodgman clipping algorithm. Clip a given polygon against another (convex) polygon.
 *
 * Polygons can be given in clockwise or anti-clockwise order (the resulting clipped polygon)
 * has the same order.
 *
 * Warning : the resulting polygon may have parallel adjacent edges. 
 *
 * @param 		  	sub_tab 	vertices of the polygon to clip.
 * @param 		  	sub_len 	number of vertices of the polygon to clip
 * @param 		  	clip_tab	vertices of convex polygon that delimit the clipping region.
 * @param 		  	clip_len	number of vertices of the clipping region.
 * @param [in,out]	res_tab 	array to store the list of vertices of the clipped polygon.
 * 					            (size 2*sub_len + clip_len is always enough)
 * @param [in,out]	res_len 	placeholder to store the number of vertices.
 **/
inline void Sutherland_Hodgman_clipping(const fVec2  * sub_tab , const size_t sub_len, 
	                                    const fVec2  * clip_tab, const size_t clip_len, 
	                                    fVec2  * res_tab , size_t & res_len)
	{
	MTOOLS_ASSERT(clip.len > 0);
	res_len = 0;
	if (sub_len == 0) return;
	static const size_t STATIC_MAX_SIZE = 16;
	fVec2   tmp1[STATIC_MAX_SIZE];
	fVec2 * tmp2 =  ((2 * sub_len + clip_len > STATIC_MAX_SIZE) ?  (new fVec2[2*sub_len + clip_len]) : nullptr);
	fVec2 * tmp_tab = ((tmp2 != nullptr) ? tmp2 : tmp1);	
	const size_t L = clip_len;
	fVec2 * p1 = (L & 1) ? tmp_tab : res_tab; size_t l1 = 0;
	fVec2 * p2 = (L & 1) ? res_tab : tmp_tab; size_t l2 = 0;
	int dir = winding(clip_tab, clip_len);
	internals_clipping::Sutherland_Hodgman_clipping_sub(sub_tab, sub_len, clip_tab[L-1], clip_tab[0], dir, p2, l2);
	for (size_t i = 0; i < L-1; i++) 		
		{
		mtools::swap<fVec2*>(p1, p2);
		mtools::swap<size_t>(l1, l2);
		if (l1 == 0) { res_len = 0; delete[] tmp2; return; }
		internals_clipping::Sutherland_Hodgman_clipping_sub(p1, l1, clip_tab[i], clip_tab[i + 1], dir, p2, l2);
		}
	res_len = l2;
	MTOOLS_ASSERT(p2 == res_tab);
	delete [] tmp2;
	}


/**
 * Sutherland Hodgman clipping algorithm. Clip a given polygon against another (convex) polygon.
 * 
 * Polygons can be given in clockwise or anti-clockwise order (the resulting clipped polygon)
 * has the same order.
 * 
 * Warning : the resulting polygon may have parallel adjacent edges.
 *
 * @param	polygon	   	the polygon to clip.
 * @param	clip_region	convex polygonal region to clip against.
 *
 * @return	the clipped polygon.
**/
std::vector<fVec2> Sutherland_Hodgman_clipping(const std::vector<fVec2> & polygon,
										const std::vector<fVec2> & clip_region)
	{
	if (polygon.size() == 0) return;
	std::vector<fVec2> res(2 * polygon.size() + clip_region.size());
	size_t len = 0;
	Sutherland_Hodgman_clipping(polygon.data(), polygon.size(), clip_region.data(), clip_region.size(), res.data(), len);
	res.resize(len);
	return res;
	}


/**
 * Sutherland hodgman clipping algorithm. Clip a given polygon against a rectangle box.
 *
 * Polygon may be given in clockwise or anti-clockwise order (the resulting clipped polygon)
 * has the same order.
 *
 * Warning : the resulting polygon may have two parrallel adjacent edges. 
 *
 * @param 		  	sub_tab	   	list of verticve of polygon to clip.
 * @param 		  	sub_len	   	number of vertices.
 * @param 		  	clippingBox	the box region to clip into.
 * @param [in,out]	res_tab	   	array to store the list of vertices of the clipped polygon.
 * 					            (must be of size at least 2*sub_len + 4 to be sure)
 * @param [in,out]	res_len	   	store the number of vertices.
 **/
inline void Sutherland_Hodgman_clipping(const fVec2  * sub_tab, const size_t sub_len,
	                                    const fBox2 & clippingBox,
		   							    fVec2  * res_tab, size_t & res_len)
	{
	fVec2 clip_tab[4] = {
		{ clippingBox.min[0], clippingBox.min[1] },
		{ clippingBox.max[0], clippingBox.min[1] },
		{ clippingBox.max[0], clippingBox.max[1] },
		{ clippingBox.min[0], clippingBox.max[1] } };
	Sutherland_Hodgman_clipping(sub_tab,sub_len, clip_tab, 4, res_tab, res_len);
	}


/**
 * Sutherland hodgman clipping algorithm. Clip a given polygon against a rectangle box.
 *
 * Polygon may be given in clockwise or anti-clockwise order (the resulting clipped polygon)
 * has the same order.
 *
 * Warning : the resulting polygon may have two parrallel adjacent edges. 
 *
 * @param	polygon	   	The polygon to clip
 * @param	clippingBox	The clipping box
 *
 * @return a vector containing the clipped polygon.
**/
std::vector<fVec2> Sutherland_Hodgman_clipping(const std::vector<fVec2> & polygon, const fBox2 & clippingBox)
	{
	MTOOLS_ASSERT(!clippingBox.isEmpty());
	if (polygon.size() == 0) return;
	fVec2 clip_tab[4] = { { clippingBox.min[0], clippingBox.min[1] },
	                      { clippingBox.max[0], clippingBox.min[1] },
	                      { clippingBox.max[0], clippingBox.max[1] },
	                      { clippingBox.min[0], clippingBox.max[1] } };
	std::vector<fVec2> res(2*polygon.size() + 4);
	size_t len = 0;
	Sutherland_Hodgman_clipping(polygon.data(), polygon.size(), clip_tab, 4, res.data(), len);
	res.resize(len);
	return res;
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
inline bool Colin_SutherLand_lineclip(fVec2 & P1, fVec2 & P2, const fBox2 & B)
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
inline bool Colin_SutherLand_lineclip(iVec2 & P1, iVec2 & P2, const iBox2 & B)
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
				x = P1.X() + (int64)std::round((B.max[1] - P1.Y()) / m);
				y = B.max[1];
			}
			else if (temp & 4)
			{ 	//Line clips bottom edge
				x = P1.X() + (int64)std::round((B.min[1] - P1.Y()) / m);
				y = B.min[1];
			}
			else if (temp & 1)
			{ 	//Line clips left edge
				x = B.min[0];
				y = P1.Y() + (int64)std::round(m*(B.min[0] - P1.X()));
			}
			else if (temp & 2)
			{ 	//Line clips right edge
				x = B.max[0];
				y = P1.Y() + (int64)std::round(m*(B.max[0] - P1.X()));
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








