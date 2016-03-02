/** @file watch.cpp */
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

#include "io/watch.hpp"
#include "misc/timefct.hpp"
#include "misc/stringfct.hpp"
#include "misc/indirectcall.hpp"
#include "io/fltkSupervisor.hpp"
#include "graphics/rgbc.hpp"

#include <time.h>

namespace mtools
{

    namespace internals_watch
        {

        /* Implementation of WatchObj */

        WatchObj::WatchObj(const std::string & name, int rate) : _rate(rate) , _fltkwin(nullptr), _name_button(nullptr), _value_button(nullptr),_name(name) {}

        WatchObj::~WatchObj() { }

        std::string WatchObj::get() const
            {
            return _get();
            }

        void WatchObj::set(const std::string & value)
            {
            _set(value);
            }

        
        std::string WatchObj::type() const { MTOOLS_ERROR("pure virtual method WatchObj::type(), access forbidden"); return std::string(); }

        std::string WatchObj::_get() const { MTOOLS_ERROR("pure virtual method WatchObj::get(), access forbidden"); return std::string(); }

        size_t WatchObj::_set(const std::string & value) { MTOOLS_ERROR("pure virtual method WatchObj::set(), access forbidden"); return 0; }

        bool WatchObj::writable() const { MTOOLS_ERROR("pure virtual method WatchObj::writable(), access forbidden"); return false; }

        int WatchObj::refreshRate() const { return _rate; }

        int WatchObj::refreshRate(int newrate) { if (newrate < 0) { newrate = 0; } else if (newrate > 600) { newrate = 600; } _rate = newrate; return _rate; }

        void WatchObj::assignFltkWin(FltkWatchWin * p, Fl_Button * nameButton, Fl_Button * valueButton) { _fltkwin = p;  _name_button = nameButton; _value_button = valueButton; }

        /* fltk watch window */
        class FltkWatchWin
            {

            public:


                /** Default constructor. Create and show the watch window */
                FltkWatchWin(const std::string & name, int posX, int posY) :
                    mapspied(), _initTime(0), win(nullptr), tileWin(nullptr), nameCol(nullptr), valueCol(nullptr), downBox(nullptr), dialogWin(nullptr), button_Ok(nullptr), button_Cancel(nullptr), input_widget(nullptr), slider_widget(nullptr), rate_box(nullptr)
                    {
                    std::time(&_initTime); // save the creation time of the window
                    win = new Fl_Double_Window(posX, posY, DEFAULT_W, DEFAULT_H, name.c_str());
                    win->begin();
                    tileWin = new Fl_Tile(0, 0, DEFAULT_W, DEFAULT_H);
                    tileWin->begin();
                    nameCol = new Fl_Box(0, 0, NAME_W, ENTRY_H, "name"); nameCol->box(FL_BORDER_BOX); nameCol->labelfont(FL_TIMES_BOLD); // name column
                    valueCol = new Fl_Box(NAME_W, 0, DEFAULT_W - NAME_W, ENTRY_H, "value"); valueCol->box(FL_BORDER_BOX); valueCol->labelfont(FL_TIMES_BOLD); // value column
                    downBox = new Fl_Box(0, ENTRY_H, DEFAULT_W, DEFAULT_H - ENTRY_H,""); downBox->box(FL_FLAT_BOX);          // dummy invisible used to fill the tile
                    auto boundarybox = new Fl_Box(ENTRY_H, ENTRY_H, DEFAULT_W - 2* ENTRY_H, DEFAULT_H - 2* ENTRY_H);
                    tileWin->end();
                    tileWin->resizable(boundarybox);
                    win->end();
                    win->resizable(tileWin);
                    Fl::add_timeout(1.0, main_timer_callback, this);
                    win->callback(static_windows_callback, this);
                    win->show();
                    }

      
                /** Destructor. Empty the watch map and delte the fltk window */
                ~FltkWatchWin()
                    {
                    while (mapspied.size() != 0) { remove(mapspied.begin()->first,false); } // empty the map of spied variables
                    Fl::remove_timeout(main_timer_callback, this); // remove the main timeout
                    delete win; // delete the fltk window and all its children.
                    }


                /**
                 * Moves the window
                 **/
                void move(int X, int Y)
                    {
                    win->resize(X, Y, win->w(), win->h());
                    win->redraw();
                    }


