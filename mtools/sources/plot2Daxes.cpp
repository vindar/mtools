/** @file plot2Daxes.cpp */
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


#include "graphics/plot2Daxes.hpp"
#include "misc/indirectcall.hpp"
#include "io/internal/fltkSupervisor.hpp"

 

namespace mtools
{

    const bool Plot2DAxes::DEFAULT_GRAD_SHOW = true;
    const bool Plot2DAxes::DEFAULT_NUM_SHOW = true;
    const RGBc Plot2DAxes::DEFAULT_GRAD_COLOR = RGBc::c_Black;
    const RGBc Plot2DAxes::DEFAULT_NUM_COLOR = RGBc::c_Black;
    const float Plot2DAxes::DEFAULT_SCALING = 1.0;




        Plot2DAxes::Plot2DAxes(std::string name) :  internals_graphics::Plotter2DObj(name),
                                                    _gradStatus(DEFAULT_GRAD_SHOW), _numStatus(DEFAULT_NUM_SHOW),
                                                    _gradColor(DEFAULT_GRAD_COLOR), _numColor(DEFAULT_NUM_COLOR),
                                                    _scaling(DEFAULT_SCALING)
            {
            }


        Plot2DAxes::Plot2DAxes(Plot2DAxes && o) : internals_graphics::Plotter2DObj(std::move(o)),
            _gradStatus(o._gradStatus), _numStatus(o._numStatus),
            _gradColor(o._gradColor), _numColor(o._numColor),
            _scaling(o._scaling),
            _range(o._range),
            _imageSize(o._imageSize)
            {
            }


        Plot2DAxes::~Plot2DAxes()
            {
            detach();
            }


