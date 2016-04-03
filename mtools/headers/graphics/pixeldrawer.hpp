/** @file pixeldrawer.hpp */
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

#include "../misc/threadworker.hpp"
#include "drawable2DInterface.hpp"
#include "customcimg.hpp"
#include "rgbc.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/misc.hpp"
#include "../misc/metaprog.hpp"
#include "../randomgen/fastRNG.hpp"


#include <algorithm>
#include <ctime>
#include <mutex>
#include <atomic>




namespace mtools
    {






    template<typename ObjType> class ThreadPixelDrawer : public ThreadWorker
        {

        public:

            /**
            * Constructor. Associate the object. The thread is initially suspended.
            *
            * @param [in,out]  obj     pointer to the object to be drawn. Must implement a method recognized
            *                          by GetColorSelector().
            * @param [in,out]  opaque  (Optional) The opaque data to passed to getColor(), nullptr if not
            *                          specified.
            **/
            ThreadPixelDrawer(ObjType * obj, void * opaque = nullptr) : ThreadWorker(),
                _obj(obj),
                _opaque(opaque),
                _keepPrevious(false),
                _validParam(false),
                _range(fBox2()),
                _temp_range(fBox2()),
                _im(nullptr),
                _temp_im(nullptr),
                _subBox(iBox2()),
                _temp_subBox(iBox2()),
                _dens(0.0),
                _dlx(0.0), _dly(0.0),
                _is1to1(false),
                _range1to1(iBox2())
                {
                static_assert(mtools::GetColorSelector<ObjType>::has_getColor, "The object must be implement one of the getColor() method recognized by GetColorSelector.");
                }


            /**
            * Destructor. Stop the thread.
            **/
            virtual ~ThreadPixelDrawer()
                {
                }


            /**
            * Determines if the drawing parameter are valid. Return false if for example, the image is set
            * to nullptr, or the subbox is incorrect, or if the range is too small, or too large, or empty.
            *
            * If it return false, nothing will be drawn and the quality will stay 0.
            *
            * @return  true if the paramter are valid and flase otherwise.
            **/
            inline bool validParam()  const { return (bool)_validParam; }


            /**
            * Sets the drawing parameters.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            *
            * @param   range       The range to draw.
            * @param [in,out]  im  The image to draw into.
            * @param   subBox      The part of the image to draw (border inclusive). If empty, use the whole
            *                      image.
            **/
            void setParameters(const fBox2 & range, ProgressImg * im, const iBox2 & subBox = iBox2())
                {
                sync();
                _temp_range = range;
                _temp_im = im;
                _temp_subBox = subBox;
                signal(SIGNAL_NEWPARAM);
                }


            /**
            * Force a redraw.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            *
            * @param   keepPrevious    If true, keep the previous drawing so that quality starts from 1 and
            *                          not 0 if possible.
            **/
            void redraw(bool keepPrevious = true)
                {
                sync();
                _keepPrevious = keepPrevious;
                signal(SIGNAL_REDRAW);
                }



        private:


            /**
            * The main 'work' method
            **/
            virtual void work() override
                {
                MTOOLS_INSURE((bool)_validParam);
                if (!((bool)_keepPrevious)) _draw_veryfast();
                if ((bool)_is1to1) { _draw_1to1(); return; }
                if (!((bool)_keepPrevious)) _draw_fast();
                _draw_stochastic();
                _draw_perfect();
                }


            /**
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


            /* set the new parameter */
            int _setNewParam()
                {
                const int MIN_IMAGE_SIZE = 2;
                const double RANGE_MIN_VALUE = 1.0e-17;
                const double RANGE_MAX_VALUE = 1.0e17;
                _range = _temp_range;
                _im = _temp_im;
                _subBox = _temp_subBox;
                _keepPrevious = false;
                setProgress(0);
                if ((_im == nullptr) || (_im->width() < MIN_IMAGE_SIZE) || (_im->height() < MIN_IMAGE_SIZE)) { _validParam = false; return THREAD_RESET_AND_WAIT; }   // make sure im is not nullptr and is big enough.
                if (_subBox.isEmpty()) { _subBox = iBox2(0, _im->width() - 1, 0, _im->height() - 1); } // subbox = whole image if empty. 
                if ((_subBox.min[0] < 0) || (_subBox.max[0] >= (int64)_im->width()) || (_subBox.min[1] < 0) || (_subBox.max[1] >= (int64)_im->height())) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // make sure _subBox is a proper subbox of im
                if ((_subBox.lx() < MIN_IMAGE_SIZE) || (_subBox.ly() < MIN_IMAGE_SIZE)) { _validParam = false; return THREAD_RESET_AND_WAIT; } // make sure _subBox is a proper subbox of im
                const double rlx = _range.lx();
                const double rly = _range.ly();
                if ((rlx < RANGE_MIN_VALUE) || (rly < RANGE_MIN_VALUE)) { _validParam = false; return THREAD_RESET_AND_WAIT; } // prevent zooming in too far
                if ((std::abs(_range.min[0]) > RANGE_MAX_VALUE) || (std::abs(_range.max[0]) > RANGE_MAX_VALUE) || (std::abs(_range.min[1]) > RANGE_MAX_VALUE) || (std::abs(_range.max[1]) > RANGE_MAX_VALUE)) { _validParam = false; return THREAD_RESET_AND_WAIT; } // prevent zooming out too far
                _validParam = true;
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                _dlx = rlx / ilx;
                _dly = rly / ily;
                _dens = _dlx*_dly;
                const double epsx = rlx - ilx;
                const double epsy = rly - ily;
                if ((std::abs(epsx) < 1.0) && (std::abs(epsy) < 1.0))
                    {// do 1 to 1 drawing;
                    _is1to1 = true;
                    _range.min[0] += epsx / 2.0; _range.max[0] -= epsx / 2.0;
                    _range.min[1] += epsy / 2.0; _range.max[1] -= epsy / 2.0;
                    _range1to1.min[0] = (int64)std::ceil(_range.min[0]); _range1to1.max[0] = (int64)_range1to1.min[0] + ilx - 1;
                    _range1to1.min[1] = (int64)std::ceil(_range.min[1]); _range1to1.max[1] = (int64)_range1to1.min[1] + ily - 1;
                    }
                else
                    {
                    _is1to1 = false;
                    }
                return THREAD_RESET;
                }


            /* trigger a redraw */
            int _setRedraw()
                {
                if (!((bool)_validParam)) return THREAD_RESET_AND_WAIT;
                if ((progress() >= 5) && ((bool)_keepPrevious))
                    {
                    _im->normalize(_subBox);
                    setProgress(5);
                    return THREAD_RESET;
                    }
                setProgress(0);
                return THREAD_RESET;
                }


            /* very fast drawing */
            void _draw_veryfast()
                {
                // TODO
                //setProgress(4);
                return;
                }


            /* number of pixels in the box to draw */
            int64 _nbPixels() const
                {
                return (_subBox.lx() + 1)*(_subBox.yx() + 1);
                }




            /* fast drawing */
            void _draw_fast()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const double px = _dlx;
                const double py = _dly;
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                const fBox2 r = _range;
                const int64 width = _im->width();
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                size_t pa = (size_t)(width - ilx);
                if (_dens < 0.5)
                    { // low density, try to reduce color queries
                    int64  prevsy = (int64)r.min[1] - 3; // cannot match anything
                    for (int64 j = 0; j < ily; j++)
                        {
                        check();
                        const double y = r.min[1] + (j + 0.5)*py;
                        const int64 sy = (int64)floor(y + 0.5);
                        if (sy == prevsy)
                            {
                            std::memcpy((imData + off), (imData + off) - width, (size_t)ilx*sizeof(RGBc64));
                            std::memset((normData + off), 0, (size_t)ilx);
                            off += (size_t)width;
                            }
                        else
                            {
                            prevsy = sy;
                            RGBc64 coul;
                            int64 prevsx = (int64)r.min[0] - 3; // cannot match anything
                            for (int64 i = 0; i < ilx; i++)
                                {
                                const double x = r.min[0] + (i + 0.5)*px;
                                const int64 sx = (int64)floor(x + 0.5);
                                if (prevsx != sx) // use for _dlx < 0.5  
                                    {
                                    coul = mtools::GetColorSelector<ObjType>::call(*_obj, { sx , sy }, _opaque);
                                    prevsx = sx;
                                    }
                                imData[off] = coul;
                                normData[off] = 0;
                                off++;
                                }
                            off += pa;
                            }
                        }
                    }
                else
                    { // high density, do not try to re-use color
                    for (int64 j = 0; j < ily; j++)
                        {
                        check();
                        const double y = r.min[1] + (j + 0.5)*py;
                        const int64 sy = (int64)floor(y + 0.5);
                        for (int64 i = 0; i < ilx; i++)
                            {
                            const double x = r.min[0] + (i + 0.5)*px;
                            const int64 sx = (int64)floor(x + 0.5);
                            imData[off] = mtools::GetColorSelector<ObjType>::call(*_obj, { sx , sy }, _opaque);
                            normData[off] = 0;
                            off++;
                            }
                        off += pa;
                        }
                    }
                setProgress(5);
                }



            /* perfect 1 to 1 drawing */
            void _draw_1to1()
                {
                const int64 xmin = _range1to1.min[0];
                const int64 xmax = _range1to1.max[0];
                const int64 ymin = _range1to1.min[1];
                const int64 ymax = _range1to1.max[1];
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1])); // offset of the first point in the image
                size_t pa = (size_t)(_im->width() - (_subBox.lx() + 1)); // padding needed to get to the next line
                if (pa != 0)
                    {
                    for (int64 j = ymin; j <= ymax; j++)
                        {
                        check();
                        for (int64 i = xmin; i <= xmax; i++)
                            {
                            imData[off] = mtools::GetColorSelector<ObjType>::call(*_obj, { i , j }, _opaque);
                            normData[off] = 0;
                            off++;
                            }
                        off += pa;
                        }
                    }
                else
                    {
                    for (int64 j = ymin; j <= ymax; j++)
                        {
                        check();
                        for (int64 i = xmin; i <= xmax; i++)
                            {
                            imData[off] = mtools::GetColorSelector<ObjType>::call(*_obj, { i , j }, _opaque);
                            normData[off] = 0;
                            off++;
                            }
                        }
                    }
                setProgress(100);
                }


            /* stochastic drawing */
            void _draw_stochastic()
                {
                const double DENSITY_SKIP_STOCHASTIC = 5.0;
                if (_dens < DENSITY_SKIP_STOCHASTIC) return;
                // compute the number of sample required            
                int sampleToDo;
                if (_dens < 10.0) { sampleToDo = (int)_dens / 2; }
                else if (_dens < 20000) { sampleToDo = 5 + ((int)_dens) / 20; }
                else sampleToDo = 1000;
                int sampleDone = 1; // 1 sample currently done (the fast one)            
                if ((sampleToDo - sampleDone) < 199)
                    { // ok, we do everything on a single pass. 
                    _draw_stochastic_batch(1, sampleToDo - sampleDone, sampleDone, sampleToDo);
                    }
                else
                    { // need multiple passes
                    _draw_stochastic_batch(1, 199, sampleDone, sampleToDo);  // go to 200
                    _progimage_div2(); // go back to 100
                    int batchSize = 2;
                    while (batchSize * 100 < sampleToDo)
                        {
                        _draw_stochastic_batch(batchSize, 100, sampleDone, sampleToDo); // go from 100 to 200
                        _progimage_div2(); // back to 100
                        batchSize *= 2;
                        }
                    _draw_stochastic_batch(batchSize, sampleToDo / batchSize, sampleDone, sampleToDo); // do the remaining passes
                    }
                setProgress(50);
                return;
                }


            /* divide by two the color and number of query in every pixel
            of the subbox of the progressimage. */
            void _progimage_div2()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                const int64 width = _im->width();
                const size_t pa = (size_t)(width - ilx);
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                check();
                for (int64 jj = 0; jj < ily; jj++)
                    {
                    for (int64 ii = 0; ii < ilx; ii++)
                        {
                        imData[off].div2();
                        normData[off] >>= 1;
                        off++;
                        }
                    off += pa;
                    }
                check();
                return;
                }



            void _draw_stochastic_batch(const int batchsize, const int nb, int & sampleDone, const int sampleToDo)
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const double px = _dlx; // lenght of a screen pixel
                const double py = _dly; // height od a screen pixel
                const int64 ilx = _subBox.lx() + 1; // number of horiz pixel in dest image
                const int64 ily = _subBox.ly() + 1; // number of vert pixel in dest image
                const fBox2 r = _range; // corresponding range
                const int64 width = _im->width();
                const size_t pa = (size_t)(width - ilx);
                const int sd = sampleDone;
                for (int nbb = 0; nbb < nb; nbb++)
                    {
                    size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                    fBox2 pixBox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);
                    for (int64 jj = 0; jj < ily; jj++)
                        {
                        check();
                        for (int64 ii = 0; ii < ilx; ii++)
                            {
                            iBox2 siteBox((int64)std::floor(pixBox.min[0] + 0.5), (int64)std::ceil(pixBox.max[0] - 0.5), (int64)std::floor(pixBox.min[1] + 0.5), (int64)std::ceil(pixBox.max[1] - 0.5));
                            if (px > 2.0)
                                { // adjust horizontal boundary
                                const double dxmin = pixBox.min[0] + 0.5 - siteBox.min[0]; if (dxmin < 0.5) siteBox.min[0]++;
                                const double dxmax = siteBox.max[0] + 0.5 - pixBox.max[0]; if (dxmax <= 0.5) siteBox.max[0]--;
                                }
                            if (py > 2.0)
                                {// adjust vertical boundary
                                const double dymin = pixBox.min[1] + 0.5 - siteBox.min[1]; if (dymin < 0.5) siteBox.min[1]++;
                                const double dymax = siteBox.max[1] + 0.5 - pixBox.max[1]; if (dymax <= 0.5) siteBox.max[1]--;
                                }
                            int64 iR = 0, iG = 0, iB = 0, iA = 0;
                            for (int l = 0; l < batchsize; l++)
                                {
                                const int64 i = siteBox.min[0] + (_fastgen() % (siteBox.max[0] - siteBox.min[0] + 1));
                                const int64 j = siteBox.min[1] + (_fastgen() % (siteBox.max[1] - siteBox.min[1] + 1));
                                const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { i, j }, _opaque);
                                iR += c.comp.R; iG += c.comp.G; iB += c.comp.B; iA += c.comp.A;
                                }
                            imData[off].add(RGBc64((uint16)(iR / batchsize), (uint16)(iG / batchsize), (uint16)(iB / batchsize), (uint16)(iA / batchsize)));
                            normData[off]++;
                            off++;
                            pixBox.min[0] += px; pixBox.max[0] += px;
                            }
                        off += pa;
                        pixBox.min[1] += py;
                        pixBox.max[1] += py;
                        pixBox.min[0] = r.min[0];
                        pixBox.max[0] = r.min[0] + px;
                        }
                    setProgress(5 + (45 * (sd + nbb)) / sampleToDo);
                    }
                sampleDone += (nb*batchsize);
                }


            /* perfect drawing by computing the average color at each pixel */
            void _draw_perfect()
                {
                const double PERFECT_HIGH_DENSITY = 200.0;        // density above which we give weight 1 to all site interecting the pixel (and not weighted by the average of the interesection). 
                const double PERFECT_ULTRAHIGH_DENSITY = 5000.0;  // density above which we check even more often to insure the thread does not get stuck.
                if (_dens < PERFECT_HIGH_DENSITY) _draw_perfect_lowdensity();
                else
                    {
                    if (_dens < PERFECT_ULTRAHIGH_DENSITY) _draw_perfect_highdensity(); else _draw_perfect_ultrahighdensity();
                    }
                setProgress(100);
                return;
                }


            /* draw perfectly, ultra high density : do nothing more than the stochasitc approximation */
            void _draw_perfect_ultrahighdensity()
                {
                // stochastic is good enough, do nothing...
                return;
                }


            /* draw perfectly, hidh density, check() after every pixel */
            void _draw_perfect_highdensity()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const double px = _dlx; // lenght of a screen pixel
                const double py = _dly; // height od a screen pixel
                const int64 ilx = _subBox.lx() + 1; // number of horiz pixel in dest image
                const int64 ily = _subBox.ly() + 1; // number of vert pixel in dest image
                const fBox2 r = _range; // corresponding range
                const int64 width = _im->width();
                const size_t pa = (size_t)(width - ilx);
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                // initial position of the pixel bounding box (left-bottom)
                fBox2 pixBox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);
                for (int64 jj = 0; jj < ily; jj++)
                    {
                    for (int64 ii = 0; ii < ilx; ii++)
                        {
                        check();
                        { // work on the pixel....
                          // compute the integer box of sites that intersect pixBox
                        iBox2 siteBox((int64)std::floor(pixBox.min[0] + 0.5), (int64)std::ceil(pixBox.max[0] - 0.5), (int64)std::floor(pixBox.min[1] + 0.5), (int64)std::ceil(pixBox.max[1] - 0.5));
                        if (px > 2.0)
                            { // adjust horizontal boundary
                            const double dxmin = pixBox.min[0] + 0.5 - siteBox.min[0];  // how much of the left pixels
                            if (dxmin < 0.5) siteBox.min[0]++;
                            const double dxmax = siteBox.max[0] + 0.5 - pixBox.max[0];  // how much of the right pixels
                            if (dxmax <= 0.5) siteBox.max[0]--;
                            }
                        if (py > 2.0)
                            {// adjust vertical boundary
                            const double dymin = pixBox.min[1] + 0.5 - siteBox.min[1];  // how much of the bottom pixels
                            if (dymin < 0.5) siteBox.min[1]++;
                            const double dymax = siteBox.max[1] + 0.5 - pixBox.max[1];  // how much of the top pixels
                            if (dymax <= 0.5) siteBox.max[1]--;
                            }
                        int64 iR = 0, iG = 0, iB = 0, iA = 0;
                        for (int64 j = siteBox.min[1]; j <= siteBox.max[1]; j++)
                            {
                            for (int64 i = siteBox.min[0]; i <= siteBox.max[0]; i++)
                                {
                                const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { i, j }, _opaque);
                                iR += c.comp.R; iG += c.comp.G; iB += c.comp.B; iA += c.comp.A;
                                }
                            }
                        int64 aera = (siteBox.max[0] - siteBox.min[0] + 1)*(siteBox.max[1] - siteBox.min[1] + 1);
                        imData[off] = RGBc64((uint16)(iR / aera), (uint16)(iG / aera), (uint16)(iB / aera), (uint16)(iA / aera));
                        normData[off] = 0;
                        }
                        // ok, next pixel...
                        off++;
                        pixBox.min[0] += px; pixBox.max[0] += px;
                        }
                    off += pa;
                    pixBox.min[1] += py;
                    pixBox.max[1] += py;
                    pixBox.min[0] = r.min[0];
                    pixBox.max[0] = r.min[0] + px;
                    setProgress((int)((50 * jj) / ily + 50));
                    }
                return;
                }


            /* draw perfectly, low density, check() after every line */
            void _draw_perfect_lowdensity()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const double px = _dlx; // lenght of a screen pixel
                const double py = _dly; // height od a screen pixel
                const int64 ilx = _subBox.lx() + 1; // number of horiz pixel in dest image
                const int64 ily = _subBox.ly() + 1; // number of vert pixel in dest image
                const fBox2 r = _range; // corresponding range

                const int64 width = _im->width();
                const size_t pa = (size_t)(width - ilx);
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));

                // for saving the color, cannot match anything initially
                int64  prev_i = (int64)r.min[0] - 3;
                int64  prev_j = (int64)r.min[1] - 3;
                RGBc coul;

                // initial position of the pixel bounding box (left-bottom)
                fBox2 pixBox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);

                for (int64 jj = 0; jj < ily; jj++)
                    {
                    check();
                    for (int64 ii = 0; ii < ilx; ii++)
                        {
                                { // work on the pixel....
                                  // compute the integer box of sites that intersect pixBox
                                iBox2 siteBox((int64)std::floor(pixBox.min[0] + 0.5), (int64)std::ceil(pixBox.max[0] - 0.5), (int64)std::floor(pixBox.min[1] + 0.5), (int64)std::ceil(pixBox.max[1] - 0.5));
                                // switch depending on the size of the box
                                if (siteBox.min[0] == siteBox.max[0])
                                    { // only a vertical line
                                    if (siteBox.min[1] == siteBox.max[1])
                                        { // single pixel
                                        const int64 pix_i = siteBox.min[0];
                                        const int64 pix_j = siteBox.min[1];
                                        if ((prev_i != pix_i) || (prev_j != pix_j))
                                            { // query only if we do not already know the color
                                            coul = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);
                                            prev_i = pix_i; prev_j = pix_j;
                                            }
                                        imData[off] = coul;
                                        normData[off] = 0;
                                        }
                                    else
                                        { // process the vertical line
                                        const int64 pix_i = siteBox.min[0];
                                        int64 pix_j = siteBox.min[1];
                                        const RGBc coul_min = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);
                                        pix_j++;
                                        int64 iR = 0, iG = 0, iB = 0, iA = 0;
                                        while (pix_j < siteBox.max[1])
                                            {
                                            const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);
                                            iR += c.comp.R; iG += c.comp.G; iB += c.comp.B; iA += c.comp.A;
                                            pix_j++;
                                            }
                                        const RGBc coul_max = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);

                                        const double dymin = pixBox.min[1] + 0.5 - siteBox.min[1];  // how much of the bottom pixels
                                        const double dymax = siteBox.max[1] + 0.5 - pixBox.max[1]; // how much of the top pixels
                                        const double aera = dymin + dymax + (double)(siteBox.max[1] - siteBox.min[1] - 1);

                                        double fR = (dymin*coul_min.comp.R + ((double)iR) + dymax*coul_max.comp.R) / aera;
                                        double fG = (dymin*coul_min.comp.G + ((double)iG) + dymax*coul_max.comp.G) / aera;
                                        double fB = (dymin*coul_min.comp.B + ((double)iB) + dymax*coul_max.comp.B) / aera;
                                        double fA = (dymin*coul_min.comp.A + ((double)iA) + dymax*coul_max.comp.A) / aera;

                                        imData[off] = RGBc64((uint16)fR, (uint16)fG, (uint16)fB, (uint16)fA);
                                        normData[off] = 0;
                                        }
                                    }
                                else
                                    {
                                    if (siteBox.min[1] == siteBox.max[1])
                                        { // process the horizontal line
                                        int64 pix_i = siteBox.min[0];
                                        const int64 pix_j = siteBox.min[1];
                                        const RGBc coul_min = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);
                                        pix_i++;
                                        int64 iR = 0, iG = 0, iB = 0, iA = 0;
                                        while (pix_i < siteBox.max[0])
                                            {
                                            const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);
                                            iR += c.comp.R; iG += c.comp.G; iB += c.comp.B; iA += c.comp.A;
                                            pix_i++;
                                            }
                                        const RGBc coul_max = mtools::GetColorSelector<ObjType>::call(*_obj, { pix_i, pix_j }, _opaque);

                                        const double dxmin = pixBox.min[0] + 0.5 - siteBox.min[0];  // how much of the left pixels
                                        const double dxmax = siteBox.max[0] + 0.5 - pixBox.max[0]; // how much of the right pixels
                                        const double aera = dxmin + dxmax + (double)(siteBox.max[0] - siteBox.min[0] - 1);

                                        double fR = (dxmin*coul_min.comp.R + ((double)iR) + dxmax*coul_max.comp.R) / aera;
                                        double fG = (dxmin*coul_min.comp.G + ((double)iG) + dxmax*coul_max.comp.G) / aera;
                                        double fB = (dxmin*coul_min.comp.B + ((double)iB) + dxmax*coul_max.comp.B) / aera;
                                        double fA = (dxmin*coul_min.comp.A + ((double)iA) + dxmax*coul_max.comp.A) / aera;

                                        imData[off] = RGBc64((uint16)fR, (uint16)fG, (uint16)fB, (uint16)fA);
                                        normData[off] = 0;
                                        }
                                    else
                                        { // process the whole square

                                        double fR, fG, fB, fA;
                                        double aera;

                                        // sum over the interior pixel
                                        {
                                        int64 iR = 0, iG = 0, iB = 0, iA = 0;
                                        for (int64 j = siteBox.min[1] + 1; j < siteBox.max[1]; j++)
                                            {
                                            for (int64 i = siteBox.min[0] + 1; i < siteBox.max[0]; i++)
                                                {
                                                const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { i, j }, _opaque);
                                                iR += c.comp.R; iG += c.comp.G; iB += c.comp.B; iA += c.comp.A;
                                                }
                                            }
                                        aera = ((double)(siteBox.max[0] - siteBox.min[0] - 1))*((double)(siteBox.max[1] - siteBox.min[1] - 1));
                                        fR = (double)iR; fG = (double)iG; fB = (double)iB; fA = (double)iA;
                                        }

                                        const double dxmin = pixBox.min[0] + 0.5 - siteBox.min[0];  // how much of the left pixels
                                        const double dxmax = siteBox.max[0] + 0.5 - pixBox.max[0];  // how much of the right pixels
                                        const double dymin = pixBox.min[1] + 0.5 - siteBox.min[1];  // how much of the bottom pixels
                                        const double dymax = siteBox.max[1] + 0.5 - pixBox.max[1];  // how much of the top pixels

                                                                                                    // sum over the 4 corners
                                        {
                                        const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { siteBox.min[0], siteBox.min[1] }, _opaque); const double a = dxmin*dymin;
                                        aera += a; fR += a*c.comp.R; fG += a*c.comp.G; fB += a*c.comp.B; fA += a*c.comp.A;
                                        }
                                        {
                                        const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { siteBox.max[0], siteBox.min[1] }, _opaque); const double a = dxmax*dymin;
                                        aera += a; fR += a*c.comp.R; fG += a*c.comp.G; fB += a*c.comp.B; fA += a*c.comp.A;
                                        }
                                        {
                                        const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { siteBox.min[0], siteBox.max[1] }, _opaque); const double a = dxmin*dymax;
                                        aera += a; fR += a*c.comp.R; fG += a*c.comp.G; fB += a*c.comp.B; fA += a*c.comp.A;
                                        }
                                        {
                                        const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { siteBox.max[0], siteBox.max[1] }, _opaque); const double a = dxmax*dymax;
                                        aera += a;  fR += a*c.comp.R; fG += a*c.comp.G; fB += a*c.comp.B; fA += a*c.comp.A;
                                        }

                                        // sum over the 4 boundary lines
                                        {
                                        int64 uR = 0, uG = 0, uB = 0, uA = 0;
                                        for (int64 i = siteBox.min[0] + 1; i < siteBox.max[0]; i++)
                                            {
                                            const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { i, siteBox.min[1] }, _opaque);
                                            uR += c.comp.R; uG += c.comp.G; uB += c.comp.B; uA += c.comp.A;
                                            }
                                        aera += (siteBox.max[0] - siteBox.min[0] - 1)*dymin;
                                        fR += dymin*uR; fG += dymin*uG; fB += dymin*uB; fA += dymin*uA;
                                        }
                                        {
                                        int64 uR = 0, uG = 0, uB = 0, uA = 0;
                                        for (int64 i = siteBox.min[0] + 1; i < siteBox.max[0]; i++)
                                            {
                                            const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { i, siteBox.max[1] }, _opaque);
                                            uR += c.comp.R; uG += c.comp.G; uB += c.comp.B; uA += c.comp.A;
                                            }
                                        aera += (siteBox.max[0] - siteBox.min[0] - 1)*dymax;
                                        fR += dymax*uR; fG += dymax*uG; fB += dymax*uB; fA += dymax*uA;
                                        }
                                        {
                                        int64 uR = 0, uG = 0, uB = 0, uA = 0;
                                        for (int64 j = siteBox.min[1] + 1; j < siteBox.max[1]; j++)
                                            {
                                            const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { siteBox.min[0], j }, _opaque);
                                            uR += c.comp.R; uG += c.comp.G; uB += c.comp.B; uA += c.comp.A;
                                            }
                                        aera += (siteBox.max[1] - siteBox.min[1] - 1)*dxmin;
                                        fR += dxmin*uR; fG += dxmin*uG; fB += dxmin*uB; fA += dxmin*uA;
                                        }
                                        {
                                        int64 uR = 0, uG = 0, uB = 0, uA = 0;
                                        for (int64 j = siteBox.min[1] + 1; j < siteBox.max[1]; j++)
                                            {
                                            const RGBc c = mtools::GetColorSelector<ObjType>::call(*_obj, { siteBox.max[0], j }, _opaque);
                                            uR += c.comp.R; uG += c.comp.G; uB += c.comp.B; uA += c.comp.A;
                                            }
                                        aera += (siteBox.max[1] - siteBox.min[1] - 1)*dxmax;
                                        fR += dxmax*uR; fG += dxmax*uG; fB += dxmax*uB; fA += dxmax*uA;
                                        }

                                        // compute final color
                                        imData[off] = RGBc64((uint16)(fR / aera), (uint16)(fG / aera), (uint16)(fB / aera), (uint16)(fA / aera));
                                        normData[off] = 0;
                                        }
                                    }
                                }
                                // ok, next pixel...
                                off++;
                                pixBox.min[0] += px; pixBox.max[0] += px;
                        }
                    off += pa;
                    pixBox.min[1] += py;
                    pixBox.max[1] += py;
                    pixBox.min[0] = r.min[0];
                    pixBox.max[0] = r.min[0] + px;
                    setProgress((int)((50 * jj) / ily + 50));
                    }
                return;
                }



            ThreadPixelDrawer(const ThreadPixelDrawer &) = delete;               // no copy
            ThreadPixelDrawer & operator=(const ThreadPixelDrawer &) = delete;   //


            static const int SIGNAL_NEWPARAM = 4;
            static const int SIGNAL_REDRAW = 5;

            ObjType * _obj;                         // the object to draw.
            void * _opaque;                         // opaque data passed to _obj;

            std::atomic<bool> _keepPrevious;        // do we keep something from the previous drawing
            std::atomic<bool> _validParam;          // indicate if the parameter are valid.

            fBox2 _range;                           // the range
            std::atomic<fBox2> _temp_range;         // and the temporary use to communicate with the thread.
            ProgressImg* _im;                       // the image to draw onto
            std::atomic<ProgressImg*> _temp_im;     // and the temporary use to communicate with the thread.
            iBox2 _subBox;                          // part of the image to draw
            std::atomic<iBox2> _temp_subBox;        // and the temporary use to communicate with the thread.

            double _dens;                           // density : average number of sites per pixel
            double _dlx, _dly;                      // horizontal and vertical density (size of image pixel in real coord)
            bool _is1to1;                           // true if we have a 1 to 1 drawing
            iBox2 _range1to1;                       // integer range box in the case of 1 to 1 drawing

            FastRNG _fastgen;                       // fast RNG

        };



    /**
    * PixelDrawer class
    *
    * Use several threads to draw from a getColor function into a progressImg.
    *
    * @tparam  ObjType Type of the object type.
    **/

    template<typename ObjType> class PixelDrawer
        {

        public:

            /**
            * Constructor.
            *
            * @param [in,out]  obj The object to draw
            **/
            PixelDrawer(ObjType * obj, int nbthread = 1) : _obj(obj), _vecThread()
                {
                static_assert(mtools::GetColorSelector<ObjType>::has_getColor, "The object must be implement one of the getColor() method recognized by GetColorSelector.");
                if (nbthread < 1) nbthread = 1;
                nbThreads(nbthread);
                }


            /** Destructor. */
            ~PixelDrawer()
                {
                _deleteAllThread();
                }


            /**
            * Return the current number of threads.
            **/
            int nbThreads() const { return (int)_vecThread.size(); }


            /**
            * Change the number of thread.
            *
            * All threads are disabled and setParam must be called again to set the parameters.
            **/
            void nbThreads(int nb)
                {
                if (nb < 1) nb = 1;
                if (nb == nbThreads()) return;
                _deleteAllThread();
                _vecThread.resize(nb);
                for (size_t i = 0; i < nb; i++) { _vecThread[i] = new ThreadPixelDrawer<ObjType>(_obj); }
                }


            /**
            * Determines if the drawing parameters are valid.
            **/
            inline bool validParam()  const
                {
                if (_vecThread.size() == 0) return false;
                sync();
                for (size_t i = 0; i < _vecThread.size(); i++) { if (!((_vecThread[i])->validParam())) return false; }
                return true;
                }


            /** Synchronises all threads */
            void sync()
                {
                for (size_t i = 0; i < _vecThread.size(); i++)
                    {
                    (_vecThread[i])->sync();
                    }
                }


            /**
            * Get the current progress value (which is the min of the progress of all the threads).
            **/
            inline int progress() const
                {
                if (_vecThread.size() == 0) return 0;
                int pr = (_vecThread[0])->progress();
                for (size_t i = 1; i < _vecThread.size(); i++)
                    {
                    const int q = (_vecThread[i])->progress();
                    if (q < pr) { pr = q; }
                    }
                return pr;
                }


            /**
            * Enables/Disable all the threads.
            **/
            void enable(bool newstatus)
                {
                if (_vecThread.size() == 0) return;
                sync();
                if (newstatus == _vecThread[0]->enable()) return;
                for (size_t i = 0; i < _vecThread.size(); i++) { _vecThread[i]->enable(newstatus); }
                }


            /**
            * Query if the threads are currently enabled.
            **/
            inline bool enable()
                {
                if (_vecThread.size() == 0) return false;
                sync();
                return _vecThread[0]->enable();
                }


            /**
            * Sets the drawing parameters.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            *
            * @param   range       The range to draw.
            * @param [in,out]  im  The image to draw into.
            * @param   subBox      The part of the image to draw (border inclusive). If empty, use the whole
            *                      image.
            **/
            void setParameters(const fBox2 & range, ProgressImg * im, iBox2 subBox = iBox2())
                {
                if (subBox.isEmpty()) { subBox = iBox2(0, im->width() - 1, 0, im->height() - 1); }
                const size_t nt = _vecThread.size();
                const int64 H = subBox.ly() + 1;
                if ((nt == 0) || (H < (int64)(3 * nt))) return;
                int64 h = H / nt;
                size_t m = (size_t)(H % nt);
                iBox2 cbox(subBox.min[0], subBox.max[0], subBox.min[1], subBox.min[1] + h - 1);
                for (size_t i = 0; i < (nt - m); i++)
                    {
                    _vecThread[i]->setParameters(_computeRange(range, subBox, cbox), im, cbox);
                    cbox.min[1] += h; cbox.max[1] += h;
                    }
                cbox.max[1]++;
                for (size_t i = (nt - m); i < nt; i++)
                    {
                    _vecThread[i]->setParameters(_computeRange(range, subBox, cbox), im, cbox);
                    cbox.min[1] += (h + 1); cbox.max[1] += (h + 1);
                    }
                MTOOLS_INSURE(cbox.min[1] == 1 + subBox.max[1]);
                }


            /**
            * Force a redraw.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            **/
            void redraw(bool keepPrevious)
                {
                for (size_t i = 0; i < _vecThread.size(); i++) { (_vecThread[i])->redraw(keepPrevious); }
                }


        private:


            fBox2 _computeRange(fBox2 range, iBox2 subBox, iBox2 cBox)
                {
                const double px = range.lx() / (subBox.lx() + 1);
                const double py = range.ly() / (subBox.ly() + 1);
                const double xmin = range.min[0] + px*(cBox.min[0] - subBox.min[0]);
                const double xmax = range.max[0] - px*(subBox.max[0] - cBox.max[0]);
                const double ymin = range.min[1] + py*(cBox.min[1] - subBox.min[1]);
                const double ymax = range.max[1] - py*(subBox.max[1] - cBox.max[1]);
                return fBox2(xmin, xmax, ymin, ymax);
                }


            /* delete all worker thread */
            void _deleteAllThread()
                {
                for (size_t i = 0; i < _vecThread.size(); i++) { delete(_vecThread[i]); }
                _vecThread.clear();
                }


            // no copy
            PixelDrawer(const PixelDrawer &) = delete;
            PixelDrawer & operator=(const PixelDrawer &) = delete;


            ObjType * _obj;                                             // the object to draw.
            std::vector< ThreadPixelDrawer<ObjType>*  > _vecThread;     // vector of all the threads. 


        };



    }

/* end of file */

