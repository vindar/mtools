/** @file vec.hpp */
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


#include "../misc/error.hpp"
#include "../misc/misc.hpp"
#include "../misc/stringfct.hpp"
#include "../io/serialization.hpp"
#include "complex.hpp"

#include <algorithm>
#include <string>
#include <type_traits>
#include <ostream>
#include <initializer_list>


namespace mtools
{

    /**
     * A N-dimensional vector.
     *
     * @tparam  T   Generic type parameter.
     * @tparam  N   Type of the n.
     **/
    template<typename T, size_t N> class Vec
    {

        static_assert(N > 0, "Template parameter N must be non zero");

    public:

        /**
         * Default constructor. No initialization of the vector coordinate !
         **/
        Vec() {}


        /**
         * Constructor. Fill the vector with an initializer_list. If the initializer_list is too small,
         * the last value is used to complete the initialization. If the list is too long, the remaining
         * values are discarded.
         *
         * @param   l   the intializer list.
         **/
        Vec(std::initializer_list<T> l)
            {
            auto it = l.begin();
            T v = 0;
            for(size_t i = 0; i < N; i++)
                {
                if (it != l.end()) {v = (*it); ++it; }
                _m_tab[i] = v;
                }
            }


        /**
         * Constructor. Fill the vector with a single value.
         *
         * @param   v   the value to fill the vector with.
         **/
        Vec(const T & v) { for (size_t i = 0; i < N; i++) _m_tab[i] = v; }


        /**
         * Constructor for two-dimensional vector
         *
         * @param   x   The x coordinate.
         * @param   y   The y coordinate
         **/
        Vec(const T & x, const T & y) { static_assert(N == 2, "template parameter N must be 2"); _m_tab[0] = x; _m_tab[1] = y; return; }


		/**
		 * Constructor from a complex number
		 */
		Vec(const mtools::complex<T> & c)  { static_assert(N == 2, "template parameter N must be 2"); _m_tab[0] = c.real(); _m_tab[1] = c.imag(); return;  }


		/**
		 * Explicit cast from a 2 dimensionnal vector to complex number.
		 */
		explicit operator std::complex<T>() const  { static_assert(N == 2, "template parameter N must be 2"); return mtools::complex<T>(_m_tab[0], _m_tab[1]); }


        /**
         * Constructor for 3-dim vector
         *
         * @param   x   The x coordinate.
         * @param   y   The y coordinate.
         * @param   z   The z coordinate.
         **/
        Vec(const T & x, const T & y, const T & z) { static_assert(N == 3, "template parameter N must be 3");  _m_tab[0] = x; _m_tab[1] = y; _m_tab[2] = z;  return; }


        /**
        * Default copy constructor.
        **/
        Vec(const Vec & V) = default;


        /**
        * Copy constructor from another template parameter.
        **/
        template<typename U> Vec(const Vec<U, N> & V) { for (size_t i = 0; i < N; i++) { _m_tab[i] = (T)(V[i]); } }


        /**
        * Default assignment operator.
        **/
        Vec & operator=(const Vec & R) = default;


        /**
        * Assignment operator from another type.
        **/
//        template<typename U> Vec<U,N> & operator=(const Vec<U,N> & V)  { for (size_t i = 0; i < N; i++) { _m_tab[i] = (T)(V[i]); } return(*this); }


        /**
        * Assignment operator from constant value.
        **/
        Vec & operator=(const T & x) { for (size_t i = 0; i < N; i++) { _m_tab[i] = x; } return(*this); }


        /**
         * Equality operator.
         **/
        inline bool operator==(const Vec<T,N> & V) const {for (size_t i = 0; i < N; i++) {if (_m_tab[i] != V._m_tab[i]) { return false; } } return true;}


        /**
         * Inequality operator.
         **/
        inline bool operator!=(const Vec<T, N> & V) const { return(!(operator==(V))); }


        /**
         * Less-than comparison operator. Lexicographical order.
         **/
        inline bool operator<(const Vec<T, N> & V) const { for (size_t i = 0; i < N; i++) { if (_m_tab[i] < V._m_tab[i]) { return true; } else if (_m_tab[i] > V._m_tab[i]) { return false; } } return false; }


        /**
         * Less-than-or-equal comparison operator. Lexicographical order.
         **/
        inline bool operator<=(const Vec<T, N> & V) const { for (size_t i = 0; i < N; i++) {if (_m_tab[i] < V._m_tab[i]) { return true; } else if (_m_tab[i] > V._m_tab[i]) { return false; } } return true; }


         /**
          * Greater-than comparison operator. Lexicographical order.
          **/
         inline bool operator>(const Vec<T, N> & V) const { return(!(operator<=(V))); }


         /**
          * Greater-than-or-equal comparison operator. Lexicographical order.
          * @return true if the first parameter is greater than or equal to the second.
          **/
         inline bool operator>=(const Vec<T, N> & V) const { return(!(operator<(V))); }


         /**
          * Addition assignment operator: coordinate by coordinate.
          *
          * @param  V   The vector to add.
          *
          * @return the vector for chaining.
          **/
         inline Vec<T, N> & operator+=(const Vec<T, N> & V) { for (size_t i = 0; i < N; i++) { _m_tab[i] += V._m_tab[i]; } return(*this); }


         /**
          * Subtraction assignment operator: coordinate by coordinate.
           *
          * @param  V   The vector to substract.
          *
          * @return the vector for chaining.
         **/
         inline Vec<T, N> & operator-=(const Vec<T, N> & V) { for (size_t i = 0; i < N; i++) { _m_tab[i] -= V._m_tab[i]; } return(*this); }


         /**
          * Multiplication assignment operator: coordinate by coordinate.
          *
          * @param  V   The vector to multiply with.
          *
          * @return the vector for chaining.
          **/
         inline Vec<T, N> & operator*=(const Vec<T, N> & V) { for (size_t i = 0; i < N; i++) { _m_tab[i] *= V._m_tab[i]; } return(*this); }


         /**
          * Division assignment operator: coordinate by coordinate.
          *
          * @param  V   The vector to divide with..
          *
          * @return the vector for chaining.
          **/
         inline Vec<T, N> & operator/=(const Vec<T, N> & V) { for (size_t i = 0; i < N; i++) { _m_tab[i] /= V._m_tab[i]; } return(*this); }


         /**
          * Scalar addition assignment operator
          **/
         inline Vec<T, N> & operator+=(const T & v) { for (size_t i = 0; i < N; i++) { _m_tab[i] += v; } return(*this); }


         /**
          * Scalar subtraction assignment operator.
          **/
         inline Vec<T, N> & operator-=(const T & v) { for (size_t i = 0; i < N; i++) { _m_tab[i] -= v; } return(*this); }


         /**
          * Multiplication assignment operator.
          **/
         inline Vec<T, N> & operator*=(const T & v) { for (size_t i = 0; i < N; i++) { _m_tab[i] *= v; } return(*this); }


         /**
          * Division assignment operator.
          **/
         inline Vec<T, N> & operator/=(const T & v) { for (size_t i = 0; i < N; i++) { _m_tab[i] /= v; } return(*this); }


         /**
          * Access vector elements.
          *
          * @param  i   Zero-based index of the coordinate.
          *
          * @return The value of the coodinate
          **/
         inline T & operator[](size_t i) { MTOOLS_ASSERT(i < N); return _m_tab[i]; }
         inline const T & operator[](size_t i) const { MTOOLS_ASSERT(i < N);  return _m_tab[i]; }


         /**
          * Gets the X coordinate (first coordinate).
          **/
         inline T & X() { return _m_tab[0]; }
         inline const T & X() const { return _m_tab[0]; }


         /**
          * Gets the Y coordinate (second coordinate).
          **/
         inline T & Y() { static_assert(N > 1, "template parameter N must be at least 2"); return _m_tab[1]; }
         inline const T & Y() const { static_assert(N > 1, "template parameter N must be at least 2"); return _m_tab[1]; }


         /**
          * Gets the Z coordinate (third coordinate).
          **/
         inline T & Z() { static_assert(N > 2, "template parameter N must be at least 3"); return _m_tab[2]; }
         inline const T & Z() const { static_assert(N > 2, "template parameter N must be at least 3"); return _m_tab[2]; }


         /**
          * Compute the square norm of the vector.
          *
          * @return The square norm as an elemnt of type T.
          **/
         inline T norm2() const { T v = 0; for (size_t i = 0; i < N; i++) { v += (_m_tab[i] * _m_tab[i]); } return v;}


         /**
          * Compute the norm of the vector (return a double).
          *
          * @return the norm of the vector as a double.
          **/
         inline double norm() const { return(sqrt((double)(norm2()))); }


         /**
         * Normalise the vector so that its norm is 1, does nothing if the vector is 0.
         **/
         inline void normalize() { double a = norm(); if (a>0) { for (int n = 0; n<N; n++) _m_tab[n] = (T)(_m_tab[n]/a); } }


         /**
          * @brief  Swap the elements at position i and j.
          *
          * @param  i   position of the first element.
          * @param  j   position of the second element.
          **/
         inline void swap(size_t i, size_t j) { MTOOLS_ASSERT((i<N)&&(j<N)); T tmp = _m_tab[i]; _m_tab[i] = _m_tab[j]; _m_tab[j] = tmp; }


        /**
         * Convert the vector into a string.
         *
         * @param   includeTypeInfo Set to true (default) to include information about the template
         *                          parameters.
         *
         * @return  A std::string that represents the vector in the form "[x,y,z,...]".
         **/
        std::string toString(bool includeTypeInfo = true) const
            {
            std::string s;
            if (includeTypeInfo) { s += std::string("Vec<") + typeid(T).name() + "," + mtools::toString(N) + ">"; }
            s += std::string("[") + mtools::toString(_m_tab[0]);
            for (size_t i = 1; i < N; i++) { s += std::string(",") + mtools::toString(_m_tab[i]); }
            return(s+"]");
            }


        /**
         * Return a pointer to the underlying C array of length N (const version)
         **/
        const T * data() const { return _m_tab; }


        /**
         * Return a pointer to the underlying C array of length N (non-const version)
         **/
        T * data() { return _m_tab; }


        /**
         * Reverses the order of the coordinates.
         **/
        void reverse() {for (size_t i = 0; i < (N / 2); i++) { std::swap(_m_tab[i], _m_tab[N - i - 1]); }}


        /**
         * Set every coordinate of the vector to v.
         **/
        void clear(const T & v)
            {
            for (size_t i = 0; i < N; i++) {_m_tab[i] = v;}
            }


        /**
         * serialise/deserialize the vector. Works with boost and with the custom serialization classes
         * OArchive and IArchive. the method performs both serialization and deserialization.
         **/
        template<typename U> void serialize(U & Archive,const int version = 0)
            {
            Archive & _m_tab;
            }

    private:

        T _m_tab[N];        ///< the array.
    };


