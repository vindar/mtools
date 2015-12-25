/** @file cimg_plugin.hpp */
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


/* 
 * Plugin for the CImg library.
 */

#ifndef INC_cimg_plugin_HPP
#define INC_cimg_plugin_HPP


    /**
     * Return the color of a pixel.
     *
     * @param   pos The position of the pixel
     *
     * @return  The color. If the image has only 3 channel, the alpha channel is set to 255 and to its correct value otherwise.
     **/
    inline mtools::RGBc getPixel(mtools::iVec2 pos) const
        {
        const int x = (int)pos.X();
        const int y = (int)pos.Y();
        const char r = operator()(x, y, 0, 0);
        const char g = operator()(x, y, 0, 1);
        const char b = operator()(x, y, 0, 2);
        const char a = ((spectrum() >= 4) ? (operator()(x, y, 0, 3)) : 255);
        return mtools::RGBc(r, g, b, a);
        }


    /**
     * Sets the color of a pixel.
     *
     * @param   pos     The position of the pixel.
     * @param   color   The color. The alpha channel is set if the image has more than 3 channel and
     *                  ignored otherwise.
     **/
    inline void setPixel(mtools::iVec2 pos, mtools::RGBc color)
        {
        const int x = (int)pos.X();
        const int y = (int)pos.Y();
        operator()(x, y, 0, 0) = color.R;
        operator()(x, y, 0, 1) = color.G;
        operator()(x, y, 0, 2) = color.B;
        if (spectrum() >= 4) { operator()(x, y, 0, 3) = color.A; }
        }




    /**
     * Return the image size into a iVec2 structure.
     **/
    inline mtools::iVec2 imageSize() const { return mtools::iVec2(width(), height()); }


    /**
     * return the image aspect ratio lx/ly.
     **/
    inline double imageAspectRatio() const { MTOOLS_ASSERT((width() > 0) && (height() > 0)); return(((double)width()) / (double)height()); }


    /**
     * Return the position (i,j) of the pixel in the image associated with the absolute coordinate
     * (x,y) w.r.t. to a mapping rectangle R.
     *
     * @param   R       the rectangle defining the range represented by the image.
     * @param   coord   the absolute coordinate to transform.
     *
     * @return  the associated pixel the image (no clipping, may be outside of the image).
     **/
	inline mtools::iVec2 getImageCoord(const mtools::fBox2 & R,const mtools::fVec2 & coord) const {return R.absToPixel(coord,imageSize());}


    /**
     * Return the absolute position of a pixel (i,j) of the image according to a mapping rectangle R.
     *
     * @param   R       the rectangle defining the range represented by the image.
     * @param   pixpos  The position of the pixel in the image.
     *
     * @return  the associated absolute coordinate.
     **/
    inline mtools::fVec2 getAbsCoord(const mtools::fBox2 & R, const mtools::iVec2 & pixpos) const {return R.pixelToAbs(pixpos, imageSize());}


    /**
     * Return an enlarged rectangle of R with the same centering but such that the ratio of the
     * returned  rectangle match the ratio of the image.
     *
     * @param   R   the original rectangle.
     *
     * @return  A possibly enlarged rectangle (with same centering) with same aspect ratio as the
     *          image.
     **/
	inline mtools::fBox2 respectImageAspectRatio(const mtools::fBox2 & R) const {return(R.fixedRatioEnclosingRect(imageAspectRatio()));}
	

    /**
     * The canonical range rectangle corresponding to the image size
     **/
    inline mtools::fBox2 canonicalRange() const { MTOOLS_ASSERT((this->width() > 0) && (this->height() > 0)); mtools::fBox2 r(0, (double)width(), 0, (double)height()); return r; }


    /**
     * Compute the intersection of two lines.
     *
     * @param   LA1     First point on line A.
     * @param   LA2     Second point on line A.
     * @param   LB1     First point on line B.
     * @param   LB2     Second point on line B.
     * @param [out] P   Buffer to put the intersection Point.
     *
     * @return  true the lines intersect and false if they are paralell.
     **/
    static inline bool intersection(mtools::fVec2 LA1, mtools::fVec2 LA2, mtools::fVec2 LB1, mtools::fVec2 LB2, mtools::fVec2 & P)
        {
        const double a1 = LA2.Y() - LA1.Y();   
        const double b1 = LA1.X() - LA2.X();   
        const double a2 = LB2.Y() - LB1.Y();   
        const double b2 = LB1.X() - LB2.X();   
        const double delta = a1*b2 - a2*b1; if (delta == 0) { return false; }
        const double c1 = LA1.X()*a1 + LA1.Y()*b1; const double c2 = LB1.X()*a2 + LB1.Y()*b2;
        P.X() = (b2*c1 - b1*c2) / delta;
        P.Y() = (a1*c2 - a2*c1) / delta;
        return true;
        }


    /**
     * fill the image with a single RGBc color.
     *
     * @param   color   The color to fill the image with.
     *
     * @return  The image for chaining.
     **/
    inline CImg<T> & clear(mtools::RGBc color)
        {
        if ((color.R == color.G) && (color.G == color.B))
            {
            if ((spectrum() <= 3) || ((spectrum() == 4)&&(color.A == color.R))) { fill(color.R); return(*this); }
            }
        if (spectrum() > 0)
            {
            get_shared_channel(0).fill(color.R);
            if (spectrum() > 1)
                {
                get_shared_channel(1).fill(color.G);
                if (spectrum() > 2)
                    {
                    get_shared_channel(2).fill(color.B);
                    for (int c = 3; c < spectrum(); c++) get_shared_channel(c).fill(color.A);
                    }
                }
            }
        return(*this);
        }


    /**
     * Fill the image with a checkerboard pattern. Default colors are two slightly distincts shades
     * of gray.
     *
     * @param   color1      The first color.
     * @param   color2      The second color.
     * @param   sizeSquare  The side lengh of an elementary square.
     *
     * @return  The image for chaining.
     **/
    CImg<T> & checkerboard(mtools::RGBc color1 = mtools::RGBc(200, 200, 200, 255), mtools::RGBc color2 = mtools::RGBc(220, 220, 220, 255), int sizeSquare = 50)
        {
        MTOOLS_ASSERT((spectrum() == 3) || (spectrum() == 4));
        const int lx = width();
        const int ly = height();
        MTOOLS_ASSERT(lx*ly > 0);
        int nx = 0, ny = 0, cy = 0, cx = 0;
        if (color1 == color2) 
            {   
            clear(color1);
            return(*this);
            }
        if (spectrum() == 3)
            {
            T * p0 = data(0, 0, 0, 0);
            T * p1 = data(0, 0, 0, 1);
            T * p2 = data(0, 0, 0, 2);
            for (int j = 0; j < ly; j++)
                {
                cx = cy; nx = 0;
                for (int i = 0; i < lx; i++)
                    {
                    if (cx == 0) { (*p0) = color1.R; (*p1) = color1.G; (*p2) = color1.B; } else { (*p0) = color2.R; (*p1) = color2.G; (*p2) = color2.B; }
                    if ((++nx) == sizeSquare) { cx = 1 - cx; nx = 0; }
                    ++p0; ++p1; ++p2;
                    }
                if ((++ny) == sizeSquare) { cy = 1 - cy; ny = 0; }
                }
            }
        else
            {
            T * p0 = data(0, 0, 0, 0);
            T * p1 = data(0, 0, 0, 1);
            T * p2 = data(0, 0, 0, 2);
            T * p3 = data(0, 0, 0, 3);
            for (int j = 0; j < ly; j++)
                {
                cx = cy; nx = 0;
                for (int i = 0; i < lx; i++)
                    {
                    if (cx == 0) { (*p0) = color1.R; (*p1) = color1.G; (*p2) = color1.B; (*p3) = color1.A; } else { (*p0) = color2.R; (*p1) = color2.G; (*p2) = color2.B; (*p3) = color1.A; }
                    if ((++nx) == sizeSquare) { cx = 1 - cx; nx = 0; }
                    ++p0; ++p1; ++p2; ++p3;
                    }
                if ((++ny) == sizeSquare) { cy = 1 - cy; ny = 0; }
                }
            }
        return(*this);
        }


     /**
     * Reverse an image along its Y axis
     **/
	inline CImg<T> &  reverseY()
		{
		mirror('y');
		return(*this);
		}


    /**
     * draw a point on the image.
     *
     * @param   P       The position of the point.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& drawPoint(mtools::iVec2 P, mtools::RGBc color, const float opacity = 1)
        {
        draw_point((int)P.X(),(int)P.Y(), color.buf(), opacity);
        return(*this);
        }


    /**
     * Draw a point using a circular pen. 
     *
     * @param   P       position of the pointi n the image.
     * @param   rad     radius of the circle pen (in pixel)
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  the image for chaining
     **/
    CImg<T>& drawPointCirclePen(mtools::iVec2 P,int rad, mtools::RGBc color, const float opacity = 1)
        {
        draw_circle((int)P.X(),(int)P.Y(), rad, color.buf(), opacity);
        return(*this);
        }


    /**
     * Draw a point using a square pen. 
     *
     * @param   P       position of the pointi n the image.
     * @param   rad     radius of the square pen (in pixel)
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  the image for chaining
     **/
    CImg<T>& drawPointSquarePen(mtools::iVec2 P, int rad, mtools::RGBc color, const float opacity = 1)
        {
        draw_rectangle((int)P.X() - rad, (int)P.Y() - rad, (int)P.X() + rad, (int)P.Y() + rad, color.buf(), opacity);
        return(*this);
        }


    /**
     * Draw a line.
     *
     * @param   P1      The first point
     * @param   P2      The second point.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& drawLine(mtools::iVec2 P1, mtools::iVec2 P2, mtools::RGBc color, float opacity = 1)
        {
        draw_line((int)P1.X(), (int)P1.Y(), (int)P2.X(), (int)P2.Y(), color.buf(), opacity);
        return(*this);
        }
    

    /**
     * Draw an horizontal line.
     *
     * @param   y       The y coordinate of the line.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& drawHorizontalLine(int y, mtools::RGBc color, float opacity = 1)
        {
        draw_line(0, y, width() , y , color.buf(), opacity);
        return(*this);
        }


    /**
     * Draw a vertical line.
     *
     * @param   x       The x coordinate of the line.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& drawVerticalLine(int x, mtools::RGBc color, float opacity = 1)
        {
        draw_line(x, 0, x, height(), color.buf(), opacity);
        return(*this);
        }


    /**
     * Draw line using a circle pen.
     *
     * @param   P1      The first point
     * @param   P2      The second point.
     * @param   rad     The circle radius.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& drawLineCirclePen(mtools::iVec2 P1,mtools::iVec2 P2,int rad, mtools::RGBc color, float opacity = 1)
        {
        // Bresenham's line algorithm, see wikipedia
        int x1 = (int)P1.X(), y1 = (int)P1.Y();
        int x2 = (int)P2.X(), y2 = (int)P2.Y();
        const bool steep = (fabs((double)(y2 - y1)) > fabs((double)(x2 - x1)));
        if (steep) { std::swap(x1, y1); std::swap(x2, y2); }
        if (x1 > x2) { std::swap(x1, x2); std::swap(y1, y2); }
        const double dx = x2 - x1;
        const double dy = fabs((double)(y2 - y1));
        double error = dx / 2.0f;
        const int ystep = (y1 < y2) ? 1 : -1;
        int y = (int)y1;
        const int maxX = (int)x2;
        for (int x = (int)x1; x<maxX; x++)
            {
            if (steep) { draw_circle(y, x, rad, color.buf(), opacity); }
            else { draw_circle(x, y, rad, color.buf(), opacity); }
            error -= dy;
            if (error < 0) { y += ystep; error += dx; }
            }
        return(*this);
        }


    /**
     * Draw line using a square pen.
     *
     * @param   P1      The first point
     * @param   P2      The second point.
     * @param   rad     The square radius.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& drawLineSquarePen(mtools::iVec2 P1, mtools::iVec2 P2, int rad, mtools::RGBc color, float opacity = 1)
    {
        // Bresenham's line algorithm, see wikipedia
        int x1 = (int)P1.X(), y1 = (int)P1.Y();
        int x2 = (int)P2.X(), y2 = (int)P2.Y();
        const bool steep = (fabs((double)(y2 - y1)) > fabs((double)(x2 - x1)));
        if (steep) { std::swap(x1, y1); std::swap(x2, y2); }
        if (x1 > x2) { std::swap(x1, x2); std::swap(y1, y2); }
        const double dx = x2 - x1;
        const double dy = fabs((double)(y2 - y1));
        double error = dx / 2.0f;
        const int ystep = (y1 < y2) ? 1 : -1;
        int y = (int)y1;
        const int maxX = (int)x2;
        for (int x = (int)x1; x<maxX; x++)
        {
            if (steep) { draw_rectangle(y - rad, x - rad, y + rad, x + rad, color.buf(), opacity); }
            else { draw_rectangle(x - rad, y - rad, x + rad, y + rad, color.buf(), opacity); }
            error -= dy;
            if (error < 0) { y += ystep; error += dx; }
        }
        return(*this);
    }


    /**
     * Return a (static) font with a given height and possibly variable width. The font is created
     * on first use and destroyed when the program quits. This method is thread-safe.
     *
     * @param   font_height     Height of the font (between 5 and 256).
     * @param   variable_width  True if the font should have variable width.
     *
     * @return  The font. If font_height < 5 (resp. > 256) return a font of size 5 (resp. 256).
     **/
    static const CImgList<floatT> & getFont(unsigned int font_height, bool variable_width = true)
    {
        static std::mutex mut;      // mutex for sync.
        mut.lock(); // enter critical section
        static std::unique_ptr< CImgList<floatT> > font_tab[257];
        static std::unique_ptr< CImgList<floatT> > vfont_tab[257];
        font_height = (font_height < 5) ? 5 : (font_height>256 ? 256 : font_height);
        if (variable_width)
            {
            if (vfont_tab[font_height] == nullptr)
                { // create the variable size font of size fontheight
                const unsigned int  ref_height = font_height <= 13 ? 13 : font_height <= 28 ? 24 : font_height <= 32 ? 32 : 57, padding_x = font_height <= 18 ? 1 : font_height <= 32 ? 2 : 3;
                vfont_tab[font_height] = std::unique_ptr< CImgList<floatT> >(new CImgList<floatT>(CImgList<floatT>::font(ref_height, true)));
                CImgList<floatT> & ff = *(vfont_tab[font_height]);
                ff[0].assign(1, font_height);
                if (ref_height == font_height) cimglist_for(ff, l) ff[l].resize(ff[l]._width + padding_x, -100, -100, -100, 0);
                if (ff[0]._spectrum<3) cimglist_for_in(ff, 0, 255, l) ff[l].resize(-100, -100, 1, 3);
                if (ref_height != font_height)
                    {
                    cimglist_for(ff, l)
                        {
                        CImg<floatT> & c = ff[l];
                        if (c._height != font_height) 
                            {
                            c.resize(cimg::max(1U, c._width*font_height / c._height), font_height, -100, -100, c._height>font_height ? 2 : 3);
                            c.resize(c._width + padding_x, -100, -100, -100, 0);
                            }
                        }
                    }
                }
            mut.unlock(); // leave critical section
            return(*(vfont_tab[font_height]));
            }
        if (font_tab[font_height] == nullptr)
            { // create the fixed size font of size fontheight
            const unsigned int  ref_height = font_height <= 13 ? 13 : font_height <= 28 ? 24 : font_height <= 32 ? 32 : 57;
            font_tab[font_height] = std::unique_ptr< CImgList<floatT> >(new CImgList<floatT>(CImgList<floatT>::font(ref_height, false)));
            CImgList<floatT> & ff = *(font_tab[font_height]);
            ff[0].assign(1, font_height);
            if (ff[0]._spectrum<3) cimglist_for_in(ff, 0, 255, l) ff[l].resize(-100, -100, 1, 3);
            if (ref_height != font_height)
                {
                cimglist_for(ff, l) ff[l].resize(cimg::max(1U, ff[l]._width*font_height / ff[l]._height), font_height, -100, -100, ff[0]._height>font_height ? 2 : 5);
                }
            }
        mut.unlock(); // leave critical section
        return(*(font_tab[font_height]));
    }


    /**
     * Compute the font size in order to adjust a text to a given box. Set boxsize.X() (resp
     * boxsize.Y() ) for removing a constrain on X (resp Y). The smallest value returned is 5 and
     * the maximum value is 256 (even if the text does not fit). Use minheight and maxheight for
     * addtionnal constrained on the size of the font.
     *
     * @param   text            The text.
     * @param   boxsize         the size of the box.
     * @param   variable_width  true to use a variable width font.
     * @param   minheight       The optional minimum font height requested.
     * @param   maxheight       The optional maximum font height requsted.
     *
     * @return  the font height.
     **/
    static unsigned int computeFontSize(const std::string & text, mtools::iVec2 boxsize, bool variable_width = true, unsigned int minheight = 5, unsigned int maxheight = 256)
        {
        if ((maxheight < 6) || ((boxsize.Y() >= 0) && (boxsize.Y()<6))) { return 5; }
        if (minheight > 256) { return 256; }
        maxheight = ((maxheight > 256) ? 256 : maxheight);
        minheight = ((minheight < 6) ? 5 : minheight);
        if (maxheight <= minheight) { return maxheight; }
        if ((text.length() == 0) || ((boxsize.X()<0) && (boxsize.Y()<0))) { return maxheight; }
        if ((boxsize.Y() >= 0) && (boxsize.Y()<maxheight)) { maxheight = (unsigned int)boxsize.Y(); }
        mtools::iVec2 TS = getTextDimensions(text, maxheight, variable_width);
        if (((boxsize.X()<0) || (TS.X() <= boxsize.X())) && ((boxsize.Y()<0) || (TS.Y() <= boxsize.Y()))) { return maxheight; }
        TS = getTextDimensions(text, minheight, variable_width);
        if (!(((boxsize.X()<0) || (TS.X() <= boxsize.X())) && ((boxsize.Y()<0) || (TS.Y() <= boxsize.Y())))) { return minheight; }
        while (maxheight - minheight >1)
            {
            unsigned int f = (maxheight + minheight) / 2;
            TS = getTextDimensions(text, f, variable_width);
            if (((boxsize.X()<0) || (TS.X() <= boxsize.X())) && ((boxsize.Y()<0) || (TS.Y() <= boxsize.Y()))) { minheight = f; }
            else { maxheight = f; }
            }
        return minheight;
        }


    /**
     * Compute the dimension of a text given a font.
     *
     * @param   text                The text.
     * @param   font_height         The height of the font.
     * @param   variable_width      True if the font has variable width.
     *
     * @return  The text dimensions as a 2d vector.
     **/
    static mtools::iVec2 getTextDimensions(const std::string & text, unsigned int font_height, bool variable_width = true)
        {
        if (text.length() == 0) {return mtools::iVec2(0,0); }
        const CImgList<floatT> & ff = getFont(font_height, variable_width);
        int x = 0, y = 0, w = 0;
        unsigned char c = 0;
        for (size_t i = 0; i < text.length(); ++i)
            {
            c = text[i];
            switch (c)
                {
                case '\n': y += ff[0]._height; if (x>w) w = x; x = 0; break;
                case '\t': x += 4 * ff[' ']._width; break;
                default: if (c<ff._width) x += ff[c]._width;
                }
            }
        if (x != 0 || c == '\n') { if (x>w) w = x; y += ff[0]._height; }
        return mtools::iVec2(w,y);
        }


    /**
     * Draw a text into an image. This method is thread-safe, which is not the case of the initial
     * cimg method.
     *
     * @param   text            The text to draw.
     * @param   Pos             Coordinate of the reference point for the text.
     * @param   xcentering      X pos. of reference point ("l" = left, "c" = center, "r" = right).
     * @param   ycentering      Y pos. of reference point ("t" = top, "c" = center, "b" = bottom).
     * @param   fontsize        The font size to use.
     * @param   variable_width  true to use a variable width font.
     * @param   color           The color.
     * @param   opacity         The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& drawText(const std::string & text, mtools::iVec2 Pos, char xcentering, char ycentering, int fontsize, bool variable_width, mtools::RGBc color, double opacity = 1.0)
    {
        mtools::iVec2 TS(0,0);
        if (((xcentering == 'c') || (xcentering == 'C')) || ((xcentering == 'r') || (xcentering == 'R')) || ((ycentering == 'c') || (ycentering == 'C')) || ((ycentering == 'b') || (ycentering == 'B'))) {TS = getTextDimensions(text, fontsize, variable_width);}
        if ((xcentering == 'c') || (xcentering == 'C')) { Pos.X() -= TS.X() / 2; }
        if ((xcentering == 'r') || (xcentering == 'R')) { Pos.X() -= TS.X(); }
        if ((ycentering == 'c') || (ycentering == 'C')) { Pos.Y() -= TS.Y() / 2; }
        if ((ycentering == 'b') || (ycentering == 'B')) { Pos.Y() -= TS.Y(); }
        //draw_text((int)Pos.X(), (int)Pos.Y(), text.c_str(), color.buf(), 0, (float)opacity, getFont(fontsize, variable_width));
        _draw_text<unsigned char, unsigned char>((int)Pos.X(), (int)Pos.Y(), text.c_str(), color.buf(), 0, (float)opacity, getFont(fontsize, variable_width), false);        
        // THIS VERSION BUG IN 32BIT MODE (BUT NOT IN 64BIT MODE) AND I DO NOT UNDERSTAND WHY....
        // draw_text(x,y,"%s",color.buf(),0,(float)opacity,safeFont(fontsize,is_variable_width),text); 
        return(*this);
    }


    /**
     * Draw a text into an image using aboslute coordinate for the text position. 
     *
     * @param   R               the absolute range represented in the image.
     * @param   text            The text to draw.
     * @param   Pos             Coordinate of the reference point for the text.
     * @param   xcentering      X pos. of reference point ("l" = left, "c" = center, "r" = right).
     * @param   ycentering      Y pos. of reference point ("t" = top, "c" = center, "b" = bottom).
     * @param   fontsize        The font size to use (in pixel)
     * @param   variable_width  true to use a variable width font.
     * @param   color           The color.
     * @param   opacity         The opacity.
     *
     * @return  the image for chaining.
     **/
     CImg<T>& fBox2_drawText(const mtools::fBox2 & R,const std::string & text, mtools::fVec2 Pos, char xcentering, char ycentering, int fontsize, bool variable_width, mtools::RGBc color, double opacity = 1.0)
        {
        drawText(text, getImageCoord(R,Pos), xcentering, ycentering, fontsize, variable_width, color, opacity);
        return(*this);
        }

  
    /**
     * Compute the font size in order to adjust a text to a given box in aboslute coordinate.  Set
     * boxsize.X() (resp boxsize.Y() ) for removing a constrain on X (resp Y). The smallest value
     * returned is 5 and the maximum value is 256 (even if the text does not fit). Use minheight and
     * maxheight for addtionnal constrained on the size of the font.
     *
     * @param   R               the absolute range represented in the image.
     * @param   text            The text.
     * @param   boxsize         the size of the box (in absolute size)
     * @param   variable_width  true to use a variable width font.
     * @param   minheight       The optional minimum font height requested.
     * @param   maxheight       The optional maximum font height requsted.
     *
     * @return  the font height.
     **/
     unsigned int fBox2_computeFontSize(const mtools::fBox2 & R,const std::string & text, mtools::fVec2 boxsize, bool variable_width = true, unsigned int minheight = 5, unsigned int maxheight = 256)
        {
        return computeFontSize(text, getImageCoord(R, boxsize) - getImageCoord(R, { 0, 0 }), variable_width, minheight, maxheight);
        }


     /**
      * fill a portion using the flood fill algorithm.
      *
      * @param  R                   the absolute range represented in the image.
      * @param  Pos                 The position.
      * @param  color               The color.
      * @param  opacity             The opacity.
      * @param  sigma               The tolerance (see CImg::draw_fill method).
      * @param  is_high_connexity   The is high connexity (true = 8, false = 4).
      *
      * @return The image for chaining.
      **/
     CImg<T>& fBox2_floodFill(const mtools::fBox2 & R, mtools::fVec2 Pos, mtools::RGBc color, const float opacity = 1, const float sigma = 0, const bool is_high_connexity = false)
        {
         mtools::iVec2 P = getImageCoord(R, Pos);
         draw_fill((int)P.X(), (int)P.Y(), color.buf(), opacity, sigma, false);
         return(*this);
        }


     /**
      * draw a point on the image. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  P       The position of the point.
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return the image for chaining.
      **/
     CImg<T>& fBox2_drawPoint(const mtools::fBox2 & R, mtools::fVec2 P, mtools::RGBc color, const float opacity = 1)
        {
        drawPoint(getImageCoord(R,P), color, opacity);
        return(*this);
        }


     /**
      * Draw a point using a circular pen. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  P       position of the pointi n the image.
      * @param  rad     radius of the circle pen (in pixel).
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return the image for chaining.
      **/
     CImg<T>& fBox2_drawPointCirclePen(const mtools::fBox2 & R, mtools::fVec2 P, int rad, mtools::RGBc color, const float opacity = 1)
        {
        drawPointCirclePen(getImageCoord(R, P), rad, color, opacity);
        return(*this);
        }


     /**
      * Draw a point using a square pen. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  P       position of the pointi n the image.
      * @param  rad     radius of the square pen (in pixel)
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return the image for chaining.
      **/
     CImg<T>& fBox2_drawPointSquarePen(const mtools::fBox2 & R, mtools::fVec2 P, int rad, mtools::RGBc color, const float opacity = 1)
        {     
        drawPointSquarePen(getImageCoord(R, P), rad, color, opacity);
        return(*this);
        }


     /**
      * Draw a line. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  P1      The first point.
      * @param  P2      The second point.
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return The image for chaining.
      **/
     CImg<T>& fBox2_drawLine(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, mtools::RGBc color, float opacity = 1)
        {
        drawLine(getImageCoord(R, P1), getImageCoord(R, P2), color, opacity);
        return(*this);
        }


     /**
      * Draw an horizontal line. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  y       The y coordinate of the line.
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return The image for chaining.
      **/
     CImg<T>& fBox2_drawHorizontalLine(const mtools::fBox2 & R, double y, mtools::RGBc color, float opacity = 1)
        {
        drawHorizontalLine((int)getImageCoord(R, { 0, y }).Y(), color, opacity);
        return(*this);
        }


     /**
      * Draw a vertical line. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  x       The x coordinate of the line.
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return The image for chaining.
      **/
     CImg<T>& fBox2_drawVerticalLine(const mtools::fBox2 & R, double x, mtools::RGBc color, float opacity = 1)
        {
        drawVerticalLine((int)getImageCoord(R, { x, 0 }).X(), color, opacity);
        return(*this);
        }


     /**
      * Draw line using a circle pen. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  P1      The first point.
      * @param  P2      The second point.
      * @param  rad     The circle radius (in pixels).
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return The image for chaining.
      **/
     CImg<T>& fBox2_drawLineCirclePen(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, int rad, mtools::RGBc color, float opacity = 1)
        {
        drawLineCirclePen(getImageCoord(R, P1), getImageCoord(R, P2), rad, color, opacity);
        return(*this);
        }


     /**
      * Draw line using a square pen. Use absolute coordinate.
      *
      * @param  R       the absolute range represented in the image.
      * @param  P1      The first point.
      * @param  P2      The second point.
      * @param  rad     The square radius (in pixel).
      * @param  color   The color.
      * @param  opacity The opacity.
      *
      * @return The image for chaining.
      **/
     CImg<T>& fBox2_drawLineSquarePen(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, int rad, mtools::RGBc color, float opacity = 1)
        {
        drawLineSquarePen(getImageCoord(R, P1), getImageCoord(R, P2), rad, color, opacity);
        return(*this);
        }


    /**
     * draw a 2D bezier spline.
     *
     * @param   R           the absolute range represented in the image.
     * @param   P1          Start point.
     * @param   PA          First control point.
     * @param   PB          Second control point.
     * @param   P2          End point.
     * @param   color       The color.
     * @param   opacity     The opacity.
     * @param   precision   The precision (affects the number of point drawn).
     *
     * @return  The image for chaining.
     **/
	CImg<T>& fBox2_draw_spline(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 PA, mtools::fVec2 PB, mtools::fVec2 P2,mtools::RGBc color,float opacity = 1, float precision = 0.25)
		{
        mtools::iVec2 Q1 = getImageCoord(R, P1);
        mtools::iVec2 QA = getImageCoord(R, PA);
        mtools::iVec2 QB = getImageCoord(R, PB);
        mtools::iVec2 Q2 = getImageCoord(R, P2);
		float u1 = (float)(QA.X() - Q1.X()), v1 = (float)(QA.Y() - Q1.Y());
		float u2 = (float)(Q2.X() - QB.X()), v2 = (float)(Q2.Y() - QB.Y());
        draw_spline((int)Q1.X(), (int)Q1.Y(), u1, v1, (int)Q2.X(), (int)Q2.Y(), u2, v2, color.buf(), opacity, precision);
		return(*this);
		}


    /**
     * Draw a triangle in absolute coordinate.
     *
     * @param   R       the absolute range represented in the image.
     * @param   P1      First point point (abs coord).
     * @param   P2      Second point point (abs coord).
     * @param   P3      Third point (abs coord).
     * @param   color   The color.
     * @param   opacity The opacity.
     * @param   filled  true if the triangle should be filled.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& fBox2_draw_triangle(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, mtools::fVec2 P3, mtools::RGBc color, float opacity = 1.0, bool filled = true)
		{
        mtools::iVec2 Q1 = getImageCoord(R, P1);
        mtools::iVec2 Q2 = getImageCoord(R, P2);
        mtools::iVec2 Q3 = getImageCoord(R, P3);
        if (filled) draw_triangle((int)Q1.X(), (int)Q1.Y(), (int)Q2.X(), (int)Q2.Y(), (int)Q3.X(), (int)Q3.Y(), color.buf(), opacity);
        else
            {
            drawLine(Q1, Q2, color, opacity);
            drawLine(Q2, Q3, color, opacity);
            drawLine(Q3, Q1, color, opacity);
            }
        return(*this);
		}


    /**
     * Draw a rectangle in absolute coordinate.
     *
     * @param   R       the absolute range represented in the image.
     * @param   P1      First corner point (abs coord).
     * @param   P2      Second corner point (abs coord).
     * @param   color   The color.
     * @param   opacity The opacity.
     * @param   filled  true if the rectangle should be filled.
     *
     * @return  The image for chaining.
     **/
    CImg<T>& fBox2_draw_rectangle(const mtools::fBox2 & R, mtools::fVec2 P1, mtools::fVec2 P2, mtools::RGBc color, float opacity = 1.0,bool filled = true)
		{
        mtools::iVec2 Q1 = getImageCoord(R, P1);
        mtools::iVec2 Q2 = getImageCoord(R, P2);
        if (filled)  draw_rectangle((int)Q1.X(), (int)Q1.Y(), (int)Q2.X(), (int)Q2.Y(), color.buf(), opacity);
        else
            {
            drawLine(Q1, { Q1.X(),Q2.Y() }, color, opacity);
            drawLine(Q1, { Q2.X(),Q1.Y() }, color, opacity);
            drawLine(Q2, { Q1.X(),Q2.Y() }, color, opacity);
            drawLine(Q2, { Q2.X(),Q1.Y() }, color, opacity);
            }
		return(*this);
		}


    /**
     * Draw  a circle in absolute coordinate. This may become an ellispe if the ratio is not 1:1.
     *
     * @param   R       the absolute range represented in the image.
     * @param   C       The center of the circle (abs coord).
     * @param   rad     The radius of the circle (abs size).
     * @param   color   The color.
     * @param   opacity The opacity.
     * @param   filled  true if the circle should be filled.
     *
     * @return  The image for chaining.
     **/
	CImg<T>& fBox2_draw_circle(const mtools::fBox2 & R,mtools::fVec2 C,double rad,mtools::RGBc color,float opacity = 1,bool filled = true)
		{
        mtools::iVec2 Q = getImageCoord(R, C);
        float rx = (float)((getImageCoord(R, { rad, 0 }) - getImageCoord(R, { 0, 0 })).X());
        float ry = (float)((getImageCoord(R, { 0,0}) - getImageCoord(R, { 0, rad })).Y());
		if (filled)
			{
            draw_ellipse((int)Q.X(), (int)Q.Y(), rx, ry, 0, color.buf(), opacity);
			}
		else
			{
            draw_ellipse((int)Q.X(), (int)Q.Y(), rx, ry, 0, color.buf(), opacity, -1);
			}
		return(*this);
		}


    /**
     * Draw the axes on an image.
     *
     * @param   R           The rect representing the range of the image. 
     * @param   color       The color.
     * @param   opacity     The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& fBox2_drawAxes(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Black,float opacity = 1.0)
		{
		fBox2_drawHorizontalLine(R,0,color,opacity);
		fBox2_drawVerticalLine(R,0,color,opacity);
		return(*this);
		}


    /**
     * Draw the integer grid (ie line of the form (x,j) and (i,y)) where (i,j) are integers.
     *
     * @param   R           The rect representing the range of the image.
     * @param   color       The color.
     * @param   opacity     The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& fBox2_drawGrid(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Gray, float opacity = 0.5)
		{
		if (R.lx() <=  width()/2)   {mtools::int64 i= (mtools::int64)R.min[0] - 2; while(i < (mtools::int64)R.max[0] + 2) {fBox2_drawVerticalLine(R,(double)i,color,opacity); i++;}}
		if (R.ly() <=  height()/2)  {mtools::int64 j= (mtools::int64)R.min[1] - 2; while(j < (mtools::int64)R.max[1] + 2) {fBox2_drawHorizontalLine(R,(double)j,color,opacity); j++;}}
		return(*this);
		}


    /**
     * Draw the cells around integer points ie draw line of the form (x,j+1/2) and (i+1/2,y)
     *
     * @param   R           The rect representing the range of the image.
     * @param   color       The color.
     * @param   opacity     The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& fBox2_drawCells(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Gray, float opacity = 0.5)
		{
		if (R.lx() <=  width()/2)  {mtools::int64 i= (mtools::int64)R.min[0]-2; while(i < (mtools::int64)R.max[0]+2) {fBox2_drawVerticalLine(R,i-0.5,color,opacity); i++;}}
		if (R.ly() <=  height()/2) {mtools::int64 j= (mtools::int64)R.min[1]-2; while(j < (mtools::int64)R.max[1]+2) {fBox2_drawHorizontalLine(R,j-0.5,color,opacity); j++;}}
		return(*this);
		}


    /**
     * Add the graduations on the axis.
     *
     * @param   R       The rect representing the range of the image.
     * @param   scaling The scaling factor for the graduation size (default 1.0). Note that there is
     *                  already a scaling of the graduation w.r.t. the image size. This parameter
     *                  enables to multiply the automatic scaling with a new factor.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& fBox2_drawGraduations(const mtools::fBox2 & R, float scaling = 1.0, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 0.7)
		{
        scaling = scaling*((float)(std::sqrt(width()*height()) / 1000.0));
        int gradsize = (int)(3 * scaling); if (gradsize == 0) { gradsize = 1; }
		const int winx = width(), winy = height();
		int py = winy-1 - (int)ceil(((-R.min[1])/(R.max[1]-R.min[1]))*winy - ((double)1.0/2.0));
		int px = (int)ceil(((-R.min[0])/(R.max[0]-R.min[0]))*winx - ((double)1.0/2.0));
		if ((px > -1) && (px < winx)) 
			{
			int l,zz; double k,xx,kk,pp,xx2,op,v1,v2; 
			op = ::log10(R.ly()); if (op<0) {l = ((int)(op))-1;} else {l = ((int)(op));}
			k = ::pow(10.0,(double)(l));
			v1 = floor(R.min[1]/k); v1 = v1 - 1; v2 = floor(R.max[1]/k); 
			v2 = v2 + 1;
			kk = k; pp = kk/5;
			if ((v2 - v1) < 5) {kk = k/2; pp = kk/5;} else {if ((v2 - v1) > 8) {kk = k*2; pp = kk/2; v1 = ((v1/2)*2) - 2;}}
			xx =k*v1; xx2 = k*v1;
			while(xx2 <= (R.max[1] + 2*k))
				{
                xx = xx + kk; xx2 = xx2 + pp; 
                zz = (int)R.absToPixel(mtools::fVec2(0, xx),mtools::iVec2(winx, winy)).Y();
				if ((zz >= -10) && (zz < winy+10)) {if (xx!=0) {draw_line(px-2*gradsize,zz,px+2*gradsize,zz,color.buf(),opacity);}}
                zz = (int)R.absToPixel(mtools::fVec2(0, xx2), mtools::iVec2(winx, winy)).Y();
				if ((zz > -2) && (zz < winy+1)) {if (xx2 != 0) {draw_line(px-gradsize,zz,px+gradsize,zz,color.buf(),opacity);}}
				}
			}
		if ((py > -1) && (py < winy))
			{
		    int l,zz; double k,xx,kk,pp,xx2,op,v1,v2;
            op = ::log10(R.lx()); if (op<0) {l = ((int)op)-1;} else {l = (int)op;}
			k = ::pow(10.0,(double)(l));
			v1 = floor(R.min[0]/k);  v1 = v1 - 1; v2 = floor(R.max[0]/k);  v2 = v2 + 1;
			kk = k; pp = kk/5;
			if ((v2 - v1) < 5) {kk = k/2; pp = kk/5;} else {if ((v2 - v1) > 8) {kk = k*2; pp = kk/2; v1 = ((v1/2)*2) - 2;}}
			xx =k*v1; xx2 = k*v1;
			while(xx2 <= (R.max[0] + 2*k))
				{
			    xx = xx + kk; xx2 = xx2 + pp; 
                zz = (int)R.absToPixel(mtools::fVec2(xx, 0), mtools::iVec2(winx, winy)).X();
				if ((zz >= -30) && (zz < winx+30)) {if (xx !=0) {draw_line(zz,py-2*gradsize,zz,py+2*gradsize,color.buf(),opacity);}}
                zz = (int)R.absToPixel(mtools::fVec2(xx2, 0), mtools::iVec2(winx, winy)).X();
				if ((zz > -2) && (zz < winx+1))  {if (xx2 != 0) {draw_line(zz,py-gradsize,zz,py+gradsize,color.buf(),opacity);}}
				}	
            }
		return(*this);
		}


    /**
     * Draw the numbering on the axis.
     *
     * @param   R       The rect representing the range of the image.
     * @param   scaling The scaling factor for the numbers. (default 1.0). Note that there is already
     *                  a scaling of the font w.r.t. the image size. This parameter enables to
     *                  multiply this automatic scaling with a new factor.
     * @param   color   The color.
     * @param   opacity The opacity.
     *
     * @return  the image for chaining.
     **/
    CImg<T>& fBox2_drawNumbers(const mtools::fBox2 & R, float scaling = 1.0, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 0.7)
		{
        scaling = scaling*((float)(std::sqrt(width()*height()) / 1000.0));
        int gradsize = (int)(3 * scaling); if (gradsize == 0) { gradsize = 1; }
        int fontsize = 5 + (int)(10 * scaling);
		const int winx = width(), winy = height();
		int py = winy-1 - (int)ceil(((-R.min[1])/(R.max[1]-R.min[1]))*winy - ((double)1.0/2.0));
		int px = (int)ceil(((-R.min[0])/(R.max[0]-R.min[0]))*winx - ((double)1.0/2.0));
		if ((px > -1) && (px < winx)) 
			{
			int l,zz; double k,xx,kk,pp,xx2,op,v1,v2; 
			op = ::log10(R.ly()); if (op<0) {l = ((int)(op))-1;} else {l = ((int)(op));}
			k = ::pow(10.0,(double)(l));
			v1 = floor(R.min[1]/k); v1 = v1 - 1; v2 = floor(R.max[1]/k); 
			v2 = v2 + 1;
			kk = k; pp = kk/5;
			if ((v2 - v1) < 5) {kk = k/2; pp = kk/5;} else {if ((v2 - v1) > 8) {kk = k*2; pp = kk/2; v1 = ((v1/2)*2) - 2;}}
			xx =k*v1; xx2 = k*v1;
			while(xx2 <= (R.max[1] + 2*k))
				{
				xx = xx + kk; xx2 = xx2 + pp; 
                zz = (int)R.absToPixel(mtools::fVec2(0, xx), mtools::iVec2(winx, winy)).Y();
				if ((zz >= -10) && (zz < winy+10))
					{
					if (xx!=0) 
						{
						std::string tt = mtools::doubleToStringNice(xx); 
						if ((zz < py -3) || (zz> py+3)) {drawText(tt, mtools::iVec2(px + 4*gradsize , zz), 'l', 'c', fontsize, true, color, opacity);}
						}
					}
				}
			}
		if ((py > -1) && (py < winy))
			{
			int l,zz; double k,xx,kk,pp,xx2,op,v1,v2;
            op = ::log10(R.lx()); if (op<0) {l = ((int)op)-1;} else {l = (int)op;}
			k = ::pow(10.0,(double)(l));
			v1 = floor(R.min[0]/k);  v1 = v1 - 1; v2 = floor(R.max[0]/k);  v2 = v2 + 1;
			kk = k; pp = kk/5;
			if ((v2 - v1) < 5) {kk = k/2; pp = kk/5;} else {if ((v2 - v1) > 8) {kk = k*2; pp = kk/2; v1 = ((v1/2)*2) - 2;}}
			xx =k*v1; xx2 = k*v1;
			while(xx2 <= (R.max[0] + 2*k))
				{
			    xx = xx + kk; xx2 = xx2 + pp; 
                zz = (int)R.absToPixel(mtools::fVec2(xx, 0), mtools::iVec2(winx, winy)).X();
			    if ((zz >= -30) && (zz < winx+30))
					{
					if (xx!=0) 
						{
                        std::string tt = mtools::doubleToStringNice(xx);
						if ((zz < px -3) || (zz> px+3)) {drawText(tt, mtools::iVec2(zz, py + 4*gradsize), 'c', 't', fontsize, true, color, opacity);}
						}
					}
				}
            }
		return (*this);
		}





#endif
/* end of file */

