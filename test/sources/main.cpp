#include "stdafx_test.h"

#include "mtools.hpp"

#include "misc/threadworker.hpp"

using namespace mtools;



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
            std::cout << "work\n";
            while (1)
                {
                switch (_workPhase)
                    {
                    case WORK_1TO1: { _draw_1to1(); break; }
                    case WORK_FAST: { _draw_fast(); break; }
                    case WORK_STOCHASTIC: { _draw_stochastic(); break; }
                    case WORK_PERFECT: { _draw_perfect(); break; }
                    case WORK_FINISHED: { return; }
                    case WORK_NOTHING: { MTOOLS_ERROR("hum..."); }
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
            static const double DENSITY_SKIP_STOCHASTIC = 5.0;
            while (1)
                {
                switch (_workPhase)
                    {
                    case WORK_NOTHING:
                        {
                        if (_is1to1) { _workPhase = WORK_1TO1; return; }
                        _workPhase = WORK_FAST;
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
            setProgress(1);
            uint64 tim = mtools::Chronometer(); std::cout << "finished in " << tim << " ms\n";
            }



        /* stochastic drawing sample a pixel
        * may be interrupted at each line */
        void _draw_stochastic()
            {
            std::cout << "draw stochastic \n";  mtools::Chronometer();
            // compute the number of sample required            
            int sampleToDo;
            if (_dens < 10.0) { sampleToDo = (int)_dens/2; } else if (_dens < 20000) { sampleToDo = 5 + ((int)_dens)/20; } else sampleToDo = 1000;
            std::cout << "number of passes = " << sampleToDo  << "\n";

            int sampleDone = 1; // 1 sample currently done (the fast one)            
            if ((sampleToDo - sampleDone ) < 199)
                { // ok, we do everything on a single pass. 
                _draw_stochastic_batch(1, sampleToDo - sampleDone, sampleDone, sampleToDo);
                }
            else
                { // need multiple passes
                _draw_stochastic_batch(1, 199, sampleDone, sampleToDo);  // go to 200
                _progimage_div2(); // go back to 100
                int batchSize = 2; 
                while(batchSize*100 < sampleToDo) 
                    { 
                    _draw_stochastic_batch(batchSize, 100, sampleDone, sampleToDo); // go from 100 to 200
                    _progimage_div2(); // back to 100
                    batchSize *= 2;
                    }
                _draw_stochastic_batch(batchSize, sampleToDo/batchSize, sampleDone, sampleToDo); // do the remaining passes
                }
            setProgress(50);
            uint64 tim = mtools::Chronometer(); std::cout << "finished in " << tim << " ms\n";
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
                        for (int l = 0;l < batchsize; l++)
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
                setProgress(1 + (49 * sd) / sampleToDo);
                }
            sampleDone += (nb*batchsize);
            }





        /* perfect drawing by computing the average color at each pixel
        * may be interrupted at each line if density <= 255 and at each
        * pixel if density > 255*/
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
            uint64 tim = mtools::Chronometer(); std::cout << "finished in " << tim << " ms\n";
            return;
            }


        /* draw perfectly, ultra high density : do nothing more than the stochasitc approximation */
        void _draw_perfect_ultrahighdensity()
            {
            std::cout << "draw perfect  ULTRA HIGH density = " << _dens << "]\n";  mtools::Chronometer();
      
            // stochastic is good enough, do nothing...
            return;
            }


        /* draw perfectly, hidh density, check() after every pixel */
        void _draw_perfect_highdensity()
            {
            std::cout << "draw perfect HIGH density = " << _dens << "]\n";  mtools::Chronometer();

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
            std::cout << "draw perfect LOW DENSITY = " << _dens << "]\n";  mtools::Chronometer();

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

            for(int64 jj = 0; jj < ily; jj++)
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

    //return RGBc::c_Blue;
    //if ((x < 0) || (x >= lenx)) return RGBc::c_Maroon;
    //if ((y < 0) || (y >= leny)) return RGBc::c_Green;

    if (x < 0) return RGBc::c_Maroon;
    if (y < 0) return RGBc::c_Green;

    return lenna.getPixel({ x % lenx , leny - 1 - (y % leny) });
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

    mtools::Img<unsigned char> dispIm(LLX, LLY, 1, 4);
    cimg_library::CImg<unsigned char> * cim = (cimg_library::CImg<unsigned char> *)&dispIm;

    fBox2 r(-0.5, UX - 0.5, -0.5, UY - 0.5);

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
        if (DD.is_key(cimg_library::cimg::keyARROWDOWN)) { double sh = r.ly() / 20; r.min[1] -= sh; r.max[1] -= sh; TPD.setParameters(r, &progIm, SubB); } //
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