    /**
    * Compute the square of the euclidian distance between two vectors
    *
    * @param   V1  The first vector
    * @param   V2  The second vector
    *
    * @return  The square of the euclidian distance.
    **/
    template<typename T, size_t N> inline T dist2(const Vec<T, N> & V1, const Vec<T, N> & V2) { T v = 0; for (size_t i = 0; i < N; i++) { const T a = (V1[i] - V2[i]);   v += (a*a); } return v; }


    /**
     * Compute the euclidian distance between two vectors
     *
     * @param   V1  The first vector
     * @param   V2  The second vector
     *
     * @return  The distance as a double.
     **/
    template<typename T, size_t N> inline double dist(Vec<T, N> V1, const Vec<T, N> & V2) { return(sqrt((double)dist2(V1,V2))); }


    /**
     * Addition operator. Coordinates by coordinates
     **/
    template<typename T, size_t N> inline Vec<T, N> operator+(Vec<T, N> V1, const Vec<T, N> & V2) { V1 += V2; return V1;}


    /**
     * Substraction operator. Coordinates by coordinates
     **/
    template<typename T, size_t N> inline Vec<T, N> operator-(Vec<T, N> V1, const Vec<T, N> & V2) { V1 -= V2; return V1; }


    /**
     * Multiplication operator. Coordinates by coordinates
     **/
    template<typename T, size_t N> inline Vec<T, N> operator*(Vec<T, N> V1, const Vec<T, N> & V2) { V1 *= V2; return V1; }


