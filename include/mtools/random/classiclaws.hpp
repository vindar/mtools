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

#include "../misc/internal/mtools_export.hpp"
#include "../misc/misc.hpp" 
#include "../misc/error.hpp"
#include "../maths/specialFunctions.hpp"

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
        if ((random_t::min() == 0)&&(random_t::max() == 4294967295UL)) return ((uint64)gen() + (((uint64)gen()) << 32));
        if ((random_t::min() == 0)&&(random_t::max() == 18446744073709551615ULL)) return gen();
        MTOOLS_ERROR("Unif_64 need the random engine to have packed 32 or 64 bits. Use std::independent_bits_engine to construct a packed engine from an unpacked one.");
        return 0;
        }


    /**
    * Construct a uniform unsigned integer value in the range [0,2^32-1] .
    * This means that every bit is iid bernoulli 1/2.
    *
    * @param [in,out]  gen The random number generator
    *
    * @return  The uniform random unsinged integer over the entire range.
    **/
    template<class random_t> inline uint32 Unif_32(random_t & gen)
        {
        if ((random_t::min() == 0) && (random_t::max() == 4294967295UL)) return ((uint32)gen());
        if ((random_t::min() == 0) && (random_t::max() == 18446744073709551615ULL)) return ((uint32)(gen() & 4294967295));
        MTOOLS_ERROR("Unif_32 need the random engine to have packed 32 or 64 bits. Use std::independent_bits_engine to construct a packed engine from an unpacked one.");
        return 0;
        }


    /* uniform in the range [0,2^16 -1] i.e. the lowest 16 bits are iid bernoulli 1/2 */
    template<class random_t> inline uint32 Unif_16(random_t & gen) { return (Unif_32(gen) & 65535); }

    /* uniform in the range [0,2^8 -1] i.e. the lowest 8 bits are iid bernoulli 1/2 */
    template<class random_t> inline uint32 Unif_8(random_t & gen) { return (Unif_32(gen) & 255); }

    /* uniform in the range [0,2^4 -1] i.e. the lowest 4 bits are iid bernoulli 1/2 */
    template<class random_t> inline uint32 Unif_4(random_t & gen) { return (Unif_32(gen) & 15); }

    /* uniform in the range [0,2^3 -1] i.e. the lowest 3 bits are iid bernoulli 1/2 */
    template<class random_t> inline uint32 Unif_3(random_t & gen) { return (Unif_32(gen) & 7); }

    /* uniform in the range [0,2^3 -1] i.e. the lowest 2 bits are iid bernoulli 1/2 */
    template<class random_t> inline uint32 Unif_2(random_t & gen) { return (Unif_32(gen) & 3); }

    /* uniform in the range [0,2^3 -1] i.e. the lowest bit is a bernoulli 1/2 */
    template<class random_t> inline uint32 Unif_1(random_t & gen) { return (Unif_32(gen) & 1); }


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
     * In average, the generation is 1/256 slower than the classic operator()
     *
     * @param [in,out]  gen The random number generator.
     *
     * @return  the random number.
     **/
    template<class random_t> inline double Unif_highprecision(random_t & gen) 
        {
        double b = 1.0;
        double a = Unif(gen);
        while (a*256 < 1.0) { b /= 256; a = Unif(gen); }
        return a*b;
        }


    /**
     * Return a random variable uniformely distributed in [0,1]^D.
     *
     * @tparam  D       dimension of the space.
     * @param [in,out]  gen The random number generator.     
     *
     * @returns the random point.
     **/
    template<int D, typename random_t> fVec<D> Unif_dimD(random_t & gen)
        {
        fVec<D> P;
        for(int i = 0; i < D; i++) { P[i] = Unif_highprecision(gen); }
        return P;
        }


    /**
     * Return a random variable uniformely distributed in a given box of R^D.
     *
     * @tparam  D               dimension of the space.
     * @param   box             the box
     * @param [in,out]  gen     The random number generator.
     *
     * @returns the random point.
     */
    template<int D, typename random_t> fVec<D> Unif_dimD(const fBox<D>& box, random_t & gen)
        {
        fVec<D> P;
        for (int i = 0; i < D; i++) { P[i] = (Unif_highprecision(gen) * (box.max[i] - box.min[i])) + box.min[i]; }
        return P;
        }



    /**
    * Sample a discrete random variable X taking value in [0,N] from its CDF distribution.
    *
    * @tparam  random_t    Random number generator (such the operator() return a uniform rv in [0,1[).
    * @param   tab The CDF array such that tab[i] = P(X &lt;= i), it size should be at least N.
    *              (there is no need to define tab[N] = 1.0).
    * @param   N   the support of the RV X is [0,N] ie tab is at least N elements long.
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
    * Sample a discrete random variable X taking value in Z from its cumulatice distribution
    * function.
    *
    * @param   cdf         CDF functor such that cdf(i) = P(S <= i) for any int64
    * @param [in,out]  gen the random number generator.
    *
    * @return  a value in [-4611686018427387904, 4611686018427387904] (truncate the rv if out of these bounds).
    **/
    template<class random_t, class CDF> inline int64 sampleDiscreteRVfromCDF(CDF cdf, random_t & gen)
        {
        double a = Unif(gen); // uniform value in [0,1[
        int64 i, j;
        if (cdf(0) <= a)
            { // value is strictly positive
            if (cdf(1) > a) return 1;
            j = 2;
            while (cdf(j) <= a)
                {
                if (j >= 4611686018427387904) return 4611686018427387904;   // out of bounds
                j *= 2;
                }
            i = j / 2;
            }
        else
            { // value is negative or zero
            if (cdf(-1) <= a) return 0;
            i = -2;
            while (cdf(i) > a)
                {
                if (i <= -4611686018427387904) return -4611686018427387904;   // out of bounds
                i *= 2;
                }
            j = i / 2;
            }
        // cdf(i) <= a < cdf(j)
        while ((j - i)>1)
            { //  dichotomy until j = i+1
            int64 g = (i + j) / 2;
            if (a >= cdf(g)) { i = g; }
            else { j = g; }
            }
        return j;
        }



    /**
    * create a Binomial randon variable.
    * Taken from numerical recipes 
    * 
    **/
    class BinomialLaw
        {


        public:

        /**
         * Constructor. Set the parameters.
         *
         * @param   nn  number of trials.
         * @param   ppp Probability of success of a trial.
         **/
        BinomialLaw(int nn, double ppp) { setParam(nn, ppp); }


        /**
         * Set the parameter of the binomial.
         *
         * @param   nn  number of trials.
         * @param   ppp Probability of success of a trial.
         **/
        void setParam(int nn, double ppp)
            {
            pp = ppp;  
            n = nn;
            int j;
            pb = p = (pp <= 0.5 ? pp : 1.0 - pp);
            if (n <= 64) 
                {
                uz = 0; uo = 0xffffffffffffffffLL; rltp = 0;
                for (j = 0;j<5;j++) pbits[j] = 1 & ((int)(pb *= 2.));
                pb -= floor(pb); swch = 0;
                }
            else if (n*p < 30.) 
                {
                cdf[0] = exp(n*log(1 - p));
                for (j = 1;j<64;j++) cdf[j] = cdf[j - 1] + exp(gammln(n + 1.) - gammln(j + 1.) - gammln(n - j + 1.) + j*log(p) + (n - j)*log(1. - p));
                swch = 1;
                }
            else 
                {
                np = n*p; glnp = gammln(n + 1.); plog = log(p); pclog = log(1. - p); sq = sqrt(np*(1. - p));
                if (n < 1024) for (j = 0;j <= n;j++) logfact[j] = gammln(j + 1.);
                swch = 2;
                }
            }


            /**
            * Return a Binomial(n,p) distributed random variable.
            *
            * @param [in,out]  gen The random generator.
            *
            * @return  the random variable obtained.
            **/
            template<class random_t> int operator()(random_t & gen)
            {
            int j, k, kl, km;
            double y, u, v, u2, v2, b;
            if (swch == 0) 
                {
                unfin = uo;
                for (j = 0;j<5;j++) 
                    { 
                    diff = unfin & ( Unif_64(gen) ^ (pbits[j] ? uo : uz));
                    if (pbits[j]) rltp |= diff; else rltp = rltp & ~diff; unfin = unfin & ~diff;
                    }
                k = 0;
                for (j = 0;j<n;j++) 
                    {
                    if (unfin & 1) { if (Unif(gen) < pb) ++k; } else { if (rltp & 1) ++k; }
                    unfin >>= 1; rltp >>= 1;
                    }
                }
            else if (swch == 1) 
                {
                y = Unif(gen); kl = -1; k = 64;
                while (k - kl>1) 
                    {
                    km = (kl + k) / 2;
                    if (y < cdf[km]) k = km; else kl = km;
                    }
                }
            else {
                for (;;) 
                    {
                    u = 0.645*Unif(gen);
                    v = -0.63 + 1.25*Unif(gen);
                    v2 = v*v;
                    if (v >= 0.) { if (v2 > 6.5*u*(0.645 - u)*(u + 0.2)) continue; } else { if (v2 > 8.4*u*(0.645 - u)*(u + 0.1)) continue; }
                    k = int(floor(sq*(v / u) + np + 0.5));
                    if ((k < 0) || (k > n)) continue;
                    u2 = u*u;
                    if (v >= 0.) { if (v2 < 12.25*u2*(0.615 - u)*(0.92 - u)) break; } else { if (v2 < 7.84*u2*(0.615 - u)*(1.2 - u)) break; }
                    b = sq*exp(glnp + k*plog + (n - k)*pclog - (n < 1024 ? logfact[k] + logfact[n - k] : gammln(k + 1.) + gammln(n - k + 1.)));
                    if (u2 < b) break;
                    }
                }
            if (p != pp) k = n - k;
            return k;
            }


        private:

            double pp, p, pb, np, glnp, plog, pclog, sq;
            int n, swch;
            uint64 uz, uo, unfin, diff, rltp;
            int pbits[5];
            double cdf[64];
            double logfact[1024];

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
        void setParam(double lambda = 1.0) {l = lambda; MTOOLS_ASSERT(lambda > 0.0); }


        /**
         * Randoms the given generate.
         *
         * @param [in,out]  gen The random generator
         *
         * @return  the random variable.
        **/
		template<class random_t> double operator()(random_t & gen) const { return(-log(1- Unif(gen))/l); }


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
            void setParam(double alpha) { MTOOLS_ASSERT((alpha > 0.0)&(alpha < 1.0)); l = -log(1 - alpha); a = alpha; }


            /**
            * Get a random number.
            *
            * @param [in,out]  gen The random generator
            *
            * @return  the random variable.
            **/
            template<class random_t> int64 operator()(random_t & gen) const 
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
     * 
     * Use numerical recipes rejection method instead of the classic Box-Muller algorithm.
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
        NormalLaw(double m = 0.0, double sigma2 = 1.0) { setParam(m, sigma2); }


        /**
        * Set the parameters.
        *
        * @param   m       mean of the r.v.
        * @param   sigma2  variance of the r.v.
        **/
        inline void setParam(double m = 0.0, double sigma2 = 1.0) { MTOOLS_ASSERT(sigma2 > 0.0); mu = m; sig = sqrt(sigma2); }


        /**
         * Generate of normal r.v.
         *
         * @param [in,out]  gen the random number generator.
         *
         * @return  A double.
        **/
		template<class random_t> inline double operator()(random_t & gen) const 
            {
            double u, v, x, y, q;
            do {
                u = Unif(gen); v = 1.7156*(Unif(gen) - 0.5);
                x = u - 0.449871; y = std::abs(v) + 0.386595;
                q = (x*x) + y*(0.19600*y - 0.25472*x);
                }
            while ((q > 0.27597) && (q > 0.27846 || (v*v) > -4.*log(u)*(u*u)));
            return mu + sig*v/u;
            }


    private:
        double mu, sig;
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
        StableLaw(double alpha,double beta,double C,double m) { setParam(alpha, beta, C, m); }

        
        /**
        * Set the parameters
        *
        * @param   alpha   autosimilarity parameter !=1 (for alpha=1 use CauchyLaw)
        * @param   beta    symetry parameter
        * @param   C       scaling paramter.
        * @param   m       centering paramter.
        **/
        void setParam(double alpha,double beta,double C,double m) 
            {
            p_alpha = alpha; p_beta = beta; p_C = C; p_m = m;
            MTOOLS_ASSERT((p_alpha > 0.0) && (p_alpha < 2.0) && (p_alpha != 1.0));
            MTOOLS_ASSERT((p_beta > -1.0) && (p_beta < 1.0));
            MTOOLS_ASSERT(p_C > 0.0);
            double zeta = -p_beta*tan(PI*p_alpha / 2);
            S = pow(1 + (zeta*zeta), 1 / (2 * p_alpha));
            xi = (1 / p_alpha)*atan(-zeta);
            ialpha = 1 / p_alpha;
            talpha = (1 - p_alpha) / p_alpha;
            }


        /**
         * Return a stable random variable.
         *
         * @param [in,out]  gen the random number generator.
         *
         * @return  the random variable.
        **/
		template<class random_t> double operator()(random_t & gen) const
            {
			double U1 = Unif(gen);
			double U2 = Unif(gen);
            double U = PI*(U1-0.5); // uniform on [-pi/2,pi/2]
            double W = -log(1-U2);  // exp of parameter 1
            double X = S*(sin(p_alpha*(U+xi))/pow(cos(U),ialpha))*pow(cos(U - p_alpha*(U+xi))/W,talpha); // normalised
            return(p_C*X + p_m);
            }


    private:

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
    * 
    * TODO : Replace the code by that of numerical recipe which is faster.
    **/
    class CauchyLaw
    {
    public:

        /**
         * Constructor. Set the parameters
         *
         * @param   beta    symetry parameter.
         * @param   C       scaling parameter.
         * @param   m       centering parameter.
        **/
        CauchyLaw(double beta, double C, double m)  { setParam(beta, C, m);  }


        /**
        * Set the parameters
        *
        * @param   beta    symetry parameter.
        * @param   C       scaling parameter.
        * @param   m       centering parameter.
        **/
        void setParam(double beta,double C,double m) 
            {
            p_beta = beta; p_C = C; p_m = m;
            MTOOLS_ASSERT((p_beta >= -1.0) && (p_beta <= 1.0) && (p_C> 0.0));
            mm = (2 / PI)*p_beta*p_C*log(p_C) + p_m; // compute the value of the associated constant
            }


        /**
         * Generate a Cauchy random variable.
         *
         * @param [in,out]  gen The random number generator.
         *
         * @return  the random variable
        **/
		template<class random_t> double operator()(random_t & gen) const
            {
			double U1 = Unif(gen);
			double U2 = Unif(gen);
            double U = PI*(U1-0.5); // uniform on [-pi/2,pi/2]
            double W = -log(1-U2);  // exp of parameter 1
            double X = (2/PI)*((PI/2 + p_beta*U)*tan(U) - p_beta*log(((PI/2)*W*cos(U))/(PI/2 + p_beta*U)) ); // normalized
            return(p_C*X + mm);
            }


    private:

        double p_beta,p_C,p_m,mm;
    };




    /**
    * create a Gamma random variable.
    * The density of the GAMMA(alpha,Beta) on [0,infty) is given by
    * 
    * f(x) = beta^alpha * x^(alpha-1) * exp(-beta*x) / Gamma(alpha)
    * 
    * paramterization (alpha,beta) is the same as the parametrization (k, theta) with the relation alpha = k and theta = 1/beta.
    * 
    * Taken from numerical recipes
    **/
    class GammaLaw
        {

        public:

            /**
             * Constructor Set the parameters.
             *
             * @param   aalph   parameter alpha.
             * @param   bbet    parameter beta.
             **/
            GammaLaw(double aalph, double bbet) : normale() { setParam(aalph, bbet);  }


            /**
            * Set the parameters.
            *
            * @param   aalph   parameter alpha.
            * @param   bbet    parameter beta.
            **/
            void setParam(double aalph, double bbet)
                {
                alph = aalph;  oalph = aalph;  bet = bbet;
                MTOOLS_ASSERT(alph > 0);
                if (alph < 1.) alph += 1.;
                a1 = alph - 1. / 3.;
                a2 = 1. / sqrt(9.*a1);
                }


            /**
            * Generate a Gamma random variable.
            *
            * @param [in,out]  gen The random number generator.
            *
            * @return  the random variable
            **/
            template<class random_t> double operator()(random_t & gen)
                {
                double u, v, x;
                do 
                    {
                    do { x = normale(gen); v = 1. + a2*x; }
                    while (v <= 0.);
                    v = v*v*v; u = Unif(gen);
                    }
                while((u > 1. - 0.331*(x*x*x*x)) && (log(u) > 0.5*(x*x) + a1*(1. - v + log(v))));
                if (alph == oalph) return a1*v / bet; else { do u = Unif(gen); while (u == 0.); return pow(u, 1. / oalph)*a1*v / bet; }
                }


        private:

            NormalLaw normale;
            double alph, oalph, bet, a1, a2;


        };



    /**
    * create a Beta random variable.
    * The density of the BETA(alpha,Beta) on [0,1] is given by
    *
    * f(x) = (1/C) * x^(alpha-1) * (1-x)^(beta-1).
    *
    * Taken from numerical recipes
    **/
    class BetaLaw
        {

        public:

            /**
             * Constructor Set the parameters.
             *
             * @param   alpha   parameter alpha.
             * @param   beta    parameter beta.
             **/
            BetaLaw(double alpha, double beta) : G1(alpha,1), G2(beta,1) { }


            /**
             * Set the parameters.
             *
             * @param   alpha   parameter alpha.
             * @param   beta    parameter beta.
             **/
            inline void setParam(double alpha, double beta) { G1.setParam(alpha, 1); G2.setParam(beta, 1); }


            /**
            * Generate a Beta random variable.
            *
            * @param [in,out]  gen The random number generator.
            *
            * @return  the random variable
            **/
            template<class random_t> inline double operator()(random_t & gen) { double x = G1(gen); double y = G2(gen); return x / (x + y); }


        private:

            GammaLaw G1, G2;

        };



    /**
    * create a Poisson random variable.
    *
    * Taken from numerical recipes
    **/
    class PoissonLaw
        {

        public:

            /**
             * Constructor Set the parameters.
             *
             * @param   lambdaa parameter lambda.
             **/
            PoissonLaw(double lambdaa) : lambold(-1.), logfact(1024, -1.) { setParam(lambdaa); }


            /**
             * Set the parameters.
             *
             * @param   lambdaa parameter lambda.
             **/
            void setParam(double lambdaa) { lambda = lambdaa; }


            /**
            * Generate a Poisson random variable.
            *
            * @param [in,out]  gen The random number generator.
            *
            * @return  the random variable
            **/
            template<class random_t> double operator()(random_t & gen)
                {
                double u, u2, v, v2=0;
				double p, t, lfac;
                int k;
                if (lambda < 5.) 
                    {
                    if (lambda != lambold) lamexp = exp(-lambda);
                    k = -1; t = 1.;
                    do { ++k; t *= Unif(gen); } while (t > lamexp);
                    }
                else 
                    {
                    if (lambda != lambold) { sqlam = sqrt(lambda); loglam = log(lambda); }
                    for (;;) 
                        {
                        u = 0.64*Unif(gen); v = -0.68 + 1.28*Unif(gen);
                        if (lambda > 13.5) 
                            {
                            v2 = v*v;
                            if (v >= 0.) { if (v2 > 6.5*u*(0.64 - u)*(u + 0.2)) continue; } else { if (v2 > 9.6*u*(0.66 - u)*(u + 0.07)) continue; }
                            }
                        k = int(floor(sqlam*(v / u) + lambda + 0.5));
                        if (k < 0) continue;
                        u2 = u*u;
                        if (lambda > 13.5) 
                            {
                            if (v >= 0.) { if (v2 < 15.2*u2*(0.61 - u)*(0.8 - u)) break; } else { if (v2 < 6.76*u2*(0.62 - u)*(1.4 - u)) break; }
                            }
                        if (k < 1024) 
                            {
                            if (logfact[k] < 0.) logfact[k] = gammln(k + 1.);
                            lfac = logfact[k];
                            }
                        else lfac = gammln(k + 1.);
                        p = sqlam*exp(-lambda + k*loglam - lfac);
                        if (u2 < p) break;
                        }
                    }
                lambold = lambda;
                return k;
                }

        private:

            double lambda, sqlam, loglam, lamexp, lambold;
            std::vector<double> logfact;

        };






    /**
     * Simulate a D-dimensional Poisson point process PPP with a given density inside a box of R^d.
     * 
     * If (an upper obund of) the maximum of the density in the box is unknown, it is estimated 
     * numerically first (but it is better to give the exact value/upper bound) if possible. 
     *
     * @tparam          D                  dimension of the space.
     * @param           gen                The random number generator.
     * @param           density            the density function, signature fVec<D> -> double.
     * @param           boundary           the boundary box.
     * @param           maxdensity         (Optional) the maximum density. value <= 0 means unknown: the max density is then estimated by sampling points and taking a some margin.
     * @param           mesh               (Optional) the mesh size to use for estimating max density when unknown (0 for automatic choice)
     * @param           max_margin         (Optional) margin to use for the estimated maximum of the density if not specified (1 is default).
     *
     * @returns A vector<fVec<D>> containing the points of the Poisson point process.
     **/
    template<int D, typename DENSITY_FUN, typename random_t> std::vector<mtools::fVec<D>> PoissonPointProcess(random_t& gen, DENSITY_FUN& density, fBox<D> boundary, double maxdensity = 0, size_t mesh = 0, double max_margin = 1.0)
        {
        if (maxdensity <= 0)
            { // estimate the maximum of the density
            if (mesh == 0)
                {
                switch (D)
                    {
                    case 1: mesh = 1001; break;
                    default: mesh = 101; break;
                    }
                }
            maxdensity = maxFunction<D>(density, boundary, mesh) * (1.0 + std::max(max_margin,0.0));
            }
        // compute the area of the box
        double aera = boundary.area() * maxdensity;
        // generate the number of points in the box
        PoissonLaw Poisson(aera);
        int64 nb_points = (int64)Poisson(gen);
        // generate the points of the PPP using the rejection method
        std::vector<fVec<D>> points;
        for (int64 i = 0; i < nb_points; i++)
            {
            fVec<D> P = Unif_dimD<D>(boundary, gen);
            const double u = Unif_highprecision(gen) * maxdensity;
            if (u < density(P)) { points.push_back(P); }
            }
        return points;
        }


    /**
     * Simulate a 1D Poisson point process PPP with a given density inside an interval [minx,maxx].
     *
     * @param [in,out]  gen                The random number generator.
     * @param [in,out]  density            the density function, signature double -> double.
     * @param           xmin               the min value of the interval
     * @param           xmax               the max value of the interval
     * @param           maxdensity         (Optional) the maximum density. value <= 0 means unknown: the max density is then estimated by sampling points and taking a some margin.
     * @param           mesh               (Optional) the mesh size to use for estimating max density when unknown (0 for automatic choice)
     * @param           max_margin         (Optional) margin to use for the estimated maximum of the density if not specified (1 is default).
     *
     * @returns A vector<double> containing the points of the Poisson point process.
     */
    template<typename DENSITY_FUN, typename random_t> std::vector<double> PoissonPointProcess_1D(random_t& gen, DENSITY_FUN& density, double xmin, double xmax, double maxdensity = 0, size_t mesh = 0, double max_margin = 1.0)
        {
        if (xmax < xmin) std::swap(xmin, xmax);
        fBox1 b;
        b.min[0] = xmin; 
        b.max[0] = xmax;
        auto V = PoissonPointProcess(gen, [&density](fVec1 x) { return density(x[0]); }, b, maxdensity, mesh, max_margin);
        std::vector<double> res;
        res.reserve(V.size());
        for (auto& v : V) { res.push_back(v[0]); }
        return res;
        }






    /** used by PoissonPointProcess_fast() */
    template<int D, typename DENSITY_FUN, typename DENSITY_MAX> double _rejectedRatio(DENSITY_FUN & fun, fBox<D> B, DENSITY_MAX& density_max, size_t nb_samples)
        {       
        const double threshold = density_max(B);
        size_t nb_reject = 0;
        for (size_t i = 0; i < nb_samples; i++)
            {
            const fVec<D> P = Unif_dimD<D>(B, gen);
            if (Unif_highprecision(gen) * threshold >= fun(P)) { nb_reject++; }
            }
        double A = B.area();
        double rej = ((double)nb_reject) / nb_samples;
        //cout << "box : " << B << "\n";
        //cout << "max : " << threshold << "\n\n";        
        return A*rej*threshold;
        }


    


    /** used by PoissonPointProcess_fast() */
    template<int D, typename DENSITY_FUN, typename DENSITY_MAX> std::vector<fBox<D> > _splitBoxToMinimizeRejection(DENSITY_FUN & fun, DENSITY_MAX & density_max, fBox<D> B, size_t nbsplit, size_t nb_samples)
        {
        std::multimap<double, fBox<D> > mapB;
        const double rr = -_rejectedRatio(fun, B, density_max, nb_samples);
        mapB.insert({ rr, B });
        while ((mapB.size() < nbsplit))
            {
            B = mapB.begin()->second;
            //cout << "\nSELECTION " << B << "\n\n";
            mapB.erase(mapB.begin());
            double bestrr1 = -mtools::INF;
            double bestrr2 = -mtools::INF;
            fBox<D> bestB1;
            fBox<D> bestB2;
            for (int k = 0; k < D; k++)
                {
                //cout << "\nsplit " << k << "\n";
                const fBox<D> B1 = B.get_split(k, true);
                const double rr1 = -_rejectedRatio(fun, B1, density_max, nb_samples);
                const fBox<D> B2 = B.get_split(k, false);
                const double rr2 = -_rejectedRatio(fun, B2, density_max, nb_samples);
                if (std::min(rr1, rr2) > std::min(bestrr1, bestrr2))
                    {
                    bestrr1 = rr1;
                    bestB1 = B1;
                    bestrr2 = rr2;
                    bestB2 = B2;                    
                    }
                }
            mapB.insert({ bestrr1, bestB1 });
            mapB.insert({ bestrr2, bestB2 });
            }
        std::vector<fBox<D> > res;
        res.reserve(mapB.size());
        for (auto& [d, b] : mapB) { res.push_back(b); }
        return res;
        }


    /**
     * Simulate a D-dimensional Poisson point process PPP with a given density inside a box of R^d.
     * -> Version where the maximum of the density is known for each box 
     * -> Fast version by splitting the box into smaller sub-boxes to minimize rejection. 
     *
     * @tparam          D                  dimension of the space.
     * @param           gen                The random number generator.
     * @param           density            the density function, signature fVec<D> -> double.
     * @param           density_max        An upper obund on the density inside a given box, signature Box<D> -> double.
     * @param           boundary           the boundary box.
     * @param           nb_splits          (Optional) number of splitting of the main boundary box  (0 for automatic choice).
     * @param           nb_samples         (Optional) number of sample in each box to estimated maximum of density and estimate rejection (0 for automatic choice).
     *
     * @returns A vector containing the points of the Poisson point process.
     **/
    template<int D, typename DENSITY_FUN, typename DENSITY_MAX, typename random_t> std::vector<mtools::fVec<D>> PoissonPointProcess_fast(random_t& gen, DENSITY_FUN& density, DENSITY_MAX& density_max, fBox<D> boundary, size_t nb_splits = 0, size_t nb_samples = 0)
        {
        if (nb_splits == 0)
            {
            switch (D)
                {
                case 1: nb_splits = 20; break;
                default: nb_splits = 60; break;
                }
            }
        if (nb_samples == 0)
            {
            switch (D)
                {
                case 1: nb_samples = 1000; break;
                default: nb_samples = 1000; break;
                }
            }
        auto VB = _splitBoxToMinimizeRejection(density, density_max, boundary, nb_splits, nb_samples);
        std::vector<fVec<D>> res;
        for (auto b : VB)
            {
            std::vector<fVec<D>> points = PoissonPointProcess(gen, density, b, density_max(b));
            res.insert(res.end(), points.begin(), points.end());
            }
        return res;
        }


     /**
     * Simulate a D-dimensional Poisson point process PPP with a given density inside a box of R^d.
     * -> version where the maximum of the density is unknown and estimated by sampling points and taking a some margin.
     * -> Fast version by splitting the box into smaller sub-boxes to minimize rejection. 
     *
     * Version 
     * 
     * @tparam          D                  dimension of the space.
     * @param           gen                The random number generator.
     * @param           density            the density function, signature fVec<D> -> double.
     * @param           boundary           the boundary box.
     * @param           nb_splits          (Optional) number of splitting of the main boundary box  (0 for automatic choice).
     * @param           nb_samples         (Optional) number of sample in each box to estimated maximum of density and estimate rejection (0 for automatic choice).
     * @param           mesh               (Optional) number of points in each direction of the mesh used to estimated the maximum of the density (0 for automatic choice).
     * @param           max_margin         (Optional) margin to use for the estimated maximum of the density (default 1)
     *
     * @returns A vector containing the points of the Poisson point process.
     **/
    template<int D, typename DENSITY_FUN, typename random_t> std::vector<mtools::fVec<D>> PoissonPointProcess_fast(random_t& gen, DENSITY_FUN& density, fBox<D> boundary, size_t nb_splits = 0, size_t nb_samples = 0, size_t mesh = 0, double max_margin = 1.0)
        {
        if (mesh == 0)
            {
            switch (D)
                {
                case 1: mesh = 1001; break;
                default: mesh = 101; break;
                }
            }
        return PoissonPointProcess_fast(gen, density, [&density,&mesh,&max_margin](const fBox<D>& B) { return maxFunction<D>(density, B, mesh)* (max_margin + 1); }, boundary, nb_splits, nb_samples);
        }


    /**
     * Simulate a 1-dimensional Poisson point process PPP with a given density inside a interval [minx,maxx].
     * -> Version where the maximum of the density is known for each interval
     * -> Fast version by splitting the box into smaller sub-boxes to minimize rejection.
     *
     * @tparam          D                  dimension of the space.
     * @param           gen                The random number generator.
     * @param           density            the density function, signature double -> double.
     * @param           density_max        an uper bound on the maximum of the density in a box: signature "(double xmin, double xmax) -> double"                                     
     * @param           xmin               the min value of the full interval
     * @param           xmax               the max value of the full interval
     * @param           nb_splits          (Optional) number of splitting of the main boundary box  (0 for automatic choice).
     * @param           nb_samples         (Optional) number of sample in each box to estimated maximum of density and estimate rejection (0 for automatic choice).
     *
     * @returns A vector<double> containing the points of the Poisson point process.
     **/
    template<typename DENSITY_FUN, typename DENSITY_MAX, typename random_t> std::vector<double> PoissonPointProcess_fast_1D(random_t& gen, DENSITY_FUN& density, DENSITY_MAX & density_max, double xmin, double xmax, size_t nb_splits = 0, size_t nb_samples = 0)
        {
        if (xmax < xmin) std::swap(xmin, xmax);
        fBox1 B;
        B.min[0] = xmin;
        B.max[0] = xmax;
        auto V = PoissonPointProcess_fast(gen, [&density](fVec1 x) { return density(x[0]); }, [&density_max](const fBox1 & bb) { return density_max(bb.min[0], bb.max[0]); }, B, nb_splits, nb_samples);
        std::vector<double> res;
        res.reserve(V.size());
        for (auto& v : V) { res.push_back(v[0]); }
        return res;
        }


    /**
     * Simulate a 1-dimensional Poisson point process PPP with a given density inside a interval [minx, maxx].
     * -> version where the maximum of the density is unknown and estimated by sampling points and taking a some margin.
     * -> Fast version by splitting the box into smaller sub-boxes to minimize rejection.
     *
     * @tparam          D                  dimension of the space.
     * @param           gen                The random number generator.
     * @param           density            the density function, signature double -> double.
     * @param           xmin                the min value of the interval
     * @param           xmax                the max value of the interval
     * @param           nb_splits          (Optional) number of splitting of the main boundary box  (0 for automatic choice).
     * @param           nb_samples         (Optional) number of sample in each box to estimated maximum of density and estimate rejection (0 for automatic choice).
     * @param           mesh               (Optional) number of points in each direction of the mesh used to estimated the maximum of the density (0 for automatic choice).
     * @param           max_margin         (Optional) margin to use for the estimated maximum of the density (1 is default)
     *
     * @returns A vector<double> containing the points of the Poisson point process.
     **/
    template<typename DENSITY_FUN, typename random_t> std::vector<double> PoissonPointProcess_fast_1D(random_t& gen, DENSITY_FUN& density, double xmin, double xmax, size_t nb_splits = 0, size_t nb_samples = 0, size_t mesh = 0, double max_margin = 1.0)
        {
        if (mesh == 0) mesh = 1001;
        return PoissonPointProcess_fast_1D(gen, density, [&density, &mesh, &max_margin](double xmin, double xmax) {return maxFunction_1D(density, xmin, xmax, mesh) * (max_margin + 1); }, xmin, xmax, nb_splits, nb_samples);
        }





    



	/** 
	* Symmetric distribution over an the integer interval {0,1,..,N-1}
	*
	* VERY VERY FAST !
	*
	* The law of the random variable returned is  B*X + B*(N-1 - X)
	* where B is a Bernouilli 1/2 and X is uniform in {0,..,D-1} where D
	* is the largest power of two smaller than N.
	**/
	class FastLaw
		{
		public:

			/**
			* Constructor. Set the parameter N.
			*
			* @param	N	Support of the law is {0,..,N-1} with 0 < N < 2^31.
			**/
			FastLaw(uint32 N) : Nminus1(N - 1), Lminus1(pow2rounddown(N) - 1)
				{
				MTOOLS_ASSERT(N > 0);
				MTOOLS_ASSERT(N < (1UL << 31));
				}


			/**
			* Set the parameter.
			*
			* @param	N	Support of the law is {0,..,N-1} with 0 < N < 2^31.
			**/
			void setParam(uint32 N)
				{
				MTOOLS_ASSERT(N > 0);
				MTOOLS_ASSERT(N < (1UL << 31));
				Nminus1 = N - 1;
				Lminus1 = pow2rounddown(N) - 1;
				}


			/**
			* Get a random number.
			*
			* @param [in,out]	gen	The random generator.
			*
			* @return	the random variable in {0,...,N-1}
			**/
			template<class random_t> MTOOLS_FORCEINLINE uint32 operator()(random_t & gen) const
				{
				return operator()((uint32)gen());
				}


			/**
			* Get a random number. Take a uniform integer as input parameter instead of a generator.
			* Since only certain bits are used, this permit to reuse the same input several time (by appying a shift). 
			*
			* @param [in,out]	x	an uniformly distributed integer (in fact, only bits up to pow2roundup(N + 1) are used).
			*
			* @return	the random variable in {0,...,N-1}
			**/
			MTOOLS_FORCEINLINE uint32 operator()(uint32_t x) const
				{
				uint32 v = (x >> 1) & Lminus1;
				return ((x & 1)*v + (~x & 1)*(Nminus1 - v));
				}


		private:
			uint32 Nminus1, Lminus1;
		};



}


/* end of file  */


