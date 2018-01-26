/** @file plot2Dimage.hpp */
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
	 * Plot Object which encapsulate a Image object. The image is either centered at the origin or
	 * such that its bottom left corner is at the origin. It is possible to change the image while
	 * being displayed or to remove it by passing nullptr.
	 **/
	class  Plot2DImage : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
		{

		public:

			static const int TYPECENTER = 0;        ///< the image is centered around the origin.
			static const int TYPEBOTTOMLEFT = 1;    ///< the image is positioned so that its bottom left corner is a the origin



			/**
			* Constructor. Pointer version.
			*
			* @param [in,out]  im  Pointer to the Image object to draw, nullptr to draw nothing.
			* @param   nbthread    The number of threads to use for drawing.
			* @param   name        The name of the plot
													**/
			Plot2DImage(Image * im, int nbthreads = 1, std::string name = "Image");


			/**
			* Constructor. Reference version.
			*
			* @param [in,out]  im  The Image object to draw.
			* @param   nbthread    The number of threads to use for drawing.
			* @param   name        The name of the plot
			**/
			Plot2DImage(Image & im, int nbthreads = 1, std::string name = "Image");


			/**
			* Move constructor.
			**/
			Plot2DImage(Plot2DImage && o);

			/**
			* Destructor. Remove the object if it is still inserted.
			**/
			virtual ~Plot2DImage();


			/**
			 * Change the image (pointer version).
			 *
			 * @param [in,out]	im	The new image or nullptr if there is none. The previous image (if any) is
			 * 						removed but the pointer is not deleted.
			 **/
			void image(Image * im);


			/**
			* Change the image (reference version).
			*
			* @param [in,out]  im  The new image.
			**/
			void image(Image & im);


			/**
			* Get the current image. Does not interrupt any work in progress.
			*
			* @return  a pointer to the current image or nullptr it there is none.
			**/
			Image * image() const;


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
				if (_im == nullptr) return RGBc::c_Transparent;
				const int64 lx = _im->lx();
				const int64 ly = _im->ly();
				int64 x = pos.X();
				int64 y = pos.Y();
				if (_typepos == TYPECENTER) { x += lx/2; y += ly/2; }
				return _im->getPixel(x, ly - 1 - y);
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
			Image * _im;									 // pointer to the source image

			PixelDrawer<Plot2DImage> * _PD;					 // the pixel drawer
			ProgressImg * _proImg;							 // the progress image

			Fl_Round_Button * _checkButtonCenter;			 // option window buttons
			Fl_Round_Button * _checkButtonBottomLeft;		 //

		};




	/**
	* Factory function for constructing a plot2DCImage object from a Image object. Reference version
	**/
	inline Plot2DImage makePlot2DImage(Image & im, int nbthreads = 1, std::string name = "Image")
		{
		return Plot2DImage(im, nbthreads, name);
		}


	/**
	* Factory function for constructing a plot2DCImage object from a Image object. Pointer version
	**/
	inline Plot2DImage makePlot2DImage(Image * im, int nbthreads = 1, std::string name = "Image")
		{
		return Plot2DImage(im, nbthreads, name);
		}





	}


/* end of file */



