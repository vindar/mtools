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

        WatchObj::WatchObj(const std::string & name, int rate) : _rate(rate) , _prev_value(), _fltkwin(nullptr), _name(name) {}


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

        int WatchObj::refreshRate(int newrate) { _rate = newrate; return _rate; }

        void WatchObj::assignFltkWin(FltkWatchWin * p) { _fltkwin = p; }



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


                /** Default constructor. */
                FltkWatchWin()
                    {
                    win = new Fl_Double_Window(DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H, "Watch Window");
                    win->begin();
                    tileWin = new Fl_Tile(0, 0, DEFAULT_W, DEFAULT_H);
                    tileWin->begin();
                    auto boundaryBox = new Fl_Box(3 * ENTRY_H, ENTRY_H, DEFAULT_W - 6 * ENTRY_H, DEFAULT_H - 2 * ENTRY_H);        // invisible box that prevent resizing to zero size
                    nameCol = new Fl_Box(0, 0, NAME_W, ENTRY_H, "name"); nameCol->box(FL_BORDER_BOX);                     // name column
                    valueCol = new Fl_Box(NAME_W, 0, DEFAULT_W - NAME_W, ENTRY_H, "value"); valueCol->box(FL_BORDER_BOX);  // value column
                    downBox = new Fl_Box(0, ENTRY_H, DEFAULT_W, DEFAULT_H - ENTRY_H); downBox->box(FL_FLAT_BOX);          // dummy invisible used to fill the tile
                    tileWin->end();
                    tileWin->resizable(boundaryBox);
                    win->end();
                    win->resizable(tileWin);
                    win->show();
                    }

                    //dummyCol  = new Fl_Box(NAME_W + VALUE_W, 0, DEFAULT_W + ADDITIONAL_W - (NAME_W + VALUE_W), ENTRY_H); dummyCol->box(FL_FLAT_BOX);

//                    auto B = new Fl_Button(0, ENTRY_H, NAME_W, ENTRY_H, "n"); B->box(FL_BORDER_BOX); B->color(RGBc::c_White);
//                    auto T = new Fl_Button(NAME_W, ENTRY_H, DEFAULT_W - NAME_W, ENTRY_H, "1.2345"); T->box(FL_BORDER_BOX); T->color(RGBc::c_White);

//                    auto vvv1 = new Fl_Box(NAME_W + VALUE_W, ENTRY_H, DEFAULT_W + ADDITIONAL_W - (NAME_W + VALUE_W), ENTRY_H, "bbb"); vvv1->box(FL_FLAT_BOX);

//                    auto B2 = new Fl_Button(0, 2*ENTRY_H, NAME_W, ENTRY_H, "m"); B2->box(FL_BORDER_BOX); B2->color(RGBc::c_White);
//                    auto T2 = new Fl_Button(NAME_W, 2*ENTRY_H, DEFAULT_W - NAME_W, ENTRY_H, "11232431.2345"); T2->box(FL_BORDER_BOX); T2->color(RGBc::c_White);


//                    auto vvv2 = new Fl_Box(NAME_W + VALUE_W, 2*ENTRY_H, DEFAULT_W + ADDITIONAL_W - (NAME_W + VALUE_W), ENTRY_H, "ccc"); vvv2->box(FL_FLAT_BOX);

                    


                /* add a line in the window, return the two input box created */
                void addLine()
                    {
                    }

                /* remove a line */
                void removeLine()
                    {
                    }


                /** Destructor. */
                ~FltkWatchWin()
                    {
                    delete win;
                    }


                /**
                * Removes a variable from the spy window.
                **/
                void remove(const std::string & name)
                    {
                    mtools::cout << "REMOVING " << name << "....\n\n";
                    return;
                    }


                /**
                * Change the refresh rate of a variable
                **/
                void refreshRate(const std::string & name, int newrate)
                    {
                    return;
                    }


                /**
                * Add a variable to the spy window.
                **/
                void add(const std::string & name, WatchObj * obj)
                    {
                    obj->assignFltkWin(this); // associate this watch window with the object

                    mtools::cout << "add " << name << "....\n\n";

                    return;
                    }

            private:



                /* create a timer associated with a name */
                void createTimer(const std::string & name, int rate) { Fl::add_timeout((60.0 / rate), timer_callback, (void*)(&(mapspied[name]))); }

                /* create a color timer associated with a name */
                void createColorTimer(const std::string & name) { Fl::add_timeout(0.2, timer_color_callback, (void*)(&(mapspied[name]))); }

                /* remove all timers associated with a name */
                void removeTimers(const std::string & name)
                    {
                    Fl::remove_timeout(timer_callback, (void*)(&(mapspied[name])));         // remove a possible update timer
                    Fl::remove_timeout(timer_color_callback, (void*)(&(mapspied[name])));   // remove a possible color timer
                    }

                /* static callback, redirect to timer */
                static void timer_callback(void* p) 
                    { 
                    if (p == nullptr) { return; } 
                    WatchObj * pobj = (WatchObj *)p;                                    // get the corresponding object
                    pobj->_fltkwin->timer(pobj->_name);                                 // call the timer method
                    Fl::repeat_timeout((60.0/pobj->refreshRate()) , timer_callback, p); // set the next timer
                    }

                /* static callback, redirect to timer_color */
                static void timer_color_callback(void* p)
                    {
                    if (p == nullptr) { return; } 
                    WatchObj * pobj = (WatchObj *)p;            // get the corresponding object
                    pobj->_fltkwin->timer_color(pobj->_name);   // call the color timer method
                    }

                /* called with the name of the variable to update */
                void timer(const std::string & name)
                    {
                    }

                /* called with the name of the variable to color  */
                void timer_color(const std::string & name)
                    {
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
            mtools::IndirectMemberProc<FltkWatchWin, const std::string &> proxy((*_fltkobj), &FltkWatchWin::remove, name);
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

