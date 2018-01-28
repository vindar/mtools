
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:



	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}
	

			/**
			* Draw a circle/ellipse. Alternative method. About for times slower than the regular method but :
			* 
			*  - can draw with non-integer center and radii !  
			*  - can restrict drawing to a subbox B (useful when the ellipse is much larger than the image). 
			**/
			template<bool blend, bool outline, bool fill, bool usepen>  inline  void _draw_ellipse2(iBox2 B, fVec2 P, double rx, double ry, RGBc color, RGBc fillcolor, int32 penwidth)
				{
				B = intersectionRect(B, iBox2((int64)floor(P.X() - rx - 1),
					(int64)ceil(P.X() + rx + 1),
					(int64)floor(P.Y() - ry - 1),
					(int64)ceil(P.Y() + ry + 1)));

				const double rx2 = rx*rx;
				const double ry2 = ry*ry;
				const double Rx2 = (rx + 0.5)*(rx + 0.5);
				const double Ry2 = (ry + 0.5)*(ry + 0.5);
				const double Rxy2 =  Rx2*Ry2;

				int64 xmin = B.max[0];
				int64 xmax = B.min[0];

				for (int64 y = B.min[1]; y <= B.max[1]; y++)
					{	
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);

					if (xmin > xmax)
						{ 
						if (dy2 > Ry2) continue;  // line is empty. 
						if (P.X() <= (double)B.min[0])
							{ 
							const double dx = (double)B.min[0] - P.X();
							if ( (dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
							}
						else if (P.X() >= (double)B.max[0])
							{
							const double dx = P.X() - (double)B.max[0];
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
							}							
						xmin = B.min[0]; xmax = B.max[0]; 
						}
					const double ly = dy2 - absdy + 0.25;
					const double Ly = dy2 + absdy + 0.25;
					const double g1 = rx2 - ly * rx2/ry2 - 0.25;
					const double g2 = rx2 - Ly * rx2/ry2 - 0.25;
					double dx = (double)(xmin - P.X());

					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx;
						if ((xmin == B.min[0]) || (lx  > g1)) break;
						xmin--;
						dx--;
						}
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx  <= g2) || (xmax < xmin)) break;
						if (outline) { if ((lx  < g2) || (Lx  <  g1)) { _updatePixel<blend, usepen, false, usepen>(xmin, y, color, 255, penwidth); } }
						xmin++;
						dx++;
						}
					dx = (double)(xmax - P.X());
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx;
						if ((xmax == B.max[0]) || (lx  > g1)) break;
						xmax++;
						dx++;
						}
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx*dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx <= g2) || (xmax < xmin)) break;
						if (outline) { if ((lx  < g2) || (Lx  <  g1)) { _updatePixel<blend, usepen, false, usepen>(xmax, y, color, 255, penwidth); } }
						xmax--;
						dx--;
						}
					if (fill) { if (xmin < xmax) { _hline<blend, false>(xmin, xmax, y, fillcolor); } }
					}
				}





			/**
			* Draw an antialised circle/ellipse. Alternative method.About for times slower than the regular method but :
			* 
			*  -can draw with non - integer center and radii !
			*  -can restrict drawing to a subbox B(useful when the ellipse is much larger than the image).
			**/
			template<bool blend, bool fill, bool usepen> void _draw_ellipse2_AA(iBox2 B, fVec2 P, double rx, double ry, RGBc color, RGBc fillcolor, int32 penwidth)
				{
				B = intersectionRect(B, iBox2((int64)floor(P.X() - rx - 1),
					(int64)ceil(P.X() + rx + 1),
					(int64)floor(P.Y() - ry - 1),
					(int64)ceil(P.Y() + ry + 1)));

				const double ex2 = rx * rx;
				const double ey2 = ry * ry;
				const double exy2 = ex2 * ey2;
				const double Rx2 = (rx + 0.5)*(rx + 0.5);
				const double rx2 = (rx - 0.5)*(rx - 0.5);
				const double Ry2 = (ry + 0.5)*(ry + 0.5);
				const double ry2 = (ry - 0.5)*(ry - 0.5);
				const double rxy2 = rx2*ry2;
				const double Rxy2 = Rx2*Ry2;
				const double Rx2minus025 = Rx2 - 0.25;
				const double Rx2overRy2 = Rx2 / Ry2;
				const double rx2minus025 = rx2 - 0.25;
				const double rx2overry2 = rx2 / ry2;

				int64 xmin = B.max[0];
				int64 xmax = B.min[0];

				for (int64 y = B.min[1]; y <= B.max[1]; y++)
					{
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);

					if (xmin > xmax)
						{
						if (dy2 > Ry2) continue;  // line is empty. 
						if (P.X() <= (double)B.min[0])
							{
							const double dx = (double)B.min[0] - P.X();
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
							}
						else if (P.X() >= (double)B.max[0])
							{
							const double dx = P.X() - (double)B.max[0];
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
							}
						xmin = B.min[0]; xmax = B.max[0];
						}


					const double v = ex2 * dy2;
					const double vv = ex2 * v;
					const double vminusexy2 = v - exy2;
					const double ly = dy2 - absdy + 0.25;
					const double Ly = dy2 + absdy + 0.25;				
					const double g1 = Rx2minus025 - Rx2overRy2 * ly;
					const double g2 = rx2minus025 - rx2overry2 * Ly;
					double dx = (double)(xmin - P.X());
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						if ((xmin == B.min[0]) || (lx > g1)) break;
						xmin--;
						dx--;
						}
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx < g2) || (xmax < xmin)) break;
						if (lx < g1)
							{
							const double u = ey2 * dx2;
							const double uu = ey2 * u;
							double d = (u + vminusexy2) * fast_invsqrt((float)(uu + vv)); // d = twice the distance of the point to the ideal ellipse
							if (d < 0) d = (fill ? 0 : -d);
							if (d < 2) { _updatePixel<blend, usepen, true, usepen>(xmin, y, color, 256 - (int32)(128 * d), penwidth); }
							}
						xmin++;
						dx++;
						}
					dx = (double)(xmax - P.X());
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						if ((xmax == B.max[0]) || (lx > g1)) break;
						xmax++;
						dx++;
						}
					while (1)
						{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx < g2) || (xmax < xmin)) break;
						if (lx < g1)
							{
							const double u = ey2 * dx2;
							const double uu = ey2 * u;
							double d = (u + vminusexy2) * fast_invsqrt((float)(uu + vv)); // d = twice the distance of the point to the ideal ellipse
							if (d < 0) d = (fill ? 0 : -d);
							if (d < 2) { _updatePixel<blend, usepen, true, usepen>(xmax, y, color, 256 - (int32)(128 * d), penwidth); }
							}
						xmax--;
						dx--;
						}
					if (fill) { if (xmin < xmax) { _hline<blend, false>(xmin, xmax, y, fillcolor); } }
					}
				}





			/**
			* Draw a circle.
			*
			* @param	P				position of the center.
			* @param	r				radius.
			* @param	color			color to use.
			* @param	blend			true to use blending.
			* @param	antialiasing	true to use antialiasing.
			* @param	penwidth		(Optional) The pen width (0 = unit width)
			**/
			inline void good_draw_circle(iVec2 P, int64 r, RGBc color, bool blend, bool antialiasing, int32 penwidth = 0)
				{
				if (isEmpty() || (r < 1)) return;
				iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
				iBox2 imBox = imageBox();
				if (penwidth > 0)
					{ // large pen
					_correctPenOpacity(color, penwidth);
					circleBox.enlarge(penwidth);
					iBox2 B = intersectionRect(circleBox, imBox);
					if (B.isEmpty()) return; // nothing to draw.
					if (circleBox.isIncludedIn(imBox))
						{ // included
						if (antialiasing)
							{
							if (blend) _draw_circle_AA<true, false, true>(P.X(), P.Y(), r, color, penwidth); else _draw_circle_AA<false, false, true>(P.X(), P.Y(), r, color, penwidth);
							}
						else
							{
							if (blend) _draw_circle<true, false, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth); else _draw_circle<false, false, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth);
							}
						return;
						}
					// not included
					if (B.area() * 8 > circleBox.area())
					{ // still faster to use draw everything using the first method and checking the range
						if (antialiasing)
						{
							if (blend) _draw_circle_AA<true, true, true>(P.X(), P.Y(), r, color, penwidth); else _draw_circle_AA<false, true, true>(P.X(), P.Y(), r, color, penwidth);
						}
						else
						{
							if (blend) _draw_circle<true, true, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth); else _draw_circle<false, true, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth);
						}
						return;
					}
					// use alternate method
					B.enlarge(penwidth);
					double rr = (double)r;
					if (antialiasing)
						{
						if (blend) _draw_ellipse2_AA<true, false, true>(B, P, rr, rr, color, color, penwidth); else _draw_ellipse2_AA<false, false, true>(B, P, rr, rr, color, color, penwidth);
						}
					else
						{
						if (blend) _draw_ellipse2<true, true, false, true>(B, P, rr, rr, color, color, penwidth); else _draw_ellipse2<false, true, false, true>(B, P, rr, rr, color, color, penwidth);
						}
					return;
					}
				iBox2 B = intersectionRect(circleBox, imBox);
				if (B.isEmpty()) return; // nothing to draw.
				if (circleBox.isIncludedIn(imBox))
					{ // included
					if (antialiasing)
						{
						if (blend) _draw_circle_AA<true, false, false>(P.X(), P.Y(), r, color, 0); else _draw_circle_AA<false, false, false>(P.X(), P.Y(), r, color, 0);
						}
					else
						{
						if (blend) _draw_circle<true, false, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0); else _draw_circle<false, false, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0);
						}
					return;
					}
				// not included
				if (B.area() * 8 > circleBox.area())
					{ // still faster to use draw everything using the first method and checking the range
					if (antialiasing)
						{
						if (blend) _draw_circle_AA<true, true, false>(P.X(), P.Y(), r, color, 0); else _draw_circle_AA<false, true, false>(P.X(), P.Y(), r, color, 0);
						}
					else
						{
						if (blend) _draw_circle<true, true, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0); else _draw_circle<false, true, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0);
						}
					return;
					}
				// use alternate method
				double rr = (double)r;
				if (antialiasing)
					{
					if (blend) _draw_ellipse2_AA<true, false,false>(B, P, rr, rr, color, color, 0); else _draw_ellipse2_AA<false, false,false>(B, P, rr, rr, color, color, 0);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, false, false>(B, P, rr, rr, color, color, 0); else _draw_ellipse2<false, true, false, false>(B, P, rr, rr, color, color, 0);
					}
				return;
				}


			/**
			 * Draw an ellipse.
			 *
			 * @param	P				position of the center.
			 * @param	rx				x radius
			 * @param	ry				y radius
			 * @param	color			color to use.
			 * @param	blend			true to use blending.
			 * @param	antialiasing	true to use antialiasing.
			 * @param	penwidth		(Optional) The pen width (0 = unit width)
			 **/
			inline void good_draw_ellipse(iVec2 P, int64 rx, int64 ry, RGBc color, bool blend, bool antialiasing, int32 penwidth = 0)
				{
				if (isEmpty() || (rx < 1) || (ry < 1)) return;
				iBox2 circleBox(P.X() - rx, P.X() + rx, P.Y() - ry, P.Y() + ry);
				iBox2 imBox = imageBox();
				if (penwidth > 0)
					{ // large pen
					_correctPenOpacity(color, penwidth);
					circleBox.enlarge(penwidth);
					iBox2 B = intersectionRect(circleBox, imBox);
					if (B.isEmpty()) return; // nothing to draw.
					if (circleBox.isIncludedIn(imBox))
						{ // included
						if (antialiasing)
							{
							if (blend) _draw_ellipse_in_rect_AA<true, false, true>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, penwidth); else _draw_ellipse_in_rect_AA<false, false, true>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, penwidth);
							}
						else
							{
							if (blend) _draw_ellipse<true, false, true, false, false, false, true>(P.X(), P.Y(), rx, ry, color, color, penwidth); else _draw_ellipse<false, false, true, false, false, false, true>(P.X(), P.Y(), rx, ry, color, color, penwidth);
							}
						return;
						}
					// not included
					if (B.area() * 8 > circleBox.area())
						{ // still faster to use draw everything using the first method and checking the range
						if (antialiasing)
							{
							if (blend) _draw_ellipse_in_rect_AA<true, true, true>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, penwidth); else _draw_ellipse_in_rect_AA<false, true, true>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, penwidth);
							}
						else
							{
							if (blend) _draw_ellipse<true, true, true, false, false, false, true>(P.X(), P.Y(), rx, ry, color, color, penwidth); else _draw_ellipse<false, true, true, false, false, false, true>(P.X(), P.Y(), rx, ry, color, color, penwidth);
							}
						return;
						}
					// use alternate method
					B.enlarge(penwidth);
					double rrx = (double)rx;
					double rry = (double)ry;
					if (antialiasing)
						{
						if (blend) _draw_ellipse2_AA<true, false, true>(B, P, rrx, rry, color, color, penwidth); else _draw_ellipse2_AA<false, false, true>(B, P, rrx, rry, color, color, penwidth);
						}
					else
						{
						if (blend) _draw_ellipse2<true, true, false, true>(B, P, rrx, rry, color, color, penwidth); else _draw_ellipse2<false, true, false, true>(B, P, rrx, rry, color, color, penwidth);
						}
					return;
					}
				iBox2 B = intersectionRect(circleBox, imBox);
				if (B.isEmpty()) return; // nothing to draw.
				if (circleBox.isIncludedIn(imBox))
					{ // included
					if (antialiasing)
						{
						if (blend) _draw_ellipse_in_rect_AA<true, false, false>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, 0); else _draw_ellipse_in_rect_AA<false, false, false>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, 0);
						}
					else
						{
						if (blend) _draw_ellipse<true, false, true, false, false, false, false>(P.X(), P.Y(), rx, ry, color, color, 0); else _draw_ellipse<false, false, true, false, false, false, false>(P.X(), P.Y(), rx, ry, color, color, 0);
						}
					return;
					}
				// not included
				if (B.area() * 8 > circleBox.area())
					{ // still faster to use draw everything using the first method and checking the range
					if (antialiasing)
						{
						if (blend) _draw_ellipse_in_rect_AA<true, true, false>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, 0); else _draw_ellipse_in_rect_AA<false, true, false>(P.X() - rx, P.Y() - ry, P.X() + rx, P.Y() + ry, color, 0);
						}
					else
						{
						if (blend) _draw_ellipse<true, true, true, false, false, false, false>(P.X(), P.Y(), rx, ry, color, color, 0); else _draw_ellipse<false, true, true, false, false, false, false>(P.X(), P.Y(), rx, ry, color, color, 0);
						}
					return;
					}
				// use alternate method
				double rrx = (double)rx;
				double rry = (double)ry;
				if (antialiasing)
					{
					if (blend) _draw_ellipse2_AA<true, false,false>(B, P, rrx, rry, color, color, 0); else _draw_ellipse2_AA<false, false,false>(B, P, rrx, rry, color, color, 0);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, false, false>(B, P, rrx, rry, color, color, 0); else _draw_ellipse2<false, true, false, false>(B, P, rrx, rry, color, color, 0);
					}
				return;
				}


			/**
			 * Draw a filled circle.
			 *
			 * @param	P		   	position of the center.
			 * @param	r		   	radius.
			 * @param	color	   	color of the border.
			 * @param	fillcolor  	color of the interior.
			 * @param	blend	   	true to use blending.
			 * @param	antialiased	true to use antialiasing.
			 **/
			void good_draw_filled_circle(iVec2 P, int64 r, RGBc color, RGBc fillcolor, bool blend, bool antialiased)
				{
				if (isEmpty() || (r < 1)) return;
				iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(circleBox, imBox);
				if (B.isEmpty()) return; // nothing to draw. 
				if (antialiased)
					{
					double rr = (double)r;
					if (blend) _draw_ellipse2_AA<true, true, false>(B, P, rr, rr, color, fillcolor, 0); else _draw_ellipse2_AA<false, true, false>(B, P, rr, rr, color, fillcolor, 0);
					return;
					}
				if (circleBox.isIncludedIn(imBox))
					{ // circle is completely inside the image
					if (blend) _draw_circle<true, false, true, true, false>(P.X(), P.Y(), r, color, fillcolor, 0); else _draw_circle<false, false, true, true, false>(P.X(), P.Y(), r, color, fillcolor, 0);
					return;
					}
				double rr = (double)r;
				if (blend) _draw_ellipse2<true, true, true, false>(B, P, rr, rr, color, fillcolor, 0); else _draw_ellipse2<false, true, true, false>(B, P, rr, rr, color, fillcolor, 0);
				return;
				}


			/**
			 * Draw a filled ellipse.
			 *
			 * @param	P		   	position of the center.
			 * @param	rx		   	x-radius.
			 * @param	ry		   	y-radius.
			 * @param	color	   	color of the border.
			 * @param	fillcolor  	color of the interior.
			 * @param	blend	   	true to use blending.
			 * @param	antialiased	true to use antialiasing.
			 **/
			void good_draw_filled_ellipse(iVec2 P, int64 rx, int64 ry, RGBc color, RGBc fillcolor, bool blend, bool antialiased)
				{
				if (isEmpty() || (rx < 1) || (ry < 1)) return;
				iBox2 circleBox(P.X() - rx, P.X() + rx, P.Y() - ry, P.Y() + ry);
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(circleBox, imBox);
				if (B.isEmpty()) return; // nothing to draw. 
				double rrx = (double)rx;
				double rry = (double)ry;
				if (antialiased)
					{
					if (blend) _draw_ellipse2_AA<true, true, false>(B, P, rrx, rry, color, fillcolor, 0); else _draw_ellipse2_AA<false, true, false>(B, P, rrx, rry, color, fillcolor, 0);
					return;
					}
				if (blend) _draw_ellipse2<true, true, true, false>(B, P, rrx, rry, color, fillcolor, 0); else _draw_ellipse2<false, true, true, false>(B, P, rrx, rry, color, fillcolor, 0);				
				return;
				}


			/**
			 * Draw ellipse fitting inside a rectangle.
			 * 
			 * Remark: the pen width does not count: if penwidth > 0, the ellipse will overflow its bounding
			 * box by exactly penwidth pixel on each side.
			 *
			 * @param	boundingBox	The enclosing rectangle.
			 * @param	color	   	color.
			 * @param	blend	   	true to use blending.
			 * @param	antialiased	true to use antialiasing.
			 * @param	penwidth   	(Optional) The pen width (0 = unit width)
			 **/
			void good_draw_ellipse_in_rect(iBox2 boundingBox, RGBc color, bool blend, bool antialiased, int penwidth = 0)
				{
				if (isEmpty() || (boundingBox.isEmpty())) return;
				iBox2 imBox = imageBox();
				fVec2 P{ 0.5*(boundingBox.min[0] + boundingBox.max[0]) , 0.5*(boundingBox.min[1] + boundingBox.max[1]) };
				double rx = 0.5*(boundingBox.max[0] - boundingBox.min[0]);
				double ry = 0.5*(boundingBox.max[1] - boundingBox.min[1]);
				if (penwidth > 0)
					{
					_correctPenOpacity(color, penwidth);
					imBox.enlarge(penwidth);
					iBox2 B = intersectionRect(boundingBox, imBox);
					if (B.isEmpty()) return; // nothing to draw. 
					if (antialiased)
						{
						if (blend) _draw_ellipse2_AA<true, false, true>(B, P, rx, ry, color, color, penwidth); else _draw_ellipse2_AA<false, false, true>(B, P, rx, ry, color, color, penwidth);
						return;
						}
					if (blend) _draw_ellipse2<true, true, false, true>(B, P, rx, ry, color, color, penwidth); else _draw_ellipse2<false, true, false, true>(B, P, rx, ry, color, color, penwidth);
					return;
					}
				iBox2 B = intersectionRect(boundingBox, imBox);
				if (B.isEmpty()) return; // nothing to draw. 
				if (antialiased)
					{
					if (blend) _draw_ellipse2_AA<true, false, false>(B, P, rx, ry, color, color, 0); else _draw_ellipse2_AA<false, false, false>(B, P, rx, ry, color, color, 0);
					return;
					}
				if (blend) _draw_ellipse2<true, true, false, false>(B, P, rx, ry, color, color, 0); else _draw_ellipse2<false, true, false, false>(B, P, rx, ry, color, color, 0);
				return;
				}


			/**
			 * Draw a filled ellipse fitting inside a rectangle.
			 *
			 * @param	boundingBox	The enclosing rectangle.
			 * @param	color	   	border color.
			 * @param	fillcolor  	interior color.
			 * @param	blend	   	true to use blending.
			 * @param	antialiased	true to use antialiasing.
			 **/
			void good_draw_filled_ellipse_in_rect(iBox2 boundingBox, RGBc color, RGBc fillcolor, bool blend, bool antialiased)
				{
				if (isEmpty() || (boundingBox.isEmpty())) return;
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(boundingBox, imBox);
				if (B.isEmpty()) return; // nothing to draw. 
				fVec2 P { 0.5*(boundingBox.min[0] + boundingBox.max[0]) , 0.5*(boundingBox.min[1] + boundingBox.max[1]) };
				double rx = 0.5*(boundingBox.max[0] - boundingBox.min[0]);
				double ry = 0.5*(boundingBox.max[1] - boundingBox.min[1]);
				if (antialiased)
					{
					if (blend) _draw_ellipse2_AA<true, true, false>(B, P, rx, ry, color, fillcolor, 0); else _draw_ellipse2_AA<false, true, false>(B, P, rx, ry, color, fillcolor, 0);
					return;
					}
				if (blend) _draw_ellipse2<true, true, true, false>(B, P, rx, ry, color, fillcolor, 0); else _draw_ellipse2<false, true, true, false>(B, P, rx, ry, color, fillcolor, 0);
				return;
				}






	};




	MT2004_64 gen;