                /**
                * Removes a variable from the spy window.
                **/
                void remove(const std::string & name, bool reposition)
                    {
                    // make sure that the variable is being watched
                    auto it = mapspied.find(name);
                    if (it == mapspied.end()) { MTOOLS_ERROR(std::string("FltkWatchWin::remove(), variable with name [") + name + "] does not exist"); }
                    WatchObj * obj = it->second;
                    removeTimers(obj);                   // remove all timers
                    detachWindow(obj,reposition);        // remove the fltk button window from the watch window and reposition if required
                    mapspied.erase(it);                  // remove from the map
                    delete(obj);                         // delete the object
                    return;
                    }


                /**
                * Change the refresh rate of a variable
                **/
                void refreshRate(const std::string & name, int newrate)
                    {
                    // make sure that the variable is being watched
                    auto it = mapspied.find(name);
                    if (it == mapspied.end()) { MTOOLS_ERROR(std::string("FltkWatchWin::refreshRate(), variable with name [") + name + "] does not exist"); }
                    WatchObj * obj = it->second;
                    obj->refreshRate(newrate);
                    removeTimers(obj);
                    paintName(obj,RGBc::c_Black);
                    paintValue(obj, RGBc::c_Black);
                    createTimer(obj);
                    return;
                    }


                /**
                * Add a variable to the spy window.
                **/
                void add(const std::string & name, WatchObj * obj)
                    {
                    // make sure that no variable with the same identifer exist
                    if (mapspied.find(name) != mapspied.end()) { MTOOLS_ERROR(std::string("FltkWatchWin::add(), variable with name [") + name + "] is already in watch window"); }
                    attachWindow(obj); // create the fltk windows
                    mapspied[name] = obj;   // insert in the map
                    createTimer(obj); // set an update timer
                    createColorTimer(obj);  // set a color timer
                    return;
                    }

            private:


                /* detach windows */
                void detachWindow(WatchObj * obj, bool reposition)
                    {
                    int Y = obj->_name_button->y();
                    int H = obj->_name_button->h();
                    delete obj->_name_button;
                    delete obj->_value_button;
                    obj->assignFltkWin(this, nullptr, nullptr);
                    if (!reposition) return;
                    int n =tileWin->children();
                    for(int k = 0; k < n; k++)
                        {
                        auto p = tileWin->child(k);                        
                        if (p != downBox) { if (p->y() > Y) { p->resize(p->x(), p->y() - H, p->w(), p->h()); } }
                        else { p->resize(p->x(), p->y() - H, p->w(), p->h() + H); }
                        }

                    win->resize(win->x(), win->y(), win->w(), win->h() - H);
                    win->redraw();
                    }

                /* attach windows */
                void attachWindow(WatchObj * obj)
                    {
                    int X = downBox->x();   // position of the line
                    int Y = downBox->y();   // position of the line
                    if (downBox->h() <= 2*ENTRY_H)
                        {
                        win->resize(win->x(), win->y(), win->w(), win->h() + ENTRY_H);
                        }
                    Fl_Button * nameButton  = new Fl_Button(nameCol->x(), Y, nameCol->w(), ENTRY_H); 
                    nameButton->box(FL_BORDER_BOX); 
                    nameButton->color((Fl_Color)RGBc::c_White);
                    nameButton->copy_label(obj->_name.c_str());
                    Fl_Button * valueButton = new Fl_Button(valueCol->x(), Y, valueCol->w(), ENTRY_H); 
                    valueButton->box(FL_BORDER_BOX); 
                    valueButton->color((Fl_Color)RGBc::c_White);
                    valueButton->labelcolor((Fl_Color)RGBc::c_Red);
                    valueButton->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                    valueButton->labelfont(FL_COURIER_BOLD);
                    valueButton->copy_label(obj->get().c_str());
                    tileWin->add(nameButton);
                    tileWin->add(valueButton);
                    downBox->resize(X, Y+ENTRY_H, tileWin->w(), tileWin->h() - Y - ENTRY_H);
                    win->redraw();  // ask for a redraw
                    obj->assignFltkWin(this, nameButton, valueButton);
                    nameButton->callback(static_name_callback, obj);
                    valueButton->callback(static_value_callback, obj);
                    }

