/** @file plot2Dbasegraph.cpp */
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

#include "graphics/plot2Dbasegraph.hpp"
#include "misc/indirectcall.hpp"
#include "io/fltkSupervisor.hpp"


namespace mtools
{

    namespace internals_graphics
    {


        const int Plot2DBaseGraph::DEFAULT_PLOT_QUALITY = 15;
        const int Plot2DBaseGraph::RANGE_SAMPLE_SIZE = 10000;

        Plot2DBaseGraph::Plot2DBaseGraph(double minDomain, double maxDomain, std::string name) : internals_graphics::Plotter2DObjWithColor(name), _minDomain(minDomain), _maxDomain(maxDomain),
            _drawMethod(true), _dichoQuality(DEFAULT_PLOT_QUALITY), _tickness(1), _drawOver(false), _drawUnder(false), _drawOverColor(RGBc::c_Blue), _drawUnderColor(RGBc::c_Green), _drawOverOpacity(0.2f), _drawUnderOpacity(0.2f)
            {
            }


        Plot2DBaseGraph::Plot2DBaseGraph(std::string name) : internals_graphics::Plotter2DObjWithColor(name), _minDomain(-std::numeric_limits<double>::infinity()), _maxDomain(+std::numeric_limits<double>::infinity()),
            _drawMethod(true), _dichoQuality(DEFAULT_PLOT_QUALITY), _tickness(1), _drawOver(false), _drawUnder(false), _drawOverColor(RGBc::c_Blue), _drawUnderColor(RGBc::c_Green), _drawOverOpacity(0.2f), _drawUnderOpacity(0.2f)
            {
            }



        Plot2DBaseGraph::Plot2DBaseGraph(Plot2DBaseGraph && obj) : internals_graphics::Plotter2DObjWithColor(std::move(obj)),
            _range(obj._range), _imageSize(obj._imageSize),
            _minDomain(obj._minDomain), _maxDomain(obj._maxDomain),
            _drawMethod(obj._drawMethod), _dichoQuality(obj._dichoQuality), _tickness(obj._tickness), _drawOver(obj._drawOver), _drawUnder(obj._drawUnder), _drawOverColor(obj._drawOverColor),
            _drawUnderColor(obj._drawUnderColor), _drawOverOpacity(obj._drawOverOpacity), _drawUnderOpacity(obj._drawUnderOpacity)
            {}





        Plot2DBaseGraph::~Plot2DBaseGraph()
            {
            detach();
            }


        void Plot2DBaseGraph::drawLines()
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph> proxy(*this, &Plot2DBaseGraph::drawLines); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawMethod = true;
            _updateWidgets();
            }


