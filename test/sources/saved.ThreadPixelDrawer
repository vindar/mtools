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
                        default: { MTOOLS_ERROR("wtf!"); return 0; }
                        }
                    }


       
                /* set the new parameter */
                int _setNewParam()
                    {
                    const int MIN_IMAGE_SIZE = 2..................T;
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
                    std::cout << "draw perfect\n";  mtools::Chronometer();
                    RGBc64 * imData = _im->imData();
                    uint8 * normData = _im->normData();
                    const double px = _dlx; // lenght of a screen pixel
                    const double py = _dly; // height od a screen pixel
                    const int64 ilx = _subBox.lx() + 1; // number of horiz pixel in dest image
                    const int64 ily = _subBox.ly() + 1; // number of vert pixel in dest image
                    const fBox2 r = _range; // corresponding range
					
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
						
					//_dens < 1 // try to reduce pixel queries (by remembering last x-position)
					          // compute exact intersection aera 
							  
							 
						





						
					//1 < _dens < 50  // compute exact intersection aera 
					//50 <_dens // compute only pixel intersecting the aera
					
					_dens > 
						
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
        
        mtools::Img<unsigned char> dispIm(LLX, LLY, 1, 4);
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


