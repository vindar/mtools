/** @file sitedrawer.hpp */
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
#include "../misc/internal/threadworker.hpp"
#include "internal/drawable2DInterface.hpp"
#include "image.hpp"
#include "rgbc.hpp"
#include "progressimg.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/misc.hpp"
#include "../misc/metaprog.hpp"
#include "../random/gen_fastRNG.hpp"
#include "internal/getcolorselector.hpp"

#include <algorithm>
#include <ctime>
#include <mutex>
#include <atomic>


namespace mtools
    {




    /**
    * Threaded Site drawer class.
    *
    * Template class that create a unique thread used to draw a site image.
    *
    * @tparam  ObjType Type of object to draw. Must implement a color recognized by the
    *                  GetImageSelector() (cf file getcolorselector.hpp).
    **/
    template<typename ObjType> class SiteDrawer : public ThreadWorker
        {

        public:

            /**
             * Constructor. Associate the object. The thread is initially suspended.
             *
             * @param [in,out]  obj     pointer to the object to be drawn. Must implement a method recognized
             *                          by GetImageSelector().
             * @param [in,out]  opaque  (Optional) The opaque data to passed to getColor(), nullptr if not
             *                          specified.
             **/
            SiteDrawer(ObjType * obj, void * opaque) : _obj(obj), _opaque(opaque), _validParam(false)
                {
                static_assert(mtools::GetImageSelector<ObjType>::has_getImage, "The object must be implement one of the getImage() method recognized by GetImageSelector.");
                }


            /**
            * Destructor. Stop the thread.
            **/
            virtual ~SiteDrawer()
                {
                enable(false);
                sync();
                }


            /**
             * Determines if the drawing parameter are valid. Return false if the range is too far out too
             * draw sites or if the range is incorrect.
             *
             * @return  true if drawing is possible and false otherwise.
             **/
            inline bool validParam()  const { return (bool)_validParam; }


            /**
             * Sets the drawing parameters.
             * 
             * Returns immediately, use sync() to wait for the operation to complete.
             *
             * @param   range       The range to draw.
             * @param   sizeImage   The size of the image to create.
             **/
            void setParameters(const fBox2 & range, const iVec2 & sizeIm)
                {
                sync();
                _temp_range = range;
                _temp_sizeIm = sizeIm;
                signal(SIGNAL_NEWPARAM);
                }


            /**
            * Force a redraw.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            **/
            void redraw()
                {
                sync();
                signal(SIGNAL_REDRAW);
                }


            /**
             * Draw onto a given image
             *
             * @param [in,out]  im  The image to draw onto. must have the same size as that passed by SetParameters()
             * @param   opacity     The opacity to use when drawing.
             *
             * @return  Quality of the image (0 if no image or invalid parameters).
             **/
            int drawOnto(Image & im, float opacity = 1.0)
                {

                return 1;
                }


        private:


            /**
            * Override from the ThreadWorker class.
            * The main 'work' method
            **/
            virtual void work() override
                {
                MTOOLS_INSURE((bool)_validParam);
                }


            /**
            * Override from the ThreadWorker class.
            * Handles the thread messages
            **/
            virtual int message(int64 code) override
                {
                switch (code)
                    {
                    case SIGNAL_NEWPARAM: { return _setNewParam(); }
                    case SIGNAL_REDRAW: { return _setRedraw(); }
                    default: { MTOOLS_ERROR("wtf!"); return 0; }
                    }
                }


            /**
             * Sets new parameters.
             **/
            int _setNewParam()
                {

                return THREAD_RESET;
                }


            /**
             * redraw the whole image
             *
             * @return  An int.
             **/
            int _setRedraw()
                {

                return THREAD_RESET;
                }



            SiteDrawer(const SiteDrawer &) = delete;               // no copy
            SiteDrawer & operator=(const SiteDrawer &) = delete;   //


            static const int SIGNAL_NEWPARAM = 4;
            static const int SIGNAL_REDRAW = 5;

            ObjType * _obj;                         // the object to draw.
            void * _opaque;                         // opaque data passed to _obj;

            std::atomic<bool> _validParam;          // indicate if the parameter are valid.

            fBox2 _range;                           // the range
            std::atomic<fBox2> _temp_range;         // and the temporary used to communicate with the thread.
            iVec2 _sizeIm;                          // size of the image to draw
            std::atomic<iVec2> _temp_sizeIm;        // and the temporary used to communicate with the thread.



            Img<unsigned char>  _exact_qbuf;			// quality buffer of the same size as exact_im: 0 = not drawn. 1 = dirty. 2 = clean 
            Image				_exact_im;	    		// the non-rescaled image of size (_wr.lx()*_exact_sx , _wr.ly()*_exact_sy)
            int					_exact_sx, _exact_sy;	// size of a site image in the exact image
            iBox2				_exact_r;				// the rectangle describing the sites in the exact_image
            int					_exact_qi, _exact_qj;	// position we continue from for improving quality
            int					_exact_phase;			// 0 = remain undrawn sites, 1 = remain dirty site, 2 = finished
            uint32              _exact_Q0;              // number of images not drawn 
            uint32              _exact_Q23;             // number of images of good quality



        };









    }

/* end of file */

