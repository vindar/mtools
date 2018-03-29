			/****************************************************************************************
			 * BRESENHAM NEW METHODS 
			 ***************************************************************************************/


			/**
			* Draw a bresenham segment (template version)
			**/
			template<bool BLEND, bool USEOP, bool USEPEN, int SIDE> void _draw_bseg(const internals_bseg::BSeg seg, bool draw_last, RGBc color, int32 penwidth, int32 op, bool checkrange = true)
				{
				if (checkrange)
					{
					const int64 of = ((USEPEN) && (penwidth > 0)) ? (penwidth + 2) : 0;
					iBox2 B(-of, _lx - 1 + of, -of, _ly - 1 + of);
					seg.move_inside_box(B);												// move inside the box
					seg.len() = std::min<int64>(seg.lenght_inside_box(B), seg.len());	// truncate to stay inside the box
					}					
				if (draw_last) (seg.len())++;
				if (seg.x_major())
					{
					while (seg.len() > 0) { _update_pixel_bresenham<BLEND, USEPEN, USEOP, USEPEN, SIDE>(seg, color, op, penwidth); seg.move<true>(); }
					}
				else
					{
					while (seg.len() > 0) { _update_pixel_bresenham<BLEND, USEPEN, USEOP, USEPEN, SIDE>(seg, color, op, penwidth); seg.move<false>(); }
					}
				}


			/**
			 * Draw a Bresenham segment.
			 */
			void _draw_bseg(const internals_bseg::BSeg & seg, bool draw_last, RGBc color, int32 penwidth = 0, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
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
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						else
							{
							const bool USEOP = false;
							if (blend)
								{
								const bool BLEND = true;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
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
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						else
							{
							const bool USEOP = false;
							if (blend)
								{
								const bool BLEND = true;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
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
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						else
							{
							const bool USEOP = false;
							if (blend)
								{
								const bool BLEND = true;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
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
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						else
							{
							const bool USEOP = false;
							if (blend)
								{
								const bool BLEND = true;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
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
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						else
							{
							const bool USEOP = false;
							if (blend)
								{
								const bool BLEND = true;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
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
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						else
							{
							const bool USEOP = false;
							if (blend)
								{
								const bool BLEND = true;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							else
								{
								const bool BLEND = false;
								_draw_bseg<BLEND, USEOP, USEPEN, SIDE>(seg, draw_last, color, penwidth, op, checkrange);
								}
							}
						}
					}
				}


			/**
			 * Draw a Bresenham segment between two integer-valued points
			 */
			MTOOLS_FORCEINLINE void _draw_bseg(const iVec2 & P1, const iVec2 & P2, bool draw_last, RGBc color, int32 penwidth = 0, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_draw_bseg(internals_bseg::BSeg(P1, P2), draw_last, color, penwidth, blend, side, op, checkrange);
				}


			/**
			 * Draw a Bresenham segment between two integer-valued points
			 */
			MTOOLS_FORCEINLINE void _draw_bseg(const fVec2 & P1, const fVec2 & P2, bool draw_last, RGBc color, int32 penwidth = 0, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_draw_bseg(internals_bseg::BSeg(P1, P2), draw_last, color, penwidth, blend, side, op, checkrange);
				}


			/**
			 * Find the maximum distance where two segments intersect. 
			 * checkrange = true to move the segment inside the 
			 */
			int64 _bseg_find_max_intersection(internals_bseg::BSeg segA, internals_bseg::BSeg segB, bool checkrange = true)
				{
				MTOOLS_ASSERT(segA.pos() == segB.pos());
				int64 r = 0;
				if (checkrange)
					{
					iBox2 B(0, _lx - 1, 0, _ly - 1);
					int64 r = segA.move_inside_box(B);
					if (segA.len() < 0) return 1;
					segB.move(r); // move the second line by the same amount.
					segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());	// truncate to stay inside the box
					}
				int64 lena = segA.len() - 1;
				int64 lenb = segB.len() - 1;
				int64 l = 0;
				int64 maxp = 0;
				if (segA.x_major())
					{
					if (segB.x_major())
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(segA.X() - segB.X()) + abs(segA.Y() - segB.Y()); if (o == 0) maxp = l;
							segA.move<true>(); segB.move<true>();
							l++;
							}
						}
					else
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(segA.X() - segB.X()) + abs(segA.Y() - segB.Y()); if (o == 0) maxp = l;
							segA.move<true>(); segB.move<false>();
							l++;
							}
						}
					}
				else
					{
					if (segB.x_major())
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(segA.X() - segB.X()) + abs(segA.Y() - segB.Y()); if (o == 0) maxp = l;
							segA.move<false>(); segB.move<true>();
							l++;
							}
						}
					else
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(segA.X() - segB.X()) + abs(segA.Y() - segB.Y()); if (o == 0) maxp = l;
							segA.move<false>(); segB.move<false>();
							l++;
							}
						}
					}
				return ((maxp == 0) ? 1 : (r + maxp));
				}


			/**
			* Draw the segment A while avoiding segment A. Must share the same start point (template)
			**/
			template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid(internals_bseg::BSeg segA, internals_bseg::BSeg segB, bool draw_last, RGBc color, int32 op, bool checkrange = true)
				{
				MTOOLS_ASSERT(segA.pos() == segB.pos());
				if (draw_last) segA.len()++;
				if (checkrange)
					{
					iBox2 B(0, _lx - 1, 0, _ly - 1);
					int64 r = segA.move_inside_box(B);
					if (segA.len() < 0) return;
					segB.move(r);																// move the second line by the same amount.
					segA.len() = std::min<int64>(segA.lenght_inside_box(B), segA.len());		// truncate to stay inside the box
					}	
				int64 lena = segA.len() - 1;
				int64 lenb = segB.len() - 1;
				int64 l = 0;
				if (segA.x_major())
					{
					if (segB.x_major())
						{
						while (l <= lena)
							{
							if ((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
							segA.move<true>(); segB.move<true>();
							l++; 
							}
						}
					else
						{
						while (l <= lena)
							{
							if ((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
							segA.move<true>(); segB.move<false>();
							l++;
							}
						}
					}
				else
					{
					if (segB.x_major())
						{
						while (l <= lena)
							{
							if ((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
							segA.move<false>(); segB.move<true>();
							l++;
							}
						}
					else
						{
						while (l <= lena)
							{
							if ((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
							segA.move<false>(); segB.move<false>();
							l++;
							}
						}
					}
				}


			/**
			* Draw the segment A while avoiding segment A. Must share the same start point
			**/
			void _bseg_avoid(const internals_bseg::BSeg & segA, const internals_bseg::BSeg & segB, bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
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
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						else
							{
							const bool BLEND = false;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						}
					else
						{
						const bool USEOP = false;
						if (blend)
							{
							const bool BLEND = true;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						else
							{
							const bool BLEND = false;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
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
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						else
							{
							const bool BLEND = false;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						}
					else
						{
						const bool USEOP = false;
						if (blend)
							{
							const bool BLEND = true;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						else
							{
							const bool BLEND = false;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
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
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						else
							{
							const bool BLEND = false;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						}
						else
						{
						const bool USEOP = false;
						if (blend)
							{
							const bool BLEND = true;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						else
							{
							const bool BLEND = false;
							_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, draw_last, color, op, checkrange);
							}
						}
					}
				}


			/**
			* Draw a [P,Q] while avoiding [P,R]
			*/
			MTOOLS_FORCEINLINE void _bseg_avoid(const fVec2 & P, const fVec2 & Q, const fVec2 & R, const bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_bseg_avoid(internals_bseg::BSeg(P, Q), internals_bseg::BSeg(P, R), draw_last, color, blend, side, op, checkrange);
				}


			/**
			* Draw a [P,Q] while avoiding [P,R]
			*/
			MTOOLS_FORCEINLINE void _bseg_avoid(const iVec2 & P, const iVec2 & Q, const iVec2 & R, const bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_bseg_avoid(internals_bseg::BSeg(P, Q), internals_bseg::BSeg(P, R), draw_last, color, blend, side, op, checkrange);
				}



			/**
			* draw segA while avoiding segB and segC. All 3 must have the same start point. (template)
			**/
			template<bool BLEND, bool USEOP, int SIDE> void _bseg_avoid(internals_bseg::BSeg segA, internals_bseg::BSeg segB, internals_bseg::BSeg segC, bool draw_last, RGBc color, int32 op, bool checkrange)
				{
				MTOOLS_ASSERT(segA.pos() == segB.pos());
				MTOOLS_ASSERT(segA.pos() == segC.pos());
				if (draw_last) segA.len()++;
				if (checkrange)
					{
					iBox2 B(0, _lx - 1, 0, _ly - 1);
					int64 r = segA.move_inside_box(B);
					if (segA.len() < 0) return;
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
					if (segB.x_major())
						{
						if (segC.x_major())
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND,false,USEOP,false,SIDE>(segA, color, op, 0);
								segA.move<true>(); segB.move<true>(); segC.move<true>();
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<true>(); segB.move<true>(); segC.move<false>();
								l++;
								}
							}
						}
					else
						{
						if (segC.x_major())
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<true>(); segB.move<false>(); segC.move<true>();
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<true>(); segB.move<false>(); segC.move<false>();
								l++;
								}
							}
						}
					}
				else
					{
					if (segB.x_major())
						{
						if (segC.x_major())
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<false>(); segB.move<true>(); segC.move<true>();
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<false>(); segB.move<true>(); segC.move<false>();
								l++;
								}
							}
						}
					else
						{
						if (segC.x_major())
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<false>(); segB.move<false>(); segC.move<true>();
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (segA.X() != segB.X()) || (segA.Y() != segB.Y())) && ((l > lenc) || (segA.X() != segC.X()) || (segA.Y() != segC.Y()))) _update_pixel_bresenham<BLEND, false, USEOP, false, SIDE>(segA, color, op, 0);
								segA.move<false>(); segB.move<false>(); segC.move<false>();
								l++;
								}
							}
						}
					}
				}


			/**
			* draw segA while avoiding segB and segC. All 3 must have the same start point.
			**/
			void _bseg_avoid(const internals_bseg::BSeg & segA, const internals_bseg::BSeg & segB, const internals_bseg::BSeg & segC, bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
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
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
							else
							{
								const bool BLEND = false;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
						}
						else
						{
							const bool USEOP = false;
							if (blend)
							{
								const bool BLEND = true;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
							else
							{
								const bool BLEND = false;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
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
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
							else
							{
								const bool BLEND = false;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
						}
						else
						{
							const bool USEOP = false;
							if (blend)
							{
								const bool BLEND = true;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
							else
							{
								const bool BLEND = false;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
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
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
							else
							{
								const bool BLEND = false;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
						}
						else
						{
							const bool USEOP = false;
							if (blend)
							{
								const bool BLEND = true;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
							else
							{
								const bool BLEND = false;
								_bseg_avoid<BLEND, USEOP, SIDE>(segA, segB, segC, draw_last, color, op, checkrange);
							}
						}
					}
				}


			/**
			* Draw a [P,Q] while avoiding [P,R1] and [P,R2]
			*/
			MTOOLS_FORCEINLINE void _bseg_avoid(const fVec2 & P, const fVec2 & Q, const fVec2 & R1, const fVec2 & R2, const bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_bseg_avoid(internals_bseg::BSeg(P, Q), internals_bseg::BSeg(P, R1), internals_bseg::BSeg(P, R2), draw_last, color, blend, side, op, checkrange);
				}


			/**
			* Draw a [P,Q] while avoiding [P,R1] and [P,R2]
			*/
			MTOOLS_FORCEINLINE void _bseg_avoid(const iVec2 & P, const iVec2 & Q, const iVec2 & R1, const iVec2 & R2, const bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_bseg_avoid(internals_bseg::BSeg(P, Q), internals_bseg::BSeg(P, R1), internals_bseg::BSeg(P, R2), draw_last, color, blend, side, op, checkrange);
				}




			/**
			* draw segA while avoiding segB and segC where the 3 segment form a triangle 
			* such that A and B have the same start pixel and the start pixel of C is the end pixel of A
			**/
			MTOOLS_FORCEINLINE void _bseg_avoid_triangle(const internals_bseg::BSeg & segA, const internals_bseg::BSeg & segB, const internals_bseg::BSeg & segC, 
				                                         bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				MTOOLS_ASSERT(segA.pos() == segB.pos());
				int64 lena = segA.len();
				int64 lenb = segB.len();
				int64 lenc = segC.len();
				int64 G = lenb - lenc;
				if (G > lena) { G = lena; } else { if (G < -lena) { G = -lena; } }
				int64 lP = 1 + ((lena + G) >> 1);
				int64 lQ = 1 + ((lena - G) >> 1);
				if (lP + lQ > lena + 1)
					{
					if (lP > lQ) { lP--; } else { lQ--; }
					}
				MTOOLS_ASSERT(lP + lQ == lena + 1);
				internals_bseg::BSeg segA1 = segA;
				internals_bseg::BSeg segA2 = segA.get_reverse();
				MTOOLS_ASSERT(segA2.pos() == segC.pos());
				segA1.len() = lena + 1 - lQ;
				segA2.len() = lena + 1 - lP;
				_bseg_avoid(segA1, segB, false, color, blend, side, op, checkrange);
				_bseg_avoid(segA2, segC, false, color, blend, side, op, checkrange);
				}


			/**
			* draw [P,Q] while avoiding [P,R] and [Q,R]
			**/
			MTOOLS_FORCEINLINE void _bseg_avoid_triangle(const iVec2 & P, const iVec2 & Q, const iVec2 & R, bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_bseg_avoid_triangle(internals_bseg::BSeg(P,Q), internals_bseg::BSeg(P,R), internals_bseg::BSeg(Q,R), draw_last, color, blend, side, op, checkrange);
				}


			/**
			* draw [P,Q] while avoiding [P,R] and [Q,R]
			**/
			MTOOLS_FORCEINLINE void _bseg_avoid_triangle(const fVec2 & P, const fVec2 & Q, const fVec2 & R, bool draw_last, RGBc color, bool blend = true, int side = 0, int32 op = -1, bool checkrange = true)
				{
				_bseg_avoid_triangle(internals_bseg::BSeg(P, Q), internals_bseg::BSeg(P, R), internals_bseg::BSeg(Q, R), draw_last, color, blend, side, op, checkrange);
				}
