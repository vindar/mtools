/** @file plotter2Dobj.cpp */
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

#include "graphics/plotter2Dobj.hpp"
#include "misc/error.hpp"
#include "misc/indirectcall.hpp"
#include "io/fltkSupervisor.hpp"
#include "graphics/customcimg.hpp"
#include "graphics/drawable2Dobject.hpp"
#include "graphics/rangemanager.hpp"


namespace mtools
{


    namespace internals_graphics
    {

        std::atomic<int> Plotter2DObj::_totPlotNB((int)0); // init of the static variable.


        Plotter2DObj::Plotter2DObj(const std::string & name) :
            _crange(fBox2()),
            _cwinSize(iVec2()),
            _missedSetParam(false),
            _ownercb(nullptr),
            _data(nullptr),
            _data2(nullptr),
            _rm(nullptr),
            _AD(nullptr),
            _opacity(1.0),
            _drawOn(true),
            _suspended(false),
            _name(name),
            _optionWin(nullptr),
            _extOptionWin(nullptr),
            _plotNB((int)_totPlotNB)
            {
            _totPlotNB++;
            }



        Plotter2DObj::~Plotter2DObj()
            {
            detach(); // just in case, won't do much good but anyway...
            }


        Plotter2DObj::Plotter2DObj(Plotter2DObj && obj) :
            _crange(fBox2()),
            _cwinSize(iVec2()),
            _missedSetParam(false),
            _ownercb(nullptr),
            _data(nullptr),
            _data2(nullptr),
            _rm(nullptr),
            _AD(nullptr),
            _opacity(1.0),
            _drawOn(true),
            _suspended(false),
            _name(std::move(obj._name)),
            _optionWin(nullptr),
            _extOptionWin(nullptr),
            _plotNB((int)_totPlotNB)
                {
                if (obj.isInserted()) MTOOLS_DEBUG("move ctor on an inserted plotter2DObj");
                obj.detach(); // detach if needed
                }


        int Plotter2DObj::ID() const { return _plotNB;  }


        std::string Plotter2DObj::name() const
            {
            return _name;
            }


        void Plotter2DObj::name(const std::string & newname)
            {
            if (!isFltkThread()) // we need to run the method in FLTK
                {
                IndirectMemberProc<Plotter2DObj, const std::string & > proxy(*this, &Plotter2DObj::name, newname); // registers the call
                runInFltkThread(proxy);
                return;
                }
            _name = newname;
            if ((pnot)_ownercb == nullptr) return;
            _nameBox->copy_label(_name.c_str());
            _nameBox->redraw();
            }


        float Plotter2DObj::opacity() const
            {
            return _opacity;
            }


        void Plotter2DObj::opacity(float op)
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
                {
                IndirectMemberProc<Plotter2DObj,float> proxy(*this, &Plotter2DObj::opacity, op); // registers the call
                runInFltkThread(proxy);
                return;
                }
            if (op <= 0.0) op = 0.0; else if (op >= 1.0) op = 1.0;
            _opacity = op;
            if ((pnot)_ownercb == nullptr) return;  // return if not inserted
            _opacitySlider->value(op);
            refresh();
            }


        bool Plotter2DObj::enable() const { return _drawOn; }


