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

  

	/******************************************************************************************
	* Interface for a general Bezier curve structure.
	*
	* Specialized in - BezierQuadratic
	*                - BezierRationalQuadraticQuadratic
	*                - BezierCubic
	*
	*******************************************************************************************/
	struct Bezier
	{

		/** default constructor */
		Bezier() = default;


		/**
		 * The start point of the curve (at t=0). 
		 **/
		virtual fVec2 startPoint() const = 0; 

		/**
		* The end point of the curve (at t=1).
		**/
		virtual fVec2 endPoint() const = 0;


		/**
		 * Return the position of the point at time t.
		 **/
		virtual fVec2 eval(double t) const = 0;


		/**
		* Return the position of the point at time t (same as eval()).
		**/
		fVec2 operator()(double t) const { return eval(t); }


		/**
		* Set the standard representation for rational curves.
		* Does nothing for non-rational curves.
		**/
		virtual void normalize() {}


		/**
		* Return the times of intersection of the curve with the vertical line X = x; 
		* (<0 for unused values).
		**/
		virtual std::tuple<double, double, double> intersect_vline(double x) const = 0;


		/**
		* Return the times of intersection of the curve with the horizontal line X = x;
		* (<0 for unused values).
		**/
		virtual std::tuple<double, double, double> intersect_hline(double y) const = 0;


		/**
		* Return the times the curve has a vertical tangent.
		* (<0 for unused values).
		**/
		virtual std::pair<double,double> tangent_v() const = 0;


		/**
		* Return the times the curve has an horizontal tangent.
		* (<0 for unused values).
		**/
		virtual std::pair<double, double> tangent_h() const = 0;


		/**
		* Compute all the intersection times between the cuve and the rectangle B. 
		* the array res store the times of intersection (size at least 12)
		* 
		* return the number of intersection times found. 
		**/
		int intersect_rect(fBox2 B, double(&res)[12]) const
			{
			int nb = 0;
			std::tuple<double, double, double> r;
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			double & r3 = std::get<2>(r);
			r = intersect_vline(B.min[0]);
			if (r1 > 0) { double y = eval(r1).Y(); if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r1; nb++; } }
			if (r2 > 0) { double y = eval(r2).Y(); if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r2; nb++; } }
			if (r3 > 0) { double y = eval(r3).Y(); if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r3; nb++; } }
			r = intersect_vline(B.max[0]);
			if (r1 > 0) { double y = eval(r1).Y(); if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r1; nb++; } }
			if (r2 > 0) { double y = eval(r2).Y(); if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r2; nb++; } }
			if (r3 > 0) { double y = eval(r3).Y(); if ((y >= B.min[1]) && (y <= B.max[1])) { res[nb] = r3; nb++; } }
			r = intersect_hline(B.min[1]);
			if (r1 > 0) { double x = eval(r1).X(); if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r1; nb++; } }
			if (r2 > 0) { double x = eval(r2).X(); if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r2; nb++; } }
			if (r3 > 0) { double x = eval(r3).X(); if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r3; nb++; } }
			r = intersect_hline(B.max[1]);
			if (r1 > 0) { double x = eval(r1).X(); if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r1; nb++; } }
			if (r2 > 0) { double x = eval(r2).X(); if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r2; nb++; } }
			if (r3 > 0) { double x = eval(r3).X(); if ((x >= B.min[0]) && (x <= B.max[0])) { res[nb] = r3; nb++; } }
			if (nb > 0) std::sort(res, res + nb);
			return nb;
			}


		/**
		 * Compute the minimal bounding box of the curve 
		 **/
		fBox2 boundingBox() const
			{
			fVec2 S = startPoint();
			fVec2 E = endPoint();
			fBox2 B(std::min<double>(S.X(), E.X()), std::max<double>(S.X(), E.X()), std::min<double>(S.Y(), E.Y()), std::max<double>(S.Y(), E.Y()));
			std::pair<double, double> r;
			double & r1 = r.first;
			double & r2 = r.second;
			r = tangent_v();
			if (r1 > 0) { B.swallowPoint(eval(r1)); }
			if (r2 > 0) { B.swallowPoint(eval(r2)); }
			r = tangent_h();
			if (r1 > 0) { B.swallowPoint(eval(r1)); }
			if (r2 > 0) { B.swallowPoint(eval(r2)); }
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


	};







	/******************************************************************************************
	* Structure representing a quadratic Bezier curve.
	*
	* Defined by 3 points P0, P1, P2.
	*
	*  -> curve start at P0, end at P2 and with control point P1.
	*  -> parametrisation : f(t) = (1-t)^2 P0 + 2t(1-t) P1 + t^2 P2.
	*******************************************************************************************/
	struct BezierQuadratic : public Bezier
		{

		fVec2 P0;		//< start point of the curve
		fVec2 P1;		//< control point
		fVec2 P2;		//< end point of the curve


		/** Constructor */
		BezierQuadratic() = default;


		/**
		 * Constructor.
		 *
		 * @param	p0	The start point of the curve (t=0)
		 * @param	p1	The control point
		 * @param	p2	The end point of the curve (t=1)
		 **/
		BezierQuadratic(fVec2 p0, fVec2 p1, fVec2 p2) : P0(p0), P1(p1), P2(p2) {}


		/* start point of the curve */
		virtual fVec2 startPoint() const override { return P0; }


		/* end point of the curve */
		virtual fVec2 endPoint() const override { return P2; }


		/** compute the position of the point on the curve at time t. */
		virtual inline fVec2 eval(double t) const override
			{
			return  fVec2(((P0.X() - 2 * P1.X() + P2.X())*t + 2 * (P1.X() - P0.X()))*t + P0.X(),
				          ((P0.Y() - 2 * P1.Y() + P2.Y())*t + 2 * (P1.Y() - P0.Y()))*t + P0.Y());
			}

		
		/**
		* Return the points of intersection with the line of equation Y = y0 
		* (return <0 if no intersecton).
		**/
		virtual std::tuple<double,double, double> intersect_hline(double y0) const override
			{
			std::tuple<double, double, double> r{ -1,-1,-1 };
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			_solve(P0.Y(), P1.Y(), P2.Y(), y0, r1, r2);
			return r;
			}


		/**
		* Return the points of intersection with the line of equation Y = y0
		* (return <0 if no intersecton).
		**/
		virtual std::tuple<double, double, double> intersect_vline(double x0) const override
			{
			std::tuple<double, double, double> r{ -1,-1,-1 };
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			_solve(P0.X(), P1.X(), P2.X(), x0, r1, r2);			
			return r;
			}


		/**
		* Return the point where the curve has an horizontal tangent.
		* (return <0 if none).
		**/
		virtual std::pair<double,double> tangent_h() const override
			{
			std::pair<double, double> r{ -1,-1 };
			double & r1 = r.first;
			_solve_deriv(P0.Y(), P1.Y(), P2.Y(), r1);
			return r;
			}


		/**
		* Return the point where the curve has a vertical tangent.
		* (return <0 if none).
		**/
		virtual std::pair<double, double> tangent_v() const override
			{
			std::pair<double, double> r{ -1,-1 };
			double & r1 = r.first;
			_solve_deriv(P0.X(), P1.X(), P2.X(), r1);
			return r;
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
				//int nb = 
					mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
				if (r1 >= 1) r1 = -1;
				if (r2 >= 1) r2 = -1;
				}

		};





	
	/******************************************************************************************
	* Structure representing a rational quadratic Bezier curve.
	* 
	* Defined by 3 points P0, P1, P2 and associated weights w1, w2, w3.
	* 
	*  -> curve start at P0, end at P2 and with control point P1.
	*                               w0 (1-t)^2 P0 + w1 2t(1-t) P1 + w2 t^2 P2 
	*  -> parametrisation : f(t) =  -----------------------------------------  for t in [0,1].
	*                                    w0 (1-t)^2 + w1 2t(1-t) + w2 t^2
	*
	* the curve can always be reparametrized such that w0 = w2 = 1 without changing the 
	* points Pi (but this changes the time parametrization). Use the normalize() method. 
	*******************************************************************************************/			
	struct BezierRationalQuadratic : public Bezier
		{

		fVec2 P0;		//< start point of the curve
		fVec2 P1;		//< control point
		fVec2 P2;		//< end point of the curve

		double w0;		//< weight of P0
		double w1;		//< weight of P1
		double w2;		//< weight of P2


		/** Constructor */
		BezierRationalQuadratic() = default;


		/**
		 * Constructor.
		 *
		 * @param	p0	the start point of the curve (t=0)
		 * @param	u0	weight of P0.
		 * @param	p1	the control point.
		 * @param	u1	weight of P1.
		 * @param	p2	the end point of the curve (t=1)
		 * @param	u2	weight of P2.
		 **/
		BezierRationalQuadratic(fVec2 p0, double u0, fVec2 p1, double u1, fVec2 p2, double u2) : P0(p0), P1(p1), P2(p2), w0(u0), w1(u1), w2(u2)  {}


		/* start point of the curve */
		virtual fVec2 startPoint() const override { return P0; }


		/* end point of the curve */
		virtual fVec2 endPoint() const override { return P2; }


		/** compute the position of the point on the curve at time t. */
		virtual inline fVec2 eval(double t) const override
			{
			double N = ((w0 - 2 * w1 + w2)*t + (-2 * w0 + 2 * w1))*t + w0;
			double X = ((w0*P0.X() - 2 * w1*P1.X() + w2 * P2.X())*t + (-2 * w0*P0.X() + 2 * w1*P1.X()))*t + w0 * P0.X();
			double Y = ((w0*P0.Y() - 2 * w1*P1.Y() + w2 * P2.Y())*t + (-2 * w0*P0.Y() + 2 * w1*P1.Y()))*t + w0 * P0.Y();
			return  fVec2(X / N, Y / N);
			}


		/**
		* Set the standard representation where the endpoints have weight 1
		**/
		virtual void normalize() override
			{
			MTOOLS_ASSERT(w0*w2 > 0);
			w1 = sqrt((w1*w1) / (w0*w2));
			w0 = 1;
			w2 = 1;
			}


		/**
		* Return the points of intersection with the line of equation Y = y0 
		* (return <0 if no intersecton).
		**/
		virtual std::tuple<double,double, double> intersect_hline(double y0) const override
			{
			std::tuple<double, double, double> r{ -1,-1,-1 };
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			_solve(P0.Y(), P1.Y(), P2.Y(), y0, r1, r2);
			return r;
			}


		/**
		* Return the points of intersection with the line of equation Y = y0
		* (return <0 if no intersecton).
		**/
		virtual std::tuple<double, double, double> intersect_vline(double x0) const override
			{
			std::tuple<double, double, double> r{ -1,-1,-1 };
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			_solve(P0.X(), P1.X(), P2.X(), x0, r1, r2);
			return r;
			}


		/**
		* Return the point where the curve has an horizontal tangent.
		* (return <0 if none).
		**/
		virtual std::pair<double,double> tangent_h() const override
			{
			std::pair<double, double> r{ -1,-1 };
			double & r1 = r.first;
			double & r2 = r.second;
			_solve_deriv(P0.Y(), P1.Y(), P2.Y(), r1, r2);
			return r;
			}


		/**
		* Return the point where the curve has a vertical tangent.
		* (return <0 if none).
		**/
		virtual std::pair<double, double> tangent_v() const override
			{
			std::pair<double, double> r{ -1,-1 };
			double & r1 = r.first;
			double & r2 = r.second;
			_solve_deriv(P0.X(), P1.X(), P2.X(), r1, r2);
			return r;
			}


		/**
		* Split the curve in two: 
		* - first curve [0,T]  
		* - second curve [T,1]
		**/
		std::pair<BezierRationalQuadratic, BezierRationalQuadratic> split(double T) const
			{
			std::pair<BezierRationalQuadratic, BezierRationalQuadratic> R;
			BezierRationalQuadratic & a = R.first;
			BezierRationalQuadratic & b = R.second;
			a.w0 = w0;
			a.P0 = P0;
			b.w2 = w2;
			b.P2 = P2;
			a.w1 = (1 - T)*w0 + T * w1;
			a.P1 = (1 - T)*w0*P0 + T * w1*P1;
			b.w1 = (1 - T)*w1 + T * w2;
			b.P1 = (1 - T)*w1*P1 + T * w2*P2;
			a.w2 = b.w0 = (1 - T)*a.w1 + T * b.w1;
			a.P2 = b.P0 = ((1 - T)*a.P1 + T * b.P1) / a.w2;
			a.P1 /= a.w1;
			b.P1 /= b.w1;
			return R;
			}


		private:


			/** solve when the derivative is zero. */
			void _solve_deriv(double x0, double x1, double x2, double & r1, double & r2) const
			{
				double a = -2 * w0*w1*x0 + 2 * w0*w1*x1 + 2 * w0*w2*x0 - 2 * w0*w2*x2 - 2 * w1*w2*x1 + 2 * w1*w2*x2;
				double b = 4 * w0*w1*x0 - 4 * w0*w1*x1 - 2 * w0*w2*x0 + 2 * w0*w2*x2;
				double c = -2 * w0*w1*x0 + 2 * w0*w1*x1;
				r1 = -1; r2 = -1;
				//int nb =
					mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
				if (r1 >= 1) r1 = -1;
				if (r2 >= 1) r2 = -1;
			}


			/** Solve the equation for a given value */
			void _solve(double x0, double x1, double x2, double z, double & r1, double & r2) const
			{
				double a = x0 * w0 - 2 * x1*w1 + x2 * w2 - z * (w0 - 2 * w1 + w2);
				double b = -2 * x0*w0 + 2 * x1*w1 - z * (-2 * w0 + 2 * w1);
				double c = x0 * w0 - z * w0;
				r1 = -1; r2 = -1;
				//int nb = 
					mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
				if (r1 >= 1) r1 = -1;
				if (r2 >= 1) r2 = -1;
			}


		};





	/******************************************************************************************
	* Structure representing a cubic Bezier curve.
	*
	* Defined by 4 points P0, P1, P2 and P3.
	*
	*  -> curve start at P0, end at P3 and with control point P1 and P2.
	*  -> parametrisation : f(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3 (1-t) t^2 P2 + t^3 P3
	*
	*******************************************************************************************/			
	struct BezierCubic : public Bezier
		{

		fVec2 P0;		//< start point of the curve
		fVec2 P1;		//< first control point
		fVec2 P2;		//< second control point
		fVec2 P3;		//< end point of the curve


		/** Constructor */
		BezierCubic() = default;


		/**
		 * Constructor.
		 *
		 * @param	p0	the start point of the curve (t=0)
		 * @param	p1	the first control point.
		 * @param	p2	the second control point.
		 * @param	p3	the end point of the curve (t=1)
		 **/
		BezierCubic(fVec2 p0, fVec2 p1, fVec2 p2, fVec2 p3) : P0(p0), P1(p1), P2(p2), P3(p3)  {}


		/* start point of the curve */
		virtual fVec2 startPoint() const override { return P0; }


		/* end point of the curve */
		virtual fVec2 endPoint() const override { return P3; }


		/** compute the position of the point on the curve at time t. */
		virtual inline fVec2 eval(double t) const override
			{
			return  fVec2((((P3.X() + 3 * (P1.X() - P2.X()) - P0.X())*t + 3 * (P2.X() - 2 * P1.X() + P0.X()))*t + 3 * (P1.X() - P0.X()))*t + P0.X(),
			     	      (((P3.Y() + 3 * (P1.Y() - P2.Y()) - P0.Y())*t + 3 * (P2.Y() - 2 * P1.Y() + P0.Y()))*t + 3 * (P1.Y() - P0.Y()))*t + P0.Y());
			}


		/**
		* Return the points of intersection with the line of equation Y = y0 
		* (return <0 if no intersecton).
		**/
		virtual std::tuple<double,double, double> intersect_hline(double y0) const override
			{
			std::tuple<double, double, double> r{ -1,-1,-1 };
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			double & r3 = std::get<2>(r);
			_solve(P0.Y(), P1.Y(), P2.Y(), P3.Y(), y0, r1, r2, r3);
			return r;
			}


		/**
		* Return the points of intersection with the line of equation Y = y0
		* (return <0 if no intersecton).
		**/
		virtual std::tuple<double, double, double> intersect_vline(double x0) const override
			{
			std::tuple<double, double, double> r{ -1,-1,-1 };
			double & r1 = std::get<0>(r);
			double & r2 = std::get<1>(r);
			double & r3 = std::get<2>(r);
			_solve(P0.X(), P1.X(), P2.X(), P3.X(), x0, r1, r2, r3);
			return r;
			}


		/**
		* Return the point where the curve has an horizontal tangent.
		* (return <0 if none).
		**/
		virtual std::pair<double,double> tangent_h() const override
			{
			std::pair<double, double> r{ -1,-1 };
			double & r1 = r.first;
			double & r2 = r.second;
			_solve_deriv(P0.Y(), P1.Y(), P2.Y(), P3.Y(), r1, r2);
			return r;
			}


		/**
		* Return the point where the curve has a vertical tangent.
		* (return <0 if none).
		**/
		virtual std::pair<double, double> tangent_v() const override
			{
			std::pair<double, double> r{ -1,-1 };
			double & r1 = r.first;
			double & r2 = r.second;
			_solve_deriv(P0.X(), P1.X(), P2.X(), P3.X(), r1, r2);
			return r;
			}


		/**
		* Split the curve in two: 
		* - first curve [0,T]  
		* - second curve [T,1]
		**/
		std::pair<BezierCubic, BezierCubic> split(double T) const
			{
			std::pair<BezierCubic, BezierCubic> R;
			BezierCubic & a = R.first;
			BezierCubic & b = R.second;
			a.P0 = P0;
			b.P3 = P3;
			a.P1 = (1 - T)*P0 + T * P1;
			b.P2 = (1 - T)*P2 + T * P3;
			fVec2 C = (1 - T)*P1 + T * P2;
			a.P2 = (1 - T)*a.P1 + T * C;
			b.P1 = (1 - T)*C + T * b.P2;
			a.P3 = b.P0 = (1 - T)*a.P2 + T * b.P1;
			return R;
			}


		private:

			/** solve when the derivative is zero. */
			static void _solve_deriv(double x0, double x1, double x2, double x3, double & r1, double & r2)
				{
				double a = (3 * x1 - 3 * x2 + x3 - x0);
				double b = 2 * (x0 - 2 * x1 + x2);
				double c = (x1 - x0);
				r1 = -1; r2 = -1;
				//int nb = 
					mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
				if (r1 >= 1) r1 = -1;
				if (r2 >= 1) r2 = -1;
				}


			/** Solve the equation for a given value */
			static void _solve(double x0, double x1, double x2, double x3, double z, double & r1, double & r2, double & r3)
				{
				double a = x3 + 3 * (x1 - x2) - x0;
				double b = 3 * (x0 - 2 * x1 + x2);
				double c = 3 * (x1 - x0);
				double d = x0 - z;
				r1 = -1; r2 = -1; r3 = -1;
				//int nb = 
					mtools::gsl_poly_solve_cubic(a, b, c, d, &r1, &r2, &r3);
				if (r1 >= 1) r1 = -1;
				if (r2 >= 1) r2 = -1;
				if (r3 >= 1) r3 = -1;
				}

		};


	/**
	 * Splits a curve into sub-curve by keeping only the parts of the original curve that stay
	 * inside the box.
	 *
	 * @tparam	BezierClass	Type of the bezier curve.
	 * @param 		  	B		 	the box to consider.
	 * @param 		  	curve	 	The initial curve.
	 * @param [in,out]	subcurves	array to hold the sub-curves (at most 5).
	 *
	 * @return	The number of sub-curves in the resulting array (this may be 0 if the initial curve
	 * 			does not enter the box B).
	 **/
	template<typename BezierClass> int splitBezierInsideBox(fBox2 B, BezierClass curve, BezierClass(&subcurves)[5])
		{
		double res[12];
		int nb = curve.intersect_rect(B, res); // compute intersection times with B. 
		for (int i = (nb - 1); i > 0; i--)
			{ // time rescaling for sub-curve
			res[i] = (res[i] - res[i - 1]) / (1.0 - res[i - 1]);
			}
		int tot = 0;
		for (int i = 0; i < nb; i++)
			{
			std::tie(subcurves[tot], curve) = curve.split(res[i]);
			if (B.isInside(subcurves[tot].eval(0.5)))
				{
				subcurves[tot].normalize();
				tot++;
				}
			}
		if (B.isInside(curve.eval(0.5)))
			{
			subcurves[tot] = curve;
			subcurves[tot].normalize();
			tot++;
			}
		return tot; 
		}







    }


/* end of file */


