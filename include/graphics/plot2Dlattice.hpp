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

#include "../misc/internal/mtools_export.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2Dobject.hpp"
#include "latticedrawer.hpp"
#include "internal/rangemanager.hpp"
#include "../misc/internal/forward_fltk.hpp"

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





	namespace internals_graphics
		{


		class  Plot2DLatticeBase : public internals_graphics::Plotter2DObj
			{

			public:

				Plot2DLatticeBase(std::string name);

				Plot2DLatticeBase(Plot2DLatticeBase && o);

				virtual ~Plot2DLatticeBase();

				static const int TYPEPIXEL = LatticeDrawer<int>::TYPEPIXEL;
				static const int TYPEIMAGE = LatticeDrawer<int>::TYPEIMAGE;
				static const int REMOVE_NOTHING = LatticeDrawer<int>::REMOVE_NOTHING;
				static const int REMOVE_BLACK = LatticeDrawer<int>::REMOVE_BLACK;
				static const int REMOVE_WHITE = LatticeDrawer<int>::REMOVE_WHITE;

			protected:

				virtual void removed(Fl_Group * optionWin) override;

				/* dummy */
				virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override;

				void insertUI(Fl_Group * & optionWin, int reqWidth, int imageType, bool hasImage, float op, int transcolor);

				void updateUI(int imageType, bool hasImage, float op, int transcolor);

				virtual void _setImageType(int imageType) = 0;

				virtual void _setOpacity(float op) = 0;

				virtual void _setTransColor(int transcol) = 0;

			private:

				void _updateUIinFLTK(int imageType, bool hasImage, float op, int transcolor);

				static void _roundButtonCB_static(Fl_Widget * W, void * data);
				void _roundButtonCB(Fl_Widget * W);

				static void _opacifySliderCB_static(Fl_Widget * W, void * data);
				void _opacifySliderCB(Fl_Widget * W);

				static void _checkBlackCB_static(Fl_Widget * W, void * data);
				void _checkBlackCB(Fl_Widget * W);

				static void _checkWhiteCB_static(Fl_Widget * W, void * data);
				void _checkWhiteCB(Fl_Widget * W);

				Fl_Round_Button * _checkButtonImage;    // the "use getImage" button
				Fl_Round_Button * _checkButtonColor;    // the "use getColor" button
				Fl_Value_Slider * _opacifySlider;       // the opacify slider ctrl
				Fl_Check_Button * _checkBlack;          // the "black" check button 
				Fl_Check_Button * _checkWhite;          // the "white" check button 

			};


		}



    /**
     * Plot Object which encapsulate a Lattice object.
     *
     * @tparam  T   Object which fulfills the same requirements as those needed by the LatticeDrawer
     *              class. 
     **/
    template< typename T > class  Plot2DLattice : public internals_graphics::Plot2DLatticeBase
        {

        public:

            /**
             * Constructor. Pointer version : permit to pass nullptr if the methods are static.
             *
             * @param [in,out]  obj The lattice object that must fullfill the requirement of LatticeDrawer 
             * @param   name        The name of the plot.
             **/
            Plot2DLattice(T * obj, std::string name = "Lattice") : Plot2DLatticeBase(name),  _LD(nullptr), _encD(nullptr)
                {
                _LD = new LatticeDrawer<T>(obj);
                }


            /**
             * Constructor. Reference verison
             *
             * @param [in,out]  obj The lattice object that must fullfill the requirement of LatticeDrawer
             * @param   name        The name of the plot.
             **/
            Plot2DLattice(T & obj, std::string name = "Lattice") : Plot2DLatticeBase(name), _LD(nullptr), _encD(nullptr)
                {
                _LD = new LatticeDrawer<T>(&obj);
                }


            /**
             * Move constructor.
             **/
            Plot2DLattice(Plot2DLattice && o) : Plot2DLatticeBase(std::move(o)), _LD((LatticeDrawer<T>*)o._LD), _encD(nullptr)
                {
                o._LD = nullptr; // so that the Latice drawer is not destroyed when the first object goes out of scope.
                o._encD = nullptr;
                }

            /**
             * Destructor. Remove the object if it is still inserted.
             **/
            virtual ~Plot2DLattice()
                {
                detach(); 
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
                if (isInserted()) { this->updateUI(_LD->imageType(), _LD->hasImage(), _LD->opacify(), _LD->transparentColor()); }
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
				if (isInserted()) { this->updateUI(_LD->imageType(), _LD->hasImage(), _LD->opacify(), _LD->transparentColor()); }
                }
            

            /**
            * Set how transparent color are handled when drawing pixel-type images.
            *
            * @param   type    The new type: one of REMOVE_NOTHING, REMOVE_WHITE, REMOVE_BLACK.
            **/
            void transparentColor(int type) 
                { 
                _LD->transparentColor(type);
				if (isInserted()) { this->updateUI(_LD->imageType(), _LD->hasImage(), _LD->opacify(), _LD->transparentColor()); }
                }


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


            virtual void removed(Fl_Group * optionWin) override
                {
				Plot2DLatticeBase::removed(optionWin);
                delete _encD;
                _encD = nullptr;
                }


            virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
                {
				insertUI(optionWin, reqWidth, _LD->imageType(), _LD->hasImage(), _LD->opacify(), _LD->transparentColor());
				_encD = new internals_graphics::EncapsulateDrawable2DObject(_LD, false);
				return _encD;
                }


			virtual void _setImageType(int imageType) override { _LD->setImageType(imageType); }

			virtual void _setOpacity(float op) override { _LD->opacify(op); }

			virtual void _setTransColor(int transcol) override { _LD->transparentColor(transcol); }


            LatticeDrawer<T> * _LD;										// the lattice drawer
            internals_graphics::EncapsulateDrawable2DObject * _encD;	// encapsulation


        };







}


/* end of file */



