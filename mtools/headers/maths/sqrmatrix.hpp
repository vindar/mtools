/** @file sqrmatrix.hpp */
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
#include "vec.hpp"


namespace mtools
{


/**
 * A simple class representing a NxN matrix
 *
 * @tparam  T   Type of the coeffcient, must implement the classical arithmetic operations.
 * @tparam  N   dimension of the matrix.
 **/
template<class T, size_t N> class SqrMatrix
 {

 public:

     /**
     * Default constructor. Does not initialise its elements.
     **/
     SqrMatrix() {return;}


     /**
      * Constructor. Construct a matrix with all its coefficients equal to v
      **/
	 SqrMatrix(T v) {setCst(v);}


     /**
      * Copy Constructor.
      **/
	 SqrMatrix(const SqrMatrix<T,N> & M) {operator=(M);}


     /**
      * Assignment operator.
      **/
	 inline SqrMatrix & operator=(const SqrMatrix<T,N> & M) {for(size_t n=0;n<N*N;n++) {tab[n] = M.tab[n];} return(*this);}


     /**
      * Equality operator.
      **/
	 inline bool operator==(const SqrMatrix<T,N> & M) const {for(size_t n=0;n<N*N;n++) {if (tab[n] != M.tab[n]) {return false;}} return true;}


     /**
      * Inequality operator.
      **/
	 inline bool operator!=(const SqrMatrix<T,N> & M) const {return(!(this->operator==(M)));}


     /**
      * Less-than comparison operator. Lexicographical comparison order M(0,0), M(1,0), M(2,0), ... M(N-1,0) , M(0,1), M(1,1), ... M(N-2,N-1), M(N-1,N-1).
      **/
	 inline bool operator<(const SqrMatrix<T,N> & M) const {for(size_t n=0;n<N*N;n++) {if (tab[n] < M.tab[n]) {return true;} if (tab[n] > M.tab[n]) {return false;}} return false;}


     /**
      * Less-than-or-equal comparison operator. Lexicographical comparison order M(0,0), M(1,0), M(2,0), ... M(N-1,0) , M(0,1), M(1,1), ... M(N-2,N-1), M(N-1,N-1).
      **/
	 inline bool operator<=(const SqrMatrix<T,N> & M) const {for(size_t n=0;n<N*N;n++) {if (tab[n] < M.tab[n]) {return true;} if (tab[n] > M.tab[n]) {return false;}} return true;}


     /**
      * Greater-than comparison operator. Lexicographical comparison order M(0,0), M(1,0), M(2,0), ... M(N-1,0) , M(0,1), M(1,1), ... M(N-2,N-1), M(N-1,N-1).
      **/
	 inline bool operator>(const SqrMatrix<T,N> & M) const {return(!(this->operator<=(M)));}


     /**
      * Greater-than-or-equal comparison operator. Lexicographical comparison order M(0,0), M(1,0), M(2,0), ... M(N-1,0) , M(0,1), M(1,1), ... M(N-2,N-1), M(N-1,N-1).
      **/
	 inline bool operator>=(const SqrMatrix<T,N> & M) const {return(!(this->operator<(M)));}


     /**
      * Access the element M_{i,j}. 
      **/
	 inline T & operator()(size_t  i, size_t j) {MTOOLS_ASSERT((i<N)&&(j<N)); return tab[i + N*j];}

     /**
     * Access the element M_{i,j} (const version).
     **/
     inline const T & operator()(size_t i,size_t j) const {MTOOLS_ASSERT((i<N) && (j<N)); return tab[i + N*j];}


     /**
      * Fill the matrix with a constant value
      *
      * @param  v   The value to fill the matrix with.
      **/
	 inline void setCst(T v) {for(size_t n=0;n<N*N;n++) {tab[n] = v;}}


     /**
     * Set the matrix as the identity matrix.
     **/
     inline void setIdentity() {setCst(0); for(size_t i=0;i<N;i++) {tab[i+N*i] = 1;}}


     /**
      * Exchange two columns j1 and j2 of the matrix.
      *
      * @param  j1  The first column.
      * @param  j2  The second column.
      **/
	 inline void exchangeColumn(size_t j1,size_t j2) 
		{
        MTOOLS_ASSERT((j1<N) && (j2<N));
		if (j1 == j2) {return;}
		T temp; for(size_t i=0;i<N;i++) {temp = tab[i + N*j1]; tab[i + N*j1] = tab[i + N*j2]; tab[i + N*j2] = temp;}
		}


     /**
     * Exchange two lines i1 and i2 of the matrix.
     *
     * @param  i1  The first column.
     * @param  i2  The second column.
     **/
     inline void exchangeLine(size_t  i1, size_t  i2)
		{
        MTOOLS_ASSERT((i1<N) && (i2<N));
		if (i1 == i2) {return;}
		T temp; for(size_t j=0;j<N;j++) {temp = tab[i1 + N*j]; tab[i1 + N*j] = tab[i2 + N*j]; tab[i2 + N*j] = temp;}
		}


     /**
      * Elementary operation: Multiply columns C_j  <--  l*C_j
      *
      * @param  l   The scalar value to multiply the column with.
      * @param  j   index of the column.
      **/
	 inline void multColumn(T l, size_t j)
		{
        MTOOLS_ASSERT(j<N);
		if (l==1) {return;}
		for(size_t i=0;i<N;i++) {tab[i + N*j] = tab[i + N*j]*l;}
		}


     /**
      * Elementary operation: Multiply line L_i  <--  l*L_i
      *
      * @param  l   The scalar value to multiply the line with.
      * @param  i   index of the line.
      **/
	 inline void multLine(T l, size_t i)
		{
        MTOOLS_ASSERT(i<N);
        if (l==1) {return;}
		for(size_t j=0;j<N;j++) {tab[i + N*j] = tab[i + N*j]*l;}
		}


     /**
      * Elementary operation: Multiply & substract columns C_j1  <--  C_j1 - l*C_j2
      *
      * @param  l   The scalar value
      * @param  j1  index of the first column.
      * @param  j2  index of the second column.
      **/
	 inline void multSubColumn(T l, size_t j1, size_t j2)
		{
        MTOOLS_ASSERT((j1<N) && (j2<N));
		if (l == 0) {return;}
		for(size_t i=0;i<N;i++) {tab[i + N*j1] = tab[i + N*j1] - (tab[i + N*j2]*l);}
		}


     /**
      * Elementary operation: Multiply & substract columns L_i1  <--  L_i1 - l*L_i2
      *
      * @param  l   The scalar value
      * @param  i1  index of the first line.
      * @param  i2  index of the second line.
      **/
	 inline void multSubLine(T l, size_t i1,size_t i2)
		{
        MTOOLS_ASSERT((i1<N) && (i2<N));
		if (l == 0) {return;}
		for(size_t j=0;j<N;j++) {tab[i1 + N*j] = tab[i1 + N*j] - (tab[i2 + N*j]*l);}
		}


     /**
      * Compute the determinant of the matrix.
      *
      * @return the determinant.
      **/
     T det() const
		{
		T det = 1; // the determinant
		SqrMatrix<T,N> A((*this)); // auxiliary matrix = this object
		/* Elementary operations to reduce the matrix to an upper triangular matrix */
		for(size_t k=0;k<N;k++)
			{
			if (A(k,k) == 0) // swap needed if the diagonal coefficient is zero
				{
				size_t l = k+1;
				while(l<N)
					{
					if (A(l,k)!=0) 
						{
						A.exchangeLine(k,l);		// exchange the lines k and l
						det = -det;					// invert the sign of the determinant
						break;
						} 
					l++;
					}
				if (l==N) {return((T)0);}	// could not find a non-zero coefficient : the matrix is not invertible !
				}
			det *= A(k,k); // multiply the determinant by the diagonal coefficient
			A.multLine(1/A(k,k),k); // the coefficient A(k,k) is now equal to 1. 
			for(size_t l=k+1;l<N;l++) // set all the coeffcients below the diagonal to 0
				{
				A.multSubLine(A(l,k),l,k);  // L_l  <--  L_l - M_{l,k}L_k
				}
			}
		return det;
		}


     /**
      * Invert the matrix and returns its determinant. 
      *     - if the determinant is 0, then the object is left in an undetermined state !
      *     - if the determinant is not zero, then the object now contain the inverse matrix
      *
      * @return the determinant of the matrix.
      **/
	 T invert()
		{
		T det = 1;		  // the determinant
		SqrMatrix<T,N> M; // auxiliary matrix
		M.setIdentity();  // starting with the identity matrix
 		SqrMatrix<T,N> & A = (*this); // commodity reference
		T v; // auxilary variable
		/* Elementary operations to reduce the matrix to an upper triangular matrix with coefficients 1 on the diagonal */
		for(size_t k=0;k<N;k++)
			{
			if (A(k,k) == 0) // swap needed if the diagonal coefficient is zero
				{
                size_t l = k+1;
				while(l<N)
					{
					if (A(l,k)!=0) 
						{
						exchangeLine(k,l); M.exchangeLine(k,l);		// exchange the lines k and l
						det = -det;									// invert the sign of the determinant
						break;
						} 
					l++;
					}
				if (l==N) {return(0);}	// could not find a non-zero coefficient : the matrix is not invertible !
				}
			det *= A(k,k); // multiply the determinant by the diagonal coefficient
			v = 1/A(k,k);
			multLine(v,k); M.multLine(v,k); // the coefficient A(k,k) is now equal to 1. 
			for(size_t l=k+1;l<N;l++) // set all the coeffcients below the diagonal to 0
				{
				v = A(l,k);
				multSubLine(v,l,k); M.multSubLine(v,l,k);  // L_l  <--  L_l - M_{l,k}L_k
				}
			}
		/* Elementary operations to reduce the upper diagonal matrix to the identity */
		for(size_t k=0;k<(N-1);k++)
			{
			for(size_t l=0 ; l<(N-1-k); l++)
				{
				v = A(l,N-1-k);
				multSubLine(v,l,N-1-k); M.multSubLine(v,l,N-1-k);  // L_l  <--  L_l - M_{l,N-1-k}L_k
				}
			}
		/* we are done ! */
		A = M;	// set the matrix to its inverse
		return det; // and return the determinant
		}


     /**
     * transpose the matrix.
     **/
     inline void transpose()
		{
		 for(size_t i=1;i<N;i++)
			{
			 for(size_t j=0;j<i;j++)
				{
				T temp = (*this)(i,j); (*this)(i,j) = (*this)(j,i); (*this)(j,i) = temp;
				}
			}
		}


    /**
    * Multiply every coefficient of the matrix by a scalar.
    **/
     inline void operator*=(T v)
		{
		 for(size_t n=0;n<N*N;n++)	{tab[n] *= v;}
		}


    /**
    * Divide every coefficient of the matrix by a scalar.
    **/
	 inline void operator/=(T v)
		{
		for(size_t n=0;n<N*N;n++) {tab[n] /= v;}
		}


     /**
     * Add a matrix
     **/
     inline void operator+=(const SqrMatrix<T,N> & M)
		{
		 for(size_t i=0;i<N;i++)
			{
			 for(size_t j=0;j<N;j++)
				{
				(*this)(i,j) +=  M(i,j);
				}
			}
		}


     /**
     * Substract a matrix
     **/
     inline void operator-=(const SqrMatrix<T,N> & M)
		{
		 for(size_t i=0;i<N;i++)
			{
			 for(size_t j=0;j<N;j++)
				{
				(*this)(i,j) -= M(i,j);
				}
			}
		}


     /**
     * Multiply by a matrix (right multiplication) i.e. (THIS) = (THIS)*M
     **/
     inline void operator*=(const SqrMatrix<T,N> & M)
		{
		 (*this) = ((*this)*M);
		}


    /**
     * Print the matrix into a string. Use the toString funtion to convert the T elements into
     * strings.
     **/
	std::string toString() const
		{
		std::string s;
		for(size_t i=0;i<N;i++)
			{
			for(size_t j=0;j<N;j++) {s += mtools::toString((*this)(i,j)) + " \t";}
			s+= "\n";
			}
		return s;
		}


 private:

	T tab[N*N]; // the data of the matrix
 };


