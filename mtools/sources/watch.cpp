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

#include "misc/stringfct.hpp"
#include "misc/indirectcall.hpp"
#include "io/fltk_supervisor.hpp"
#include "io/watch.hpp"
#include "graphics/rgbc.hpp"
#include "io/console.hpp"

namespace mtools
{

    namespace internals_watch
        {

        /* Implementation of WatchObj */

        WatchObj::WatchObj(const std::string & name, int rate) : _rate(rate) , _prev_value(), _fltkwin(nullptr), _name_button(nullptr), _value_button(nullptr),_name(name) {}

        WatchObj::~WatchObj() { }


        std::string WatchObj::get(bool & changed) const
            {
            std::string newstr = _get();
            if (newstr == _prev_value) changed = false; else changed = true;
            _prev_value = newstr;
            return newstr;
            }

        bool WatchObj::set(const std::string & value)
            {
            _set(value);
            bool changed = false;
            get(changed);
            return changed;
            }

        std::string WatchObj::_get() const { MTOOLS_ERROR("pure virtual method WatchObj::get(), access forbidden"); return std::string(); }

        size_t WatchObj::_set(const std::string & value) { MTOOLS_ERROR("pure virtual method WatchObj::set(), access forbidden"); return 0; }

        bool WatchObj::writable() const { MTOOLS_ERROR("pure virtual method WatchObj::writable(), access forbidden"); return false; }

        int WatchObj::refreshRate() const { return _rate; }

        int WatchObj::refreshRate(int newrate) { if (newrate < 0) { newrate = 0; } else if (newrate > 600) { newrate = 600; } _rate = newrate; return _rate; }

        void WatchObj::assignFltkWin(FltkWatchWin * p, Fl_Button * nameButton, Fl_Button * valueButton) { _fltkwin = p;  _name_button = nameButton; _value_button = valueButton; }

        /* fltk watch window */
        class FltkWatchWin
            {
            static const int DEFAULT_W = 650;
            static const int DEFAULT_H = 250;
            static const int DEFAULT_X = 0;
            static const int DEFAULT_Y = 480;
            static const int NAME_W   = 150;
            static const int ENTRY_H = 25;

            Fl_Double_Window *  win;
            Fl_Tile *           tileWin;            
            Fl_Box *            nameCol;
            Fl_Box *            valueCol;
            Fl_Box *            downBox;
                
            public:


                /** Default constructor. Create and show the watch window */
                FltkWatchWin()
                    {
                    win = new Fl_Double_Window(DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H, "Watch Window");
                    win->begin();
                    tileWin = new Fl_Tile(0, 0, DEFAULT_W, DEFAULT_H);
                    tileWin->begin();
                    nameCol = new Fl_Box(0, 0, NAME_W, ENTRY_H, "name"); nameCol->box(FL_BORDER_BOX);                     // name column
                    valueCol = new Fl_Box(NAME_W, 0, DEFAULT_W - NAME_W, ENTRY_H, "value"); valueCol->box(FL_BORDER_BOX);  // value column
                    downBox = new Fl_Box(0, ENTRY_H, DEFAULT_W, DEFAULT_H - ENTRY_H,"dummy"); downBox->box(FL_FLAT_BOX);          // dummy invisible used to fill the tile
                    auto boundarybox = new Fl_Box(ENTRY_H, ENTRY_H, DEFAULT_W - 2* ENTRY_H, DEFAULT_H - 2* ENTRY_H);
                    tileWin->end();
                    tileWin->resizable(boundarybox);
                    win->end();
                    win->resizable(tileWin);
                    win->show();
                    }

      
                /** Destructor. Empty the watch map and delte the fltk window */
                ~FltkWatchWin()
                    {
                    while (mapspied.size() != 0) { remove(mapspied.begin()->first,false); } // empty the map of spied variables
                    delete win; // delete the fltk window and its children.
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
                    detachWindow(obj,reposition);        // remove the fltk button window from the watch window and reposition
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
                    if (it == mapspied.end()) { MTOOLS_ERROR(std::string("FltkWatchWin::remove(), variable with name [") + name + "] does not exist"); }
                    WatchObj * obj = it->second;
                    obj->refreshRate(newrate);
                    removeTimers(obj);
                    paintBlack(obj);
                    createTimer(obj,0.001);
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
                    createTimer(obj,0.001); // set an update timer
                    createColorTimer(obj);  // set a color timer
                    return;
                    }

