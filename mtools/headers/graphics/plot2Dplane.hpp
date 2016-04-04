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
     * @param   nbthreads   The number of thread to use for drawing.
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DPlane&lt;T&gt;
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
     * @param   nbthreads   The number of threads to use for drawing.
     * @param   name        The name of the plot.
     *
     * @return  A Plot2DPlane&lt;T&gt;
     **/
    template<typename T> Plot2DPlane<T> makePlot2DPlane(T * obj, int nbthreads = 1, std::string name = "Plane")
        {
        return Plot2DPlane<T>(obj, nbthreads, name);
        }





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
             * @param   nbthread    The number of threads to use for drawing.
             * @param   name        The name of the plot.
             **/
            Plot2DPlane(T * obj, int nbthread = 1, std::string name = "Plane") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _proImg(nullptr)
                {
                _LD = new PlaneDrawer<T>(obj, nbthread);
                _proImg = new ProgressImg();
                }


            /**
             * Constructor. Reference verison.
             *
             * @param [in,out]  obj The plane object that must fullfill the requirement of PlaneDrawer. The
             *                      object must survive the plot.
             * @param   nbthread    The number of threads to use for drawing.
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

            /**
             * Override from the Drawable2DInterface. 
             **/
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


            /**  
             * Override from the Drawable2DInterface.
             **/
            virtual void resetDrawing() override
                {
                _LD->redraw();
                _LD->sync();
                }


            /**
            * Override from the Drawable2DInterface.
            **/
            virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override
                {
                int q = _LD->progress();
                _proImg->blit(im,opacity,true);
                return q;
                }


            /**
            *Override from the Drawable2DInterface.
            **/
            virtual int quality() const override { return _LD->progress(); }


            /**
            *Override from the Drawable2DInterface.
            **/
            virtual void enableThreads(bool status) override { _LD->enable(status); _LD->sync();  }


            /**
            *Override from the Drawable2DInterface.
            **/
            virtual bool enableThreads() const override { return _LD->enable(); }


            /**
            *Override from the Drawable2DInterface.
            **/
            virtual int nbThreads() const override { return _LD->nbThreads(); }


            /**
            * Override from the Plot2DObj base class method. 
            **/
            virtual void removed(Fl_Group * optionWin) override
                {
                _LD->enable(false); // so that the underlying object will not be ccessed anymore.
                }


            /**
            * Override from the Plot2DObj base class method.
            * There is no option window for a plane object...
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



