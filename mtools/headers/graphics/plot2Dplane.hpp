/** @file plot2Dplane.hpp */
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
#include "planedrawer.hpp"
#include "rangemanager.hpp"
#include "../misc/indirectcall.hpp"

#include <atomic>

namespace mtools
    {

    template< typename T > class  Plot2DPlane;



    /**
    * Factory function for creating a plot2DPlane (reference version).
    *
    * @tparam  T   The class must implement the PlaneDrawer requirements.
    * @param [in,out]  obj The Plane object to plot (reference version)
    * @param   name        The name of the plot.
    *
    * @return  A Plot2DPlane<T>
    **/
    template<typename T> Plot2DPlane<T> makePlot2DPlane(T & obj, std::string name = "Plane")
        {
        return Plot2DPlane<T>(obj, name);
        }



    /**
    * Factory function for creating a plot2DPlane (pointer version).
    *
    * @tparam  T   The class must implement the PlaneDrawer requirements.
    * @param [in,out]  obj The Plane object to plot (reference version)
    * @param   name        The name of the plot.
    *
    * @return  A Plot2DPlane<T>
    **/
    template<typename T> Plot2DPlane<T> makePlot2DPlane(T * obj, std::string name = "Plane")
        {
        return Plot2DPlane<T>(obj, name);
        }




    /**
    * Plot Object which encapsulate a Plane object.
    *
    * @tparam  T   Object which fulfills the same requirements as those needed by the PlaneDrawer
    *              class.
    **/
    template< typename T > class  Plot2DPlane : public internals_graphics::Plotter2DObj
        {

        public:

            /**
             * Constructor. Pointer version : permit to pass nullptr if the methods are static.
             *
             * @param [in,out]  obj The plane object that must fullfill the requirement of PlaneDrawer. The
             *                      object must survive the plot.
             * @param   name        The name of the plot.
             **/
            Plot2DPlane(T * obj, std::string name = "Plane") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _encD(nullptr)
                {
                _LD = new PlaneDrawer<T>(obj);
                }


            /**
            * Constructor. Reference verison
            *
            * @param [in,out]  obj The plane object that must fullfill the requirement of PlaneDrawer. The
            *                      object must survive the plot.
            * @param   name        The name of the plot.
            **/
            Plot2DPlane(T & obj, std::string name = "Plane") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _encD(nullptr)
                {
                _LD = new PlaneDrawer<T>(&obj);
                }


            /**
            * Move constructor.
            **/
            Plot2DPlane(Plot2DPlane && o) : internals_graphics::Plotter2DObj(std::move(o)), _LD((PlaneDrawer<T>*)o._LD), _LD(nullptr), _encD(nullptr)
                {
                o._LD = nullptr; // so that the plane drawer is not destroyed when the first object goes out of scope.
                o._encD = nullptr;
                }


            /**
            * Destructor. Remove the object if it is still inserted.
            **/
            virtual ~Plot2DPlane()
                {
                detach(); // detach from its owner if there is still one.
                delete _LD; // remove the plane drawer
                }


            /**
            * Query the definition domain.
            *
            * @return  The current definition domain.
            **/
            fBox2 domain() const
                {
                return _LD->domain();
                }

            /**
            * Query if the domain is the whole plane.
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
            void domain(mtools::fBox2 R)
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
                return _LD->domain();
                }


            virtual fBox2 favouriteRangeY(fBox2 R) override
                {
                if ((_LD->isDomainEmpty() || _LD->isDomainFull())) return fBox2(); // no favourite range
                return _LD->domain();
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
            * Override of the removed method, nothing to do...
            **/
            virtual void removed(Fl_Group * optionWin) override
                {
                delete _encD;
                _encD = nullptr;
                }


            /**
            * Override of the inserted method. There is no option window for a plane object...
            **/
            virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
                {
                optionWin = nullptr;
                _encD = new internals_graphics::EncapsulateDrawable2DObject(_LD, false);
                return _encD;
                }


        private:

            PlaneDrawer<T> * _LD;                 // the plane drawer object
            internals_graphics::EncapsulateDrawable2DObject * _encD;

        };



    }


/* end of file */