    /**
     * Division operator. Coordinates by coordinates
     **/
    template<typename T, size_t N> inline Vec<T, N> operator/(Vec<T, N> V1, const Vec<T, N> & V2) { V1 /= V2; return V1; }


    /**
     * Scalar addition operator.
     **/
    template<typename T, size_t N> inline Vec<T, N> operator+(const T & a, Vec<T, N> V) { V += a; return V; }
    template<typename T, size_t N> inline Vec<T, N> operator+(Vec<T, N> V, const T & a) { V += a; return V; }


    /**
     * Scalar substraction operator.
     **/
    template<typename T, size_t N> inline Vec<T, N> operator-(const T & a, Vec<T, N> V) { for (int i = 0; i < N; i++) { V._m_tab[i] = a - V._m_tab[i]; } return V; }
    template<typename T, size_t N> inline Vec<T, N> operator-(Vec<T, N> V,const T & a)  { V -= a; return V; }


    /**
     * Scalar multiplication operator.
     **/
    template<typename T, size_t N> inline Vec<T, N> operator*(const T & a, Vec<T, N> V) { V *= a; return V; }
    template<typename T, size_t N> inline Vec<T, N> operator*(Vec<T, N> V,const T & a) { V *= a; return V; }


    /**
     * Scalar division operator.
     **/
    template<typename T, size_t N> inline Vec<T, N> operator/(const T & a, Vec<T, N> V) { V /= a; return V; }
    template<typename T, size_t N> inline Vec<T, N> operator/(Vec<T, N> V, const T & a) { V /= a; return V; }


