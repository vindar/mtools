/** @file plot2Dcimg.hpp */
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


#include "plotter2Dobj.hpp"
#include "drawable2DInterface.hpp"
#include "latticedrawer.hpp"
#include "rangemanager.hpp"
#include "../misc/indirectcall.hpp"
#include "../io/fltkSupervisor.hpp"

#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include <atomic>

namespace mtools
{
    /* forward declaration */
    class  Plot2DCImg;


    /**
    * Factory function for a constructing a plot2DCImg image from a CImg object. Reference version
    **/
    Plot2DCImg makePlot2DCImg(Img<unsigned char> & im, std::string name = "CImg image");


    /**
    * Factory function for a constructing a plot2DCImg image from a CImg object. Pointer version
    **/
    Plot2DCImg makePlot2DCImg(Img<unsigned char> * im, std::string name = "CImg image");



    /**
     * Plot Object which encapsulate a Img<unsigned char> image. The image is either centered at
     * the origin or such that its bottom left corner is at the origin.
     * It is possible to change the image even while being displayed or to remove it by passing nullptr.
     **/
    class  Plot2DCImg : public internals_graphics::Plotter2DObj
    {

    public:

        static const int TYPECENTER = 0;        ///< the image is centered around the origin.
        static const int TYPEBOTTOMLEFT = 1;    ///< the image is positioned so that its bottom left corner is a the origin


        /**
         * Constructor. Pointer version.
         *
         * @param [in,out]  im  Pointer to the CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown). nullptr to draw nothing.
         * @param   name        The name of the plot
         **/
        Plot2DCImg(Img<unsigned char> * im, std::string name = "CImg image");


        /**
        * Constructor. Reference version.
        *
        * @param [in,out]  im  The CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown)
        * @param   name        The name of the plot
        **/
        Plot2DCImg(Img<unsigned char> & im, std::string name = "CImg image");


        /**
        * Move constructor.
        **/
        Plot2DCImg(Plot2DCImg && o);


        /**
        * Destructor. Remove the object if it is still inserted.
        **/
        virtual ~Plot2DCImg();


        /**
         * Change the image (pointer version).
         *
         * @param [in,out]  im  The new image or nullptr if there is none. The previous image (if any) is
         *                      NOT deleted.
         **/
        void image(Img<unsigned char> * im);


        /**
        * Change the image (reference version).
        *
        * @param [in,out]  im  The new image.
        **/
        void image(Img<unsigned char> & im);


        /**
         * Get the current image. Does interrupt any work in progress.
         *
         * @return  a pointer to the current image or nullptr it there is none.
         **/
        Img<unsigned char> * image() const;


        /**
         * Sets the image position.
         *
         * @param   posType Position of the image : TYPECENTER to center the image around the origin and
         *                  TYPEBOTTOMLEFTto postion it such that the bottom left corner of the image is
         *                  at the origin.
         **/
        void position(int posType);


        /**
         * Query the image position. Does not interrupt any work in progress.
         *
         * @return  TYPECENTER is the image is centered around the origin and TYPEBOTTOMLEFT if its lower
         *          left corner is at the origin.
         **/
        int position() const;


        /**
         * The getColor function associated with the image.
         *
         * @param   pos The position
         *
         * @return  The color of the pixel at that position.
         **/
        inline RGBc getColor(iVec2 pos)
            {
            if (_im == nullptr) return RGBc::c_TransparentWhite;
            if (_typepos == TYPEBOTTOMLEFT) { pos.Y() = _im->height() - 1 - pos.Y(); }
            else
                {
                pos.X() -= _im->width() /2;
                pos.Y() = _im->height()/2 - 1 - pos.Y();
                }
            const int64 dx = _im->width();
            const int64 dxy = dx*_im->height();
            int64 off = pos.X() + dx*pos.Y();
            if ((off < 0) || (off >= dxy)) return RGBc::c_TransparentWhite;
            const unsigned char * p = _im->data();
            const char r = *(p + off);
            const char g = *(p + off + dxy);
            const char b = *(p + off + 2*dxy);
            const char a = ((_im->spectrum() < 4) ? 255 : (*(p + off + 3*dxy)));
            return RGBc(r, g, b, a);
            }


        virtual fBox2 favouriteRangeX(fBox2 R) override;


        virtual fBox2 favouriteRangeY(fBox2 R) override;


        virtual bool hasFavouriteRangeX() override;


        virtual bool hasFavouriteRangeY() override;


    protected:


        /**
        * Override of the removed method
        **/
        virtual void removed(Fl_Group * optionWin) override;


        /**
        * Override of the inserted method
        **/
        virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override;


    private:

        /* set the domain */
        void _setDomain();


        /* update the state of the buttons */
        void _updatePosTypeInFLTK();


        /* callback for the round buttons */
        static void _roundButtonCB_static(Fl_Widget * W, void * data);
        void _roundButtonCB(Fl_Widget * W);

        std::atomic<int>  _typepos;                      // position of the image wrt the origin
        Img<unsigned char> * _im;                       // the image
        LatticeDrawer<Plot2DCImg> * _LD;                 // the lattice drawer used for drawing the image
        internals_graphics::EncapsulateDrawable2DObject * _encD;
        
        Fl_Round_Button * _checkButtonCenter;
        Fl_Round_Button * _checkButtonBottomLeft;


    };



}


/* end of file */



