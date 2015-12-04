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


#include "stdafx_mtools.h"


#include "graphics/view2Dwidget.hpp"
#include "misc/error.hpp"
#include "randomgen/fastRNG.hpp"



namespace mtools
{

    namespace internals_graphics
    {


        View2DWidget::View2DWidget(int X, int Y, int W, int H) : CImgWidget(X, Y, W, H),
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
            _stocIm = new cimg_library::CImg<uint32>(1, 1, 1, 3);
            _stocImAlt = new cimg_library::CImg<uint32>(1, 1, 1, 3);
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
            redraw(); flush();
            }




        void View2DWidget::discardImage() { _discardIm = true; }


        void View2DWidget::improveImageFactor(cimg_library::CImg<unsigned char> * im)
            {
            if ((im == nullptr) || (im->width() <= 0) || (im->height() <= 0) || (im->spectrum() < 3)) { setImage(nullptr); _stocR.clear(); _nbRounds = 0; _discardIm = false; return; }
            _stocR = _RM->getRange(); // save the range for this image
            if ((_stocIm->width() * _zoomFactor != im->width()) || (_stocIm->height()*_zoomFactor != im->height()))
                { // resize needed, in this case, we also reset _nbRounds 
                _stocIm->resize(im->width() / _zoomFactor, im->height() / _zoomFactor, 1, 3, -1);
                _nbRounds = 0;
                }
            if (_zoomFactor == 1)
                { // case : no zoom
                for (int j = 0;j < _stocIm->height(); j++)
                    {
                    for (int i = 0;i < _stocIm->width(); i++)
                        {
                        _stocIm->operator()(i, j, 0, 0) = (uint32)(im->operator()(i, j, 0, 0));
                        _stocIm->operator()(i, j, 0, 1) = (uint32)(im->operator()(i, j, 0, 1));
                        _stocIm->operator()(i, j, 0, 2) = (uint32)(im->operator()(i, j, 0, 2));
                        }
                    }
                _nbRounds = 1;
                setImage32(_stocIm, _nbRounds);
                _discardIm =false;
                return;
                }
            if ((_discardIm)||(_nbRounds <= 0))
                { // zoom, first time we draw
                int rx = _zoomFactor / 2, ry = _zoomFactor / 2;
                for (int j = 0;j < _stocIm->height(); j++)
                    {
                    for (int i = 0;i < _stocIm->width(); i++)
                        {
                        _stocIm->operator()(i, j, 0, 0) = (uint32)(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry, 0, 0));
                        _stocIm->operator()(i, j, 0, 1) = (uint32)(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry, 0, 1));
                        _stocIm->operator()(i, j, 0, 2) = (uint32)(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry, 0, 2));
                        }
                    }
                _nbRounds = 1;
                setImage32(_stocIm, _nbRounds);
                _discardIm = false;
                return;
                }
            const int NBR = 4;
            for (int j = 0;j < _stocIm->height(); j++)
                { // zoom, improving the image
                for (int i = 0;i < _stocIm->width(); i++)
                    {
                    for (int k = 0;k < NBR;k++)
                        {
                        int rx = (int)floor(_g_fgen.unif()*_zoomFactor);
                        int ry = (int)floor(_g_fgen.unif()*_zoomFactor);
                        _stocIm->operator()(i, j, 0, 0) += (uint32)(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry, 0, 0));
                        _stocIm->operator()(i, j, 0, 1) += (uint32)(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry, 0, 1));
                        _stocIm->operator()(i, j, 0, 2) += (uint32)(im->operator()(i*_zoomFactor + rx, j*_zoomFactor + ry, 0, 2));
                        }
                    }
                }
            _nbRounds += NBR;
            setImage32(_stocIm, _nbRounds);
            redrawView();
            }


        void View2DWidget::displayMovedImage(RGBc bkColor)
        {
            fRect _stocRalt = _RM->getRange(); // current range
            if ((_stocRalt == _stocR) && (_stocIm->width() == w()) && (_stocIm->height() == h())) { return; } // nothing to do
            _stocImAlt->resize(w(),h(), 1, 3, -1); // make the alt image to the current view size
            _stocImAlt->clear(bkColor); // clear to the correct bk
            fRect subR = mtools::intersectionRect(_stocRalt, _stocR); // the intersection rectangle
            if ((!subR.isEmpty())&&(_nbRounds >0))
                {
                iVec2 mR1 = _stocR.absToPixel(fVec2(subR.xmin, subR.ymax), iVec2(_stocIm->width(), _stocIm->height()));
                iVec2 MR1 = _stocR.absToPixel(fVec2(subR.xmax, subR.ymin), iVec2(_stocIm->width(), _stocIm->height()));
                iRect iR1 = iRect(mR1.X(), MR1.X(), mR1.Y(), MR1.Y()); // the rectangle inside the stocIm image 
                iVec2 mR2 = _stocRalt.absToPixel(fVec2(subR.xmin, subR.ymax), iVec2(_stocImAlt->width(), _stocImAlt->height()));
                iVec2 MR2 = _stocRalt.absToPixel(fVec2(subR.xmax, subR.ymin), iVec2(_stocImAlt->width(), _stocImAlt->height()));
                iRect iR2 = iRect(mR2.X(), MR2.X(), mR2.Y(), MR2.Y()); // the rectangle inside the the stocImAlt image
                iR1.ymin++; iR1.ymax++; iR2.ymin++; iR2.ymax++; // correct the range so that the index are non negative
                if ((!iR1.isEmpty()) && (!iR2.isEmpty()) && (iR1.lx()*iR1.ly()*iR2.lx()*iR2.ly() > 0)) // redundant, check again is not empty
                    {
                    const double stx = ((double)(iR1.xmax - iR1.xmin)) / ((double)(iR2.xmax - iR2.xmin));
                    const double sty = ((double)(iR1.ymax - iR1.ymin)) / ((double)(iR2.ymax - iR2.ymin));
                    const int j1 = (int)iR2.ymin; const int j2 = (int)iR2.ymax;
                    const int i1 = (int)iR2.xmin; const int i2 = (int)iR2.xmax;
                        for (int j = j1; j < j2; j++)
                        {
                        int y = (int)(iR1.ymin + (int)floor((j - iR2.ymin)*sty));
                        for (int i = i1; i < i2; i++)
                            {
                            int x = (int)(iR1.xmin + (int)floor((i - iR2.xmin)*stx));
                            _stocImAlt->operator()(i, j, 0, 0) = _stocIm->operator()(x, y, 0, 0)/_nbRounds;
                            _stocImAlt->operator()(i, j, 0, 1) = _stocIm->operator()(x, y, 0, 1)/_nbRounds;
                            _stocImAlt->operator()(i, j, 0, 2) = _stocIm->operator()(x, y, 0, 2)/_nbRounds;
                            }
                        }
                    }
                }
            swapStocIm(); // swap the image
            _nbRounds = 1; // set the number of round to 1 for display
            _stocR = _RM->getRange(); // save the range for this new image
            setImage32(_stocIm, _nbRounds); // display
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
                if (_RM == nullptr) return CImgWidget::handle(e); // no range manager, disabled the widget and ignore events.
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
                            iRect R(_zoom1, _currentMouse);
                            if ((R.lx() > 10) && (R.ly() > 10))
                                {
                                fRect range = _RM->getRange();
                                fRect R(_RM->pixelToAbs(((int64)_zoomFactor)*_zoom1), _RM->pixelToAbs(((int64)_zoomFactor)*_currentMouse)); // absolute rectangle
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
                    case FL_KEYUP: return 1;
                    }
                return CImgWidget::handle(e);
                }



        void View2DWidget::draw()
        {
            if (damage() == FL_DAMAGE_USER1) // erase only the overlay.
                {
                if (_isIn(_prevMouse)) // erase the cross if it was previously drawn
                    {
                    partDraw(iRect(0, w()-1, _prevMouse.Y(), _prevMouse.Y()));
                    partDraw(iRect(_prevMouse.X(), _prevMouse.X(), 0, h()-1));
                    }
                if (_isIn(_zoom2)) // erase the rectangles if previously drawn
                    {
                    iRect R(_zoom1, _zoom2);
                    partDraw(iRect(R.xmin, R.xmin, R.ymin, R.ymax));
                    partDraw(iRect(R.xmax, R.xmax, R.ymin, R.ymax));
                    partDraw(iRect(R.xmin, R.xmax, R.ymin, R.ymin));
                    partDraw(iRect(R.xmin, R.xmax, R.ymax, R.ymax));
                    if (fixedRatio())
                        {
                        partDraw(iRect(_encR.xmin, _encR.xmin, _encR.ymin, _encR.ymax));
                        partDraw(iRect(_encR.xmax, _encR.xmax, _encR.ymin, _encR.ymax));
                        partDraw(iRect(_encR.xmin, _encR.xmax, _encR.ymin, _encR.ymin));
                        partDraw(iRect(_encR.xmin, _encR.xmax, _encR.ymax, _encR.ymax));
                        }
                    }
                }
            else { CImgWidget::draw(); } // redraw the whole thing otherwise
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
                    iRect R(_zoom1, _zoom2);
                    if (fixedRatio()) fl_color(FL_GRAY); else fl_color(FL_RED);
                    fl_line((int)R.xmin, (int)R.ymin, (int)R.xmin, (int)R.ymax);
                    fl_line((int)R.xmax, (int)R.ymin, (int)R.xmax, (int)R.ymax);
                    fl_line((int)R.xmin, (int)R.ymin, (int)R.xmax, (int)R.ymin);
                    fl_line((int)R.xmin, (int)R.ymax, (int)R.xmax, (int)R.ymax);
                    if (fixedRatio())
                        {
                        fRect range = _RM->getRange();
                        fRect aR(_RM->pixelToAbs(((int64)_zoomFactor)*_zoom1), _RM->pixelToAbs(((int64)_zoomFactor)*_zoom2)); // absolute rectangle
                        fRect bR = aR.fixedRatioEnclosedRect(range.lx() / range.ly());
                        auto v1 = _RM->absToPix(fVec2{ bR.xmin, bR.ymin }); v1 /= ((int64)_zoomFactor);
                        auto v2 = _RM->absToPix(fVec2{ bR.xmax, bR.ymax }); v2 /= ((int64)_zoomFactor);
                        _encR = iRect( v1, v2 );
                        fl_color(FL_RED);
                        fl_line((int)_encR.xmin, (int)_encR.ymin, (int)_encR.xmin, (int)_encR.ymax);
                        fl_line((int)_encR.xmax, (int)_encR.ymin, (int)_encR.xmax, (int)_encR.ymax);
                        fl_line((int)_encR.xmin, (int)_encR.ymin, (int)_encR.xmax, (int)_encR.ymin);
                        fl_line((int)_encR.xmin, (int)_encR.ymax, (int)_encR.xmax, (int)_encR.ymax);
                        }
                    }
                }
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








