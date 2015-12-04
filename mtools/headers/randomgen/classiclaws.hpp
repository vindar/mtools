/** @file classiclaws.hpp */
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

#include <cmath>

namespace mtools
{



    /**
    * create a Binomial randon variable.
    * METHOD: inverse distribution function for n*p < 30 and split in several part if n*p is larger
    * see the paper for details:
    *    Kachitvichyanukul, V. and Schmeiser, B. W. (1988).
    *    Binomial random variate generation
    *    Communications of the ACM 31, 216-222.
    *    (Algorithm BTPEC).
    *
    * Parameters: 
    *     - n : number of trials in the binomial distribution.
    *     - p : probability od success of a trial.
    **/
	class BinomialLaw
	{
	public:

        /**
         * Constructor. Set the parameters.
         *
         * @param   n   number of trials.
         * @param   p   Probability of success of a trial.
        **/
		BinomialLaw(uint32 n = 1, double p = 0.5) {setParam(n,p);}


        /**
         * Set the parameter of the binomial.
         *
         * @param   n   number of trials.
         * @param   p   Probability of success of a trial.
        **/
		inline void setParam(uint32 n, double p)
			{
			pp = p;
			sn = n;
			if (pp>0.5) {pp=1.0-pp; inv = true;} else {inv = false;}
			if ((pp>0)&&(sn>0))
				{
				sN = (uint32)((29.0/pp));	// max number of trial such that  N*p < 30
				if (n<sN) {sN = n;}			// we do not need to split
				q = 1-pp;
				r = pp/q;
				sqN = pow(q,(double)sN);
				sg = r*(sN+1);
				}
			}


        /**
         * Return a Binomial(n,p) distributed random variable.
         *
         * @param [in,out]  gen The random generator.
         *
         * @return  the random variable obtained.
        **/
		template<class CRandomGen> inline uint32 operator()(CRandomGen & gen) const
			{
			if (pp == 0) {if (inv) {return(sn);} return 0;} // parameter p = 0 or 1, return 0 or sn
			if (sn < 2) // 0 or 1 trial
				{
				if (sn==0) {return 0;} // 0 trial, return 0;
				if (gen() < pp)
					{
					if (inv) {return 0;} return 1;
					}
				if (inv) {return 1;}
				return 0;
				}
			uint32 x  = 0; 
			uint32 n  = sn; 
			uint32 N  = sN;
			double qN = sqN;
			double g  = sg;
			while(n != 0)
				{
				if (n < N) // less than N remaining trials to do
					{
					N = n;
					qN = pow(q,(double)N);
					g = r*(N+1);
					}
				// invert the cdf
				uint32 ix;
				while(1) 
					{
					ix = 0;
					double f = qN;
					double u = gen(); // uniform on [0,1)
					while(1)
						{
						if (u < f)		goto doneBinom;
						if (ix > 110)	break;
						u -= f;
						ix++;
						f *= (g / ix - r);
						}
					}
				doneBinom:
				x += ix;
				n -= N;
				}
			if (x > sn) {x = sn;}
			if (inv) {return(sn-x);}
			return x;
			}

	private:
		uint32 sn;
		uint32 sN;
		bool inv;
		double pp,q,r,sqN,sg;

	};



    /**
     * Create an exponential distribution (by inverting the CDF). 
     * 
     * Parameter: - lambda \> 0.
     * 
     * The density of X is P(X in dx) = lambda*exp(-lambda*x)dx on [0,infty).
     * 
     * The expectation of X is thus E[X] = 1/lambda.
    **/
    class ExponentialLaw
    {
    public:

        /**
         * Constructor. Set the parameter.
         *
         * @param   lambda  Parameter of the exponential law (inverse of its expectation).
        **/
        ExponentialLaw(double lambda = 1.0) : l(lambda) { MTOOLS_ASSERT(lambda > 0.0); }


        /**
        * Set the parameter.
        *
        * @param   lambda  Paramter of the exponential law (inverse of its expectation).
        **/
        inline void setParam(double lambda = 1.0) {l = lambda; MTOOLS_ASSERT(lambda > 0.0); }


        /**
         * Randoms the given generate.
         *
         * @param [in,out]  gen The random generator
         *
         * @return  the random variable.
        **/
		template<class random_t> inline double operator()(random_t & gen) const { return(-log(1-gen())/l); }


    private:
        double l;
    };



    /**
    * Create a geometric random variable.
    *
    * Parameter: - alpha \> 0.
    *
    * The law is given by P( X = k) = alpha*(1-alpha)^(k-1) pour k = 1,2...
    *
    * The expectation of X is thus E[X] = 1/alpha
    **/
    class GeometricLaw
        {
        public:

