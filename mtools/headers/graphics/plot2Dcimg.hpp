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

#include "image.hpp"
#include "../misc/internal/mtools_export.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2DInterface.hpp"
#include "pixeldrawer.hpp"
#include "internal/rangemanager.hpp"
#include "../misc/indirectcall.hpp"
#include "../io/internal/fltkSupervisor.hpp"
#include "../misc/internal/forward_fltk.hpp"

#include <atomic>



namespace mtools
{


    /**
     * Plot Object which encapsulate a CImg image. The image is either centered at
     * the origin or such that its bottom left corner is at the origin.
     * It is possible to change the image even while being displayed or to remove it by passing nullptr.
     **/
    class  Plot2DCImg : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
    {

    public:

        static const int TYPECENTER = 0;        ///< the image is centered around the origin.
        static const int TYPEBOTTOMLEFT = 1;    ///< the image is positioned so that its bottom left corner is a the origin



        /**
         * Constructor. Pointer version.
         *
         * @param [in,out]  im  Pointer to the CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown). nullptr to draw nothing.
		 * @param   nbthread    The number of threads to use for drawing.
         * @param   name        The name of the plot
         **/
		Plot2DCImg(cimg_library::CImg<unsigned char> * im, int nbthreads = 1, std::string name = "CImg image");


        /**
        * Constructor. Reference version.
        *
        * @param [in,out]  im  The CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown)
		* @param   nbthread    The number of threads to use for drawing.
        * @param   name        The name of the plot
        **/
		Plot2DCImg(cimg_library::CImg<unsigned char> & im, int nbthreads = 1, std::string name = "CImg image");


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
		void image(cimg_library::CImg<unsigned char> * im);

        /**
        * Change the image (reference version).
        *
        * @param [in,out]  im  The new image.
        **/
		void image(cimg_library::CImg<unsigned char> & im);


        /**
         * Get the current image. Does not interrupt any work in progress.
         *
         * @return  a pointer to the current image or nullptr it there is none.
         **/
		cimg_library::CImg<unsigned char> * image() const;


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
			const int64 lx = _im->width();
			const int64 ly = _im->height();
			int64 x = pos.X();
			int64 y = pos.Y();
			if (_typepos == TYPECENTER) { x += lx/2; y += ly/2; }
			if ((x <0)||(y < 0)||(x >= lx)||(y >= ly)) return RGBc::c_TransparentWhite;
			y = ly - 1 - y;
			const int64 lxy = lx*ly;
			int64 off = x + lx*y;
			const unsigned char * p = _im->data();
			const char r = *(p + off);
			const char g = *(p + off + lxy);
			const char b = *(p + off + 2*lxy);
			const char a = ((_im->spectrum() < 4) ? 255 : (*(p + off + 3*lxy)));
			return RGBc(r, g, b, a);
            }


		
		virtual fBox2 favouriteRangeX(fBox2 R) override;
		
		virtual fBox2 favouriteRangeY(fBox2 R) override;
			
		virtual bool hasFavouriteRangeX() override;

		virtual bool hasFavouriteRangeY() override;
			

    protected:

		/***************************************
		* Override from the Drawable2DInterface.
		****************************************/
		virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override;

		virtual void resetDrawing() override;

		virtual int drawOnto(Image & im, float opacity = 1.0) override;

		virtual int quality() const override;

		virtual void enableThreads(bool status) override; 

		virtual bool enableThreads() const override;

		virtual int nbThreads() const override;


		/***************************************
		* Override from the Drawable2DInterface.
		****************************************/

		virtual void removed(Fl_Group * optionWin) override;

		virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override;


    private:


		/* compute the range of the image */
		fBox2 computeRange();

        /* update the state of the buttons */
		void _updatePosTypeInFLTK();


        /* callback for the round buttons */
		static void _roundButtonCB_static(Fl_Widget * W, void * data);

		void _roundButtonCB(Fl_Widget * W);



        std::atomic<int>  _typepos;                      // position of the image wrt the origin
		cimg_library::CImg<unsigned char> * _im;         // pointer to the source image

		PixelDrawer<Plot2DCImg> * _PD;					 // the pixel drawer
		ProgressImg * _proImg;							 // the progress image

        Fl_Round_Button * _checkButtonCenter;			 // option window buttons
        Fl_Round_Button * _checkButtonBottomLeft;		 //

    };




	/**
	* Factory function for a constructing a plot2DCImg image from a CImg object. Reference version
	**/
	inline Plot2DCImg makePlot2DCImg(cimg_library::CImg<unsigned char> & im, int nbthreads = 1, std::string name = "CImg image")
		{
		return Plot2DCImg(im, nbthreads, name);
		}


	/**
	* Factory function for a constructing a plot2DCImg image from a CImg object. Pointer version
	**/
	inline Plot2DCImg makePlot2DCImg(cimg_library::CImg<unsigned char> * im, int nbthreads = 1, std::string name = "CImg image")
		{
		return Plot2DCImg(im, nbthreads, name);
		}





}


/* end of file */



