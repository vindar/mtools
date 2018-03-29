		


        template<bool blend, bool checkrange, bool useop, bool usepen, bool useaa, bool side>  MTOOLS_FORCEINLINE void _lineBresenham(_bdir line, _bpos pos, int64 len, RGBc color, int32 penwidth, int32 op)
			{
			}

		template<bool blend, bool checkrange, bool useop, bool usepen, bool useaa, bool side>  MTOOLS_FORCEINLINE void _lineBresenham(const iVec2 P1, const iVec2 P2, RGBc color, bool draw_last, int32 penwidth, int32 op)
			{
			}

			/**
			* Return the max distance where the two line intersect. linea and lineb must share the same start pixel. 
			* segment are considered open ended : [a, a + lena[
			**/
		template<bool checkrange> int64 _lineBresenham_find_max_intersection(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb)
			{
			}
            
            
		template<bool blend, bool checkrange, bool useop, bool useaa, bool side> void _lineBresenham_avoid(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb, RGBc color, int32 op)

        
        		/**
			 * Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels
			 * which also belong to the segment [P,P2]. (Always perfect.)
			 * 
			 * stop_before represented the number of pixel at the end of the line which are not drawn
			 * i.e 0 = draw [P,Q], 
			 *     1 = draw [P,Q[   
			 *    >1 = remove more pixels  
			 *    <0 = extend the line adding stop_before additional pixels.
			 **/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid(iVec2 P, iVec2 Q, iVec2 P2, RGBc color, int64 stop_before, int32 op)
				{
				if (P == Q) return;
				_bdir linea;
				_bpos posa;
				int64 lena = _init_line(P, Q, linea, posa) + 1 - stop_before;
				_bdir lineb;
				_bpos posb;
				int64 lenb = _init_line(P, P2, lineb, posb) + 1;
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(linea, posa, lena, lineb, posb, lenb, color, op);
				}
                
                
                
             			/**
			* Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels
			* which also belong to the segments [P,P2] and [P,P3].  (always perfect.)
			*
			* stop_before represented the number of pixel at the end of the line which are not drawn
			* i.e 0 = draw [P,Q],
			*     1 = draw [P,Q[
			*    >1 = remove som more pixels
			*    <0 = extend the line adding stop_before additional pixels.
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid(iVec2 P, iVec2 Q, iVec2 P2, iVec2 P3, RGBc color, int64 stop_before, int32 op)
				{
				if ((P2 == P3)||(P3 == P)) { _lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, P2, color, stop_before, op); return; }
				if (P2 == P) { _lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, P3, color, stop_before, op); return; }
				if (P == Q) return;
				_bdir linea;
				_bpos posa;
				int64 lena = _init_line(P, Q, linea, posa) + 1 - stop_before;
				_bdir lineb;
				_bpos posb;
				int64 lenb = _init_line(P, P2, lineb, posb) + 1;
				_bdir linec;
				_bpos posc;
				int64 lenc = _init_line(P, P3, linec, posc) + 1;
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(linea, posa, lena, lineb, posb, lenb, linec, posc, lenc, color, op);
				}