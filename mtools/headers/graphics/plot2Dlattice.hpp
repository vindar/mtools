/** @file plot2Dlattice.hpp */
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
#include "../misc/indirectcall.hpp"

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Slider.H>



#include <atomic>

namespace mtools
{
    template< typename T > class  Plot2DLattice;

        


    /**
     * Factory function for a plot2Dlattice (reference version).
     *
     * @code{.cpp}
     *    auto L1 = makePlot2DLattice( LatticeObjImage<colorFct, imageFct>::get() ); // create an object from global color and image functions.
     *    auto L2 = makePlot2DLattice( LaticeObj<colorFct>::get() ); // create an object using only a global color functions.
     *    auto L3 = makePlot2DLAticce( myLatticeObj , "my lattice"); //  creates from a class implementing getColor (and maybe getImage) and give the plot a sepcific name.
     * @endcode.
     *
     * @tparam  T   The class must implement the LatticeDrawer requirements. Such an object can also be
     *              created from a global/class static function via the LatticeObj and LatticeObjImage
     *              classes (and a pointer of the right type can be retrieved via 'get()' method).
     *              See the example below.
     * @param [in,out]  obj The lattice object to plot (reference version)
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DLattice<T>
     **/
    template<typename T> Plot2DLattice<T> makePlot2DLattice(T & obj, std::string name = "Lattice")
        {
        return Plot2DLattice<T>(obj,name);
        }


    /**
     * Factory function for a plot2Dlattice (pointer verison).
     *
     * @code{.cpp}
     *    auto L1 = makePlot2DLattice( LatticeObjImage<colorFct, imageFct>::get() ); // create an object from global color and image functions.
     *    auto L2 = makePlot2DLattice( LaticeObj<colorFct>::get() ); // create an object using only a global color functions.
     *    auto L3 = makePlot2DLAticce( myLatticeObj , "my lattice"); //  creates from a class implementing getColor (and maybe getImage) and give the plot a sepcific name.
     * @endcode.
     *
     * @tparam  T   The class must implement the LatticeDrawer requirements. Such an object can also be
     *              created from a global/class static function via the LatticeObj and LatticeObjImage
     *              classes (and a pointer of the right type can be retrieved via 'get()' method).
     *              See the example below.
     * @param [in,out]  obj The lattice object to plot (reference version)
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DLattice<T>
     **/
    template<typename T> Plot2DLattice<T> makePlot2DLattice(T * obj, std::string name = "Lattice")
        {
        return Plot2DLattice<T>(obj,name);
        }



