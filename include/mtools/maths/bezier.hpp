/** @file bezier.hpp */
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


#include "../misc/internal/mtools_export.hpp"
#include "../misc/error.hpp"
#include "../misc/misc.hpp"
#include "../misc/stringfct.hpp"
#include "vec.hpp"
#include "box.hpp"

#include <algorithm>


namespace mtools
    {

    

	/** for internal usage only. */
	namespace bezier_internals
	{

		/** solve when the derivative is zero. */
		inline void _quadratic_bezier_solve_deriv(double x0, double x1, double x2, double & r)
			{
			r = -1;
			double dem = x0 + x2 - 2 * x1;
			if (dem != 0)
				{
				double res = (x0 - x1) / dem;
				r = ((res > 1) ? -1 : res);
				}
			}


		/** Solve the equation for a given value */
		inline void _quadratic_bezier_solve(double x0, double x1, double x2, double z, double & r1, double & r2)
			{
			double a = x0 - 2 * x1 + x2;
			double b = 2 * (x1 - x0);
			double c = x0 - z;
			r1 = -1; r2 = -1;
			int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
			if (r1 >= 1) r1 = -1;
			if (r2 >= 1) r2 = -1;
			}


		/** solve when the derivative is zero. */
		inline void _rational_bezier_solve_deriv(double x0, double x1, double x2, double w0, double w1, double w2, double & r1, double & r2)
			{ 
			double a = -2 * w0*w1*x0 + 2 * w0*w1*x1 + 2 * w0*w2*x0 - 2 * w0*w2*x2 - 2 * w1*w2*x1 + 2 * w1*w2*x2;
			double b = 4 * w0*w1*x0 - 4 * w0*w1*x1 - 2 * w0*w2*x0 + 2 * w0*w2*x2;
			double c = -2 * w0*w1*x0 + 2 * w0*w1*x1;
			r1 = -1; r2 = -1;
			int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
			if (r1 >= 1) r1 = -1;
			if (r2 >= 1) r2 = -1;
			}


		inline void _rational_bezier_solve(double x0, double x1, double x2, double w0, double w1, double w2, double z, double & r1, double & r2)
			{
			double a = x0*w0 - 2*x1*w1 + x2*w2 - z*(w0 - 2*w1 + w2);
			double b = -2 * x0*w0 + 2 * x1*w1 - z * (-2 * w0 + 2 * w1);
			double c = x0 * w0 - z * w0;
			r1 = -1; r2 = -1;
			int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
			if (r1 >= 1) r1 = -1;
			if (r2 >= 1) r2 = -1;
			}


		/** solve when the derivative is zero. */
		void _cubic_bezier_solve_deriv(double x0, double x1, double x2, double x3, double & r1, double & r2)
			{
			double a = (3 * x1 - 3 * x2 + x3 - x0);
			double b = 2 * (x0 - 2 * x1 + x2);
			double c = (x1 - x0);
			r1 = -1; r2 = -1;
			int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
			if (r1 >= 1) r1 = -1;
			if (r2 >= 1) r2 = -1;
			}


		/** Solve the equation for a given value */
		void _cubic_bezier_solve(double x0, double x1, double x2, double x3, double z, double & r1, double & r2, double & r3)
			{
			double a = x3 + 3 * (x1 - x2) - x0;
			double b = 3 * (x0 - 2 * x1 + x2);
			double c = 3 * (x1 - x0);
			double d = x0 - z;
			r1 = -1; r2 = -1; r3 = -1;
			int nb = mtools::gsl_poly_solve_cubic(a, b, c, d, &r1, &r2, &r3);
			if (r1 >= 1) r1 = -1;
			if (r2 >= 1) r2 = -1;
			if (r3 >= 1) r3 = -1;
			}


	}



	/**********************************************************************************
	*  QUADRATIC BEZIER CURVE
	*
	* Defined by 3 points P0, P1, P2.
	*
	*  -> curve start at P0, end at P2 and with control point P1.
	*  -> parametrisation : f(t) = (1-t)^2 P0 + 2t(1-t)P1 + t^2 P2 for t in [0,1]
	*
	**********************************************************************************/


	/**
	* Compute the position of the point on the curve at time t.
	*
	* @param	P0	start point of the curve.
	* @param	P1	control point of the curve.
	* @param	P2	end point of the curve.
	* @param	t 	time.
	*
	* @return	The position on the curve at time t.
	**/
	inline fVec2 quadratic_bezier_eval(fVec2 P0, fVec2 P1, fVec2 P2, double t)
		{
		return  fVec2(((P0.X() - 2 * P1.X() + P2.X())*t + 2 * (P1.X() - P0.X()))*t + P0.X(),
			((P0.Y() - 2 * P1.Y() + P2.Y())*t + 2 * (P1.Y() - P0.Y()))*t + P0.Y());
		}


	/**
	* Compute the time the curve has an horizontal tangent.
	*
	* @param 		  	P0 	start point of the curve.
	* @param 		  	P1 	control point of the curve.
	* @param 		  	P2 	end point of the curve.
	* @param [out]	r  	time when the tangent is horizontal (<0 is there is none).
	**/
	inline void quadratic_bezier_horizontal_tangent(fVec2 P0, fVec2 P1, fVec2 P2, double & r)
		{
		bezier_internals::_quadratic_bezier_solve_deriv(P0.Y(), P1.Y(), P2.Y(), r);
		}


	/**
	* Compute the time the curve has a vertical tangent.
	*
	* @param 		  	P0 	start point of the curve.
	* @param 		  	P1 	control point of the curve.
	* @param 		  	P2 	end point of the curve.
	* @param [out]	r  	time when the tangent is vertical (<0 is there is none).
	**/
	inline void quadratic_bezier_vertical_tangent(fVec2 P0, fVec2 P1, fVec2 P2, double & r)
		{
		bezier_internals::_quadratic_bezier_solve_deriv(P0.X(), P1.X(), P2.X(), r);
		}


	/**
	* Find the time when the curve intersect the horizontal line Y = y0.
	*
	* @param 	   	P0	start point of the curve.
	* @param 	   	P1	control point of the curve.
	* @param 	   	P2	end point of the curve.
	* @param 	   	y0	line equation is Y = y0.
	* @param [out]	r1	store possible intersection time (<0 if not valid).
	* @param [out]	r2	store possible intersection time (<0 if not valid).
	**/
	inline void quadratic_bezier_intersect_hline(fVec2 P0, fVec2 P1, fVec2 P2, double y0, double & r1, double & r2)
		{
		bezier_internals::_quadratic_bezier_solve(P0.Y(), P1.Y(), P2.Y(), y0, r1, r2);
		}


	/**
	* Find the time when the curve intersect the vertical line X = x0.
	*
	* @param 	   	P0	start point of the curve.
	* @param 	   	P1	control point of the curve.
	* @param 	   	P2	end point of the curve.
	* @param 	   	y0	line equation is X = x0.
	* @param [out]	r1	store possible intersection time (<0 if not valid).
	* @param [out]	r2	store possible intersection time (<0 if not valid).
	**/
	inline void quadratic_bezier_intersect_vline(fVec2 P0, fVec2 P1, fVec2 P2, double x0, double & r1, double & r2)
		{
		bezier_internals::_quadratic_bezier_solve(P0.X(), P1.X(), P2.X(), x0, r1, r2);
		}


	/**
	* Compute the times when a curve intersect a rectangle.
	*
	* @param 	   	B			The rectangle to test.
	* @param 	   	P0			start point of the curve.
	* @param 	   	P1			control point of the curve.
	* @param 	   	P2			end point of the curve.
	* @param [out]	restimes	an array of size at least 8 to store the intersection times. There are
	* 							returned ordered increasingly.
	*
	* @return	The number of intersection times found.
	**/
	inline int quadratic_bezier_intersect_rect(fBox2 B, fVec2 P0, fVec2 P1, fVec2 P2, double(&restimes)[8])
		{
		int nb = 0;
		double r1, r2;

		quadratic_bezier_intersect_vline(P0, P1, P2, B.min[0], r1, r2);
		if (r1 > 0)
		{
			double y = quadratic_bezier_eval(P0, P1, P2, r1).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double y = quadratic_bezier_eval(P0, P1, P2, r2).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r2; nb++; }
		}

		quadratic_bezier_intersect_vline(P0, P1, P2, B.max[0], r1, r2);
		if (r1 > 0)
		{
			double y = quadratic_bezier_eval(P0, P1, P2, r1).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double y = quadratic_bezier_eval(P0, P1, P2, r2).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r2; nb++; }
		}

		quadratic_bezier_intersect_hline(P0, P1, P2, B.min[1], r1, r2);
		if (r1 > 0)
		{
			double x = quadratic_bezier_eval(P0, P1, P2, r1).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double x = quadratic_bezier_eval(P0, P1, P2, r2).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r2; nb++; }
		}

		quadratic_bezier_intersect_hline(P0, P1, P2, B.max[1], r1, r2);
		if (r1 > 0)
		{
			double x = quadratic_bezier_eval(P0, P1, P2, r1).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double x = quadratic_bezier_eval(P0, P1, P2, r2).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r2; nb++; }
		}

		if (nb >0) std::sort(restimes, restimes + nb);
		return nb;
		}



	/**
	* Compute the minimal bounding box of the curve.
	*
	* @param 	   	P0 	start point of the curve.
	* @param 	   	P1 	control point of the curve.
	* @param 	   	P2 	end point of the curve.
	*
	* @return	the curve bounding box.
	**/
	inline fBox2 quadratic_bezier_boundingbox(fVec2 P0, fVec2 P1, fVec2 P2)
		{
		fBox2 B(std::min<double>(P0.X(), P2.X()), std::max<double>(P0.X(), P2.X()), std::min<double>(P0.Y(), P2.Y()), std::max<double>(P0.Y(), P2.Y()));
		double rx;
		quadratic_bezier_vertical_tangent(P0, P1, P2, rx);
		if (rx > 0) { B.swallowPoint(quadratic_bezier_eval(P0, P1, P2, rx)); }
		double ry;
		quadratic_bezier_horizontal_tangent(P0, P1, P2, ry);
		if (ry > 0) { B.swallowPoint(quadratic_bezier_eval(P0, P1, P2, ry)); }
		return B;
		}


	/**
	* Compute the minimal bounding box of the curve.
	* integer version, conservative estimates.
	*
	* @param 	   	P0 	start point of the curve.
	* @param 	   	P1 	control point of the curve.
	* @param 	   	P2 	end point of the curve.
	*
	* @return	the curve bounding box.
	**/
	inline iBox2 quadratic_bezier_boundingbox(iVec2 P0, iVec2 P1, iVec2 P2)
		{
		fBox2 fB = quadratic_bezier_boundingbox((fVec2)P0, (fVec2)P1, (fVec2)P2);
		iBox2 iB = fB.integerEnclosingRect_larger();
		return iB;
		}


	/**
	* Split a curve in two part, [0,t] and [t,1].
	*
	* @param 	   	P0 	start point of the curve.
	* @param 	   	P1 	control point of the curve.
	* @param 	   	P2 	end point of the curve.
	* @param 	   	t  	splitting time.
	* @param [out]	P0a	start point for the curve on [0,t].
	* @param [out]	P1a	control point for the curve on [0,t].
	* @param [out]	P2a	end point for the curve on [0,t].
	* @param [out]	P0b	start point for the curve on [t,1].
	* @param [out]	P1b	control point for the curve on [t,1].
	* @param [out]	P2b	end point for the curve on [T,1].
	**/
	inline void quadratic_bezier_split(fVec2 P0, fVec2 P1, fVec2 P2, double t, fVec2 & P0a, fVec2 & P1a, fVec2 & P2a, fVec2 & P0b, fVec2 & P1b, fVec2 & P2b)
		{
		P0a = P0;
		P2b = P2;
		P2a = P0b = quadratic_bezier_eval(P0, P1, P2, t);
		P1a = (1 - t)*P0 + t * P1;
		P1b = (1 - t)*P1 + t * P2;
		}


	/**
	* Split a curve in two part, [0,t] and [t,1].
	* (integer version).
	*
	* @param 	   	P0 	start point of the curve.
	* @param 	   	P1 	control point of the curve.
	* @param 	   	P2 	end point of the curve.
	* @param 	   	t  	splitting time.
	* @param [out]	P0a	start point for the curve on [0,t].
	* @param [out]	P1a	control point for the curve on [0,t].
	* @param [out]	P2a	end point for the curve on [0,t].
	* @param [out]	P0b	start point for the curve on [t,1].
	* @param [out]	P1b	control point for the curve on [t,1].
	* @param [out]	P2b	end point for the curve on [T,1].
	**/
	inline void quadratic_bezier_split(iVec2 P0, iVec2 P1, iVec2 P2, double t, iVec2 & P0a, iVec2 & P1a, iVec2 & P2a, iVec2 & P0b, iVec2 & P1b, iVec2 & P2b)
		{
		fVec2 fP0a, fP1a, fP2a, fP0b, fP1b, fP2b;
		quadratic_bezier_split((fVec2)P0, (fVec2)P1, (fVec2)P2, t, fP0a, fP1a, fP2a, fP0b, fP1b, fP2b);
		P0a = (iVec2)fP0a; P1a = (iVec2)fP1a; P2a = (iVec2)fP2a;
		P0b = (iVec2)fP0b; P1b = (iVec2)fP1b; P2b = (iVec2)fP2b;
		}







	/**********************************************************************************
	*  RATIONAL QUADRATIC BEZIER CURVE
	*
	* Defined by 3 points P0, P1, P2 with associated weights (w0,w1,w2)
	*
	*  -> curve start at P0, end at P2 and with control point P1. 
	*  -> parametrisation : f(t) = ( w0 (1-t)^2 P0 + w1 2t(1-t) P1 + w2 t^2 P2 ) / (w0 (1-t)^2 + w1 2t(1-t)+ w2 t^2)  for t in [0,1].
	*
	* the curve can always be reparametrized such that w0 = w2 = 1 without changing the 
	* points Pi (but this changes the time parametrization). 
	**********************************************************************************/



	/**
	* Compute the position of the point on the curve at time t.
	*
	* @param 		  	P0 	start point of the curve.
	* @param 		  	w0 	weight of P0.
	* @param 		  	P1 	control point of the curve.
	* @param 		  	w1 	weight of P1.
	* @param 		  	P2 	start point of the curve.
	* @param 		  	w2 	weight of P2.
	* @param	t 	time.
	*
	* @return	The position on the curve at time t.
	**/
	inline fVec2 rational_bezier_eval(fVec2 P0, double w0,  fVec2 P1, double w1,  fVec2 P2, double w2,  double t)
		{
		double N = ((w0 - 2 * w1 + w2)*t + (-2 * w0 + 2 * w1))*t + w0;
		double X = ((w0*P0.X() - 2 * w1*P1.X() + w2 * P2.X())*t + (-2 * w0*P0.X() + 2 * w1*P1.X()))*t + w0 * P0.X();
		double Y = ((w0*P0.Y() - 2 * w1*P1.Y() + w2 * P2.Y())*t + (-2 * w0*P0.Y() + 2 * w1*P1.Y()))*t + w0 * P0.Y();
		return  fVec2(X/N,Y/N);
		}

	/**
	 * Compute the standard representation of a rational quadratic bezier curve where the endpoints
	 * have weight 1
	 *
	 * @param	w0	weight of the start point P0.
	 * @param	w1	weight of the control point P1.
	 * @param	w2	weight of the end point P2.
	 *
	 * @return	the weight W to give to P1 so that the same curve is given by (P0,1), (P1,W), (P2,1).
	 **/
	double rational_bezier_normalise(double w0, double w1, double w2)
		{
		MTOOLS_ASSERT(w0*w2 > 0);
		return sqrt((w1*w1) / (w0*w2));
		}


	/**
	* Compute the time the curve has an horizontal tangent.
	*
	* @param 			P0 	start point of the cure.
	* @param 			w0 	weight of P0.
	* @param 			P1 	control point of the curve.
	* @param 			w1 	weight of P1.
	* @param 			P2 	start point of the curve.
	* @param 			w2 	weight of P2.
	* @param [out]		r1	store possible horizontal tangent time (<0 if not valid).
	* @param [out]		r2	store possible horizontal tangent time (<0 if not valid).
	**/
	inline void rational_bezier_horizontal_tangent(fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2, double & r1, double & r2)
		{
		bezier_internals::_rational_bezier_solve_deriv(P0.Y(), P1.Y(), P2.Y(), w0, w1, w2, r1 , r2);
		}


	/**
	* Compute the time the curve has a vertical tangent.
	*
	* @param 			P0 	start point of the cure.
	* @param 			w0 	weight of P0.
	* @param 			P1 	control point of the curve.
	* @param 			w1 	weight of P1.
	* @param 			P2 	start point of the curve.
	* @param 			w2 	weight of P2.
	* @param [out]		r1	store possible vertical tangent time (<0 if not valid).
	* @param [out]		r2	store possible vertical tangent time (<0 if not valid).
	**/
	inline void rational_bezier_vertical_tangent(fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2, double & r1, double & r2)
		{
		bezier_internals::_rational_bezier_solve_deriv(P0.X(), P1.X(), P2.X(), w0, w1, w2, r1, r2);
		}


	/**
	* Find the times when the curve intersect the horizontal line Y = y0.
	*
	* @param 			P0 	start point of the cure.
	* @param 			w0 	weight of P0.
	* @param 			P1 	control point of the curve.
	* @param 			w1 	weight of P1.
	* @param 			P2 	start point of the curve.
	* @param 			w2 	weight of P2.
	* @param 	   		y0	line equation is X = x0.
	* @param [out]		r1	store possible intersection time (<0 if not valid).
	* @param [out]		r2	store possible intersection time (<0 if not valid).
	**/
	inline void rational_bezier_intersect_hline(fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2, double y0, double & r1, double & r2)
		{
		bezier_internals::_rational_bezier_solve(P0.Y(), P1.Y(), P2.Y(), w0, w1, w2, y0, r1, r2);
		}


	/**
	* Find the times when the curve intersect the vertical line X = x0.
	*
	* @param 			P0 	start point of the cure.
	* @param 			w0 	weight of P0.
	* @param 			P1 	control point of the curve.
	* @param 			w1 	weight of P1.
	* @param 			P2 	start point of the curve.
	* @param 			w2 	weight of P2.
	* @param 	   		y0	line equation is X = x0.
	* @param [out]		r1	store possible intersection time (<0 if not valid).
	* @param [out]		r2	store possible intersection time (<0 if not valid).
	**/
	inline void rational_bezier_intersect_vline(fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2, double x0, double & r1, double & r2)
		{
		bezier_internals::_rational_bezier_solve(P0.X(), P1.X(), P2.X(), w0, w1, w2, x0, r1, r2);
		}



	/**
	* Compute the times when a curve intersect a rectangle.
	*
	* @param 	   	B			The rectangle to test.
	* @param 		P0 			start point of the cure.
	* @param 		w0 			weight of P0.
	* @param 		P1 			control point of the curve.
	* @param 		w1 			weight of P1.
	* @param 		P2 			start point of the curve.
	* @param 		w2 			weight of P2.
	* @param [out]	restimes	an array of size at least 8 to store the intersection times. There are
	* 							returned ordered increasingly.
	*
	* @return	The number of intersection times found.
	**/
	inline int rational_bezier_intersect_rect(fBox2 B, fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2, double(&restimes)[8])
	{
		int nb = 0;
		double r1, r2;

		rational_bezier_intersect_vline(P0, w0, P1, w1, P2, w2, B.min[0], r1, r2);
		if (r1 > 0)
		{
			double y = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r1).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double y = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r2).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r2; nb++; }
		}

		rational_bezier_intersect_vline(P0, w0, P1, w1, P2, w2, B.max[0], r1, r2);
		if (r1 > 0)
		{
			double y = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r1).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double y = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r2).Y();
			if ((y >= B.min[1]) && (y <= B.max[1])) { restimes[nb] = r2; nb++; }
		}

		rational_bezier_intersect_hline(P0, w0, P1, w1, P2, w2, B.min[1], r1, r2);
		if (r1 > 0)
		{
			double x = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r1).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double x = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r2).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r2; nb++; }
		}

		rational_bezier_intersect_hline(P0, w0, P1, w1, P2, w2, B.max[1], r1, r2);
		if (r1 > 0)
		{
			double x = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r1).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r1; nb++; }
		}
		if (r2 > 0)
		{
			double x = rational_bezier_eval(P0, w0, P1, w1, P2, w2, r2).X();
			if ((x >= B.min[0]) && (x <= B.max[0])) { restimes[nb] = r2; nb++; }
		}

		if (nb >0) std::sort(restimes, restimes + nb);
		return nb;
	}



	/**
	* Compute the minimal bounding box of the curve.
	*
	* @param 		P0 			start point of the cure.
	* @param 		w0 			weight of P0.
	* @param 		P1 			control point of the curve.
	* @param 		w1 			weight of P1.
	* @param 		P2 			start point of the curve.
	* @param 		w2 			weight of P2.
	*
	* @return	the curve bounding box.
	**/
	inline fBox2 rational_bezier_boundingbox(fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2)
		{
		fBox2 B(std::min<double>(P0.X(), P2.X()), std::max<double>(P0.X(), P2.X()), std::min<double>(P0.Y(), P2.Y()), std::max<double>(P0.Y(), P2.Y()));
		double rx1, rx2;
		rational_bezier_vertical_tangent(P0, w0, P1, w1, P2, w2, rx1, rx2);
		if (rx1 > 0) { B.swallowPoint(rational_bezier_eval(P0, w0, P1, w1, P2, w2, rx1)); }
		if (rx2 > 0) { B.swallowPoint(rational_bezier_eval(P0, w0, P1, w1, P2, w2, rx2)); }
		double ry1,ry2;
		rational_bezier_horizontal_tangent(P0, w0, P1, w1, P2, w2, ry1, ry2);
		if (ry1 > 0) { B.swallowPoint(rational_bezier_eval(P0, w0, P1, w1, P2, w2, ry1)); }
		if (ry2 > 0) { B.swallowPoint(rational_bezier_eval(P0, w0, P1, w1, P2, w2, ry2)); }
		return B;
		}


	/**
	* Compute the minimal bounding box of the curve.
	* integer version, conservative estimates.
	*
	* @param 		P0 			start point of the cure.
	* @param 		w0 			weight of P0.
	* @param 		P1 			control point of the curve.
	* @param 		w1 			weight of P1.
	* @param 		P2 			start point of the curve.
	* @param 		w2 			weight of P2.
	*
	* @return	the curve bounding box.
	**/
	inline iBox2 rational_bezier_boundingbox(iVec2 P0, double w0, iVec2 P1, double w1, iVec2 P2, double w2)
		{
		fBox2 fB = rational_bezier_boundingbox((fVec2)P0, w0, (fVec2)P1, w1, (fVec2)P2, w2);
		iBox2 iB = fB.integerEnclosingRect_larger();
		return iB;
		}


	/**
	 * Split a curve in two part, [0,t] and [t,1].
	 *
	 * @param 		  	P0 	start point of the curve.
	 * @param 		  	w0 	weight of P0.
	 * @param 		  	P1 	control point of the curve.
	 * @param 		  	w1 	weight of P1.
	 * @param 		  	P2 	start point of the curve.
	 * @param 		  	w2 	weight of P2.
	 * @param 		  	t  	splitting time.
	 * @param [out]	  	P0a	start point for the curve on [0,t].
	 * @param [out]		w0a	weight of P0a.
	 * @param [out]	  	P1a	control point for the curve on [0,t].
	 * @param [out]		w0a	weight of P1a.
	 * @param [out]	  	P2a	end point for the curve on [0,t].
	 * @param [out]		w0a	weight of P2a.
	 * @param [out]	  	P0b	start point for the curve on [t,1].
	 * @param [out]		w0a	weight of P0b.
	 * @param [out]	  	P1b	control point for the curve on [t,1].
	 * @param [out]		w0a	weight of P1b.
	 * @param [out]	  	P2b	end point for the curve on [T,1].
	 * @param [out]		w0a	weight of P2b.
	 **/
	inline void rational_bezier_split(fVec2 P0, double w0, fVec2 P1, double w1, fVec2 P2, double w2,  
		                              double t, 
		                              fVec2 & P0a, double & w0a, fVec2 & P1a, double & w1a, fVec2 & P2a, double & w2a, 
		                              fVec2 & P0b, double & w0b, fVec2 & P1b, double & w1b, fVec2 & P2b, double & w2b) ///< The 2b)
		{
		w0a = w0;
		P0a = P0;
		w2b = w2;
		P2b = P2;
		w1a = (1 - t)*w0 + t * w1;
		P1a = (1 - t)*w0*P0 + t *w1*P1;
		w1b = (1 - t)*w1 + t * w2;
		P1b = (1 - t)*w1*P1 + t * w2*P2;
		w2a = w0b = (1 - t)*w1a + t * w1b;
		P2a = P0b = ((1 - t)*P1a + t * P1b )/ w2a;
		P1a /= w1a;
		P1b /= w1b;
		}


	/**
	* Split a curve in two part, [0,t] and [t,1].
	* (integer version)
	*
	* @param 		  	P0 	start point of the curve.
	* @param 		  	w0 	weight of P0.
	* @param 		  	P1 	control point of the curve.
	* @param 		  	w1 	weight of P1.
	* @param 		  	P2 	start point of the curve.
	* @param 		  	w2 	weight of P2.
	* @param 		  	t  	splitting time.
	* @param [out]	  	P0a	start point for the curve on [0,t].
	* @param [out]		w0a	weight of P0a.
	* @param [out]	  	P1a	control point for the curve on [0,t].
	* @param [out]		w0a	weight of P1a.
	* @param [out]	  	P2a	end point for the curve on [0,t].
	* @param [out]		w0a	weight of P2a.
	* @param [out]	  	P0b	start point for the curve on [t,1].
	* @param [out]		w0a	weight of P0b.
	* @param [out]	  	P1b	control point for the curve on [t,1].
	* @param [out]		w0a	weight of P1b.
	* @param [out]	  	P2b	end point for the curve on [T,1].
	* @param [out]		w0a	weight of P2b.
	**/
	inline void rational_bezier_split(iVec2 P0, double w0, iVec2 P1, double w1, iVec2 P2, double w2,
									  double t,
									  iVec2 & P0a, double & w0a, iVec2 & P1a, double & w1a, iVec2 & P2a, double & w2a,
									  iVec2 & P0b, double & w0b, iVec2 & P1b, double & w1b, iVec2 & P2b, double & w2b) ///< The 2b)
		{
		fVec2 fP0a, fP1a, fP2a, fP0b, fP1b, fP2b;
		rational_bezier_split((fVec2)P0, w0,  (fVec2)P1, w1, (fVec2)P2, w2, t, fP0a, w0a, fP1a, w1a, fP2a, w2a, fP0b, w0b, fP1b, w1b,  fP2b, w2b);
		P0a = (iVec2)fP0a; P1a = (iVec2)fP1a; P2a = (iVec2)fP2a;
		P0b = (iVec2)fP0b; P1b = (iVec2)fP1b; P2b = (iVec2)fP2b;
		}








	/**********************************************************************************
	*  CUBIC BEZIER CURVE
	*
	* Defined by 4 points P0, P1, P2 and P3.
	*
	*  -> curve start at P0, end at P3 and with control point P1 and P2.
	*  -> parametrisation : f(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t)t^2 P2 + t^3 P3 for t in [0,1]. 
	*
	**********************************************************************************/




	struct BezierQuadratic
		{

		fVec2 P0;		// start point of the curve
		fVec2 P1;		// control point
		fVec2 P2;		// end point of the curve


		/** Constructor */
		BezierQuadratic() = default;


		/** Constructor */
		BezierQuadratic(fVec2 p0, fVec2 p1, fVec2 p2) : P0(p0), P1(p1), P2(p2) {}


		/** compute the position of the poont on the curve at time t. */
		inline fVec2 eval(double t) const
			{
			return  fVec2(((P0.X() - 2 * P1.X() + P2.X())*t + 2 * (P1.X() - P0.X()))*t + P0.X(),
				          ((P0.Y() - 2 * P1.Y() + P2.Y())*t + 2 * (P1.Y() - P0.Y()))*t + P0.Y());
			}


		/** compute the position of the poont on the curve at time t (same as eval()). */
		fVec2 operator()(double t) const { return eval(t); }
		

		/**
		* Return the point of intersection with the line of equation Y = y0 
		* (return <0 if no intersecton).
		**/
		std::pair<double,double> intersect_hline(double y0) const
			{
			std::pair<double, double> res;
			_solve(P0.Y(), P1.Y(), P2.Y(), y0, res.first, res.second);
			return res;
			}


		/**
		* Return the point of intersection with the line of equation Y = y0
		* (return <0 if no intersecton).
		**/
		std::pair<double, double> intersect_vline(double x0) const
			{
			std::pair<double, double> res;
			_solve(P0.X(), P1.X(), P2.X(), x0, res.first, res.second);
			return res;
			}


		/**
		* Return the point where the curve has an horizontal tangent.
		* (return <0 if none).
		**/
		double tangent_h() const
			{
			double r;
			_solve_deriv(P0.Y(), P1.Y(), P2.Y(), r);
			return r;
			}


		/**
		* Return the point where the curve has a vertical tangent.
		* (return <0 if none).
		**/
		double tangent_v() const
			{
			double r;
			_solve_deriv(P0.X(), P1.X(), P2.X(), r);
			return r;
			}


		/**
		* Compute all the intersection times between the cuve and the rectangle B. 
		* the array res store the times of intersection (size at least 12)
		* 
		* return the number of intersection times found. 
		**/
		int intersect_rect(fBox2 B, double(&res)[12]) const
			{
			int nb = 0;
			std::pair<double, double> r;
			double & r1 = r.first;
			double & r2 = r.second;
			r = intersect_vline(B.min[0]);
			if (r1 > 0)
				{
				double y = eval(r1).Y();
				if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r1; nb++; }
				}
			if (r2 > 0)
				{
				double y = eval(r2).Y();
				if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r2; nb++; }
				}
			r = intersect_vline(B.max[0]);
			if (r1 > 0)
				{
				double y = eval(r1).Y();
				if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r1; nb++; }
				}
			if (r2 > 0)
				{
				double y = eval(r2).Y();
				if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r2; nb++; }
				}
			r = intersect_hline(B.min[1]);
			if (r1 > 0)
				{
				double x = eval(r1).X();
				if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r1; nb++; }
				}
			if (r2 > 0)
				{
				double x = eval(r2).X();
				if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r2; nb++; }
				}
			r = intersect_hline(B.max[1]);
			if (r1 > 0)
				{
				double x = eval(r1).X();
				if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r1; nb++; }
				}
			if (r2 > 0)
				{
				double x = eval(r2).X();
				if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r2; nb++; }
				}
			if (nb >0) std::sort(res, res + nb);
			return nb;
			}


		/**
		 * Compute the minimal bounding box of the curve 
		 **/
		fBox2 boundingBox() const
			{
			fBox2 B(std::min<double>(P0.X(), P2.X()), std::max<double>(P0.X(), P2.X()), std::min<double>(P0.Y(), P2.Y()), std::max<double>(P0.Y(), P2.Y()));
			double rx = tangent_v();
			if (rx > 0) { B.swallowPoint(eval(rx)); }
			double ry = tangent_h();
			if (ry > 0) { B.swallowPoint(eval(ry)); }
			return B;
			}


		/**
		* Return the 'integer valued' bounding box of the curve
		* (conservative estimates).
		**/
		iBox2 integerBoundingBox() const
			{
			fBox2 fB = boundingBox();
			return fB.integerEnclosingRect_larger();
			}



		/**
		* Split the curve in two: 
		* - first curve [0,T]  
		* - second curve [T,1]
		**/
		std::pair<BezierQuadratic, BezierQuadratic> split(double T) const
			{
			std::pair<BezierQuadratic, BezierQuadratic> R;
			BezierQuadratic & a = R.first;
			BezierQuadratic & b = R.second;
			a.P0 = P0;
			b.P2 = P2;
			a.P1 = (1 - T)*P0 + T * P1;
			b.P1 = (1 - T)*P1 + T * P2;
			a.P2 = b.P0 = (1 - T)*a.P1 + T * b.P1;
			return R;
			}



		private:


			/** solve when the derivative is zero. */
			static void _solve_deriv(double x0, double x1, double x2, double & r)
				{
				r = -1;
				double dem = x0 + x2 - 2 * x1;
				if (dem != 0)
					{
					double res = (x0 - x1) / dem;
					r = ((res > 1) ? -1 : res);
					}
				}


			/** Solve the equation for a given value */
			static void _solve(double x0, double x1, double x2, double z, double & r1, double & r2)
				{
				double a = x0 - 2 * x1 + x2;
				double b = 2 * (x1 - x0);
				double c = x0 - z;
				r1 = -1; r2 = -1;
				int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
				if (r1 >= 1) r1 = -1;
				if (r2 >= 1) r2 = -1;
				}

	};


    }


/* end of file */


