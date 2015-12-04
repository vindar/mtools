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
#include <random>

namespace mtools
{

   
    /**
    * Construct a uniform unsigned integer value in the range [0,2^64-1] .
    * This means that every bit is iid bernoulli 1/2.
    *
    * @param [in,out]  gen The random number generator
    *
    * @return  The uniform random unsinged integer over the entire range.
    **/
    template<class random_t> inline uint64 Unif_64(random_t & gen)
        {
        if ((random_t::min() == 0)&&(random_t::max() == 4294967295)) return ((uint64)gen() + (((uint64)gen()) << 32));
        if ((random_t::min() == 0)&&(random_t::max() == 18446744073709551615)) return gen();
        MTOOLS_ERROR("Unif_64 need the random engine to have packed 32 or 64 bits. Use std::independent_bits_engine to construct a packed engine from an unpacked one.");
        }


    /**
     * Construct a real-valued uniform number in [0,1[.
     *
     * @param [in,out]  gen The random number generator
     *
     * @return  A double uniformely distributed in [0,1[. 
     **/
    template<class random_t> inline double Unif(random_t & gen) { return ((Unif_64<random_t>(gen) >> 11)) * (1.0 / 9007199254740992.0); }


    /**
    * Construct a real-valued uniform number in the CLOSED interval [0,1].
    *
    * @param [in,out]  gen The random number generator
    *
    * @return  A double uniformely distributed in [0,1].
    **/
    template<class random_t> inline double Unif_01closed(random_t & gen) { return ((Unif_64<random_t>(gen) >> 11)) * (1.0 / 9007199254740991.0); }


    /**
    * Construct a real-valued uniform number in the OPEN interval ]0,1[.
    *
    * @param [in,out]  gen The random number generator
    *
    * @return  A double uniformely distributed in ]0,1[.
    **/
    template<class random_t> inline double Unif_01open(random_t & gen) { return (((Unif_64<random_t>(gen)) >> 12) + 0.5) * (1.0 / 4503599627370496.0); }


    /**
    * Construct a real-valued uniform number in [a,b[.
    *
    * @param   a           lower bound (included).
    * @param   b           upper bound (excluded).
    * @param [in,out]  gen The random number generator
    *
    * @return  A double uniformely distributed in [a,b[.
    **/
    template<class random_t> inline double Unif(double a, double b, random_t & gen) { return ((Unif<random_t>(gen))*(b - a) + a); }


    /**
     * Construct a uniform integer valued random variable in the range [A,B].
     *
     * @param   A           The minimal value
     * @param   B           The maximal value
     * @param [in,out]  gen The random number generator
     *
     * @return  A 64bit integer uniformely distributed in [A,B]
     **/
    template<class random_t> inline int64 Unif_int(int64 A, int64 B, random_t & gen) { return((int64)((Unif<random_t>(gen))*(B - A + 1)) + A); }


    /**
     * generate a high precision random number on [0,1) interval with high precision in the
     * neighbourood of zero. This means that when the returned value is very small (close to zero)
     * the number of significative digit stays roughly the same whereas for normal Unif() the
     * minimal step for each value is 1.0/4503599627370496.0 even for small value. useful for
     * simulationg unbounded RV via their CDF
     * 
     * In average, the generation is 1/256 slower than the classic rand_double0()
     *
     * @param [in,out]  gen The random number generator.
     *
     * @return  the random number.
     **/
    template<class random_t> inline double Unif_highprecision(random_t & gen) 
        {
        double b = 1.0, a = Unif(gen);
        while (a*256 < 1.0) { b /= 256; a = rand_double0(); }
        return a*b;
        }


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
        double a = Unif(gen); // get a random value in [0,1[
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
		template<class random_t> inline uint32 operator()(random_t & gen) const
			{
			if (pp == 0) {if (inv) {return(sn);} return 0;} // parameter p = 0 or 1, return 0 or sn
			if (sn < 2) // 0 or 1 trial
				{
				if (sn==0) {return 0;} // 0 trial, return 0;
				if (Unif(gen) < pp)
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
					double u = Unif(gen); // uniform on [0,1)
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
		template<class random_t> inline double operator()(random_t & gen) const { return(-log(1- Unif(gen))/l); }


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
                if (a >= 0.6) { uint64 r = 1; while (Unif(gen) >= a) { r++; } return r; }
                return 1+(int64)floor(-log(1 - Unif(gen)) / l);
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
		template<class random_t> inline double operator()(random_t & gen) const { return( s*sqrt(-2*log(1- Unif(gen)))*sin(TWOPI*(1- Unif(gen))) + c); }


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
			double U1 = Unif(gen);
			double U2 = Unif(gen);
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
			double U1 = Unif(gen);
			double U2 = Unif(gen);
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





}


/* end of file  */