            /**
            * Constructor. Set the parameter.
            *
            * @param   alpha  Parameter of the geometric (probability of success).
            **/
            GeometricLaw(double alpha) : a(alpha) { MTOOLS_ASSERT((alpha > 0.0)&(alpha<1.0)); l = -log(1 - alpha); }


            /**
            * Set the parameter.
            *
            * @param   alpha  Parameter of the geometric (probability of success).
            **/
            inline void setParam(double alpha) { MTOOLS_ASSERT((alpha > 0.0)&(alpha < 1.0)); l = -log(1 - alpha); a = alpha; }


            /**
            * Get a random number.
            *
            * @param [in,out]  gen The random generator
            *
            * @return  the random variable.
            **/
            template<class random_t> inline int64 operator()(random_t & gen) const 
                { 
                if (a >= 0.6) { uint64 r = 1; while (gen() >= a) { r++; } return r; }
                return 1+(int64)floor(-log(1 - gen()) / l); 
                }


        private:
            double a,l;
        };



    /**
     * Create an normal distributed random number X using the Box-Muller algorithm.
     *
     * Parameter: m in R and sigma2 > 0.
     * 
     * The density of X is P(X in dx) = 1/sqrt(2*pi*sigma2)*exp(-(x-m)^2 /(2*sigma2))dx.
     * 
     * The expectation of X is thus E[X] = m and variance = sigma2.
     **/
    class NormalLaw
    {
    public:

        /**
         * Constructor. Set the parameters.
         *
         * @param   m       mean of the r.v.
         * @param   sigma2  variance of the r.v.
        **/
        NormalLaw(double m = 0.0, double sigma2 = 1.0) : c(m) { MTOOLS_ASSERT(sigma2 > 0.0);  s = sqrt(sigma2); }


        /**
        * Constructor. Set the parameters.
        *
        * @param   m       mean of the r.v.
        * @param   sigma2  variance of the r.v.
        **/
        inline void setParam(double m = 0.0,double sigma2 = 1.0) {c = m; MTOOLS_ASSERT(sigma2 > 0.0);  s = sqrt(sigma2); }


        /**
         * Generate of normal r.v.
         *
         * @param [in,out]  gen the random number generator.
         *
         * @return  A double.
        **/
		template<class random_t> inline double operator()(random_t & gen) const { return( s*sqrt(-2*log(1-gen()))*sin(TWOPI*(1-gen())) + c); }


    private:
        double c,s;
    };


    /**
    * create an stable random variable. use the Chambers-Mallows-Stuck method (generalized 
    * Box-Muller algorithm).
    * cf: http://math.u-bourgogne.fr/monge/bibliotheque/ebooks/csa/htmlbook/node235.html
    *
    * @warning Works for alpha != 1, for alpha = 1, see CauchyLaw.
    *
    * Parameters: 
    *   - alpha in (0,2] - {1}      autosimilarity parameter (for alpha=1 c.f. CauchyLaw) 
    *   - beta  in [-1,1]           symetry parameter
    *   - C      \>  0                scaling paramter
    *   - m     in R                centering paramter
    *
    * The characteristic function is given by:
    *
    * f(t) = ln E[exp(itX)] = - C^alpha |t|^alpha ( 1 - i.beta.sign(t).tan(Pi.alpha/2) ) + i.m.t
    *
    **/
    class StableLaw
    {
    public:
        /**
         * Constructor. Set the parameters
         *
         * @param   alpha   autosimilarity parameter in [0,2[ and !=1 (for alpha=1 use CauchyLaw and for alpha = 2 use NormalLaw)
         * @param   beta    symetry parameter
         * @param   C       scaling paramter.
         * @param   m       centering paramter.
        **/
        StableLaw(double alpha,double beta,double C,double m) : p_alpha(alpha), p_beta(beta), p_C(C), p_m(m) {_createval();}

        
        /**
        * Set the parameters
        *
        * @param   alpha   autosimilarity parameter !=1 (for alpha=1 use CauchyLaw)
        * @param   beta    symetry parameter
        * @param   C       scaling paramter.
        * @param   m       centering paramter.
        **/
        inline void setParam(double alpha,double beta,double C,double m) {p_alpha = alpha; p_beta = beta; p_C = C; p_m = m; _createval();}



