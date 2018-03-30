
#include "mtools/mtools.hpp"
using namespace mtools;







class TestImage : public Image
{

public:


	/******************************************************************************************************************************************************
	*																				   																      *
	*                                                           BRESENHAM SEGMENT DRAWING                                                                 *
	*																																					  *
	*******************************************************************************************************************************************************/


	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}


	/** update a pixel on a bresenham segment */
	template<bool X_MAJOR, bool BLEND, bool CHECKRANGE, bool USEOP, bool USEPEN, int SIDE> MTOOLS_FORCEINLINE void _bseg_update_pixel(const internals_bseg::BSeg & seg, RGBc color, int32 op, int32 penwidth)
	{
		if (SIDE != 0)
			{
			int32 aa = seg.AA<SIDE,X_MAJOR>();
			if (USEOP) { aa *= op; aa >>= 8; }
			_updatePixel<BLEND, CHECKRANGE, true, USEPEN>(seg.X(), seg.Y(), color, aa, penwidth);
			}
		else
			{
			_updatePixel<BLEND, CHECKRANGE, USEOP, USEPEN>(seg.X(), seg.Y(), color, op, penwidth);
			}
	}


	/** Used by _bseg_draw */
	template<bool BLEND, bool USEOP, bool USEPEN, int SIDE> void _bseg_draw_template(internals_bseg::BSeg seg, bool draw_last, RGBc color, int32 penwidth, int32 op, bool checkrange = true)
		{
		if (draw_last) seg.inclen();
		if (checkrange)
			{
			const int64 of = ((USEPEN) && (penwidth > 0)) ? (penwidth + 2) : 0;
			iBox2 B(-of, _lx - 1 + of, -of, _ly - 1 + of);
			seg.move_inside_box(B);												// move inside the box
			seg.len() = std::min<int64>(seg.lenght_inside_box(B), seg.len());	// truncate to stay inside the box
			}
		if (seg.x_major())
			{
			const bool X_MAJOR = true;
			while (seg.len() > 0) { _bseg_update_pixel<X_MAJOR, BLEND, USEPEN, USEOP, USEPEN, SIDE>(seg, color, op, penwidth); seg.move<X_MAJOR>(); }
			}
		else
			{
			const bool X_MAJOR = false;
			while (seg.len() > 0) { _bseg_update_pixel<X_MAJOR, BLEND, USEPEN, USEOP, USEPEN, SIDE>(seg, color, op, penwidth); seg.move<X_MAJOR>(); }
			}
		}


	/**
	 * Draw a Bresenham segment.
	 *
	 * @param	seg		  	segment to draw.
	 * @param	draw_last 	true to draw the endpoint.
	 * @param	color	  	color.
	 * @param	penwidth  	(Optional) if positive, use larger pen.
	 * @param	blend	  	(Optional) true for blending.
	 * @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	 * @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	 * @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	 * 						is sure that the segment does not exit the image.
	 **/
	void _bseg_draw(const internals_bseg::BSeg & seg, bool draw_last, RGBc color, int32 penwidth = 0, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
	{
		if (op == 0) return;
		const bool useop = ((op > 0) && (op < 256));
		const bool usepen = (penwidth > 0);
		if (side > 0)
		{
			const int SIDE = 1;
			if (usepen)
			{
				const bool USEPEN = true;
				if (useop)
				{
					const bool USEOP = true;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
				else
				{
					const bool USEOP = false;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
			}
			else
			{
				const bool USEPEN = false;
				if (useop)
				{
					const bool USEOP = true;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
				else
				{
					const bool USEOP = false;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
			}
		}
		else if (side < 0)
		{
			const int SIDE = -1;
			if (usepen)
			{
				const bool USEPEN = true;
				if (useop)
				{
					const bool USEOP = true;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
				else
				{
					const bool USEOP = false;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
			}
			else
			{
				const bool USEPEN = false;
				if (useop)
				{
					const bool USEOP = true;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
				else
				{
					const bool USEOP = false;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
			}
		}
		else
		{
			const int SIDE = 0;
			if (usepen)
			{
				const bool USEPEN = true;
				if (useop)
				{
					const bool USEOP = true;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
				else
				{
					const bool USEOP = false;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
			}
			else
			{
				const bool USEPEN = false;
				if (useop)
				{
					const bool USEOP = true;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
				else
				{
					const bool USEOP = false;
					if (blend)
					{
						const bool BLEND = true;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
					else
					{
						const bool BLEND = false;
						_bseg_draw_template<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
					}
				}
			}
		}
	}


	/** used by _bseg_avoid1 */
	template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid1_template(internals_bseg::BSeg segA, bool lastA, internals_bseg::BSeg segB, bool lastB, RGBc color, int32 op, bool checkrange = true)
		{
		MTOOLS_ASSERT(segA == segB); // same start position
		if (lastA) segA.inclen();
		if (lastB) segB.inclen();
		if (checkrange)
			{
			iBox2 B(0, _lx - 1, 0, _ly - 1);
			int64 r = segA.move_inside_box(B);
			if (segA.len() <= 0) return;
			segB.move(r);																// move the second line by the same amount.
			segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());		// truncate to stay inside the box
			}
		int64 lena = segA.len() - 1;
		int64 lenb = segB.len() - 1;
		int64 l = 0;
		if (segA.x_major())
			{
			const bool X_MAJOR = true;
			while (l <= lena)
				{
				if ((l > lenb) || (segA != segB)) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); l++;
				}
			}
		else
			{
			const bool X_MAJOR = false;
			while (l <= lena)
				{
				if ((l > lenb) || (segA != segB)) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); l++;
				}
			}
		}


	/**
	 * Draw the bresenham segment segA while avoiding segB
	 *
	 *            /
	 *          B/
	 *          /
	 *         +------A-------
	 *
	 * @param	segA	  	segment to draw.
	 * @param	lastA	  	true to consider the closed segment.
	 * @param	segB	  	segment to avoid : must share the same start pixel as segA.
	 * @param	lastB	  	true to consider the closed segment.
	 * @param	color	  	color to use.
	 * @param	blend	  	(Optional) true to use blending.
	 * @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	 * @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	 * @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	 * 						is sure that the segment does not exit the image.
	 **/
	void _bseg_avoid1(const internals_bseg::BSeg & segA, bool lastA, const internals_bseg::BSeg & segB, bool lastB, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
	{
		if (op == 0) return;
		const bool useop = ((op > 0) && (op < 256));
		if (side > 0)
		{
			const int SIDE = 1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
			}
		}
		else if (side < 0)
		{
			const int SIDE = -1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
			}
		}
		else
		{
			const int SIDE = 0;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid1_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, color, op, checkrange);
				}
			}
		}
	}


	/** Used by _bseg_avoid2 */
	template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid2_template(internals_bseg::BSeg segA, bool lastA, internals_bseg::BSeg segB, bool lastB, internals_bseg::BSeg segC, bool lastC, RGBc color, int32 op, bool checkrange)
		{
		MTOOLS_ASSERT(segA == segB);
		MTOOLS_ASSERT(segA == segC);
		if (lastA) segA.inclen();
		if (lastB) segB.inclen();
		if (lastC) segC.inclen();
		if (checkrange)
			{
			iBox2 B(0, _lx - 1, 0, _ly - 1);
			int64 r = segA.move_inside_box(B);
			if (segA.len() <= 0) return;
			segB.move(r);																// move the second line by the same amount.
			segC.move(r);																// move the third line by the same amount.
			segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());		// truncate to stay inside the box
			}
		int64 lena = segA.len() - 1;
		int64 lenb = segB.len() - 1;
		int64 lenc = segC.len() - 1;
		int64 l = 0;
		if (segA.x_major())
			{
			const bool X_MAJOR = true;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segC.move(); l++;
				}
			}
		else
			{
			const bool X_MAJOR = false;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segC.move(); l++;
				}
			}
		}


	/**
	 * Draw the bresenham segment segA while avoiding segB and segC
	 *
	 *      \     /        
	 *      C\   /B        
	 *        \ /          
	 *         +------A-------
	 *          
	 * @param	segA	  	segment to draw.
	 * @param	lastA	  	true to consider the closed segment.
	 * @param	segB	  	first segment to avoid : must share the same start pixel as segA.
	 * @param	lastB	  	true to consider the closed segment.
	 * @param	segC	  	second segment to avoid : must share the same start pixel as segA.
	 * @param	lastC	  	true to consider the closed segment.
	 * @param	color	  	color to use.
	 * @param	blend	  	(Optional) true to use blending.
	 * @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	 * @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	 * @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	 * 						is sure that the segment does not exit the image.
	 **/
	void _bseg_avoid2(const internals_bseg::BSeg & segA, bool lastA, const internals_bseg::BSeg & segB, bool lastB, const internals_bseg::BSeg & segC, bool lastC, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
	{
		if (op == 0) return;
		const bool useop = ((op > 0) && (op < 256));
		if (side > 0)
		{
			const int SIDE = 1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
			}
		}
		else if (side < 0)
		{
			const int SIDE = -1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
			}
		}
		else
		{
			const int SIDE = 0;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid2_template<BLEND, USEOP, SIDE>(segA, lastA, segB, lastB, segC, lastC, color, op, checkrange);
				}
			}
		}
	}


	/** Used by _bseg_avoid11 */
	template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid11_template(internals_bseg::BSeg segA, internals_bseg::BSeg segB, bool lastB, internals_bseg::BSeg segD, bool lastD, RGBc color, int32 op, bool checkrange)
	{
		MTOOLS_ASSERT(segA == segB);

		if (lastB) segB.inclen();

		int64 dd = (segA.len() - segD.len()) + (lastD ? 0 : 1); segD.len() = segA.len(); segD.reverse();	// D is now synchronized with A

		if (checkrange)
			{
			iBox2 B(0, _lx - 1, 0, _ly - 1);
			int64 r = segA.move_inside_box(B);
			if (segA.len() <= 0) return;
			segB.move(r);																// move the second line by the same amount.
			segD.move(r); dd -= r;														// move the third line by the same amount.
			segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());		// truncate to stay inside the box
			}

		int64 lena = segA.len() - 1;
		int64 lenb = segB.len() - 1;
		int64 l = 0;
		if (segA.x_major())
			{
			const bool X_MAJOR = true;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l < dd) || (segA != segD))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segD.move(); l++;
				}
			}
		else
			{
			const bool X_MAJOR = false;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l < dd) || (segA != segD))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segD.move(); l++;
				}
			}
	}


	/**
	* Draw the bresenham segment segA while avoiding segB and segD (at opposite ends)
	*
	*            /        \
	*          B/          \D
	*          /            \
	*         +------A-------+
	* 
	* @param	segA	  	segment to draw.
	* @param	segB	  	first segment to avoid : must share the same start pixel as segA.
	* @param	lastB	  	true to consider the closed segment.
	* @param	segD	  	second segment to avoid : its start pixel must be the end pixel of segA.
	* @param	lastD	  	true to consider the closed segment.
	* @param	color	  	color to use.
	* @param	blend	  	(Optional) true to use blending.
	* @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	* @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	* @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	* 						is sure that the segment does not exit the image.
	**/
	void _bseg_avoid11(const internals_bseg::BSeg & segA, const internals_bseg::BSeg & segB, bool lastB, const internals_bseg::BSeg & segD, bool lastD, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
	{
		if (op == 0) return;
		const bool useop = ((op > 0) && (op < 256));
		if (side > 0)
		{
			const int SIDE = 1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
			}
		}
		else if (side < 0)
		{
			const int SIDE = -1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
			}
		}
		else
		{
			const int SIDE = 0;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid11_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segD, lastD, color, op, checkrange);
				}
			}
		}
	}


	/** Used by _bseg_avoid21 */
	template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid21_template(internals_bseg::BSeg segA, internals_bseg::BSeg segB, bool lastB, internals_bseg::BSeg segC, bool lastC, internals_bseg::BSeg segD, bool lastD, RGBc color, int32 op, bool checkrange)
		{
		MTOOLS_ASSERT(segA == segB);
		MTOOLS_ASSERT(segA == segC);

		if (lastB) segB.inclen();
		if (lastC) segC.inclen();

		int64 dd = (segA.len() - segD.len()) + (lastD ? 0 : 1); segD.len() = segA.len(); segD.reverse();	// D is now synchronized with A

		if (checkrange)
			{
			iBox2 B(0, _lx - 1, 0, _ly - 1);
			int64 r = segA.move_inside_box(B);
			if (segA.len() <= 0) return;
			segB.move(r);
			segC.move(r);
			segD.move(r); dd -= r;
			segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());
			}

		int64 lena = segA.len() - 1;
		int64 lenb = segB.len() - 1;
		int64 lenc = segC.len() - 1;
		int64 l = 0;
		if (segA.x_major())
			{
			const bool X_MAJOR = true;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC)) && ((l < dd) || (segA != segD))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segC.move(); segD.move(); l++;
				}
			}
		else
			{
			const bool X_MAJOR = false;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC)) && ((l < dd) || (segA != segD))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segC.move(); segD.move(); l++;
				}
			}
		}


	/**
	* Draw the bresenham segment segA while avoiding segB, segC and segD
	*     
	*      \     /              /
	*      C\   /B             /D
	*        \ /              /
	*         +------A-------+
	*
	* @param	segA	  	segment to draw.
	* @param	segB	  	first segment to avoid : must share the same start pixel as segA.
	* @param	lastB	  	true to consider the closed segment.
	* @param	segC	  	second segment to avoid : must share the same start pixel as segA.
	* @param	lastC	  	true to consider the closed segment.
	* @param	segD	  	third segment to avoid : its start pixel must be the end pixel of segA.
	* @param	lastD	  	true to consider the closed segment.
	* @param	color	  	color to use.
	* @param	blend	  	(Optional) true to use blending.
	* @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	* @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	* @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	* 						is sure that the segment does not exit the image.
	**/
	void _bseg_avoid21(const internals_bseg::BSeg & segA, const internals_bseg::BSeg & segB, bool lastB, const internals_bseg::BSeg & segC, bool lastC, const internals_bseg::BSeg & segD, bool lastD, 
		               RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
	{
		if (op == 0) return;
		const bool useop = ((op > 0) && (op < 256));
		if (side > 0)
		{
			const int SIDE = 1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
			}
		}
		else if (side < 0)
		{
			const int SIDE = -1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
			}
		}
		else
		{
			const int SIDE = 0;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid21_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, color, op, checkrange);
				}
			}
		}
	}


	/** Used by _bseg_avoid22 */
	template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid22_template(internals_bseg::BSeg segA, internals_bseg::BSeg segB, bool lastB, internals_bseg::BSeg segC, bool lastC, internals_bseg::BSeg segD, bool lastD, internals_bseg::BSeg segE, bool lastE, RGBc color, int32 op, bool checkrange)
		{
		MTOOLS_ASSERT(segA == segB);
		MTOOLS_ASSERT(segA == segC);

		if (lastB) segB.inclen();
		if (lastC) segC.inclen();

		int64 dd = (segA.len() - segD.len()) + (lastD ? 0 : 1); segD.len() = segA.len(); segD.reverse();	// D is now synchronized with A
		int64 ee = (segA.len() - segE.len()) + (lastE ? 0 : 1); segE.len() = segA.len(); segE.reverse();	// E is now synchronized with A

		if (checkrange)
			{
			iBox2 B(0, _lx - 1, 0, _ly - 1);
			int64 r = segA.move_inside_box(B);
			if (segA.len() <= 0) return;
			segB.move(r);
			segC.move(r);
			segD.move(r); dd -= r;
			segE.move(r); ee -= r;
			segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());
			}

		int64 lena = segA.len() - 1;
		int64 lenb = segB.len() - 1;
		int64 lenc = segC.len() - 1;
		int64 l = 0;
		if (segA.x_major())
			{
			const bool X_MAJOR = true;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC)) && ((l < dd) || (segA != segD)) && ((l < ee) || (segA != segE))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segC.move(); segD.move(); segE.move(); l++;
			}
			}
		else
			{
			const bool X_MAJOR = false;
			while (l <= lena)
				{
				if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC)) && ((l < dd) || (segA != segD)) && ((l < ee) || (segA != segE))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
				segA.move<X_MAJOR>(); segB.move(); segC.move(); segD.move(); segE.move(); l++;
				}
			}
		}


	/**
	* Draw the bresenham segment segA while avoiding segB, segC and segD
	*
	*      \     /        \     /
	*      C\   /B        E\   /D
	*        \ /            \ /
	*         +------A-------+
	*
	* @param	segA	  	segment to draw.
	* @param	segB	  	first segment to avoid : must share the same start pixel as segA.
	* @param	lastB	  	true to consider the closed segment.
	* @param	segC	  	second segment to avoid : must share the same start pixel as segA.
	* @param	lastC	  	true to consider the closed segment.
	* @param	segD	  	third segment to avoid : its start pixel must be the end pixel of segA.
	* @param	lastD	  	true to consider the closed segment.
	* @param	segE	  	third segment to avoid : its start pixel must be the end pixel of segA.
	* @param	lastE	  	true to consider the closed segment.
	* @param	color	  	color to use.
	* @param	blend	  	(Optional) true to use blending.
	* @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	* @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	* @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	* 						is sure that the segment does not exit the image.
	**/
	void _bseg_avoid22(const internals_bseg::BSeg & segA, bool lastA, const internals_bseg::BSeg & segB, bool lastB, const internals_bseg::BSeg & segC, bool lastC, const internals_bseg::BSeg & segD, bool lastD, const internals_bseg::BSeg & segE, bool lastE,
		               RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
	{
		if (op == 0) return;
		const bool useop = ((op > 0) && (op < 256));
		if (side > 0)
		{
			const int SIDE = 1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
			}
		}
		else if (side < 0)
		{
			const int SIDE = -1;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
			}
		}
		else
		{
			const int SIDE = 0;
			if (useop)
			{
				const bool USEOP = true;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
			}
			else
			{
				const bool USEOP = false;
				if (blend)
				{
					const bool BLEND = true;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
				else
				{
					const bool BLEND = false;
					_bseg_avoid22_template<BLEND, USEOP, SIDE>(segA, segB, lastB, segC, lastC, segD, lastD, segE, lastE, color, op, checkrange);
				}
			}
		}
	}


	/** Used by _bseg_fill_triangle */
	void _bseg_fill_interior_angle(iVec2 P, iVec2 Q1, iVec2 Q2, internals_bseg::BSeg & seg1, internals_bseg::BSeg & seg2, RGBc color, bool fill_last, bool blend, bool checkrange)
	{
		MTOOLS_ASSERT((P.Y() - Q1.Y())*(P.Y() - Q2.Y()) > 0);
		int64 dir = (P.Y() > Q1.Y()) ? -1 : 1;
		int64 y = P.Y();
		int64 ytarget = Q1.Y() + dir * (fill_last ? 1 : 0);
		if ((Q1.X() - P.X())*abs(Q2.Y() - P.Y())  > (Q2.X() - P.X())*abs(Q1.Y() - P.Y())) mtools::swap(seg1, seg2);
		if (checkrange)
		{
			const bool CHECKRANGE = true;
			if (blend)
			{
				const bool BLEND = true;
				_bseg_fill_interior_angle_sub<BLEND, CHECKRANGE>(dir, y, ytarget, seg1, seg2, color);
			}
			else
			{
				const bool BLEND = false;
				_bseg_fill_interior_angle_sub<BLEND, CHECKRANGE>(dir, y, ytarget, seg1, seg2, color);
			}
		}
		else
		{
			const bool CHECKRANGE = false;
			if (blend)
			{
				const bool BLEND = true;
				_bseg_fill_interior_angle_sub<BLEND, CHECKRANGE>(dir, y, ytarget, seg1, seg2, color);
			}
			else
			{
				const bool BLEND = false;
				_bseg_fill_interior_angle_sub<BLEND, CHECKRANGE>(dir, y, ytarget, seg1, seg2, color);
			}
		}
	}


	/** Used by _bseg_fill_triangle */
	template<bool BLEND, bool CHECKRANGE> void _bseg_fill_interior_angle_sub(int64 dir, int64 y, int64 ytarget, internals_bseg::BSeg & sega, internals_bseg::BSeg & segb, RGBc color)
	{
		// fix the range. 
		if (dir > 0)
		{
			if (ytarget >= _ly) { ytarget = _ly; }
			if ((ytarget <= 0) || (y >= ytarget)) return;
			if (y < 0)
			{ // move y up to 0
				sega.move_y_dir(-y);
				segb.move_y_dir(-y);
				y = 0;
				MTOOLS_ASSERT((sega.Y() == y) && (segb.Y() == y));
			}
		}
		else
		{
			if (ytarget < 0) { ytarget = -1; }
			if ((ytarget >= _ly - 1) || (y <= ytarget)) return;
			if (y > _ly - 1)
			{ // move y down to ly-1
				sega.move_y_dir(y - _ly + 1);
				segb.move_y_dir(y - _ly + 1);
				y = _ly - 1;
				MTOOLS_ASSERT((sega.Y() == y) && (segb.Y() == y));
			}
		}
		if (sega.x_major())
		{
			if (segb.x_major())
			{
				if (sega.step_x() < 0)
				{
					if (segb.step_x() > 0)
					{
						while (y != ytarget)
						{
							_hline<BLEND, CHECKRANGE>(sega.X() + 1, segb.X() - 1, y, color);
							sega.move_y_dir<true>();
							segb.move_y_dir<true>();
							y += dir;
						}
					}
					else
					{
						while (y != ytarget)
						{
							segb.move_y_dir<true>();
							_hline<BLEND, CHECKRANGE>(sega.X() + 1, segb.X(), y, color);
							sega.move_y_dir<true>();
							y += dir;
						}
					}
				}
				else
				{
					if (segb.step_x() > 0)
					{
						while (y != ytarget)
						{
							sega.move_y_dir<true>();
							_hline<BLEND, CHECKRANGE>(sega.X(), segb.X() - 1, y, color);
							segb.move_y_dir<true>();
							y += dir;
						}
					}
					else
					{
						while (y != ytarget)
						{
							sega.move_y_dir<true>();
							segb.move_y_dir<true>();
							_hline<BLEND, CHECKRANGE>(sega.X(), segb.X(), y, color);
							y += dir;
						}
					}
				}
			}
			else
			{
				if (sega.step_x() < 0)
				{
					while (y != ytarget)
					{
						_hline<BLEND, CHECKRANGE>(sega.X() + 1, segb.X() - 1, y, color);
						sega.move_y_dir<true>();
						segb.move_y_dir<false>();
						y += dir;
					}
				}
				else
				{
					while (y != ytarget)
					{
						sega.move_y_dir<true>();
						_hline<BLEND, CHECKRANGE>(sega.X(), segb.X() - 1, y, color);
						segb.move_y_dir<false>();
						y += dir;
					}
				}
			}
		}
		else
		{
			if (segb.x_major())
			{
				if (segb.step_x() > 0)
				{
					while (y != ytarget)
					{
						_hline<BLEND, CHECKRANGE>(sega.X() + 1, segb.X() - 1, y, color);
						segb.move_y_dir<true>();
						sega.move_y_dir<false>();
						y += dir;
					}
				}
				else
				{
					while (y != ytarget)
					{
						segb.move_y_dir<true>();
						_hline<BLEND, CHECKRANGE>(sega.X() + 1, segb.X(), y, color);
						sega.move_y_dir<false>();
						y += dir;
					}
				}
			}
			else
			{
				while (y != ytarget)
				{
					_hline<BLEND, CHECKRANGE>(sega.X() + 1, segb.X() - 1, y, color);
					segb.move_y_dir<false>();
					sega.move_y_dir<false>();
					y += dir;
				}
			}
		}
	}


	/**
	 * Fill the interior of a triangle (fP1, fP2, fP3) delimited by bresenham segments. Only the
	 * interior is filled (segment are not drawn over).
	 *
	 * @param	fP1		  	first point.
	 * @param	fP2		  	second point.
	 * @param	fP3		  	third point.
	 * @param	fillcolor 	color to use.
	 * @param	blend	  	(Optional) true to use blending.
	 * @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	 * 						is sure that the triangle is completely inside the image.
	 **/
	void _bseg_fill_triangle(fVec2 fP1, fVec2 fP2, fVec2 fP3, RGBc fillcolor, bool blend = true, bool checkrange = true)
		{
		if (fP1.Y() > fP2.Y()) { mtools::swap(fP1, fP2); } // reorder by increasing Y value
		if (fP1.Y() > fP3.Y()) { mtools::swap(fP1, fP3); } //
		if (fP2.Y() > fP3.Y()) { mtools::swap(fP2, fP3); } //
		iVec2 P1 = round(fP1); int64 y1 = P1.Y();
		iVec2 P2 = round(fP2); int64 y2 = P2.Y();
		iVec2 P3 = round(fP3); int64 y3 = P3.Y();		
		if (y1 == y3) return; //flat, nothing to draw. 
		if (y1 == y2)
			{
			internals_bseg::BSeg seg31(fP3, fP1);
			internals_bseg::BSeg seg32(fP3, fP2);
			_bseg_fill_interior_angle(P3, P1, P2, seg31, seg32, fillcolor, false, blend, checkrange);
			return;
			}
		if (y2 == y3)
			{
			internals_bseg::BSeg seg12(fP1, fP2);
			internals_bseg::BSeg seg13(fP1, fP3);
			_bseg_fill_interior_angle(P1, P2, P3, seg12, seg13, fillcolor, false, blend, checkrange);
			return;
			}
		internals_bseg::BSeg seg12(fP1, fP2);
		internals_bseg::BSeg seg13(fP1, fP3);
		internals_bseg::BSeg seg23(fP2, fP3);
		internals_bseg::BSeg seg21 = seg12.get_reverse();
		internals_bseg::BSeg seg31 = seg13.get_reverse();
		internals_bseg::BSeg seg32 = seg23.get_reverse();

		bool fl3;
		fVec2 vA = (fP3 - fP1), vB = (fP2 - fP1);
		double det = vA.X()*vB.Y() - vB.X()*vA.Y();
		seg23.move_y_dir(1);
		seg21.move_y_dir(1);
		if (det < 0) 
			{ 
			fl3 = (seg23.X() < seg21.X()) ? true : false; 
			}
		else 
			{
			fl3 = (seg23.X() > seg21.X()) ? true : false;
			}

		_bseg_fill_interior_angle(P3, P2, P1, seg32, seg31, fillcolor, fl3, blend, checkrange);
		_bseg_fill_interior_angle(P1, P2, P3, seg12, seg13, fillcolor, !fl3, blend, checkrange);
		return;
		}





};