                /* create a timer */
                void createTimer(WatchObj * obj)
                    {
                    Fl::add_timeout(0.001, timer_callback, obj);
                    }

                /* create a color timer */
                void createColorTimer(WatchObj * obj) 
                    { 
                    Fl::remove_timeout(timer_color_callback, obj);   // remove a possible previous timer
                    Fl::add_timeout(0.4, timer_color_callback, obj); // create the new one
                    }

                /* remove all timers associated with an object */
                void removeTimers(WatchObj * obj)
                    {
                    Fl::remove_timeout(timer_callback, obj);         // remove a possible update timer
                    Fl::remove_timeout(timer_color_callback, obj);   // remove a possible color timer
                    }

                /* static callback, redirect to timer */
                static void timer_callback(void* p) 
                    { 
                    if (p == nullptr) { return; } 
                    WatchObj * obj = (WatchObj *)p;
                    obj->_fltkwin->timer(obj);
                    if (obj->refreshRate() > 0)
                        {
                        Fl::repeat_timeout((60.0 / obj->refreshRate()), timer_callback, obj); // set the next timer
                        obj->_fltkwin->createColorTimer(obj); // set a color timer
                        }
                    else 
                        {
                        obj->_fltkwin->removeTimers(obj);
                        }
                    }

                /* timer method, return true to set also color timer */
                void timer(WatchObj * obj)
                    {
                    std::string val = obj->get();
                    if (val != std::string(obj->_value_button->label()))
                        {  
                        obj->_value_button->copy_label(val.c_str());
                        obj->_value_button->redraw_label();
                        paintValue(obj, RGBc::c_Red);
                        } 
                    else 
                        { 
                        paintValue(obj, RGBc::c_Black); 
                        }
                    if (obj->refreshRate() == 0)
                        {
                        obj->_fltkwin->paintValue(obj, RGBc::c_Gray);
                        obj->_fltkwin->paintName(obj, RGBc::c_Gray);
                        }
                    else
                        {
                        paintName(obj, RGBc::c_Red);
                        }
                    return;
                    }

                /* static callback, redirect to timer_color */
                static void timer_color_callback(void* p)
                    {
                    if (p == nullptr) { return; }
                    WatchObj * obj = (WatchObj *)p;
                    obj->_fltkwin->timer_color(obj);
                    }

                /* color timer method  */
                void timer_color(WatchObj * obj)
                    {
                    if (obj->refreshRate() > 0)
                        {
                        paintName(obj, RGBc::c_Black);
                        paintValue(obj, RGBc::c_Black);
                        }
                    }

                /* static callback, redirects to main_timer */
                static void main_timer_callback(void *p)
                    {
                    if (p == nullptr) { return; }
                    ((FltkWatchWin*)p)->main_timer();
                    Fl::repeat_timeout(1.0, main_timer_callback, p); // set the next timer
                    }

                /* main timer method */
                void main_timer()
                    {
                    //"time elapsed:"
                    auto off = std::time(nullptr) - _initTime; // save the creation time of the window
                    std::string str("time elapsed: ");
                    str += mtools::durationToString((uint64)(1000 * off), false);
                    downBox->copy_label(str.c_str());
                    downBox->redraw_label();
                    }

                /* color the name if needed */
                void paintName(WatchObj * obj, RGBc color)
                    {
                    if (obj->_name_button->labelcolor() != (Fl_Color)color)
                        {
                        obj->_name_button->labelcolor((Fl_Color)color);
                        obj->_name_button->redraw_label();
                        }
                    }

                /* color the value if needed */
                void paintValue(WatchObj * obj, RGBc color)
                    {
                    if (obj->_value_button->labelcolor() != (Fl_Color)color)
                        {
                        obj->_value_button->labelcolor((Fl_Color)color);
                        obj->_value_button->redraw_label();
                        }
                    }

                /* static callback, redirect to windows_callback */
                static void static_windows_callback(Fl_Widget* W, void* p) { if (p == nullptr) { return; } ((FltkWatchWin*)p)->window_callback(W); }

