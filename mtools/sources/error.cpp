/** @file error.cpp */
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


#include "misc/error.hpp"
#include "graphics/customcimg.hpp"
#include "io/logfile.hpp"

#include <iostream>

namespace mtools
    {
    namespace internals_error
        {

        std::string truncateFilename(std::string s)
            {
            size_t pos = s.find_last_of(L'\\');
            if (pos != std::wstring::npos) { s = s.substr(pos + 1, std::wstring::npos); }
            pos = s.find_last_of(L'/');
            if (pos != std::wstring::npos) { s = s.substr(pos + 1, std::wstring::npos); }
            return s;
            }

        void display(const std::string & title, const std::string & msg)
            {
            std::string msg2 = std::string("*** ") + title + " ***\n" + msg.c_str() + "\n*********************************\n\n";
            mtools::LogFile lf("abort.txt"); lf << msg2; // log to abort.txt
            std::cerr << "\n\n" << msg2;   // send to std::cerr
            }


        void displayGraphics(const std::string & title, const std::string & msg)
            {

//            cimg_library::cimg::dialog(title.c_str(), msg.c_str()); // display with cimg
            fl_alert((title + "\n" + msg).c_str());
            }


        void _debugs(const std::string & file, int line, const std::string & s)
            {
            static mtools::LogFile lf("debug.txt");
            std::string msg = std::string("[DEBUG ") + truncateFilename(file) + " (" + std::to_string(line) + ")] " + s + "\n";
            std::clog << msg;
            lf << msg;
            return;
            }


        }
    }


/* end of file */

