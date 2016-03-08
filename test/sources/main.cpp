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
            size_t off = (size_t)(subBox.min[0] + _width*subBox.min[1]);
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


        // blit, image must have the exact same size
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



    /**
    * Class used for creating a simple worker thread.
    *
    * Two (pure) virtual method must be overwritten:
    *
    * - work() : performs the thread's work()
    * - message(int64 code) : process incomming messages and determine how the thread continu working.
    *
    */
    class ThreadWorker
        {

        public:

            /**
            * Constructor.
            * The thread is initially disabled and its work status set to false (i.e. waiting).
            **/
            ThreadWorker() :
                _progress(PROGRESS_NONE),
                _thread_status(false),
                _work_status(false),
                _msg(MSG_NONE),
                _code(0)
                {
                _th = new std::thread(&ThreadWorker::_threadProc, this);
                }


            /** Destructor. Stop the thread. */
            virtual ~ThreadWorker()
                {
                sync();
                _signal(MSG_QUIT);
                _th->join();
                delete _th;
                }


            /**
            * Return the current progress value.
            **/
            inline int progress() const { return _progress; }


            /**
            * Enables/Disable the thread. A disable thread can still process signals but cannot perform any
            * work.
            *
            * Returns immediately. Call sync() to wait for the completion of the command.
            *
            * @param   newstatus   true to enable and false to disable the thread.
            **/
            void enable(bool newstatus)
                {
                sync();
                if (newstatus == (bool)_thread_status) return;
                _signal((newstatus ? MSG_ENABLE : MSG_DISABLE));
                }


            /**
            * Query if the thread is currently enabled.
            **/
            inline bool enable() const { return (bool)_thread_status; }


            /** Query the work status: true for work on and false for wait on. */
            inline bool workStatus() const { return (bool)_work_status; }


            /**
            * True if there is no pending command and false if there is one pending.
            **/
            inline bool ready() const { return(((int)_msg) == MSG_NONE); }


            /** Wait for the completion of any pending command. */
            void sync()
                {
                if (((int)_msg) == MSG_NONE) return;
                std::unique_lock<std::mutex> lock(_mut_wait);
                while (((int)_msg) != MSG_NONE) { _cv_wait.wait(lock); }
                }


            /**
            * Send a signal to the thread.
            *
            * Wait for previous command to finish before sending the new one then return immediately after
            * sending the new command. Called sync()  to wait for the completion of the new command.
            *
            * @param   code    The code.
            **/
            void signal(int64 code)
                {
                sync();
                _signal(MSG_CODE, code);
                }


        protected:

            static const int THREAD_CONTINUE = 9;
            static const int THREAD_RESET = 10;
            static const int THREAD_WAIT = 11;
            static const int THREAD_RESET_AND_WAIT = 12;


            /**
            * Sets the value of the current work progress.
            **/
            inline void setProgress(int val) { _progress = val; }


            /**
            * Checks if there are any pending signal and process them if needed.
            *
            * !!! This method must be called regularly inside work() !!!
            **/
            inline void check() { if (((int)_msg) == MSG_NONE) { return; } _processInside(); }


            /**
            * Pure virtual method that performs the thread's work.
            *
            * The method must call check() regularly in order to process messages. The check() method will
            * throw an exception if work must stop. Do not catch it !
            **/
            virtual void work() = 0;


            /**
            * Pure virtual method called when there is a message to process.
            *
            * @param   code    The message code
            *
            * @return  Determine how work should continue from now on. One of
            *          THREAD_CONTINUE : continue work as before.
            *          THREAD_RESET : restart the work() method
            *          THREAD_WAIT : wait for an other message before continuing to work.
            *          THREAD_RESET_AND_WAIT: exit the work() method and wait for another message.
            **/
            virtual int message(int64 code) = 0;



        private:

            std::atomic<int>    _progress;          // progress indicator
            std::atomic<bool>   _thread_status;     // true id the thread is enabled and false otherwise
            std::atomic<bool>   _work_status;       // true if work on and false if waiting

            std::atomic<int>    _msg;               // message type, one of MSG_NONE, MSG_CODE, MSG_QUIT, MSG_ENABLE, MSG_DISABLE
            std::atomic<int64>  _code;              // code used for communication

            std::condition_variable _cv_wakeup;     // used for waking up the thread
            std::mutex _mut_wakeup;                 // associated condition variable.
            std::condition_variable _cv_wait;       // used for waiting for the thread to answer
            std::mutex _mut_wait;                   // associated condition variable.
            std::thread * _th;                      // the thread object.


            static const int PROGRESS_NONE = 0;

            static const int MSG_NONE = 4;
            static const int MSG_CODE = 5;
            static const int MSG_ENABLE = 6;
            static const int MSG_DISABLE = 7;
            static const int MSG_QUIT = 8;

            static const int64 CODE_NONE = 0;


            /* send a signal to the thread */
            void _signal(int msg, int64 code = CODE_NONE)
                {
                MTOOLS_ASSERT(((int)_msg) == MSG_NONE);
                std::unique_lock<std::mutex> lock(_mut_wakeup);
                _code = code;
                _msg = msg;
                _cv_wakeup.notify_one();
                }


            /* put the thread to sleep, return when there is a message */
            void _threadSleep()
                {
                if (((int)_msg) != MSG_NONE) return;
                std::unique_lock<std::mutex> lock(_mut_wakeup);
                while (((int)_msg) == MSG_NONE) { _cv_wakeup.wait(lock); }
                }


            /* indicate that the thread is ready */
            void _threadReady()
                {
                MTOOLS_ASSERT(((int)_msg) != MSG_NONE);
                std::unique_lock<std::mutex> lock(_mut_wait);
                _msg = MSG_NONE;
                _code = CODE_NONE;
                _cv_wait.notify_one();
                }


            /* the thread procedure */
            void _threadProc()
                {

            wait_label:
                _threadSleep();
                switch ((int)_msg)
                    {
                    case MSG_ENABLE: { _thread_status = true; break; }
                    case MSG_DISABLE: { _thread_status = false; break; }
                    case MSG_QUIT: { _threadReady(); return; }
                    case MSG_CODE:
                        {
                        int r = message(_code);
                        switch (r)
                            {
                            case THREAD_CONTINUE: { break; }
                            case THREAD_RESET: { _work_status = true; break; }
                            case THREAD_WAIT: { _work_status = false; break; }
                            case THREAD_RESET_AND_WAIT: { _work_status = false; break; }
                            default: { MTOOLS_ERROR("wtf?"); }
                            }
                        break;
                        }
                    default: { MTOOLS_ERROR("wtf?"); }
                    }
                _threadReady();

            work_label:
                if ((!((bool)_thread_status)) || (!((bool)_work_status))) goto wait_label;
                try
                    {
                    work();
                    _work_status = false;
                    goto wait_label;
                    }
                catch (int r)
                    {
                    if ((int)_msg == MSG_QUIT) { _threadReady(); return; }
                    switch (r)
                        {
                        case THREAD_RESET: { _work_status = true; _threadReady(); goto work_label; }
                        case THREAD_RESET_AND_WAIT: { _work_status = false; _threadReady(); goto wait_label; }
                        default: { MTOOLS_ERROR("wtf?"); }
                        }
                    }
                }


            /* process a message inside check() */
            void _processInside()
                {
                while (1)
                    {
                    switch ((int)_msg)
                        {
                        case MSG_ENABLE:
                            {
                            _thread_status = true;
                            if ((bool)_work_status) { _threadReady(); return; }
                            break;
                            }
                        case MSG_DISABLE: { _thread_status = false; break; }
                        case MSG_QUIT: { throw((int)0); }
                        case MSG_CODE:
                            {
                            int r = message(_code);
                            switch (r)
                                {
                                case THREAD_CONTINUE:
                                    {
                                    _work_status = true;
                                    if ((bool)_thread_status) { _threadReady(); return; }
                                    break;
                                    }
                                case THREAD_WAIT: { _work_status = false; break; }
                                case THREAD_RESET: { throw((int)r); }
                                case THREAD_RESET_AND_WAIT: { throw((int)r); }
                                default: { MTOOLS_ERROR("wtf?"); }
                                }
                            break;
                            }
                        default: { MTOOLS_ERROR("wtf?"); }
                        }
                    _threadReady();
                    _threadSleep();
                    }
                }

        };






























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
                    _range1to1(iBox2()),
                    _workPhase(WORK_NOTHING)
                    {
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
                    std::cout << "work\n";                    
                    while (1)
                        {
                        switch (_workPhase)
                            {
                            case WORK_1TO1:         { _draw_1to1(); break; }
                            case WORK_FAST:         { _draw_fast(); break; }
                            case WORK_STOCHASTIC:   { _draw_stochastic(); break; }
                            case WORK_PERFECT:      { _draw_perfect(); break; }
                            case WORK_FINISHED:     { return; }
                            case WORK_NOTHING:      { MTOOLS_ERROR("hum..."); }
                            default: { MTOOLS_ERROR("wtf?"); }
                            }
                        _assignWork();
                        }
                    }


                /**
                * Handles the thread messages
                **/
                virtual int message(int64 code) override
                    {
                    switch (code)
                        {
                        case SIGNAL_NEWPARAM: { return _setNewParam(); }
                        case SIGNAL_REDRAW:   { return _setRedraw(); }
                        default: { MTOOLS_ERROR("wtf!"); }
                        }
                    }


       
                /* set the new parameter */
                int _setNewParam()
                    {
                    const int MIN_IMAGE_SIZE = 5;
                    const double RANGE_MIN_VALUE = 1.0e-17;
                    const double RANGE_MAX_VALUE = 1.0e17;
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
                    else
                        {
                        _is1to1 = false;
                        }
                    setProgress(0);
                    _workPhase = WORK_NOTHING;
                    _assignWork();
                    return THREAD_RESET;
                    }



                /* trigger a redraw */
                int _setRedraw()
                    {
                    if (!((bool)_validParam)) return THREAD_RESET_AND_WAIT;
                    _workPhase = WORK_NOTHING;
                    _assignWork();
                    if ((progress() > 0) && ((bool)_keepPrevious))
                        {
                        setProgress(1);
                        if (_workPhase == WORK_FAST) 
                            {
                            _im->normalize(_subBox); _assignWork();
                            }
                        return THREAD_RESET;
                        }
                    else
                        {
                        setProgress(0);
                        }
                    return THREAD_RESET;
                    }



                /* set the next type of work to perform */
                void _assignWork()
                    {
                    std::cout << "assign work\n";
                    static const double DENSITY_SKIP_FAST = 1.0;
                    static const double DENSITY_SKIP_STOCHASTIC = 4.0;
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
                                return;
                                }
                            default:
                                {
                                MTOOLS_ERROR("wtf...");
                                }
                            }
                        }
                    }



                /* perfect 1 to 1 drawing */
                void _draw_1to1()
                    {
                    std::cout << "draw 1 on 1 \n"; mtools::Chronometer();
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
                                normData[off] = 1;
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
                                normData[off] = 1;
                                off++;
                                }
                            }
                        }
                    setProgress(100);
                    uint64 tim = mtools::Chronometer(); std::cout << "finished in " << tim << " ms\n";
                    }



                /* fast drawing */
                void _draw_fast()
                    {
                    std::cout << "draw fast \n";  mtools::Chronometer();
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
                                std::memset((normData + off), 1, (size_t)ilx);
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
                                    normData[off] = 1;
                                    off++;
                                    }
                                off += pa;
                                }
                            }
                        }
                    else
                        { // high density, do not try to re-use color
                        for(int64 j = 0; j < ily; j++)
                            {
                            check();
                            const double y = r.min[1] + (j + 0.5)*py;
                            const int64 sy = (int64)floor(y + 0.5);
                            for (int64 i = 0; i < ilx; i++)
                                {
                                const double x = r.min[0] + (i + 0.5)*px;
                                const int64 sx = (int64)floor(x + 0.5);
                                imData[off] = mtools::GetColorSelector<ObjType>::call(*_obj, { sx , sy }, _opaque);
                                normData[off] = 1;
                                off++;
                                }
                            off += pa;
                            }
                        }
                    setProgress(1);
                    uint64 tim = mtools::Chronometer(); std::cout << "finished in " << tim << " ms\n";
                    }


                /* stochastic drawing sample a pixel
                * may be interrupted at each line */
                void _draw_stochastic()
                    {
                    std::cout << "draw stochastic \n";
                    return _draw_fast();
                    // two case : add only a single sample or add multiple samples
                    /* remeber x value to prevent multiple horizontal queries */
                    /* try to skip similar vertical lines */

                    }


                /* perfect drawing by computing the average color at each pixel
                * may be interrupted at each line if density <= 255 and at each
                * pixel if density > 255*/
                void _draw_perfect()
                    {
                    std::cout << "draw perfect \n";
                    return _draw_fast();
                    }



                /* return the y value of the integer strip containing the pixel at coord j
                if indeed contained in such a strip, otherwise, return distinct values < _range.min[1] */
                int64 _findLineStrip(int j)
                    {
                    const double m = _range.min[1];
                    const double a = m + _dly*j;
                    const double z = std::floor(a + 0.5);
                    return((a + _dly <= z + 0.5) ? ((int64)z) : ((int64)m - j - 10));
                    }


                /* return a value that will not match any returned by _findLineStrip */
                int64 _noLineStrip()
                    {
                    return ((int64)_range.min[1] - 10);
                    }



                static const int WORK_NOTHING = 100;
                static const int WORK_1TO1 = 101;
                static const int WORK_FAST = 102;
                static const int WORK_STOCHASTIC = 103;
                static const int WORK_PERFECT = 104;
                static const int WORK_FINISHED = 105;

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

                int _workPhase;                         // current work phase

                FastRNG _fastgen;                       // fast RNG

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

    inline RGBc colorImage(int64 x, int64 y)
        {
        if ((x < 0) || (x >= lenx)) return RGBc::c_Maroon;
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
        loadImage();
        const int LLX = 2200;
        const int LLY = 1400;
        const int UX = 2000;
        const int UY = 1300;

        ProgressImg progIm(LLX, LLY);
        progIm.clear((RGBc64)RGBc::c_Red);
        
        mtools::Img<unsigned char> dispIm(LLX, LLY, 1, 3);
        cimg_library::CImg<unsigned char> * cim = (cimg_library::CImg<unsigned char> *)&dispIm;

        fBox2 r(-0.5, UX-0.5, -0.5, UY-0.5);

        ThreadPixelDrawer<decltype(colorImage)> TPD(colorImage, nullptr);

        iBox2 SubB(50, 50 + UX - 1, 20, 20 + UY - 1);

        TPD.setParameters(r, &progIm, SubB);
        TPD.sync();
        TPD.enable(true);
        TPD.sync();
        int drawtype = 0, isaxe = 1, isgrid = 0, iscell = 1; // flags
        cimg_library::CImgDisplay DD(*cim); // display
        while ((!DD.is_closed())) 
            {
            uint32 k = DD.key();
            if ((DD.is_key(cimg_library::cimg::keyA))) { TPD.enable(!TPD.enable()); std::this_thread::sleep_for(std::chrono::milliseconds(50)); }   // type A for toggle axe (with graduations)
            if ((DD.is_key(cimg_library::cimg::keyG))) { isgrid = 1 - isgrid; std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // type G for toggle grid
            if ((DD.is_key(cimg_library::cimg::keyC))) { iscell = 1 - iscell; std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // type C for toggle cell
            if (DD.is_key(cimg_library::cimg::keyESC)) { TPD.redraw(); } // [ESC] to force complete redraw
            if (DD.is_key(cimg_library::cimg::keyARROWUP)) { double sh = r.ly() / 20; r.min[1] += sh; r.max[1] += sh; TPD.setParameters(r, &progIm, SubB); } // move n the four directions
            if (DD.is_key(cimg_library::cimg::keyARROWDOWN)) { double sh = r.ly() / 20; r.min[1] -= sh; r.max[1] -= sh; TPD.setParameters(r, &progIm, SubB);} //
            if (DD.is_key(cimg_library::cimg::keyARROWLEFT)) { double sh = r.lx() / 20; r.min[0] -= sh; r.max[0] -= sh; TPD.setParameters(r, &progIm, SubB); } //
            if (DD.is_key(cimg_library::cimg::keyARROWRIGHT)) { double sh = r.lx() / 20; r.min[0] += sh; r.max[0] += sh; TPD.setParameters(r, &progIm, SubB); } //
            if (DD.is_key(cimg_library::cimg::keyPAGEDOWN)) { double lx = r.max[0] - r.min[0]; double ly = r.max[1] - r.min[1]; r.min[0] = r.min[0] - (lx / 8.0); r.max[0] = r.max[0] + (lx / 8.0); r.min[1] = r.min[1] - (ly / 8.0);  r.max[1] = r.max[1] + (ly / 8.0); TPD.setParameters(r, &progIm, SubB); }
            if (DD.is_key(cimg_library::cimg::keyPAGEUP)) { if ((r.lx()>0.5) && (r.ly()>0.5)) { double lx = r.max[0] - r.min[0]; double ly = r.max[1] - r.min[1]; r.min[0] = r.min[0] + (lx / 10.0); r.max[0] = r.max[0] - (lx / 10.0); r.min[1] = r.min[1] + (ly / 10.0); r.max[1] = r.max[1] - (ly / 10.0); } TPD.setParameters(r, &progIm, SubB); }
       
            TPD.sync();
  //          std::cout << "quality = " << TPD.quality() << "\n";
            progIm.blit(dispIm);
//            if (isaxe) { dispIm.fBox2_drawAxes(r).fBox2_drawGraduations(r).fBox2_drawNumbers(r); }
//            if (isgrid) { dispIm.fBox2_drawGrid(r); }
//            if (iscell) { dispIm.fBox2_drawCells(r); }
            DD.display(*cim);
            }
        return;
        }







    int main(int argc, char * argv[])
        {

        /*
        cimg_library::CImg<mtools::RGBc> imrgbc;

        imrgbc.resize(100, 100, 1, 1);
        */
     //   RGBc * p = 0;
     //   imrgbc.draw_circle(50, 50, 10,p, 1.0);

        test(); 
        /*
        cout << sizeof(RGBc);
        cout.getKey();

        loadImage();

        Plotter2D plotter;
        auto P = mtools::makePlot2DLattice(colorImage, "test");
        plotter[P];
        plotter.plot();
        */
        return 0;
        }




