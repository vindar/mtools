/** @file peelingUIPT.hpp */
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


#include "../misc/misc.hpp" 
#include "../misc/error.hpp"
#include "../maths/specialFunctions.hpp"
#include "classiclaws.hpp"

#include <cmath>
#include <random>

namespace mtools
	{


	/**
	* Cumulative distribution of the random variable associated with the peeling of the Infinite Uniform
	* Half-plane Triangulation.
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
	inline double UIHPTpeelCDF(int64 k)
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
	* of the Infinite Uniform Half plane Triangulation (cf UIHPTpeelCDF()).
	*
	* @param [in,out]  gen the random number generator
	**/
	template<class random_t> inline int64 UIHPTpeelLaw(random_t & gen)
		{
		return sampleDiscreteRVfromCDF(UIHPTpeelCDF, gen);
		}


	/**
	* Cumulative distribution of the random variables associated with the peeling of the Infinite
	* Uniform Triangulation.
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
	* @param   k   number of vertice to remove (k &lt;= m and k =-1 for adding one).
	* @param   m   number of vertice on the boundary is m + 2.
	*
	* @return  The value of sum( p_{i,m}, i = -1..k) which is the probability that we remove at most k
	*          vertices from a peeling step when the boundary has m+2 vertices.
	**/
	inline double UIPTpeelCDF(int64 k, int64 m)
		{
		if (k < -1) return 0;
		if (k >= m) return 1.0;
		if (k < 1) return (2 * m + 3.0) / (3 * m + 3.0);
		// we have 1 <= k < m
		return (1 - (2.0*(m - k + 0.5)*m / (3.0*(m + 1.0)*(k + 1.0)*(2 * m + 1.0)))*exp(factln(2 * k) + 2.0*factln(m - 1) + factln(2 * m - 2 * k - 1) - 2.0*factln(k) - 2.0*factln(m - k - 1) - factln(2 * m - 1)));
		}


	/* Proxy object acting as a functor for the CDF of  UIPTpeelCDF(k,m) for a given m */
	struct UIPTpeelCDFobj
		{
		UIPTpeelCDFobj(int64 m) : _m(m) {}
		inline double operator()(int64 k) { return UIPTpeelCDF(k, _m); }
		private: int64 _m;
		};


	/**
	* Sample a random variable according to increment of the size of the boundary when peeling to
	* UIPT. with a boundary of (m+2) vertices ie sampled from the CDF UIPTpeelCDF(.,m).
	*
	* @param   m           the size of the boudary is m+2.
	* @param [in,out]  gen the random number generator.
	*
	* @return  The number of vertices removed from the boundary (or -1 if one was added).
	**/
	template<class random_t> inline int64 UIPTpeelLaw(int64 m, random_t & gen)
		{
		UIPTpeelCDFobj O(m);
		return sampleDiscreteRVfromCDF(O, gen);
		}


	/**
	* Cumulative distribution of the random variables associated with the peeling of a
	* Free Boltzmann Triangulation (FBT).
	*
	* c.f. Angel (2002) Growth and Percolation on the Uniform Infinite Planar Triangulation, p15.
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
	*
	**/
	inline double UIPT_FBTpeelCDF(const int64 k, const int64 m)
		{
		const double mm = (const double)m;
		const double kk = (const double)k;
		if (k < -1) return 0.0;
		if (k >= m) return 1.0;
		if (k < 1) return (2 * mm + 1.0) / (3 * (mm + 3.0));
		// we have 1 <= k < m
		return
			(1.0 / 6.0)*(
			(5 * (mm + 2.0) / (mm + 3.0)) -
				((16 * kk*kk*kk - 24 * kk*kk*mm + 6 * kk*mm*mm + mm*mm*mm - 18 * kk*mm + 9 * mm*mm - 16 * kk + 8 * mm)*(kk + 1.0) / ((mm + 3)*(mm - kk)*(m + 1 - k)))*
				exp((gammln(2 * mm - 2 * kk) + gammln(2 * kk + 1) + 2 * gammln(mm)) - (2 * gammln(mm - kk) + 2 * gammln(kk + 2) + gammln(2 * mm)))
				);
		}


	/* Proxy object acting as a functor for the CDF of  UIPT_FBTpeelCDF(k,m) for a given m */
	struct UIPT_FBTpeelCDFobj
		{
		UIPT_FBTpeelCDFobj(int64 m) : _m(m) {}
		inline double operator()(int64 k) { return UIPT_FBTpeelCDF(k, _m); }
		private: int64 _m;
		};


	/**
	* Sample a random variable according to the splitting of the boundary when peeling
	* a Free Boltzmann Triangulation (type II) of the m+2 gon.
	*
	* @param   m           the size of the boundary is m+2 with vertices { x_0, x_1, ..., x_{m+1} }
	* @param [in,out]  gen the random number generator.
	*
	* @return  index 1 <= i <= m of the vertex we reattaches to when peeling (x_{m+1},x_0)
	* 		   or -1 f we discover a new vertice.
	**/
	template<class random_t> inline int64 UIPT_FBTpeelLaw(int64 m, random_t & gen)
		{
		UIPT_FBTpeelCDFobj O(m);
		return sampleDiscreteRVfromCDF(O, gen);
		}


	}


/* end of file  */