        void Plot2DAxes::scaling(float scaling)
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
                {
                IndirectMemberProc<Plot2DAxes, float> proxy(*this, &Plot2DAxes::scaling, scaling); // registers the call
                runInFltkThread(proxy);
                return;
                }
            if (scaling < 0.1) { scaling = 0.1f; } else if (scaling >= 5.0f) scaling = 5.0f;
            _scaling = scaling;
            if (!isInserted()) return;
            _scaleSlider->value(scaling);
            refresh();
            }


        void Plot2DAxes::graduations(bool show, RGBc color)
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
                {
                IndirectMemberProc<Plot2DAxes, bool, RGBc> proxy(*this, &Plot2DAxes::graduations, show, color); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _gradStatus = show;
            if (!_gradStatus) _numStatus = false;
            _gradColor = color;
            if (!isInserted()) return; // not inserted, we are done
            _gradButton->value(_gradStatus ? 1 : 0);
            _numButton->value(_numStatus ? 1 : 0);
            _gradColorButton->color((Fl_Color)_gradColor);
            _gradColorButton->color2((Fl_Color)_gradColor);
            _gradColorButton->redraw();
            refresh();
            yieldFocus();
            }


        void Plot2DAxes::graduations(bool show)
            {
            graduations(show, _gradColor);
            }


        void Plot2DAxes::graduations(RGBc color)
            {
            graduations(_gradStatus, color);
            }


        void Plot2DAxes::numbers(bool show, RGBc color)
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
                {
                IndirectMemberProc<Plot2DAxes, bool, RGBc> proxy(*this, &Plot2DAxes::numbers, show,color); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _numStatus = show;
            if (_numStatus) _gradStatus = true;
            _numColor = color;
            if (!isInserted()) return; // not inserted, we are done
            _gradButton->value(_gradStatus ? 1 : 0);
            _numButton->value(_numStatus ? 1 : 0);
            _numColorButton->color((Fl_Color)_numColor);
            _numColorButton->color2((Fl_Color)_numColor);
            _numColorButton->redraw();
            refresh();
            yieldFocus();
            }


        void Plot2DAxes::numbers(bool show)
            {
            numbers(show, _numColor);
            }


        void Plot2DAxes::numbers(RGBc color)
            {
            numbers(_numStatus, color);
            }


        RGBc Plot2DAxes::color() const { return _gradColor; }


        void Plot2DAxes::color(RGBc coul)
            {
            numbers(coul);
            graduations(coul);
            }


        void Plot2DAxes::setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
            {
            _range = range;
            _imageSize = imageSize;
            }


        int Plot2DAxes::drawOnto(Img<unsigned char> & im, float opacity)
            {
            im.fBox2_drawAxes(_range, _gradColor, opacity);
            if (_gradStatus) im.fBox2_drawGraduations(_range, _scaling, _gradColor, opacity);
            if (_numStatus) im.fBox2_drawNumbers(_range, _scaling, _numColor, opacity);
            return 100;
            }


        void Plot2DAxes::removed(Fl_Group * optionWin)
            {
            Fl::delete_widget(optionWin);
            }


        internals_graphics::Drawable2DInterface * Plot2DAxes::inserted(Fl_Group * & optionWin, int reqWidth)
            {
            /* create the option window */
            optionWin = new Fl_Group(0, 0, reqWidth,60); // create the option group

            _gradButton = new Fl_Check_Button(25, 10, 90, 15, "Graduations.");
            _gradButton->labelfont(0);
            _gradButton->labelsize(11);
            _gradButton->color2(FL_RED);
            _gradButton->callback(_gradButtonCB_static, this);
            _gradButton->when(FL_WHEN_CHANGED);
            _gradButton->value(_gradStatus ? 1 : 0);

            _numButton = new Fl_Check_Button(25, 35, 90, 15, "Numbers.");
            _numButton->labelfont(0);
            _numButton->labelsize(11);
            _numButton->color2(FL_RED);
            _numButton->callback(_numButtonCB_static, this);
            _numButton->when(FL_WHEN_CHANGED);
            _numButton->value(_numStatus ? 1 : 0);

            _gradColorButton = new Fl_Button(10, 10, 15, 15);
            _gradColorButton->color2((Fl_Color)_gradColor);
            _gradColorButton->color((Fl_Color)_gradColor);
            _gradColorButton->callback(_gradColorButtonCB_static, this);

            _numColorButton = new Fl_Button(10, 35, 15, 15);
            _numColorButton->color2((Fl_Color)_numColor);
            _numColorButton->color((Fl_Color)_numColor);
            _numColorButton->callback(_numColorButtonCB_static, this);

            _scaleSlider = new Fl_Value_Slider(reqWidth - 150, 30, 140, 15);
            _scaleSlider->align(Fl_Align(FL_ALIGN_TOP));
            _scaleSlider->box(FL_FLAT_BOX);
            _scaleSlider->type(FL_HOR_NICE_SLIDER);
            _scaleSlider->range(0.1, 5.0);
            _scaleSlider->step(0.1);
            _scaleSlider->value(_scaling);
            _scaleSlider->color2(FL_RED);
            _scaleSlider->callback(_scaleSliderCB_static, this);
              
            auto b = new Fl_Box(reqWidth - 105, 10, 80, 15, "scaling");
            b->labelfont(0);
            b->labelsize(11);

            optionWin->end();
        
            return this; 
            }



            void Plot2DAxes::_gradButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DAxes*)data)->_gradButtonCB(W); }
            void Plot2DAxes::_gradButtonCB(Fl_Widget * W)
                {
                graduations(_gradButton->value() == 0 ? false : true);
                }

            void Plot2DAxes::_numButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DAxes*)data)->_numButtonCB(W); }
            void Plot2DAxes::_numButtonCB(Fl_Widget * W)
                {
                numbers(_numButton->value() == 0 ? false : true);
                }

            void Plot2DAxes::_gradColorButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DAxes*)data)->_gradColorButtonCB(W); }
            void Plot2DAxes::_gradColorButtonCB(Fl_Widget * W)
                {
                unsigned char R = _gradColor.comp.R;
                unsigned char G = _gradColor.comp.G;
                unsigned char B = _gradColor.comp.B;
                if (fl_color_chooser("Axes Color", R, G, B, 1) != 0) { graduations(RGBc(R, G, B)); }
                }

            void Plot2DAxes::_numColorButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DAxes*)data)->_numColorButtonCB(W); }
            void Plot2DAxes::_numColorButtonCB(Fl_Widget * W)
                {
                unsigned char R = _numColor.comp.R;
                unsigned char G = _numColor.comp.G;
                unsigned char B = _numColor.comp.B;
                if (fl_color_chooser("Numbers Color", R, G, B, 1) != 0) { numbers(RGBc(R, G, B)); }
                }

            void Plot2DAxes::_scaleSliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DAxes*)data)->_scaleSliderCB(W); }
            void Plot2DAxes::_scaleSliderCB(Fl_Widget * W)
                {
                scaling((float)_scaleSlider->value());
                }


}

/* end of file */

