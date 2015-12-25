/** @file plot2Dcimg.cpp */
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

#include "graphics/plot2Dcimg.hpp"


namespace mtools
{


    Plot2DCImg makePlot2DCImg(CImg<unsigned char> & im, std::string name)
        {
        return Plot2DCImg(im, name);
        }


    Plot2DCImg makePlot2DCImg(CImg<unsigned char> * im, std::string name)
        {
        return Plot2DCImg(im, name);
        }


    Plot2DCImg::Plot2DCImg(CImg<unsigned char> * im, std::string name) : internals_graphics::Plotter2DObj(name), _typepos(TYPEBOTTOMLEFT), _im(im)
        {
        _LD = new LatticeDrawer<Plot2DCImg>(this);
        _setDomain();
        }

    Plot2DCImg::Plot2DCImg(CImg<unsigned char> & im, std::string name) : internals_graphics::Plotter2DObj(name), _typepos(TYPEBOTTOMLEFT), _im(&im)
        {
        _LD = new LatticeDrawer<Plot2DCImg>(this);
        _setDomain();
        }


    Plot2DCImg::Plot2DCImg(Plot2DCImg && o) : internals_graphics::Plotter2DObj(std::move(o)), _typepos((int)o._typepos), _im(o._im)
        {
        _LD = new LatticeDrawer<Plot2DCImg>(this);
        _setDomain();
        }


    Plot2DCImg::~Plot2DCImg()
        {
        detach(); // detach from its owner if there is still one.
        delete _LD; // remove the lattice drawer
        _im = nullptr;
        }


    void Plot2DCImg::image(CImg<unsigned char> * im)
        {
        enable(false);
        _im = im;
        _setDomain();
        enable(true);
        resetDrawing();
        }


    void Plot2DCImg::image(CImg<unsigned char> & im)
        {
        enable(false);
        _im = &im;
        _setDomain();
        enable(true);
        resetDrawing();
        }


    CImg<unsigned char> * Plot2DCImg::image() const { return _im; }


    void Plot2DCImg::position(int posType)
        {
        if (((posType != TYPECENTER) && (posType != TYPEBOTTOMLEFT)) || (posType == _typepos)) return; // nothing to do
        _typepos = posType;
        _setDomain();
        if (isInserted())
            {
            IndirectMemberProc<Plot2DCImg> proxy(*this, &Plot2DCImg::_updatePosTypeInFLTK); // update the status of the button in the fltk thread
            runInFLTKThread(proxy); // and also refresh the drawing if needed
            resetDrawing();
            }
        }


    int Plot2DCImg::position() const
        {
        return _typepos;
        }


    fBox2 Plot2DCImg::favouriteRangeX(fBox2 R)
        {
        if ((_LD->isDomainEmpty() || _LD->isDomainFull())) return fBox2(); // no favourite range
        iBox2 D = _LD->domain();
        return fBox2((double)D.min[0] - 0.5, (double)D.max[0] + 0.5, (double)D.min[1] - 0.5, (double)D.max[1] + 0.5);
        }


    fBox2 Plot2DCImg::favouriteRangeY(fBox2 R)
        {
        if ((_LD->isDomainEmpty() || _LD->isDomainFull())) return fBox2(); // no favourite range
        iBox2 D = _LD->domain();
        return fBox2((double)D.min[0] - 0.5, (double)D.max[0] + 0.5, (double)D.min[1] - 0.5, (double)D.max[1] + 0.5);
        }


    bool Plot2DCImg::hasFavouriteRangeX()
        {
        return(!(_LD->isDomainEmpty() || _LD->isDomainFull())); // there is a favourite range if the domain is neither empy nor full
        }


    bool Plot2DCImg::hasFavouriteRangeY()
        {
        return(!(_LD->isDomainEmpty() || _LD->isDomainFull())); // there is a favourite range if the domain is neither empy nor full
        }


    void Plot2DCImg::removed(Fl_Group * optionWin)
        {
        Fl::delete_widget(optionWin);
        _checkButtonCenter = nullptr;
        _checkButtonBottomLeft = nullptr;
        }


    internals_graphics::Drawable2DObject * Plot2DCImg::inserted(Fl_Group * & optionWin, int reqWidth)
        {
        /* create the option window */
        optionWin = new Fl_Group(0, 0, reqWidth, 60); // create the option group
        _checkButtonCenter = new Fl_Round_Button(15, 10, reqWidth - 20, 15, "Origin at the center.");
        _checkButtonCenter->labelfont(0);
        _checkButtonCenter->labelsize(11);
        _checkButtonCenter->color2(FL_RED);
        _checkButtonCenter->type(102);
        _checkButtonCenter->callback(_roundButtonCB_static, this);
        _checkButtonCenter->when(FL_WHEN_CHANGED);
        _checkButtonBottomLeft = new Fl_Round_Button(15, 35, reqWidth - 20, 15, "Origin at the bottom left corner.");
        _checkButtonBottomLeft->labelfont(0);
        _checkButtonBottomLeft->labelsize(11);
        _checkButtonBottomLeft->color2(FL_RED);
        _checkButtonBottomLeft->type(102);
        _checkButtonBottomLeft->callback(_roundButtonCB_static, this);
        _checkButtonBottomLeft->when(FL_WHEN_CHANGED);
        if (_typepos == TYPECENTER) { _checkButtonCenter->setonly(); }
        else { _checkButtonBottomLeft->setonly(); }
        optionWin->end();
        return _LD;
        }


    void Plot2DCImg::_setDomain()
        {
        if (_im == nullptr) { _LD->domainEmpty(); return; }
        if (_typepos == TYPECENTER)
            {
            _LD->domain(iBox2(-_im->width() / 2, _im->width() - 1 - _im->width() / 2, _im->height() / 2 - _im->height(), _im->height() / 2 - 1)); return;
            }
        _LD->domain(iBox2(0, _im->width() - 1, 0, _im->height() - 1));
        }


    void Plot2DCImg::_updatePosTypeInFLTK()
        {
        if (_typepos == TYPECENTER) { _checkButtonCenter->setonly(); }
        else { _checkButtonBottomLeft->setonly(); }
        }


    void Plot2DCImg::_roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DCImg*)data)->_roundButtonCB(W); }
    void Plot2DCImg::_roundButtonCB(Fl_Widget * W)
        {
        if (W == _checkButtonCenter) { _typepos = TYPECENTER; }
        else { _typepos = TYPEBOTTOMLEFT; }
        _setDomain();
        resetDrawing();
        }


}
/* end of file */

