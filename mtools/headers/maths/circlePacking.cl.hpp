/** @file circlePacking.cl..hpp */
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


namespace mtools 
	{

	namespace internals_circlepacking
		{


		/* The openCL program for circle packing
		   use by the CirclePackingLAbelGPU class from circlePacking.hpp
		   Algorithm from Stephenson & Collins (2003) */
		static const char * circlePacking_openCLprogram = R"CLsource(

//#pragma OPENCL EXTENSION cl_khr_fp64 : enable

//#define FPTYPE		  		// these defined are passed
//#define FPTYPE_VEC8 			// via compiler's option
//#define NBVERTICES     		// 
//#define MAXGROUPSIZE   		//


#define M_2PI 6.283185307179586477
#define M_1PI 3.141592653589793238


/** fast random number generator **/
inline uint rand(__global uint4  * g_gen) 
                {
				uint4 gen = (*g_gen);				
                gen.x ^= gen.x << 16; 
				gen.x ^= gen.x >> 5; 
				gen.x ^= gen.x << 1;
                gen.w = gen.x; 
				gen.x = gen.y; 
				gen.y = gen.z; 
				gen.z = gen.w ^ gen.x ^ gen.y;
				(*g_gen) = gen;				
                return gen.z;
                }
				
				
/** Compute the angle between the two circles of radius y and z 
    that adjacent to the circle of radius x **/
inline FPTYPE angleEuclidian(FPTYPE rx, FPTYPE ry, FPTYPE rz)
	{
	const FPTYPE a = rx + ry;
	const FPTYPE b = rx + rz;
	const FPTYPE c = ry + rz;
	const FPTYPE r = (a*a + b*b - c*c) / (2 * a*b);
	//const FPTYPE r = ((a/b) + (b/a) - (c/a)*(c/b))/2;	// <- this version is slower by 30% than the one above but much better at limiting the exponent swing... 	
	return((r >= 1.0) ? 0.0 : ((r <= -1.0) ? (M_1PI) : (acos(r))));
	}	


/** Update the radii **/	
__kernel void updateRadius(__global FPTYPE g_radiiTab1[NBVERTICES],						// original radii of each vertex
				      	   __global FPTYPE g_radiiTab2[NBVERTICES],						// updated radii of each vertex
                           __global const int g_degreeTab[NBVERTICES],					// degree of each vertex
                           __global const int g_neighbourOffset[NBVERTICES],			// offset to the list of neighbours of each vertex
						   __global const int * g_neighbourList,						// the list of neighbours
						   __global FPTYPE g_error[NBVERTICES],							// error term 
						   __global FPTYPE g_lambdastar[NBVERTICES] 					// max admissible lambda
					      )	
	{				
	const int index = get_global_id(0);				// global index
	const int deg = g_degreeTab[index]; 			// degree of the site
	if (deg == 0) return;							// dummy site, nothing to do

	const FPTYPE v  = g_radiiTab1[index];			// radius
	const int offset = g_neighbourOffset[index];	// offset for the list of neighbours

	// compute the sum angle, use Kahan summation algorithm to reduce roundoff errors
	FPTYPE theta = 0.0;
	FPTYPE C = 0.0;	
	const FPTYPE firstR = g_radiiTab1[g_neighbourList[offset]];	
	FPTYPE prevR = firstR;
	for(int k = 1; k < deg; k++) // add the angles due to the neighbours
		{ 
		FPTYPE nextR = g_radiiTab1[g_neighbourList[offset + k]];
		FPTYPE Y = angleEuclidian(v,prevR,nextR) - C;
		FPTYPE T = theta + Y;
		C = (T-theta) - Y;
		theta = T;
		prevR = nextR;
		}	
	theta += (angleEuclidian(v,prevR,firstR) - C); // close the flower

	// compute the new radius
	const FPTYPE ik     = 1.0/((FPTYPE)deg);
	const FPTYPE beta   = sin(theta*ik*0.5);
	const FPTYPE tildev = beta*v/(1.0-beta);
	const FPTYPE delta  = sinpi(ik);
	const FPTYPE u      = (1.0-delta)*tildev/delta;
	g_radiiTab2[index] 	= u;								// save new radius	

	const FPTYPE e      = theta - M_2PI;					// error term
	g_error[index]		= (e*e);							// save error

	const FPTYPE l 		= (v - u);							// delta between old and new radius	
	g_lambdastar[index] = ((l <= 0.0) ? 1.0e10 : (u/l));	// save lambdastar
	}

	
	
/** reduction kernel to sum up the errors and compute lambda star **/		
__kernel void reduction( __global FPTYPE * errorin, __global FPTYPE * errorout, __global FPTYPE * lambdain, __global FPTYPE * lambdaout) 
	{			
	__local FPTYPE temperror[MAXGROUPSIZE];
	__local FPTYPE templambda[MAXGROUPSIZE];
	const int lid   = get_local_id(0);
	const int gid   = get_global_id(0);
	const int gsize = get_local_size(0);
	temperror[lid] = errorin[gid];
	templambda[lid] = lambdain[gid];
	barrier(CLK_LOCAL_MEM_FENCE);
	for(int i = gsize/2; i>0; i >>= 1) 
		{
		if(lid < i) 
			{ 
			temperror[lid] += temperror[lid + i]; 			
			templambda[lid] = fmin(templambda[lid], templambda[lid + i]); 			
			}
		barrier(CLK_LOCAL_MEM_FENCE);
		}		
	if (lid == 0) 
		{ 
		const int ggd = get_group_id(0);
		errorout[ggd] = temperror[0]; 
		lambdaout[ggd] = templambda[0]; 
		}		
	}	
	
	
/** final reduction, compute the acceleration factor 
 * 
 * param.s0 = error
 * param.s1 = lambda
 * param.s2 = flags
 * param.s3 = target error
 **/
__kernel void reduction_finale( __global FPTYPE * errorin, __global FPTYPE * lambdain, __global FPTYPE_VEC8 * param, __global uint4 * g_gen) 
	{		
	__local FPTYPE temperror[MAXGROUPSIZE];
	__local FPTYPE templambda[MAXGROUPSIZE];
	const int lid   = get_local_id(0);
	const int gid   = get_global_id(0);
	const int gsize = get_local_size(0);
	temperror[lid] = errorin[gid];
	templambda[lid] = lambdain[gid];	
	barrier(CLK_LOCAL_MEM_FENCE);
	for(int i = gsize/2; i>0; i >>= 1) 
		{
		if(lid < i) 
			{ 
			temperror[lid] += temperror[lid + i]; 
			templambda[lid] = fmin(templambda[lid], templambda[lid + i]); 					
			}
		barrier(CLK_LOCAL_MEM_FENCE);
		}				
	if (lid == 0) 
		{
		FPTYPE_VEC8 prev = (*param);
		FPTYPE_VEC8 next = prev;
		next.s0 = sqrt(temperror[0]);		// new error		
		next.s5 = fmin(next.s0, next.s5);	// new minimum error
		next.s1 = next.s0/prev.s0;		// new lambda
		next.s2 -= 1.0;
		if ((next.s0 > next.s3)&&(prev.s2 < 1.0)&&(next.s1 < 1.0)) // flag is set, new lambda smaller than 1 and not reached target
			{ // we accelerate
			if (fabs(next.s1 - prev.s1) < prev.s4) { next.s1 = (next.s1)/(1.0 - next.s1);  } // super-acceleration 
			next.s1 = fmin(next.s1,((FPTYPE)(0.5))*templambda[0]); // new lambda
			next.s2 = (rand(g_gen) & 1) ? 5.0 : 0.0; // do we accelerate			
			}			
		(*param) = next;
		}
	}	

	
/* perform acceleration as requested */
__kernel void accelerate( __global FPTYPE g_radiiTab1[NBVERTICES],  __global FPTYPE g_radiiTab2[NBVERTICES],  __global FPTYPE_VEC8 * param)	
	{
	const int index       = get_global_id(0);		// global index
	const FPTYPE r0 	  = g_radiiTab1[index];		// previous radius
	const FPTYPE r  	  = g_radiiTab2[index];		// new radius
	const FPTYPE_VEC8 par = (*param);				// info wether we accelerate
	const FPTYPE v        = ((par.s2 >= 5.0) ? (r + (par.s1)*(r-r0)) : r);
	g_radiiTab1[index] = v;
	g_radiiTab2[index] = v;	
	}



)CLsource";

		}

	}


/* end of file */