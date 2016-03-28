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

#include <iostream>

#include "plotter2Dobj.hpp"
#include "drawable2DInterface.hpp"
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
    template<typename T> Plot2DPlane<T> makePlot2DPlane(T & obj, int nbthreads = 1, std::string name = "Plane")
        {
        return Plot2DPlane<T>(obj, nbthreads, name);
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
    template<typename T> Plot2DPlane<T> makePlot2DPlane(T * obj, int nbthreads = 1, std::string name = "Plane")
        {
        return Plot2DPlane<T>(obj, nbthreads, name);
        }







    /*

    Drawable2DInterface() {}
    virtual ~Drawable2DInterface() {}
    virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) = 0;
    virtual void resetDrawing() { if (nbThreads()>0) { MTOOLS_ERROR("resetDrawing should be overriden."); } return; }
    virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) = 0;
    virtual int quality() const { if (nbThreads()>0) { MTOOLS_ERROR("quality() should be overriden."); } return 100; }
    virtual void enableThreads(bool status) { return; }
    virtual bool enableThreads() const { return false; }




    virtual Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth);


    /**
    * This method is called when the owner of the widget removes it. This is the place where the
    * option window should be deleted. The default implementation of this method simply calls
    * "delete_widget(optionWin)" or does nothing if no option window was created and should suffice
    * when using only simple fltk widgets (i.e. no timer or other thing that must be removed before
    * the object goes out of scope).
    * This method is called by the owner from the fltk thread.
    *
    * CAN BE OVERRIDEN IN DERIVED CLASS.
    *
    * @param [in,out]  optionWin   A pointer to the same option window that was given when inserted
    *                              was called.
    **/
    //virtual void removed(Fl_Group * optionWin);

    














    /**
    * Plot Object which encapsulate a Plane object.
    *
    * @tparam  T   Object which fulfills the same requirements as those needed by the PlaneDrawer
    *              class.
    **/
    template< typename T > class Plot2DPlane : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
        {

        public:

            /**
             * Constructor. Pointer version : permit to pass nullptr if the methods are static.
             *
             * @param [in,out]  obj The plane object that must fullfill the requirement of PlaneDrawer. The
             *                      object must survive the plot.
             * @param   name        The name of the plot.
             **/
            Plot2DPlane(T * obj, int nbthread = 1, std::string name = "Plane") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _proImg(nullptr)
                {
                _LD = new PlaneDrawer<T>(obj, nbthread);
                _proImg = new ProgressImg();
                }


            /**
            * Constructor. Reference verison
            *
            * @param [in,out]  obj The plane object that must fullfill the requirement of PlaneDrawer. The
            *                      object must survive the plot.
            * @param   name        The name of the plot.
            **/
            Plot2DPlane(T & obj, int nbthread = 1, std::string name = "Plane") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _proImg(nullptr)
                {
                _LD = new PlaneDrawer<T>(&obj, nbthread);
                _proImg = new ProgressImg();
                }


            /**
            * Move constructor.
            **/
            Plot2DPlane(Plot2DPlane && o) : internals_graphics::Plotter2DObj(std::move(o)), _LD((PlaneDrawer<T>*)o._LD), _proImg(o._proImg)
                {
                o._LD = nullptr;     // so that the plane drawer is not destroyed when the first object goes out of scope.
                o._proImg = nullptr;
                }


            /**
            * Destructor. Remove the object if it is still inserted.
            **/
            virtual ~Plot2DPlane()
                {
                detach();   // detach from its owner if there is still one.
                delete _LD;     // remove the plane drawer
                delete _proImg; // and the progress image
                }


  


        protected:

            virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override
                {
                if ((_proImg->height() != imageSize.X()) || (_proImg->width() != imageSize.Y()))
                    {
                    auto npimg  = new ProgressImg(imageSize.X(), imageSize.Y());
                    _LD->setParameters(range, npimg);
                    _LD->sync();
                    delete _proImg;
                    _proImg = npimg;
                    return;
                    }
                _LD->setParameters(range, _proImg);
                _LD->sync();
                _LD->enable(_LD->enable());
                }


            virtual void resetDrawing() override
                {
                _LD->redraw();
                _LD->sync();
                }


            virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override
                {
                int q = _LD->progress();
                _proImg->blit(im,opacity,true);
                return q;
                }

            virtual int quality() const override { return _LD->progress(); }


            virtual void enableThreads(bool status) override { _LD->enable(status); _LD->sync();  }

            virtual bool enableThreads() const override { return _LD->enable(); }

            virtual int nbThreads() const { return _LD->nbThreads(); }



            /**
            * Override of the removed method, nothing to do...
            **/
            virtual void removed(Fl_Group * optionWin) override
                {
                _LD->enable(false);
                }


            /**
            * Override of the inserted method. There is no option window for a plane object...
            **/
            virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
                {
                optionWin = nullptr;
                return this;
                }


        private:

            PlaneDrawer<T> * _LD;
            ProgressImg * _proImg;

        };



    }


/* end of file */