        /**
         * Return a stable random variable.
         *
         * @param [in,out]  gen the random number generator.
         *
         * @return  the random variable.
        **/
		template<class random_t> inline double operator()(random_t & gen) const
            {
			double U1 = gen();
			double U2 = gen();
            double U = PI*(U1-0.5); // uniform on [-pi/2,pi/2]
            double W = -log(1-U2);  // exp of parameter 1
            double X = S*(sin(p_alpha*(U+xi))/pow(cos(U),ialpha))*pow(cos(U - p_alpha*(U+xi))/W,talpha); // normalised
            return(p_C*X + p_m);
            }


    private:

        inline void _createval()
            {
            MTOOLS_ASSERT((p_alpha > 0.0) && (p_alpha < 2.0) && (p_alpha != 1.0));
            MTOOLS_ASSERT((p_beta > -1.0)&& (p_beta < 1.0));
            MTOOLS_ASSERT(p_C > 0.0);
            double zeta = -p_beta*tan(PI*p_alpha/2);
            S = pow(1+(zeta*zeta),1/(2*p_alpha));
            xi = (1/p_alpha)*atan(-zeta);
            ialpha = 1/p_alpha;
            talpha = (1-p_alpha)/p_alpha;
            }

        double p_alpha,p_beta,p_C,p_m;
        double S,xi,ialpha,talpha;
    };



    /**
    * create Cauchy random variables. Use the Chambers-Mallows-Stuck method (generalized Box-Muller 
    * algorithm). cf: http://math.u-bourgogne.fr/monge/bibliotheque/ebooks/csa/htmlbook/node235.html
    * 
    * Parameters: 
    *     - beta  in [-1,1]           symetry parameter
    *     - C      \>  0                scaling paramter
    *     - m     in R                centering paramter
    * 
    * Characteristic function: f(t) = ln E[exp(itX)] = - C.|t|.( 1 + i.beta.sign(t).(2/Pi).ln|t| ) + i.m.t
    **/
    class CauchyLaw
    {
    public:

        /**
         * Constructor. Set the parameters
         *
         * @param   beta    symetry parameter.
         * @param   C       scaling paramter.
         * @param   m       centering paramter.
        **/
        CauchyLaw(double beta,double C,double m) : p_beta(beta), p_C(C), p_m(m) {_createval();}


        /**
        * Set the parameters
        *
        * @param   beta    symetry parameter.
        * @param   C       scaling paramter.
        * @param   m       centering paramter.
        **/
        inline void setParam(double beta,double C,double m) {p_beta = beta; p_C = C; p_m = m; _createval();}


        /**
         * Generate a Cauchy random variable.
         *
         * @param [in,out]  gen The rnadom number generator.
         *
         * @return  the random variable
        **/
		template<class random_t> inline double operator()(random_t & gen) const
            {
			double U1 = gen();
			double U2 = gen();
            double U = PI*(U1-0.5); // uniform on [-pi/2,pi/2]
            double W = -log(1-U2);  // exp of parameter 1
            double X = (2/PI)*((PI/2 + p_beta*U)*tan(U) - p_beta*log(((PI/2)*W*cos(U))/(PI/2 + p_beta*U)) ); // normalized
            return(p_C*X + mm);
            }




    private:

        inline void _createval()
            {
            MTOOLS_ASSERT((p_beta >= -1.0) && (p_beta <= 1.0) &&(p_C> 0.0));
            mm = (2/PI)*p_beta*p_C*log(p_C) + p_m; // compute the value of the associated constant
            }

        double p_beta,p_C,p_m,mm;
    };



    /**
    * Sample a discrete random variable X taking value in [0,N] from its CDF distribution.
    *
    * @tparam  random_t    Random number generator (such the operator() return a uniform rv in [0,1[).
    * @param   tab The CDF array such that tab[i] = P(X &lt;= i), it size should be at least N.
    *              (there is no need to define tab[N] = 1.0).
    * @param   N   the support of the RV X is [0,N] ir tab is at least N elements long.
    *
    * @return  A random position in [0,N] chosing according to the CDF.
    **/
    template<class random_t> inline int64 sampleDiscreteRVfromCDF(const double * tab, size_t N, random_t & gen)
        {
        double a = gen(); // get a random value in [0,1[
        if (a < tab[0]) { return 0; }       // extreme value cases
        if (a >= tab[N - 1]) { return N; }  //
        size_t n1 = 0;	    // lower bound, we know that tab[n1] <= a
        size_t n2 = N - 1;	// uppper bound, we know that tab[n2] > a
        while ((n2 - n1)>1)
            { // loop until we reduced the interval to size 1
            const size_t g = (n1 + n2) / 2;
            if (a >= tab[g]) { n1 = g; }
            else { n2 = g; }
            }
        return n2; // we now have tab[n1 = n2-1] <= a < tab[n2].
        }


}


/* end of file  */


