#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;


/**
 * The progress image class.
 * 
 * Simple class whcih encapsulate a RGBc64 image together with a uchar buffer that specifies the
 * normalisation for each pixel.
 **/
class ProgressImg
    {

    RGBc64 * _imData;
    uint8 * _normData;
    size_t _width;
    size_t _height;

    public:

        /**
        * Construct an empty image
        **/
        ProgressImg() : _width(0), _height(0), _imData(nullptr), _normData(nullptr) {}


        /**
         * Construct an image with the given size.
         **/
        ProgressImg(size_t LX, size_t LY) : _width(0), _height(0), _imData(nullptr), _normData(nullptr) { resize(LX, LY); }


        /** Destructor. */
        ~ProgressImg()
            {
            delete [] _normData;
            delete[] _imData;
            }


        /**
         * Clears the whole image to a given color. Faster if all channel are equal. The normalization
         * is set to 1.
         **/
        void clear(RGBc64 color)
            {
            if (_imData == nullptr) return;
            const size_t l = (size_t)_width*(size_t)_height;
            memset(_normData, 1, l);
            if ((color.comp.R == color.comp.G) && (color.comp.R == color.comp.B) && (color.comp.R == color.comp.A))
                {
                memset(_imData, color.comp.R, sizeof(RGBc64)*l);
                }
            else
                {
                for (size_t z = 0; z < l; z++) _imData[z] = color;
                }
            }

        /**
         * Raw image resizing. Keep the current buffer if the new size is smaller than the previous one.
         **/
        void resize(size_t newLX, size_t newLY)
            {
            if ((newLX <= 0) || (newLY <= 0))
                {
                delete [] _normData; _normData = nullptr;
                delete [] _imData; _imData = nullptr;
                _width = 0; _height = 0;
                return;
                }
            if ((newLX*newLY) > (_width*_height))
                {
                size_t l = newLX*newLY;
                delete [] _normData; _normData = new uint8[l];
                delete [] _imData; _imData = new RGBc64[l];
                }
            _width = newLX;
            _height = newLY;
            }


        /**
        * Return the current normalised pixel color.
        *
        * @param   x   The x coordinate.
        * @param   y   The y coordinate.
        *
        * @return  The normalized pixel color.
        **/
        inline RGBc64 operator()(size_t x, size_t y) const
            {
            const size_t off = x + _width*y;
            return _imData[off].getRGBc(_normData[off]);
            }


        /**
        * Return the height of the image
        **/
        inline size_t height() const { return _height; }


        /**
        * Return the width of the image.
        **/
        inline size_t width()  const { return _width; }


        /**
         * Return a pointer to the color buffer.
         **/
        inline RGBc64 * imData() { return _imData; }


        /**
        * Return a pointer to the normalization buffer.
        **/
        inline uint8 * normData() { return _normData; }


        /**
        * Normalises a portion of the image so that the multiplying factor for each pixel of the sub
        * image 1.
        *
        * No bound checking and no checking that the normalising factor is not zero. 
        *
        * @param   subBox  The sub box describing the portion to normalize.
        **/
        void normalize(iBox2 subBox)
            {
            size_t off = subBox.min[0] + _width*subBox.min[1];
            const int64 lx = subBox.lx();
            const int64 ly = subBox.ly();
            const size_t pa = (size_t)(_width - (subBox.lx()+1));
            for (int64 y = 0; y <= ly; y++)
                {
                for (int64 x = 0; x <= ly; x++)
                    {
                    _imData[off].normalize(_normData[off]);
                    _normData[off] = 1;
                    off++;
                    }
                off += pa;
                }
            }


        /** Normalises the whole image. */
        void normalize()
            {
            iBox2 B(0, _width - 1, 0, _height - 1);
            normalize(B);
            }


        // blit fast, image must have the exact same size
        void blit(Img<unsigned char> & im)
            {
            size_t N = (size_t)_width*_height;
            unsigned char * p1 = im.data();
            unsigned char * p2 = im.data() + N;
            unsigned char * p3 = im.data() + 2*N;
            for (size_t z = 0; z < N; z++)
                {
                const unsigned char o = _normData[z];
                RGBc coul = _imData[z].getRGBc((o == 0) ? 1 : o);
                p1[z] = coul.comp.R;
                p2[z] = coul.comp.G;
                p3[z] = coul.comp.B;
                }
            }

    };



