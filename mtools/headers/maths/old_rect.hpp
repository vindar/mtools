/** @file rect.hpp */
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
    template<typename T> class Rect;


    /**
     * Floating point rectangle.
     **/
    typedef Rect<double> fRect; 


    /**
     * Integer rectangle.
     **/
    typedef Rect<int64> iRect;


    /**
     * Class representing a rectangle.
     *
     * The rectangle is of the form [xmin,xmax]x[ymin,ymax].
     * if  (xmin > xmax) or (ymin > ymax) the rectangle is considered empty. 
     **/
    template<typename T> class Rect
    {

    public:

        /**
         * Default constructor. Create a completely empty rectangle.
         **/
        Rect() : xmin(1), xmax(0), ymin(1), ymax(0) { return; }


        /**
         * Constructor. Set the dimensions of the rectangle.
         * If (xmax < xmin) or (ymax < ymin) the rectangle is empty.
         *
         * @param   Xmin    minimum x-value.
         * @param   Xmax    maximum x-value.
         * @param   Ymin    minimum y-value.
         * @param   Ymax    maximum y value.
         **/
        Rect(T Xmin, T Xmax, T Ymin, T Ymax) : xmin(Xmin), xmax(Xmax), ymin(Ymin), ymax(Ymax) { return; }


        /**
         * Constructor from 2 points. Create the rectangle having these 2 points as corner. The
         * rectangle obtained can be flat (if both point have a coordinate in commom) or even reduced to a
         * single point (if the points are equal) but it cannot be empty. 
         *
         * @param   P1  The first point.
         * @param   P2  The second point
         **/
        Rect(Vec<T,2> P1, Vec<T,2> P2)
            {
            if (P1.X() < P2.X()) { xmin = P1.X(); xmax = P2.X(); } else { xmin = P2.X(); xmax = P1.X(); }
            if (P1.Y() < P2.Y()) { ymin = P1.Y(); ymax = P2.Y(); } else { ymin = P2.Y(); ymax = P1.Y(); }
            }


        /**
         * Default copy constructor.
         **/
        Rect(const Rect & R) = default;


        /**
         * Copy constructor from another template parameter.
         **/
        template<typename U> Rect(const Rect<U> & R) : xmin((T)R.xmin), xmax((T)R.xmax), ymin((T)R.ymin), ymax((T)R.ymax) {}


        /**
         * Default assignment operator.
         **/
        Rect & operator=(const Rect<T> & R) = default;


        /**
        * Assignment operator from another type.
        **/
        template<typename U> Rect & operator=(const Rect<U> & R)
            {
            xmin = (T)R.xmin; xmax = (T)R.xmax;
            ymin = (T)R.ymin; ymax = (T)R.ymax;
            return(*this);
            }


        /**
         * Default destructor.
         **/
        ~Rect() = default;


        /**
         * Query if the rectangle is empty i.e. if it is empty in one of the 2 directions (hor. or vert.)
         *
         * @return  true if empty, false if not.
         **/
        inline bool isEmpty() const { return ((xmax < xmin) || (ymax < ymin)); }


        /**
         * Queries if the rectangle if "horizontally" empty i.e. xmax < xmin.
         *
         * @return  true if horizontally empty, false if not.
         **/
        inline bool isHorizontallyEmpty() const { return (xmax < xmin); }


        /**
         * Queries if the rectangle if "vertically" empty i.e. ymax < ymin.
         *
         * @return  true if vertically empty, false if not.
         **/
        inline bool isVerticallyEmpty() const { return (ymax < ymin); }


        /**
         * Queries if the rectangle is empty in both direction i.e. vert. and hor.
         * 
         * @return  true if completely empty, false if not.
         **/
        inline bool isCompletelyEmpty() const { return ((xmax < xmin)&&(ymax < ymin)); }


        /**
        * Query if the rectangle is reduced to a single point.
        *
        * @return  true if it is a signle point, false if not.
        **/
        inline bool isPoint() const { return ((xmax == xmin) && (ymax == ymin)); }


        /**
         * Set the rectangle as completely empty (in both directions).
         **/
        inline void clear() { xmin = 1; xmax = 0; ymin = 1; ymax = 0; }


        /**
         * Set the rectangle as vertically empty (does no change the horizontal range).
         **/
        inline void clearVertically() { ymin = 1; ymax = 0; }


        /**
         * Set the rectangle as horizontally empty (does no change the vertical range).
         **/
        inline void clearHorizontally() { xmin = 1; xmax = 0; }


        /**
         * Query if a point is inside the closed rectangle
         *
         * @param   pos The position to check
         *
         * @return  true if the point is in the close rectangle and false otherwise
         **/
        inline bool isInside(const Vec<T, 2> pos) const
        {
        return ((pos.X() >= xmin) && (pos.X() <= xmax) && (pos.Y() >= ymin) && (pos.Y() <= ymax));
        }


        /**
        * Query if a point is inside the open rectangle
        *
        * @param   pos The position to check
        *
        * @return  true if the point is in the close rectangle and false otherwise
        **/
        inline bool isStrictlyInside(const Vec<T, 2> pos) const
        {
        return ((pos.X() > xmin) && (pos.X() < xmax) && (pos.Y() > ymin) && (pos.Y() < ymax));
        }


        /**
        * Enlarge the rectangle in order to contain a given point. If the rectangle alreasy contain the
        * point do nothing.
        *
        * @param   pos The position of the point to swallow.
        *
        * @return  true if the point was swallowed and false if it was already contained in the rectangle.
        **/
        inline bool swallowPoint(const Vec<T, 2> pos)
            {
            bool b = false;
            if (pos.X() < xmin) { xmin = pos.X(); b = true; }
            if (pos.X() > xmax) { xmax = pos.X(); b = true; }
            if (pos.Y() < ymin) { ymin = pos.Y(); b = true; }
            if (pos.Y() > ymax) { ymax = pos.Y(); b = true; }
            return b;
            }


        /**
         * Try to enlarge the rectangle using points from another rectangle if possible. By defnition,
         * the resulting rectangle contain the initial one and is included in the union of the intial
         * one and the source R used to enlarge.
         * 
         * @warning Both rectangles should not be empty.
         *
         * @param   R   the rectagle R use for the enlargment.
         **/
        inline void enlargeWith(const Rect & R)
            {
            const bool containV = ((R.xmin <= xmin) && (xmax <= R.xmax)) ? true : false;
            const bool containH = ((R.ymin <= ymin) && (ymax <= R.ymax)) ? true : false;
            if ((!containV) && (!containH)) return;    // nothing to do
            if (containV && containH) { *this = R; return; } // initial rectangle is included in R so we set to R
            if (containV)
                { // try to improve horizontally
                if ((R.ymax > ymax) && (R.ymin <= ymax)) { ymax = R.ymax; }
                if ((R.ymin < ymin) && (R.ymax >= ymin)) { ymin = R.ymin; }
                return;
                }
            // try to improve vertically
            if ((R.xmax > xmax) && (R.xmin <= xmax)) { xmax = R.xmax; }
            if ((R.xmin < xmin) && (R.xmax >= xmin)) { xmin = R.xmin; }
            }

        /**
         * Compute the distance of a point inside the rectangle to its boundary
         * 
         * @param   pos The position.
         *
         * @return  the distance between the point and the boundary or a negative value if the point is not inside the rectangle.
         **/
        inline T boundaryDist(const Vec<T, 2> & pos) const
            {
            const T lx1 = xmax - pos.X();
            const T lx2 = pos.X() - xmin;
            const T ly1 = ymax - pos.Y();
            const T ly2 = pos.Y() - ymin;
            return std::min<T>(std::min<T>(lx1, lx2), std::min<T>(ly1, ly2)); // distance from the boundary
            }


        /**
         * Compute the center of the rectangle. The position returned has no meaning if the rectangle is
         * empty.
         *
         * @return  The position of the center.
         **/
        inline Vec<T, 2> center() const { return Vec<T, 2>((xmax+xmin)/2 , (ymax + ymin)/2); }

        /**
         * Return the width : max(0,xmax-xmin) of the rectangle.
         * (may be 0 even for non empty rectangle or positive even if rectangle is empty).
         **/
        inline T lx() const { return std::max<T>(0, xmax - xmin); }


        /**
         * Return the height : max(0,ymax-ymin) of the rectangle.
         * (may be 0 even for non empty rectangle or positive even if rectangle is empty).
         **/
        inline T ly() const { return std::max<T>(0, ymax - ymin); }


        /**
         * Return the smallest of both length lx() and ly(). 
         *
         * @return  The min length between lx() and ly().
         **/
        inline T minlxy() const { return std::min(lx(), ly()); }


        /**
        * Return the largest of both length lx() and ly().
        *
        * @return  The max length between lx() and ly().
        **/
        inline T maxlxy() const { return std::max(lx(), ly()); }


        /**
         * Equality operator.
         *
         * @param   R   The rectangle to compare with.
         *
         * @return  true if both rectangle are either equal or both empty.
         **/
        inline bool operator==(const Rect<T> & R) const
            {
            if (isEmpty()) { if (R.isEmpty()) return true; return false; }
            if (R.isEmpty()) return false;
            return((xmin == R.xmin) && (ymin == R.ymin) && (xmax == R.xmax) && (ymax == R.ymax));
            }


        /**
         * Inequality operator.
         *
         * @param   R   The rectangle to compare with.
         *
         * @return  true they are different (two empty rectangles are considered equal).
         **/
        inline bool operator!=(const Rect<T> & R) const { return(!(operator==(R))); }


        /**
         * Less-than-or-equal comparison operator. Check if the rectangle is included in another
         * rectangle (for the inclusion partial order).
         *
         * @param   R   The rectangle to check if it contains this.
         *
         * @return  true if R contains this, false otherwise. An empty rectangle contains nothing but is
         *          contained in every non empty rectangle.
         **/
        bool operator<=(const Rect<T> & R) const
            {
            if (R.isEmpty()) return false;
            if (isEmpty()) return true;
            return((xmin >= R.xmin) && (ymin >= R.ymin) && (xmax <= R.xmax) && (ymax <= R.ymax));
            }


        /**
         * Greater-than-or-equal comparison operator. Check if the rectangle contains another rectangle
         * (for the inclusion partial order).
         *
         * @param   R   The rectangle to check if it is contained in this.
         *
         * @return  true if R is contained in this, false otherwise. An empty rectangle contains nothing
         *          but is contained in every non empty rectangle.
         **/
        bool operator>=(const Rect<T> & R) const { return(R.operator<=(*this)); }


        /**
         * Strictly less-than comparison operator. Check if the rectangle is strictly included in
         * another rectangle (for the inclusion partial order).
         *
         * @param   R   The rectangle to check if it strictly contains this.
         *
         * @return  true if R strictly contains this, false otherwise. An empty rectangle contains
         *          nothing but is strictly contained in every non empty rectangle.
         **/
        bool operator<(const Rect<T> & R) const { return ((operator!=(R)) && (operator<=(R))); }


        /**
         * Strictly greater-than comparison operator. Check if the rectangle contains striclty another
         * rectangle (for the inclusion partial order).
         *
         * @param   R   The rectangle to check if it is strictly contained in this.
         *
         * @return  true if R is striclty contained in this, false otherwise. An empty rectangle contains
         *          nothing but is striclty contained in every non empty rectangle.
         **/
        bool operator>(const Rect<T> & R) const { return ((operator!=(R)) && (operator>=(R))); }


        /**
         * Print the rectangle into a std::string.
         *
         * @return  a string in the form "[xmin,ymin]x[ymin,ymax]" or "(empty)[xmin,ymin]x[ymin,ymax]"
         **/
        std::string toString() const { return((isEmpty() ? (std::string("(empty)[")) : (std::string("["))) + mtools::toString(xmin) + "," + mtools::toString(xmax) + "]x[" + mtools::toString(ymin) + "," + mtools::toString(ymax) + "]");}


        /**
         * Return the subrectangle corresponding to the intersection of *this and R. The returned rectangle 
         * is "seen" relatively to *this i.e. its coordinate are w.r.t. ((*this).xmin , (*this).ymin).
         * If either R or this is empty, the returned rectangle is empty. 
         *
         * @param   R   The rectangle to intersect
         *
         * @return  The intersection rectangle seen as a sub rectangle of *this. 
         **/
        Rect<T> relativeSubRect(const Rect<T> & R) const
            {
            Rect<T> S;
            if (isEmpty() || R.isEmpty()) { return S; }
            S.xmin = std::max<T>(xmin, R.xmin) - xmin;
            S.ymin = std::max<T>(ymin, R.ymin) - ymin;
            S.xmax = std::min<T>(xmax, R.xmax) - xmin;
            S.ymax = std::min<T>(ymax, R.ymax) - ymin;
            return S;
            }


        /**
         * Returns the area inside the rectangle.
         *
         * @return  The area is 0 if the rectangle is empty or if the rectangle is flat). 
         **/
        inline T area() const
            {
            if (isEmpty()) return 0;
            return((xmax - xmin)*(ymax - ymin));
            }


        /**
         * Returns the area of the intersection of the rectangle with the square [x-0.5,x+0.5]*[y-0.5,y+0.5].
         *
         * @param   x   The x coordinate of the point to check.
         * @param   y   The y coordinate of the point to check.
         *
         * @return  The area of the intersection.
         **/
        inline double pointArea(double x,double y) const
            {
            double llx = std::min<double>(xmax, x + 0.5) - std::max<double>(xmin, x - 0.5); if (llx <= 0) { return 0.0; }
            double lly = std::min<double>(ymax, y + 0.5) - std::max<double>(ymin, y - 0.5); if (lly <= 0) { return 0.0; }
            return(llx*lly);
            }


        /**
         * Returns an integer rectangle containing all the points (i,j) such that the unit rectangle 
         * [i-0.5,i+0.5]*[j-0.5,j+0.5] intersects the rectangle.
         * 
         * @return  the enclosing Rect<int64>.
         **/
        inline iRect integerEnclosingRect() const {return(iRect((int64)floor(xmin + 0.5) , (int64)ceil(xmax - 0.5) , (int64)floor(ymin + 0.5) , (int64)ceil(ymax - 0.5)));}


        /**
         * Returns the minimal centered enclosing rectangle with a given aspect ratio.
         *
         * @param   lxperly The desired ratio lx/ly. 
         *
         * @return  the enclosing rectangle with this ratio.
         **/
        inline fRect fixedRatioEnclosingRect(double lxperly) const
            {
            if ((lx() <= 0)||(ly() <= 0)) return fRect();
            double rat = ((double)lx()) / ((double)ly());
            if (rat < lxperly) { return fRect(((double)(xmin + xmax)) / 2.0 - ly()*lxperly / 2.0, ((double)(xmin + xmax)) / 2.0 + ly()*lxperly / 2.0, (double)ymin, (double)ymax); }
            return fRect((double)xmin,(double)xmax, ((double)(ymin + ymax))/2.0 - (lx()/lxperly)/2.0, ((double)(ymin + ymax)) / 2.0 + (lx()/lxperly)/2.0);
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
            if (rat < lxperly) { return fRect((double)xmin, (double)xmax, ((double)(ymin + ymax))/2.0 - ((double)lx()/lxperly)/2.0, ((double)(ymin + ymax))/2.0 + ((double)lx()/lxperly)/2.0); }
            return fRect(((double)(xmin + xmax))/2.0 - ((double)ly())*lxperly / 2.0, ((double)(xmin + xmax))/2.0 + ((double)ly())*lxperly/2.0, (double)ymin, (double)ymax);
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
            if (x < -2000000000) { x = -2000000000; } else if (x > +2000000000) { x = +2000000000; } // dirty, for overflow
            double y = floor((((absCoord.Y() - ymin) / ly())*scrSize.Y()) + 0.5);
            if (y < -2000000000) { y = -2000000000; } else if (y > +2000000000) { y = +2000000000; } // dirty, for overflow
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
            double x = xmin + (xmax - xmin)*((double)(2*pixCoord.X() + 1)/((double)(2*scrSize.X())));
            double y = ymin + (ymax - ymin)*((double)(2*(scrSize.Y() - 1 - pixCoord.Y()) + 1) / ((double)(2 * scrSize.Y())));
            return fVec2(x, y);
            }


        /**
         * Serializes / deserialize the object. Compatible with boost and with the custom serialization
         * classes OArchive and IArchive. The method performs both serialization and deserialization.
         **/
        template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0)
            {
            ar & xmin;
            ar & xmax;
            ar & ymin;
            ar & ymax;
            }


        T xmin;     ///< the minimum x-value
        T xmax;     ///< the maximum x-value
        T ymin;     ///< the minimum y-value
        T ymax;     ///< the maximum y-value
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
    template<typename T> inline Rect<T> intersectionRect(const Rect<T> & R1, const Rect<T> & R2) {return(Rect<T>(std::max<T>(R1.xmin, R2.xmin), std::min<T>(R1.xmax, R2.xmax), std::max<T>(R1.ymin, R2.ymin), std::min<T>(R1.ymax, R2.ymax)));}


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


