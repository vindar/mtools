
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:



	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}
	












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
		imA.draw_ellipse(center[i], rx[i], ry[i], RGBc::getDistinctColor(i),true,true,3);
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



}



/**********************************************************************************
 *  QUADRATIC BEZIER CURVE
 *  
 * Defined by 3 point P0, P1, P2.   
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
inline void quadratic_bezier_intersect_hline(fVec2 P0, fVec2 P1, fVec2 P2,  double y0, double & r1, double & r2)
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
inline int quadratic_bezier_intersect_rect(fBox2 B, fVec2 P0, fVec2 P1, fVec2 P2, double (&restimes)[8])
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






void testQuad(const fBox2 & B, fVec2 P0, fVec2 P1, fVec2 P2, Image & im)
{
	RGBc color;
	auto C = B;
	C.enlarge(2);
	double res[8];
	int nb = quadratic_bezier_intersect_rect(C, P0, P1, P2, res);
	for (int i = (nb - 1); i > 0; i--)
		{
		res[i] = (res[i] - res[i-1])/(1.0 - res[i-1]);
		}

	for (int i = 0; i < nb; i++)
		{
		fVec2 P0a, P1a, P2a, P0b, P1b, P2b;
		quadratic_bezier_split(P0, P1, P2, res[i], P0a, P1a, P2a, P0b, P1b, P2b);
		P0 = P0b;  P1 = P1b; P2 = P2b;

		auto bb = quadratic_bezier_boundingbox(P0a, P1a, P2a);												// comput the bb. 
		color = (C.isInside(quadratic_bezier_eval(P0a, P1a, P2a, 0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
		im.draw_quad_bezier(P0a, P2a, P1a, 1.0, color, true, true, true);
	}

	auto bb = quadratic_bezier_boundingbox(P0, P1, P2);								// comput the bb. 
	color = (C.isInside(quadratic_bezier_eval(P0, P1, P2, 0.5))) ? RGBc::c_Red : RGBc::c_Blue;	// set the color		
	im.draw_quad_bezier(P0, P2, P1, 1.0, color, true, true, true);
}








/**********************************************************************************
*  RATIONAL QUADRATIC BEZIER
**********************************************************************************/


/**
* Evaluate a rational quadratic bezier curve at position t.
**/
fVec2 _rat_bezier_eval(fVec2 P0, fVec2 P1, fVec2 P2, double w, double t)
	{
	if (w - 1 == 0) return quadratic_bezier_eval(P0, P1, P2, t);
	double D = 1 + 2 * (((1 - w)*t + (w - 1))*t);
	return fVec2( (((P0.X() - 2 * P1.X() * w + P2.X())*t + 2 * (w * P1.X() - P0.X()))*t + P0.X()) / D, 
		          (((P0.Y() - 2 * P1.Y() * w + P2.Y())*t + 2 * (w * P1.Y() - P0.Y()))*t + P0.Y()) / D );
	}


/**
* Find the roots inside [0,1] of the derivative of a rationnal quadratic bezier 
* equation if they exists.
**/
void _rat_bezier_solve_deriv(double x0, double x1, double x2, double w, double & r1, double & r2)
	{
	double a = (x2 - x0)*(w - 1);
	double b = ( (x0 -x1)*2*w + x2 - x0);
	double c = (x1*w - x0*w);
	r1 = -1;  r2 = -1;
	int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
	if (r1 >= 1) r1 = -1;
	if (r2 >= 1) r2 = -1;
	}


/**
* Find the root inside [0,1] of rational quad. bezier equation
* f(t) = z for the curve with parameters (x0,x1, x2, w)
**/
void _rat_bezier_solve(double x0, double x1, double x2, double w, double z, double & r1, double & r2)
{
	double a = (-2 * x1*w + x0 + x2 - z * (-2 * w + 2));
	double b = 2 * (x1*w - x0 - z * (w - 1));
	double c = x0 - z;
	r1 = -1; r2 = -1;
	int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
	if (r1 >= 1) r1 = -1;
	if (r2 >= 1) r2 = -1;
}



fBox2 ratBezierBoundingBox(fVec2 P0, fVec2 P1, fVec2 P2, double w)
	{
	fBox2 B(std::min<double>(P0.X(), P2.X()), std::max<double>(P0.X(), P2.X()), std::min<double>(P0.Y(), P2.Y()), std::max<double>(P0.Y(), P2.Y()));
	double rx1, rx2;
	_rat_bezier_solve_deriv(P0.X(), P1.X(), P2.X(), w, rx1, rx2);
	if (rx1 > 0) { B.swallowPoint(_rat_bezier_eval(P0, P1, P2, w, rx1)); }
	if (rx2 > 0) { B.swallowPoint(_rat_bezier_eval(P0, P1, P2, w, rx2)); }
	double ry1, ry2;
	_rat_bezier_solve_deriv(P0.Y(), P1.Y(), P2.Y(), w, ry1, ry2);
	if (ry1 > 0) { B.swallowPoint(_rat_bezier_eval(P0, P1, P2, w, ry1)); }
	if (ry2 > 0) { B.swallowPoint(_rat_bezier_eval(P0, P1, P2, w, ry2)); }
	return B;
	}