int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

	RGBc RR = RGBc::c_Red.getMultOpacity(0.5);
	RGBc GG = RGBc::c_Green.getMultOpacity(0.5);
	RGBc BB = RGBc::c_Blue.getMultOpacity(0.5);

	RGBc FF = RGBc::c_Yellow.getMultOpacity(0.5f);

	int64 L = 50;

	TestImage im(L, L);
	im.clear(RGBc(240,240,240));

	using namespace internals_bseg;




	fVec2 P1(10,10);

	fVec2 P2(37.49,25.51);

	fVec2 P3(13,20.99);

	im._bseg_draw(BSeg(P1, P2), true, RR);
	im._bseg_avoid1(BSeg(P1, P3), true, BSeg(P1, P2), true, GG);
	im._bseg_avoid11(BSeg(P2, P3), BSeg(P2, P1), true, BSeg(P3, P1), true, BB);

	im._bseg_fill_triangle(P1, P2, P3, FF);


	/*
	im.blendPixel({ 40, 9 }, BB);
	im.blendPixel({ 10, 9 }, BB);
	im.blendPixel({ 20, 9 }, BB);
	im.blendPixel({ 30, 9 }, BB);
	*/
	Plotter2D plotter;
	auto P = makePlot2DImage(im);
	plotter[P];
	plotter.range().setRange(fBox2{ -0.5, L - 0.5, -0.5, L - 0.5});

	plotter.gridObject(true);
	plotter.gridObject()->setUnitCells();

	plotter.plot();

	return 0;
	}