 /**
  * Transpose of a matrix.
  *
  * @param  M   The matrix
  *
  * @return The transposed matrix.
  **/
 template<class T, size_t N> inline SqrMatrix<T,N> transpose(const SqrMatrix<T,N> & M)
	{
	SqrMatrix<T,N> R;
	 for(size_t i=0;i<N;i++)
		{
		for(size_t j=0;j<N;j++)
			{
			R(i,j) = M(j,i);
			}
		}
	 return R;
	}


 /**
  * Multiply a matrix by a scalar
  *
  * @param  v   The scalar.
  * @param  M   The matrix
  *
  * @return the matrix vM.
  **/
 template<class T, size_t N> inline SqrMatrix<T,N> operator*(const T & v, const SqrMatrix<T,N> & M)
	 {
	 SqrMatrix<T,N> R;
	 for(size_t i=0;i<N;i++)
		{
		for(size_t j=0;j<N;j++)
			{
			R(i,j) = M(i,j)*v;
			}
		}
	 return R;
	}
 

 /**
  * (Right) Multiplication of a matrix with a vector.
  *
  * @param  M   The matrix.
  * @param  V   The vector.
  *
  * @return the vector MV.
  **/
 template<class T, size_t N> inline Vec<T,N> operator*(const SqrMatrix<T,N> & M, const Vec<T,N> & V)
	 {
	 Vec<T,N> W(0);
	 for(size_t i=0;i<N;i++)
		{
        for(size_t j=0;j<N;j++)
            {
			W[i] += ((M(i,j))*(V[j]));
			}
		}
	 return W;
	 }