                /* ask whether we should force quit the program */
                void window_callback(Fl_Widget* W)
                    {
                    if ((Fl::event() == FL_SHORTCUT) && Fl::event_key() == FL_Escape) { return; } // dont use escape to quit the window
                    if (fl_choice("Do you want to quit?\n Choosing [Yes] will abort the process...", "No", "Yes", nullptr) == 1)
                        {
                        mtools::fltkExit(0);
                        }
                    return;
                    }

                /* static callback for name button */
                static void static_name_callback(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->name_callback((WatchObj *)p); }

                /* callback for name button */
                void name_callback(WatchObj * obj)
                    {
                    MTOOLS_ASSERT(dialogWin == nullptr);
                    int DIALOG_VALUE_W = 400;
                    int DIALOG_VALUE_H = 200;
                    int X = win->x() + win->w() / 2 - DIALOG_VALUE_W / 2;
                    int Y = win->y() + win->h() / 2 - DIALOG_VALUE_H / 2;
                    dialogWin = new Fl_Double_Window(X, Y, DIALOG_VALUE_W, DIALOG_VALUE_H, "Refresh rate");
                    dialogWin->begin();
                    auto txt2 = new Fl_Box(0, 20, 80, 25, "Variable : "); txt2->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
                    auto txt3 = new Fl_Box(0, 50, 80, 25, "Type : "); txt3->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
                    auto txt4 = new Fl_Box(0, 100, 80, 25, "Rate : "); txt4->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
                    auto txt5 = new Fl_Box(80, 20, DIALOG_VALUE_W - 100, 25);  txt5->copy_label(obj->_name.c_str()); txt5->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);  txt5->box(FL_FLAT_BOX); txt5->color(fl_lighter(FL_BACKGROUND_COLOR));
                    auto txt6 = new Fl_Box(80, 50, DIALOG_VALUE_W - 100, 25);  txt6->copy_label(obj->type().c_str()); txt6->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT); txt6->box(FL_FLAT_BOX); txt6->color(fl_lighter(FL_BACKGROUND_COLOR));
                    button_Ok = new Fl_Button(DIALOG_VALUE_W / 3 - 80 / 2, DIALOG_VALUE_H - 45, 80, 35, "Set!"); button_Ok->labelcolor((Fl_Color)RGBc::c_Red);
                    button_Cancel = new Fl_Button(2 * DIALOG_VALUE_W / 3 - 80 / 2, DIALOG_VALUE_H - 45, 80, 35, "Cancel");
                    button_Ok->callback(static_dialname_ok, obj);
                    button_Cancel->callback(static_dialname_cancel, obj);
                    rate_box = new Fl_Box(80, 100, 40, 25); rate_box->copy_label(mtools::toString(obj->refreshRate()).c_str()); rate_box->align(FL_ALIGN_INSIDE);  rate_box->box(FL_FLAT_BOX); rate_box->color(fl_lighter(FL_BACKGROUND_COLOR));
                    slider_widget = new Fl_Slider(130, 103, DIALOG_VALUE_W - 150, 20); 
                    slider_widget->align(Fl_Align(FL_ALIGN_RIGHT));
                    slider_widget->box(FL_FLAT_BOX);
                    slider_widget->type(FL_HOR_NICE_SLIDER);
                    slider_widget->range(0, 600);
                    slider_widget->step(1);
                    slider_widget->value(obj->refreshRate());
                    slider_widget->color2(FL_RED);
                    slider_widget->callback(static_dialname_slider, obj);
                    dialogWin->end();
                    dialogWin->callback(static_dialvalue_cancel, obj);
                    dialogWin->set_modal();
                    dialogWin->show();
                    }


                /* static callback for the ok button for name dialog */
                static void static_dialname_ok(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->dialname_ok((WatchObj *)p); }

                /* callback for the ok button for name dialog */
                void dialname_ok(WatchObj * obj)
                    {
                    int val = (int)slider_widget->value();
                    obj->refreshRate(val);
                    removeTimers(obj);
                    createTimer(obj);
                    createColorTimer(obj);
                    delete dialogWin;
                    dialogWin = nullptr;
                    }

                /* static callback for the ok button for name dialog */
                static void static_dialname_cancel(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->dialname_cancel((WatchObj *)p); }

                /* callback for the ok button for name dialog */
                void dialname_cancel(WatchObj * obj)
                    {
                    delete dialogWin;
                    dialogWin = nullptr;
                    }

