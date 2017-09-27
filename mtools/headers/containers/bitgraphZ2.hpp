/** @file bitgraphZ2.hpp */
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
#include "../misc/stringfct.hpp"
#include "../misc/misc.hpp"

#include <string>
#include <math.h>
#include <limits.h>


namespace mtools
{
    /**
     * Class representing square of Z^2 centered at zero (ie of the form [-X,X-1]^2) where each site
     * is represented by exactle 1 bit i.e. each site is either set or unset. Factorize full/empty
     * subsquare.
     * 
     * @code{.cpp}
     * BitGraphZ2<10> G(100); // 100MB of memory
     * 
     * RGBc colorFct(iVec2 pos)
     * {
     * if (G.Get(pos.X(), pos.Y())) { return RGBc::c_Blue; }
     * return RGBc::c_TransparentWhite;
     * }
     * 
     * int main()
     * {
     * for (int j = 0;j<70;j++) { for (int i = 0;i<90;i++) { G.Set(i, j); } }
     * G.Unset(0, 0);
     * G.Unset(1, 1);
     * Plotter2D P;
     * auto L = makePlot2DLattice(LatticeObj<colorFct>::get());
     * P[L];
     * P.plot();
     * return 0;
     * }
     * @endcode.
     *
     * @tparam  N   Each subsquare is of size 8N x 8N.
    **/
    template<int32 N> class BitGraphZ2
    {
    public:


        /**
         * Constructor. The memory used is about 16L^2 + V(8N^2 + 16)
         *
         * @param   L   the main square is of size 2L x 2L.
         * @param   V   Number of subsquare to allocate
        **/
        BitGraphZ2(int32 L, int32 V) 
            {
            init(L,V);
            }


        /**
         * Constructor. the L and V parameters are automatically deduced such that the quare grid of
         * 2Lx2L uses 1/6 of the total memory.
         *
         * @param   MB  memory (in MB) to use.
        **/
        BitGraphZ2(int32 MB) 
        {
        int64 bta = ((int64)MB)*1024*1024;
        int32 L = (int32)sqrt((double)(bta/(6*16)));
        int32 V = (int32)((5*bta)/(6*((int64)sizeof(_MSQ))));
        init(L,V);
        }


        /**
         * Destructor.
        **/
        ~BitGraphZ2() {delete [] MStab; delete [] Grid;}


        /**
         * Clears the graph (unset all bits).
        **/
        void clear() 
            {
            minx = LLONG_MAX; maxx = LLONG_MIN;
            miny = LLONG_MAX; maxy = LLONG_MIN;
            totset=0; 
            v=0; 
            for(size_t i=0;i<((size_t)(4*LL*LL));i++) {Grid[i]=-1;}
            }


        /**
         * Get a point at coordinate (x,y). Return false is the point is outside of the whole square.
         *
         * @param   x   The x coordinate.
         * @param   y   The y coordinate.
         *
         * @return  return true if the point is set and false otherwise.
        **/
        inline bool get(int64 x,int64 y) const
            {            
            int64 rx = (8*LL*N) + x; int64 ry = (8*LL*N) + y;
            if (((rx<0)||(rx >= 16*(LL*N)))||((ry<0)||(ry >= 16*(LL*N)))) {return false;}
            size_t Gpos = (size_t)((rx/(8*N)) + ((2*LL)*(ry/(8*N))));
            int32 p = Grid[Gpos];
            if (p == -1) {return false;}
            if (p == -2) {return true;}
            unsigned char a = MStab[Grid[Gpos]].get((rx%(8*N)),(ry%(8*N)));
            if (a!= 0) {return true;}
            return false;
            }


        /**
         * Set the point at a given coordinate. Does nothing is outside of the whole square.
         *
         * @param   x   The x coordinate.
         * @param   y   The y coordinate.
        **/
        inline void set(int64 x,int64 y)
            {
            if (x > maxx) {maxx = x;}
            if (y > maxy) {maxy = y;}
            if (x < minx) {minx = x;}
            if (y < miny) {miny = y;}
            int64 rx = (8*LL*N) + x; int64 ry = (8*LL*N) + y;
            if ((rx < 0) || (rx >= 16 * (LL*N))) { return; }
            if ((ry < 0)||(ry >= 16*(LL*N))) { return; }
            size_t Gpos = (size_t)((rx/(8*N)) + ((2*LL)*(ry/(8*N))));
            int32 p = Grid[Gpos];
            if (p == -2) {return;}
            if (p == -1) {if (v == VV) {cleanup(); if (v == VV) {MTOOLS_ERROR("BitGraphZ2::Set(), out of memory !"); }} Grid[Gpos] = v; MStab[v].reset0(Gpos); v++;}
            MStab[Grid[Gpos]].set((rx%(8*N)),(ry%(8*N)),totset);
            }


        /**
        * Unset the point at a given coordinate. Does nothing is outside of the whole square.
        *
        * @param   x   The x coordinate.
        * @param   y   The y coordinate.
        **/
        inline void unset(int64 x,int64 y)
            {
            if (x > maxx) {maxx = x;}
            if (y > maxy) {maxy = y;}
            if (x < minx) {minx = x;}
            if (y < miny) {miny = y;}
            int64 rx = (8*LL*N) + x; int64 ry = (8*LL*N) + y;
            if (((rx<0)||(rx >= 16*(LL*N)))||((ry<0)||(ry >= 16*(LL*N)))) {return;}
            size_t Gpos = (size_t)((rx/(8*N)) + ((2*LL)*(ry/(8*N))));
            int32 p = Grid[Gpos];
            if (p == -1) {return;}
            if (p == -2) {if (v == VV) {cleanup(); if (v == VV) {MTOOLS_ERROR("BitGraphZ2::Unset(), out of memory !"); }} Grid[Gpos] = v; MStab[v].reset1(Gpos); v++;}
            MStab[Grid[Gpos]].unset((rx%(8*N)),(ry%(8*N)),totset);
            }


        /**
         * Number of point currently set.
        **/
        inline uint64 nbSet() const {return totset;}


        /**
         * Return true if the subsquare containing point (x,y) is completely full
         * - one must previously call stat() or cleanup() to cleanup the grid
         * - return false otherwise (or if (x,y) outside the grid).
         * -
         *
         * @param   x   The x coordinate of the point
         * @param   y   The y coordinate of the point
         *
         * @return  true if square full, false if not.
        **/
        inline bool isSquareSet(int64 x,int64 y)
        {
            int64 rx = (8*LL*N) + x; int64 ry = (8*LL*N) + y;
            if (((rx<0)||(rx >= 16*(LL*N)))||((ry<0)||(ry >= 16*(LL*N)))) {return false;}
            size_t Gpos = (size_t)((rx/(8*N)) + ((2*LL)*(ry/(8*N))));
            int32 p = Grid[Gpos];
            if (p == -2) {return true;}
            return false;
        }
    


        /**
        * Return true if the subsquare containing point (x,y) is completely empty
        * - one must previously call stat() or cleanup() to cleanup the grid
        * - return false otherwise (or if (x,y) outside the grid).
        * -
        *
        * @param   x   The x coordinate of the point
        * @param   y   The y coordinate of the point
        *
        * @return  true if square is empty, false if not.
        **/
        inline bool isSquareUnSet(int64 x,int64 y)
        {
            int64 rx = (8*LL*N) + x; int64 ry = (8*LL*N) + y;
            if (((rx<0)||(rx >= 16*(LL*N)))||((ry<0)||(ry >= 16*(LL*N)))) {return false;}
            size_t Gpos = (size_t)((rx/(8*N)) + ((2*LL)*(ry/(8*N))));
            int32 p = Grid[Gpos];
            if (p == -1) {return true;}
            return false;
        }


        /**
         * Print some informations concerning the object in a std::string and does a cleanup of the
         * memory.
         *
         * @return  A std::string.
        **/
        std::string stats()
        {
            std::string s;
            s += "*****************************************************\n";
            s += "BitGraphZ2 object statistics\n\n";
            s += "- memory used         : " + toString(memory()) + "Mb\n";
            s += "- lattice represented : [ " + toString(minV()) + " , " + toString(maxV()) + " ]^2\n";
            s += "- Main grid size      : [ " + toString(-LL) + " , " + toString(LL-1) + " ]^2 (" + toString((4*sizeof(int32)*LL*LL)/(1024*1024)) + "Mb)\n";
            s += "- Size of a subsquare : " + toString(N*8) + " x " + toString(N*8) + " (" + toString(sizeof(_MSQ)) + "b each)\n";
            s += "- Number of subsquare : " + toString(VV) + " (" + toString(((int64)VV)*sizeof(_MSQ) /(1024*1024)) + "Mb)\n\n";
            s += "Number of point set : " + toString(nbSet()) + "\n";
            s += "Surrounding square : ";
            if (minX() != LLONG_MAX) {
            s += "[ " + toString(minX()) + " , " + toString(maxX()) + " ] x [ " + toString(minY()) + " , " + toString(maxY()) + " ]\n";
            } else {s += "No point set yet !\n";}
            s += "Memory used before cleanup\t" + toString(v) + "/" + toString(VV) + " (" + toString((int)(100*(((double)v)/((double)VV))))+ "%)\n";
            cleanup();
            s += "Memory used after cleanup\t" + toString(v) + "/" + toString(VV) + " (" + toString((int)(100*(((double)v)/((double)VV))))+ "%)\n";
            s += "*****************************************************\n";
            return s;            
        }


        /**
         * return the minimal X (also Y) value that can be adressed by the object that is simply equal
         * to -8*N*L.
         *
         * @return  -8*N*L.
        **/
        inline int64 minV() const {return(-8*LL*N);}


        /**
        * return the maximal X (also Y) value that can be adressed by the object that is simply equal
        * to +8*N*L - 1
        *
        * @return  +8*N*L - 1.
        **/
        inline int64 maxV() const {return(8*LL*N - 1);}


        /**
         * Minimum x coordinate of all point set.
         * if no point have been set, min functions return LLONG_MAX and max function return LLONG_MIN.
         * @return  An int64.
        **/
        inline int64 minX() const {return(minx);}


        /**
        * Maximum x coordinate of all point set.
        * if no point have been set, min functions return LLONG_MAX and max function return LLONG_MIN.
        * @return  An int64.
        **/
        inline int64 maxX() const {return(maxx);}


        /**
        * Minimum y coordinate of all point set.
        * if no point have been set, min functions return LLONG_MAX and max function return LLONG_MIN.
        * @return  An int64.
        **/
        inline int64 minY() const {return(miny);}


        /**
        * Maximum y coordinate of all point set.
        * if no point have been set, min functions return LLONG_MAX and max function return LLONG_MIN.
        * @return  An int64.
        **/
        inline int64 maxY() const {return(maxy);}



        /**
         * return the number of MB allocated for the object
         *
         * @return  size of memory used in MB.
        **/
        inline uint64 memory() const {return((((uint64)VV)*sizeof(_MSQ) + 4*sizeof(int32)*LL*LL)/(1024*1024));}

    private:

        /* Initialization of the object, called by the ctors */
        void init(int32 L,int32 V)
            {
            if ((N<2)||(N>4191))           {MTOOLS_ERROR("BitGraphZ2::Init(), template parameter N has incorrect value !"); }
            if ((V<2)||(V>2000000000))     {MTOOLS_ERROR("BitGraphZ2::Init(), constructor parameter L or V is incorrect !"); }
            if ((L<2)||(L>1000000))        {MTOOLS_ERROR("BitGraphZ2::Init(), parameter L incorrect !");}
            VV = V; LL = (int64)L;
            Grid  = new int32[(size_t)(4*LL*LL)];
            MStab = new _MSQ[(size_t)(VV)];
            clear();
            }

        /* Private cleanup function */
       void cleanup()
            {
            int32 i=0; 
            int32 j=0;
            while(j < v)
                {
                size_t nb = MStab[j].nbdone();
                if (i==j) 
                    {
                    if (nb == 0) {Grid[MStab[i].getpos()] = -1;}
                    else {if (nb < ((size_t)(64*N*N))) {i++;} else {Grid[MStab[i].getpos()] = -2;}}
                    }
                else
                    {
                    if (nb == 0) {Grid[MStab[j].getpos()] = -1;}
                    else {if (nb < ((size_t)(64*N*N))) {Grid[MStab[j].getpos()] = i; MStab[i] = MStab[j];  i++;} else {Grid[MStab[j].getpos()] = -2;}}
                    }
                j++;
                }
            v = i;
            }

       /* no copy */
       BitGraphZ2(const BitGraphZ2 &);
       BitGraphZ2 & operator=(const BitGraphZ2 &);


        /* Private Variables */

       /* sub square class */
        class _MSQ
            {
            public:
                _MSQ()                                                      {return;}
                inline void             reset0(size_t p)                    {pos = p; nb = 0; memset(tab,0,(size_t)(8*N*N));}
                inline void             reset1(size_t p)                    {pos = p; nb = 64*N*N; memset(tab,255,(size_t)(8*N*N));}
                inline unsigned char    get(int64 x,int64 y) const          {unsigned char a = tab[(size_t)((N*y) + (x/8))]; return( a & (1 << (x%8)) );}
                inline void             set(int64 x,int64 y,uint64 & t)     {unsigned char a = tab[(size_t)((N*y) + (x/8))]; unsigned char b = (a | (1 << (x%8))); if (b!=a) {tab[(size_t)((N*y) + (x/8))] = b; nb++; t++;}}
                inline void             unset(int64 x,int64 y,uint64 & t)   {unsigned char a = tab[(size_t)((N*y) + (x/8))]; unsigned char mask =  255 - (1 << (x%8)); unsigned char b = (a & mask); if (b!=a) {tab[(size_t)((N*y) + (x/8))] = b; nb--; t--;}}
                inline size_t           getpos() const                      {return pos;}
                inline size_t           nbdone() const                      {return nb;}
            private:
                size_t  pos;
                size_t  nb;
                unsigned char tab[8*N*N];
            };


        int64 minx,maxx,miny,maxy;
        uint64  totset;                 
        int32   v;                      

        int32 * Grid;                   
        int32   VV;             
        int64   LL;                     
        _MSQ *  MStab;                  
    };

}


/* end of file.h */




