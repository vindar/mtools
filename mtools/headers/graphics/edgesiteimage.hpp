/** @file edgesiteimage.hpp */
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
#include "../maths/vec.hpp"
#include "customcimg.hpp"


namespace mtools
{
    /**
     * Class for creating an image of a site of Z^2
     *
     * - There can be a center site or nothing.
     * - In each for direction, there can either be an outgoing arrow, an ingoing arrow, an double
     * arrow, a simple edge or nothing at all.
     * - There can be a text in the center of the image (inside the site).
     * - There can be a text next to each arrow/edge in each direction.
     * - The color of each element can be customized.
     *
     * Once every parameters of the image are set, it is created using the makeImage() method.
     *
     * @par Example
     * @code{.cpp}
     * int main()
     * {
     *  EdgeSiteImage ES;
     *  ES.site(true, RGBc::c_Gray);		       // draw a gray center site
     *  ES.left(ES.NOEDGE, RGBc::c_Green);         // green edge on the left
     *  ES.right(ES.EDGE);	            	       // outgoing arrow on the right (black)
     *  ES.up(ES.ARROW_OUTGOING);			       // ingoing arrow up (black)
     *  ES.text("Center text", RGBc::c_Purple);	   // the center text  in purple
     *  ES.textup("up text");					   // up text (black)
     *  ES.textleft("left text");				   // left text (black)
     *  ES.textdown("this is my\ntext :-)");       // another text down
     *  Img<unsigned char> image;   // the cim image to draw into
     *  ES.makeImage(image, 200, 300);			   // draw it with size (200x300)
     *  CImgDisplay Disp(image); while (!Disp.is_closed()) { Sleep(1); }	// display on the screen
     *  return 0
     *  }
     * @endcode.
     **/
	class EdgeSiteImage
	{

    public:

        /**
         * Values that represent type of edges to draw
         **/
        enum TypeEdge
            {
            NOEDGE,                ///< no edge
            EDGE,                  ///< a simple edge
            ARROW_OUTGOING,        ///< an outgoing arrow
            ARROW_INGOING,         ///< an ingoing arrow
            ARROW_BOTH             ///< both outgoing and ingoing arrow
            };


    /**
     * Constructor. By default, an empty red site with a white transparent background.
     **/
	EdgeSiteImage() : _up(NOEDGE),_down(NOEDGE),_left(NOEDGE),_right(NOEDGE),
                      _site(false),_cbk(RGBc::c_TransparentWhite), _csite(RGBc::c_Red),
					  _cup(RGBc::c_Black),_cdown(RGBc::c_Black),_cleft(RGBc::c_Black),_cright(RGBc::c_Black),_ctext(RGBc::c_Black),
					  _ctextup(RGBc::c_Blue),_ctextdown(RGBc::c_Blue),_ctextleft(RGBc::c_Blue),_ctextright(RGBc::c_Blue)
	{return;}


    /**
     * Default copy constructor.
     **/
    EdgeSiteImage(const EdgeSiteImage &) = default;


    /**
     * Default assignment operator.
     **/
    EdgeSiteImage & operator=(const EdgeSiteImage &) = default;


    /**
     * Set whether the site at the center is occupied (default: not occupied, color site = red)
     **/
	inline EdgeSiteImage & site(bool isSite) {_site = isSite; return(*this);}


    /**
     * Set whether the site at the center is occupied and its color (default: not occupied, color
     * site = red)
     **/
	inline EdgeSiteImage & site(bool isSite,RGBc color) {_site = isSite; _csite = color;  return(*this);}


    /**
     * Set the site color (default: red).
     **/
	inline EdgeSiteImage & siteColor(RGBc color) {_csite = color;  return(*this);}


    /**
     * Set the background color (default white). Choose a color with R = G = B slightly faster
     * drawing of the image.
     **/
	inline EdgeSiteImage & bkColor(RGBc color) {_cbk = color; return(*this);}


    /**
     * Set the type of edge in the up direction. (by default : NOEDGE)
     **/
    inline EdgeSiteImage & up(TypeEdge type) { MTOOLS_ASSERT((type >= 0) && (type <5)); _up = type; return(*this); }


    /**
     * Set the type of edge in the up direction and the color. (by default : NOEDGE, color black)
     **/
    inline EdgeSiteImage & up(TypeEdge type, RGBc color) { MTOOLS_ASSERT((type >= 0) && (type <5)); _up = type; _cup = color; return(*this); }


    /**
     * Set the color of the edge in the up direction (by default : color black)
     **/
	inline EdgeSiteImage & upColor(RGBc color) {_cup = color; return(*this);}


    /**
     * Set the type of edge in the down direction. (by default : NOEDGE)
     **/
    inline EdgeSiteImage & down(TypeEdge type) { MTOOLS_ASSERT((type >= 0) && (type <5)); _down = type; return(*this); }


    /**
     * Set the type of edge in the down direction and the color. (by default : NOEDGE, color black)
     **/
    inline EdgeSiteImage & down(TypeEdge type, RGBc color) { MTOOLS_ASSERT((type >= 0) && (type <5)); _down = type; _cdown = color; return(*this); }


    /**
     * Set the color of the edge in the down direction (by default : color black)
     **/
	inline EdgeSiteImage & downColor(RGBc color) {_cdown = color; return(*this);}


    /**
     * Set the type of edge in the left direction. (by default : NOEDGE)
     **/
    inline EdgeSiteImage & left(TypeEdge type) { MTOOLS_ASSERT((type >= 0) && (type <5)); _left = type; return(*this); }


    /**
     * Set the type of edge in the left direction and the color. (by default : NOEDGE, color black)
     **/
    inline EdgeSiteImage & left(TypeEdge type, RGBc color) { MTOOLS_ASSERT((type >= 0) && (type <5)); _left = type; _cleft = color; return(*this); }


