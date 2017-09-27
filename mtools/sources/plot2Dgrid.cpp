/** @file plot2Dgrid.cpp */
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

#include "graphics/plot2Dgrid.hpp"
#include "misc/indirectcall.hpp"
#include "io/internal/fltkSupervisor.hpp"


namespace mtools
{



    Plot2DGrid::Plot2DGrid(bool fitToAxes, std::string name) : internals_graphics::Plotter2DObj(name), _win(nullptr), _color(RGBc::c_Black),  _hspace(1.0), _hoffset(0), _vspace(1.0), _voffset(0),  _fittoaxes(fitToAxes)
        {
        opacity(DEFAULT_OPACITY);
        }


    Plot2DGrid::Plot2DGrid(Plot2DGrid && o) : internals_graphics::Plotter2DObj(std::move(o)), _win(nullptr), _color((RGBc)o._color), _hspace((double)o._hspace),
        _hoffset((double)o._hoffset), _vspace((double)o._vspace), _voffset((double)o._voffset), _fittoaxes((bool)o._fittoaxes)
        {
        }


    Plot2DGrid::~Plot2DGrid()
        {
        detach();
        }


    bool Plot2DGrid::fitToAxes() const { return _fittoaxes; }

    void Plot2DGrid::fitToAxes(bool fit)
        {
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid,bool> proxy(*this, &Plot2DGrid::fitToAxes,fit); // registers the call
            runInFltkThread(proxy);
            return;
            }
        _fittoaxes = fit;
        _updateWidgets();
        refresh();
        }


    void Plot2DGrid::setUnitGrid()
        {
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid> proxy(*this, &Plot2DGrid::setUnitGrid); // registers the call
            runInFltkThread(proxy);
            return;
            }
        _fittoaxes = false;
        _hspace = 1.0; _vspace = 1.0;
        _hoffset = 0.0; _voffset = 0.0;
        _updateWidgets();
        refresh();
        }


    void Plot2DGrid::setUnitCells()
        {
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid> proxy(*this, &Plot2DGrid::setUnitCells); // registers the call
            runInFltkThread(proxy);
            return;
            }
        _fittoaxes = false;
        _hspace = 1.0; _vspace = 1.0;
        _hoffset = 0.5; _voffset = 0.5;
        _updateWidgets();
        refresh();
        }



    double Plot2DGrid::horizontalSpacing() const { return _hspace; }


    void Plot2DGrid::horizontalSpacing(double val)
        {
        if (std::isnan(val)) val = -1.0;
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid, double> proxy(*this, &Plot2DGrid::horizontalSpacing, val); // registers the call
            runInFltkThread(proxy);
            return;
            }
        if (val <= 0.0) { _hspace = 0.0; } else { _hspace = val; }
        _fittoaxes = false;
        _fixOffset();
        _updateWidgets();
        refresh();
        }



    double Plot2DGrid::horizontalOffset() const { return _hoffset; }



    void Plot2DGrid::horizontalOffset(double offset)
    {
        if (std::isnan(offset)) offset = 0.0;
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid, double> proxy(*this, &Plot2DGrid::horizontalOffset, offset); // registers the call
            runInFltkThread(proxy);
            return;
            }
        if (_hspace > 0.0) { _hoffset = offset; }
        _fittoaxes = false;
        _fixOffset();
        _updateWidgets();
        refresh();
        }



    double Plot2DGrid::verticalSpacing() const { return _vspace; }



    void Plot2DGrid::verticalSpacing(double val)
        {
        if (std::isnan(val)) val = -1.0;
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid, double> proxy(*this, &Plot2DGrid::verticalSpacing, val); // registers the call
            runInFltkThread(proxy);
            return;
            }
        if (val <= 0.0) { _vspace = 0.0; } else { _vspace = val; }
        _fittoaxes = false;
        _fixOffset();
        _updateWidgets();
        refresh();
        }



    double Plot2DGrid::verticalOffset() const { return _voffset; }



    void Plot2DGrid::verticalOffset(double offset)
        {
        if (std::isnan(offset)) offset = 0.0;
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid, double> proxy(*this, &Plot2DGrid::verticalOffset, offset); // registers the call
            runInFltkThread(proxy);
            return;
            }
        if (_vspace > 0.0) { _voffset = offset; }  // nothing to do
        _fittoaxes = false;
        _fixOffset();
        _updateWidgets();
        refresh();
        }



    RGBc Plot2DGrid::color() const { return _color; }



    void Plot2DGrid::color(RGBc col)
        {
        if (!isFltkThread()) // run the method in FLTK if not in it
            {
            IndirectMemberProc<Plot2DGrid, RGBc> proxy(*this, &Plot2DGrid::color, std::move(col)); // registers the call
            runInFltkThread(proxy);
            return;
            }
        _color = col;
        _updateWidgets();
        refresh();
        }




        void Plot2DGrid::setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
        {
        _range = range;
        _imageSize = imageSize;
        if (_fittoaxes) { _fitToAxes();  _updateWidgets(); }
        }



    void Plot2DGrid::_fitToAxes()
    {
        int l; double op, k, kk, pp, v1, v2;

        op = ::log10(_range.ly()); if (op < 0) { l = ((int)(op)) - 1; } else { l = ((int)(op)); }
        k = ::pow(10.0, (double)(l));
        v1 = floor(_range.min[1] / k); v1 = v1 - 1; v2 = floor(_range.max[1] / k); v2 = v2 + 1;
        kk = k; pp = kk / 5;
        if ((v2 - v1) < 5) { kk = k / 2; pp = kk / 5; } else { if ((v2 - v1) > 8) { kk = k * 2; pp = kk / 2; v1 = ((v1 / 2) * 2) - 2; } }
        _vspace = ((pp < kk) ? pp : kk);
        _voffset = 0.0;

        op = ::log10(_range.lx()); if (op < 0) { l = ((int)op) - 1; } else { l = (int)op; }
        k = ::pow(10.0, (double)(l));
        v1 = floor(_range.min[0] / k);  v1 = v1 - 1; v2 = floor(_range.max[0] / k);  v2 = v2 + 1;
        kk = k; pp = kk / 5;
        if ((v2 - v1) < 5) { kk = k / 2; pp = kk / 5; } else { if ((v2 - v1) > 8) { kk = k * 2; pp = kk / 2; v1 = ((v1 / 2) * 2) - 2; } }
        _hspace = ((pp < kk) ? pp : kk);
        _hoffset = 0.0;
    }


        int Plot2DGrid::drawOnto(Img<unsigned char> & im, float opacity)
        {
            MTOOLS_ASSERT((im.width() == _imageSize.X()) && (im.height() == _imageSize.Y()));
            const int MINPXL = 5;

            double pxlx = _imageSize.X() / _range.lx();
            if (pxlx*_hspace >= (double)MINPXL)
                { // ok, we should draw the vertical lines
                for (int i = -1; i < _imageSize.X(); i++)
                    {
                    double e1 = std::remainder(_range.pixelToAbs(iVec2(i, 0), _imageSize).X() - _hoffset, _hspace);
                    double e2 = std::remainder(_range.pixelToAbs(iVec2(i + 1, 0), _imageSize).X() - _hoffset, _hspace);
                    if ((e1 <= 0.0) && (e2 >= 0.0))
                        {
                        if ((-e1) <= e2)
                            {
                            if (i >= 0)
                                {
                                im.drawVerticalLine(i, _color, opacity);
                                }
                            }
                        else
                            {
                            if (_imageSize.X())
                                {
                                im.drawVerticalLine(i+1, _color, opacity);
                                }
                            }
                        i += (MINPXL - 3);
                        }
                    }
                }

            double pxly = _imageSize.Y() / _range.ly();
            if (pxly*_vspace >= (double)MINPXL)
                { // ok, we should draw the horizontal lines
                for (int i = -1; i < _imageSize.Y(); i++)
                    {
                    double e1 = std::remainder(_range.pixelToAbs(iVec2(0,i), _imageSize).Y() - _voffset, _vspace);
                    double e2 = std::remainder(_range.pixelToAbs(iVec2(0,i + 1), _imageSize).Y() - _voffset, _vspace);
                    if ((e1 >= 0.0) && (e2 <= 0.0))
                        {
                        if ((e1) <= (-e2))
                            {
                            if (i >= 0)
                                {
                                im.drawHorizontalLine(i, _color, opacity);
                                }
                            }
                        else
                            {
                            if (_imageSize.Y())
                                {
                                im.drawHorizontalLine(i+1, _color, opacity);
                                }
                            }
                        i += (MINPXL - 3);
                        }
                    }
                }

            return 100;
        }



        void Plot2DGrid::removed(Fl_Group * optionWin)
        {
        _win = nullptr;
        Fl::delete_widget(optionWin);
        }



        internals_graphics::Drawable2DInterface * Plot2DGrid::inserted(Fl_Group * & optionWin, int reqWidth)
        {

            /* create the option window */
            _win = new Fl_Group(0, 0, reqWidth,90); // create the option group
            optionWin = _win;

            _colorButton = new Fl_Button(5, 5, 15, 15);
            _colorButton->color2((Fl_Color)((RGBc)_color));
            _colorButton->color((Fl_Color)((RGBc)_color));
            _colorButton->callback(_colorCB_static, this);

            auto colorText = new Fl_Box(20, 5, 100, 15, "Color of the grid.");
            colorText->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT));
            colorText->labelfont(0);
            colorText->labelsize(11);

            _unitGridButton = new Fl_Button(140, 5, 50, 16, "unit grid");
            _unitGridButton->labelfont(0);
            _unitGridButton->labelsize(9);
            _unitGridButton->callback(_unitGridCB_static, this);

            _unitCellsButton = new Fl_Button(200, 5, 50, 16, "unit cells");
            _unitCellsButton->labelfont(0);
            _unitCellsButton->labelsize(9);
            _unitCellsButton->callback(_unitCellsCB_static, this);


            _fitAxesCheckbox = new Fl_Check_Button(5, 25, 150, 15, "Match the axes graduations.");
            _fitAxesCheckbox->labelfont(0);
            _fitAxesCheckbox->labelsize(11);
            _fitAxesCheckbox->color2(FL_RED);
            _fitAxesCheckbox->callback(_fixAxesCB_static, this);
            _fitAxesCheckbox->when(FL_WHEN_CHANGED);
            _fitAxesCheckbox->value((bool)_fittoaxes ? 1 : 0);

            auto textH1 = new Fl_Box(5, 45, 80, 15, "hor. spacing:");
            textH1->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT));
            textH1->labelfont(0);
            textH1->labelsize(11);

            _hspaceInput = new Fl_Input(88,45,55,15);
            _hspaceInput->textsize(11);
            _hspaceInput->box(FL_BORDER_BOX);
            _hspaceInput->callback(_hspaceCB_static, this);
            _hspaceInput->when(FL_WHEN_ENTER_KEY|FL_WHEN_RELEASE);

            auto textH2 = new Fl_Box(156, 45, 40, 15, "offset:");
            textH2->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT));
            textH2->labelfont(0);
            textH2->labelsize(11);

            _hoffsetInput = new Fl_Input(200, 45, 55, 15);
            _hoffsetInput->textsize(11);
            _hoffsetInput->box(FL_BORDER_BOX);
            _hoffsetInput->callback(_hoffsetCB_static, this);
            _hoffsetInput->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);

            auto textV1 = new Fl_Box(5, 65, 80, 15, "ver. spacing:");
            textV1->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT));
            textV1->labelfont(0);
            textV1->labelsize(11);

            _vspaceInput = new Fl_Input(88, 65, 55, 15);
            _vspaceInput->textsize(11);
            _vspaceInput->box(FL_BORDER_BOX);
            _vspaceInput->callback(_vspaceCB_static, this);
            _vspaceInput->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);

            auto textV2 = new Fl_Box(156, 65, 40, 15, "offset:");
            textV2->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT));
            textV2->labelfont(0);
            textV2->labelsize(11);

            _voffsetInput = new Fl_Input(200, 65, 55, 15);
            _voffsetInput->textsize(11);
            _voffsetInput->box(FL_BORDER_BOX);
            _voffsetInput->callback(_voffsetCB_static, this);
            _voffsetInput->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);

            optionWin->end();

            _fixOffset();
            _updateWidgets();

            return this;

        }


        void Plot2DGrid::_fixOffset()
            {
            if (_hspace > 0.0)
               {
               _hoffset = std::remainder(_hoffset, _hspace);
               if (_hoffset < 0.0) { _hoffset = _hoffset + _hspace; }
               }
            if (_vspace > 0.0)
                {
                _voffset = std::remainder(_voffset, _vspace);
                if (_voffset < 0.0) { _voffset = _voffset + _vspace; }
                }
            }


        void Plot2DGrid::_updateWidgets()
            {
            if (_win == nullptr) return; // nothing to do
            if (((bool)_fittoaxes) == true)
                {
                _fitAxesCheckbox->value(1);
                _fitToAxes();
                _vspaceInput->deactivate();
                _hspaceInput->deactivate();
                _voffsetInput->deactivate();
                _hoffsetInput->deactivate();
                }
            else
                {
                _fitAxesCheckbox->value(0);
                _vspaceInput->activate();
                _hspaceInput->activate();
                _voffsetInput->activate();
                _hoffsetInput->activate();
                }

            if (_vspace <= 0.0)
                {
                _vspaceInput->value("none");
                _voffsetInput->value("none");
                }
            else
                {
                _vspaceInput->value(doubleToStringNice(_vspace).c_str());
                _voffsetInput->value(doubleToStringNice(_voffset).c_str());
                }
            if (_hspace <= 0.0)
                {
                _hspaceInput->value("none");
                _hoffsetInput->value("none");
                }
            else
                {
                _hspaceInput->value(doubleToStringNice(_hspace).c_str());
                _hoffsetInput->value(doubleToStringNice(_hoffset).c_str());
                }
            _colorButton->color((Fl_Color)((RGBc)_color));
            _colorButton->color2((Fl_Color)((RGBc)_color));
            _colorButton->redraw();
            }



        /* callbacks for the fltk widgets */

        void Plot2DGrid::_hspaceCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_hspaceCB(W); }
        void Plot2DGrid::_hspaceCB(Fl_Widget * W)
            {
            double v = -1.0;
            mtools::fromString(((Fl_Input*)W)->value(), v);
            horizontalSpacing(v);
            yieldFocus();
            }

        void Plot2DGrid::_vspaceCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_vspaceCB(W); }
        void Plot2DGrid::_vspaceCB(Fl_Widget * W)
            {
            double v = -1.0;
            mtools::fromString(((Fl_Input*)W)->value(), v);
            verticalSpacing(v);
            yieldFocus();
        }

        void Plot2DGrid::_hoffsetCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_hoffsetCB(W); }
        void Plot2DGrid::_hoffsetCB(Fl_Widget * W)
            {
            double v = 0.0;
            mtools::fromString(((Fl_Input*)W)->value(), v);
            horizontalOffset(v);
            yieldFocus();
        }

        void Plot2DGrid::_voffsetCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_voffsetCB(W); }
        void Plot2DGrid::_voffsetCB(Fl_Widget * W)
            {
            double v = 0.0;
            mtools::fromString(((Fl_Input*)W)->value(), v);
            verticalOffset(v);
            yieldFocus();
            }

        void Plot2DGrid::_colorCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_colorCB(W); }
        void Plot2DGrid::_colorCB(Fl_Widget * W)
            {
            RGBc coul = _color;
            if (fl_color_chooser("Axes Color", coul.comp.R, coul.comp.G, coul.comp.B, 1) != 0)
                {
                color(coul);
                }
            yieldFocus();
            }


        void Plot2DGrid::_unitGridCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_unitGridCB(W); }
        void Plot2DGrid::_unitGridCB(Fl_Widget * W)
            {
            _fittoaxes = false;
            setUnitGrid();
            yieldFocus();
            }


        void Plot2DGrid::_unitCellsCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_unitCellsCB(W); }
        void Plot2DGrid::_unitCellsCB(Fl_Widget * W)
            {
            _fittoaxes = false;
            setUnitCells();
            yieldFocus();
            }


        void Plot2DGrid::_fixAxesCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DGrid*)data)->_fixAxesCB(W); }
        void Plot2DGrid::_fixAxesCB(Fl_Widget * W)
            {
            fitToAxes(_fitAxesCheckbox->value() == 0 ? false : true);
            yieldFocus();
            }


}
/* end of file */