        void Plotter2DObj::enable(bool status)
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
              {
              IndirectMemberProc<Plotter2DObj, bool> proxy(*this, &Plotter2DObj::enable, status); // registers the call
              runInFltkThread(proxy);
              return;
              }
            _drawOn = status;
            _suspended = (_drawOn) ? false : true; // override the suspended flag
            if ((pnot)_ownercb == nullptr) return;  // return if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _onOffButton->value(_drawOn);
            if (_drawOn)
                {
                if (_missedSetParam)
                    {
                    ((AutoDrawable2DObject*)_AD)->setParam(_crange, _cwinSize);
                    _missedSetParam = false;
                    }
                _nameBox->activate();
                if (!hasFavouriteRangeX()) { _useRangeX->deactivate(); } else { _useRangeX->activate(); }
                if (!hasFavouriteRangeY()) { _useRangeY->deactivate(); } else { _useRangeY->activate(); }
                if (!((hasFavouriteRangeX())&&(hasFavouriteRangeY()))) { _useRangeXY->deactivate(); } else { _useRangeXY->activate(); }
                _opacitySlider->activate();
                if (_progBar != nullptr) _progBar->activate();
                if (_optionWin != nullptr) _optionWin->activate();
                }
            else
                {
                _nameBox->deactivate();
                _useRangeX->deactivate();
                _useRangeY->deactivate();
                _useRangeXY->deactivate();
                _opacitySlider->deactivate();
                if (_progBar != nullptr) _progBar->deactivate();
                if (_optionWin != nullptr) _optionWin->deactivate();
                }
            ((AutoDrawable2DObject*)_AD)->workThread(_drawOn);
            refresh();
            yieldFocus();
            }


        bool Plotter2DObj::suspend() const { return _suspended; }


        void Plotter2DObj::suspend(bool status)
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
              {
              IndirectMemberProc<Plotter2DObj,bool> proxy(*this, &Plotter2DObj::suspend,status); // registers the call
              runInFltkThread(proxy);
              return;
              }
            if ((_drawOn == false)||(_suspended == status)) return; // nothing to do
            _suspended = status;
            if ((pnot)_ownercb == nullptr) return;  // return if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            ((AutoDrawable2DObject*)_AD)->workThread(!status);
            if (!status)
                { // we resume activites
                if (_missedSetParam)
                    { 
                    ((AutoDrawable2DObject*)_AD)->setParam(_crange, _cwinSize);
                    _missedSetParam = false;
                    }
                refresh();
                yieldFocus();
                }
            }


        void Plotter2DObj::moveUp()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_UP);
            }


        void Plotter2DObj::moveDown()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_DOWN);
            }


        void Plotter2DObj::moveTop()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_TOP);
            }


        void Plotter2DObj::moveBottom()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_BOTTOM);
            }


        void Plotter2DObj::autorangeX()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            if (_suspended) return; // or not enabled
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_USERANGEX);
            }

        void Plotter2DObj::autorangeY()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            if (_suspended) return; // or not enabled
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_USERANGEY);
            }

        void Plotter2DObj::autorangeXY()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            if (_suspended) return; // or not enabled
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_USERANGEXY);
            }


        fBox2 Plotter2DObj::favouriteRangeX(fBox2 R)
            {
            return fBox2(); // completely empty rectangle.
            }


        fBox2 Plotter2DObj::favouriteRangeY(fBox2 R)
            {
            return fBox2(); // completely empty rectangle.
            }


        bool Plotter2DObj::hasFavouriteRangeX() { return false; }


        bool Plotter2DObj::hasFavouriteRangeY() { return false; }


        void Plotter2DObj::refresh()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _makecallback(_REQUEST_REFRESH);
            }


        int Plotter2DObj::quality() const
            {
            if ((pnot)_ownercb == nullptr) return 0;  // 0 if not inserted
            if (!_drawOn) return 100; // and 100 if not enabled.
            if (_suspended) return 0; // and 0 if enabled but suspended
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            return(((AutoDrawable2DObject*)_AD)->quality());
            }


        bool Plotter2DObj::needWork() const
            {
            if ((pnot)_ownercb == nullptr) return false;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            return ((AutoDrawable2DObject*)_AD)->needWork();
            }


        void Plotter2DObj::resetDrawing(bool refresh)
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            if (_suspended) return; // or suspended
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            ((AutoDrawable2DObject*)_AD)->resetDrawing(); // reset the drawing
            if (refresh) _makecallback(_REQUEST_REFRESH);
            }


        bool Plotter2DObj::isInserted() const
            {
            return ((pnot)_ownercb != nullptr);
            }


        int Plotter2DObj::drawOnto(cimg_library::CImg<unsigned char> & im)
            {
            if ((pnot)_ownercb == nullptr) return 0;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            if (!_drawOn) return 100; // 100 if disabled
            if (_suspended) return 0; // 0 if suspended
            return ((AutoDrawable2DObject*)_AD)->drawOnto(im,_opacity); // reset the drawing
            }


        void Plotter2DObj::setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr);
            _crange = range; // save the range;
            _cwinSize = imageSize; // save the image size
            if (_suspended) { _missedSetParam = true; return; } // disabled so we do not call the underlying object but we remember to update the range when enabled.
            _missedSetParam = false;
            ((AutoDrawable2DObject*)_AD)->setParam(range,imageSize); // set the parameters
            }


        /* to override */
        Drawable2DObject * Plotter2DObj::inserted(Fl_Group * & optionWin, int reqWidth)
            {
            MTOOLS_ERROR("Plotter2DObj::inserted must be overriden !"); return nullptr;
            }


        /* default version */
        void Plotter2DObj::removed(Fl_Group * optionWin)
            {
            if (optionWin != nullptr) { Fl::delete_widget(optionWin); } // default behaviour : mark widget for deletion.
            }


        void Plotter2DObj::colorCB(Fl_Widget * W) { return; }


        RGBc Plotter2DObj::nameWidgetColor() const { return RGBc::c_TransparentWhite; }


        void Plotter2DObj::setNameWidgetColor()
            {
            if (!isFltkThread()) // run the method in FLTK if not in it
                {
                IndirectMemberProc<Plotter2DObj> proxy(*this, &Plotter2DObj::setNameWidgetColor); // registers the call
                runInFltkThread(proxy);
                return;
                }
            if ((pnot)_ownercb == nullptr) return;  // return if not inserted
            RGBc coul = nameWidgetColor();
            if (coul == RGBc::c_TransparentWhite) { return; }
            _nameBox->color(coul);
            if ((int)coul.R + (int)coul.G + (int)coul.B < 300)
                {
                _nameBox->labelcolor(RGBc(230, 230, 230));
                }
            else
                {
                _nameBox->labelcolor(RGBc::c_Black);
                }
            _nameBox->redraw();
            refresh();
            yieldFocus();
            }


        void Plotter2DObj::detach()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            _makecallback(_REQUEST_DETACH);
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) == nullptr); // did we really get our call to  _removed() ?
            MTOOLS_ASSERT(((pnot)_ownercb) == nullptr);             // if so, these should already be set to nullptr.
            _AD = nullptr;
            _ownercb = nullptr;
            _data = nullptr;
            _data2 = nullptr;
            _rm = nullptr;
            _optionWin = nullptr;
            _extOptionWin = nullptr;
            }


        void Plotter2DObj::yieldFocus()
            {
            if ((pnot)_ownercb == nullptr) return;  // do nothing if not inserted
            _makecallback(_REQUEST_YIELDFOCUS);
            }


        const RangeManager * Plotter2DObj::range() const
            {
            if ((pnot)_ownercb == nullptr) return nullptr;  // do nothing if not inserted
            return _rm;
            }


        /* called when we are inserted */
        void Plotter2DObj::_inserted(pnot cb, RangeManager * rm, void * data, void * data2, int hintWidth)
        {
            MTOOLS_ASSERT(isFltkThread()); // the owner should be calling from the fltk thread !
            if (((pnot)_ownercb) != nullptr) // we are already inserted, detach before going further
               {
               MTOOLS_DEBUG("Plotter2DObj::_inserted, already inserted, we detach before going any further");
               MTOOLS_ASSERT(((pnot)_ownercb) != cb); // make sure it is a different owner.
               detach(); // ok, we detach from the previous owner.
               }
            _ownercb = cb; //save the callback
            _data = data;  //save the data
            _data2 = data2;// and the additional data
            _rm = rm;      //save the range manager
            Fl_Group::current(0); // prevent adding to option window to some other window.
            _extOptionWin = nullptr;
            Drawable2DObject * D = inserted(_optionWin, hintWidth); // get the drawable object and the option window
            Fl_Group::current(0); // close the FL_group if needed.
            MTOOLS_ASSERT(D != nullptr); // the drawable object should exist.
            _AD = new AutoDrawable2DObject(D,false); // create the auto drawable object, we do not start the thread yet so there is no access for the moment.

            /* we create the window */
            int ow = (_optionWin == nullptr) ? hintWidth : _optionWin->w();

            // extended option window
            _extOptionWin = new Fl_Group(0, 0, ow,40); // create the extended group
            _extOptionWin->box(FL_UP_BOX);

            _titleBox = new Fl_Box(0, 0, ow, 40);
            _titleBox->color(fl_lighter(FL_BACKGROUND_COLOR));
            _titleBox->box(FL_UP_BOX);

            _nameBox = new Fl_Button(55, 0, ow - 55, 20, _name.c_str());
            _nameBox->color(fl_lighter(FL_BACKGROUND_COLOR));
            _nameBox->labelcolor(FL_BLACK);
            _nameBox->box(FL_UP_BOX);
            _nameBox->labelfont(0);
            _nameBox->labelsize(16);
            _nameBox->callback(_nameColorCB_static, this);
            setNameWidgetColor();

            _upButton = new Fl_Button(0, 0, 20, 20, "@#8->");
            _upButton->labelcolor(FL_BLACK);
            _upButton->box(FL_UP_BOX);
            _upButton->callback(_upButtonCB_static, this);

            _downButton = new Fl_Button(20, 0, 20, 20, "@#2->");
            _downButton->labelcolor(FL_BLACK);
            _downButton->box(FL_UP_BOX);
            _downButton->callback(_downButtonCB_static, this);

            _onOffButton = new Fl_Light_Button(40, 0, 15, 20);
            _onOffButton->color2(FL_RED);
            _onOffButton->box(FL_UP_BOX);
            _onOffButton->value(1);
            _onOffButton->callback(_onOffButtonCB_static, this);

            _useRangeX = new Fl_Button(2, 22, 15, 16, "X");
            _useRangeX->color(fl_lighter(FL_BACKGROUND_COLOR));
            _useRangeX->labelcolor(FL_BLACK);
            _useRangeX->box(FL_UP_BOX);
            _useRangeX->labelfont(10);
            _useRangeX->labelsize(10);
            _useRangeX->callback(_useRangeXCB_static, this);
            if (!hasFavouriteRangeX()) { _useRangeX->deactivate(); }

            _useRangeY = new Fl_Button(2 + 15, 22, 15, 16, "Y");
            _useRangeY->color(fl_lighter(FL_BACKGROUND_COLOR));
            _useRangeY->labelcolor(FL_BLACK);
            _useRangeY->box(FL_UP_BOX);
            _useRangeY->labelfont(10);
            _useRangeY->labelsize(10);
            _useRangeY->callback(_useRangeYCB_static, this);
            if (!hasFavouriteRangeY()) { _useRangeY->deactivate(); }

            _useRangeXY = new Fl_Button(2 + 30, 22, 23, 16, "X/Y");
            _useRangeXY->color(fl_lighter(FL_BACKGROUND_COLOR));
            _useRangeXY->labelcolor(FL_BLACK);
            _useRangeXY->box(FL_UP_BOX);
            _useRangeXY->labelfont(10);
            _useRangeXY->labelsize(10);
            _useRangeXY->callback(_useRangeXYCB_static, this);
            if (!(hasFavouriteRangeX() && hasFavouriteRangeY())) { _useRangeXY->deactivate(); }

            _opacitySlider = new Fl_Value_Slider(60, 23, ow - 105 - 65, 14);
            _opacitySlider->labelfont(0);
            _opacitySlider->labelsize(11);
            _opacitySlider->color(fl_lighter(FL_BACKGROUND_COLOR));
            _opacitySlider->align(Fl_Align(FL_ALIGN_LEFT));
            _opacitySlider->box(FL_FLAT_BOX);
            _opacitySlider->type(FL_HOR_NICE_SLIDER);
            _opacitySlider->range(0, 1.0);
            _opacitySlider->step(0.01);
            _opacitySlider->value(_opacity);
            _opacitySlider->color2(FL_RED);
            _opacitySlider->callback(_opacitySliderCB_static, this);

            _progBar = nullptr;
            if (((AutoDrawable2DObject*)_AD)->needWork()) // check if the object need work
                {
                _progBar = new Fl_Progress(ow - 105, 22, 84, 16);
                _progBar->minimum(0.0);
                _progBar->maximum(100.0);
                _progBar->value(100.0);
                _progBar->color(fl_darker(FL_GRAY));
                _progBar->labelcolor(FL_WHITE);
                _progBar->selection_color(FL_RED);
                _progBar->labelsize(11);
                _progVal = -1;
                _progBar->copy_label("stopped");
                // and add the timer
                Fl::add_timeout(0.1, _timerCB_static, this);
                }

            _unrollButton = new Fl_Button(ow - 20, 22, 16, 16, "@-42>>");
            _unrollButton->labelcolor(fl_lighter(FL_BLACK));
            _unrollButton->box(FL_UP_BOX);
            _unrollButton->callback(_unrollButtonCB_static, this);
            if (_optionWin == nullptr) { _unrollButton->deactivate(); }

            _extOptionWin->end();
            _extOptionWin->resizable(0);

            if (!_drawOn) // deactivate widget if object not enabled
                {
                _nameBox->deactivate();
                _useRangeX->deactivate();
                _useRangeY->deactivate();
                _useRangeXY->deactivate();
                _opacitySlider->deactivate();
                if (_progBar != nullptr) _progBar->deactivate();
                if (_optionWin != nullptr) _optionWin->deactivate();
                }

            setParam(rm->getRange(), rm->getWinSize()); // save the parameters for the range of the drawing which will be applied when we enable the object
            enable(_drawOn);                            // so maybe right now, or later on...
            return;
            }


        /* test whether the option win is inside the extOption Window.*/
        bool Plotter2DObj::_unrolled() const
            {
            if (_optionWin == nullptr) return false;
            return (_extOptionWin->find(_optionWin) != _extOptionWin->children());
            }


        /* insert/remove the addtional option from the extended option window */
        void Plotter2DObj::_insertOptionWin(bool status)
            {
            if (_optionWin == nullptr) return;
            if (status == true)
                {
                if (_unrolled()) return;
                _extOptionWin->resize(_extOptionWin->x(), _extOptionWin->y(), _optionWin->w(), _optionWin->h() + 40); // resize to the right size
                _extOptionWin->add(_optionWin);  // add the custom option window
                _optionWin->resize(_extOptionWin->x(), _extOptionWin->y() + 40, _optionWin->w(), _optionWin->h()); // move it at the right place
                return;
                }
            if (!_unrolled()) { return; }
            _extOptionWin->remove(_optionWin);  // remove the custom option window
            _extOptionWin->resize(_extOptionWin->x(), _extOptionWin->y(), _optionWin->w(), 40); // resize to the right size
            return;
            }


        /* return the objct option window */
        Fl_Group * Plotter2DObj::_optionWindow() const
            {
            if ((pnot)_ownercb == nullptr) return nullptr;
            return _extOptionWin;
            }


        /* called by the owner when detached */
        void Plotter2DObj::_removed()
            {
            MTOOLS_ASSERT(isFltkThread()); // we should be in the fltk thread
            MTOOLS_ASSERT(((pnot)_ownercb) != nullptr); // check that we are indeed inserted.
            MTOOLS_ASSERT(((Fl_Group *)_extOptionWin) != nullptr); // cannnot be null
            Fl::remove_timeout(_timerCB_static, this); // remove the timer for the progress bar if there is one.
            delete ((AutoDrawable2DObject*)_AD); // delete the auto drawer, this stops the worker thread if there is one.
            _AD = nullptr;
            _insertOptionWin(false); // make the optionWin group stand alone.
            removed(_optionWin); // call the virtual function that takes care of deleting _optionWin
            _optionWin = nullptr; // mark as deleted
            Fl::delete_widget(_extOptionWin); // send widget deletion request
            _extOptionWin = nullptr; // mark as deleted.
            _rm = nullptr;          // remove obsolete info about the previous owner.
            _data = nullptr;        //
            _data2 = nullptr;       //
            _ownercb = nullptr;     // officially not inserted anymore
            }


        void Plotter2DObj::_makecallback(int code)
            {
            if (((pnot)_ownercb) == nullptr) return; // does nothing if not inserted.
            if (isFltkThread())
                {
                ((pnot)_ownercb)(_data,_data2, this, code); // inside FLTK, we call directly
                }
            else
                {
                IndirectProc<void *, void *, void *, int> proxy((pnot)_ownercb, (void *)_data, (void*)_data2, (void*)this, code); // registers the call
                runInFltkThread(proxy); // make the call to the owner indicating we are de-inserting ourselves.
                }
            }




        void Plotter2DObj::_upButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_upButtonCB(W); }
        void Plotter2DObj::_upButtonCB(Fl_Widget * W)
            {
            moveUp();
            }


        void Plotter2DObj::_downButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_downButtonCB(W); }
        void Plotter2DObj::_downButtonCB(Fl_Widget * W)
            {
            moveDown();
            }


        void Plotter2DObj::_useRangeXCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_useRangeXCB(W); }
        void Plotter2DObj::_useRangeXCB(Fl_Widget * W)
            {
            autorangeX();
            }

        void Plotter2DObj::_useRangeYCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_useRangeYCB(W); }
        void Plotter2DObj::_useRangeYCB(Fl_Widget * W)
            {
            autorangeY();
            }

        void Plotter2DObj::_useRangeXYCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_useRangeXYCB(W); }
        void Plotter2DObj::_useRangeXYCB(Fl_Widget * W)
            {
            autorangeXY();
            }



        void Plotter2DObj::_onOffButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_onOffButtonCB(W); }
        void Plotter2DObj::_onOffButtonCB(Fl_Widget * W)
            {
            enable((_onOffButton->value() == 0) ? false : true);
            }


        void Plotter2DObj::_opacitySliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_opacitySliderCB(W); }
        void Plotter2DObj::_opacitySliderCB(Fl_Widget * W)
            {
            opacity((float)_opacitySlider->value());
            }


        void Plotter2DObj::_nameColorCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_nameColorCB(W); }
        void Plotter2DObj::_nameColorCB(Fl_Widget * W)
            {
            colorCB(W); // call the virtual method.
            }


        void Plotter2DObj::_unrollButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_unrollButtonCB(W); }
        void Plotter2DObj::_unrollButtonCB(Fl_Widget * W)
            {
            _insertOptionWin(!_unrolled()); // roll / unroll the option window
            _extOptionWin->redraw(); // mark for redrawing
            if (_unrolled()) { _unrollButton->label("@-48>>"); } else { _unrollButton->label("@-42>>"); }
            _makecallback(_REQUEST_FIXOBJECTWIN); // request the whole scroll window with the object to be redrawn
            }


        /* timer callback update the progress for for the quality */
        void Plotter2DObj::_timerCB_static(void * data) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DObj*)data)->_timerCB(); }
        void Plotter2DObj::_timerCB()
            {
            MTOOLS_ASSERT(((AutoDrawable2DObject*)_AD) != nullptr); // check that the auto drawer still exist
            bool thron = ((AutoDrawable2DObject*)_AD)->workThread(); // thread status
            if (thron == 0) // thread is off
                {
                if (_progVal != -1)
                    {
                    _progVal = -1;
                    _progBar->selection_color(FL_DARK_RED);
                    _progBar->labelsize(11);
                    _progBar->value(100);
                    _progBar->copy_label("stopped");
                    _progBar->redraw();
                    }
                }
            else
                {
                int q = ((AutoDrawable2DObject*)_AD)->quality();
                if (_progVal != q)
                    {
                    _progVal = q;
                    _progBar->selection_color((q<100) ? FL_DARK_BLUE : FL_DARK_GREEN);
                    _progBar->labelsize(11);
                    _progBar->value((float)q);
                    auto progText = mtools::toString(q) + "%";
                    _progBar->copy_label(progText.c_str());
                    _progBar->redraw();
                    }
                }
            Fl::repeat_timeout(0.1, _timerCB_static, this); // repeat the timer
            }











        /**********************************************************************************************************************************************************************************************/


        std::atomic<int> Plotter2DObjWithColor::_noColorPlot((int)0);


        Plotter2DObjWithColor::Plotter2DObjWithColor(const std::string & name) : Plotter2DObj(name), _color(RGBc::c_Black), _no((int)_noColorPlot)
            {
            _color = RGBc::getDistinctColor((int)_no % 32);
            ++_noColorPlot;
            }


        Plotter2DObjWithColor::~Plotter2DObjWithColor()
           {
            detach(); // just in case, won't do much good but anyway...
            }


        Plotter2DObjWithColor::Plotter2DObjWithColor(Plotter2DObjWithColor && obj) : Plotter2DObj(std::move(obj)), _color((RGBc)obj._color), _no((int)obj._no)
            {
            }


        RGBc Plotter2DObjWithColor::color() const { return _color; }


        void Plotter2DObjWithColor::color(RGBc coul)
            {
            _color = coul;
            setNameWidgetColor();
            }


        void Plotter2DObjWithColor::colorCB(Fl_Widget * W)
            {
            RGBc coul = _color;
            if (fl_color_chooser("Plot color", coul.R, coul.G, coul.B, 1) != 0) { color(coul); } else { yieldFocus(); }
            }


        RGBc Plotter2DObjWithColor::nameWidgetColor() const
            {
            return _color;
            }











    }


}

/* end of file */


