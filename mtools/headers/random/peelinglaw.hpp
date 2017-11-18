/** @file peelinglaw.hpp */
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
#include "../misc/misc.hpp" 
#include "../misc/error.hpp"
#include "../maths/specialFunctions.hpp"
#include "classiclaws.hpp"

#include <cmath>
#include <random>

namespace mtools
	{


	/*******************************************************************************************************************
	* 
	*                                     UI(H)PT : UNIFORM INFINITE (HALF)-PLANAR TRIANGULATION
	*
	********************************************************************************************************************/


	/**
	* Cumulative distribution of the random walk associated with the peeling of the Infinite Uniform
	* Half-plane Triangulation (type II).
	*
	* c.f. Angel (2002) Growth and Percolation on the Uniform Infinite Planar Triangulation, p15.
	*
	* The random variable S takes value in {-1}U{1,2,3,...} with distribution:
	*    - P(S = -1) = 2/3
	*    - P(S = k) = 2*(2k-2)! /((k-1)!*(k+1)!*4^k) pour k = 1,2,3,...
	*
	* The CDF is explicit:
	*
	* P(S <= k) = 0                                   if k < -1
	*           = 2/3                                 if k < 1
	*           = 1 - (k+1)*(2k)!/(3*4^k*(k+1)!^2)    if k >= 1
	*
	* @param   k   The value to query
	*
	* @return  the probability P(S <= k) exact up to double precision.
	**/
	inline double UIHPT_CDF(int64 k)
		{
		if (k < -1) return 0;
		switch (k)
			{
			case -1: return (2.0 / 3.0);
			case 0: return (2.0 / 3.0);
			case 1: return (11.0 / 12.0);
			case 2: return (23.0 / 24.0);
			case 3: return (187.0 / 192.0);
			case 4: return (377.0 / 384.0);
			case 5: return (505.0 / 512.0);
			case 6: return (1013.0 / 1024.0);
			case 7: return (16241.0 / 16384.0);
			case 8: return (97589.0 / 98304.0);
			case 9: return (390785.0 / 393216.0);
			case 10: return (782233.0 / 786432.0);
			case 11: return (6262063.0 / 6291456.0);
			case 12: return (12530909.0 / 12582912.0);
			case 13: return (50145923.0 / 50331648.0);
			case 14: return (33442997.0 / 33554432.0);
			case 15: return (1070510209.0 / 1073741824.0);
			case 16: return (2141590703.0 / 2147483648.0);
			}
		return 1.0 - ((k + 1) / 3.0)*exp(factln(2 * k) - k*log(4.0) - 2 * factln(k + 1));
		}


	/**
	* Sample a random variable according to the law of the walk associated with the peeling process
	* of the Infinite Uniform Half plane Triangulation.
	*
	* @param [in,out]  gen the random number generator
	**/
	template<class random_t> inline int64 UIHPTLaw(random_t & gen)
		{
		return sampleDiscreteRVfromCDF(UIHPT_CDF, gen);
		}


	/**
	* Cumulative distribution of the random variables associated with the peeling of the Infinite
	* Uniform Triangulation (type II).
	*
	* c.f. Angel (2002) Growth and Percolation on the Uniform Infinite Planar Triangulation, p15.
	*
	* Compute the CDF of the markov chain describing the size of the boundary when peeling a
	* uniform infinite planar triangulation.
	*
	* This correspond to applying the doob h-transform with h(x) = Gamma(x + 3/2) / Gamma(x + 1/2)
	* i.e. p_{k,m} = (h(m-k)/h(m))p_k  (where p_k is distributed as UIHPTpeelCDF()).
	*
	* (p_k and p_{k,m} are the same as in Angel (2002)).
	*
	* @param   k   number of vertice to remove (0 <= k <= m or k =-1 for adding one).
	* @param   m   number of vertice on the boundary is m + 2.
	*
	* @return  The value of sum( p_{i,m}, i = -1..k) which is the probability that we remove at most k
	*          vertices from a peeling step when the boundary has m+2 vertices.
	**/
	inline double UIPT_CDF(int64 k, int64 m)
		{
		if (k < -1) return 0;
		if (m == 0) { return 1.0; } // always discover a new vertex when boundary has size 2
		if (k >= m) return 1.0;
		if (k < 1) return (2 * m + 3.0) / (3 * m + 3.0);
		// we have 1 <= k < m
		return (1 - (2.0*(m - k + 0.5)*m / (3.0*(m + 1.0)*(k + 1.0)*(2 * m + 1.0)))*exp(factln(2 * k) + 2.0*factln(m - 1) + factln(2 * m - 2 * k - 1) - 2.0*factln(k) - 2.0*factln(m - k - 1) - factln(2 * m - 1)));
		}


	/* Proxy object acting as a functor for the CDF of  UIPTpeelCDF(k,m) for a given m */
	struct UIPT_CDF_obj
		{
		UIPT_CDF_obj(int64 m) : _m(m) {}
		inline double operator()(int64 k) { return UIPT_CDF(k, _m); }
		private: int64 _m;
		};


	/**
	* Sample a random variable according to increment of the size of the boundary when peeling to
	* UIPT of type II with a boundary of (m+2) vertices i.e. sampled from the CDF UIPTpeelCDF(.,m).
	*
	* @param   m           the size of the boudary is m+2.
	* @param [in,out]  gen the random number generator.
	*
	* @return  The number of vertices removed from the boundary (or -1 if one was added).
	**/
	template<class random_t> inline int64 UIPTLaw(int64 m, random_t & gen)
		{
		UIPT_CDF_obj O(m);
		return sampleDiscreteRVfromCDF(O, gen);
		}


	/**
	* Cumulative distribution of the random variables associated with the peeling of a
	* free Boltzmann Triangulation of type II.
	*
	* c.f. Angel (2003) Growth and Percolation on the Uniform Infinite Planar Triangulation.
	*
	* Compute the CDF of the markov chain describing the size position k of the splitting of an
	* m+2 gon with boundary { x_0, x_1, ..., x_{m+1} } when peeling the edge (x_{m+1}, x_0).
	*
	* q_(m,k) = -1 if a new vertex is discovered.
	* q_(m,k) = i in {1,..m} if the triangle discovered is (x_{m+1}, x_0, x_i)
	*
	* The density is explicit (cf Angel) and the CDF is also explicit (use maple to compute it)
	*
	* @param   m   consider a FBT of the (m+2) gon
	* @param   k   maximum index to reattach to.
	*
	* @return  the probability that the index i the peeled edge (x_{m+1},x_0) reattaches to is
	* 		   such that i <= k (with the  convention that the index for discovering a new vertex is -1)
	* 		   -> In the particular case m = 0 (2-gon), the method returns 1/9 (probab. of creating a 
	* 		   triangle) for k=-1 and returns 1.0 for k >= 0. 
	*
	**/
	inline double freeBoltzmanTriangulation_CDF(const int64 k, const int64 m)
		{
		const double mm = (const double)m;
		const double kk = (const double)k;
		if (k < -1) return 0.0;
		const double q = (2 * mm + 1.0) / (3 * (mm + 3.0));
		if (k == -1) return q;
		if (k >= m) return 1.0;
		if (k == 0) return q;
		// we have 1 <= k < m
		return
			(1.0 / 6.0)*(
			(5 * (mm + 2.0) / (mm + 3.0)) -
				((16 * kk*kk*kk - 24 * kk*kk*mm + 6 * kk*mm*mm + mm*mm*mm - 18 * kk*mm + 9 * mm*mm - 16 * kk + 8 * mm)*(kk + 1.0) / ((mm + 3)*(mm - kk)*(m + 1 - k)))*
				exp((gammln(2 * mm - 2 * kk) + gammln(2 * kk + 1) + 2 * gammln(mm)) - (2 * gammln(mm - kk) + 2 * gammln(kk + 2) + gammln(2 * mm)))
				);
		}


	/* Proxy object acting as a functor for the CDF of  UIPT_FBTpeelCDF(k,m) for a given m */
	struct freeBoltzmanTriangulation_CDF_obj
		{
		freeBoltzmanTriangulation_CDF_obj(int64 m) : _m(m) {}
		inline double operator()(int64 k) { return freeBoltzmanTriangulation_CDF(k, _m); }
		private: int64 _m;
		};


	/**
	* Sample a random variable according to the splitting of the boundary when peeling
	* a Free Boltzmann Triangulation (type II) of the m+2 gon.
	*
	* @param   m           the size of the boundary is m+2 with vertices { x_0, x_1, ..., x_{m+1} }
	* @param [in,out]  gen the random number generator.
	*
	* @return  if (m > 0)
	* 		    -> return -1 if we discover a new vertex.
	* 		       return  1 <= i <= m if the triangle with base (x_{m+1},x_0) reattaches itself to
	* 		       vertex x_i on the boundary.
	*
	* 		   if (m == 0)
	* 		    -> return -1 if we discover a new vertex.
	* 		       return 0 if we stop peeling (ie the two edges should collapse).
	**/      
	template<class random_t> inline int64 freeBoltzmanTriangulationLaw(int64 m, random_t & gen)
		{
		freeBoltzmanTriangulation_CDF_obj O(m);
		int64 v = sampleDiscreteRVfromCDF(O, gen);
		if ((m > 0) && (v > 0) && (Unif_1(gen))) { v = m + 1 - v; } // re-symmetrize to reduce numerical error, even if it is theorically uneeded.
		return v;
		}






	/*******************************************************************************************************************
	*
	*                                     HYPERBOLIC INFINITE PLANAR TRIANGULATIONS
	*
	********************************************************************************************************************/



	/**
	 * Cumulative distribution of the random walk associated with the peeling of an hyperbolic
	 * infinite Half-plane Triangulation (type II).
	 * 
	 * c.f. Angel, Ray (2014) Classification of half planar maps.
	 *      Curien (2014) Planar stochastic hyperbolic infinite triangulations
	 * 
	 * The parameter kappa is that of the 'inside' Boltzman triangulation of the n-gon: each
	 * interior vertice has weight kappa with
	 * 
	 *                                    0 < kappa <= 27/2
	 * 
	 * The case kappa = 2/27 correpsond to the UIPT.
	 * 
	 * We use the parametrization of Angel-Ray (2014) and Curien (2015).
	 * 
	 *                                     0 < theta <= 1/6
	 * 
	 *                                   2/3 <= alpha < 1
	 * 
	 * with              kappa   =   theta*(1-2*theta)^2   =   alpha^2 * (1-alpha)/2
	 * 
	 * ie                               alpha = 1 - 2*theta
	 * 
	 * the case alpha = 2/3  <-> theta = 1/6 correspond to the free Boltzmann triangulation.
	 *
	 * @param	k	 	The value to query.
	 * @param	theta	parameter theta in (0,1/6].
	 *
	 * @return	the probability P(S <= k).
	 **/
	inline double hyperbolicIHPT_CDF(int64 k, double theta)
		{
		if (k < -1) return 0.0;
		const double alpha = 1 - 2 * theta;
		if (k <= 0) { return alpha; }
		return(1.0 - (k + 1)*(1 - alpha)*exp(k*log((1 - alpha) / (2 * alpha)) + factln(2 * k) - 2 * factln(k + 1)));
		}
	

	/* Proxy object acting as a functor for hyperbolicIHPT_CDF	for a given theta */
	struct hyperbolicIHPT_CDF_obj
		{
		hyperbolicIHPT_CDF_obj(double theta) : _theta(theta) {}
		inline double operator()(int64 k) { return hyperbolicIHPT_CDF(k, _theta); }
		private: double _theta;
		};


	/**
	* Sample a random variable according to the law of the walk associated with the peeling process
	* of an hyperbolic infinite Half plane Triangulation.
	*
	* @param	theta	parameter theta in (0,1/6].
	* @param [in,out]  gen the random number generator
	**/
	template<class random_t> inline int64 hyperbolicIHPTLaw(double theta, random_t & gen)
		{
		hyperbolicIHPT_CDF_obj O(theta);
		return sampleDiscreteRVfromCDF(O, gen);
		}



	/**
	* Sample a random variable according to increment of the size of the boundary when peeling an
	* hyperbolic infinite planar triangulation of type II with a boundary of (m+2) vertices.
	* This increment correpsond to those of the random walk with step hyperbolicIHPTLaw() 
	* conditionned to stay non-negative. 
	*
	* The h-transform is not explicit as for the UIPT. But for theta < 1/6, the walk has positive 
	* drift hence the law is pretty close to the non-conditionned law. The method for simulating
	* is just to make a few steps in advance and accept the first step if the walk does not go 
	* below 0.
	* 
	* TODO : MAKE A BETTER APPROXIMATION....
	*
	* @param   m           the size of the boudary is m+2.
	* @param   theta	   hyperbolicity parameter theta in (0,1/6).
	* @param [in,out]  gen the random number generator.
	*
	* @return  The number of vertices removed from the boundary (or -1 if one was added).
	**/
	template<class random_t> inline int64 hyperbolicIPTLaw(int64 m, double theta, random_t & gen)
		{
		hyperbolicIHPT_CDF_obj O(theta);
		const int NBSTEP = 10;
		while (1)
			{
			int64 x0 = sampleDiscreteRVfromCDF(O, gen);
			int64 pos = m - x0;
			for (int i = 0; i < NBSTEP; i++)
				{
				if (pos < 0) { break; }
				pos -= sampleDiscreteRVfromCDF(O, gen);
				}
			if (pos >= 0) { return x0;  }
			}
		}



	/**
	* Cumulative distribution of the peeling of a general Boltzmann triangulation of type II.
	*
	* c.f. Angel (2003) Growth and Percolation on the Uniform Infinite Planar Triangulation.  
	*      Angel, Ray (2014) Classification of half planar maps.
	*      Curien (2014) Planar stochastic hyperbolic infinite triangulations
	*      
	* A kappa-Boltzman triangulation of the n-gon is such that each interior vertice has weight 
	* kappa with
	* 
	*                                    0 < kappa <= 27/2 
	*                                    
	* The case kappa = 2/27 correpsond to the UIPT (free Boltzman triangulations).
	*
	* We use the parametrization of Angel-Ray (2014) and Curien (2015).
	* 
	*                                     0 < theta <= 1/6
	*                                     
	*                                   2/3 <= alpha < 1
	* 
	* with                 kappa   =   theta*(1-2*theta)^2   =   alpha^2 * (1-alpha)/2 
	*                                        
	* ie                                 alpha = 1 - 2*theta
	*                                
	* the case alpha = 2/3  <-> theta = 1/6 correpsond to the free Boltzmann triangulation.  
	*
	* The density and cdf are explicit (use maple for the cdf). 
	* 
	* @param   k		maximum index to reattach to.
	* @param   m		consider a Boltzamnn of the (m+2) gon with parameter theta.
	* @param   theta    parameter of the boltzmann parametrized as above  in (0,1/6].
	*
	* @return  the probability that the index i the peeled edge (x_{m+1},x_0) reattaches to is
	* 		   such that i <= k (with the  convention that the index for discovering a new vertex is -1)
	* 		   -> In the particular case m = 0 (2-gon), the method returns the probab. of creating a
	* 		   triangle for k=-1 and returns 1.0 for k >= 0.
	*
	**/
	inline double generalBoltzmanTriangulation_CDF(const int64 kk, const int64 mm,  const double theta)
		{
		const double k = (double)kk;
		const double m = (double)mm;	
		if (kk < -1) return 0.0;
		const double q = 2 * (2 * m + 1)*theta*(6 * m*theta - m + 12 * theta - 3) / ((6 * m*theta - m + 6 * theta - 2)*(m + 3));
		if (kk == -1) { return q; }
		if (kk >= mm) return 1.0;
		if (kk == 0) { return q; }
		// we have 1 <= k < m		
		const double m2 = m*m;
		const double m3 = m2*m;
		const double k2 = k*k;
		const double theta2 = theta*theta;
		double A = (36*m2*theta2 - 12*m2*theta + 84*m*theta2 + m2 - 44*m*theta + 24*theta2 + 5*m - 24*theta + 6)*k2
		     	   + (-36*m3*theta2 + 12*m3*theta - 84*m2*theta2 - m3 + 44*m2*theta - 24*m*theta2 - 5*m2 + 24*m*theta - 6*m)*k
			       - 24*m3*theta2 + 10*m3*theta - 84*m2*theta2 - m3 + 48*m2*theta - 84*m*theta2 - 6*m2 + 62*theta*m - 24*theta2 - 11*m + 24*theta - 6;
		A *= ((2*k - m)*(k + 1)) / ((m + 1 - k)*(m - k));
		A *= exp(gammln(2*m - 2*k) + gammln(2*k + 1) + 2 * gammln(m) - 2 * gammln(m - k) - 2 * gammln(k + 2) - gammln(2 * m));
		A -= 24*m2*theta2 - 10*m2*theta + 60*m*theta2 + m2 - 38*m*theta + 24*theta2 + 5*m - 24*theta + 6;
		A /= 2*(6*m*theta - m + 6*theta - 2)*(m + 3);
		return(A + q);
		}


	/* Proxy object acting as a functor for the CDF of generalBoltzmanTriangulation_CDF for a given m, theta */
	struct generalBoltzmanTriangulation_CDF_obj
		{
		generalBoltzmanTriangulation_CDF_obj(int64 m, double theta) : _m(m), _theta(theta) {}
		inline double operator()(int64 k) { return generalBoltzmanTriangulation_CDF(k, _m,_theta); }
		private: int64  _m;
				 double _theta;
		};


	/**
	* Sample a random variable according to the splitting of the boundary when peeling
	* a general Boltzmann Triangulation (type II) of the m+2 gon with parameter theta
	*
	* c.f. Angel (2003) Growth and Percolation on the Uniform Infinite Planar Triangulation.
	*      Angel, Ray (2014) Classification of half planar maps.
	*      Curien (2014) Planar stochastic hyperbolic infinite triangulations
	*
	* A kappa-Boltzman triangulation of the n-gon is such that each interior vertice has weight
	* kappa with
	*
	*                                    0 < kappa <= 27/2
	*
	* The case kappa = 2/27 correpsond to the UIPT (free Boltzman triangulations).
	*
	* We use the parametrization of Angel-Ray (2014) and Curien (2015).
	*
	*                                     0 < theta <= 1/6
	*
	*                                   2/3 <= alpha < 1
	*
	* with                 kappa   =   theta*(1-2*theta)^2   =   alpha^2 * (1-alpha)/2
	*
	* ie                                 alpha = 1 - 2*theta
	*
	* the case alpha = 2/3  <-> theta = 1/6 correpsond to the free Boltzmann triangulation.
	*
	* @param   m           the size of the boundary is m+2 with vertices { x_0, x_1, ..., x_{m+1} }
	* @param   theta       parameter of the boltzmann parametrized as above in (0,1/6].
	* @param [in,out]  gen the random number generator.
	*
	* @return  if (m > 0)
	* 		    -> return -1 if we discover a new vertex.
	* 		       return  1 <= i <= m if the triangle with base (x_{m+1},x_0) reattaches itself to
	* 		       vertex x_i on the boundary.
	*
	* 		   if (m == 0)
	* 		    -> return -1 if we discover a new vertex.
	* 		       return 0 if we stop peeling (ie the two edges should collapse).
	**/
	template<class random_t> inline int64 generalBoltzmanTriangulationLaw(int64 m, double theta, random_t & gen)
		{
		generalBoltzmanTriangulation_CDF_obj O(m,theta);
		int64 v = sampleDiscreteRVfromCDF(O, gen);
		if ((m > 0) && (v > 0) && (Unif_1(gen))) { v = m + 1 - v; } // re-symmetrize to reduce numerical error, even if it is theorically uneeded.
		return v;
		}

	}


/* end of file  */


