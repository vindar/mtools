/** @file drawable2Dobject.cpp */
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

#include "graphics/internal/drawable2Dobject.hpp"


namespace mtools
{

namespace internals_graphics
{


     
    AutoDrawable2DObject::AutoDrawable2DObject(Drawable2DObject * obj, bool startThread) :  _mustexit(false), _threadon(false), _obj(obj)
            {
            MTOOLS_ASSERT(obj != nullptr);
            if ((!_obj->needWork())||(!startThread)) return; // no work needed, return directly
            _startThread(); // start the thread
            }


        
        AutoDrawable2DObject::~AutoDrawable2DObject()
            {
            _stopThread(); // stop the thread if needed.
            }


        void AutoDrawable2DObject::setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
            {
            std::lock_guard<std::mutex> lg(_mut);
            _obj->setParam(range, imageSize);
            }


        void AutoDrawable2DObject::resetDrawing()
            {
            std::lock_guard<std::mutex> lg(_mut);
            _obj->resetDrawing();
            }


   
        int AutoDrawable2DObject::drawOnto(Image & im, float opacity)
            {
            std::lock_guard<std::mutex> lg(_mut);
            return _obj->drawOnto(im,opacity);
            }


    
        int AutoDrawable2DObject::quality() const
            {
            return _obj->quality();
            }


      
        bool AutoDrawable2DObject::needWork() const
            {
            return _obj->needWork();
            }


        void AutoDrawable2DObject::workThread(bool status)
            {
            std::lock_guard<std::mutex> lg(_mut);
            if (status) _startThread(); else _stopThread();
            }


        bool AutoDrawable2DObject::workThread() const
            {
            return (_threadon == true);
            }


        /* the thread procedure doing the work */
        void AutoDrawable2DObject::_workerThread()
            {
            try
                {
                int nb = 0;
                _threadon = true; // ok we are on...
                while (!_mustexit) // loop until we are required to exit
                    {
                    int q = _obj->work(500); // work for 1/3 of a second
                    if (q == 100) {
                        std::this_thread::yield();
                        if (nb >= 100) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); nb = 0; }
                        nb++;
                        }
                    else { nb = 0; }
                    }
                _threadon = false; // ...and we are off
                }
            catch (std::exception & exc)
                {
                std::string msg = std::string("Exception caught in an autoDrawable2DObject : [") + exc.what() + "].";
                MTOOLS_ERROR(msg.c_str());
                }
            }


        /* start the thread if it is not active */
        void AutoDrawable2DObject::_startThread()
        {
        if ((_obj->needWork() == false) || (_threadon == true)) return; // do nothing if no work needed or thread already started
        _mustexit = false;
        std::thread th(&AutoDrawable2DObject::_workerThread, this); // launch the thread
        while (_threadon == false) { std::this_thread::yield(); } // wait for confirmation
        th.detach(); // useless but still...
        }


        /* stop the thread if it is active */
        void AutoDrawable2DObject::_stopThread()
            {
            if (_threadon == true) // check if the thread is on
                {
                _mustexit = true;
                _obj->stopWork();
                while (_threadon == true) { _obj->stopWork();  std::this_thread::yield(); }
                }
            _mustexit = false;
            }

}

}


/* end of file */
