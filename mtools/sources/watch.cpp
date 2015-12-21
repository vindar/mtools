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

        WatchObj::WatchObj(int rate) : _rate(rate) {}

        WatchObj::~WatchObj() { }

        std::string WatchObj::get() const { MTOOLS_ERROR("pure virtual method WatchObj::get(), access forbidden"); return std::string(); }

        size_t WatchObj::set(const std::string & value) { MTOOLS_ERROR("pure virtual method WatchObj::set(), access forbidden"); return 0; }

        bool WatchObj::writable() const { MTOOLS_ERROR("pure virtual method WatchObj::writable(), access forbidden"); return false; }

        int WatchObj::refreshRate() const { return _rate; }

        int WatchObj::refreshRate(int newrate) { _rate = newrate; return _rate; }






        class FltkWatchWin
            {

            public:


                /** Default constructor. */
                FltkWatchWin()
                    {

                    }


                /** Destructor. */
                ~FltkWatchWin()
                    {

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
                    mtools::cout << "add " << name << "....\n\n";
                    return;
                    }


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

