/***********************************************************************************************************************//**
* @file planedrawer.hpp
* @version 2.2
* @author Vindar
* @date 02 / 03 / 2015
* @copyright GNU Public License.
* @note header only
*
* Create an image of a portion of R^2
***************************************************************************************************************************/

#ifndef INC_planedrawer_HPP
#define INC_planedrawer_HPP


#include "drawable2Dobject.hpp"
#include "customcimg.hpp"
#include "rgbc.hpp"
#include "maths/rect.hpp"
#include "misc/misc.hpp"
#include "misc/metaprog.hpp"

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
    template<mtools::RGBc (*getColorFun)(mtools::fVec pos)> class PlaneObj
        {
        public:
       inline static RGBc getColor(mtools::fVec2 pos) { return getColorFun(pos); }
       inline static PlaneObj<getColorFun> * get() { return(nullptr); }
        };


/**
 * Draws part of a plane into into a CImg image. This class implement the Drawable2DObject interface.
 * 
 * - The parameters of the drawing are set using the `setImageType` , `setParam`, `ResetDrawing`
 * method. The method `work` is used to create the drawing itself. The actual warping of the
 * image into a given CImg image is performed using the `drawOnto` method which is quite fast.
 * 
 * - All the public methods of this class are thread-safe : they can be called simultaneously
 * from any thread and the call are lined up. In particular, the `work` method can be time
 * expensive and might be better called from a worker thread : see the `AutoDrawable2DObject`
 * class for a generic implementation.
 * 
 * - The template PlaneObj must implement a method `RGBc getColor(fVec2 pos)` which return the
 * color associated with a given point. The method should be made as fast as possible. The fourth
 * channel of the returned color will be used when drawing on 4 channel images and ignored when
 * drawing on 3 channel images.
 *        
 * @tparam  PlaneObj    Type of the lattice object. Can be any class provided that it defines the 
 *                      method `RGBc getColor(fVec2 pos)` method.
 **/