    /**
     * The dot product U.V between two vectors.
     *
     * @param   U  The first vector.
     * @param   V  The second vector.
     *
     * @return  The dot product.
     **/
    template<typename T, size_t N> inline T dotProduct(const Vec<T, N> & U, const Vec<T, N> & V) { T v = 0; for (size_t i = 0; i < N; i++) { v += (U[i] * V[i]); } return v; }


    /**
     * Cross product UxV of two 3-dimensional vectors U and V.
     *
     * @tparam  N   must be equal to 3.
     * @param   U   The first vector.
     * @param   V   The second vector.
     *
     * @return  the cross product UxV.
     **/
    template<typename T, size_t N> inline T crossProduct(const Vec<T, N> & U, const Vec<T, N> & V) { static_assert(N==3,"dimension must be 3 for croos product."); return Vec<T, N>(U[1]*V[2] - U[2]*V[1], U[2]*V[0] - U[0]*V[2], U[0]*V[1] - U[1]*V[0]); }


    /**
     * Compute the square L2 norm of a vector.
     *
     * @param   V   The vector.
     *
     * @return  The square norm.
     **/
    template<typename T, size_t N> inline  T norm2(const Vec<T, N> & V) { return V.norm2(); }


    /**
    * Compute the L2 norm of a vector.
    *
    * @param   V   The vector.
    *
    * @return  The L2 norm as a double
    **/
    template<class T, size_t N> inline  double norm(const Vec<T, N> & V) { return V.norm(); }


    /**
     * integer valued vector
     **/
    template<size_t N> using iVec = Vec<int64, N>;


    /**
     * floating point valued vector
     **/
    template<size_t N> using fVec = Vec<double, N>;


    /**
     * 2-dim integer vector
     **/
    typedef iVec<2> iVec2;


    /**
     * 3-dim integer vector
     **/
    typedef iVec<3> iVec3;


    /**
     * 2-dim floating point vector
     **/
    typedef fVec<2> fVec2;


    /**
     * 3-dim floating point vector
     **/
    typedef fVec<3> fVec3;


}


/* end of file */