        void Plot2DBaseGraph::drawDots(int quality)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, int> proxy(*this, &Plot2DBaseGraph::drawDots, quality); // registers the call
                runInFltkThread(proxy);
                return;
                }
            if (quality < 0) quality = 0; else if (quality > 30) quality = 30;
            _dichoQuality = quality;
            _drawMethod = false;
            _updateWidgets();
            }


        void Plot2DBaseGraph::tickness(int tick)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph,int> proxy(*this, &Plot2DBaseGraph::tickness,tick); // registers the call
                runInFltkThread(proxy);
                return;
                }
            if (tick < 1) tick = 1; else if (tick>20) tick = 20;
            _tickness = tick;
            _updateWidgets();
            }


        void Plot2DBaseGraph::epigraph(bool status)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, bool> proxy(*this, &Plot2DBaseGraph::epigraph, status); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawOver = status;
            _updateWidgets();
            }


        void Plot2DBaseGraph::epigraphColor(RGBc color)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, RGBc> proxy(*this, &Plot2DBaseGraph::epigraphColor, color); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawOverColor = color;
            _updateWidgets();
            }


        void Plot2DBaseGraph::epigraphOpacity(float opacity)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, float> proxy(*this, &Plot2DBaseGraph::epigraphOpacity, opacity); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawOverOpacity = opacity;
            _updateWidgets();
            }


        void Plot2DBaseGraph::hypograph(bool status)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, bool> proxy(*this, &Plot2DBaseGraph::hypograph, status); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawUnder = status;
            _updateWidgets();
            }


        void Plot2DBaseGraph::hypographColor(RGBc color)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, RGBc> proxy(*this, &Plot2DBaseGraph::hypographColor, color); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawUnderColor = color;
            _updateWidgets();
            }


        void Plot2DBaseGraph::hypographOpacity(float opacity)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraph, float> proxy(*this, &Plot2DBaseGraph::hypographOpacity, opacity); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _drawUnderOpacity = opacity;
            _updateWidgets();
            }


        fBox2 Plot2DBaseGraph::favouriteRangeX(fBox2 R)
            {
            double l = _maxDomain - _minDomain;
            if ((l > 2 * DBL_MIN) && (l < DBL_MAX / 2))
                {
                R.min[0] = _minDomain;
                R.max[0] = _maxDomain;
                }
            else
                {
                R.clearHorizontally();
                }
            return R;
            }


        fBox2 Plot2DBaseGraph::favouriteRangeY(fBox2 R)
            {
            if ((R.lx() < DBL_MIN * 2) || (R.lx() > DBL_MAX / 2))
                {
                R = favouriteRangeX(R);
                if (R.isHorizontallyEmpty()) return fBox2();
                }
            R.clearVertically();
            estimateYRange(R);
            if ((R.min[1] > -DBL_MAX / 2) && (R.max[1] < DBL_MAX / 2)) return R;
            R.clearVertically();
            return R;
            }


        bool Plot2DBaseGraph::hasFavouriteRangeX()
            {
            return (!favouriteRangeX(fBox2()).isHorizontallyEmpty());
            }


        bool Plot2DBaseGraph::hasFavouriteRangeY()
            {
            return true;
            }


        void Plot2DBaseGraph::setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
            {
            _range = range;
            _imageSize = imageSize;
            }


        int Plot2DBaseGraph::drawOnto(Img<unsigned char> & im, float opacity)
            {
            if (_drawOver)  { _drawOverOrBelow(true, im, _range, _drawOverColor, _drawOverOpacity*opacity); }
            if (_drawUnder) { _drawOverOrBelow(false, im, _range, _drawUnderColor, _drawUnderOpacity*opacity); }
            if (_drawMethod) {_drawWithInterpolation(_dichoQuality, im, _range, color(), opacity, _tickness); } else { _drawWithDicho(_dichoQuality, im, _range, color(), opacity, _tickness); }
            return 100;
            }


        Fl_Group * Plot2DBaseGraph::optionalPanel(int reqWidth)
            {
            return nullptr;
            }


        void Plot2DBaseGraph::optionalPanelRemoved(Fl_Group * opt)
            {
            if (opt != nullptr) Fl::delete_widget(opt);
            return;
            }



        internals_graphics::Drawable2DObject * Plot2DBaseGraph::inserted(Fl_Group * & optionWin, int reqWidth)
                {

                auto gr = new Fl_Group(0, 0, reqWidth, 125); // create the option group

                auto w = new Fl_Box(10, 5, 90, 15, "Drawing Method:");
		w->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                w->labelfont(0);
                w->labelsize(11);

                auto w2 = new Fl_Box(110, 5, 40, 15, "tickness");
		w2->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                w2->labelfont(0);
                w2->labelsize(11);

                _ticknessSlider = new Fl_Value_Slider(160, 5 + 1, 80, 14);
                _ticknessSlider->labelfont(0);
                _ticknessSlider->labelsize(11);
                _ticknessSlider->align(Fl_Align(FL_ALIGN_LEFT));
                _ticknessSlider->box(FL_FLAT_BOX);
                _ticknessSlider->type(FL_HOR_NICE_SLIDER);
                _ticknessSlider->range(1, 20);
                _ticknessSlider->step(1);
                _ticknessSlider->value(_tickness);
                _ticknessSlider->color2(FL_RED);
                _ticknessSlider->callback(_ticknessSliderCB_static, this);

                _interpolateCheck = new Fl_Round_Button(20, 25, 190, 15, "Connect the drawing using lines.");
                _interpolateCheck->labelfont(0);
                _interpolateCheck->labelsize(11);
                _interpolateCheck->color2(FL_RED);
                _interpolateCheck->type(102);
                 _interpolateCheck->callback(_roundButtonCB_static, this);
                _interpolateCheck->when(FL_WHEN_CHANGED);
                _dichoCheck = new Fl_Round_Button(20, 45, 140, 15, "Use dots. Precision:");
                _dichoCheck->labelfont(0);
                _dichoCheck->labelsize(11);
                _dichoCheck->color2(FL_RED);
                _dichoCheck->type(102);
                _dichoCheck->callback(_roundButtonCB_static, this);
                _dichoCheck->when(FL_WHEN_CHANGED);
                if (_drawMethod) { _interpolateCheck->setonly(); } else { _dichoCheck->setonly(); }

                _dichoQualitySlider = new Fl_Value_Slider(160, 45+1, 80, 14);
                _dichoQualitySlider->labelfont(0);
                _dichoQualitySlider->labelsize(11);
                _dichoQualitySlider->align(Fl_Align(FL_ALIGN_LEFT));
                _dichoQualitySlider->box(FL_FLAT_BOX);
                _dichoQualitySlider->type(FL_HOR_NICE_SLIDER);
                _dichoQualitySlider->range(0, 30);
                _dichoQualitySlider->step(1);
                _dichoQualitySlider->value(_dichoQuality);
                _dichoQualitySlider->color2(FL_RED);
                _dichoQualitySlider->callback(_dichoQualitySliderCB_static, this);

                _overButton = new Fl_Check_Button(25, 70+5, 120, 15, "Fill the epigraph");
                _overButton->labelfont(0);
                _overButton->labelsize(11);
                _overButton->color2(FL_RED);
                _overButton->callback(_overButtonCB_static, this);
                _overButton->when(FL_WHEN_CHANGED);
                _overButton->value(_drawOver ? 1 : 0);

                _underButton = new Fl_Check_Button(25, 90 + 5, 120, 15, "Fill the hypograph");
                _underButton->labelfont(0);
                _underButton->labelsize(11);
                _underButton->color2(FL_RED);
                _underButton->callback(_underButtonCB_static, this);
                _underButton->when(FL_WHEN_CHANGED);
                _underButton->value(_drawUnder ? 1 : 0);

                _overColorButton = new Fl_Button(10, 70 + 5, 15, 15);
                _overColorButton->color2((Fl_Color)_drawOverColor);
                _overColorButton->color((Fl_Color)_drawOverColor);
                _overColorButton->callback(_overColorButtonCB_static, this);

                _underColorButton = new Fl_Button(10, 90 + 5, 15, 15);
                _underColorButton->color2((Fl_Color)_drawUnderColor);
                _underColorButton->color((Fl_Color)_drawUnderColor);
                _underColorButton->callback(_underColorButtonCB_static, this);

                _overSlider = new Fl_Value_Slider(145, 70 + 1 + 5, 80, 14);
                _overSlider->labelfont(0);
                _overSlider->labelsize(11);
                _overSlider->align(Fl_Align(FL_ALIGN_LEFT));
                _overSlider->box(FL_FLAT_BOX);
                _overSlider->type(FL_HOR_NICE_SLIDER);
                _overSlider->range(0, 1.0);
                _overSlider->step(0.01);
                _overSlider->value(_drawOverOpacity);
                _overSlider->color2(FL_RED);
                _overSlider->callback(_overSliderCB_static, this);

                _underSlider = new Fl_Value_Slider(145, 90 + 1 + 5, 80, 14);
                _underSlider->labelfont(0);
                _underSlider->labelsize(11);
                _underSlider->align(Fl_Align(FL_ALIGN_LEFT));
                _underSlider->box(FL_FLAT_BOX);
                _underSlider->type(FL_HOR_NICE_SLIDER);
                _underSlider->range(0, 1.0);
                _underSlider->step(0.01);
                _underSlider->value(_drawUnderOpacity);
                _underSlider->color2(FL_RED);
                _underSlider->callback(_underSliderCB_static, this);

                gr->end();
                gr->resizable(0);

                _optGroup = optionalPanel(reqWidth); // get the optional pannel
                if (_optGroup == nullptr)
                    {
                    optionWin = gr;
                    }
                else
                    {
                    _optGroup->end();
                    optionWin = new Fl_Group(0, 0, reqWidth, _optGroup->h() + gr->h());
                    optionWin->add(_optGroup);
                    optionWin->add(gr);
                    optionWin->resizable(0);
                    _optGroup->resize(0, 0, reqWidth, _optGroup->h());
                    gr->resize(0, _optGroup->h(), reqWidth, gr->h());
                    optionWin->end();
                    }

                return(this);
                }


        void Plot2DBaseGraph::removed(Fl_Group * optionWin)
                {
                if (_optGroup != nullptr)
                    {
                    optionWin->remove(_optGroup);
                    optionalPanelRemoved(_optGroup);
                    }
                Fl::delete_widget(optionWin);
                }


        void Plot2DBaseGraph::estimateYRange(fBox2 & R) const
            {
                R.min[1] = 1.0;
                R.max[1] = -1.0;
                if (R.max[0] < R.min[0]) { return; }
                for (int i = 0; i < RANGE_SAMPLE_SIZE + 1; i++)
                {
                    double y = _function(R.min[0] + (R.lx() / RANGE_SAMPLE_SIZE)*i);
                    if (!std::isnan(y))
                    {
                        if (R.min[1] > R.max[1]) { R.min[1] = y; R.max[1] = y; } else
                        {
                        if (y < R.min[1]) R.min[1] = y; else  if (y > R.max[1]) R.max[1] = y;
                        }
                    }
                }
                return;
            }


        void Plot2DBaseGraph::_roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_roundButtonCB(W); }
        void Plot2DBaseGraph::_roundButtonCB(Fl_Widget * W)
                {
                if (W == _interpolateCheck) { _drawMethod = true; } else { _drawMethod = false; }
                refresh();
                }


        void Plot2DBaseGraph::_dichoQualitySliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_dichoQualitySliderCB(W); }
        void Plot2DBaseGraph::_dichoQualitySliderCB(Fl_Widget * W)
                {
                _dichoQuality = (int)_dichoQualitySlider->value();
                refresh();
                }


        void Plot2DBaseGraph::_overButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_overButtonCB(W); }
        void Plot2DBaseGraph::_overButtonCB(Fl_Widget * W)
                {
                _drawOver = _overButton->value() == 0 ? false : true;
                refresh();
                }


        void Plot2DBaseGraph::_underButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_underButtonCB(W); }
        void Plot2DBaseGraph::_underButtonCB(Fl_Widget * W)
                {
                _drawUnder = _underButton->value() == 0 ? false : true;
                refresh();
                }


        void Plot2DBaseGraph::_overColorButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_overColorButtonCB(W); }
        void Plot2DBaseGraph::_overColorButtonCB(Fl_Widget * W)
                {
                RGBc coul = _drawOverColor;
                if (fl_color_chooser("Color to use", coul.comp.R , coul.comp.G, coul.comp.B, 1) != 0)
                    {
                    _drawOverColor = coul;
                    _overColorButton->color((Fl_Color)coul);
                    _overColorButton->color2((Fl_Color)coul);
                    _overColorButton->redraw();
                    refresh();
                    }
                }


        void Plot2DBaseGraph::_underColorButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_underColorButtonCB(W); }
        void Plot2DBaseGraph::_underColorButtonCB(Fl_Widget * W)
                {
                RGBc coul = _drawUnderColor;
                if (fl_color_chooser("Color to use", coul.comp.R , coul.comp.G, coul.comp.B, 1) != 0)
                    {
                    _drawUnderColor = coul;
                    _underColorButton->color((Fl_Color)coul);
                    _underColorButton->color2((Fl_Color)coul);
                    _underColorButton->redraw();
                    refresh();
                    }
                }


        void Plot2DBaseGraph::_underSliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_underSliderCB(W); }
        void Plot2DBaseGraph::_underSliderCB(Fl_Widget * W)
            {
                _drawUnderOpacity = (float)_underSlider->value();
                refresh();
            }


        void Plot2DBaseGraph::_overSliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_overSliderCB(W); }
        void Plot2DBaseGraph::_overSliderCB(Fl_Widget * W)
            {
            _drawOverOpacity = (float)_overSlider->value();
            refresh();
            }


        void Plot2DBaseGraph::_ticknessSliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraph*)data)->_ticknessSliderCB(W); }
        void Plot2DBaseGraph::_ticknessSliderCB(Fl_Widget * W)
            {
            _tickness = (int)_ticknessSlider->value();
            refresh();
            }




        void Plot2DBaseGraph::_updateWidgets()
            {
            if (!isInserted()) return; // nothing to do if not inserted.
            if (_drawMethod == true) { _interpolateCheck->setonly(); } else { _dichoCheck->setonly(); }
            _ticknessSlider->value(_tickness);
            _dichoQualitySlider->value(_dichoQuality);
            _overButton->value(_drawOver ? 1 : 0);
            _underButton->value(_drawUnder ? 1 : 0);
            _overColorButton->color((Fl_Color)_drawOverColor);
            _overColorButton->color2((Fl_Color)_drawOverColor);
            _overColorButton->redraw();
            _underColorButton->color((Fl_Color)_drawUnderColor);
            _underColorButton->color2((Fl_Color)_drawUnderColor);
            _underColorButton->redraw();
            _overSlider->value(_drawOverOpacity);
            _underSlider->value(_drawUnderOpacity);
            refresh();
            }


        void Plot2DBaseGraph::_drawPoint(int i, int j, Img<unsigned char> & im, const RGBc & coul, const float opacity, const int tickness)
            {
            if (tickness <= 1) { im.drawPoint(iVec2(i, j), coul, opacity); } else { im.drawPointCirclePen(iVec2(i, j), tickness-1, coul, opacity); }
            }


        void Plot2DBaseGraph::_drawLine(int i, int j1, int j2, Img<unsigned char> & im, const RGBc & coul, const float opacity, const int tickness)
            {
            if (tickness <= 1) { im.drawLine(iVec2(i, j1), iVec2(i + 1, j2), coul, opacity); } else { im.drawLineCirclePen(iVec2(i, j1), iVec2(i + 1, j2), tickness-1, coul, opacity); }
            }


        void Plot2DBaseGraph::_dicho(int j0, int i1, int i2, int j3, double x0, double x3, int depth, Img<unsigned char> & im, const fBox2 & R, const RGBc & coul, const float & opacity, const int & tickness)
            {
                if (depth <= 0) return;
                double esp = (x3 - x0)/3.0;
                double x1 = x0 + esp;
                double x2 = x3 - esp;
                double y1 = _function(x1);
                double y2 = _function(x2);

                int j1 = ((y1 >= R.min[1]) && (y1 <= R.max[1])) ? (im.height() - 1 - (int)floor((y1 - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y1 >= R.max[1]) ? -tickness - 1 : im.height() + tickness);
                int j2 = ((y2 >= R.min[1]) && (y2 <= R.max[1])) ? (im.height() - 1 - (int)floor((y2 - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y2 >= R.max[1]) ? -tickness - 1 : im.height() + tickness);

                if (j1 != j0) _drawPoint(i1, j1, im, coul, opacity,tickness);
                if (j2 != j3)
                    {
                    if ((i2 != i1) || ((j2 != j1) && (j2 != j0))) _drawPoint(i2, j2, im, coul, opacity, tickness);
                    }

                if ((j0 - j1 > 1) || (j0 - j1 < -1))
                    { // iterate between x0 and x1
                    _dicho(j0, i1, i1, j1, x0, x1, depth - 1, im, R, coul, opacity, tickness);
                    }
                if ((j2 - j1 > 1) || (j2 - j1 < -1))
                    { // iterate between x1 and x2
                    _dicho(j1, i1, i2, j2, x1, x2, depth - 1, im, R, coul, opacity, tickness);
                    }
                if ((j3 - j2 > 1) || (j3 - j2 < -1))
                    { // iterate between x2 and x3
                    _dicho(j2, i2, i2, j3, x2, x3, depth - 1, im, R, coul, opacity, tickness);
                    }
            }


        void Plot2DBaseGraph::_drawWithDicho(int depth, Img<unsigned char> & im, const fBox2 & R, const RGBc coul, const float opacity, const int tickness)
            {
            double eps = R.lx() / im.width();
            double x1 = R.min[0] + (eps / 2.0);
            double y1 = _function(x1);
            int j1 = ((y1 >= R.min[1]) && (y1 <= R.max[1])) ? (im.height() - 1 - (int)floor((y1 - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y1 >= R.max[1]) ? -1 - tickness : im.height()+tickness);
            _drawPoint(0, j1, im, coul, opacity, tickness);
            for (int i = 1; i < im.width(); i++)
                {
                double x2 = x1 + eps;
                double y2 = _function(x2);
                int j2 = ((y2 >= R.min[1]) && (y2 <= R.max[1])) ? (im.height() - 1 - (int)floor((y2 - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y2 >= R.max[1]) ? -1 - tickness : im.height() + tickness);
                _drawPoint(i, j2, im, coul, opacity, tickness);
                if ((j2 - j1 > 1) || (j2 - j1 < -1))
                    {
                    _dicho(j1, i-1, i, j2, x1, x2, depth, im, R, coul, opacity,tickness);
                    }
                x1 = x2;
                y1 = y2;
                j1 = j2;
                }
            }


        void Plot2DBaseGraph::_drawWithInterpolation(int depth, Img<unsigned char> & im, const fBox2 & R, const RGBc coul, const float opacity, const int tickness)
            {
                double eps = R.lx() / im.width();
                double x1 = R.min[0] + (eps / 2.0);
                double y1 = _function(x1);
                int j1 = ((y1 >= R.min[1]) && (y1 <= R.max[1])) ? (im.height() - 1 - (int)floor((y1 - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y1 >= R.max[1]) ? -1 - tickness : im.height() + tickness);
                _drawPoint(0, j1, im, coul, opacity, tickness);
                for (int i = 1; i < im.width(); i++)
                {
                    double x2 = x1 + eps;
                    double y2 = _function(x2);
                    int j2 = ((y2 >= R.min[1]) && (y2 <= R.max[1])) ? (im.height() - 1 - (int)floor((y2 - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y2 >= R.max[1]) ? -1-tickness : im.height() + tickness);
                    if ((!std::isnan(y1)) && (!std::isnan(y2))) _drawLine(i - 1, j1, j2, im, coul, opacity, tickness);
                    x1 = x2;
                    y1 = y2;
                    j1 = j2;
                }
            }


        void Plot2DBaseGraph::_drawOverOrBelow(bool over, Img<unsigned char> & im, const fBox2 & R, RGBc coul, const float opacity)
            {
            double eps = R.lx() / im.width();
            double x = R.min[0] + (eps / 2.0);
            for (int i = 0; i < im.width(); i++)
                {
                double y = _function(x);
                int j = ((y >= R.min[1]) && (y <= R.max[1])) ? (im.height() - 1 - (int)floor((y - R.min[1]) / R.ly()*im.height() + 0.5)) : ((y >= R.max[1]) ? -1 : im.height());
                if (!std::isnan(y))
                    {
                    if (over) im.drawLine(iVec2(i, -1), iVec2(i, j - 1), coul, opacity); else im.drawLine(iVec2(i, im.height()), iVec2(i, j + 1), coul, opacity);
                    }
                x += eps;
                }
            }














        Plot2DBaseGraphWithInterpolation::Plot2DBaseGraphWithInterpolation(double minDomain, double maxDomain, std::string name) : Plot2DBaseGraph(minDomain, maxDomain, name), _interpolationType(INTERPOLATION_NONE), _optGroup(nullptr)
            {
            }



        Plot2DBaseGraphWithInterpolation::Plot2DBaseGraphWithInterpolation(Plot2DBaseGraphWithInterpolation && obj) : Plot2DBaseGraph(std::move(obj)), _interpolationType((int)obj._interpolationType), _optGroup(nullptr)
            {
            }





        Plot2DBaseGraphWithInterpolation::~Plot2DBaseGraphWithInterpolation()
            {
            detach();
            }


        void Plot2DBaseGraphWithInterpolation::interpolationMethod(int type)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plot2DBaseGraphWithInterpolation, int> proxy(*this, &Plot2DBaseGraphWithInterpolation::interpolationMethod, type); // registers the call
                runInFltkThread(proxy);
                return;
                }
            if ((_interpolationType < 0) || (_interpolationType > 3)) _interpolationType = INTERPOLATION_NONE; else _interpolationType = type;
            _setInterpolationButtons();
            refresh();
            }


        int Plot2DBaseGraphWithInterpolation::interpolationMethod() const { return _interpolationType; }

        void Plot2DBaseGraphWithInterpolation::interpolationNone() { interpolationMethod(INTERPOLATION_NONE); }

        void Plot2DBaseGraphWithInterpolation::interpolationLinear() { interpolationMethod(INTERPOLATION_LINEAR); }

        void Plot2DBaseGraphWithInterpolation::interpolationCubic() { interpolationMethod(INTERPOLATION_CUBIC); }

        void Plot2DBaseGraphWithInterpolation::interpolationMonotoneCubic() { interpolationMethod(INTERPOLATION_MONOTONE_CUBIC); }




        Fl_Group * Plot2DBaseGraphWithInterpolation::optionalPanel(int reqWidth)
            {
                _optGroup = new Fl_Group(0, 0, reqWidth, 70); // create the option group

                auto b = new Fl_Box(10, 5, 150, 15, "Interpolation method:");
		b->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
                b->labelfont(0);
                b->labelsize(11);

                _interNone = new Fl_Round_Button(20, 25, 50, 15, "None");
                _interNone->labelfont(0);
                _interNone->labelsize(11);
                _interNone->color2(FL_RED);
                _interNone->type(102);
                _interNone->callback(_interButtonCB_static, this);
                _interNone->when(FL_WHEN_CHANGED);

                _interLinear = new Fl_Round_Button(130, 25, 65, 15, "Linear");
                _interLinear->labelfont(0);
                _interLinear->labelsize(11);
                _interLinear->color2(FL_RED);
                _interLinear->type(102);
                _interLinear->callback(_interButtonCB_static, this);
                _interLinear->when(FL_WHEN_CHANGED);

                _interCubic = new Fl_Round_Button(20, 45, 50, 15, "Cubic");
                _interCubic->labelfont(0);
                _interCubic->labelsize(11);
                _interCubic->color2(FL_RED);
                _interCubic->type(102);
                _interCubic->callback(_interButtonCB_static, this);
                _interCubic->when(FL_WHEN_CHANGED);

                _interCubicMono = new Fl_Round_Button(130, 45, 115, 15, "Monotone Cubic");
                _interCubicMono->labelfont(0);
                _interCubicMono->labelsize(11);
                _interCubicMono->color2(FL_RED);
                _interCubicMono->type(102);
                _interCubicMono->callback(_interButtonCB_static, this);
                _interCubicMono->when(FL_WHEN_CHANGED);

                _setInterpolationButtons();
                return _optGroup;
            }

        void Plot2DBaseGraphWithInterpolation::optionalPanelRemoved(Fl_Group * opt)
            {
            Fl::delete_widget(_optGroup);
            _optGroup = nullptr;
            }


        void Plot2DBaseGraphWithInterpolation::_interButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DBaseGraphWithInterpolation*)data)->_interButtonCB(W); }
        void Plot2DBaseGraphWithInterpolation::_interButtonCB(Fl_Widget * W)
            {
            if (W == _interNone)        _interpolationType = INTERPOLATION_NONE; else
            if (W == _interLinear)      _interpolationType = INTERPOLATION_LINEAR; else
            if (W == _interCubic)       _interpolationType = INTERPOLATION_CUBIC; else
            if (W == _interCubicMono)   _interpolationType = INTERPOLATION_MONOTONE_CUBIC;
            refresh();
            yieldFocus();
            }


        void Plot2DBaseGraphWithInterpolation::_setInterpolationButtons()
            {
            if (_optGroup == nullptr) return;
            if (_interpolationType == INTERPOLATION_NONE) { _interNone->setonly(); return; }
            if (_interpolationType == INTERPOLATION_LINEAR) { _interLinear->setonly(); return; }
            if (_interpolationType == INTERPOLATION_CUBIC) { _interCubic->setonly(); return; }
            if (_interpolationType == INTERPOLATION_MONOTONE_CUBIC) { _interCubicMono->setonly(); return; }
            MTOOLS_ERROR("hum?");
            }


        const int Plot2DBaseGraphWithInterpolation::INTERPOLATION_NONE = 0;
        const int Plot2DBaseGraphWithInterpolation::INTERPOLATION_LINEAR = 1;
        const int Plot2DBaseGraphWithInterpolation::INTERPOLATION_CUBIC = 2;
        const int Plot2DBaseGraphWithInterpolation::INTERPOLATION_MONOTONE_CUBIC = 3;


    }

}


/* end of file */