                /* static callback for the slider for name dialog */
                static void static_dialname_slider(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->dialname_slider((WatchObj *)p); }

                /* callback for the slider for name dialog */
                void dialname_slider(WatchObj * obj)
                    {
                    int val = (int)slider_widget->value();
                    rate_box->copy_label(mtools::toString(val).c_str());
                    rate_box->redraw_label();              
                    }




                /* static callback for value button */
                static void static_value_callback(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->value_callback((WatchObj *)p); }

                /* callback for value button */
                void value_callback(WatchObj * obj)
                    {
                    if (!(obj->writable())) { return; }
                    MTOOLS_ASSERT(dialogWin == nullptr);
                    int DIALOG_VALUE_W = 400;
                    int DIALOG_VALUE_H = 200;
                    int X = win->x() + win->w() / 2 - DIALOG_VALUE_W / 2;
                    int Y = win->y() + win->h() / 2 - DIALOG_VALUE_H / 2;
                    dialogWin = new Fl_Double_Window(X,Y,DIALOG_VALUE_W, DIALOG_VALUE_H, "Change Value");
                    dialogWin->begin();
                    auto txt1 = new Fl_Box(0, 0, DIALOG_VALUE_W, 40, "!!! Danger zone: use at your own risk !!!"); txt1->labelcolor((Fl_Color)RGBc::c_Red);
                    auto txt2 = new Fl_Box(0, 50, 80, 25, "Variable : "); txt2->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
                    auto txt3 = new Fl_Box(0, 80, 80, 25, "Type : "); txt3->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
                    auto txt4 = new Fl_Box(0, 110, 80, 25, "Value : "); txt4->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
                    auto txt5 = new Fl_Box(80, 50, DIALOG_VALUE_W - 100, 25);  txt5->copy_label(obj->_name.c_str()); txt5->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);  txt5->box(FL_FLAT_BOX); txt5->color(fl_lighter(FL_BACKGROUND_COLOR));
                    auto txt6 = new Fl_Box(80, 80, DIALOG_VALUE_W - 100, 25);  txt6->copy_label(obj->type().c_str()); txt6->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT); txt6->box(FL_FLAT_BOX); txt6->color(fl_lighter(FL_BACKGROUND_COLOR));
                    input_widget = new Fl_Input(80, 110, DIALOG_VALUE_W - 100, 25); input_widget->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT); input_widget->box(FL_FLAT_BOX); input_widget->color(FL_BACKGROUND2_COLOR); input_widget->value(obj->get().c_str());
                    button_Ok = new Fl_Button(DIALOG_VALUE_W/3 - 80/2, DIALOG_VALUE_H -45 , 80, 35, "Set!"); button_Ok->labelcolor((Fl_Color)RGBc::c_Red);
                    button_Cancel = new Fl_Button(2*DIALOG_VALUE_W / 3 - 80/2, DIALOG_VALUE_H - 45, 80, 35, "Cancel");
                    button_Ok->callback(static_dialvalue_ok, obj);
                    button_Cancel->callback(static_dialvalue_cancel, obj);
                    input_widget->callback(static_dialvalue_ok, obj);
                    input_widget->when(FL_WHEN_ENTER_KEY);
                    dialogWin->end();
                    dialogWin->callback(static_dialvalue_cancel,obj);
                    dialogWin->set_modal();
                    dialogWin->show();
                    }

                /* static callback for the ok button for value dialog */
                static void static_dialvalue_ok(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->dialvalue_ok((WatchObj *)p); }

                /* callback for the ok button for value dialog */
                void dialvalue_ok(WatchObj * obj)
                    {
                    std::string newval(input_widget->value());  // get the new value
                    if (newval != std::string(obj->_value_button->label()))
                        {
                        obj->set(newval); // set the value
                        removeTimers(obj); // remove timers
                        createTimer(obj); // set an update timer
                        createColorTimer(obj);  // set a color timer
                        }
                    delete dialogWin;
                    dialogWin = nullptr;
                    }

                /* static callback for the ok button for value dialog */
                static void static_dialvalue_cancel(Fl_Widget* W, void* p) { if (p == nullptr) { return; }  ((WatchObj *)p)->_fltkwin->dialvalue_cancel((WatchObj *)p); }

                /* callback for the ok button for value dialog */
                void dialvalue_cancel(WatchObj * obj)
                    {
                    delete dialogWin;
                    dialogWin = nullptr;
                    }


                std::map<std::string, WatchObj* > mapspied;       // map of all spied variables. 

                time_t _initTime;   // time of creation of the watch window

                Fl_Double_Window *  win;
                Fl_Tile *           tileWin;
                Fl_Box *            nameCol;
                Fl_Box *            valueCol;
                Fl_Box *            downBox;

                Fl_Double_Window *  dialogWin;
                Fl_Button *         button_Ok;
                Fl_Button *         button_Cancel;
                Fl_Input  *         input_widget;
                Fl_Slider *         slider_widget;
                Fl_Box *            rate_box;

                static const int ENTRY_H = 25;
                static const int DEFAULT_W = 650;
                static const int DEFAULT_H = 4 * ENTRY_H;
                static const int NAME_W = 150;


            };




        }



    using namespace mtools::internals_watch;

    int WatchWindow::_nbWatchWin = 0;


        /* Implementation of WatchWindow */

        WatchWindow::WatchWindow() : _fltkobj(nullptr) , _nc(0), _ind(0) , _X(DEFAULT_X), _Y(DEFAULT_Y), _nb(0)
            {
            _name = std::string("Watch ") + toString(++_nbWatchWin);
            }

        WatchWindow::WatchWindow(const std::string  & name, int X, int Y) : _fltkobj(nullptr), _nc(0), _ind(0), _X(X), _Y(Y), _nb(0), _name(name)
            {
            ++_nbWatchWin;
            }

        void WatchWindow::move(int X, int Y)
            {
            _X = X; _Y = Y;
            if (_fltkobj == nullptr) { return; }
            mtools::IndirectMemberProc<FltkWatchWin, int , int> proxy((*_fltkobj), &FltkWatchWin::move, _X, _Y);
            mtools::runInFltkThread(proxy);
            }


        WatchWindow::~WatchWindow() 
            { 
            clear(); 
            }


        void WatchWindow::remove(const std::string & name)
            {
            createIfNeeded();
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &, bool> proxy((*_fltkobj), &FltkWatchWin::remove, name,true);
            mtools::runInFltkThread(proxy);
            _nb--;
            if (_nb == 0) { clear(); }
            }


        void WatchWindow::clear()
            {
            if (_fltkobj != nullptr) 
                { 
                mtools::deleteInFltkThread(_fltkobj); 
                _fltkobj = nullptr; 
                }
            _nb = 0;
            }


        void WatchWindow::refreshRate(const std::string & name, int newrate)
            {
            if (fltkThreadStopped()) return;
            createIfNeeded();
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &, int> proxy((*_fltkobj), &FltkWatchWin::refreshRate, name, newrate);
            mtools::runInFltkThread(proxy);
            }


        void WatchWindow::createIfNeeded() 
            { 
            if (_fltkobj == nullptr) { _fltkobj = mtools::newInFltkThread<internals_watch::FltkWatchWin,const std::string &, int &,int &>(_name, _X,_Y); } 
            }


        void WatchWindow::transmit(const std::string & name, internals_watch::WatchObj * p)
            {
            if (fltkThreadStopped()) return;
            _nb++;
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &, WatchObj* > proxy((*_fltkobj), &FltkWatchWin::add, name, p);
            mtools::runInFltkThread(proxy);
            }



        namespace internals_watch
            {

            WatchWindow * GlobalWatchWindow::_get(int mode)
                {
                static std::atomic<int> init((int)0);  // using local static variables : no initialization problem !
                static std::atomic<WatchWindow *> pwatch((WatchWindow *)nullptr);
                if (mode > 0)
                    {
                    if (init++ == 0) { MTOOLS_DEBUG("Creating the global watch window."); pwatch = new WatchWindow("Global Watch"); } // first time, create the console
                    }
                if (mode < 0)
                    {
                    if (--init == 0) { MTOOLS_DEBUG("Destroying the global watch window."); WatchWindow * p = pwatch; pwatch = (WatchWindow *)nullptr; std::this_thread::yield(); delete p;  } // last one, delete the console
                    }
                return pwatch; // mode = 0, just return a pointer to the Watch window object
                }

            }
}


/* end of file */

