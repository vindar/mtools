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
#include "drawable2Dobject.hpp"
#include "latticedrawer.hpp"
#include "rangemanager.hpp"
#include "misc/indirectcall.hpp"

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
    Plot2DCImg makePlot2DCImg(CImg<unsigned char> & im, std::string name = "CImg image");

    /**
    * Factory function for a constructing a plot2DCImg image from a CImg object. Pointer version
    **/
    Plot2DCImg makePlot2DCImg(CImg<unsigned char> * im, std::string name = "CImg image");



    /**
    * Plot Object which encapsulate a CImg<unsigned char> image.
    *
    **/
    class  Plot2DCImg : public internals_graphics::Plotter2DObj
    {

    public:

        /**
         * Constructor. Pointer version.
         *
         * @param [in,out]  im  Pointer to the CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown). nullptr to draw nothing.
         * @param   name        The name of the plot
         **/
        Plot2DCImg(CImg<unsigned char> * im, std::string name = "CImg image")  : internals_graphics::Plotter2DObj(name)
            {
            _LD = new LatticeDrawer<T>(obj);
            }

        /**
        * Constructor. Reference version.
        *
        * @param [in,out]  im  The CImg image to draw, must be 3 or 4 channels (otherwise nothing is shown)
        * @param   name        The name of the plot
        **/
        Plot2DCImg(CImg<unsigned char> & im, std::string name = "CImg image") : internals_graphics::Plotter2DObj(name)
        {
            _LD = new LatticeDrawer<T>(obj);
        }


        /**
        * Move constructor.
        **/
        Plot2DCImg(Plot2DCImg && o) : internals_graphics::Plotter2DObj(std::move(o)),  _LD((LatticeDrawer<T>*)o._LD)
        {
            o._LD = nullptr; // so that the Latice drawer is not destroyed when the first object goes out of scope.
        }

        /**
        * Destructor. Remove the object if it is still inserted.
        **/
        virtual ~Plot2DCImg()
        {
            detach(); // detach from its owner if there is still one.
            delete _LD; // remove the lattice drawer
        }


        /**
        * Sets image type (pixel or images). The drawer may discard this request and decide to draw in
        * pixel mode anyway if there is no getImage() method or if we are too far away
        *
        * @param   imageType   Type of the image.
        **/
        void setDrawingType(int imageType = TYPEPIXEL)
        {
            /*
            _LD->setImageType(imageType);
            if (isInserted())
            {
                IndirectMemberProc<Plot2DLattice> proxy(*this, &Plot2DLattice::_updateImageTypeInFLTK); // update the status of the button in the fltk thread
                runInFLTKThread(proxy); // and also refresh the drawing if needed
            }
            */
        }


//        static const int TYPEPIXEL = LatticeDrawer<T>::TYPEPIXEL; ///< draw each site with a square of a given color (0)
//        static const int TYPEIMAGE = LatticeDrawer<T>::TYPEIMAGE; ///< draw (if possible) each site using an image for the site (1)


    protected:


        /**
        * Override of the removed method
        **/
        virtual void removed(Fl_Group * optionWin) override
        {
            Fl::delete_widget(optionWin);
            _checkButtonColor = nullptr;
            _checkButtonImage = nullptr;
        }



        /**
        * Override of the inserted method
        **/
        virtual internals_graphics::Drawable2DObject * inserted(Fl_Group * & optionWin, int reqWidth) override
        {
            /* create the option window */
            optionWin = new Fl_Group(0, 0, reqWidth, 60); // create the option group
            _checkButtonColor = new Fl_Round_Button(15, 10, reqWidth - 20, 15, "Use the getColor() method.");
            _checkButtonColor->labelfont(0);
            _checkButtonColor->labelsize(11);
            _checkButtonColor->color2(FL_RED);
            _checkButtonColor->type(102);
            _checkButtonColor->callback(_roundButtonCB_static, this);
            _checkButtonColor->when(FL_WHEN_CHANGED);
            _checkButtonImage = new Fl_Round_Button(15, 35, reqWidth - 20, 15, "Use the getImage() method.");
            _checkButtonImage->labelfont(0);
            _checkButtonImage->labelsize(11);
            _checkButtonImage->color2(FL_RED);
            _checkButtonImage->type(102);
            _checkButtonImage->callback(_roundButtonCB_static, this);
            _checkButtonImage->when(FL_WHEN_CHANGED);
            if (_LD->imageType() == _LD->TYPEIMAGE) { _checkButtonImage->setonly(); }
            else { _checkButtonColor->setonly(); }
            if (!_LD->hasImage()) _checkButtonImage->deactivate();
            optionWin->end();
            return _LD;
        }


    private:


        /* update the state of the round button and request a redraw */
        void _updateImageTypeInFLTK()
        {
            if (_LD->imageType() == _LD->TYPEIMAGE) { _checkButtonImage->setonly(); }
            else { _checkButtonColor->setonly(); }
            refresh();
        }


        /* callback for the round buttons */
        static void _roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLattice*)data)->_roundButtonCB(W); }
        void _roundButtonCB(Fl_Widget * W)
        {
            if (W == _checkButtonImage) { _LD->setImageType(_LD->TYPEIMAGE); }
            else { _LD->setImageType(_LD->TYPEPIXEL); }
            refresh();
        }


        Fl_Round_Button * _checkButtonImage;    // the "use getImage" button
        Fl_Round_Button * _checkButtonColor;    // the "use getColor" button



        LatticeDrawer<T> * _LD;                 // the lattice drawer used for drawing the image


    };



}


/* end of file */



