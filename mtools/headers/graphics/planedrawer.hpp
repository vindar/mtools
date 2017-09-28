/** @file planedrawer.hpp */
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
#include "customcimg.hpp"
#include "rgbc.hpp"
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
     * ThreadPlaneDrawer class
     * 
     * Use a single thread to draw from a getColor function into a progressImg. Used by the
     * PlaneDrawer class which combined several instance of the class to optimized the drawing using
     * several threads.
     *
     * @tparam  ObjType Type of the object to draw. Must implement a method recognized by
     *                  GetColorPlaneSelector (cf file getcolorselector.hpp).
     **/
    template<typename ObjType> class ThreadPlaneDrawer : public ThreadWorker
        {


        public:

            /**
            * Constructor. Associate the object. The thread is initially suspended.
            *
            * @param [in,out]  obj     pointer to the object to be drawn. Must implement a method recognized
            *                          by GetColorPlaneSelector
            * @param [in,out]  opaque  (Optional) The opaque data to passed to getColor(), nullptr if not
            *                          specified.
            **/
            ThreadPlaneDrawer(ObjType * obj, void * opaque = nullptr) : ThreadWorker(),
                _obj(obj),
                _opaque(opaque),
                _validParam(false),
                _range(fBox2()),
                _temp_range(fBox2()),
                _im(nullptr),
                _temp_im(nullptr),
                _subBox(iBox2()),
                _temp_subBox(iBox2())
                {
                static_assert(mtools::GetColorPlaneSelector<ObjType>::has_getColor, "The object must be implement one of the getColor() method recognized by GetColorPlaneSelector.");
                }


            /**
            * Destructor. Stop the thread.
            **/
            virtual ~ThreadPlaneDrawer()
                {
                enable(false);
                sync();
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
             **/
            void redraw()
                {
                sync();
                signal(SIGNAL_REDRAW);
                }


        private:


            /**
            * Override from ThreadWorker.
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
                const double RANGE_MIN_VALUE = std::numeric_limits<double>::min() * 100000;
                const double RANGE_MAX_VALUE = std::numeric_limits<double>::max() / 100000;
                _range = _temp_range;
                _im = _temp_im;
                _subBox = _temp_subBox;
                if ((_im == nullptr) || (_im->width() < MIN_IMAGE_SIZE) || (_im->height() < MIN_IMAGE_SIZE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; }   // make sure im is not nullptr and is big enough.
                if (_subBox.isEmpty()) { _subBox = iBox2(0, _im->width() - 1, 0, _im->height() - 1); } // subbox = whole image if empty. 
                if ((_subBox.min[0] < 0) || (_subBox.max[0] >= (int64)_im->width()) || (_subBox.min[1] < 0) || (_subBox.max[1] >= (int64)_im->height())) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // make sure _subBox is a proper subbox of im
                if ((_subBox.lx() < MIN_IMAGE_SIZE) || (_subBox.ly() < MIN_IMAGE_SIZE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // make sure _subBox is a proper subbox of im
                const double rlx = _range.lx();
                const double rly = _range.ly();
                if ((rlx < RANGE_MIN_VALUE) || (rly < RANGE_MIN_VALUE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // prevent zooming in too far
                if ((std::abs(_range.min[0]) > RANGE_MAX_VALUE) || (std::abs(_range.max[0]) > RANGE_MAX_VALUE) || (std::abs(_range.min[1]) > RANGE_MAX_VALUE) || (std::abs(_range.max[1]) > RANGE_MAX_VALUE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // prevent zooming out too far
                setProgress(0);
                _validParam = true;
                return THREAD_RESET;
                }



            /* trigger a redraw */
            int _setRedraw()
                {
                if (!((bool)_validParam)) return THREAD_RESET_AND_WAIT;
                setProgress(0);
                return THREAD_RESET;
                }



            /**
            * Override from ThreadWorker.
            * The main 'work' method
            **/
            virtual void work() override
                {
                MTOOLS_INSURE((bool)_validParam);
                _drawFast();
                setProgress(1);
                for (int i = 0; i < 254;i++)
                    {
                    _drawStochastic();
                    setProgress(1 + (i*99)/255);
                    }
                setProgress(100);
                }



            /* draw by sampling the color at the center of each pixel */
            void _drawFast()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const fBox2 r = _range;
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                const double px = r.lx() / ilx;
                const double py = r.ly() / ily;
                const double px2 = px / 2;
                const double py2 = py / 2;
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                const size_t pa = (size_t)(_im->width() - ilx);
                fBox2 cbox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);
                for (int64 j = 0; j < ily; j++)
                    {
                    check();
                    for (int64 i = 0; i < ilx; i++)
                        {
                        imData[off] = (mtools::GetColorPlaneSelector<ObjType>::call(*_obj, fVec2{ cbox.min[0] + px2 , cbox.min[1] + py2 }, cbox, 1, _opaque)).first;
                        normData[off] = 0;
                        off++;
                        cbox.min[0] += px;
                        cbox.max[0] += px;
                        }
                    off += pa;
                    cbox.min[1] += py;
                    cbox.max[1] += py;
                    cbox.min[0] = r.min[0];
                    cbox.max[0] = r.min[0] + px;
                    }
                }



            void _drawStochastic()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const fBox2 r = _range;
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                const double px = r.lx() / ilx;
                const double py = r.ly() / ily;
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                const size_t pa = (size_t)(_im->width() - ilx);
                fBox2 cbox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);
                for (int64 j = 0; j < ily; j++)
                    {
                    check();
                    for (int64 i = 0; i < ilx; i++)
                        {
                        std::pair<RGBc, bool> P = mtools::GetColorPlaneSelector<ObjType>::call(*_obj, fVec2{ cbox.min[0] + _fastgen.unif()*px , cbox.min[1] + _fastgen.unif()*py }, cbox, 1, _opaque);
                        if (P.second)
                            {
                            imData[off] = P.first;
                            normData[off] = 0;
                            }
                        else
                            {
                            imData[off].add(P.first);
                            normData[off]++;
                            }
                        off++;
                        cbox.min[0] += px;
                        cbox.max[0] += px;
                        }
                    off += pa;
                    cbox.min[1] += py;
                    cbox.max[1] += py;
                    cbox.min[0] = r.min[0];
                    cbox.max[0] = r.min[0] + px;
                    }
                }


            // no copy
            ThreadPlaneDrawer(const ThreadPlaneDrawer &) = delete;
            ThreadPlaneDrawer & operator=(const ThreadPlaneDrawer &) = delete;


            static const int SIGNAL_NEWPARAM = 4;
            static const int SIGNAL_REDRAW = 5;

            ObjType * _obj;                         // the object to draw.
            void * _opaque;                         // opaque data passed to _obj;

            std::atomic<bool> _validParam;          // indicate if the parameter are valid.

            fBox2 _range;                           // the range
            std::atomic<fBox2> _temp_range;         // and the temporary use to communicate with the thread.
            ProgressImg* _im;                       // the image to draw onto
            std::atomic<ProgressImg*> _temp_im;     // and the temporary use to communicate with the thread.
            iBox2 _subBox;                          // part of the image to draw
            std::atomic<iBox2> _temp_subBox;        // and the temporary use to communicate with the thread.

            FastRNG _fastgen;                       // fast RNG

        };



    /**
     * PlaneDrawer class
     * 
     * Combine several instances of ThreadPlaneDrawer (hence use several threads) to draw from a
     * getColor function into a progressImg.
     *
     * @tparam  ObjType Type of the object to draw. Must implement a method recognized by
     *                  GetColorPlaneSelector.
     **/
    template<typename ObjType> class PlaneDrawer
        {

        public:

            /**
            * Constructor.
            *
            * @param [in,out]  obj The object to draw
            **/
            PlaneDrawer(ObjType * obj, int nbthread = 1) :  _obj(obj), _vecThread()
                {
                static_assert(mtools::GetColorPlaneSelector<ObjType>::has_getColor, "The object must be implement one of the getColor() methods recognized by GetColorPlaneSelector.");
                if (nbthread < 1) nbthread = 1;
                nbThreads(nbthread);
                }


            /** Destructor. */
            ~PlaneDrawer()
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
                for (int i = 0; i < nb; i++) { _vecThread[i] = new ThreadPlaneDrawer<ObjType>(_obj); }
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
            void redraw()
                {
                for (size_t i = 0; i < _vecThread.size(); i++) { (_vecThread[i])->redraw(); }
                }


        private:


            /* compute the range of a subbox */
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
            PlaneDrawer(const PlaneDrawer &) = delete;
            PlaneDrawer & operator=(const PlaneDrawer &) = delete;


            ObjType * _obj;                                             // the object to draw.
            std::vector< ThreadPlaneDrawer<ObjType>*  > _vecThread;     // vector of all the threads. 


        };




}

/* end of file */

