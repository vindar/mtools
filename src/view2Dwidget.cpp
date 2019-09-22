/** @file view2Dwidget.cpp */
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


#include "graphics/internal/view2Dwidget.hpp"
#include "misc/error.hpp"
#include "random/gen_fastRNG.hpp"



namespace mtools
{

    namespace internals_graphics
    {


        View2DWidget::View2DWidget(int X, int Y, int W, int H) : ImageWidget(X, Y, W, H),
            _crossOn(false),
            _prevMouse(-1, -1),
            _currentMouse(-1,-1),
            _zoomOn(false),
            _crossCB(nullptr),
            _crossData(nullptr),
            _notCB(nullptr),
            _notData(nullptr),
            _RM(nullptr),
            _zoomFactor(1),
            _nbRounds(0),
            _discardIm(true)
            {
            _stocIm = new ProgressImg(1, 1);
            _stocImAlt = new ProgressImg(1, 1);
            }


        View2DWidget::~View2DWidget() 
            {
            delete _stocIm;
            delete _stocImAlt;
            }


        iVec2 View2DWidget::viewSizeFactor() const
            {
            return iVec2(w()*_zoomFactor, h()*_zoomFactor);
            }


        int View2DWidget::zoomFactor(int z)
            {
            if (_RM == nullptr) return _zoomFactor;
            if ((z < 1)|| (z > 20)||(z == _zoomFactor)) return _zoomFactor;
            int64 mem = 8 * ((int64)w())*((int64)h())*z;
            if (mem > 1500000000) return _zoomFactor;
            int oldzf = _zoomFactor;
            _zoomFactor = z;
            if (!_RM->winSize(iVec2(w()*z, h()*z))) { _zoomFactor = oldzf; }
            return _zoomFactor;
            }


        int View2DWidget::zoomFactor() const
            {
            return _zoomFactor;
            }


        void View2DWidget::setRangeManager(RangeManager * RM)
            {
            _RM = RM;
            if (_RM == nullptr) return;
            iVec2 oSize = _RM->getWinSize();
            iVec2 nSize = viewSizeFactor();
            if (oSize != nSize)
                {
                _RM->winSize(nSize);
                redrawView();
                }
            }


        void View2DWidget::setNotificationCB(pnotcb callback, void * data)
            {
            _notCB = callback;
            _notData = data;
            }


        void View2DWidget::crossOn(bool status) { _crossOn = status; if (_crossCB != nullptr) { _crossCB(_crossData, status); } }


        bool View2DWidget::crossOn() const { return _crossOn; }


        void View2DWidget::setCrossCB(pcrosscb callback, void * data)
            {
            _crossCB = callback;
            _crossData = data;
            }


        void View2DWidget::fixedRatio(bool status)
            {
            if (_RM == nullptr) return;
            _RM->fixedAspectRatio(status);
            }


        bool View2DWidget::fixedRatio() const
            {
            if (_RM == nullptr) return false;
            return _RM->fixedAspectRatio();
            }


        void View2DWidget::redrawView()
            {
			std::cerr << "AAAAAAAA\n";
			std::cerr << mtools::ChronometerMicro() << "\n";
            redraw(); 
			std::cerr << "BBBBBBB\n";
			std::cerr << mtools::ChronometerMicro() << "\n";
			flush();
			std::cerr << "CCCCCCC\n";
			std::cerr << mtools::ChronometerMicro() << "\n";

			}


        void View2DWidget::discardImage() { _discardIm = true; }


        void View2DWidget::improveImageFactor(const Image * im)
            {
            if ((im == nullptr) || (im->isEmpty())) { setImage((const Image *)nullptr); _stocR.clear(); _nbRounds = 0; _discardIm = false; return; }
            _stocR = _RM->getRange(); // save the range for this image
            if (((int64)(_stocIm->width() * _zoomFactor) != im->width()) || ((int64)(_stocIm->height()*_zoomFactor) != im->height()))
                { // resize needed, in this case, we also reset _nbRounds 
                _stocIm->resize((size_t)(im->width() / _zoomFactor), (size_t)(im->height() / _zoomFactor),false);
                _nbRounds = 0;
                }

			const int64 lx = _stocIm->width();
			const int64 ly = _stocIm->height();
			RGBc64 * pdest = _stocIm->imData();
			
			if (_zoomFactor == 1)
                { // case : no zoom
				const RGBc * psrc = im->data();
				const int64 pad = im->padding();
				for (int64 j = 0; j < ly; j++)
					{
					for (int64 i = 0; i < lx; i++) 
						{
						*(pdest++) = *(psrc++);
						}
					psrc += pad;
					}
				*_stocIm->normData() = 0;
				_nbRounds = 1;
				setImage(_stocIm);
				_discardIm =false;
				redrawView();
                return;
                }

			if ((_discardIm)||(_nbRounds <= 0))
                { // zoom, first time we draw
				const int64 rx = _zoomFactor/2, ry = _zoomFactor/2;
                for (int64 j = 0; j < ly; j++)
                    {
                    for (int64 i = 0; i < lx; i++)
                        {
						*(pdest++) = im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry);
                        }
                    }
				*_stocIm->normData() = 0;
				_nbRounds = 1;
                setImage(_stocIm);
                _discardIm = false;
				redrawView();
                return;
                }