template<typename ObjType> class ThreadPixelDrawer
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
        ThreadPixelDrawer(ObjType * obj, void * opaque = nullptr) :

            _obj(obj),
            _opaque(opaque),
            _active(false),
            _quality(0),
            _continuousDrawing(false),
            _keepPrevious(false),

            _validParam(false),
            _range(),
            _temp_range(),
            _im(nullptr),
            _temp_im(nullptr),
            _subBox(),
            _temp_subBox(),

            _dens(0.0),
            _dlx(0.0), _dly(0.0),
            _is1to1(false),
            _range1to1(),

            _workPhase(WORK_NOTHING),
            _beginPhase(true),

            _msg(THREAD_READY)

            {
            static_assert(mtools::GetColorSelector<ObjType>::has_getColor, "No compatible getColor / getImage / operator() method found...");
            _th = new std::thread(&ThreadPixelDrawer::_threadProc, this);
            }


        /**
        * Destructor.
        *
        *  Destructor. Stop the thread.
        **/
        ~ThreadPixelDrawer()
            {
            sync();
            _signal(SIGNAL_QUIT);
            _th->join();
            delete _th;
            }


        /**
        * Query the status of the drawer.
        *
        * @return  true if active and false if suspended.
        **/
        bool active() const { return (bool)_active; }


        /**
        * Set the thread status. When the thread is deactivated, it is guaranted that it does not
        * called the underlying object getColor function.
        *
        * Returns immediately, use sync() to wait for the operation to complete.
        *
        * @param   newstatus   true to activate and false to deactivate.
        **/
        void active(bool newstatus)
            {
            sync();
            _signal(newstatus ? SIGNAL_RESUME : SIGNAL_SUSPEND);
            }


        /**
        * Determines if the drawing parameter are valid. Return false if for example, the image is set
        * to nullptr, or the subbox is incorrect, or if the range is too small, or too large, or empty.
        *
        * If it return false, nothing will be drawn and the quality will stay 0.
        *
        * @return  true if the paramter are valid and flase otherwise.
        **/
        bool validParam()  const { return (bool)_validParam; }

        /**
        * Determines if we are performing continuous drawing (ie, if after having finished a perfect
        * drawing, the thread keep updating the image).
        *
        * @return  true if the thread keeps drawing false otherwise.
        **/
        bool continuousDrawing() const { return (bool)_continuousDrawing; }


        /**
        * Set whether we should continue drawing once perfect quality has been reached.
        *
        * Returns immediately, use sync() to wait for the operation to complete.
        *
        * @param   true to keep drawing and false otherwise.
        **/
        void continuousDrawing(bool keepDrawing)
            {
            sync();
            _signal(keepDrawing ? SIGNAL_CONTINUOUSDRAWING_ON : SIGNAL_CONTINUOUSDRAWING_OFF);
            }


        /**
        * Return the quality of the current drawing.
        *
        * @return  between 0 and 100. With the followng meaning:
        *          0       = nothing can be show. Not all pixels have been processed.
        *          1 - 25  = stochastic drawing is in progress.
        *          26 - 99 = perfect drawing in progress.
        *          100     = perfect drawing completed.
        **/
        int quality() const { return (int)_quality; }


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
            _signal(SIGNAL_NEWPARAM);
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
            _signal(SIGNAL_REDRAW);
            }


        /** Wait for the current request to complete before returning */
        void sync()
            {
            if (((int)_msg) == THREAD_READY) return;
            std::unique_lock<std::mutex> lock(_mut_wait);
            while (((int)_msg) != THREAD_READY) { _cv_wait.wait(lock); }
            }





    private:



        //static const double USE_X_SHORTCUT = 0.3;
        //static const double USE_Y_SHORTCUT = 0.5;

        static const int WORK_NOTHING = 100;
        static const int WORK_1TO1 = 101;
        static const int WORK_FAST = 102;
        static const int WORK_STOCHASTIC = 103;
        static const int WORK_PERFECT = 104;
        static const int WORK_FINISHED = 105;

        static const int THREAD_READY = 0;
        static const int SIGNAL_QUIT = 1;
        static const int SIGNAL_SUSPEND = 2;
        static const int SIGNAL_RESUME = 3;
        static const int SIGNAL_NEWPARAM = 4;
        static const int SIGNAL_REDRAW = 5;
        static const int SIGNAL_CONTINUOUSDRAWING_ON = 6;
        static const int SIGNAL_CONTINUOUSDRAWING_OFF = 7;


        ObjType * _obj;                         // the object to draw.
        void * _opaque;                         // opaque data passed to _obj;
        std::atomic<bool> _active;              // the thread status
        std::atomic<int>  _quality;             // current quality
        std::atomic<bool> _continuousDrawing;   // true = continuous drawing enabled, false = continuous drawing disabled.
        std::atomic<bool> _keepPrevious;        // do we keep something from the previous drawing
        std::atomic<bool> _validParam;          // indicate if the parameter are valid.

        fBox2 _range;                           // the range
        fBox2 _temp_range;         // and the temporary use to communicate with the thread.

        ProgressImg* _im;                       // the image to draw onto
        std::atomic<ProgressImg*> _temp_im;     // and the temporary use to communicate with the thread.

        iBox2 _subBox;                          // part of the image to draw
        iBox2 _temp_subBox;        // and the temporary use to communicate with the thread.

        double _dens;                           // density : average number of sites per pixel
        double _dlx, _dly;                      // horizontal and vertical density (size of image pixel in real coord)
        bool _is1to1;                           // true if we have a 1 to 1 drawing
        iBox2 _range1to1;                       // integer range box in the case of 1 to 1 drawing

        int _workPhase;                         // current work phase
        bool _beginPhase;                       // true if we start a new phase

        std::atomic<int> _msg;                  // used for communication
        std::condition_variable _cv_wakeup;     // used for waking up the thread
        std::mutex _mut_wakeup;                 // associated condition variable.
        std::condition_variable _cv_wait;       // used for waiting for the thread to answer
        std::mutex _mut_wait;                   // associated condition variable.
        std::thread * _th;                      // the thread object.

        FastRNG _fastgen;                       // fast RNG



        /* send a signal to the thread */
        void _signal(const int msg)
            {
            MTOOLS_ASSERT(_msg == THREAD_READY);
            std::unique_lock<std::mutex> lock(_mut_wakeup);
            _msg = msg;
            _cv_wakeup.notify_one();
            }


        /* put the thread to sleep, return when there is a message */
        void _sleep()
            {
            if (((int)_msg) != THREAD_READY) return;
            std::unique_lock<std::mutex> lock(_mut_wakeup);
            while (((int)_msg) == THREAD_READY) { _cv_wakeup.wait(lock); }
            }


        /* indicate that the thread is ready */
        void _threadReady()
            {
            MTOOLS_ASSERT(_msg != THREAD_READY);
            std::unique_lock<std::mutex> lock(_mut_wait);
            _msg = THREAD_READY;
            _cv_wait.notify_one();
            }


        /* the thread procedure */
        void _threadProc()
            {
            while (1)
                {
                _sleep();  // sleep until we have a msg to process
                switch ((int)_msg)
                    {
                    case SIGNAL_QUIT: { _threadReady(); return; }
                    case SIGNAL_SUSPEND: { _active = false; break; }
                    case SIGNAL_RESUME: { _active = true; break; }
                    case SIGNAL_NEWPARAM: { _setNewParam(); break; }
                    case SIGNAL_REDRAW: { _redraw(); break;}
                    case SIGNAL_CONTINUOUSDRAWING_ON: { _continuousDrawing = true; }
                    case SIGNAL_CONTINUOUSDRAWING_OFF: { _continuousDrawing = false; }
                    default: { MTOOLS_ERROR("wtf?"); }
                    }
                _threadReady(); // indicate that the message was processed.
                if (((bool)_active) && ((bool)_validParam)) _work(); // work until finished or we have a message to process
                }
            }


        /* set the new parameter */
        void _setNewParam()
            {
            const int MIN_IMAGE_SIZE = 5;
            const double RANGE_MIN_VALUE = 1.0e-17;
            const double RANGE_MAX_VALUE = 1.0e17;
            _range = _temp_range;
            _im = _temp_im;
            _subBox = _temp_subBox;
            if ((_im == nullptr) || (_im->width() < MIN_IMAGE_SIZE) || (_im->height() < MIN_IMAGE_SIZE)) { _quality = 0; _validParam = false; return; }   // make sure im is not nullptr and is big enough.
            if (_subBox.isEmpty()) { _subBox = iBox2(0, _im->width() - 1, 0, _im->height() - 1); } // subbox = whole image if empty. 
            if ((_subBox.min[0] < 0) || (_subBox.max[0] >= (int64)_im->width()) || (_subBox.min[1] < 0) || (_subBox.max[1] >= (int64)_im->height())) { _quality = 0; _validParam = false; return; } // make sure _subBox is a proper subbox of im
            if ((_subBox.lx() < MIN_IMAGE_SIZE) || (_subBox.ly() >= MIN_IMAGE_SIZE)) { _quality = 0; _validParam = false; return; } // make sure _subBox is a proper subbox of im
            const double rlx = _range.lx();
            const double rly = _range.ly();
            if ((rlx < RANGE_MIN_VALUE) || (rly < RANGE_MIN_VALUE)) { _quality = 0; _validParam = false; return; } // prevent zooming in too far
            if ((std::abs(_range.min[0]) > RANGE_MAX_VALUE) || (std::abs(_range.max[0]) > RANGE_MAX_VALUE) || (std::abs(_range.min[1]) > RANGE_MAX_VALUE) || (std::abs(_range.max[1]) > RANGE_MAX_VALUE)) { _quality = 0; _validParam = false; return; } // prevent zooming out too far
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
                _range1to1.min[0] = (int64)std::ceil(_range.min[0]); _range1to1.max[0] = (int64)std::floor(_range.max[0]);
                _range1to1.min[1] = (int64)std::ceil(_range.min[1]); _range1to1.max[1] = (int64)std::floor(_range.max[1]);
                }
            _quality = 0;
            _workPhase = WORK_NOTHING;
            _assignWork();
            }


        /* trigger a redraw */
        void _redraw()
            {
            _workPhase = WORK_NOTHING;
            _assignWork();
            if (((int)_quality > 0) && ((bool)_keepPrevious))
                {
                _quality = 1;
                if (_workPhase == WORK_FAST) { _im->normalize(_subBox); _assignWork();
                    }
                return;
                }
            else
                {
                _quality = 0;
                }
            }


        /* set the next type of work to perform */
        void _assignWork()
            {
            static const double DENSITY_SKIP_FAST = 1.0;
            static const double DENSITY_SKIP_STOCHASTIC = 4.0;
            _beginPhase = true;
            while (1)
                {
                switch (_workPhase)
                    {
                    case WORK_NOTHING:
                        {
                        if (_is1to1) { _workPhase = WORK_1TO1; return; }
                        _workPhase = ((_dens < DENSITY_SKIP_FAST) ? WORK_PERFECT : WORK_FAST);
                        return;
                        }
                    case WORK_1TO1:
                        {
                        _workPhase = WORK_FINISHED;
                        break;
                        }
                    case WORK_FAST:
                        {
                        _workPhase = ((_dens < DENSITY_SKIP_STOCHASTIC) ? WORK_PERFECT : WORK_STOCHASTIC);
                        return;
                        }
                    case WORK_STOCHASTIC:
                        {
                        _workPhase = WORK_PERFECT;
                        return;
                        }
                    case WORK_PERFECT:
                        {
                        _workPhase = WORK_FINISHED;
                        break;
                        }
                    case WORK_FINISHED:
                        {
                        if (_continuousDrawing) _workPhase = WORK_PERFECT;
                        return;
                        }
                    default:
                        {
                        MTOOLS_ERROR("wtf...");
                        }
                    }
                }
            }


        /* work on the image */
        void _work()
            {
            while (_msg == THREAD_READY)
                {
                switch (_workPhase)
                    {
                    case WORK_NOTHING: { MTOOLS_ERROR("Should not be possible..."); break; }
                    case WORK_1TO1: { _draw_1to1(); break; }
                    case WORK_FAST: { _draw_fast(); break; }
                    case WORK_STOCHASTIC: { _draw_stochastic(); break; }
                    case WORK_PERFECT: { _draw_perfect(); break; }
                    case WORK_FINISHED: { _assignWork(); if (_workPhase == WORK_FINISHED) { return; } break; }
                    default: { MTOOLS_ERROR("wtf?"); }
                    }
                }
            }




        // used for saving current state of drawing */
        int64    _saved_i;                    // current i position to draw
        int64    _saved_j;                    // current j position to draw
        int64    _saved_strip;                // previous strip line value
        RGBc     _saved_prev_coul;            // previous color;
        int64    _saved_stoc_mult;            // current stochastic multiplier
        int64    _saved_stoc_total;           // current stochastic total



        /* fastest perfect drawing : one to correspondance !
         * may be interrupted at each line */
        void _draw_1to1()
            {
            const int64 xmin = _range1to1.min[0];
            const int64 xmax = _range1to1.max[0];
            const int64 ymin = _range1to1.min[1];
            const int64 ymax = _range1to1.max[1];
            RGBc64 * imData = _im->imData();
            unsigned char * normData = _im->normData();
            if (_beginPhase)
                {
                _beginPhase = false;
                _saved_j = ymin;
                }
            size_t off = _subBox.min[0] + _im->width()*(_subBox.min[1] + (_saved_j - ymin)); // offset of the first point in the image
            size_t pa = (size_t)(_im->width() - (_subBox.lx() + 1)); // padding needed to get to the next line
            for (int64 j = _saved_j; j <= ymax; j++)
                {
                if (_msg != THREAD_READY) { _saved_j = j; return; }
                for (int64 i = xmin; i <= xmax; i++)
                    {
                    imData[off] = mtools::GetColorSelector<ObjType>::call(*_obj, { i , j }, _opaque);
                    normData[off] = 1;
                    off++;
                    }
                off += pa;
                }
            _quality = 100;
            _assignWork();
            }



        /* fast drawing, site color per pixel,
        * may be interrupted at each line */
        void _draw_fast()
            {
            int64  i = 0;
            int64  j = 0;
            int64  strip = _noLineStrip();
            RGBc prev_coul = RGBc::c_Black;
            int64  stoc_mult = 0;
            int64  stoc_total = 0;
            if (!_beginPhase)
                {
                i = _saved_i;
                j = _saved_j;
                strip = _saved_strip;
                prev_coul = _saved_prev_coul;
                stoc_mult = _saved_stoc_mult;
                stoc_total = _saved_stoc_total;
                _beginPhase = false;
                }
            /* remeber x value to prevent multiple horizontal queries */
            /* try to skip similar vertical lines */
            _assignWork();
            }


        /* stochastic drawing sample a pixel
        * may be interrupted at each line if create by bunch <= 255
        * and at each pixel otherwise. */
        void _draw_stochastic()
            {
            if (_beginPhase)
                {
                _beginPhase = false;
                }
            // two case : add only a single sample or add multiple samples
            /* remeber x value to prevent multiple horizontal queries */
            /* try to skip similar vertical lines */
            

            _assignWork();
            }


        /* perfect drawing by computing the average color at each pixel
        * may be interrupted at each line if density <= 255 and at each
        * pixel if density > 255*/
        void _draw_perfect()
            {
            if (_beginPhase)
                {
                _beginPhase = false;
                }
            /* remeber x value to prevent multiple horizontal queries */
            /* try to skip similar vertical lines */
            _assignWork();
            }




        /* return the y value of the integer strip containing the pixel at coord j
        if indeed contained in such a strip, otherwise, return distinct values < _range.min[1] */
        int64 _findLineStrip(int j)
            {
            const double m = _range.min[1];
            const double a = m + _dly*j;
            const double z = std::floor(a + 0.5);
            return((a + _dly <= z + 0.5) ? ((int64)z) : ((int64)m - j - 8));
            }


        /* return a value that will not match any returned by _findLineStrip */
        int64 _noLineStrip()
            {
            return ((int64)_range.min[1] - 7);
            }



    };






    
    MT2004_64 gen;
    double sinus(double x) { return sin(x); }

    double square(double x) { return (x*x); }

    void stupidTest()
        {
        cout << "Hello World\n";
        int64 v = cout.ask("Give me a number", 0);
        watch("your number", v);
        cout << "Terminate the program gracefully by closing the plotter window\n";
        cout << "or forcefully by closing the cout or watch window\n";
        Plotter2D PL;
        auto P1 = makePlot2DFun(sinus, -2, 2, "sinus");
        auto P2 = makePlot2DFun(square, -2, 2, "square");
        PL[P1][P2];
        PL.autorangeXY();
        PL.plot();

        }






    mtools::Img<unsigned char> lenna;
    int64 lenx, leny;

    RGBc colorImage(int64 x, int64 y)
        {
        if ((x < 0) || (x >= lenx)) return RGBc::c_Cyan;
        if ((y < 0) || (y >= leny)) return RGBc::c_Green;
        return lenna.getPixel({ x, leny - 1 - y });
        }

    void loadImage()
        {
        lenna.load("lenna.jpg");
        lenx = lenna.width();
        leny = lenna.height();
        cout << "LX = " << lenx << "\n";
        cout << "LY = " << leny << "\n";
        }




    void test()
        {
        const int LLX = 800;
        const int LLY = 600;

        ProgressImg progIm(LLX, LLY);
        progIm.clear((RGBc64)RGBc::c_Cyan);
        
        mtools::Img<unsigned char> dispIm(LLX, LLY, 1, 3);
        cimg_library::CImg<unsigned char> * cim = (cimg_library::CImg<unsigned char> *)&dispIm;
        progIm.blit(dispIm);

        fBox2 r(-0.5, 799.5, 0.5, 599.5);	// the range displayed

        ThreadPixelDrawer<decltype(colorImage)> TPD(colorImage, nullptr);

        TPD.setParameters(r, &progIm, iBox2());

        int drawtype = 0, isaxe = 1, isgrid = 0, iscell = 1; // flags
        cimg_library::CImgDisplay DD(*cim); // display
        while ((!DD.is_closed())) 
            {
            uint32 k = DD.key();
            if ((DD.is_key(cimg_library::cimg::keyA))) { isaxe = 1 - isaxe; std::this_thread::sleep_for(std::chrono::milliseconds(50)); }   // type A for toggle axe (with graduations)
            if ((DD.is_key(cimg_library::cimg::keyG))) { isgrid = 1 - isgrid; std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // type G for toggle grid
            if ((DD.is_key(cimg_library::cimg::keyC))) { iscell = 1 - iscell; std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // type C for toggle cell
            if (DD.is_key(cimg_library::cimg::keyESC)) { TPD.redraw(); } // [ESC] to force complete redraw
            if (DD.is_key(cimg_library::cimg::keyARROWUP)) { double sh = r.ly() / 20; r.min[1] += sh; r.max[1] += sh; TPD.setParameters(r, &progIm, iBox2()); } // move n the four directions
            if (DD.is_key(cimg_library::cimg::keyARROWDOWN)) { double sh = r.ly() / 20; r.min[1] -= sh; r.max[1] -= sh; TPD.setParameters(r, &progIm, iBox2());} //
            if (DD.is_key(cimg_library::cimg::keyARROWLEFT)) { double sh = r.lx() / 20; r.min[0] -= sh; r.max[0] -= sh; TPD.setParameters(r, &progIm, iBox2()); } //
            if (DD.is_key(cimg_library::cimg::keyARROWRIGHT)) { double sh = r.lx() / 20; r.min[0] += sh; r.max[0] += sh; TPD.setParameters(r, &progIm, iBox2()); } //
            if (DD.is_key(cimg_library::cimg::keyPAGEDOWN)) { double lx = r.max[0] - r.min[0]; double ly = r.max[1] - r.min[1]; r.min[0] = r.min[0] - (lx / 8.0); r.max[0] = r.max[0] + (lx / 8.0); r.min[1] = r.min[1] - (ly / 8.0);  r.max[1] = r.max[1] + (ly / 8.0); TPD.setParameters(r, &progIm, iBox2()); }
            if (DD.is_key(cimg_library::cimg::keyPAGEUP)) { if ((r.lx()>0.5) && (r.ly()>0.5)) { double lx = r.max[0] - r.min[0]; double ly = r.max[1] - r.min[1]; r.min[0] = r.min[0] + (lx / 10.0); r.max[0] = r.max[0] - (lx / 10.0); r.min[1] = r.min[1] + (ly / 10.0); r.max[1] = r.max[1] - (ly / 10.0); } TPD.setParameters(r, &progIm, iBox2()); }
       
            TPD.sync();
            std::cout << "quality = " << TPD.quality() << "\n";
            progIm.blit(dispIm);
            if (isaxe) { dispIm.fBox2_drawAxes(r).fBox2_drawGraduations(r).fBox2_drawNumbers(r); }
            if (isgrid) { dispIm.fBox2_drawGrid(r); }
            if (iscell) { dispIm.fBox2_drawCells(r); }
            DD.display(*cim);
            }
        return;
        }




    int main(int argc, char * argv[])
        {

        test(); 

        cout << sizeof(RGBc);
        cout.getKey();

        loadImage();

        Plotter2D plotter;
        auto P = mtools::makePlot2DLattice(colorImage, "test");
        plotter[P];
        plotter.plot();
        return 0;
        }





    
/* end of file */