template<class PlaneObj> class PlaneDrawer : public mtools::internals_graphics::Drawable2DObject
{
  
public:

    /**
     * Constructor. Set the plane object that will be drawn. 
     *
     * @param [in,out]  obj The object to draw, it must survive the drawer.
     **/
    PlaneDrawer(LatticeObj * obj) : 
		_g_requestAbort(0), 
		_g_current_quality(0), 
		_g_obj(obj), 
		_g_imSize(201, 201), 
		_g_r(-100.5, 100.5, -100.5, 100.5), 
		_g_redraw(true)
		{
        static_assert(mtools::metaprog::has_getColor<LatticeObj, mtools::iVec2>::value, "The object T must be implement a 'RGBc getColor(iVec2 pos)' method.");
        _initInt16Buf();
		_initRand();
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
    * Set the parameters of the drawing. Calling this method interrupt any work() in progress. 
    * This method is fast, it does not draw anything.
    **/
    virtual void setParam(mtools::fRect range, mtools::iVec2 imageSize)
        {
        MTOOLS_ASSERT(!range.isEmpty());
        MTOOLS_ASSERT((imageSize.X() >0) && (imageSize.Y()>0));     // make sure the image not empty.
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
    virtual void resetDrawing()
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
     * Draw onto a given image. This method is called by the composer when it want the picture. This
     * method is fast and does not "compute" anything. It simply warp the current drawing onto a
     * given cimg image.
     * 
     * The provided cimg image may must have 3 or 4 channel and the same size as that set via
     * setParam().
     * 
     * - If im has 3 channels, then the drawer uses only 3 channels for the lattice and simply
     * superpose the image created over im (multiplying it by an optional opacity parameter).
     * 
     * - If im has 4 channels, the drawer uses also 4th channel for the lattice.
     * 
     * The method is faster when transparency is not used (i.e. when the supplied image im has 3
     * channel) and fastest when opacity is 1.0 (or 0.0, but this does nothing).
     *
     * @param [in,out]  im  The image to draw onto (must be a 3 or 4 channel image and it size must
     *                      be equal to the size previously set via the setParam() method.
     * @param   opacity     The opacity that should be applied to the picture prior to drawing onto
     *                      im. If set to 0.0, then the method returns without drawing anything. The
     *                      opacity is multiplied with other opacities when dealing with 4 channel
     *                      images.
     *
     * @return  The quality of the drawing performed (0 = nothing drawn, 100 = perfect drawing).
     **/
    virtual int drawOnto(cimg_library::CImg<unsigned char> & im, float opacity = 1.0)
        {
        MTOOLS_ASSERT((im.width() == _g_imSize.X()) && (im.height() == _g_imSize.Y()));
        MTOOLS_ASSERT((im.spectrum() == 3) || (im.spectrum() == 4));
        ++_g_requestAbort; // request immediate stop of the work method if active.
            {
            std::lock_guard<std::timed_mutex> lg(_g_lock); // and wait until we aquire the lock 
            --_g_requestAbort; // and then remove the stop request
            if (opacity > 0.0)
                {
                _warpImage(im, opacity); 
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
    virtual int quality() const {return _g_current_quality;}


    /**
     * Works on the drawing for a maximum specified period of time. If the drawing is already
     * completed, returns directly.
     * 
     * The function has the lowest priority of all the public methods and may be interrupted (hence
     * returning early) if another method such as drawOnto(),setParam()... is accessed by another
     * thread.
     * 
     * If another thread already launched some work, this method will wait until the lock is
     * released (but will return if the time allowed is exceeded).
     * 
     * A typical time span of 50/100ms is usually enough to get some drawing to show. If maxtime_ms
     * = 0, then the function return immediately (with the current quality of the drawing).
     *
     * @param   maxtime_ms  The maximum time in millisecond allowed for drawing.
     *
     * @return  The quality of the current drawing:  0 = nothing to show, 100 = perfect drawing.
     **/
    virtual int work(int maxtime_ms)
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
    virtual bool needWork() const { return true; }



    /**
     * Stop any ongoing work and then return
     **/
    virtual void stopWork()
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
    LatticeObj *      _g_obj;               // the object to draw
    iVec2             _g_imSize;            // size of the drawing
    fRect             _g_r;                 // current range
    std::atomic<bool> _g_redraw;            // true if we should redraw from scratch



// ****************************************************************
// THE PIXEL DRAWER
// ****************************************************************

fRect           _pr;                    // the current range
uint32 			_counter1,_counter2;	// counter for the number of pixel added in each cell: counter1 for cells < (_qi,_qj) and counter2 for cells >= (_qi,qj)
uint32 			_qi,_qj;		        // position where we stopped previously
int 			_phase;			        // the current phase of the drawing


/* update the quality of the picture  */
void _qualityPixelDraw() const
    {
    switch (_phase)
        {
        case 0: {_g_current_quality = 0; break; }
        case 1: {_g_current_quality = _getLinePourcent(_counter2, _nbPointToDraw(_pr, _int16_buffer_dim), 1, 25); break; }
        case 2: {_g_current_quality = _getLinePourcent(_qj, (int)_int16_buffer_dim.Y(), 26, 99); break; }
        case 3: {_g_current_quality = 100; break; }
        default: MTOOLS_INSURE(false); // wtf are we doing here
        }
    return;
    }

/* draw as much as possible of a fast drawing, return true if finished false otherwise
  if finished, then _qi,_qj are set to zero and counter1 = counter2 has the correct value */
void _drawPixel_fast(int maxtime_ms)
	{
    const fRect r = _pr;
    const double px = ((double)r.lx()) / ((double)_int16_buffer_dim.X())  // size of a pixel
               , py = ((double)r.ly()) / ((double)_int16_buffer_dim.Y()); 
	_counter1 = 1;
	bool fixstart = true; 
    RGBc coul;
    int64 prevsx = (int64)floor(r.xmin) - 2;
    int64 prevsy = (int64)floor(r.ymax) + 2;
    for (int j = 0; j < _int16_buffer_dim.Y(); j++)
    for (int i = 0; i < _int16_buffer_dim.X(); i++)
		{
		if (fixstart) {i = _qi; j = _qj; fixstart=false;}					        // fix the position of thestarting pixel 
		if (_isTime(maxtime_ms)) {_qi = i; _qj = j; return;}	                    // time's up : we quit
		double x = r.xmin + (i + 0.5)*px, y = r.ymax - (j + 0.5)*py;            	// pick the center point inside the pixel
		int64 sx = (int64)floor(x + 0.5); int64 sy = (int64)floor(y + 0.5); 		// compute the integer position which covers it
        if ((prevsx != sx) || (prevsy != sy)) 
            { // not the same point as before
            coul = _g_obj->getColor({ sx, sy });
            prevsx = sx; prevsy = sy;
            }
        _setInt16Buf(i, j, coul);						    // set the color in the buffer
		}
	// we are done
	_counter2 = _counter1; _qi=0; _qj=0;
    if (_skipStochastic(r, _int16_buffer_dim)) { _phase = 2; } else { _phase = 1; } // go to next phase, skip stochastic if not needed.
	return;
	}


/* draw as much as possible of a stochastic drawing, return true if finished, false otherwise
  if finished, then _qi,_qj are set to zero and counter1 = counter2 has the correct value */
void _drawPixel_stochastic(int maxtime_ms)
	{
    const fRect r = _pr;
    const double px = ((double)r.lx()) / ((double)_int16_buffer_dim.X())  // size of a pixel
               , py = ((double)r.ly()) / ((double)_int16_buffer_dim.Y());
    uint32 ndraw = _nbDrawPerTurn(r, _int16_buffer_dim);
    while(_counter2 < _nbPointToDraw(r, _int16_buffer_dim))
		{
		if (_counter2 == _counter1) {++_counter1;} // start of a loop: we increase counter1 
		bool fixstart = true; 
        for (int j = 0; j < _int16_buffer_dim.Y(); j++)
        for (int i = 0; i < _int16_buffer_dim.X(); i++)
			{
			if (fixstart) {i = _qi; j = _qj; fixstart=false;}		// fix the position of thestarting pixel
            if (_isTime(maxtime_ms)) { _qi = i; _qj = j; return; }	// time's up : we quit
			uint32 R=0,G=0,B=0,A=0;
 			for(uint32 k=0;k<ndraw;k++)
				{
				double x = r.xmin + (i + _rand_double0())*px, y = r.ymax - (j + _rand_double0())*py; 	// pick a point at random inside the pixel
				int64 sx = (int64)floor(x + 0.5); int64 sy = (int64)floor(y + 0.5);     			// compute the integer position which covers it
                RGBc coul = _g_obj->getColor({ sx, sy }); 			                     				// get the color of the site
                R += coul.R; G += coul.G; B += coul.B; A += coul.A;
				}
			_addInt16Buf(i,j,R/ndraw,G/ndraw,B/ndraw,A/ndraw);
			}
		// we finished a loop
		_counter2 = _counter1;	_qi=0; _qj=0;
		}
    _phase = 2; // go to next phase
	return;
	}


/* draw as much as possible of a perfect drawing, return true if finished, false otherwise
  if finished, then _qi,_qj are set to zero and counter1 = counter2 has the correct value */
void _drawPixel_perfect(int maxtime_ms)
	{
    const fRect r = _pr;
    const double px = ((double)r.lx()) / ((double)_int16_buffer_dim.X())  // size of a pixel
               , py = ((double)r.ly()) / ((double)_int16_buffer_dim.Y());
	_counter1 = 1; // counter1 must be 1
	bool fixstart = true; 
    RGBc coul;
    int64 pk = (int64)floor(r.xmin) - 2;
    int64 pl = (int64)floor(r.ymax) + 2;
    for (int j = 0; j < _int16_buffer_dim.Y(); j++)
    for (int i = 0; i < _int16_buffer_dim.X(); i++)
		{
		if (fixstart) {i = _qi; j = _qj; fixstart=false;}	// fix the position of thestarting pixel 
		fRect pixr(r.xmin + i*px,r.xmin + (i+1)*px,r.ymax - (j+1)*py,r.ymax - j*py); // the rectangle corresponding to pixel (i,j)
		iRect ipixr = pixr.integerEnclosingRect(); // the integer sites whose square intersect the pixel square
		double cr=0.0 ,cg=0.0, cb=0.0, ca=0.0, tot=0.0;
		for(int64 k=ipixr.xmin;k<=ipixr.xmax;k++) for(int64 l=ipixr.ymin;l<=ipixr.ymax;l++) // iterate over all those points
			{
            if (_isTime(maxtime_ms)) { _qi = i; _qj = j; return; } // time's up : we quit and abandon this pixel
			double a = pixr.pointArea((double)k,(double)l); // get the surface of the intersection
            if ((k != pk) || (l != pl))
                {
                coul = _g_obj->getColor({ k, l });
                pk = k; pl = l;
                }
            cr += (coul.R*a); cg += (coul.G*a); cb += (coul.B*a); ca += (coul.A*a); // get the color and add it proportionally to the intersection
			tot+=a;
			}
		_setInt16Buf(i,j,cr/tot,cg/tot,cb/tot,ca/tot);
		}
	_qi=0; _qj=0; _counter2 = _counter1;
    _phase = 3; // we are done, perfect drawing !
	return;
	}


/* the main method for drawing a pixel image */
void _workPixel(int maxtime_ms)
    {
    _startTimer();
    if (_g_imSize != _int16_buffer_dim) { _g_redraw_pix = true; } //set redraw to true if the size of the image changed
    if (_g_r != _pr) _g_redraw_pix = true; // set redraw to true if the range changed
    if (_g_redraw_pix)
        { // we must completly redraw, initialize everything
        _g_redraw_pix = false;
        _pr = _g_r;
        _qi = 0; _qj = 0;
        _counter1 = 0; _counter2 = 0;
        _resizeInt16Buf(_g_imSize);
        _phase = 0;
        }
    if (maxtime_ms > 0) 
        {
        while ((_phase != 3) && (!_isTime(maxtime_ms)))
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
                case 2: // perfect drawing phase : start from _qi,qj and make a stochastic drawing
                    {                    
                    _drawPixel_perfect(maxtime_ms);
                    break;
                    }
                default: MTOOLS_INSURE(false); // wtf are we doing here
                }
            }
        }
    _qualityPixelDraw(); // update the quality
    return;
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

/* set a color at position (i,j) */
inline void _setInt16Buf(uint32 x,uint32 y,double R,double G,double B,double A)
	{
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
    _int16_buffer[x + y*dx] = (uint16)round(R);
	_int16_buffer[x + y*dx + dxy] = (uint16)round(G);
	_int16_buffer[x + y*dx + 2*dxy] = (uint16)round(B);
    _int16_buffer[x + y*dx + 3*dxy] = (uint16)round(A);
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



/* the main method for warping the pixel image to the cimg image*/
void _warp(cimg_library::CImg<unsigned char> & im, float opacity)
    {
    _workPixel(0); // make sure everything is in sync. 
    if (_g_current_quality > 0)
        {
        if (im.spectrum() == 4)
            {
            _warpInt16Buf_4channel(im, opacity);
            }
        else
            {
            if (opacity >= 1.0) _warpInt16Buf_opaque(im); else _warpInt16Buf(im, opacity);
            }
        }
    return;
    }


/* warp the buffer onto an image using _qi,_qj,_counter1 and _counter2 : fast method when opacity = 1.0*/
inline void _warpInt16Buf(CImg<unsigned char> & im,const float op) const
	{
    const float po = 1 - op;
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
	size_t l1 = _qi + (dx*_qj);
	size_t l2 = (dxy) - l1;
	for(int c=0;c<im.spectrum();c++)
		{
		if (l1>0)
			{
			unsigned char * pdest = im.data(0,0,0,c);
			uint16 * psource = _int16_buffer + c*dxy;
			if (_counter1==0) {/* memset(pdest,0,l1); */}
			else 
				{     
                if (_counter1 == 1) { for (size_t i = 0; i < l1; i++) { (*pdest) = (unsigned char)((*pdest)*po + (*psource)*op); ++pdest; ++psource; } }
                else { for (size_t i = 0; i<l1; i++) { (*pdest) = (unsigned char)((*pdest)*po + (*psource)*op/_counter1); ++pdest; ++psource; } }
				}
			}
		if (l2>0)
			{
			unsigned char * pdest = im.data(_qi,_qj,0,c);
			uint16 * psource      = _int16_buffer + c*dxy + l1;
			if (_counter2==0) {/* memset(pdest,0,l2); */}
			else 
				{
                if (_counter2 == 1) { for (size_t i = 0; i<l2; i++) { (*pdest) = (unsigned char)((*pdest)*po + (*psource)*op); ++pdest; ++psource; } }
                else { for (size_t i = 0; i<l2; i++) { (*pdest) = (unsigned char)((*pdest)*po + (*psource)*op / _counter2);  ++pdest; ++psource; } }
				}
			}
		}
	return;
	}

/* warp the buffer onto an image using _qi,_qj,_counter1 and _counter2 : fast method when opacity = 1.0*/
inline void _warpInt16Buf_opaque(CImg<unsigned char> & im) const
{
    const size_t dx = (size_t)_int16_buffer_dim.X();
    const size_t dxy = (size_t)(dx * _int16_buffer_dim.Y());
    size_t l1 = _qi + (dx*_qj);
    size_t l2 = (dxy)-l1;
    for (int c = 0; c< im.spectrum(); c++)
    {
        if (l1>0)
        {
            unsigned char * pdest = im.data(0, 0, 0, c);
            uint16 * psource = _int16_buffer + c*dxy;
            if (_counter1 == 0) {/* memset(pdest,0,l1); */ } else
            {
                if (_counter1 == 1) { for (size_t i = 0; i<l1; i++) { (*pdest) = (unsigned char)(*psource); ++pdest; ++psource; } } else { for (size_t i = 0; i<l1; i++) { (*pdest) = (unsigned char)((*psource) / _counter1); ++pdest; ++psource; } }
            }
        }
        if (l2>0)
        {
            unsigned char * pdest = im.data(_qi, _qj, 0, c);
            uint16 * psource = _int16_buffer + c*dxy + l1;
            if (_counter2 == 0) {/* memset(pdest,0,l2); */ } else
            {
                if (_counter2 == 1) { for (size_t i = 0; i<l2; i++) { (*pdest) = (unsigned char)(*psource); ++pdest; ++psource; } } else { for (size_t i = 0; i<l2; i++) { (*pdest) = (unsigned char)((*psource) / _counter2);  ++pdest; ++psource; } }
            }
        }
    }
    return;
}


/* make B -> A */
inline unsigned char _blendcolor(unsigned char & A, float opA, unsigned char B, float opB ) const
    {
    float o = opB + opA*(1 - opB);
    if (o == 0)  return 0;
    A = (unsigned char)((B*opB + A*opA*(1 - opB))/o);
    return (unsigned char)(255 * o);
    }

/* warp the buffer onto an image using _qi,_qj,_counter1 and _counter2 : four channel on the image : use transparency 
 * PARTIAL SUPPORT */
inline void _warpInt16Buf_4channel(CImg<unsigned char> & im, float op) const
{
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
        uint16 * psource2 = _int16_buffer + 2*dxy;
        uint16 * psource_opb = _int16_buffer + 3 * dxy;
        if (_counter1 == 0) {/* memset(pdest,0,l1); */ } else
            {
            if (_counter1 == 1) 
                { 
                for (size_t i = 0; i < l1; i++) 
                    { 
                    _blendcolor((*pdest0), ((po*(*pdest_opa)) / 255), (unsigned char)(*psource0), ((op*(*psource_opb)) / 255));
                    _blendcolor((*pdest1), ((po*(*pdest_opa)) / 255), (unsigned char)(*psource1), ((op*(*psource_opb)) / 255));
                    (*pdest_opa) = _blendcolor((*pdest2), ((po*(*pdest_opa)) / 255), (unsigned char)(*psource2), ((op*(*psource_opb)) / 255));
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                    } 
                }
            else 
               { 
               for (size_t i = 0; i < l1; i++) 
                    { 
                    _blendcolor((*pdest0), ((po*(*pdest_opa)) / 255), (unsigned char)((*psource0) / _counter1), ((op*(*psource_opb) / _counter1) / 255));
                    _blendcolor((*pdest1), ((po*(*pdest_opa)) / 255), (unsigned char)((*psource1) / _counter1), ((op*(*psource_opb) / _counter1) / 255));
                    (*pdest_opa) = _blendcolor((*pdest2), ((po*(*pdest_opa)) / 255), (unsigned char)((*psource2) / _counter1), ((op*(*psource_opb) / _counter1) / 255));
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
        uint16 * psource0 = _int16_buffer +  + l1;
        uint16 * psource1 = _int16_buffer + dxy + l1;
        uint16 * psource2 = _int16_buffer + 2*dxy + l1;
        uint16 * psource_opb = _int16_buffer + 3*dxy + l1;
        if (_counter2 == 0) {/* memset(pdest,0,l2); */ } else
            {
            if (_counter2 == 1) 
                {
                for (size_t i = 0; i<l2; i++) 
                    { 
                    _blendcolor((*pdest0), (po*(*pdest_opa)) / 255, (unsigned char)(*psource0), (op*(*psource_opb)) / 255);
                    _blendcolor((*pdest1), (po*(*pdest_opa)) / 255, (unsigned char)(*psource1), (op*(*psource_opb)) / 255);
                    (*pdest_opa) = _blendcolor((*pdest2), (po*(*pdest_opa)) / 255, (unsigned char)(*psource2), (op*(*psource_opb)) / 255);
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
                    ++psource0; ++psource1; ++psource2; ++psource_opb;
                    }
                }
            else 
                { 
                for (size_t i = 0; i<l2; i++) 
                    { 
                    _blendcolor((*pdest0), (po*(*pdest_opa)) / 255, (*psource0) / _counter2, (op*(*psource_opb) / _counter2) / 255); 
                    _blendcolor((*pdest1), (po*(*pdest_opa)) / 255, (*psource1) / _counter2, (op*(*psource_opb) / _counter2) / 255);
                    (*pdest_opa) = _blendcolor((*pdest2), (po*(*pdest_opa)) / 255, (*psource2) / _counter2, (op*(*psource_opb) / _counter2) / 255);
                    ++pdest0; ++pdest1; ++pdest2; ++pdest_opa;
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
static const int _maxtic2 = 10;   	// number of tic until we look for time
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
    if (_g_drawingtype == TYPEPIXEL) { _qualityPixelDraw(); } else { _qualityImageDraw(); } // update the quality of the drawing
	if (((clock() - _stime)*1000)/CLOCKS_PER_SEC > (clock_t)ms) {_tic = _maxtic; return true;}
	_tic = 0;
	return false;
	}

/* check more often */
inline bool _isTime2(uint32 ms)
    {
    ++_tic;
    if (_g_requestAbort > 0) {return true; }
    if (_tic < _maxtic2) return false;
    if (_g_drawingtype == TYPEPIXEL) { _qualityPixelDraw(); } else { _qualityImageDraw(); } // update the quality of the drawing
    if (((clock() - _stime) * 1000) / CLOCKS_PER_SEC > (clock_t)ms) { _tic = _maxtic; return true; }
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


/* return the average number of site per pixel */
inline double _sitePerPixel(const fRect & r,const iVec2 & sizeIm) const {return (r.lx()/sizeIm.X())*(r.ly()/sizeIm.Y());}


/* return the number of stochastic draw per pixel per turn */
inline uint32 _nbDrawPerTurn(const fRect & r,const iVec2 & sizeIm) const
	{
	return 5;
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

#endif
/* end of file */