			const int NBR = 4;
			const int M = 255 / (2*NBR);

			if (_nbRounds < NBR)
				{ // improve by 1 until the number of improvement is exactly NBR.
				for (int64 j = 0; j < ly; j++)
					{
					for (int64 i = 0; i < lx; i++)
						{
						int64 rx = (int64)floor(_g_fgen.unif()*_zoomFactor);
						int64 ry = (int64)floor(_g_fgen.unif()*_zoomFactor);
						(pdest++)->add(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry));
						}
					}
				_nbRounds++;
				*_stocIm->normData() = _nbRounds - 1;
				setImage(_stocIm);
				redrawView();
				return;
				}

			if (_nbRounds >= M*2*NBR)
				{ // divide by 2 
				const int64 l = lx*ly;
				RGBc64 * p = _stocIm->imData();
				for (int64 i = 0; i<l; i++) 
					{  
					p->comp.R >>= 1;
					p->comp.G >>= 1;
					p->comp.B >>= 1;
					p->comp.A >>= 1;
					p++;
					}
				_nbRounds /= 2;
				*_stocIm->normData() = _nbRounds - 1;
				}

            for (int64 j = 0; j < ly; j++)
                { // zoom, improving the image
                for (int64 i = 0; i < lx; i++)
                    {
					for (int k = 0; k < NBR; k++)
						{
						int64 rx = (int64)floor(_g_fgen.unif()*_zoomFactor);
						int64 ry = (int64)floor(_g_fgen.unif()*_zoomFactor);
						(pdest)->add(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry));
						}
					pdest++;
                    }
                }
            _nbRounds += NBR;
			*_stocIm->normData() = _nbRounds - 1;
			setImage(_stocIm);
            redrawView();
            }



        void View2DWidget::displayMovedImage(RGBc bkColor)
        {
            fBox2 _stocRalt = _RM->getRange(); // current range
            if ((_stocRalt == _stocR) && (_stocIm->width() == (size_t)w()) && (_stocIm->height() == (size_t)h())) { return; } // nothing to do
            _stocImAlt->resize((size_t)w(),(size_t)h()); // make the alt image to the current view size
            _stocImAlt->clear(bkColor); // clear to the correct bk
            fBox2 subR = mtools::intersectionRect(_stocRalt, _stocR); // the intersection rectangle
            if ((!subR.isEmpty())&&(_nbRounds >0))
                {
                iVec2 mR1 = _stocR.absToPixel(fVec2(subR.min[0], subR.max[1]), iVec2(_stocIm->width(), _stocIm->height()));
                iVec2 MR1 = _stocR.absToPixel(fVec2(subR.max[0], subR.min[1]), iVec2(_stocIm->width(), _stocIm->height()));
                iBox2 iR1 = iBox2(mR1.X(), MR1.X(), mR1.Y(), MR1.Y()); // the rectangle inside the stocIm image 
                iVec2 mR2 = _stocRalt.absToPixel(fVec2(subR.min[0], subR.max[1]), iVec2(_stocImAlt->width(), _stocImAlt->height()));
                iVec2 MR2 = _stocRalt.absToPixel(fVec2(subR.max[0], subR.min[1]), iVec2(_stocImAlt->width(), _stocImAlt->height()));
                iBox2 iR2 = iBox2(mR2.X(), MR2.X(), mR2.Y(), MR2.Y()); // the rectangle inside the the stocImAlt image
                iR1.min[1]++; iR1.max[1]++; iR2.min[1]++; iR2.max[1]++; // correct the range so that the index are non negative
                if ((!iR1.isEmpty()) && (!iR2.isEmpty()) && (iR1.lx()*iR1.ly()*iR2.lx()*iR2.ly() > 0)) // redundant, check again is not empty
                    {
                    const double stx = ((double)(iR1.max[0] - iR1.min[0])) / ((double)(iR2.max[0] - iR2.min[0]));
                    const double sty = ((double)(iR1.max[1] - iR1.min[1])) / ((double)(iR2.max[1] - iR2.min[1]));
                    const int j1 = (int)iR2.min[1]; const int j2 = (int)iR2.max[1];
                    const int i1 = (int)iR2.min[0]; const int i2 = (int)iR2.max[0];
                    for (int j = j1; j < j2; j++)
                        {
                        int y = (int)(iR1.min[1] + (int)floor((j - iR2.min[1])*sty));
                        for (int i = i1; i < i2; i++)
                            {
                            int x = (int)(iR1.min[0] + (int)floor((i - iR2.min[0])*stx));
							const RGBc64 & srccol = *_stocIm->imData(x, y);
							RGBc64 & dstcol = *_stocImAlt->imData(i, j);
							dstcol.comp.R = srccol.comp.R / _nbRounds;
							dstcol.comp.G = srccol.comp.G / _nbRounds;
							dstcol.comp.B = srccol.comp.B / _nbRounds;
							dstcol.comp.A = 255;
                            }
                        }
                    }
                }
            swapStocIm(); // swap the image
            _nbRounds = 1; // set the number of round to 1 for display
            _stocR = _RM->getRange(); // save the range for this new image
            setImage(_stocIm); // display
            redrawView(); // and refresh the view
            return;
        }












        void View2DWidget::resize(int X, int Y, int	W, int H)
            {
            Fl_Window::resize(X, Y, W, H);
            if (_RM != nullptr)
                {
                iVec2 oSize = _RM->getWinSize();
                iVec2 nSize = iVec2{ W*_zoomFactor, H*_zoomFactor };
                if (oSize != nSize)
                    {
                    _RM->winSize(nSize);
                    redrawView();
                    }
                }
            }


        int View2DWidget::handle(int e)
                {
                if (_RM == nullptr) return ImageWidget::handle(e); // no range manager, disabled the widget and ignore events.
                switch (e)
                    {
                    case FL_LEAVE: { _saveMouse(); if (_zoomOn) { _zoomOn = false; redrawView(); } return 1; }
                    case FL_ENTER: { take_focus(); _saveMouse(); if (_zoomOn) { _zoomOn = false; redrawView(); } return 1; }
                    case FL_FOCUS: return 1;
                    case FL_UNFOCUS: return 1;
                    case FL_PUSH:
                        {
                        take_focus();
                        _saveMouse();
                        if (!_isIn(_currentMouse)) { if (_zoomOn) { _zoomOn = false; redrawView(); } return 1; }
                        int button = Fl::event_button();
                        if ((button == FL_MIDDLE_MOUSE) || (button == FL_RIGHT_MOUSE))
                            { // we center at this position
                            _zoomOn = false;
                            _RM->center(_RM->pixelToAbs(((int64)_zoomFactor)*_currentMouse));
                            redrawView();
                            return 1;
                            }
                        _zoomOn = true;
                        _zoom1 = _currentMouse;
                        return 1;
                        }
                    case FL_DRAG: // mouse moved while button pushed
                        {
                        _saveMouse();
                        if (!_isIn(_currentMouse)) { if (_zoomOn) { _zoomOn = false; redrawView(); return 1; } }
                        if ((_crossOn) || (_zoomOn)) { damage(FL_DAMAGE_USER1); flush(); }
                        return 1;
                        }
                    case FL_RELEASE: // mouse button released
                        {
                        _saveMouse();
                        if (!_isIn(_currentMouse)) { _zoomOn = false; redrawView();  return 1; }
                        _zoomOn = false;
                        int button = Fl::event_button();
                        if (button == FL_LEFT_MOUSE)
                            {
                            iBox2 R(_zoom1, _currentMouse,true);
                            if ((R.lx() > 10) && (R.ly() > 10))
                                {
                                fBox2 range = _RM->getRange();
                                fBox2 R(_RM->pixelToAbs(((int64)_zoomFactor)*_zoom1), _RM->pixelToAbs(((int64)_zoomFactor)*_currentMouse), true); // absolute rectangle
                                if (fixedRatio()) { R = R.fixedRatioEnclosedRect(range.lx() / range.ly()); }
                                _RM->setRange(R);
                                redrawView();
                                return 1;
                                }
                            }
                        damage(FL_DAMAGE_USER1);
                        return 1;
                        }
                    case FL_MOVE:
                        {
                        _saveMouse();
                        if (!_isIn(_currentMouse)) { if (_zoomOn) { _zoomOn = false; redrawView(); return 1; } } else { take_focus(); }
                        if ((_crossOn) || (_zoomOn)) { damage(FL_DAMAGE_USER1); }
                        return 1;
                        }
                    case FL_MOUSEWHEEL:
                        {
                        take_focus();
                        _saveMouse();
                        if (!_isIn(_currentMouse)) { if (_zoomOn) { _zoomOn = false; redrawView(); return 1; } }
                        int d = Fl::event_dy();
                        if (d < 0) { _RM->zoomIn();  redrawView(); return 1; }
                        if (d > 0) { _RM->zoomOut(); redrawView(); return 1; }
                        return 1;
                        }
                    case FL_KEYDOWN:
						{
                        take_focus();
						int key = Fl::event_key();
                        if ((key == FL_BackSpace) || (key == FL_Delete))
                            {
                            _RM->canonicalRange();
                            redrawView();
                            return 1;
                            }
                        if ((key == '1') || (key == '&'))
                            {
                            _RM->set1to1();
                            redrawView();
                            return 1;
                            }
                        if ((key == 'c') || (key == 'C')) // toogle cross on/off
                            {
                            _crossOn = !_crossOn;
                            if (_crossCB != nullptr) { _crossOn = _crossCB(_crossData, _crossOn); }
                            redrawView();
                            return 1;
                            }
                        if (key == FL_Page_Up)
                            {
                            _RM->zoomIn();
                            redrawView();
                            return 1;
                            }

                        if (key == FL_Page_Down)
                            {
                            _RM->zoomOut();
                            redrawView();
                            return 1;
                            }
                        if (key == FL_Left)
                            {
                            _RM->left();
							redrawView();
                            return 1;
                            }
                        if (key == FL_Right)
                            {
                            _RM->right();
                            redrawView();
                            return 1;
                            }
                        if (key == FL_Up)
                            {
							_RM->up();
                            redrawView();
							return 1;
                            }
                        if (key == FL_Down)
                            {
                            _RM->down();
                            redrawView();
                            return 1;
                            }
                        if (key == FL_Escape)
                            {
                            _RM->reset();
                            redrawView();
                            return 1;
                            }
                        if (key == FL_Enter)
                            {
                            _RM->setRatio1();
                            redrawView();
                            return 1;
                            }
                        if (key == FL_Insert)
                            {
                            fixedRatio(!fixedRatio());
                            redrawView();
                            return 1;
                            }
                        if (_notCB != nullptr) { _notCB(_notData, key); } // forward other key stoke if a callback if defined
                        return 1;
                        }
                    case FL_KEYUP:
						{
						std::cerr << "U";

						return 1; 
						}
                    }
                return ImageWidget::handle(e);
                }



        void View2DWidget::draw()
        {
			std::cerr << "begin draw\n";
			std::cerr << mtools::ChronometerMicro() << "\n";

            if (damage() == FL_DAMAGE_USER1) // erase only the overlay.
                {
                if (_isIn(_prevMouse)) // erase the cross if it was previously drawn
                    {
                    partDraw(iBox2(0, w()-1, _prevMouse.Y(), _prevMouse.Y()));
                    partDraw(iBox2(_prevMouse.X(), _prevMouse.X(), 0, h()-1));
                    }
                if (_isIn(_zoom2)) // erase the rectangles if previously drawn
                    {
                    iBox2 R(_zoom1, _zoom2, true);
                    partDraw(iBox2(R.min[0], R.min[0], R.min[1], R.max[1]));
                    partDraw(iBox2(R.max[0], R.max[0], R.min[1], R.max[1]));
                    partDraw(iBox2(R.min[0], R.max[0], R.min[1], R.min[1]));
                    partDraw(iBox2(R.min[0], R.max[0], R.max[1], R.max[1]));
                    if (fixedRatio())
                        {
                        partDraw(iBox2(_encR.min[0], _encR.min[0], _encR.min[1], _encR.max[1]));
                        partDraw(iBox2(_encR.max[0], _encR.max[0], _encR.min[1], _encR.max[1]));
                        partDraw(iBox2(_encR.min[0], _encR.max[0], _encR.min[1], _encR.min[1]));
                        partDraw(iBox2(_encR.min[0], _encR.max[0], _encR.max[1], _encR.max[1]));
                        }
                    }
                }
            else { ImageWidget::draw(); } // redraw the whole thing otherwise
            _zoom2 = { -1, -1 };
            _prevMouse = { -1, -1 };
            if (_RM == nullptr) { return; }
            if ((_zoomFactor > 1)||(_crossOn))
                {
                fl_color(FL_BLACK);
                fl_rectf(w() - 105, 5 , 100, 20);
                fl_color(FL_WHITE);
                fl_font(FL_HELVETICA, 12);
                fl_draw((std::string("[") + toString(((int64)_zoomFactor)*w()) + " x " + toString(((int64)_zoomFactor)*h()) + "]").c_str(), w()-100, 20);
                }
            if (_isIn(_currentMouse))
                {
                if (_crossOn)
                    {
                    _prevMouse = _currentMouse;
                    fl_color(FL_BLACK);
                    if ((_prevMouse.X() >= 0) && (_prevMouse.X() < ((ox() < w()) ? ox() : w()) )) { fl_line((int)_prevMouse.X(), 0, (int)_prevMouse.X(), ((oy() < h()) ? oy() : h() ) - 1); }
                    if ((_prevMouse.X() >= 0) && (_prevMouse.Y() < ((oy() < h()) ? oy() : h()) )) { fl_line(0, (int)_prevMouse.Y(), ((ox() < w()) ? ox() : w()) - 1, (int)_prevMouse.Y()); }
                    fl_rectf(5, 5, 170, 35);
                    fl_color(FL_WHITE);
                    fl_font(FL_HELVETICA, 12);
                    fVec2 pos = _RM->pixelToAbs({ ((int64)_zoomFactor)*_prevMouse.X(), ((int64)_zoomFactor)*_prevMouse.Y() });
                    fl_draw((std::string("X = ") + mtools::doubleToStringNice(pos.X())).c_str(), 10, 20);
                    fl_draw((std::string("Y = ") + mtools::doubleToStringNice(pos.Y())).c_str(), 10, 35);
                    }
                if (_zoomOn)
                    {
                    _zoom2 = _currentMouse;
                    iBox2 R(_zoom1, _zoom2, true);
                    if (fixedRatio()) fl_color(FL_GRAY); else fl_color(FL_RED);
                    fl_line((int)R.min[0], (int)R.min[1], (int)R.min[0], (int)R.max[1]);
                    fl_line((int)R.max[0], (int)R.min[1], (int)R.max[0], (int)R.max[1]);
                    fl_line((int)R.min[0], (int)R.min[1], (int)R.max[0], (int)R.min[1]);
                    fl_line((int)R.min[0], (int)R.max[1], (int)R.max[0], (int)R.max[1]);
                    if (fixedRatio())
                        {
                        fBox2 range = _RM->getRange();
                        fBox2 aR(_RM->pixelToAbs(((int64)_zoomFactor)*_zoom1), _RM->pixelToAbs(((int64)_zoomFactor)*_zoom2), true); // absolute rectangle
                        fBox2 bR = aR.fixedRatioEnclosedRect(range.lx() / range.ly());
                        auto v1 = _RM->absToPix(fVec2{ bR.min[0], bR.min[1] }); v1 /= ((int64)_zoomFactor);
                        auto v2 = _RM->absToPix(fVec2{ bR.max[0], bR.max[1] }); v2 /= ((int64)_zoomFactor);
                        _encR = iBox2( v1, v2, true );
                        fl_color(FL_RED);
                        fl_line((int)_encR.min[0], (int)_encR.min[1], (int)_encR.min[0], (int)_encR.max[1]);
                        fl_line((int)_encR.max[0], (int)_encR.min[1], (int)_encR.max[0], (int)_encR.max[1]);
                        fl_line((int)_encR.min[0], (int)_encR.min[1], (int)_encR.max[0], (int)_encR.min[1]);
                        fl_line((int)_encR.min[0], (int)_encR.max[1], (int)_encR.max[0], (int)_encR.max[1]);
                        }
                    }
                }
			std::cerr << "end draw\n";
			std::cerr << mtools::ChronometerMicro() << "\n";
            }


        bool View2DWidget::_isIn(iVec2 m)
            {
            return ((m.X() >= 0) && (m.Y() >= 0) && (m.X() < w()) && (m.Y() < h()));
            }


        void View2DWidget::_saveMouse()
            {
            _currentMouse = iVec2(Fl::event_x(), Fl::event_y());
            }

    }
}


/* end of file */








