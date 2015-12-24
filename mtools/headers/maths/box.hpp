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


#include "../misc/error.hpp"
#include "../misc/misc.hpp"
#include "../misc/stringfct.hpp"
#include "vec.hpp"

#include <string>
#include <ostream>


namespace mtools
    {

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
            Box() : min(1), max(0), xmin(min[0]), xmax(max[0]), ymin(min[1]), ymax(max[1]) { return; }


            /**
            * Constructor from min and max points.
            * (no reordering)
            **/
            Box(const Vec<T, N> & minVec, const Vec<T, N> & maxVec, bool reorderIfNeeded = true) : min(minVec), max(maxVec), xmin(min[0]), xmax(max[0]), ymin(min[1]), ymax(max[1])
                {
                if (reorderIfNeeded) MTOOLS_ERROR("old version, should be changed...");
                }


            /**
            * Constructor. Specific for dimension 2.
            **/
            Box(const T & xmin, const T & xmax, const T & ymin, const T & ymax) : min(xmin,ymin) , max(xmax,ymax), xmin(min[0]), xmax(max[0]), ymin(min[1]), ymax(max[1])
                {
                static_assert(N == 2, "dimension must be equal to 2");
                }
            

           /**
            * Default copy constructor.
            **/
            Box(const Box & B) : min(B.min), max(B.max), xmin(min[0]), xmax(max[0]), ymin(min[1]), ymax(max[1]) {}


            /**
            * Copy constructor from another template parameter.
            **/
            template<typename U> Box(const Box<U> & B) : min(B.min), max(B.max), xmin(min[0]), xmax(max[0]), ymin(min[1]), ymax(max[1]) {}


            /**
            * Default assignment operator.
            **/
            Box & operator=(const Box<T> & B) = default;


            /**
            * Assignment operator from another type.
            **/
            template<typename U> Box & operator=(const Box<U> & B) { min = B.min; max = B.max; return(*this); }


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
            inline void clearVertically() { min[1] = 1; max[1] = 0; }


            /**
            * Query if a point is inside the closed box.
            *
            * @param   pos The position to check
            *
            * @return  true if the point is in the closed box and false otherwise
            **/
            inline bool isInside(const Vec<T, N> & pos) const { return ((min <= pos) && (pos <= max)); }


            /**
            * Query if a point is inside the open box.
            *
            * @param   pos The position to check
            *
            * @return  true if the point is in the closed box and false otherwise
            **/
            inline bool isStrictlyInside(const Vec<T, N> & pos) const { return ((min < pos) && (pos < max)); }


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
                bool b = false;
                for (size_t i = 0; i < N; i++)
                    {
                    if (pos[i] < min[i]) { min[i] = pos[i]; b = true; }
                    if (pos[i] > max[i]) { max[i] = pos[i]; b = true; }
                    }
                return b;
                }


            /**
            * Try to enlarge the rectangle using points from another rectangle if possible. By defnition,
            * the resulting rectangle contain the initial one and is included in the union of the intial
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
            * Compute the distance of a point inside the rectangle to its boundary
            *
            * @param   pos The position.
            *
            * @return  the distance between the point and the boundary or a negative value if the point is not inside the rectangle.
            **/
            inline T boundaryDist(const Vec<T, N> & pos) const
                {
                const T l = max[0] - pos[0]; l = std::min<T>(l, pos[0] - min[0]);
                for (size_t i = 1; i < N; i++) { l = std::min<T>(std::min<T>(l , max[i] - pos[i]), pos[i] - min[i]); }
                return l;
                }


            /**
            * Compute the center of the rectangle. The position returned has no meaning if the rectangle is
            * empty.
            *
            * @return  The position of the center.
            **/
            inline Vec<T, N> center() const { Vec<T, N> V; for (size_t i = 0; i < N; i++) { V[i] = (min[i] + max[i]) / 2; } return V; }


            /**
            * Return the width : lenght in direction 0.
            **/
            inline T lx() const { return std::max<T>(0, max[0] - min[0]); }

            
            /**
            * Return the height : lenght in direction 1.
            **/
            inline T ly() const { static_assert(N >= 2, "dimension N must be at least 2..."); return std::max<T>(0, max[1] - min[1]); }


            /**
            * Return the depth : lenght in direction 2.
            **/
            inline T lz() const { static_assert(N >= 3, "dimension N must be at least 3..."); return std::max<T>(0, max[2] - min[2]); }


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
            inline bool operator==(const Box & B) const { return(((isEmpty())&&(B.isEmpty()))||(this->operator==(B))); }


            /**
            * Inequality operator.
            *
            * @param   B   The rectangle to compare with.
            *
            * @return  true they are different (two empty rectangles are considered equal).
            **/
            inline bool operator!=(const Box & B) const { return(!(operator==(B))); }


            /**
            * Less-than-or-equal comparison operator. Check if the rectangle is included in another
            * rectangle (for the inclusion partial order).
            *
            * @param   B   The rectangle to check if it contains this.
            *
            * @return  true if R contains this, false otherwise. An empty rectangle contains nothing but is
            *          contained in every non empty rectangle.
            **/
            bool operator<=(const Box & B) const
                {
                if (B.isEmpty()) return false;
                if (isEmpty()) return true;
                return((B.min <= min) && (B.max >= max));
                }


            /**
            * Greater-than-or-equal comparison operator. Check if the rectangle contains another rectangle
            * (for the inclusion partial order).
            *
            * @param   B   The rectangle to check if it is contained in this.
            *
            * @return  true if R is contained in this, false otherwise. An empty rectangle contains nothing
            *          but is contained in every non empty rectangle.
            **/
            bool operator>=(const Box & B) const { return(B.operator<=(*this)); }


            /**
            * Strictly less-than comparison operator. Check if the rectangle is strictly included in
            * another rectangle (for the inclusion partial order).
            *
            * @param   B   The rectangle to check if it strictly contains this.
            *
            * @return  true if R strictly contains this, false otherwise. An empty rectangle contains
            *          nothing but is strictly contained in every non empty rectangle.
            **/
            bool operator<(const Box & B) const 
                {
                if (B.isEmpty()) return false;
                if (isEmpty()) return true;
                return((B.min < min) && (B.max > max));
                }


            /**
            * Strictly greater-than comparison operator. Check if the rectangle contains striclty another
            * rectangle (for the inclusion partial order).
            *
            * @param   B   The rectangle to check if it is strictly contained in this.
            *
            * @return  true if R is striclty contained in this, false otherwise. An empty rectangle contains
            *          nothing but is striclty contained in every non empty rectangle.
            **/
            bool operator>(const Box & B) const { return(B.operator<(*this)); }


            /**
            * Print the rectangle into a std::string.
            *
            * @return  a string in the form "[X1,X2]x[Y1,Y2]x...".
            **/
            std::string toString() const 
                { 
                std::string s = isEmpty() ? std::string("(empty)") : std::string();
                for (size_t i = 0; i < N; i++)
                    {
                    if (i != 0) { s += "x"; }
                    s += std::string("[") + mtools::toString(min[i]) + "," + mtools::toString(max[i]) + "]";
                    }
                return s;
                }


            /**
            * Return the subrectangle corresponding to the intersection of *this and B. The returned rectangle
            * is "seen" relatively to *this. If either B or this is empty, the returned rectangle is empty.
            *
            * @param   B   The rectangle to intersect
            *
            * @return  The intersection rectangle seen as a sub rectangle of *this.
            **/
            Rect<T> relativeSubRect(const Box & B) const
                {
                Box S;
                if (isEmpty() || B.isEmpty()) { return S; }
                for (size_t i = 0; i < N; i++)
                    {
                    S.min[i] = std::max<T>(min[i], B.min[i]) - min[i];
                    S.max[i] = std::min<T>(max[i], B.max[i]) - min[i];
                    }
                return S;
                }


            /**
            * Returns the area inside the rectangle.
            *
            * @return  The area is 0 if the rectangle is empty or flat.
            **/
            inline T area() const
                {
                T a = max[0] - min[0];
                if (a <= 0) return 0;
                for (size_t i = 1; i < N; i++) { const T v = max[i] - min[i]; if (v <= 0) return 0; a *= m; }
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
            * Returns an integer box containing all the points (i,j) such that the unit rectangle
            * [i-0.5,i+0.5]*[j-0.5,j+0.5] intersects the rectangle.
            **/
            inline iBox<N> integerEnclosingRect() const 
                { 
                iVec<N> imin, imax;
                for (size_t i = 0; i < N; i++)
                    {
                    imin[i] = (int64)floor(min[i] + 0.5);
                    imax[i] = (int64)ceil(max[i] - 0.5);
                    }
                return iBox(imin,imax);
                }


            /**
            * Returns the minimal centered enclosing rectangle with a given aspect ratio.
            *
            * @param   lxperly The desired ratio lx/ly.
            *
            * @return  the enclosing rectangle with this ratio.
            **/
            inline fRect fixedRatioEnclosingRect(double lxperly) const
                {
                if ((lx() <= 0) || (ly() <= 0)) return fRect();
                double rat = ((double)lx()) / ((double)ly());
                if (rat < lxperly) { return fRect(((double)(xmin + xmax)) / 2.0 - ly()*lxperly / 2.0, ((double)(xmin + xmax)) / 2.0 + ly()*lxperly / 2.0, (double)ymin, (double)ymax); }
                return fRect((double)xmin, (double)xmax, ((double)(ymin + ymax)) / 2.0 - (lx() / lxperly) / 2.0, ((double)(ymin + ymax)) / 2.0 + (lx() / lxperly) / 2.0);
                }


            /**
            * Returns the maximal centered enclosed rectangle with a given aspect ratio.
            *
            * @param   lxperly The desired ratio lx/ly.
            *
            * @return  the enclosed rectangle with this ratio.
            **/
            inline fRect fixedRatioEnclosedRect(double lxperly) const
                {
                if ((lx() <= 0) || (ly() <= 0)) return fRect();
                double rat = ((double)lx()) / ((double)ly());
                if (rat < lxperly) { return fRect((double)xmin, (double)xmax, ((double)(ymin + ymax)) / 2.0 - ((double)lx() / lxperly) / 2.0, ((double)(ymin + ymax)) / 2.0 + ((double)lx() / lxperly) / 2.0); }
                return fRect(((double)(xmin + xmax)) / 2.0 - ((double)ly())*lxperly / 2.0, ((double)(xmin + xmax)) / 2.0 + ((double)ly())*lxperly / 2.0, (double)ymin, (double)ymax);
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
            iVec2 absToPixel(const fVec2 & absCoord, const iVec2 & scrSize) const
                {
                MTOOLS_ASSERT(!isEmpty());
                double x = floor((((absCoord.X() - xmin) / lx())*scrSize.X()) + 0.5);
                if (x < -2000000000) { x = -2000000000; }
                else if (x > +2000000000) { x = +2000000000; } // dirty, for overflow
                double y = floor((((absCoord.Y() - ymin) / ly())*scrSize.Y()) + 0.5);
                if (y < -2000000000) { y = -2000000000; }
                else if (y > +2000000000) { y = +2000000000; } // dirty, for overflow
                return iVec2((int64)x, scrSize.Y() - 1 - (int64)(y));
                }


            /**
            * Convert a position inside a screen into an absolute position.
            *
            * @param   pixCoord    The pixel coordinate (i,j)
            * @param   scrSize     Size of the screen in pixel (lx,ly).
            *
            * @return  A fVec2 containing the aboslute position of this pixel.
            **/
            fVec2 pixelToAbs(const iVec2 & pixCoord, const iVec2 & scrSize) const
                {
                MTOOLS_ASSERT(!isEmpty());
                double x = xmin + (xmax - xmin)*((double)(2 * pixCoord.X() + 1) / ((double)(2 * scrSize.X())));
                double y = ymin + (ymax - ymin)*((double)(2 * (scrSize.Y() - 1 - pixCoord.Y()) + 1) / ((double)(2 * scrSize.Y())));
                return fVec2(x, y);
                }


            /**
            * Serializes / deserialize the object. Compatible with boost and with the custom serialization
            * classes OArchive and IArchive. The method performs both serialization and deserialization.
            **/
            template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0)
                {
                ar & min;
                ar & max;
                }

        Vec<T, N> min; // vector with the min coordinates in each directions
        Vec<T, N> max; // vector with the max coordinates in each directions

        // reference for retro compatibility with old Rect class
        T & xmin;
        T & xmax;
        T & ymin;
        T & ymax;
        };


    /**
    * Zoom inside the rectangle (reduce the radius by 1/10th).
    *
    * @param   R   The rectangle to zoom in.
    *
    * @return  The zoomed in rectangle.
    **/
    template<typename T> inline Rect<T> zoomIn(const Rect<T> & R)
        {
        T lx = R.xmax - R.xmin;
        T ly = R.ymax - R.ymin;
        return Rect<T>(R.xmin + (lx / 10.0), R.xmax - (lx / 10.0), R.ymin + (ly / 10.0), R.ymax - (ly / 10.0));
        }


    /**
    * Zoom outside of the rectangle (increase radius by 1/8th).
    *
    * @param   R   The rectangle to zoom out.
    *
    * @return  The zoomed out rectangle.
    **/
    template<typename T> inline Rect<T> zoomOut(const Rect<T> & R)
        {
        T lx = R.xmax - R.xmin;
        T ly = R.ymax - R.ymin;
        return Rect<T>(R.xmin - (lx / 8.0), R.xmax + (lx / 8.0), R.ymin - (ly / 8.0), R.ymax + (ly / 8.0));
        }


    /**
    * Move the rectangle up by 1/20th of its height.
    *
    * @param   R   The rectangle to move up
    *
    * @return The same rectangle shifted up by 1/20th of its height.
    **/
    template<typename T> inline Rect<T> up(const Rect<T> & R)
        {
        T off = R.ly() / 20;
        return Rect<T>(R.xmin, R.xmax, R.ymin + off, R.ymax + off);
        }


    /**
    * Move the rectangle down by 1/20th of its height.
    *
    * @param   R   The rectangle to move down
    *
    * @return The same rectangle shifted down by 1/20th of its height.
    **/
    template<typename T> inline Rect<T> down(const Rect<T> & R)
        {
        T off = R.ly() / 20;
        return Rect<T>(R.xmin, R.xmax, R.ymin - off, R.ymax - off);
        }


    /**
    * Move the rectangle left by 1/20th of its length.
    *
    * @param   R   The rectangle to move left
    *
    * @return The same rectangle shifted left by 1/20th of its length.
    **/
    template<typename T> inline Rect<T> left(const Rect<T> & R)
        {
        T off = R.lx() / 20;
        return Rect<T>(R.xmin - off, R.xmax - off, R.ymin, R.ymax);
        }


    /**
    * Move the rectangle right by 1/20th of its length.
    *
    * @param   R   The rectangle to move right
    *
    * @return The same rectangle shifted right by 1/20th of its length.
    **/
    template<typename T> inline Rect<T> right(const Rect<T> & R)
        {
        T off = R.lx() / 20;
        return Rect<T>(R.xmin + off, R.xmax + off, R.ymin, R.ymax);
        }


    /**
    * The rectangle obtained as the intersection of two rectangles.
    *
    * @param   R1  The first rectangle.
    * @param   R2  The second rectangle.
    *
    * @return  the rectangle obtained as the intersection of R1 and R2. May be an empty rectangle.
    **/
    template<typename T> inline Rect<T> intersectionRect(const Rect<T> & R1, const Rect<T> & R2) { return(Rect<T>(std::max<T>(R1.xmin, R2.xmin), std::min<T>(R1.xmax, R2.xmax), std::max<T>(R1.ymin, R2.ymin), std::min<T>(R1.ymax, R2.ymax))); }


    /**
    * The rectangle obtained as the smallest rectangle containing R1 and R2.
    *
    * @param   R1  The first rectangle.
    * @param   R2  The second rectangle.
    *
    * @return  the rectangle that contain R1 and R2.
    **/
    template<typename T> inline Rect<T> unionRect(const Rect<T> & R1, const Rect<T> & R2)
        {
        if (R1.isEmpty()) { return R2; }
        if (R2.isEmpty()) { return R1; }
        return(Rect<T>(std::min<T>(R1.xmin, R2.xmin), std::max<T>(R1.xmax, R2.xmax), std::min<T>(R1.ymin, R2.ymin), std::max<T>(R1.ymax, R2.ymax)));
        }



    }


/* end of file */


