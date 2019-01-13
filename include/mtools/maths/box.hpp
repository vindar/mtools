/** @file box.hpp */
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
#include "../misc/error.hpp"
#include "../misc/misc.hpp"
#include "../misc/stringfct.hpp"
#include "vec.hpp"

#include <string>
#include <ostream>

// these undefs are needed when using cotire pre-compiled headers
#undef min
#undef max

namespace mtools
    {
 

	/* Definition of constants use for the split() method*/
	
	/* Split in half */
	const int BOX_SPLIT_UP		= 0;
	const int BOX_SPLIT_DOWN	= 1;
	const int BOX_SPLIT_LEFT	= 2;
	const int BOX_SPLIT_RIGHT	= 3;
	
	/* Split in quarter*/
	const int BOX_SPLIT_UP_LEFT = 4;
	const int BOX_SPLIT_UP_RIGHT = 5;
	const int BOX_SPLIT_DOWN_LEFT = 6;
	const int BOX_SPLIT_DOWN_RIGHT = 7;


    // forward declaration
    template<typename T, size_t N> class Box;


    /**
    * integer valued box
    **/
    template<size_t N> using iBox = Box<int64, N>;


    /**
    * floating point valued box
    **/
    template<size_t N> using fBox = Box<double, N>;


    /**
    * 2-dim integer valued box
    **/
    typedef iBox<2> iBox2;


    /**
    * 3-dim integer valued box
    **/
    typedef iBox<3> iBox3;


    /**
    * 2-dim float-valued box
    **/
    typedef fBox<2> fBox2;


    /**
    * 3-dim float-valued box
    **/
    typedef fBox<3> fBox3;



    /**
    * Class representing a N dimensional box.
    **/
    template<typename T, size_t N> class Box
        {

        public:


            /**
            * Default constructor. Create a completely empty box.
            **/
            Box() : min(1), max(0)  { }


            /**
            * Constructor from min and max points.
            **/
            Box(const Vec<T, N> & minVec, const Vec<T, N> & maxVec, bool reorderIfNeeded) : min(minVec), max(maxVec)
                {
                if (reorderIfNeeded)
                    {
                    for (size_t i = 0; i < N; i++) { if (min[i] > max[i]) { auto temp = min[i]; min[i] = max[i]; max[i] = temp; } }
                    }
                }


			/**
			 * Constructor from a single point. 
			 **/
			Box(const Vec<T, N> & vec) : min(vec), max(vec) {}


            /**
            * Constructor. Specific for dimension 2.
            **/
            Box(const T & xmin, const T & xmax, const T & ymin, const T & ymax) : min(xmin,ymin) , max(xmax,ymax)
                {
                static_assert(N == 2, "dimension must be equal to 2");
                }

			/**
			* Constructor. Specific for dimension 2.
			**/
			Box(const T & xmin, const T & xmax, const T & ymin, const T & ymax, bool reorderifneeded) : min(xmin, ymin), max(xmax, ymax)
				{
				static_assert(N == 2, "dimension must be equal to 2");
				if (reorderifneeded)
					{
					if (min[0] > max[0]) mtools::swap(min[0], max[0]);
					if (min[1] > max[1]) mtools::swap(min[1], max[1]);
					}
				}



           /**
            * Default copy constructor.
            **/
            Box(const Box & B) = default;


            /**
            * Copy constructor from another template parameter.
            **/
            template<typename U> explicit Box(const Box<U,N> & B) : min((Vec<T,N>)B.min), max((Vec<T, N>)B.max) {}


            /**
            * Default assignment operator.
            **/
            Box & operator=(const Box & B) = default;


            /**
            * Assignment operator from another type.
            **/
            //template<typename U> Box & operator=(const Box<U,N> & B) { min = B.min; max = B.max; return(*this); }


            /**
            * Default destructor.
            **/
            ~Box() = default;


            /**
            * Query if the rectangle is empty i.e. if at least a length in a direction is strictly negative.
            *
            * @return  true if empty, false if not.
            **/
            inline bool isEmpty() const { for (size_t i = 0; i < N; i++) { if (min[i] > max[i]) return true; } return false; }


            /**
            * Queries if the rectangle if "horizontally" empty (ie in the first dimension)
            *
            * @return  true if horizontally empty, false if not.
            **/
            inline bool isHorizontallyEmpty() const { return(max[0] < min[0]); }


            /**
            * Queries if the rectangle if "vertically" empty (ie in the second dimension)
            *
            * @return  true if vertically empty, false if not.
            **/
            inline bool isVerticallyEmpty() const { static_assert(N >= 2, "dimension N must be at least 2"); return(max[1] < min[1]); }


            /**
            * Queries if the rectangle is empty in every directions.
            *
            * @return  true if completely empty, false if not.
            **/
            inline bool isCompletelyEmpty() const { for (size_t i = 0; i < N; i++) { if (min[i] <= max[i]) return false; } return true; }


            /**
            * Query if the rectangle is reduced to a single point.
            *
            * @return  true if it is a signle point, false if not.
            **/
            inline bool isPoint() const { return (min == max); }


            /**
            * Set the rectangle as completely empty (in every directions).
            **/
            inline void clear() { min = 1; max = 0; }


            /**
            * Set the rectangle as horizontally empty (ie coord with index 0).
            **/
            inline void clearHorizontally() { min[0] = 1; max[0] = 0; }


            /**
            * Set the rectangle as vertically empty (ie coord with index 1).
            **/
            inline void clearVertically() { static_assert(N >= 2, "dimension N must be at least 2"); min[1] = 1; max[1] = 0; }


            /**
            * Query if a point is inside the closed box.
            *
            * @param   pos The position to check
            *
            * @return  true if the point is in the closed box and false otherwise
            **/
            inline bool isInside(const Vec<T, N> & pos) const 
                { 
                for (size_t i = 0; i < N; i++)
                    {
                    if ((pos[i] < min[i]) || (max[i] < pos[i])) return false;
                    }
                return true; 
                }


            /**
            * Query if a point is inside the open box.
            *
            * @param   pos The position to check
            *
            * @return  true if the point is in the closed box and false otherwise
            **/
            inline bool isStrictlyInside(const Vec<T, N> & pos) const
                {
                for (size_t i = 0; i < N; i++)
                    {
                    if ((pos[i] <= min[i]) || (max[i] <= pos[i])) return false;
                    }
                return true;
                }


            /**
            * Enlarge the rectangle in order to contain a given point. If the rectangle already contain the
            * point do nothing.
            *
            * @param   pos The position of the point to swallow.
            *
            * @return  true if the point was swallowed and false if it was already contained in the rectangle.
            **/
            inline bool swallowPoint(const Vec<T, N> & pos)
                {
				if (isEmpty())
					{
					min = pos;
					max = pos;
					return true;
					}
                bool b = false;
                for (size_t i = 0; i < N; i++)
                    {
                    if (pos[i] < min[i]) { min[i] = pos[i]; b = true; }
                    if (pos[i] > max[i]) { max[i] = pos[i]; b = true; }
                    }
                return b;
                }


            /**
            * Enlarge the rectangle in order to contain a given box. If the rectangle already contain the
            * box do nothing.
            *
            * @param   B    The box to swallow.
            *
            * @return  true if the box was really swallowed and false if it was already contained in the rectangle.
            **/
            inline bool swallowBox(const Box<T,N> & B)
                {
				if (B.isEmpty()) return false;
				if (isEmpty()) { *this = B; return true; }
                bool b = false;
                for (size_t i = 0; i < N; i++)
                    {
                    if (B.min[i] < min[i]) { min[i] = B.min[i]; b = true; }
                    if (B.max[i] > max[i]) { max[i] = B.max[i]; b = true; }
                    }
                return b;
                }


			/**
			 * Move each boundary of the box by a given offset. 
			 * an empty box may become not empty after this operation...
			 *
			 * @param	offset	The offset. positive to enlarge and negative to reduce the margin.
			 **/
			inline void enlarge(T offset)
				{
				for (size_t i = 0; i < N; i++)
					{
					min[i] -= offset;
					max[i] += offset;
					}
				}


			/**
			* Return a new box obtained from this box by enlarging in each direction by a given amount.
			* an empty box may become not empty after this operation...
			*
			* @param	offset	The offset. positive to enlarge and negative to reduce the margin.
			**/
			Box getEnlarge(T offset) const
				{
				Box B(*this);
				for (size_t i = 0; i < N; i++)
					{
					B.min[i] -= offset;
					B.max[i] += offset;
					}
				return B;
				}



            /**
            * Try to enlarge the rectangle using points from another rectangle if possible. By definition,
            * the resulting rectangle contain the initial one and is included in the union of the initial
            * one and the source R used to enlarge.
            *
            * @warning Both rectangles should not be empty.
            *
            * @param   B   the rectagle to use for the enlargment.
            **/
            inline void enlargeWith(const Box & B)
                {
                static_assert(N == 2, "Implemented only for dimension N=2 yet...");
                const bool containV = ((B.min[0] <= min[0]) && (max[0] <= B.max[0])) ? true : false;
                const bool containH = ((B.min[1] <= min[1]) && (max[1] <= B.max[1])) ? true : false;
                if ((!containV) && (!containH)) return;    // nothing to do
                if (containV && containH) { *this = B; return; } // initial rectangle is included in R so we set to R
                if (containV)
                    { // try to improve horizontally
                    if ((B.max[1] > max[1]) && (B.min[1] <= max[1])) { max[1] = B.max[1]; }
                    if ((B.min[1] < min[1]) && (B.max[1] >= min[1])) { min[1] = B.min[1]; }
                    return;
                    }
                // try to improve vertically
                if ((B.max[0] > max[0]) && (B.min[0] <= max[0])) { max[0] = B.max[0]; }
                if ((B.min[0] < min[0]) && (B.max[0] >= min[0])) { min[0] = B.min[0]; }
                }


			/**
			* Split the box in half along a given dimension, keeping eiter the lower or upper part.
			*
			* @param	dim			The dimension to split along.
			* @param	keepup   	True to keep the upper part and flalse to keep the lower part. 
			**/
			inline void split(size_t dim, bool keepup)
				{
				MTOOLS_ASSERT(dim < N);
				*this = get_split(dim, keepup);
				}


			/**
			* Split the box in half along a given dimension, keeping eiter the lower or upper part.
			*
			* @param	dim			The dimension to split along.
			* @param	keepup   	True to keep the upper part and flalse to keep the lower part.
			* 						
			* @return  corresponding half boxes.
			**/
			inline Box<T, N> get_split(size_t dim, bool keepup) const
				{
				MTOOLS_ASSERT(dim < N);
				Box<T, N> B(*this);
				if (keepup) B.min[dim] = (B.min[dim] + B.max[dim]) / 2; else B.max[dim] = (B.min[dim] + B.max[dim]) / 2;
				return B;
				}


			/**
			* Split the box in half or in quarter
			* Only for dimension 2 
			* 
			* @param   part Part of the box to keep, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT
			**/
			inline void split(int part)
				{
				static_assert(N == 2, "this version of split() in specific to dimension 2");
				*this = get_split(part);
				}


			/**
			* Split the box in half or in quarter
			* Only for dimension 2
			*
			* @param   part Part of the box to keep, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			*
			* @return  corresponding half boxes.
			**/
			inline Box<T, N> get_split(int part) const
				{
				static_assert(N == 2, "this version of get_split() in specific to dimension 2");
				switch (part)
					{
					case BOX_SPLIT_UP:		   { return get_split(1, true); }
					case BOX_SPLIT_DOWN:	   { return get_split(1, false); }
					case BOX_SPLIT_LEFT:	   { return get_split(0, false); }
					case BOX_SPLIT_RIGHT:	   { return get_split(0, true); }
					case BOX_SPLIT_UP_LEFT:    { return get_split(1, true).get_split(0, false); }
					case BOX_SPLIT_UP_RIGHT:   { return get_split(1, true).get_split(0, true); }
					case BOX_SPLIT_DOWN_LEFT:  { return get_split(1, false).get_split(0, false); }
					case BOX_SPLIT_DOWN_RIGHT: { return get_split(1, false).get_split(0, true); }
					default: { MTOOLS_ERROR("incorrect splitting part."); }
					}
				return *this;
				}


            /**
            * Compute the distance of a point inside the rectangle to its boundary
            *
            * @param   pos The position.
            *
            * @return  the distance between the point and the boundary or a negative value if the point is not inside the rectangle.
            **/
            inline T boundaryDist(const Vec<T, N> & pos) const
                {
                T l = max[0] - pos[0]; l = std::min<T>(l, pos[0] - min[0]);
                for (size_t i = 1; i < N; i++) { l = std::min<T>(std::min<T>(l , max[i] - pos[i]), pos[i] - min[i]); }
                return l;
                }


            /**
            * Compute the center of the rectangle. The position returned has no meaning if the rectangle is
            * empty.
            *
            * @return  The position of the center.
            **/
            inline Vec<T, N> center() const { Vec<T, N> V; for (size_t i = 0; i < N; i++) { V[i] = (min[i] + max[i])/2; } return V; }


            /**
            * Return the length in direction i. May be negatice if empty.
			* this is the same as max[i] - min[i].
            **/
            inline T l(size_t i) const { return(max[i] - min[i]); }


            /**
            * Return the width : lenght in direction 0.
            **/
            inline T lx() const { return std::max<T>(0, max[0] - min[0]); }

            
            /**
            * Return the height : lenght in direction 1.
            **/
            inline T ly() const { static_assert(N >= 2, "dimension N must be at least 2..."); return std::max<T>(0, max[1] - min[1]); }


            /**
            * Return the smallest of both length lx() and ly().
            **/
            inline T minlxy() const { return std::min(lx(), ly()); }


            /**
            * Return the largest of both length lx() and ly().
            **/
            inline T maxlxy() const { return std::max(lx(), ly()); }


            /**
            * Equality operator.
            *
            * @param   B   The rectangle to compare with.
            *
            * @return  true if both rectangle are either equal or both empty.
            **/
            inline bool operator==(const Box & B) const 
                {
                return(((isEmpty()) && (B.isEmpty())) || ((min == B.min) && (max == B.max))); 
                }


            /**
            * Inequality operator.
            *
            * @param   B   The rectangle to compare with.
            *
            * @return  true they are different (two empty rectangles are considered equal).
            **/
            inline bool operator!=(const Box & B) const { return(!(operator==(B))); }


            /**
            * Less-than-or-equal comparison operator. 
            * Check if the rectangle is included in another rectangle (for the inclusion partial order).
            *
            * @param   B   The rectangle to check if it contains this.
            *
            * @return  true if R contains this, false otherwise. An empty rectangle contains nothing but is
            *          contained in every non-empty rectangle.
            **/
            bool operator<=(const Box & B) const
                {
                if (B.isEmpty()) return false;
                if (isEmpty()) return true;
                for (size_t i = 0; i < N; i++)
                    {
                    if ((min[i] < B.min[i]) || (B.max[i] < max[i])) return false;
                    }
                return true;
                }


            /**
            * Greater-than-or-equal comparison operator. 
            * Check if the rectangle contains another rectangle (for the inclusion partial order).
            *
            * @param   B   The rectangle to check if it is contained in this.
            *
            * @return  true if R is contained in this, false otherwise. An empty rectangle contains nothing
            *          but is contained in every non empty rectangle.
            **/
            bool operator>=(const Box & B) const { return(B.operator<=(*this)); }


            /**
            * Print the rectangle into a std::string.
            *
            * @return  a string in the form "[X1,X2]x[Y1,Y2]x...".
            **/
            std::string toString() const 
                { 
				OSS os; 
                os <<  (isEmpty() ? "(empty)" : "");
                for (size_t i = 0; i < N; i++)
                    {
                    if (i != 0) { os <<  "x"; }
                    os << "[" << min[i] <<  ","  << max[i] << "]";
                    }
                return os.str();
                }


            /**
            * Return the subrectangle corresponding to the intersection of *this and B. The returned rectangle
            * is "seen" relatively to *this. If either B or this is empty, the returned rectangle is empty.
            *
            * @param   B   The rectangle to intersect
            *
            * @return  The intersection rectangle seen as a sub rectangle of *this.
            **/
            Box relativeSubRect(const Box & B) const
                {
                if (isEmpty() || B.isEmpty()) { return Box(); }
                Box S;
                for (size_t i = 0; i < N; i++)
                    {
                    S.min[i] = std::max<T>(min[i], B.min[i]) - min[i];
                    S.max[i] = std::min<T>(max[i], B.max[i]) - min[i];
                    }
                return S;
                }


            /**
            * Returns the area inside the rectangle. Do not check that the rectangle is non-empty.
            *
            * @return  The area is 0 if the rectangle is empty or flat.
            **/
            inline T area() const
                {
                T a = 1;
                for (size_t i = 0; i < N; i++) { a *= (max[i] - min[i]); }
                return a;
                }
                

            /**
             * Returns the area of the intersection of the rectangle with the square [x-0.5,x+0.5]*[y-0.5,y+0.5]*...
             * 
             * @param   pos   Position to check.
             * 
             * @return  The area of the intersection.
             **/
            inline double pointArea(const Vec<T,N> & pos) const
                {
                double a = std::min<double>(max[0], pos[0] + 0.5) - std::max<double>(min[0], pos[0] - 0.5); if (a <= 0.0) return 0.0;
                for (size_t i = 1; i < N; i++)
                    {
                    a *= std::min<double>(max[i], pos[i] + 0.5) - std::max<double>(min[i], pos[i] - 0.5); if (a <= 0.0) return 0.0;
                    }
                return a;
                }


            /**
            * Intersect the box with another one. Return true if the box was modified.
            **/
            inline bool intersectionBox(const Box<T, N> & B)
                {
                bool b = false;
                for (size_t i = 0; i < N; i++)
                    {
                    if (B.min[i] > min[i]) { min[i] = B.min[i]; b = true; }
                    if (B.max[i] < max[i]) { max[i] = B.max[i]; b = true; }
                    }
                return b;
                }


			/**
			 * Return true if this box contain the box B. 
			 * An empty B is contained in any box (even an empty one). 
			 */
			inline bool contain(const Box<T,N> & B) const
				{
				if (B.isEmpty()) return true;
				if (isEmpty()) return false;
				for (size_t i = 0; i < N; i++)
					{
					if ((B.min[i] < min[i]) || (B.max[i] > max[i])) { return false; }
					}
				return true;
				}


			/**
			* Return true if this box is contained into the box B.
			* An empty box is contained in any box (even an empty one).
			*/
			inline bool isIncludedIn(const Box<T, N> & B) const
				{
				return B.contain(*this);
				}


            /**
            * Returns an integer box containing all the points (i,j) such that the unit rectangle
            * [i-0.5,i+0.5]*[j-0.5,j+0.5] intersects the rectangle.
            **/
            inline iBox<N> integerEnclosingRect() const 
                { 
                iBox<N> B;
                for (size_t i = 0; i < N; i++)
                    {
                    B.min[i] = (int64)floor(min[i] + 0.5);
                    B.max[i] = (int64)ceil(max[i] - 0.5);
                    }
                return B;
                }


			/**
			* Returns an integer box that caontina this box, obtained by taking the floor() 
			* for min[] and the ceil() for max().
			* 
			* This box is larger than that returned by integerEnclosingRect().
			**/
			inline iBox<N> integerEnclosingRect_larger() const
				{
				iBox<N> B;
				for (size_t i = 0; i < N; i++)
					{
					B.min[i] = (int64)floor(min[i]);
					B.max[i] = (int64)ceil(max[i]);
					}
				return B;
				}


            /**
            * Returns the minimal centered enclosing rectangle with a given aspect ratio.
            *
            * @param   lxperly The desired ratio lx/ly.
            *
            * @return  the enclosing rectangle with this ratio.
            **/
            inline fBox2 fixedRatioEnclosingRect(double lxperly) const
                {
                static_assert(N == 2, "dimension N must be exactly 2.");
                const double lx = (double)(max[0] - min[0]);
                const double ly = (double)(max[1] - min[1]);
                if ((lx <= 0.0) || (ly <= 0.0)) return fBox2();
                const double rat = lx/ly;
                if (rat < lxperly) { return fBox2( ((double)(min[0] + max[0]))/2.0 - ly*lxperly/2.0, ((double)(min[0] + max[0]))/2.0 + ly*lxperly/2.0, (double)min[1], (double)max[1]); }
                return fBox2( (double)min[0], (double)max[0], ((double)(min[1] + max[1]))/2.0 - (lx/lxperly)/2.0, ((double)(min[1] + max[1]))/2.0 + (lx/lxperly)/2.0);
                }


            /**
            * Returns the maximal centered enclosed rectangle with a given aspect ratio.
            *
            * @param   lxperly The desired ratio lx/ly.
            *
            * @return  the enclosed rectangle with this ratio.
            **/
            inline fBox2 fixedRatioEnclosedRect(double lxperly) const
                {
                static_assert(N == 2, "dimension N must be exactly 2.");
                const double lx = (double)(max[0] - min[0]);
                const double ly = (double)(max[1] - min[1]);
                if ((lx <= 0.0) || (ly <= 0.0)) return fBox2();
                const double rat = lx/ly;
                if (rat < lxperly) { return fBox2((double)min[0], (double)max[0], ((double)(min[1] + max[1]))/2.0 - (lx/lxperly)/2.0, ((double)(min[1] + max[1]))/2.0 + (lx/lxperly)/2.0); }
                return fBox2(((double)(min[0] + max[0]))/2.0 - ly*lxperly/2.0, ((double)(min[0] + max[0])) / 2.0 + ly*lxperly/2.0, (double)min[1], (double)max[1]);
                }


			/**
			 * Converts a lenght from absolute to pixel lenght (in the x direction).
			 */
			inline int64 absToPixel_lenghtX(double dx, const iVec2 & scrSize) const
				{
				static_assert(N == 2, "dimension N must be exactly 2.");
				const double lx = (double)(max[0] - min[0]);
				MTOOLS_ASSERT(lx > 0.0);
				double x = floor( (dx/lx)*scrSize.X() + 0.5); 
				if (x < -2000000000) { x = -2000000000; } else if (x > +2000000000) { x = +2000000000; } // dirty, for overflow
				return (int64)x;
				}


			/**
			* Converts a lenght from absolute to pixel lenght (in the y direction).
			*/
			inline int64 absToPixel_lenghtY(double dy, const iVec2 & scrSize) const
				{
				static_assert(N == 2, "dimension N must be exactly 2.");
				const double ly = (double)(max[1] - min[1]);
				MTOOLS_ASSERT(ly > 0.0);
				double y = floor((dy / ly)*scrSize.Y() + 0.5);
				if (y < -2000000000) { y = -2000000000; } else if (y > +2000000000) { y = +2000000000; } // dirty, for overflow
				return (int64)y;
				}


            /**
            * Converts an absolute position into its associated position inside a screen. (and invert the Y
            * axis).
            *
            * @param   absCoord    the position (x,y) to convert.
            * @param   scrSize     Size of the screen in pixels (lx,ly).
            *
            * @return  An iVec2 containing the the coordinate the pixel in the screen. No clipping, The
            *          returned value may be outside of [0,lx]x[0,ly].
            **/
			inline iVec2 absToPixel(const fVec2 & absCoord, const iVec2 & scrSize) const
                {
                static_assert(N == 2, "dimension N must be exactly 2.");
                const double lx = (double)(max[0] - min[0]);
                const double ly = (double)(max[1] - min[1]);
                MTOOLS_ASSERT((lx > 0.0) && (ly > 0.0));
				const int64 L = 2000000000;
                double x = std::floor((((absCoord.X() - min[0]) / lx)*scrSize.X()) + 0.5); if (x < -L) { x = -L; } else if (x > L) { x = L; } // dirty, for overflow
                double y = std::floor((((absCoord.Y() - min[1]) / ly)*scrSize.Y()) + 0.5); if (y < -L) { y = -L; } else if (y > L) { y = L; } // dirty, for overflow
                return iVec2((int64)x, scrSize.Y() - 1 - (int64)(y));
                }

			/**
			* Converts an absolute position into its associated position inside a screen.
			* in normalized coordinate [_0.5 , lx - 0.5] x [-0.5, ly - 0.5]
			*
			* Same as above but return an real valued position. 
			**/
			inline fVec2 absToPixelf(const fVec2 & absCoord, const iVec2 & scrSize) const
				{
				static_assert(N == 2, "dimension N must be exactly 2.");
				const double lx = (double)(max[0] - min[0]);
				const double ly = (double)(max[1] - min[1]);
				MTOOLS_ASSERT((lx > 0.0) && (ly > 0.0));
				double x = ((absCoord.X() - min[0]) / lx)*scrSize.X() - 0.5;
				double y = ((absCoord.Y() - min[1]) / ly)*scrSize.Y() - 0.5;
				return fVec2(x, scrSize.Y() - 1 - y);
				}


			/**
			* Converts a box from absolute position into its associated position inside a screen.
			*
			 * @param	absBox		the box in aboslute position.
			 * @param   scrSize     Size of the screen in pixels (lx,ly).
			 *
			 * @return	the corresponding box in pixel units. 
			 */
			inline iBox2 absToPixel(const fBox2 & absBox, const iVec2 & scrSize) const
				{
				static_assert(N == 2, "dimension N must be exactly 2.");
				const iVec2 Pmin = absToPixel({absBox.min[0], absBox.max[1]}, scrSize);
				const iVec2 Pmax = absToPixel({absBox.max[0], absBox.min[1]}, scrSize);
				return iBox2(Pmin.X(), Pmax.X(), Pmin.Y(), Pmax.Y());				
				}


			/**
			* Converts a box from absolute position into its associated position 
			* in the normalized coord of the screen [_0.5 , lx - 0.5] x [-0.5, ly - 0.5]
			*
			* @param	absBox		the box in absolute position.
			* @param   scrSize     Size of the screen in pixels (lx,ly).
			*
			* @return	the corresponding box in pixel units.
			*/
			inline fBox2 absToPixelf(const fBox2 & absBox, const iVec2 & scrSize) const
				{
				static_assert(N == 2, "dimension N must be exactly 2.");
				const fVec2 Pmin = absToPixelf({ absBox.min[0], absBox.max[1] }, scrSize);
				const fVec2 Pmax = absToPixelf({ absBox.max[0], absBox.min[1] }, scrSize);
				return fBox2(Pmin.X(), Pmax.X(), Pmin.Y(), Pmax.Y());
				}


            /**
            * Convert a position inside a screen into an absolute position.
            *
            * @param   pixCoord    The pixel coordinate (i,j)
            * @param   scrSize     Size of the screen in pixel (lx,ly).
            *
            * @return  A fVec2 containing the absolute position of this pixel.
            **/
			inline fVec2 pixelToAbs(const iVec2 & pixCoord, const iVec2 & scrSize) const
                {
                static_assert(N == 2, "dimension N must be exactly 2.");
                const double lx = (double)(max[0] - min[0]);
                const double ly = (double)(max[1] - min[1]);
                MTOOLS_ASSERT((lx > 0.0) && (ly > 0.0));
                const double x = min[0] + lx*((double)(2 * pixCoord.X() + 1) / ((double)(2 * scrSize.X())));
                const double y = min[1] + ly*((double)(2 * (scrSize.Y() - 1 - pixCoord.Y()) + 1) / ((double)(2 * scrSize.Y())));
                return fVec2(x, y);
                }


            /**
            * Serializes / deserialize the object. Compatible with boost and with the custom serialization
            * classes OBaseArchive and IBaseArchive. The method performs both serialization and deserialization.
            **/
            template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0)
                {
                ar & min;
                ar & max;
                }

        Vec<T, N> min; // vector with the min coordinates in each directions
        Vec<T, N> max; // vector with the max coordinates in each directions

        };



	/**
	* Convert an fBox to iBox using std::round() instead of truncation to get the closest integer.
	**/
	template<size_t N> MTOOLS_FORCEINLINE iBox<N> round(const fBox<N> & V)
		{			
		return iBox<N>(round(V.min), round(V.max), false);
		}


    /**
    * Zoom inside the rectangle (reduce the radius by 1/10th).
    **/
    template<typename T, size_t N> inline Box<T,N> zoomIn(Box<T,N> B)
        {
        for (size_t i = 0; i < N; i++)
            {
            T l = (B.max[i] - B.min[i])/10;
            B.min[i] += l; B.max[i] -= l;
            }
        return B;
        }


    /**
    * Zoom outside of the rectangle (increase radius by 1/8th).
    **/
    template<typename T, size_t N> inline Box<T,N> zoomOut(Box<T,N> B)
        {
        for (size_t i = 0; i < N; i++)
            {
            T l = (B.max[i] - B.min[i])/8;
            B.min[i] -= l; B.max[i] += l;
            }
        return B;
        }


    /**
    * Move the rectangle left by 1/20th of its length (coord of index 0)
    **/
    template<typename T, size_t N> inline Box<T, N> left(Box<T, N> B)
        {
        T off = (B.max[0] - B.min[0])/20;
        B.min[0] -= off; B.max[0] -= off;
        return B;
        }


    /**
    * Move the rectangle right by 1/20th of its length (coord of index 0)
    **/
    template<typename T, size_t N> inline Box<T, N> right(Box<T, N> B)
        {
        T off = (B.max[0] - B.min[0])/20;
        B.min[0] += off; B.max[0] += off;
        return B;
        }


    /**
    * Move the rectangle up by 1/20th of its height (coord of index 1)
    **/
    template<typename T, size_t N> inline Box<T, N> up(Box<T, N> B)
        {
        static_assert(N >= 2, "dimension N must be at least 2.");
        T off = (B.max[1] - B.min[1])/20;
        B.min[1] += off; B.max[1] += off;
        return B;
        }


    /**
    * Move the rectangle down by 1/20th of its height (coord of index 1)
    **/
    template<typename T, size_t N> inline Box<T, N> down(Box<T, N> B)
        {
        static_assert(N >= 2, "dimension N must be at least 2.");
        T off = (B.max[1] - B.min[1])/20;
        B.min[1] -= off; B.max[1] -= off;
        return B;
        }


    /**
    * The rectangle obtained as the intersection of two rectangles. May be an empty rectangle.
    **/
    template<typename T, size_t N> inline Box<T,N> intersectionRect(const Box<T,N> & B1, const Box<T, N> & B2)
        { 
        Box<T, N> S;
        for (size_t i = 0; i < N; i++)
            {
            S.min[i] = std::max<T>(B1.min[i], B2.min[i]);
            S.max[i] = std::min<T>(B1.max[i], B2.max[i]);
            }
        return S;
        }


    /**
    * The rectangle obtained as the smallest rectangle containing B1 and B2.
    **/
    template<typename T, size_t N> inline Box<T, N> unionRect(const Box<T, N> & B1, const Box<T, N> & B2)
        {
        if (B1.isEmpty()) { return B2; }
        if (B2.isEmpty()) { return B1; }
        Box<T, N> S;
        for (size_t i = 0; i < N; i++)
            {
            S.min[i] = std::min<T>(B1.min[i], B2.min[i]);
            S.max[i] = std::max<T>(B1.max[i], B2.max[i]);
            }
        return S;
        }



	/**
	* Test if a (double valued) box has integer coordinates.
	*/
	template<size_t N> MTOOLS_FORCEINLINE bool isIntegerValued(const Box<double, N> & B)
		{
		for (size_t n = 0; n < N; n++)
			{
			if (std::round(B.min[n]) != B.min[n]) return false;
			if (std::round(B.max[n]) != B.max[n]) return false;
			}
		return true;
		}
 


	/**
	* Map a distance on the x-axis to the distance after the affine transformation that maps src_box to dst_box.
	*
	* @param	dx	initial distance along the x axis.
	* @param	src_box Source box.
	* @param	dst_box Destination box.
	*
	* @return	new distance along the x axis after applying the affine transformation.
	*/
	MTOOLS_FORCEINLINE double boxTransform_dx(double dx, const fBox2 & src_box, const fBox2 & dst_box)
		{
		MTOOLS_ASSERT((dst_box.max[0] - dst_box.min[0]) > 0);
		MTOOLS_ASSERT((src_box.max[0] - src_box.min[0]) > 0);
		return ((dst_box.max[0] - dst_box.min[0]) / (src_box.max[0] - src_box.min[0]))*dx;
		}

	/**
	* Map a distance on the y-axis to the distance after the affine transformation that maps src_box to dst_box.
	*
	* @param	dy	initial distance along the y axis.
	* @param	src_box Source box.
	* @param	dst_box Destination box.
	*
	* @return	new distance along the y axis after applying the affine transformation.
	*/
	MTOOLS_FORCEINLINE double boxTransform_dy(double dy, const fBox2 & src_box, const fBox2 & dst_box)
		{
		MTOOLS_ASSERT((dst_box.max[1] - dst_box.min[1]) > 0);
		MTOOLS_ASSERT((src_box.max[1] - src_box.min[1]) > 0);
		return ((dst_box.max[1] - dst_box.min[1]) / (src_box.max[1] - src_box.min[1]))*dy;
		}

	/**
	* Map a vector (not a position!) after the affine transformation that maps src_box to dst_box.
	*
	* @param	vec	the vector to transform
	* @param	src_box Source box.
	* @param	dst_box Destination box.
	*
	* @return	new distance along the y axis after applying the affine transformation.
	*/
	MTOOLS_FORCEINLINE fVec2 boxTransform_dx_dy(const fVec2 & vec, const fBox2 & src_box, const fBox2 & dst_box)
		{
		MTOOLS_ASSERT((dst_box.max[1] - dst_box.min[1]) > 0);
		MTOOLS_ASSERT((dst_box.max[0] - dst_box.min[0]) > 0);
		MTOOLS_ASSERT((src_box.max[1] - src_box.min[1]) > 0);
		MTOOLS_ASSERT((src_box.max[0] - src_box.min[0]) > 0);
		return fVec2( ((dst_box.max[0] - dst_box.min[0]) / (src_box.max[0] - src_box.min[0]))*vec.X(),
		              ((dst_box.max[1] - dst_box.min[1]) / (src_box.max[1] - src_box.min[1]))*vec.Y());
		}


	/**
	* Map the position of a point when the source box mapped to the destination box by an affine transformation.
	*
	* template parameter selects whether the y-axis is reversed
	*
	* @param	src_pos Source position.
	* @param	src_box Source box.
	* @param	dst_box Destination box.
	*
	* @return	the corresponding position after the affine tranformation.
	*/
	template<bool reverse_y = true> MTOOLS_FORCEINLINE fVec2 boxTransform(const fVec2 & src_pos, const fBox2 & src_box, const fBox2 & dst_box)
		{
		MTOOLS_ASSERT((dst_box.max[0] - dst_box.min[0]) > 0);
		MTOOLS_ASSERT((src_box.max[0] - src_box.min[0]) > 0);
		MTOOLS_ASSERT((dst_box.max[1] - dst_box.min[1]) > 0);
		MTOOLS_ASSERT((src_box.max[1] - src_box.min[1]) > 0);
		const double mx = (dst_box.max[0] - dst_box.min[0]) / (src_box.max[0] - src_box.min[0]);
		const double my = (dst_box.max[1] - dst_box.min[1]) / (src_box.max[1] - src_box.min[1]);
		return fVec2 ( dst_box.min[0] + mx*(src_pos.X() - src_box.min[0]),
					   (reverse_y ? (dst_box.max[1] - my * (src_pos.Y() - src_box.min[1]))
			                      : (dst_box.min[1] + my * (src_pos.Y() - src_box.min[1]))) );
		}

	/**
	* Map a box to another one using the affine trandformation that maps src_box to dst_box.
	*
	* template parameter selects whether the y-axis is reversed 
	*
	* @param	box		box to transform
	* @param	src_box Source box.
	* @param	dst_box Destination box.
	*
	* @return	the corresponding position after the affine tranformation.
	*/
	template<bool reverse_y = true> MTOOLS_FORCEINLINE fBox2 boxTransform(const fBox2 & box, const fBox2 & src_box, const fBox2 & dst_box)
		{
		MTOOLS_ASSERT((dst_box.max[0] - dst_box.min[0]) > 0);
		MTOOLS_ASSERT((src_box.max[0] - src_box.min[0]) > 0);
		MTOOLS_ASSERT((dst_box.max[1] - dst_box.min[1]) > 0);
		MTOOLS_ASSERT((src_box.max[1] - src_box.min[1]) > 0);
		const double mx = (dst_box.max[0] - dst_box.min[0]) / (src_box.max[0] - src_box.min[0]);
		const double my = (dst_box.max[1] - dst_box.min[1]) / (src_box.max[1] - src_box.min[1]);
		return fBox2(dst_box.min[0] + mx * (box.min[0] - src_box.min[0]),
				     dst_box.min[0] + mx * (box.max[0] - src_box.min[0]),
					 (reverse_y ? (dst_box.max[1] - my * (box.max[1] - src_box.min[1]))
			                    : (dst_box.min[1] + my * (box.min[1] - src_box.min[1]))),
			         (reverse_y ? (dst_box.max[1] - my * (box.min[1] - src_box.min[1]))
			                    : (dst_box.min[1] + my * (box.max[1] - src_box.min[1]))) );
		}


	/**
	 * Return the bounding box of a set of points.
	 * Version for 2 point.
	 */
	template<typename T, size_t N> Box<T, N> getBoundingBox(const Vec<T,N> & P1, const Vec<T, N> & P2)
		{
		Box<T, N> B;
		B.swallowPoint(P1); 
		B.swallowPoint(P2);
		return B; 
		}


	/**
	* Return the bounding box of a set of points.
	* Version for 3 points.
	*/
	template<typename T, size_t N> Box<T, N> getBoundingBox(const Vec<T, N> & P1, const Vec<T, N> & P2, const Vec<T, N> & P3)
		{
		Box<T, N> B;
		B.swallowPoint(P1);
		B.swallowPoint(P2);
		B.swallowPoint(P3);
		return B;
		}


	/**
	* Return the bounding box of a set of points.
	* Version for 4 points.
	*/
	template<typename T, size_t N> Box<T, N> getBoundingBox(const Vec<T, N> & P1, const Vec<T, N> & P2, const Vec<T, N> & P3, const Vec<T, N> & P4)
		{
		Box<T, N> B;
		B.swallowPoint(P1);
		B.swallowPoint(P2);
		B.swallowPoint(P3);
		B.swallowPoint(P4);
		return B;
		}


	/**
	* Return the bounding box of a set of points.
	* Version for a array of points
	*/
	template<typename T, size_t N> Box<T, N> getBoundingBox(const Vec<T, N> * tab, size_t tab_len)
		{
		Box<T, N> B;
		for(size_t l = 0; l < tab_len; l++) B.swallowPoint(tab[l]);
		return B;
		}


	/**
	* Return the bounding box of a set of points.
	* Version for vector
	*/
	template<typename T, size_t N> Box<T, N> getBoundingBox(const std::vector<Vec<T,N> > & vec)
		{
		Box<T, N> B;
		const size_t len = vec.size(); 
		for (size_t l = 0; l < len; l++) B.swallowPoint(vec[l]);
		return B;
		}

}

/* end of file */


