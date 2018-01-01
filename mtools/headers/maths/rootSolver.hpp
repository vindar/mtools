/** @file rootSolver.hpp */
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


namespace mtools
{

	/**
	 * Finds the real roots of a.x^2 + b.x + c = 0.  
	 * 
	 * Roots are returned ordered by increasing values. Values of x0,x1 is not modified if the root
	 * does not exist. By convention, the null polynom has no roots (returns 0 and x0,x1 are
	 * untouched).
	 * 
	 * Taken from poly/solve_quadratic.c in the GSL Library. https://www.gnu.org/software/gsl/.
	 *
	 * @param	a		  	coeff. of x^2.
	 * @param	b		  	coeff. of x.
	 * @param	c		  	coeff. of 1.
	 * @param [in,out]	x0	the smallest real root (if it exist).
	 * @param [in,out]	x1	the largest real root (if it exist).
	 *
	 * @return	The number of real roots (either 0 or 2).
	 **/
	int gsl_poly_solve_quadratic(double a, double b, double c, double *x0, double *x1);


	/**
	 * Finds the real roots of k.x^3 + a.x^2 + b.x + c = 0.
	 *
	 * Roots are returned ordered by increasing values. Values of x0,x1,x2 is not modified if the
	 * root does not exist. By convention, the null polynom has no roots (returns 0 and x0,x1,x2 are
	 * untouched).
	 *
	 * Taken from poly/solve_cubic.c in the GSL Library. https://www.gnu.org/software/gsl/.
	 *
	 * @param	k		  	coeff. of x^3.
	 * @param	a		  	coeff. of x^2.
	 * @param	b		  	coeff. of x.
	 * @param	c		  	coeff. of 1.
	 * @param [in,out]	x0	The smallest real root (if is exists).
	 * @param [in,out]	x1	The second smallest real root (if it exists).
	 * @param [in,out]	x2	The largest root (it it exists).
	 *
	 * @return	The number of real roots.
	 **/
	int gsl_poly_solve_cubic(double k, double a, double b, double c, double *x0, double *x1, double *x2);


}



/* end of file */