 /**
  * (Left) Multiplication of a matrix with a vector.
  *
  * @param  M   The matrix.
  * @param  V   The vector.
  *
  * @return the vector (tV)M 
  **/
 template<class T,size_t N> inline Vec<T,N> operator*(const Vec<T, N>  & V, const SqrMatrix<T,N> & M)
	{	
	 Vec<T,N> W(0);
	 for(size_t j=0;j<N;j++)
		{
			for(size_t i=0;i<N;i++)
				{
				W[j] += ((V[i])*(M(i,j)));
				}
		}
	 return W;
	}


 /**
  * Multiplication  of two matrices.
  *
  * @param  M1  The first matrix.
  * @param  M2  The second matrix.
  *
  * @return The matrix M1.M2
  **/
 template<class T, size_t N> inline SqrMatrix<T,N> operator*(const SqrMatrix<T,N> & M1, const SqrMatrix<T,N> & M2)
	{	
	 SqrMatrix<T,N> R(0);
	 for(size_t j=0;j<N;j++)
		{
		for(size_t i=0;i<N;i++)
			{
			for(size_t k=0;k<N;k++)
				{
				R(i,j) += (M1(i,k)*M2(k,j));
				}
			}
		}
	return R;
	}


 /**
 * Addition of two matrices.
 *
 * @param  M1  The first matrix.
 * @param  M2  The second matrix.
 *
 * @return The matrix M1 + M2
 **/
template<class T, size_t N> inline SqrMatrix<T,N> operator+(const SqrMatrix<T,N> & M1, const SqrMatrix<T,N> & M2)
	{	
	 SqrMatrix<T,N> R;
	 for(size_t j=0;j<N;j++)
		{
		for(size_t i=0;i<N;i++)
			{
			R(i,j) = M1(i,j) + M2(i,j);
			}
		}
	return R;
	}


/**
* Substraction of two matrices.
*
* @param  M1  The first matrix.
* @param  M2  The second matrix.
*
* @return The matrix M1 - M2
**/
template<class T, size_t N> inline SqrMatrix<T,N> operator-(const SqrMatrix<T,N> & M1, const SqrMatrix<T,N> & M2)
	{	
	 SqrMatrix<T,N> R;
	 for(size_t j=0;j<N;j++)
		{
		for(size_t i=0;i<N;i++)
			{
			R(i,j) = M1(i,j) - M2(i,j);
			}
		}
	return R;
	}



}


/* end of file */


