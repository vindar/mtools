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

#include "misc/error.hpp"
#include "io/logfile.hpp"

#include <iostream>
#include <thread>
#include <functional>

#include <FL/fl_ask.H>


namespace mtools
    {


	void setErrorCallback(mtools_error_cb cb) { internals_error::_error_cb = cb; }


	void removeErrorCallBack() { internals_error::_error_cb = nullptr; }


    namespace internals_error
        {

		mtools_error_cb _error_cb = nullptr; // the error callback function


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
            fl_alert("%s",(title + "\n" + msg).c_str());
            }


        void _debugs(const std::string & file, int line, const std::string & s)
            {
            static mtools::LogFile lf("debug.txt");
            std::string msg = std::string("[DEBUG ") + truncateFilename(file) + " (" + std::to_string(line) + ") th=" + std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id())) + "] " + s + "\n";
            std::clog << msg;
            lf << msg;
            return;
            }


		void _stopWithMsg(const std::string & title, const std::string & msg)
			{
			if (_error_cb != nullptr) _error_cb(title, msg);
			display(title, msg);
			#if (MTOOLS_BASIC_CONSOLE == 0)
			displayGraphics(title, msg);
			#endif
			exit(EXIT_FAILURE);
			}

		void _error(const std::string & file, int line, const std::string & s)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nMessage : " << s;
			if (_error_cb != nullptr) _error_cb("MTOOLS_ERROR", os.str());
			_stopWithMsg("MTOOLS_ERROR", os.str());
			}

		bool _insures1(const std::string & file, int line, const std::string & s)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s;
			if (_error_cb != nullptr) _error_cb("MTOOLS_INSURE_FAILURE", os.str());
			_stopWithMsg("MTOOLS_INSURE FAILURE", os.str());
			return true;
			}

		bool _insures2(const std::string & file, int line, const std::string & s, const std::string & msg)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s << "\nMessage : " << msg;
			if (_error_cb != nullptr) _error_cb("MTOOLS_INSURE_FAILURE", os.str());
			_stopWithMsg("MTOOLS_INSURE FAILURE", os.str());
			return true;
			}


		bool _asserts1(const std::string & file, int line, const std::string & s)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s;
			if (_error_cb != nullptr) _error_cb("MTOOLS_ASSERT_FAILURE", os.str());
			_stopWithMsg("MTOOLS_ASSERT FAILURE", os.str());
			return true;
			}


		bool _asserts2(const std::string & file, int line, const std::string & s, const std::string & msg)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s << "\nMessage : " << msg;
			if (_error_cb != nullptr) _error_cb("MTOOLS_ASSERT_FAILURE", os.str());
			_stopWithMsg("MTOOLS_ASSERT FAILURE", os.str());
			return true;
			}


		void _throws_debug(const std::string & file, int line, const std::string & s)
			{
			_debugs(file, line, s);
			if (_error_cb != nullptr)
				{
				std::ostringstream os;
				os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nThrow : " << s;
				_error_cb("MTOOLS_THROW", os.str());
				}
			throw s.c_str();
			}


		void _throws_nodebug(const std::string & file, int line, const std::string & s)
			{
			if (_error_cb != nullptr)
				{
				std::ostringstream os;
				os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nThrow : " << s;
				_error_cb("MTOOLS_THROW", os.str());
				}
			throw s.c_str();
			}


        }
    }


/* end of file */