            private:


                /* detach windows */
                void detachWindow(WatchObj * obj, bool reposition)
                    {
                    delete obj->_name_button;
                    delete obj->_value_button;

                    /* TODO */

                    obj->assignFltkWin(this, nullptr, nullptr);
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
                    nameButton->color(RGBc::c_White);
                    nameButton->copy_label(obj->_name.c_str());
                    Fl_Button * valueButton = new Fl_Button(valueCol->x(), Y, valueCol->w(), ENTRY_H); 
                    valueButton->box(FL_BORDER_BOX); 
                    valueButton->color(RGBc::c_White);
                    valueButton->labelcolor(RGBc::c_Red);
                    bool ch;
                    valueButton->copy_label(obj->get(ch).c_str());
                    tileWin->add(nameButton);
                    tileWin->add(valueButton);
                    downBox->resize(X, Y+ENTRY_H, tileWin->w(), tileWin->h() - Y - ENTRY_H);
                    win->redraw();  // ask for a redraw
                    obj->assignFltkWin(this, nameButton, valueButton);
                    }

                /* create a timer */
                void createTimer(WatchObj * obj, double r)
                    {
                    Fl::add_timeout(r, timer_callback, obj);
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
                    bool ct = obj->_fltkwin->timer(obj);
                    Fl::repeat_timeout((60.0 / obj->refreshRate()), timer_callback, obj); // set the next timer
                    if (ct) obj->_fltkwin->createColorTimer(obj); // set a color timer if needed
                    }

                /* timer method, return true to set also color timer */
                bool timer(WatchObj * obj)
                    {
                    bool ch;
                    std::string val = obj->get(ch);
                    obj->_name_button->labelcolor(RGBc::c_Red);
                    obj->_name_button->redraw();
                    obj->_name_button->redraw();
                    if (ch)
                        {
                        obj->_value_button->labelcolor(RGBc::c_Red);
                        obj->_value_button->copy_label(val.c_str());
                        obj->_value_button->redraw();
                        return true;
                        }
                    return true;
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
                    paintBlack(obj);
                    }

                /* color the name and value in black */
                void paintBlack(WatchObj * obj)
                    {
                    if (obj->_value_button->labelcolor() != (Fl_Color)RGBc::c_Black)
                        {
                        obj->_value_button->labelcolor(RGBc::c_Black);
                        obj->_value_button->redraw();
                        }
                    if (obj->_name_button->labelcolor() != (Fl_Color)RGBc::c_Black)
                        {
                        obj->_name_button->labelcolor(RGBc::c_Black);
                        obj->_name_button->redraw();
                        }
                    }

                std::map<std::string, WatchObj* > mapspied;       // map of all spied variables. 
               
            };




        }



    using namespace mtools::internals_watch;



        /* Implementation of WatchWindow */

        WatchWindow::WatchWindow() : _fltkobj(nullptr) 
            {
            }


        WatchWindow::~WatchWindow() 
            { 
            clear(); 
            }


        void WatchWindow::remove(const std::string & name)
            {
            createIfNeeded();
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &, bool> proxy((*_fltkobj), &FltkWatchWin::remove, name,true);
            mtools::runInFLTKThread(proxy);
            }


        void WatchWindow::clear()
            {
            if (_fltkobj != nullptr) { mtools::deleteInFLTKThread(_fltkobj); _fltkobj = nullptr; }
            }


        void WatchWindow::refreshRate(const std::string & name, int newrate)
            {
            createIfNeeded();
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &, int> proxy((*_fltkobj), &FltkWatchWin::refreshRate, name, newrate);
            mtools::runInFLTKThread(proxy);
            }


        void WatchWindow::createIfNeeded() 
            { 
            if (_fltkobj == nullptr) { _fltkobj = mtools::newInFLTKThread<internals_watch::FltkWatchWin>(); } 
            }


        void WatchWindow::transmit(const std::string & name, internals_watch::WatchObj * p)
            {
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &, WatchObj* > proxy((*_fltkobj), &FltkWatchWin::add, name, p);
            mtools::runInFLTKThread(proxy);
            }


}


/* end of file */