#define NN 1



	/* fast inverse squere root */





void testCE()
	{
	TestImage imA(1000, 1000);
	TestImage imB(1000, 1000);
	imA.clear(RGBc::c_White);
	imB.clear(RGBc::c_White);
	MT2004_64 gen(0);

	size_t N = 50000;

	
	int64 mult_rx = 10000; 
	int64 mult_ry = 10000;
	int64 mult_pos = 10000; 
	

	std::vector<iVec2> center(N, iVec2());
	std::vector<int64> rx(N, 1);
	std::vector<int64> ry(N, 1);

	for (size_t i = 0; i < N; i++)
		{
		center[i] = { -mult_pos + (int64)(2 * Unif(gen)*mult_pos), -mult_pos + (int64)(2 * Unif(gen)*mult_pos) };
		rx[i] = 1 + (int64)(Unif(gen)*mult_rx);
		ry[i] = 1 + (int64)(Unif(gen)*mult_ry);

		}



	cout << "Simulating A... ";
	Chronometer(); 
	for (size_t i = 0; i < N; i++)
		{
		imA.good_draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true,true,3);
		}
	auto resA = Chronometer();
	cout << "done in " << durationToString(resA, true) << "\n";


	cout << "Simulating B... ";
	Chronometer();
	for (size_t i = 0; i < N; i++)
		{
		imB.draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true, true,3);
		}
	auto resB = Chronometer();
	cout << "done in " << durationToString(resB, true) << "\n";


	auto PA = makePlot2DImage(imA, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
	auto PB = makePlot2DImage(imB, 1, "Image B");   // Encapsulate the image inside a 'plottable' object.	
	Plotter2D plotter;              // Create a plotter object
	plotter[PA][PB];                // Add the image to the list of objects to draw.  	
	plotter.autorangeXY();          // Set the plotter range to fit the image.
	plotter.plot();                 // start interactive display.

	}




















	int main(int argc, char *argv[])
	{

		MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
		


		testCE();
		cout.getKey(); 
		return 0;

		{
			TestImage im(1000, 600);
			im.clear(RGBc::c_White);

			iBox2 B(100, 800, 100, 500);
			fVec2 Pa(300, 350);
			double rx = 100;
			double ry = 200;


			int64 N = 10000; 

				{
				Chronometer();
				for (int64 i = 0; i < N; i++)
					{
					//im._draw_ellipse2_AA<true, false,false>(B, Pa, rx, ry, RGBc::c_Red, 0);
					im._draw_ellipse2<true, true, false, false>(B, Pa, rx, ry, RGBc::c_Red, RGBc::c_Red, 0);
					//im.fill_ellipse(Pa, (int64)rx, (int64)ry, RGBc::c_Red, true);
					}
				int64 res = Chronometer();
				cout << "done in = " << durationToString(res, true) << "\n";
				}


				auto P = makePlot2DImage(im, 6);   // Encapsulate the image inside a 'plottable' object.	
				Plotter2D plotter;              // Create a plotter object
				plotter.axesObject(false);      // Remove the axe system.
				plotter[P];                     // Add the image to the list of objects to draw.  	
				plotter.autorangeXY();          // Set the plotter range to fit the image.
				plotter.plot();                 // start interactive display.


		}


		{
			TestImage im(1000, 600);
			im.clear(RGBc::c_White);

			iBox2 B(100, 800, 100, 500);
			fVec2 Pa(300, 250);
			fVec2 Pb(400, 250);
			fVec2 Pc(500, 250);
			fVec2 Pd(600, 350);
			double rx = 50;
			double ry = 50;

			//im.draw_box(B, RGBc::c_Gray, true);
			//im._draw_ellipse2<true, true, false, false>(B, Pa, rx, ry, RGBc::c_Red, RGBc::c_Blue, 0);
			im._draw_ellipse2_AA<true, true,false>(B, Pa, rx, ry, RGBc::c_Red, RGBc::c_Red, 0);
			//im._draw_ellipse2<true, true, false, false>(B, Pb, rx, ry, RGBc::c_Red, RGBc::c_Blue, 0);

			//im.draw_ellipse(Pc, rx, ry, RGBc::c_Red, true, false);
			im.draw_ellipse(Pd, rx, ry, RGBc::c_Red, true, true);

			//im.draw_dot({ 304, 259 }, RGBc::c_Blue, true, 0);

			auto P = makePlot2DImage(im, 6);   // Encapsulate the image inside a 'plottable' object.	
			Plotter2D plotter;              // Create a plotter object
			plotter.axesObject(false);      // Remove the axe system.
			plotter[P];                     // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}


		return 0;



/*
		{
			Image im("hello.png");
			im.rescale(10, { im.lx() * 10,im.ly() * 10 });
			auto P = makePlot2DImage(im,6);   // Encapsulate the image inside a 'plottable' object.	
			Plotter2D plotter;              // Create a plotter object
			plotter.axesObject(false);      // Remove the axe system.
			plotter[P];                     // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}
		return 0;
		{
			Image im(800, 600, RGBc(150, 150, 150,150));  // image of size 800x600 with a light gray background

													  // draw on the image
			im.fill_ellipse_in_rect({ 100,400,50,550 }, RGBc::c_Cyan, false);
			im.draw_ellipse_in_rect({ 100,400,50,550 }, RGBc::c_Green, true, true, 4);
			im.draw_text({ 400, 300 }, "Hello\n  World!", MTOOLS_TEXT_CENTER, RGBc::c_Red.getOpacity(0.7f), 200);
			im.draw_cubic_spline({ { 10,10 },{ 100,100 },{ 200,30 },{ 300,100 },{ 600,10 } ,{ 700,300 },
				{ 720, 500 },{ 600, 480 },{ 400,500 } }, RGBc::c_Yellow.getOpacity(0.5f), true, true, true, 3);


			im.draw_line({ 0,0 }, { 800,337 }, RGBc(200, 100, 57, 200), true, true, true, 20);

			im.save("hello2.png");
			im.save("hello2.jpeg");
			im.save("hello2.bmp");

			// display the image
			auto P = makePlot2DImage(im);   // Encapsulate the image inside a 'plottable' object.	
			Plotter2D plotter;              // Create a plotter object
			plotter.axesObject(false);      // Remove the axe system.
			plotter[P];                     // Add the image to the list of objects to draw.  	
			plotter.autorangeXY();          // Set the plotter range to fit the image.
			plotter.plot();                 // start interactive display.
		}
		return 0;
		*/

		TreeFigure<int, NN> TF;

		int n = 1000;


		cout << "inserting...\n";
		mtools::Chronometer();
		/*
		{
			cout << "DEserializing...\n";
			IFileArchive ar("testTreeAR.txt");
			ar & TF;
			cout << "OK...\n";
		}
		*/
		
		for (int i = 0; i < n; i++)
		{
			double xc = Unif(gen) * (Unif(gen) - 0.5) * 20;
			double yc = Unif(gen) * (Unif(gen) - 0.5) * 12;
			double lx = Unif(gen); lx *= lx;
			double ly = Unif(gen); ly *= ly;
			lx = 0.1; ly = 0.1;
			TF.insert({ xc - lx, xc + lx, yc - ly, yc + ly }, 0);
		}


		for (int i = 0; i < n / 10; i++)
		{
			double yc = Unif(gen) * 5;
			double lx = 10 * Unif(gen)* Unif(gen);
			TF.insert({ 0, lx, yc, yc }, 0);
		}

		
		
		cout << TF << "\n";

		//	TF.insert({ 1, 2, 1, 1.6 }, nullptr);

		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";


		fBox2 R = TF.mainBoundingBox();
		R = mtools::zoomOut(R);
		Image im(10000, 10000);
		im.clear(RGBc::c_White);


		cout << "Drawing...\n";
		mtools::Chronometer();
		TF.drawTreeDebug(im, R, RGBc::c_Transparent);
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";


		cout << "Visiting...\n";
		mtools::Chronometer();
		cout << "visited = " << TF.iterate_intersect({ -5,5,0,5 }, [&](const TreeFigure<int, NN>::BoundedObject & bo) -> void { im.canvas_draw_box(R, bo.boundingbox, RGBc::c_Green.getOpacity(0.5f), true);  return; });
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";

		cout << "Visiting...\n";
		mtools::Chronometer();
		cout << "visited = " << TF.iterate_contained_in({ -5,5,0,5 }, [&](const TreeFigure<int, NN>::BoundedObject & bo) -> void { im.canvas_draw_box(R, bo.boundingbox, RGBc::c_Blue.getOpacity(0.5f), true);  return; });
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";

		cout << "Visiting...\n";
		mtools::Chronometer();
		cout << "visited = " << TF.iterate_contain({ 1,1.01,1.5,1.51 }, [&](const TreeFigure<int, NN>::BoundedObject & bo) -> void { im.canvas_draw_box(R, bo.boundingbox, RGBc::c_Yellow.getOpacity(0.2f), true);  return; });
		cout << "done in " << durationToString(mtools::Chronometer(), true) << "\n";

		/*
		{
		cout << "serializing...\n";
		OFileArchive ar("testTreeAR.txt");
		ar & TF;
		cout << "OK...\n";
		}
		*/
		auto P1 = makePlot2DImage(im);
		Plotter2D plotter;
		plotter[P1];
		plotter.autorangeXY();
		plotter.range().zoomOut();
		plotter.plot();

		mtools::cout << "Hello World\n";
		mtools::cout.getKey();
		return 0;
	}
