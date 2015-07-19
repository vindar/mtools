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


#include "drawable2Dobject.hpp"
#include "customcimg.hpp"
#include "rgbc.hpp"
#include "../maths/vec.hpp"
#include "../maths/rect.hpp"
#include "../misc/misc.hpp"
#include "../misc/metaprog.hpp"

#include <algorithm>
#include <ctime>
#include <mutex>
#include <atomic>


namespace mtools
{

    using cimg_library::CImg;

    /**
     * Encapsulate a `getColor()` function into a plane object that can be used with the PlaneDrawer
     * class. This wrapper class contain no data and just a static method, therefore, there is no need 
     * to create an instance and one can just pass nullptr to the LatticeDrawer as the associated object.
     * 
     * @tparam  getColorFun The getColor method that will be called when querying the color of a
     *                      point. The signature must match `mtools::RGBc getColor(mtools::fVec2 pos)`.
     **/
    template<mtools::RGBc (*getColorFun)(mtools::fVec2 pos)> class PlaneObj
        {
        public:
       inline static RGBc getColor(mtools::fVec2 pos) { return getColorFun(pos); }
       inline static PlaneObj<getColorFun> * get() { return(nullptr); }
        };


    /**
     * Encapsulate an extended `getColor()` function into a plane object that can be used with the
     * PlaneDrawer class. This wrapper class contain no data and just a static method, therefore,
     * there is no need to create an instance and one can just pass nullptr to the LatticeDrawer as
     * the associated object.
     *
     * @tparam  getColorFun The getColor method that will be called when querying the color of a
     *                      point. The signature must match `mtools::RGBc getColor(mtools::fVec2 pos,
     *                      fRect R)` (here R contain pos and represent the pixel containing the
     *                      point at pos.
     **/
    template<mtools::RGBc(*getColorFun)(mtools::fVec2 pos, fRect R)> class PlaneObjExt
        {
        public:
            inline static RGBc getColor(mtools::fVec2 pos, fRect R) { return getColorFun(pos, R); }
            inline static PlaneObjExt<getColorFun> * get() { return(nullptr); }
        };


/**
 * Draws part of a plane into into a CImg image. This class implement the Drawable2DObject
 * interface.
 * 
 * - The parameters of the drawing are set using the `setParam`  method. The method `work` is
 * used to create the drawing itself. The actual warping of the image onto a given CImg image is
 * performed using the `drawOnto` method.
 * 
 * - All the public methods of this class are thread-safe : they can be called simultaneously
 * from any thread and the call are lined up. In particular, the `work` method can be time
 * expensive and might be better called from a worker thread : see the `AutoDrawable2DObject`
 * class for a generic implementation.
 * 
 * - The template PlaneObj must implement a method `RGBc getColor(fVec2 pos)` or `RGBc
 * getColor(fVec2 pos, fRect R)` which must return the color associated with a given point. In
 * the seocnd version, the rectangle R contain the point pos and represent the aera of the
 * pixel drawn. The method should be made as fast as possible.
 * 
 * - The fourth channel of the returned color will be used when drawing on 4 channel images and
 * ignored when drawing on 3 channel images.
 *
 * @tparam  PlaneObj    Type of the lattice object. Can be any class which define the method
 *                      `RGBc getColor(fVec2 pos)` or `RGBc getColor(fVec2 pos, fRect R)`. If both
 *                      method are defined, the extended method is used.
 **/
template<class PlaneObj> class PlaneDrawer : public mtools::internals_graphics::Drawable2DObject
{
  
public:

    /**
     * Constructor. Set the plane object that will be drawn. 
     *
     * @param [in,out]  obj The planar object to draw, it must survive the drawer.
     **/
    PlaneDrawer(PlaneObj * obj) : 
		_g_requestAbort(0), 
		_g_current_quality(0), 
		_g_obj(obj), 
		_g_imSize(201, 201), 
		_g_r(-100.5, 100.5, -100.5, 100.5), 
		_g_redraw(true)
		{
        static_assert(((mtools::metaprog::has_getColor<PlaneObj, RGBc, fVec2>::value)|| (mtools::metaprog::has_getColorExt<PlaneObj, RGBc, fVec2, fRect>::value)), "The object T must be implement either a 'RGBc getColor(fVec2 pos)' or 'RGBc getColor(fVec2 pos, fRect R)' method.");
        _initInt16Buf();
		_initRand();
        domainFull();
        }


    /**
     * Destructor.
     **/
	~PlaneDrawer()
		{
        _g_requestAbort++; // request immediate stop of the work method if active.
            {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we acquire the lock 
            _removeInt16Buf(); // remove the int16 buffer
            }
        }

    /**
    * Get the definiton domain of the plane (does not interrupt any computation in progress).
    * By default this is everything.
    *
    * @return  An fRect.
    **/
    mtools::fRect domain() const
        {
        return _g_domR;
        }


    /**
     * Query if the domain is the whole plane (does not interrupt any computation in progress).
     *
     * @return  true if the domain is full, false if not.
     **/
    bool isDomainFull() const
        {
        return ((_g_domR.xmin <= -DBL_MAX / 2) && (_g_domR.xmax >= DBL_MAX / 2) && (_g_domR.ymin <= -DBL_MAX / 2) && (_g_domR.ymax >= DBL_MAX / 2));
        }


    /**
     * Queries if the domain is empty (does not interrupt any computation in progress).
     *
     * @return  true if the domain is empty, false if not.
     **/
    bool isDomainEmpty() const
        {
        return _g_domR.isEmpty();
        }


    /**
     * Set the definition domain. It is guaranted that no point outside of the domain are never
     * queried via getColor().
     *
     * @param   R   The new definition domain.
     **/
    void domain(mtools::fRect R)
    {
        ++_g_requestAbort; // request immediate stop of the work method if active.
        {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we aquire the lock 
            --_g_requestAbort; // and then remove the stop request
            _g_domR = R;
            _g_redraw = true;   // request redraw
        }
    }


    /**
    * Set a full definition Domain.
    **/
    void domainFull()
    {
        ++_g_requestAbort; // request immediate stop of the work method if active.
        {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we aquire the lock 
            --_g_requestAbort; // and then remove the stop request
            _g_domR.xmin = - DBL_MAX/2;
            _g_domR.xmax = DBL_MAX / 2;
            _g_domR.ymin = -DBL_MAX / 2;
            _g_domR.ymax = DBL_MAX / 2;
            _g_redraw = true;   // request redraw
        }
    }


    /**
    * Set an empty definition Domain.
    **/
    void domainEmpty()
    {
        ++_g_requestAbort; // request immediate stop of the work method if active.
        {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we aquire the lock 
            --_g_requestAbort; // and then remove the stop request
            _g_domR.clear(); // clear the domain
            _g_redraw = true;   // request redraw
        }
    }



    /**
    * Set the parameters of the drawing. Calling this method interrupt any work() in progress. 
    * This method is fast, it does not draw anything.
    **/
    virtual void setParam(mtools::fRect range, mtools::iVec2 imageSize) override
        {
        MTOOLS_ASSERT(!range.isEmpty());
        MTOOLS_ASSERT((imageSize.X() >0) && (imageSize.Y()>0));
        ++_g_requestAbort; // request immediate stop of the work method if active.
            {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we acquire the lock 
            --_g_requestAbort; // and then remove the stop request
            _g_imSize = imageSize; // set the new image size
            _g_r = range; // and the new range
			_g_current_quality = 0; // 0 quality
			_g_redraw = true; // we should start over
            }
        }


    /**
     * Force a reset of the drawing. Calling this method interrupt any work() is progress. This
     * method is fast, it does not draw anything.
     **/
    virtual void resetDrawing() override
        {
        ++_g_requestAbort; // request immediate stop of the work method if active.
            {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we acquire the lock 
            --_g_requestAbort; // and then remove the stop request
			_g_current_quality = 0; // 0 quality
			_g_redraw = true; // we should start over
            }
        }


    /**
    * Draw onto a given image. This method does not "compute" anything. It simply warp the current
    * drawing onto a given cimg image.
    *
    * The provided cimg image must have 3 or 4 channel and the same size as that set via
    * setParam(). The current drawing of the plane has its opacity channel multiplied by the
    * opacity parameter and is then superposed with the current content of im using the A over B
    * operation. If im has only 3 channel, it is considered as having full opacity.
    *
    * @param [in,out]  im  The image to draw onto (must be a 3 or 4 channel image and it size must
    *                      be equal to the size previously set via the setParam() method.
    * @param   opacity     The opacity that should be applied to the picture prior to drawing onto
    *                      im. If set to 0.0, then the method returns without drawing anything.
    *
    * @return  The quality of the drawing performed (0 = nothing drawn, 100 = perfect drawing).
    **/
    virtual int drawOnto(cimg_library::CImg<unsigned char> & im, float opacity = 1.0) override
        {
        MTOOLS_ASSERT((im.width() == _g_imSize.X()) && (im.height() == _g_imSize.Y()));
        MTOOLS_ASSERT((im.spectrum() == 3) || (im.spectrum() == 4));
        ++_g_requestAbort; // request immediate stop of the work method if active.
            {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we aquire the lock 
            --_g_requestAbort; // and then remove the stop request
            if (opacity > 0.0)
                {
                _warp(im, opacity); 
                }
            return _g_current_quality; // return the quality of the drawing
            }
        }


    /**
     * Return the quality of the current drawing. Very fast and does not interrupt any other
     * method/work that might be in progress.
     *
     * @return  The current quality between 0 (nothing to show) and 100 (perfect drawing).
     **/
    virtual int quality() const  override {return _g_current_quality;}


    /**
     * Works on the drawing for a maximum specified period of time.
     * 
     * The function has the lowest priority of all the public methods and may be interrupted (hence
     * returning early) if another method such as drawOnto(),setParam()... is accessed by another
     * thread.
     * 
     * If another thread already launched some work, this method will wait until the lock is
     * released (but will return if the time allowed is exceeded).
     * 
     * If maxtime_ms = 0, then the function return immediately (with the current quality of the
     * drawing).
     *
     * @param   maxtime_ms  The maximum time in millisecond allowed for drawing.
     *
     * @return  The quality of the current drawing:  0 = nothing to show, 100 = perfect drawing.
     **/
    virtual int work(int maxtime_ms) override
        {
        MTOOLS_ASSERT(maxtime_ms >= 0);
        if (((int)_g_requestAbort > 0) || (maxtime_ms <= 0)) { return _g_current_quality; } // do not even try to work if the flag is set
        if (!_g_lock.try_lock_for(std::chrono::milliseconds((maxtime_ms/2)+1))) { return _g_current_quality; } // could not lock the mutex, we return without doing anything
        if ((int)_g_requestAbort > 0) { _g_lock.unlock(); return _g_current_quality; } // do not even try to work if the flag is set
        _work(maxtime_ms); // go to work...
        _g_lock.unlock(); // release mutex
        return _g_current_quality; // ..and return quality
        }


    /**
     * This object need work to construct a drawing. Returns true without interrupting anything.
     *
     * @return  true.
     **/
    virtual bool needWork() const  override { return true; }



    /**
     * Stop any ongoing work and then return
     **/
    virtual void stopWork() override
        {
        ++_g_requestAbort; // request immediate stop of the work method if active.
            {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we aquire the lock 
            --_g_requestAbort; // and then remove the stop request
            }
        }



private:



	/**************************************************************************************************************************************************
	*                                                                    PRIVATE PART 
	************************************************************************************************************************************************/

    std::timed_mutex  _g_lock;             // mutex for locking
    std::atomic<int>  _g_requestAbort;     // flag used for requesting the work() method to abort.
    mutable std::atomic<int> _g_current_quality;  // the current quality of the drawing
    PlaneObj *        _g_obj;               // the object to draw
    iVec2             _g_imSize;            // size of the drawing
    fRect             _g_r;                 // current range
    std::atomic<bool> _g_redraw;            // true if we should redraw from scratch
    fRect           _g_domR;                // the definition domain

    uint32 			_counter1,_counter2;	// counter for the number of pixel added in each cell: counter1 for cells < (_qi,_qj) and counter2 for cells >= (_qi,qj)
    uint32 			_qi,_qj;		        // position where we stopped previously
    int 			_phase;			        // the current phase of the drawing


/* update the quality of the picture  */
void _qualityPixelDraw() const
    {
    switch (_phase)
        {
        case 0: {_g_current_quality = 0; break; }
        case 1: {_g_current_quality = _getLinePourcent(_counter2, 255, 1, 99); break; }
        case 2: {_g_current_quality = 100; break; }
        default: MTOOLS_INSURE(false); // wtf are we doing here
        }
    return;
    }

/* draw as much as possible of a fast drawing  */
void _drawPixel_fast(int maxtime_ms)
	{
    const fRect r = _g_r;
    const double px = ((double)r.lx()) / ((double)_int16_buffer_dim.X())  // size of a pixel
               , py = ((double)r.ly()) / ((double)_int16_buffer_dim.Y()); 
    _counter1 = 1;
	_counter2 = 0;
	bool fixstart = true; 
    for (int j = 0; j < _int16_buffer_dim.Y(); j++)
    for (int i = 0; i < _int16_buffer_dim.X(); i++)
		{
		if (fixstart) {i = _qi; j = _qj; fixstart=false;}					        // fix the position of thestarting pixel 
		if (_isTime(maxtime_ms)) {_qi = i; _qj = j; return;}	                    // time's up : we quit
        const double xmin = r.xmin + i*px; const double xmax = xmin + px; const double x = xmin + 0.5*px;
        const double ymax = r.ymax - j*py; const double ymin = ymax - py; const double y = ymax - 0.5*py;
        const fRect sR = fRect(xmin, xmax, ymin, ymax);
        const RGBc coul = _getColor(fVec2(x,y),sR);
        _setInt16Buf(i, j, coul);
		}
	// we are done
	_counter2 = 1; _qi=0; _qj=0; 
    _phase = 1;
	return;
	}


/* draw as using random points */
void _drawPixel_stochastic(int maxtime_ms)
	{
    const fRect r = _g_r;
    const double px = ((double)r.lx()) / ((double)_int16_buffer_dim.X())  // size of a pixel
               , py = ((double)r.ly()) / ((double)_int16_buffer_dim.Y());
    const uint32 ndraw = _nbDrawPerTurn(r, _int16_buffer_dim);
    while (_counter2 < 255)
        {
        if (_counter2 == _counter1) { ++_counter1; } // start of a loop: we increase counter1 
        bool fixstart = true;
        for (int j = 0; j < _int16_buffer_dim.Y(); j++)
        for (int i = 0; i < _int16_buffer_dim.X(); i++)
            {
            if (fixstart) { i = _qi; j = _qj; fixstart = false; }		// fix the position of thestarting pixel
            if (_isTime(maxtime_ms)) { _qi = i; _qj = j; return; }	// time's up : we quit
            uint32 R = 0, G = 0, B = 0, A = 0;
            const double xmin = r.xmin + i*px; const double xmax = xmin + px;
            const double ymax = r.ymax - j*py; const double ymin = ymax - py;
            const fRect sR = fRect(xmin, xmax, ymin, ymax);
            for (uint32 k = 0;k<ndraw;k++)
                {
                const double x = xmin + _rand_double0()*px;
                const double y = ymax - _rand_double0()*py;
                RGBc coul = _getColor(fVec2(x, y), sR);
                R += coul.R; G += coul.G; B += coul.B; A += coul.A;
                }
            _addInt16Buf(i, j, R / ndraw, G / ndraw, B / ndraw, A / ndraw);
            }
        // we finished a loop
        _counter2 = _counter1;	_qi = 0; _qj = 0;
        }
    _phase = 2; // we are finished with drawing
    return;
	}




/* the main method for drawing a pixel image */
void _work(int maxtime_ms)
    {
    _startTimer();
    if (_g_imSize != _int16_buffer_dim) { _g_redraw = true; } //set redraw to true if the size of the image changed
    if (_g_redraw)
        { // we must completly redraw, initialize everything
        _g_redraw = false;
        _qi = 0; _qj = 0;
        _counter1 = 0; _counter2 = 0;
        _resizeInt16Buf(_g_imSize);
        _phase = 0;
        }
    if (maxtime_ms > 0) 
        {
        while ((_phase != 2) && (!_isTime(maxtime_ms)))
            {
            switch (_phase)
                {
                case 0: // fast drawing phase : start from _qi,qj and make the fastest drawing possible
                    {
                    _drawPixel_fast(maxtime_ms);
                    break;
                    }
                case 1: // stochastic drawing phase : start from _qi,qj and make a stochastic drawing
                    {
                    _drawPixel_stochastic(maxtime_ms);
                    break;
                    }
                default: MTOOLS_INSURE(false); // wtf are we doing here
                }
            }
        }
    _qualityPixelDraw(); // update the quality
    return;
    }


/* used when there is an extended getColor() method */
inline RGBc _getColor(fVec2 pos, fRect R, mtools::metaprog::dummy<true> D) { return _g_obj->getColor(pos, R); }

/* used when there is only a regular getColor() method */
inline RGBc _getColor(fVec2 pos, fRect R, mtools::metaprog::dummy<false> D) { return _g_obj->getColor(pos); }

/* return the color of a given point, use either the object getColor or "extended" getColor method depending on the method detected */
inline RGBc _getColor(fVec2 pos, fRect R) 
    {
    if (!_g_domR.isInside(pos)) return RGBc::c_TransparentWhite;
    return _getColor(pos, R, mtools::metaprog::dummy<mtools::metaprog::has_getColorExt<PlaneObj, RGBc, fVec2, fRect>::value>()); 
    }



// *****************************
// Dealing with the int16 buffer 
// *****************************
uint16 * _int16_buffer;						// buffer for pixel drawing
iVec2    _int16_buffer_dim;                 // dimension of the buffer;

/* initialise the buffer */
inline void _initInt16Buf()
	{
	_int16_buffer = nullptr; 
    _int16_buffer_dim = iVec2(0, 0);
	}

/* remove the int16 buffer */
inline void _removeInt16Buf() {_resizeInt16Buf(iVec2(0,0));}

/* resize the buffer to the given dimension */
inline void _resizeInt16Buf(iVec2 nSize)
	{
    const int64 prod = nSize.X()*nSize.Y();
	if (prod == _int16_buffer_dim.X()*_int16_buffer_dim.Y()) {return;}
	delete []  _int16_buffer; 
    if (prod == 0) { _int16_buffer = nullptr; _int16_buffer_dim = iVec2(0, 0); return; }
	_int16_buffer = new uint16[(size_t)(prod*4)];
    _int16_buffer_dim = nSize;
	}

/* set a color at position (i,j) */
inline void _setInt16Buf(uint32 x,uint32 y,const RGBc & color)
	{
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
    _int16_buffer[x + y*dx] = color.R;
    _int16_buffer[x + y*dx + dxy] = color.G;
    _int16_buffer[x + y*dx + 2 * dxy] = color.B;
    _int16_buffer[x + y*dx + 3 * dxy] = color.A;
    }

/* add a color at position (i,j) */
inline void _addInt16Buf(uint32 x,uint32 y,uint32 R,uint32 G,uint32 B,uint32 A)
	{
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
    _int16_buffer[x + y*dx] += R;
	_int16_buffer[x + y*dx + dxy] += G;
	_int16_buffer[x + y*dx + 2*dxy] += B;
    _int16_buffer[x + y*dx + 3*dxy] += A;
    }



// *****************************
// Warping the buffer onto the image
// *****************************


/* the main method for warping the pixel image to the cimg image*/
void _warp(cimg_library::CImg<unsigned char> & im, float opacity)
    {
    MTOOLS_ASSERT((im.spectrum() == 3) || (im.spectrum() == 4));
    _work(0); // make sure everything is in sync. 
    if (_g_current_quality > 0)
        {
        if (im.spectrum() == 4) { _warpInt16Buf_4channel(im, opacity); } else { _warpInt16Buf_3channel(im, opacity); }
        }
    return;
    }



/* make B -> A and return the resulting opacity */
inline unsigned char _blendcolor4(unsigned char & A, float opA, unsigned char B, float opB) const
{
    float o = opB + opA*(1 - opB); //resulting opacity
    if (o == 0)  return 0;
    A = (unsigned char)((B*opB + A*opA*(1 - opB)) / o);
    return (unsigned char)(255 * o);
}


/* warp the buffer onto an image using _qi,_qj,_counter1 and _counter2 :
method when im has four channels : use transparency and A over B operation
*/
inline void _warpInt16Buf_4channel(CImg<unsigned char> & im, float op) const
{
    MTOOLS_ASSERT(im.spectrum() == 4);
    const float po = 1.0;
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
    const size_t l1 = _qi + (dx*_qj);
    const size_t l2 = (dxy)-l1;
    if (l1>0)
    {
        unsigned char * pdest0 = im.data(0, 0, 0, 0);
        unsigned char * pdest1 = im.data(0, 0, 0, 1);
        unsigned char * pdest2 = im.data(0, 0, 0, 2);
        unsigned char * pdest_opa = im.data(0, 0, 0, 3);
        uint16 * psource0 = _int16_buffer;
        uint16 * psource1 = _int16_buffer + dxy;
        uint16 * psource2 = _int16_buffer + 2 * dxy;
        uint16 * psource_opb = _int16_buffer + 3 * dxy;
        if (_counter1 == 0) {/* memset(pdest,0,l1); */ }
        else
        {
            if (_counter1 == 1)
            {
                for (size_t i = 0; i < l1; i++)
                {
                    const float g = ((op*(*psource_opb)) / 255);
                    _blendcolor4((*pdest0), ((po*(*pdest_opa)) / 255), (unsigned char)(*psource0), g);
                    _blendcolor4((*pdest1), ((po*(*pdest_opa)) / 255), (unsigned char)(*psource1), g);
                    (*pdest_opa) = _blendcolor4((*pdest2), ((po*(*pdest_opa)) / 255), (unsigned char)(*psource2), g);
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
            else
            {
                for (size_t i = 0; i < l1; i++)
                {
                    const float g = ((op*(*psource_opb) / _counter1) / 255);
                    _blendcolor4((*pdest0), ((po*(*pdest_opa)) / 255), (unsigned char)((*psource0) / _counter1), g);
                    _blendcolor4((*pdest1), ((po*(*pdest_opa)) / 255), (unsigned char)((*psource1) / _counter1), g);
                    (*pdest_opa) = _blendcolor4((*pdest2), ((po*(*pdest_opa)) / 255), (unsigned char)((*psource2) / _counter1), g);
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
        }
    }
    if (l2>0)
    {
        unsigned char * pdest0 = im.data(_qi, _qj, 0, 0);
        unsigned char * pdest1 = im.data(_qi, _qj, 0, 1);
        unsigned char * pdest2 = im.data(_qi, _qj, 0, 2);
        unsigned char * pdest_opa = im.data(_qi, _qj, 0, 3);
        uint16 * psource0 = _int16_buffer + +l1;
        uint16 * psource1 = _int16_buffer + dxy + l1;
        uint16 * psource2 = _int16_buffer + 2 * dxy + l1;
        uint16 * psource_opb = _int16_buffer + 3 * dxy + l1;
        if (_counter2 == 0) {/* memset(pdest,0,l2); */ }
        else
        {
            if (_counter2 == 1)
            {
                for (size_t i = 0; i<l2; i++)
                {
                    const float g = (op*(*psource_opb)) / 255;
                    _blendcolor4((*pdest0), (po*(*pdest_opa)) / 255, (unsigned char)(*psource0), g);
                    _blendcolor4((*pdest1), (po*(*pdest_opa)) / 255, (unsigned char)(*psource1), g);
                    (*pdest_opa) = _blendcolor4((*pdest2), (po*(*pdest_opa)) / 255, (unsigned char)(*psource2), g);
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
            else
            {
                for (size_t i = 0; i<l2; i++)
                {
                    const float g = (op*(*psource_opb) / _counter2) / 255;
                    _blendcolor4((*pdest0), (po*(*pdest_opa)) / 255, (unsigned char)((*psource0) / _counter2), g);
                    _blendcolor4((*pdest1), (po*(*pdest_opa)) / 255, (unsigned char)((*psource1) / _counter2), g);
                    (*pdest_opa) = _blendcolor4((*pdest2), (po*(*pdest_opa)) / 255, (unsigned char)((*psource2) / _counter2), g);
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
        }
    }
    return;
}


/* make B -> A when A has full opacity */
inline void _blendcolor3(unsigned char & A, unsigned char B, float opB) const
{
    A = (unsigned char)(B*opB + A*(1.0 - opB));
}


/* warp the buffer onto an image using _qi,_qj,_counter1 and _counter2 :
method when im has 3 channels (same as if the fourth channel was completely opaque)
*/
inline void _warpInt16Buf_3channel(CImg<unsigned char> & im, float op) const
{
    MTOOLS_ASSERT(im.spectrum() == 3);
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
    const size_t l1 = _qi + (dx*_qj);
    const size_t l2 = (dxy)-l1;
    if (l1>0)
    {
        unsigned char * pdest0 = im.data(0, 0, 0, 0);
        unsigned char * pdest1 = im.data(0, 0, 0, 1);
        unsigned char * pdest2 = im.data(0, 0, 0, 2);
        uint16 * psource0 = _int16_buffer;
        uint16 * psource1 = _int16_buffer + dxy;
        uint16 * psource2 = _int16_buffer + 2 * dxy;
        uint16 * psource_opb = _int16_buffer + 3 * dxy;
        if (_counter1 == 0) {/* memset(pdest,0,l1); */ }
        else
        {
            if (_counter1 == 1)
            {
                for (size_t i = 0; i < l1; i++)
                {
                    const float g = ((op*(*psource_opb)) / 255);
                    _blendcolor3((*pdest0), (unsigned char)(*psource0), g);
                    _blendcolor3((*pdest1), (unsigned char)(*psource1), g);
                    _blendcolor3((*pdest2), (unsigned char)(*psource2), g);
                    ++pdest0; ++pdest1; ++pdest2;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
            else
            {
                for (size_t i = 0; i < l1; i++)
                {
                    const float g = ((op*(*psource_opb) / _counter1) / 255);
                    _blendcolor3((*pdest0), (unsigned char)((*psource0) / _counter1), g);
                    _blendcolor3((*pdest1), (unsigned char)((*psource1) / _counter1), g);
                    _blendcolor3((*pdest2), (unsigned char)((*psource2) / _counter1), g);
                    ++pdest0; ++pdest1; ++pdest2;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
        }
    }
    if (l2>0)
    {
        unsigned char * pdest0 = im.data(_qi, _qj, 0, 0);
        unsigned char * pdest1 = im.data(_qi, _qj, 0, 1);
        unsigned char * pdest2 = im.data(_qi, _qj, 0, 2);
        uint16 * psource0 = _int16_buffer + +l1;
        uint16 * psource1 = _int16_buffer + dxy + l1;
        uint16 * psource2 = _int16_buffer + 2 * dxy + l1;
        uint16 * psource_opb = _int16_buffer + 3 * dxy + l1;
        if (_counter2 == 0) {/* memset(pdest,0,l2); */ }
        else
        {
            if (_counter2 == 1)
            {
                for (size_t i = 0; i<l2; i++)
                {
                    const float g = (op*(*psource_opb)) / 255;
                    _blendcolor3((*pdest0), (unsigned char)(*psource0), g);
                    _blendcolor3((*pdest1), (unsigned char)(*psource1), g);
                    _blendcolor3((*pdest2), (unsigned char)(*psource2), g);
                    ++pdest0; ++pdest1; ++pdest2;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
            else
            {
                for (size_t i = 0; i<l2; i++)
                {
                    const float g = (op*(*psource_opb) / _counter2) / 255;
                    _blendcolor3((*pdest0), (unsigned char)((*psource0) / _counter2), g);
                    _blendcolor3((*pdest1), (unsigned char)((*psource1) / _counter2), g);
                    _blendcolor3((*pdest2), (unsigned char)((*psource2) / _counter2), g);
                    ++pdest0; ++pdest1; ++pdest2;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                }
            }
        }
    }
    return;
}


// ****************************************************************
// TIME FUNCTIONS : independant of everything else
// ****************************************************************

static const int _maxtic = 100;	    // number of tic until we look for time
int _tic;							// current tic
clock_t _stime;						// start time

/* start the timer */
inline void _startTimer() {_stime = clock(); _tic = _maxtic; }


/* return true when time ms milliseconds has passed since calling startTimer() */
inline bool _isTime(uint32 ms)
	{
	++_tic;
    if (_g_requestAbort > 0) {return true; }
    if (_tic < _maxtic) return false;
    _qualityPixelDraw(); // update the quality of the drawing
	if (((clock() - _stime)*1000)/CLOCKS_PER_SEC > (clock_t)ms) {_tic = _maxtic; return true;}
	_tic = 0;
	return false;
	}



// ****************************************************************
// RANDOM NUMBER GENERATION : independant of everything else
// ****************************************************************
uint32 _gen_x,_gen_y,_gen_z;		// state of the generator


/* initialize the random number generator */
inline void _initRand()
	{
	_gen_x = 123456789; 
	_gen_y = 362436069;
	_gen_z = 521288629;
	}

/* generate a uniform number in [0,1) */
inline double _rand_double0()
	{
	uint32 t;
	_gen_x ^= _gen_x << 16; _gen_x ^= _gen_x >> 5; _gen_x ^= _gen_x << 1;
	t = _gen_x; _gen_x = _gen_y; _gen_y = _gen_z; _gen_z = t ^ _gen_x ^ _gen_y;
	return(((double)_gen_z)/(4294967296.0));
	}


// ****************************************************************
// UTILITY FUNCTION : do not use any class member variable
// ****************************************************************


/* return the number of stochastic draw per pixel per turn */
inline uint32 _nbDrawPerTurn(const fRect & r,const iVec2 & sizeIm) const
	{
	return 3;
	}


/* return the pourcentage according to the line number of _qj */
inline int _getLinePourcent(int qj,int maxqj,int minv,int maxv) const
	{
	double v= ((double)qj)/((double)maxqj);
	int p = (int)(minv + v*(maxv-minv));
	return p;
	}


};

}

/* end of file */