iBox2 ratBezierBoundingBox(iVec2 P0, iVec2 P1, iVec2 P2, double w)
	{
	fBox2 fB = ratBezierBoundingBox((fVec2)P0, (fVec2)P1, (fVec2)P2, w);
	iBox2 iB = fB.integerEnclosingRect_larger();
	return iB;
	}






/**********************************************************************************
*  CUBIC BEZIER
**********************************************************************************/


/**
* Evaluate a cubic bezier curve at position t.
**/
fVec2 _cubic_bezier_eval(fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3, double t)
	{
	return  fVec2( (((P3.X() + 3 * (P1.X() - P2.X()) - P0.X())*t + 3 * (P2.X() - 2 * P1.X() + P0.X()))*t + 3 * (P1.X() - P0.X()))*t + P0.X(),
		           (((P3.Y() + 3 * (P1.Y() - P2.Y()) - P0.Y())*t + 3 * (P2.Y() - 2 * P1.Y() + P0.Y()))*t + 3 * (P1.Y() - P0.Y()))*t + P0.Y() );
	}


/**
* Find the roots inside [0,1] of the derivative of a cubic bezier equation
* if it exists.
**/
void _cubic_bezier_solve_deriv(double x0, double x1, double x2, double x3, double & r1, double & r2)
	{
	double a = (3 * x1 - 3 * x2 + x3 - x0);
	double b = 2 * (x0 - 2*x1 + x2);
	double c = (x1 - x0);
	r1 = -1; r2 = -1;
	int nb = mtools::gsl_poly_solve_quadratic(a, b, c, &r1, &r2);
	if (r1 >= 1) r1 = -1;
	if (r2 >= 1) r2 = -1;
	}


/**
* Find the root inside [0,1] of rational quad. bezier equation
* f(t) = z for the curve with parameters (x0,x1, x2, x3)
**/
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



fBox2 cubicBezierBoundingBox(fVec2 P0, fVec2 P1, fVec2 P2, fVec2 P3)
	{
	fBox2 B(std::min<double>(P0.X(), P3.X()), std::max<double>(P0.X(), P3.X()), std::min<double>(P0.Y(), P3.Y()), std::max<double>(P0.Y(), P3.Y()));

	double rx1, rx2;
	_cubic_bezier_solve_deriv(P0.X(), P1.X(), P2.X(),  P3.X(), rx1, rx2);
	if (rx1 > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, rx1)); }
	if (rx2 > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, rx2)); }

	double ry1, ry2;
	_cubic_bezier_solve_deriv(P0.Y(), P1.Y(), P2.Y(), P3.Y(), ry1, ry2);
	if (ry1 > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, ry1)); }
	if (ry2 > 0) { B.swallowPoint(_cubic_bezier_eval(P0, P1, P2, P3, ry2)); }

	return B;
	}


iBox2 cubicBezierBoundingBox(iVec2 P0, iVec2 P1, iVec2 P2, iVec2 P3)
	{
	fBox2 fB = cubicBezierBoundingBox((fVec2)P0, (fVec2)P1, (fVec2)P2, (fVec2)P3);
	iBox2 iB = fB.integerEnclosingRect_larger();
	return iB;
	}









void testCF()
{
	size_t N = 50000;
	int64 LX = 1000;
	int64 LY = 1000;

	TestImage im(LX, LY);
	im.clear(RGBc::RGBc(240,240,200));
	MT2004_64 gen(0);


	while (1)
	{
		im.clear(RGBc::RGBc(240, 240, 200));

		iVec2 P0 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P1 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P2 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };
		iVec2 P3 = { (int64)(Unif(gen)*LX), (int64)(Unif(gen)*LY) };

		double w = Unif(gen) * 0.1;

		cout << "P0 : " << P0 << "\n";
		cout << "P1 : " << P1 << "\n";
		cout << "P2 : " << P2 << "\n";
		cout << "w : " << w << "\n";

		/*
		P0 = { 226, 803 };
		P1 = { 600, 748};
		P2 = { 665, 154};
		P3 = { 604, 485};
		*/
		

		im.draw_dot(P0, RGBc::c_Green, true, 2);
		im.draw_dot(P1, RGBc::c_Green, true, 2);
		im.draw_dot(P2, RGBc::c_Green, true, 2);
		/*
		im.draw_line(P0, P1, RGBc::c_Green, false);
		im.draw_line(P1, P2, RGBc::c_Green, false);
		im.draw_line(P2, P3, RGBc::c_Green, false);
		*/


		auto bb = ratBezierBoundingBox(P0, P1, P2, 1.0);
		im.draw_box(bb, RGBc::c_Gray, true);

		iBox2 TB{ 300,600,400,600 };
		im.draw_box(TB, RGBc::c_Yellow.getMultOpacity(0.5), true);
		im.draw_rectangle(TB, RGBc::c_Yellow, true);

		testQuad(TB, P0, P1, P2, im);



		auto PA = makePlot2DImage(im, 1, "Image A");   // Encapsulate the image inside a 'plottable' object.	
		Plotter2D plotter;              // Create a plotter object
		plotter[PA];	                // Add the image to the list of objects to draw.  	
		plotter.autorangeXY();          // Set the plotter range to fit the image.
		plotter.plot();                 // start interactive display.
	}
}






	int main(int argc, char *argv[])
	{

		MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
		
		testCF();
		cout.getKey(); 
		return 0;

	


		
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
