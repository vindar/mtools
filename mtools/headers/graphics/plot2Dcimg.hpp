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
#include "pixeldrawer.hpp"
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
     * Plot Object which encapsulate a Img<unsigned char> image. The image is either centered at
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
        Plot2DCImg(Img<unsigned char> * im, int nbthreads = 1, std::string name = "CImg image") : internals_graphics::Plotter2DObj(name), _typepos(TYPEBOTTOMLEFT), _im(im), _PD(nullptr), _proImg(nullptr)
			{
			_PD = new PixelDrawer<Plot2DCImg>(this, nbthreads);
			_proImg = new ProgressImg();
			}


        /**
        * Constructor. Reference version.
        *
        * @param [in,out]  im  The CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown)
		* @param   nbthread    The number of threads to use for drawing.
        * @param   name        The name of the plot
        **/
        Plot2DCImg(Img<unsigned char> & im, int nbthreads = 1, std::string name = "CImg image") : internals_graphics::Plotter2DObj(name), _typepos(TYPEBOTTOMLEFT), _im(&im), _PD(nullptr), _proImg(nullptr)
			{
			_PD = new PixelDrawer<Plot2DCImg>(this, nbthreads);
			_proImg = new ProgressImg();
			}


        /**
        * Move constructor.
        **/
        Plot2DCImg(Plot2DCImg && o) : internals_graphics::Plotter2DObj(std::move(o)), _typepos((int)o._typepos), _im(o._im), _PD((PixelDrawer<Plot2DCImg>*)o._PD), _proImg(o._proImg), _checkButtonCenter(nullptr), _checkButtonBottomLeft(nullptr)
			{
			o._PD = nullptr;     // so that the plane drawer is not destroyed when the first object goes out of scope.
			o._proImg = nullptr;
			}


        /**
        * Destructor. Remove the object if it is still inserted.
        **/
        virtual ~Plot2DCImg()
			{
			detach(); // detach from its owner if there is still one.
			delete _PD;     // remove the plane drawer
			delete _proImg; // and the progress image
			}



        /**
         * Change the image (pointer version).
         *
         * @param [in,out]  im  The new image or nullptr if there is none. The previous image (if any) is
         *                      NOT deleted.
         **/
		void image(Img<unsigned char> * im)
			{
			enable(false);
			_im = im;
			enable(true);
			resetDrawing();
			}


        /**
        * Change the image (reference version).
        *
        * @param [in,out]  im  The new image.
        **/
		void image(Img<unsigned char> & im)
			{
			enable(false);
			_im = &im;
			enable(true);
			resetDrawing();
			}


        /**
         * Get the current image. Does not interrupt any work in progress.
         *
         * @return  a pointer to the current image or nullptr it there is none.
         **/
        Img<unsigned char> * image() const { return _im; }


        /**
         * Sets the image position.
         *
         * @param   posType Position of the image : TYPECENTER to center the image around the origin and
         *                  TYPEBOTTOMLEFTto postion it such that the bottom left corner of the image is
         *                  at the origin.
         **/
        void position(int posType)
			{
			if (((posType != TYPECENTER) && (posType != TYPEBOTTOMLEFT)) || (posType == _typepos)) return; // nothing to do
			_typepos = posType;
			if (isInserted())
				{
				IndirectMemberProc<Plot2DCImg> proxy(*this, &Plot2DCImg::_updatePosTypeInFLTK); // update the status of the button in the fltk thread
				runInFltkThread(proxy); // and also refresh the drawing if needed
				resetDrawing();
				}
			}


        /**
         * Query the image position. Does not interrupt any work in progress.
         *
         * @return  TYPECENTER is the image is centered around the origin and TYPEBOTTOMLEFT if its lower
         *          left corner is at the origin.
         **/
        int position() const { return _typepos; }


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



		/* set the domain
		void _setDomain()
		{
		if (_im == nullptr) { _LD->domainEmpty(); return; }
		if (_typepos == TYPECENTER)
		{
		_LD->domain(iBox2(-_im->width() / 2, _im->width() - 1 - _im->width() / 2, _im->height() / 2 - _im->height(), _im->height() / 2 - 1)); return;
		}
		_LD->domain(iBox2(0, _im->width() - 1, 0, _im->height() - 1));
		}
		*/


		/*
		virtual fBox2 favouriteRangeX(fBox2 R) override
			{
			
			if ((_LD->isDomainEmpty() || _LD->isDomainFull())) return fBox2(); // no favourite range
			iBox2 D = _LD->domain();
			return fBox2((double)D.min[0] - 0.5, (double)D.max[0] + 0.5, (double)D.min[1] - 0.5, (double)D.max[1] + 0.5);
			}



		virtual fBox2 favouriteRangeY(fBox2 R) override
			{
			if ((_LD->isDomainEmpty() || _LD->isDomainFull())) return fBox2(); // no favourite range
			iBox2 D = _LD->domain();
			return fBox2((double)D.min[0] - 0.5, (double)D.max[0] + 0.5, (double)D.min[1] - 0.5, (double)D.max[1] + 0.5);
			}


		virtual bool hasFavouriteRangeX() override
			{
			return(!(_LD->isDomainEmpty() || _LD->isDomainFull())); // there is a favourite range if the domain is neither empy nor full
			}


		virtual bool hasFavouriteRangeY() override
			{
			return(!(_LD->isDomainEmpty() || _LD->isDomainFull())); // there is a favourite range if the domain is neither empy nor full
			}
			*/

    protected:

		/***************************************
		* Override from the Drawable2DInterface.
		****************************************/
		virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override
			{
			if ((_proImg->height() != (size_t)imageSize.X()) || (_proImg->width() != (size_t)imageSize.Y()))
				{
				auto npimg = new ProgressImg((size_t)imageSize.X(), (size_t)imageSize.Y());
				_PD->setParameters(range, npimg);
				_PD->sync();
				delete _proImg;
				_proImg = npimg;
				return;
				}
			_PD->setParameters(range, _proImg);
			_PD->sync();
			_PD->enable(_PD->enable());
			}

		virtual void resetDrawing() override
			{
			_PD->redraw(false);
			_PD->sync();
			}

		virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override
			{
			int q = _PD->progress();
			_proImg->blit(im, opacity, true);
			return q;
			}

		virtual int quality() const override { return _PD->progress(); }

		virtual void enableThreads(bool status) override { _PD->enable(status); _PD->sync(); }

		virtual bool enableThreads() const override { return _PD->enable(); }

		virtual int nbThreads() const override { return _PD->nbThreads(); }


		/***************************************
		* Override from the Drawable2DInterface.
		****************************************/

		virtual void removed(Fl_Group * optionWin) override
			{
			Fl::delete_widget(optionWin);
			_checkButtonCenter = nullptr;
			_checkButtonBottomLeft = nullptr;
			_PD->enable(false);
			_PD->sync();
			}


		virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
			{
			/* create the option window */
			optionWin = new Fl_Group(0, 0, reqWidth, 60); // create the option group
			_checkButtonCenter = new Fl_Round_Button(15, 10, reqWidth - 20, 15, "Origin at the center.");
			_checkButtonCenter->labelfont(0);
			_checkButtonCenter->labelsize(11);
			_checkButtonCenter->color2(FL_RED);
			_checkButtonCenter->type(102);
			_checkButtonCenter->callback(_roundButtonCB_static, this);
			_checkButtonCenter->when(FL_WHEN_CHANGED);
			_checkButtonBottomLeft = new Fl_Round_Button(15, 35, reqWidth - 20, 15, "Origin at the bottom left corner.");
			_checkButtonBottomLeft->labelfont(0);
			_checkButtonBottomLeft->labelsize(11);
			_checkButtonBottomLeft->color2(FL_RED);
			_checkButtonBottomLeft->type(102);
			_checkButtonBottomLeft->callback(_roundButtonCB_static, this);
			_checkButtonBottomLeft->when(FL_WHEN_CHANGED);
			if (_typepos == TYPECENTER) { _checkButtonCenter->setonly(); }
			else { _checkButtonBottomLeft->setonly(); }
			optionWin->end();
			return this;
			}


    private:

        /* update the state of the buttons */
        void _updatePosTypeInFLTK()
			{
			if (_typepos == TYPECENTER) { _checkButtonCenter->setonly(); } else { _checkButtonBottomLeft->setonly(); }
			}


        /* callback for the round buttons */
        static void _roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DCImg*)data)->_roundButtonCB(W); }

        void _roundButtonCB(Fl_Widget * W)
			{
			if (W == _checkButtonCenter) { _typepos = TYPECENTER; }
			else { _typepos = TYPEBOTTOMLEFT; }
			resetDrawing();
			}


        std::atomic<int>  _typepos;                      // position of the image wrt the origin
        Img<unsigned char> * _im;                        // pointer to the source image

		PixelDrawer<Plot2DCImg> * _PD;					 // the pixel drawer
		ProgressImg * _proImg;							 // the progress image

        Fl_Round_Button * _checkButtonCenter;			 // option window buttons
        Fl_Round_Button * _checkButtonBottomLeft;		 //

    };


	/**
	* Factory function for a constructing a plot2DCImg image from a CImg object. Reference version
	**/
	inline Plot2DCImg makePlot2DCImg(Img<unsigned char> & im, int nbthreads = 1, std::string name = "CImg image")
		{
		return Plot2DCImg(im, nbthreads, name);
		}


	/**
	* Factory function for a constructing a plot2DCImg image from a CImg object. Pointer version
	**/
	inline Plot2DCImg makePlot2DCImg(Img<unsigned char> * im, int nbthreads = 1, std::string name = "CImg image")
		{
		return Plot2DCImg(im, nbthreads, name);
		}


}


/* end of file */



