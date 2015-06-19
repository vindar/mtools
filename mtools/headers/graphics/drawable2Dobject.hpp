/** @file drawable2Dobject.hpp */
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


#include "maths/vec.hpp"
#include "maths/rect.hpp"
#include "misc/error.hpp"
#include "customcimg.hpp"

#include <atomic>

namespace mtools
{


namespace internals_graphics
{



    /**
     * Interface describing a drawable 2D object.
     * 
     * Any object implementing this interface can be inserted into an AutoDrawable2DObject nad
     * can therefore drawn using a Plotter2D.
     * 
     * @warning : the public methods of the interface must be thread-safe !
     **/
    class Drawable2DObject
    {

        public:

        /** Default constructor. */
        Drawable2DObject() {}

        /** virtual Destructor. */
        virtual ~Drawable2DObject() {}


        /**
         * Sets the parameters for the drawing.
         *
         * @param   range       The range to draw.
         * @param   imageSize   Size of the desired picture.
         **/
        virtual void setParam(mtools::fRect range, mtools::iVec2 imageSize) = 0;


        /**
         * Force a reset of the drawing. This method is called to indicate that the underlying object
         * drawn may have changed and every previous drawing should be discarded. 
         **/
        virtual void resetDrawing() { if (needWork() == false) { return; } MTOOLS_ERROR("Drawable2DObject::resetDrawing() must be overriden when Drawable2DObject::needWork() return true."); return; }


        /**
         * Draw onto a given image. This method is called when we want the picture. This method should
         * be as fast as possible  (eventually indicating that it is incomplete by returning a number
         * smaller than 100).
         *
         * @param [in,out]  im  The image to draw onto (must accept both 3 or 4 channel images).
         * @param   opacity     The opacity that should be applied to the picture before drawing onto im.
         *                      Hence, if opacity = 1.0, overwrite im and if opacity = 0.0 do nothing.
         *
         * @return  The quality of the drawing made (0 = nothing drawn, 100 = perfect drawing).
         **/
        virtual int drawOnto(cimg_library::CImg<unsigned char> & im, float opacity = 1.0) = 0;


        /**
         * Return an estimation of the quality of the drawing that would be returned by calling now the
         * drawOnto() method. The default implementation return 100 is needWork return false. If
         * possible, an effcient implementation should be able to return the current quality without
         * having to interrupt a working drawing thread.
         *
         * @return  A lower bound on the quality of the current drawing. Should return >0 has soon has
         *          the image is worth drawing and 100 when the drawing is perfect.
         **/
        virtual int quality() const { if (needWork() == false) { return 100; } MTOOLS_ERROR("Drawable2DObject::quality() must be overriden when Drawable2DObject::needWork() return true."); return 0; }


        /**
         * Constant method. Indicate whether drawOnto() will always return a perfect drawing (i.e.
         * return 100) or we need to call the work() method to create the image. The default
         * implementation assume that the object does not need work hence return false.
         *
         * @return  true if work is needed to construct the image. False (default) if work() does nothing
         *          and drawOnto() will always return 100 and create a perfect drawing.
         **/
        virtual bool needWork() const { return false; }


        /**
         * Work on the drawing for a maximum time of time_ms milliseconds. This method does nothing and
         * return immediately (default) if needWork() return false. The method is allowed to return
         * earlier if it is interrupted by another concurrent access to another public method or if work
         * finishes early. The default implementation does nothing and return 100.
         *
         * @param   time_ms The time in milliseconds that is allowed for drawing.
         *
         * @return  The current quality of the drawing : a call to drawOnto() should now return a picture
         *          of quality bounded below by this value.
         **/
        virtual int work(int time_ms) { if (!needWork()) { return 100; } MTOOLS_ERROR("Drawable2DObject::work() must be overriden when Drawable2DObject::needWork() return true."); return 0; }


        /**
         * Stop any work in progress. When this function return, if a work was ongoing when it was
         * called it must have ended. The default implementation does nothing and is suitable for object
         * that do not need to work.
         **/
        virtual void stopWork() { if (!needWork()) { return; } MTOOLS_ERROR("Drawable2DObject::stop() must be overriden when Drawable2DObject::needWork() return true."); return; }

    };


    /**
     * Class which automates the `work()` method of a `Drawable2DObject`. This class creates an
     * independent thread which keep the drawing of the underlying Drawable2DObject updated.
     * 
     * Any Drawable2DObject can be plugged in. If the object does not need work to draw (i.e.
     * `needWork()` return false) then this object does nothing (but it still provide an interface
     * to the underlying object). On the other hand, if `needWork()` return true, the worker thread
     * is created and can be managed via the `workThread()` method.
     **/
    class AutoDrawable2DObject
    {

    public:

         /**
          * Constructor. The object start with the work thread enabled.
          *
          * @param [in,out] obj The drawable 2D object that should be managed. Its lifetime must exceed
          *                     that of this object.
          * @param  startThread true to start the worker thread if it is needed. False to prevent starting
          *                     the worker thread.
          **/
         AutoDrawable2DObject(Drawable2DObject * obj, bool startThread = true);


         /**
         * Destructor. Stop the working thread if active but does not delete the managed object.
         **/
         virtual ~AutoDrawable2DObject();


        /**
         * Sets the parameters of the drawing.
         *
         * @param   range       The range to draw.
         * @param   imageSize   The size of the desired picture.
         **/
        void setParam(mtools::fRect range, mtools::iVec2 imageSize);


        /**
         * Force a reset of the drawing. This method is called to indicate that the underlying object
         * drawn may have changed and every previous drawing should be discarded.
         **/
        void resetDrawing();


        /**
         * Draw onto a given image. This method is called when we want the picture. This method should
         * be as fast as possible  (eventually indicating that it is incomplete by returning a number
         * smaller than 100).
         *
         * @param [in,out]  im  The image to draw onto (must accept both 3 or 4 channel images).
         * @param   opacity     The opacity that should be applied to the picture before drawing onto im.
         *                      Hence, if opacity = 1.0, overwrite im and if opacity = 0.0 do nothing.
         *
         * @return  The quality of the drawing made (0 = nothing drawn, 100 = perfect drawing).
         **/
        int drawOnto(cimg_library::CImg<unsigned char> & im, float opacity = 1.0);


        /**
         * Return the quality of the current drawing : a call to drawOnto should produce a drawing with
         * a quality bounded below by this value.
         *
         * @return  The quality of the drawing between 0 (nothng to show) and 100 (perfect).
         **/
        int quality() const;


        /**
         * Determines if the Object needs the working thread in order to produce a drawing. This method
         * simply transfer the result from the associated needWork() method of the underlying
         * Drawable2DObject.
         *
         * @return  true if work is needed and false otherwise.
         **/
        bool needWork() const;


        /**
         * Start / Stop the working thread. 
         *
         * @param   status  true to enable the working thread and false to disable it. 
         **/
        void workThread(bool status);


        /**
         * Return the status of the working thread. ALways return false if needWork() return false. 
         *
         * @return  true if the working thread is currently enabled.
         **/
        bool workThread() const;


    private:

        AutoDrawable2DObject(const AutoDrawable2DObject &) = delete;                // no copy.
        AutoDrawable2DObject & operator=(const AutoDrawable2DObject &) = delete;    //

        void _workerThread(); 

        void _stopThread();

        void _startThread();

        mutable std::mutex _mut;
        std::atomic<bool> _mustexit;
        std::atomic<bool> _threadon;

        Drawable2DObject * _obj; // the drawable object to manage
    };


}

}

/* end of file */