    /**
     * Set the color of the edge in the left direction (by default : color black)
     **/
	inline EdgeSiteImage & leftColor(RGBc color) {_cleft = color; return(*this);}


    /**
     * Set the type of edge in the right direction. (by default : NOEDGE)
     **/
    inline EdgeSiteImage & right(TypeEdge type) { MTOOLS_ASSERT((type >= 0) && (type <5)); _right = type; return(*this); }


    /**
     * Set the type of edge in the right direction and the color. (by default : NOEDGE, color black)
     **/
    inline EdgeSiteImage & right(TypeEdge type, RGBc color) { MTOOLS_ASSERT((type >= 0) && (type <5)); _right = type; _cright = color; return(*this); }


    /**
     * Set the color of the edge in the right direction (by default : color black)
     **/
	inline EdgeSiteImage & rightColor(RGBc color) {_cright = color; return(*this);}


    /**
     * Set the text drawn at the center of the image (default nothing).
     **/
	inline EdgeSiteImage & text(const std::string & txt) {_text = txt; return(*this);}


    /**
     * Set the text drawn at the center of the image and its color (default nothing, color
     * black).
     **/
    inline EdgeSiteImage & text(const std::string & txt,RGBc color) {_text = txt; _ctext = color; return(*this);}


    /**
     * Set the color of the text drawn at the center (default black).
     **/
	inline EdgeSiteImage & textColor(RGBc color) {_ctext = color; return(*this);}


    /**
     * Set the text drawn next to the up direction (default nothing).
     **/
	inline EdgeSiteImage & textup(const std::string & txt) {_textup = txt; return(*this);}


    /**
     * Set the text drawn next to the up direction and its color (default nothing, color blue).
     **/
    inline EdgeSiteImage & textup(const std::string & txt,RGBc color) {_textup = txt; _ctextup = color; return(*this);}


    /**
     * Set the color of the text drawn next to the up direction (default blue).
     **/
	inline EdgeSiteImage & textupColor(RGBc color) {_ctextup = color; return(*this);}


    /**
     * Set the text drawn next to the down direction (default nothing).
     **/
    inline EdgeSiteImage & textdown(const std::string & txt) {_textdown = txt; return(*this);}


    /**
     * Set the text drawn next to the down direction and its color (default nothing, color blue).
     **/
	inline EdgeSiteImage & textdown(const std::string & txt,RGBc color) {_textdown = txt; _ctextdown = color; return(*this);}


    /**
     * Set the color of the text drawn next to the down direction (default blue).
     **/
	inline EdgeSiteImage & textdownColor(RGBc color) {_ctextdown = color; return(*this);}


    /**
     * Set the text drawn next to the left direction (default nothing).
     **/
	inline EdgeSiteImage & textleft(const std::string & txt) {_textleft = txt; return(*this);}


    /**
     * Set the text drawn next to the left direction and its color (default nothing, color blue).
     **/
	inline EdgeSiteImage & textleft(const std::string & txt,RGBc color) {_textleft = txt; _ctextleft = color; return(*this);}


    /**
     * Set the color of the text drawn next to the left direction (default blue).
     **/
	inline EdgeSiteImage & textleftColor(RGBc color) {_ctextleft = color; return(*this);}


    /**
     * Set the text drawn next to the right direction (default nothing).
     **/
	inline EdgeSiteImage & textright(const std::string & txt) {_textright = txt; return(*this);}


    /**
     * Set the text drawn next to the right direction and its color (default nothing, color blue).
     **/
	inline EdgeSiteImage & textright(const std::string & txt,RGBc color) {_textright = txt; _ctextright = color; return(*this);}


    /**
     * Set the color of the text drawn next to the right direction (default blue).
     **/
	inline EdgeSiteImage & textrightColor(RGBc color) {_ctextright = color; return(*this);}


    /**
     * Draw the image into im without changing its size.
     *
     * @param [in,out]  im  The image to draw into.
     *
     * @return  a reference to im.
     **/
    Img<unsigned char> & makeImage(Img<unsigned char> & im) const;


    /**
     * Draw the image into im with a given size. The method is faster if  im is already of size (sx,
     * sy).
     *
     * @param [in,out]  im  The image to draw into.
     * @param   size        The requested size (lx,ly).
     *
     * @return  a reference to im.
     **/
    Img<unsigned char> & makeImage(Img<unsigned char> & im, iVec2 size) const;


	private:

    //
    // PRIVATE PART
    //
    //
    inline void _drawArrow(RGBc coul, TypeEdge type, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_extArrow(RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_intArrowSite(RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_intArrow(RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_extEdge(RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_centerEdge(RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_insideEdge(RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_centerNoSite(RGBc coul, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_centerSite(RGBc coul, Img<unsigned char> & im, double kx, double ky) const;
    inline void _draw_rect(double x0, double y0, double x1, double y1, RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky)  const;
    inline void _draw_triangle(double x0, double y0, double x1, double y1, double x2, double y2, RGBc coul, int direction, Img<unsigned char> & im, double kx, double ky) const;

    TypeEdge    _up, _down, _left, _right;                      // type of edges.
	bool        _site;                                          // is there a central site.
	RGBc        _cbk,_csite,_cup,_cdown,_cleft,_cright,_ctext;  // colors of the background, the site, the 4 edges and the center text.
	RGBc        _ctextup,_ctextdown,_ctextleft,_ctextright;     // color of the text next to the edges.
	std::string _text,_textup,_textdown,_textleft,_textright;   // the center text and the text next to the edges.
	};


}


/* end of file */