    /**
     * Plot Object which encapsulate a Lattice object.
     *
     * @code{.cpp}
     * mtools::RGBc colorBigCircle(mtools::iVec2 pos)
     *    {
     *    if (pos.norm() < 100) return mtools::RGBc::jetPalette(pos.norm() / 100);
     *    return mtools::RGBc::c_TransparentWhite;
     *    }
     *
     * const Img<unsigned char> * imageBigCircle(mtools::iVec2 pos, mtools::iVec2 size)
     *    {
     *    static Img<unsigned char>  im; mtools::EdgeSiteImage ES;
     *    if (pos.norm() < 100) { mtools::EdgeSiteImage ES; ES.site(true, mtools::RGBc::jetPalette(pos.norm() / 100)).text("A").makeImage(im, size); return &im; }
     *    return nullptr;
     *    }
     *
     * class Strip
     *    {
     *    public:
     *    mtools::RGBc getColor(mtools::iVec2 pos) // could also be static so we would not have to create an instance of the object.
     *        {
     *        if ((pos.X() <= 50) && (pos.X() >= -50)) return mtools::RGBc::c_Gray;
     *        return mtools::RGBc::c_TransparentWhite;
     *        }
     *    };
     *
     * int main()
     *    {
     *    Plotter2D P;  // the plotter
     *    Plot2DLattice< mtools::LatticeObjImage<colorBigCircle,imageBigCircle> > L1(nullptr, "Big Circle"); // encapsulate the method into an object. We do not need to create an instance and can just pass nullptr since the method are static.
     *    Strip strip; // strip object
     *    Plot2DLattice< Strip> L2(&strip, "Strip"); // the second lattice object. This one has getColor() has a non-static member and no getImage method.
     *    P[L1][L2]; // insert both objects in the plotter.
     *    P.range().setRange(mtools::fBox2(-150, 150, -150, 150)); // change the range
     *    P.range().setRatio1(); // we want a 1:1 aspect ratio.
     *    P.fourChannelImage(true); // use four channel image for better handling of transparency (in particular for the transparent white when there is nothing in the lattice).
     *    L2.opacity(0.5); //make the strip half transparent.
     *    P.solidBackGroundColor(mtools::RGBc::c_Black); // use a black background
     *    P.plot(); // display the plot.
     *    return 0;
     *    }
     * @endcode.
     *
     * @tparam  T   Object which fulfills the same requirements as those needed by the LatticeDrawer
     *              class. it must implement the `RGBc getColor(iVec2 pos)` method and possibly the
     *              `const Img<unsigned char> * getImage(iVec2 pos, iVec2 size)` methods (in staitc
     *              or non-static form).
     **/
    template< typename T > class  Plot2DLattice : public internals_graphics::Plotter2DObj
        {

        public:

            /**
             * Constructor. Pointer version : permit to pass nullptr if the methods are static.
             *
             * @param [in,out]  obj The lattice object that must fullfill the requirement of LatticeDrawer :
             *                      it must implement the `RGBc getColor(iVec2 pos)` method and possibly the
             *                      `const Img<unsigned char> * getImage(iVec2 pos, iVec2 size)` method. The
             *                      lattice object must survive the plot.
             * @param   name        The name of the plot.
             **/
            Plot2DLattice(T * obj, std::string name = "Lattice") : internals_graphics::Plotter2DObj(name), _checkButtonImage(nullptr), _checkButtonColor(nullptr), _opacifySlider(nullptr), _checkBlack(nullptr), _checkWhite(nullptr), _LD(nullptr), _encD(nullptr)
                {
                _LD = new LatticeDrawer<T>(obj);
                }


            /**
             * Constructor. Reference verison
             *
             * @param [in,out]  obj The lattice object that must fullfill the requirement of LatticeDrawer :
             *                      it must implement the `RGBc getColor(iVec2 pos)` method and possibly the
             *                      `const Img<unsigned char> * getImage(iVec2 pos, iVec2 size)` method. The
             *                      lattice object must survive the plot.
             * @param   name        The name of the plot.
             **/
            Plot2DLattice(T & obj, std::string name = "Lattice") : internals_graphics::Plotter2DObj(name), _checkButtonImage(nullptr), _checkButtonColor(nullptr), _opacifySlider(nullptr), _checkBlack(nullptr), _checkWhite(nullptr), _LD(nullptr), _encD(nullptr)
                {
                _LD = new LatticeDrawer<T>(&obj);
                }


            /**
             * Move constructor.
             **/
            Plot2DLattice(Plot2DLattice && o) : internals_graphics::Plotter2DObj(std::move(o)), _checkButtonImage(nullptr), _checkButtonColor(nullptr), _opacifySlider(nullptr), _checkBlack(nullptr), _checkWhite(nullptr), _LD((LatticeDrawer<T>*)o._LD), _encD(nullptr)
                {
                o._LD = nullptr; // so that the Latice drawer is not destroyed when the first object goes out of scope.
                o._encD = nullptr;
                }

            /**
             * Destructor. Remove the object if it is still inserted.
             **/
            virtual ~Plot2DLattice()
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
            void setImageType(int imageType = TYPEPIXEL)
                {
                _LD->setImageType(imageType);
                if (isInserted())
                    {
                    IndirectMemberProc<Plot2DLattice> proxy(*this, &Plot2DLattice::_updateImageTypeInFLTK); // update the status of the button in the fltk thread
                    runInFltkThread(proxy); // and also refresh the drawing if needed
                    }
                }


            /**
             * Set the 'opacification factor' used when drawing pixel-type images.
             *
             * @param   o   The new value in [1.0,4.0] (1.0 to disable opacification).
             **/
            void opacify(float o)
                {
                if (o < 1.0f) { o = 1.0f; } else if (o > 4.0f) { o = 4.0f; }
                _LD->opacify(o);
                if (isInserted())
                    {
                    IndirectMemberProc<Plot2DLattice> proxy(*this, &Plot2DLattice::_updateImageTypeInFLTK); // update the status of the button in the fltk thread
                    runInFltkThread(proxy); // and also refresh the drawing if needed
                    }
                }
            

            /**
            * Set how transparent color are handled when drawing pixel-type images.
            *
            * @param   type    The new type: one of REMOVE_NOTHING, REMOVE_WHITE, REMOVE_BLACK.
            **/
            void transparentColor(int type) 
                { 
                _LD->transparentColor(type);
                if (isInserted())
                    {
                    IndirectMemberProc<Plot2DLattice> proxy(*this, &Plot2DLattice::_updateImageTypeInFLTK); // update the status of the button in the fltk thread
                    runInFltkThread(proxy); // and also refresh the drawing if needed
                    }
                }


            static const int TYPEPIXEL = LatticeDrawer<T>::TYPEPIXEL; ///< draw each site with a square of a given color (0)
            static const int TYPEIMAGE = LatticeDrawer<T>::TYPEIMAGE; ///< draw (if possible) each site using an image for the site (1)

            static const int REMOVE_NOTHING = LatticeDrawer<T>::REMOVE_NOTHING; ///< do not remove transparent sites when drawing pixel images
            static const int REMOVE_BLACK = LatticeDrawer<T>::REMOVE_BLACK;     ///< transparent sites (assumed black) when drawing pixel images
            static const int REMOVE_WHITE = LatticeDrawer<T>::REMOVE_WHITE;     ///< transparent sites (assumed white) when drawing pixel images

            /**
             * Query the definition domain.
             *
             * @return  The current definition domain.
             **/
            iBox2 domain() const
                {
                return _LD->domain();
                }


            /**
            * Query if the domain is the whole lattice.
            *
            * @return  true if the domain is full, false if not.
            **/
            bool isDomainFull() const
                {
                return _LD->isDomainFull();
                }


            /**
            * Queries if the domain is empty.
            *
            * @return  true if the domain is empty, false if not.
            **/
            bool isDomainEmpty() const
                {
                return _LD->isDomainEmpty();
                }


            /**
            * Set the definition domain.
            *
            * @param   R   The new definition domain
            **/
            void domain(mtools::iBox2 R)
                {
                if (R == domain()) return;
                _LD->domain(R);
                if (isInserted())
                    {
                    enable(false); enable(true); resetDrawing(); // force update the range button on the object menu and then reset the drawing 
                    }
                }


            /**
            * Set a full definition Domain.
            **/
            void domainFull()
                {
                if (isDomainFull()) return;
                _LD->domainFull();
                if (isInserted())
                    {
                    enable(false); enable(true); resetDrawing(); // force update the range button on the object menu and then reset the drawing 
                    }
                }


            /**
            * Set an empty definition Domain.
            **/
            void domainEmpty()
                {
                if (isDomainEmpty()) return;
                _LD->domainEmpty();
                if (isInserted())
                    {
                    enable(false); enable(true); resetDrawing(); // force update the range button on the object menu and then reset the drawing 
                    }
                }




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


        protected:


            /**
             * Override of the removed method
             **/
            virtual void removed(Fl_Group * optionWin) override
                {
                Fl::delete_widget(optionWin);
                delete _encD;
                _encD = nullptr;
                _checkButtonColor = nullptr;
                _checkButtonImage = nullptr;
                }



            /**
             * Override of the inserted method
             **/
            virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
                {
                /* create the option window */
                optionWin = new Fl_Group(0, 0, reqWidth, 110); // create the option group
                Fl_Group * gr1 = new Fl_Group(0, 0, reqWidth, 110); // create the option group
                _checkButtonColor = new Fl_Round_Button(10, 5, reqWidth - 20, 15, "Use the getColor() method.");
                _checkButtonColor->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                _checkButtonColor->labelfont(0);
                _checkButtonColor->labelsize(11);
                _checkButtonColor->color2(FL_RED);
                _checkButtonColor->type(102);
                _checkButtonColor->callback(_roundButtonCB_static, this);
                _checkButtonColor->when(FL_WHEN_CHANGED);

                _checkButtonImage = new Fl_Round_Button(10, 85, reqWidth - 20, 15, "Use the getImage() method.");
                _checkButtonImage->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                _checkButtonImage->labelfont(0);
                _checkButtonImage->labelsize(11);
                _checkButtonImage->color2(FL_RED);
                _checkButtonImage->type(102);
                _checkButtonImage->callback(_roundButtonCB_static, this);
                _checkButtonImage->when(FL_WHEN_CHANGED);
                if (_LD->imageType() == _LD->TYPEIMAGE) { _checkButtonImage->setonly(); } else { _checkButtonColor->setonly();  }
                if (!_LD->hasImage()) _checkButtonImage->deactivate();
                gr1->end();

                auto label1 = new Fl_Box(30, 25, 50, 15, "Opacify");
                label1->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                label1->labelfont(0);
                label1->labelsize(11);
                _opacifySlider = new Fl_Value_Slider(80, 25, reqWidth - 80 - 30, 15, nullptr);
                _opacifySlider->labelfont(0);
                _opacifySlider->labelsize(11);
                _opacifySlider->align(Fl_Align(FL_ALIGN_RIGHT));
                _opacifySlider->box(FL_FLAT_BOX);
                _opacifySlider->type(FL_HOR_NICE_SLIDER);
                _opacifySlider->range(1.0, 4.0);
                _opacifySlider->step(0.05);
                _opacifySlider->value(_LD->opacify());
                _opacifySlider->color2(FL_RED);
                _opacifySlider->callback(_opacifySliderCB_static, this);
                auto label2 = new Fl_Box(30, 45, 145, 15, "Remove transparent pixels :");
                label2->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                label2->labelfont(0);
                label2->labelsize(11);
                int rem = _LD->transparentColor();
                _checkWhite = new Fl_Check_Button(180, 45, reqWidth - 175 - 30, 15, "white");
                _checkWhite->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                _checkWhite->labelfont(0);
                _checkWhite->labelsize(11);
                _checkWhite->value((rem == REMOVE_WHITE) ? 1 : 0);
                _checkWhite->callback(_checkWhiteCB_static, this);
                _checkBlack = new Fl_Check_Button(180, 65, reqWidth - 175 - 30, 15, "black");
                _checkBlack->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                _checkBlack->labelfont(0);
                _checkBlack->labelsize(11);
                _checkBlack->value((rem == REMOVE_BLACK) ? 1 : 0);
                _checkBlack->callback(_checkBlackCB_static, this);
                optionWin->end();
                _encD = new internals_graphics::EncapsulateDrawable2DObject(_LD, false);
                return _encD;
                }


        private:


            /* update the state of the round button and request a redraw */
            void _updateImageTypeInFLTK()
                {
                if (_LD->imageType() == _LD->TYPEIMAGE) { _checkButtonImage->setonly(); } else { _checkButtonColor->setonly(); }
                _opacifySlider->value(_LD->opacify());
                int rem = _LD->transparentColor();
                switch (rem)
                    {
                    case REMOVE_WHITE:   { _checkWhite->value(1); _checkBlack->value(0); break; }
                    case REMOVE_BLACK:   { _checkWhite->value(0); _checkBlack->value(1); break; }
                    case REMOVE_NOTHING: { _checkWhite->value(0); _checkBlack->value(0); break; }
                    default: MTOOLS_ERROR("wtf...");
                    }
                refresh();
                }


            /* callback for the round buttons */
            static void _roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLattice*)data)->_roundButtonCB(W); }
            void _roundButtonCB(Fl_Widget * W)
                {
                if (W == _checkButtonImage) { _LD->setImageType(_LD->TYPEIMAGE); } else { _LD->setImageType(_LD->TYPEPIXEL); }
                refresh();
                }


            /* callback for the opacify slider */
            static void _opacifySliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLattice*)data)->_opacifySliderCB(W); }
            void _opacifySliderCB(Fl_Widget * W)
                {
                _LD->opacify((float)_opacifySlider->value());
                refresh();
                }


            /* callback for the "black" check button */
            static void _checkBlackCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLattice*)data)->_checkBlackCB(W); }
            void _checkBlackCB(Fl_Widget * W)
                {
                if ((_checkWhite->value() == 0) && (_checkBlack->value() == 0))
                    {
                    _LD->transparentColor(REMOVE_NOTHING);
                    }
                else
                    {
                    _checkWhite->value(0);
                    _checkWhite->redraw();
                    _LD->transparentColor(REMOVE_BLACK);
                    }
                refresh();
                }


            /* callback for the "white" check button */
            static void _checkWhiteCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLattice*)data)->_checkWhiteCB(W); }
            void _checkWhiteCB(Fl_Widget * W)
                {
                if ((_checkWhite->value() == 0) && (_checkBlack->value() == 0))
                    {
                    _LD->transparentColor(REMOVE_NOTHING);
                    }
                else
                    {
                    _checkBlack->value(0);
                    _checkBlack->redraw();
                    _LD->transparentColor(REMOVE_WHITE);
                    }
                refresh();
                }

            Fl_Round_Button * _checkButtonImage;    // the "use getImage" button
            Fl_Round_Button * _checkButtonColor;    // the "use getColor" button
            Fl_Value_Slider * _opacifySlider;       // the opacify slider ctrl
            Fl_Check_Button * _checkBlack;          // the "black" check button 
            Fl_Check_Button * _checkWhite;          // the "white" check button 

            LatticeDrawer<T> * _LD;                 // the lattice drawer
            internals_graphics::EncapsulateDrawable2DObject * _encD;


        };



}


/* end of file */



