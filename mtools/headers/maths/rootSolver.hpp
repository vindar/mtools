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

#include "../misc/internal/mtools_export.hpp"

#include <math.h>

namespace mtools
{



/**
 * finds the real roots of a x^2 + b x + c = 0
 * Return the number of real roots.
 * Roots are returned ordered.
 * 
 * Taken from poly/solve_quadratic.c in the GSL Library. 
 * https://www.gnu.org/software/gsl/
 **/
inline int gsl_poly_solve_quadratic (double a, double b, double c, double *x0, double *x1)
	{
	if (a == 0) /* Handle linear case */
		{
		if (b == 0) { return 0; } else { *x0 = -c / b; return 1; };
		}
    const double disc = b * b - 4 * a * c;    
    if (disc > 0)
		{
		if (b == 0)
			{
			const double r = sqrt (-c / a);
			*x0 = -r;
			*x1 =  r;
			}
        else
			{
			const double sgnb = (b > 0 ? 1 : -1);
			const double temp = -0.5 * (b + sgnb * sqrt (disc));
			const double r1 = temp / a ;
			const double r2 = c / temp ;
			if (r1 < r2) 
				{
				*x0 = r1 ;
				*x1 = r2 ;
				} 
            else 
				{
				*x0 = r2 ;
				*x1 = r1 ;
				}
			}
        return 2;
		}
    else if (disc == 0) 
		{
        *x0 = -0.5 * b / a ;
        *x1 = -0.5 * b / a ;
        return 2 ;
		}
    else return 0;
	}




}



/* end of file */

