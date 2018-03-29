
#include "mtools/mtools.hpp"
using namespace mtools;







class testImage : public Image
{




	/******************************************************************************************************************************************************
	*																				   																      *
	*                                                           BRESENHAM SEGMENT DRAWING                                                                 *
	*																																					  *
	*******************************************************************************************************************************************************/



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
			while (seg.len() > 0) { _bseg_update_pixel<X_MAJOR, BLEND, USEPEN, USEOP, USEPEN, SIDE>(seg, color, op, penwidth); seg.move<true>(); }
			}
		else
			{
			const bool X_MAJOR = false;
			while (seg.len() > 0) { _bseg_update_pixel<X_MAJOR, BLEND, USEPEN, USEOP, USEPEN, SIDE>(seg, color, op, penwidth); seg.move<false>(); }
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
			if (segB.x_major())
			{
				while (l <= lena)
				{ 
					if ((l > lenb) || (segA != segB)) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
					segA.move<true>(); segB.move<true>(); l++;
				}
			}
			else
			{
				while (l <= lena)
				{
					if ((l > lenb) || (segA != segB)) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
					segA.move<true>(); segB.move<false>(); l++;
				}
			}
		}
		else
		{
			const bool X_MAJOR = false;
			if (segB.x_major())
			{
				while (l <= lena)
				{
					if ((l > lenb) || (segA != segB)) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
					segA.move<false>(); segB.move<true>(); l++;
				}
			}
			else
			{
				while (l <= lena)
				{
					if ((l > lenb) || (segA != segB)) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
					segA.move<false>(); segB.move<false>(); l++;
				}
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
			if (segB.x_major())
			{
				if (segC.x_major())
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<true>(); segB.move<true>(); segC.move<true>(); l++;
					}
				}
				else
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<true>(); segB.move<true>(); segC.move<false>(); l++;
					}
				}
			}
			else
			{
				if (segC.x_major())
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<true>(); segB.move<false>(); segC.move<true>(); l++;
					}
				}
				else
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<true>(); segB.move<false>(); segC.move<false>(); l++;
					}
				}
			}
		}
		else
		{
			const bool X_MAJOR = false;
			if (segB.x_major())
			{
				if (segC.x_major())
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<false>(); segB.move<true>(); segC.move<true>(); l++;
					}
				}
				else
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<false>(); segB.move<true>(); segC.move<false>(); l++;
					}
				}
			}
			else
			{
				if (segC.x_major())
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<false>(); segB.move<false>(); segC.move<true>(); l++;
					}
				}
				else
				{
					while (l <= lena)
					{
						if (((l > lenb) || (segA != segB)) && ((l > lenc) || (segA != segC))) _bseg_update_pixel<X_MAJOR, BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
						segA.move<false>(); segB.move<false>(); segC.move<false>(); l++;
					}
				}
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






	/**
	* Draw the bresenham segment segA while avoiding segB and segC (at opposite ends)
	*
	*            /        \
	*          B/          \C
	*          /            \
	*         +------A-------+
	* 
	* @param	segA	  	segment to draw.
	* @param	lastA	  	true to consider the closed segment.
	* @param	segB	  	first segment to avoid : must share the same start pixel as segA.
	* @param	lastB	  	true to consider the closed segment.
	* @param	segC	  	second segment to avoid : its start pixel must be the end pixel of segA.
	* @param	lastC	  	true to consider the closed segment.
	* @param	color	  	color to use.
	* @param	blend	  	(Optional) true to use blending.
	* @param	side	  	(Optional) 0 for no side AA and +/-1 for side AA.
	* @param	op		  	(Optional) opacity to apply if 0 <= op <= 256.
	* @param	checkrange	(Optional) True to check the range (default). Set it to false only if it
	* 						is sure that the segment does not exit the image.
	**/
	void _bseg_avoid11(const internals_bseg::BSeg & segA, bool lastA, const internals_bseg::BSeg & segB, bool lastB, const internals_bseg::BSeg & segC, bool lastC, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true);



	/**
	* Draw the bresenham segment segA while avoiding segB, segC and segD
	*     
	*      \     /              /
	*      C\   /B             /D
	*        \ /              /
	*         +------A-------+
	*
	* @param	segA	  	segment to draw.
	* @param	lastA	  	true to consider the closed segment.
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
	void _bseg_avoid21(const internals_bseg::BSeg & segA, bool lastA, const internals_bseg::BSeg & segB, bool lastB, const internals_bseg::BSeg & segC, bool lastC, const internals_bseg::BSeg & segD, bool lastD, 
		               RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true);



	/**
	* Draw the bresenham segment segA while avoiding segB, segC and segD
	*
	*      \     /        \     /
	*      C\   /B        E\   /D
	*        \ /            \ /
	*         +------A-------+
	*
	* @param	segA	  	segment to draw.
	* @param	lastA	  	true to consider the closed segment.
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
		               RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true);



};



int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows

		return 0;
	}

