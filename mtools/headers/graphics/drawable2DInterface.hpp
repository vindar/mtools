/** @file drawable2DInterface.hpp */
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


#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/error.hpp"
#include "customcimg.hpp"

#include "drawable2Dobject.hpp"

#include <atomic>

namespace mtools
{


namespace internals_graphics
{


    /**
     * Interface describing a drawable 2D object.
     * 
     * This is the type of object that a Plotter2DObj returns when inserted.
     * 
     * @warning : the public methods of the interface must be thread-safe !
     **/
    class Drawable2DInterface
    {

        public:

        /** Default constructor. */
        Drawable2DInterface() {}

        /** virtual Destructor. */
        virtual ~Drawable2DInterface() {}


        /**
         * Sets the parameters for the drawing.
         *
         * @param   range       The range to draw.
         * @param   imageSize   Size of the desired picture.
         **/
        virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) = 0;


        /**
         * Request a reset of the drawing. This method is called to indicate that the underlying object
         * drawn may have changed, previous drawing should be discarded and redrawn.
         **/
        virtual void resetDrawing() { if (nbThreads()>0) { MTOOLS_ERROR("resetDrawing should be overriden."); } return; }


        /**
         * Draw onto a given image. This method is called when we want the picture. This method should
         * be as fast as possible  (eventually indicating that it is incomplete by returning a number
         * smaller than 100).
         *
         * @param [in,out]  im  The image to draw onto.
         * @param   opacity     The opacity that should be applied to the picture before drawing onto im.
         *                      Hence, if opacity = 1.0, overwrite im and if opacity = 0.0 do nothing.
         *
         * @return  The quality of the drawing made (0 = nothing drawn, 100 = perfect drawing).
         **/
        virtual int drawOnto( Img<unsigned char> & im, float opacity = 1.0) = 0;


        /**
         * Return an estimation of the quality of the drawing that would be returned by calling now the
         * drawOnto() method. The default implementation return 100 is needWork return false. 
         *
         * @return  A lower bound on the quality of the current drawing. Should return >0 has soon has
         *          the image is worth drawing and 100 when the drawing is perfect.
         **/
        virtual int quality() const { if (nbThreads()>0) { MTOOLS_ERROR("quality() should be overriden."); } return 100;  }

		
        /**
         * Number of threads used to generate the drawing.
         *
         * @return  Number of threads used or 0 if the object does not use threads.
         **/
        virtual int nbThreads() const { return 0; }

		
        /**
         * Enable/disable the working threads. 
         *
         * @param   status  true to enable the working threads and false to disable them. 
         **/
        virtual void enableThreads(bool status) { return; }


        /**
         * Return the status of the working threads. Return false if useThreads() return false. 
         *
         * @return  true if the working threads are currently enabled.
         **/
        virtual bool enableThreads() const {return false; }

    };

	
	



    /**
    * FOR COMPATIBILITY
    * Convert a old 'Drawable2DObject' into a new 'Drawable2DInterface'
    **/
    class EncapsulateDrawable2DObject : public Drawable2DInterface
        {

        public:

            EncapsulateDrawable2DObject(Drawable2DObject * obj, bool startThread = true) : Drawable2DInterface(), _obj(new AutoDrawable2DObject(obj,startThread)) {}

            virtual ~EncapsulateDrawable2DObject() { delete _obj; }

            virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override { _obj->setParam(range, imageSize); }

            virtual void resetDrawing() override { _obj->resetDrawing(); }

            virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override { return _obj->drawOnto(im, opacity); };

            virtual int quality() const override { return _obj->quality(); }

            virtual int nbThreads() const override { return (_obj->needWork() ? 1 : 0); }

            void enableThreads(bool status) override { _obj->workThread(status); }

            bool enableThreads() const override { return _obj->workThread(); }

        private:

            EncapsulateDrawable2DObject(const EncapsulateDrawable2DObject &) = delete;
            EncapsulateDrawable2DObject& operator=(const EncapsulateDrawable2DObject &) = delete;

            AutoDrawable2DObject * _obj;

        };



    /**
	* FOR COMPATIBILITY
	* Convert a old 'AutoDrawable2DObject' into a new 'Drawable2DInterface'
    **/
	class EncapsulateAutoDrawable2DObject : public Drawable2DInterface
	{		
	public:
		
        EncapsulateAutoDrawable2DObject(AutoDrawable2DObject * obj) : Drawable2DInterface(), _obj(obj)  {}
	
        virtual ~EncapsulateAutoDrawable2DObject() {}

        virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override { _obj->setParam(range, imageSize); }
			
		virtual void resetDrawing() override { _obj->resetDrawing(); }
			
        virtual int drawOnto( Img<unsigned char> & im, float opacity = 1.0) override { return _obj->drawOnto(im, opacity);};

        virtual int quality() const override { return _obj->quality(); }
			
        virtual int nbThreads() const override { return (_obj->needWork() ? 1 : 0); }
		
        void enableThreads(bool status) override { _obj->workThread(status); }

        bool enableThreads() const override { return _obj->workThread(); }
		
	private:

        EncapsulateAutoDrawable2DObject(const EncapsulateAutoDrawable2DObject &) = delete;
        EncapsulateAutoDrawable2DObject& operator=(const EncapsulateAutoDrawable2DObject &) = delete;
			
		AutoDrawable2DObject * _obj;

	};
	


}

}

/* end of file */


