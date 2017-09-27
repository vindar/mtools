/** @file console.hpp */
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


/**************************************************************************
*
* Define the preprocessor directive MTOOLS_BASIC_CONSOLE to disable FLTK's 
* cout console and fall back on using stdout.
*
* 
* !!! When using precompiled header, put #define MTOOLS_BASIC_CONSOLE
* inside the stdafx.h file otherwise the directive is overwritten. !!!
* 
***************************************************************************/

#pragma once

#include "internal/fltkSupervisor.hpp" // make sure sentnel object for fltk thread created before the global watch object
#include "logfile.hpp"

#include <mutex>
#include <atomic>
#include <string>

namespace mtools
    {


    /* forward declarations */
    namespace internals_console
        {
        class ConsoleWidget;
        class CoutConsole;

        template<typename ConsoleClass> class CoutProxy
            {
            public:
                template<typename T> inline operator T()
                    {
                    if (_cons == nullptr) return T();
                    auto status = (*_cons).useDefaultInputValue();
                    (*_cons) << _question << " : ";
                    T val;
                    if (_hasdefaultvalue) { fromString(_defaultvalue, val); (*_cons).useDefaultInputValue(true); } else { (*_cons).useDefaultInputValue(false); }
                    (*_cons) >> val; (*_cons) << toString(val) << "\n"; 
                    (*_cons).useDefaultInputValue(status);
                    return val;
                    }                
              
                static CoutProxy _get(ConsoleClass * console, const std::string & question, const std::string & defaultvalue, bool hasdefaultvalue) { return CoutProxy(console, question, defaultvalue, hasdefaultvalue); }
                CoutProxy(const CoutProxy &) = default;
                CoutProxy & operator=(const CoutProxy &) = default;

            private:
                CoutProxy(ConsoleClass * console, const std::string & question, const std::string & defaultvalue, bool hasdefaultvalue) : _cons(console), _question(question), _defaultvalue(defaultvalue), _hasdefaultvalue(hasdefaultvalue) {};
                ConsoleClass * _cons;
                std::string _question, _defaultvalue;
                bool _hasdefaultvalue;
            };


        }


    /**
     * Hides the Windows console if present. Does nothing if the OS is not MS Windows.
     **/
    void hideMSConsole();

    /**
     * Show the Windows console if present. Does nothing if the OS is not MS Windows.
     **/
    void showMSConsole();

    /**
     * A screen console for simple input/output. Everything output to the screen is also sent to a
     * logfile.
     *
     * - The console object is thread-safe. Accessed to the console methods are mutex-protected.
     *
     * - A global Console object mtools::cout is created at program startup (but is not displayed
     * unless it is accessed) and can be used as a replacement for std::cout and std::cin.
     **/
    class Console
        {

        public:

            /**
             * Default constructor. Create a console with a default name "Console-XXX". The console is
             * hidden until the first interaction is performed.
             **/
            Console();


            /**
             * Construct a console with a given name.
             *
             * @param   filename        name of the console (associated logfile is name + ".txt").
             * @param   showAtCreation  true to show the console at construction time and false (default) to
             *                          wait until something is written/read from the console.
             **/
            Console(const std::string & filename, bool showAtCreation = false);


            /**
             * Sets a new name for the console / log file, this has no effect if the console is already displayed.
             *
             * @param   filename    new console name / log file name (without extension).
             **/
            void setName(const std::string & filename) { _consoleName = filename; }


            /**
             * Destructor.Flushed all streams.
             **/
            ~Console();


            /**
             * Clears the screen. This does not erase the logfile but it inserts a separator.
             **/
            void clear();


            /**
             * Print something in the console. Uses the toString() conversion function to convert the object
             * into a std::string.
             *
             * @tparam  T   Type of the object.
             * @param   O   The object to print.
             *
             * @return  The console for chaining.
             **/
            template<typename T> Console & operator<<(const T & O) { _print(mtools::toString(O)); return(*this); }


            /**
             * Get an object from the console. Use the fromString() conversion to convert the std::string into
             * an object.
             *
             * @tparam  T   Type of the object.
             * @param   O   A reference to store the object into.
             *
             * @return  The console for chaining.
             **/
            template<typename T> Console & operator>>(T & O)
                {
                std::string o = (_showDefaultInputValue ? mtools::toString(O) : "");
                std::string s;
                while (1)
                    {
                    s = _getText(o);
                    size_t nb = mtools::fromString(s, O);
                    if ((s.length() != 0) && (nb == s.length())) { return *this; }
                    }
                }


            /**
             * Query a boolean from the console. Set to true if the user type one of the ofllowing key: 'o',
             * 'O', 'y', 'Y', '1' and false if the user type 'N', 'n' or the escape key.
             *
             * @param [in,out]  b   The boolean to fill the anwser with.
             *
             * @return  The console for chaining
             **/
            Console & operator>>(bool & b);


            /**
            * Query a char from the console.
            *
            * @param [in,out]  c   The char to fill.
            *
            * @return  The console for chaining
            **/
            Console & operator>>(char & c);


            /**
             * Asks a question and return the value using type deduction on return type.
             *
             * @param   question    The question string.
             *
             * @return  A proxy object which can be implicitly casted to the desired type.
             **/
            internals_console::CoutProxy<Console> ask(const std::string & question)
                {
                return internals_console::CoutProxy<Console>::_get(this, question, "", false);
                }


            /**
             * Asks a question and return the value using type deduction on return type.
             *
             * @param   question        The question string.
             * @param   defaultValue    The default value. 
             *
             * @return  A proxy object which can be implicitly casted to the desired type.
             **/
            template<typename T> internals_console::CoutProxy<Console> ask(const std::string & question, const T & defaultValue)
                {
                return internals_console::CoutProxy<Console>::_get(this, question, mtools::toString(defaultValue), true);
                }


            /**
             * Determines if we print the default (ie current) value of the object when query of a new value
             * using the >> operator. By default, the flag is set to false so there is no default value.
             *
             * @return  true if we use the current vlaue of the oject as the default value when using the >>
             *          operator.
             **/
            bool useDefaultInputValue() const { return _showDefaultInputValue; }


            /**
             * Choose if we should, from now on, print a default value for the object when a value is
             * queried via the >> operator.
             *
             * @param   newstatus   true to print the current value of the object as default value when using
             *                      >> operator and false to print nothing.
             **/
            void useDefaultInputValue(bool newstatus) { _showDefaultInputValue = newstatus; }


            /**
             * Wait for the user to press a key and return its code.
             *
             * @return  The key code (FLTK code).
             **/
            int getKey();


            /**
             * Enable file logging (on by default).
             **/
            void enableLogFile() { _enableLogging = true; }


            /**
             * Disable file logging.
             **/
            void disableLogFile() { _enableLogging = false; }


            /**
             * Enable output on the screen (on by default).
             **/
            void enableScreenOutput() { _enableScreen = true; }


            /**
             * Disable output on the screen.
             **/
            void disableScreenOutput() { _enableScreen = false; }


            /**
             * Resize the console
             *
             * @param   x   x coordinate of the upper left corner.
             * @param   y   y coordinate of the upper left corner.
             * @param   w   The width of the console.
             * @param   h   The height of the console.
             **/
            void resize(int x, int y, int w, int h);


            /**
             * Move the console.
             *
             * @param   x   x coordinate of the upper left corner.
             * @param   y   y coordinate of the upper left corner.
             **/
            void move(int x, int y);


        private:

            std::string _getText(const std::string & initText = "");
            void _disableConsole();
            void _print(const std::string & s);
            void _makeWindow();
            bool _startProtect();
            void _endProtect();

            Console(const Console &) = delete;
            Console & operator=(Console &) = delete;

            std::string         _waiting_text;
            std::mutex          _mutext;
            std::atomic<size_t> _tl;
            std::atomic<internals_console::ConsoleWidget *> _CW;
            std::mutex          _mustop;
            std::atomic<int>    _disabled;

            std::atomic<bool>   _enableLogging;
            std::atomic<bool>   _enableScreen;
            std::atomic<bool>   _showDefaultInputValue;

            std::string         _consoleName;
            mtools::LogFile *   _logfile;

            static int          _consoleNumber;


            friend class mtools::internals_console::CoutConsole;

        };


    namespace internals_console
        {

        /**
        * Replaces the Console class when MTOOLS_BASIC_CONSOLE is set.
        **/
        class ConsoleBasic
            {

            public:
                ConsoleBasic(const std::string & name);
                ~ConsoleBasic();
                void setName(const std::string & filename) { _consoleName = filename; }
                template<typename T> ConsoleBasic & operator<<(const T & O) { _print(mtools::toString(O)); return(*this); }
                template<typename T> ConsoleBasic & operator>>(T & O)
                    {
                    std::string s, o = (_showDefaultInputValue ? mtools::toString(O) : "");
                    while (1) { s = _getText(o); size_t nb = mtools::fromString(s, O); if ((s.length() != 0) && (nb == s.length())) { return *this; } }
                    }
                ConsoleBasic & operator>>(bool & b);
                ConsoleBasic & operator>>(char & c);
                internals_console::CoutProxy<ConsoleBasic> ask(const std::string & question) { return internals_console::CoutProxy<ConsoleBasic>::_get(this, question, "", false); }
                template<typename T> internals_console::CoutProxy<ConsoleBasic> ask(const std::string & question, const T & defaultValue) { return internals_console::CoutProxy<ConsoleBasic>::_get(this, question, mtools::toString(defaultValue), true); }
                bool useDefaultInputValue() const { return _showDefaultInputValue; }
                void useDefaultInputValue(bool newstatus) { _showDefaultInputValue = newstatus; }
                int getKey();
                void enableLogFile() { _enableLogging = true; }
                void disableLogFile() { _enableLogging = false; }
                void enableScreenOutput() { _enableScreen = true; }
                void disableScreenOutput() { _enableScreen = false; }
                void clear() {}                             // for compatibility with Console, does nothing
                void resize(int x, int y, int w, int h) {}  // for compatibility with Console, does nothing
                void move(int x, int y) {}                  // for compatibility with Console, does nothing

            private:

                std::string _getText(const std::string & initText = "");
                void _print(const std::string & s);

                ConsoleBasic(const ConsoleBasic &) = delete;
                ConsoleBasic & operator=(ConsoleBasic &) = delete;

                std::atomic<bool>   _enableLogging;
                std::atomic<bool>   _enableScreen;
                std::atomic<bool>   _showDefaultInputValue;
                std::string         _consoleName;
                mtools::LogFile *   _logfile;
            };



        /**
        * Wrapper class for the global "cout" console when MTOOLS_NO_GRAPHICS is set
        **/
        class CoutConsoleBasic
            {
            public:
                CoutConsoleBasic() { internals_fltkSupervisor::insureFltkSentinel(); _get(1); }
                ~CoutConsoleBasic() { _get(-1); }
                void setName(const std::string & filename) { if (!_exist()) return; _get(0)->setName(filename); }
                template<typename T> CoutConsoleBasic & operator<<(const T & O) { if (!_exist()) return(*this);  _get(0)->operator<<(O); return(*this); }
                template<typename T> CoutConsoleBasic & operator>>(T & O) { if (!_exist()) return(*this); _get(0)->operator>>(O); return(*this); }
                CoutProxy<CoutConsoleBasic> ask(const std::string & question) { return CoutProxy<CoutConsoleBasic>::_get(this, question, "", false); }
                template<typename T> CoutProxy<CoutConsoleBasic> ask(const std::string & question, const T & defaultValue) { return CoutProxy<CoutConsoleBasic>::_get(this, question, mtools::toString(defaultValue), true); }
                void clear() { if (!_exist()) return; _get(0)->clear(); }
                int getKey() { if (!_exist()) return 0; return _get(0)->getKey(); }
                bool useDefaultInputValue() { if (!_exist()) return false; return _get(0)->useDefaultInputValue(); }
                void useDefaultInputValue(bool newstatus) { if (!_exist()) return; _get(0)->useDefaultInputValue(newstatus); }
                void enableLogFile() { if (!_exist()) return; _get(0)->enableLogFile(); }
                void disableLogFile() { if (!_exist()) return; _get(0)->disableLogFile(); }
                void enableScreenOutput() { if (!_exist()) return; _get(0)->enableScreenOutput(); }
                void disableScreenOutput() { if (!_exist()) return; _get(0)->disableScreenOutput(); }
                void resize(int x, int y, int w, int h) { if (!_exist()) return; _get(0)->resize(x, y, w, h); }
                void move(int x, int y) { if (!_exist()) return; _get(0)->move(x, y); }

            private:

                bool _exist()
                    {
                    if (_get(0) == nullptr) { MTOOLS_DEBUG("CoutConsoleBasic method called after object was destroyed!"); return false; }
                    return true;
                    }

                ConsoleBasic * _get(int);
                CoutConsoleBasic(const CoutConsoleBasic&) = delete;
                CoutConsoleBasic & operator=(const CoutConsoleBasic&) = delete;
            };


        /**
         * Wrapper class for the global "cout" console. Normal version
        **/
        class CoutConsole
            {
            public:
                CoutConsole() { internals_fltkSupervisor::insureFltkSentinel(); _get(1); }
                ~CoutConsole() { _get(-1); }
                void setName(const std::string & filename) { if (!_exist()) return; _get(0)->setName(filename); }
                template<typename T> CoutConsole & operator<<(const T & O) { if (!_exist()) return (*this); _get(0)->operator<<(O); return(*this); }
                template<typename T> CoutConsole & operator>>(T & O) { if (!_exist()) return (*this); _get(0)->operator>>(O); return(*this); }
                CoutProxy<CoutConsole> ask(const std::string & question) { return CoutProxy<CoutConsole>::_get(this, question, "", false); }
                template<typename T> CoutProxy<CoutConsole> ask(const std::string & question, const T & defaultValue) { return CoutProxy<CoutConsole>::_get(this, question, mtools::toString(defaultValue), true); }
                void clear() { if (!_exist()) return; _get(0)->clear(); }
                int getKey() { if (!_exist()) return 0; return _get(0)->getKey(); }
                bool useDefaultInputValue() { if (!_exist()) return false; return _get(0)->useDefaultInputValue(); }
                void useDefaultInputValue(bool newstatus) { if (!_exist()) return; _get(0)->useDefaultInputValue(newstatus); }
                void enableLogFile() { if (!_exist()) return; _get(0)->enableLogFile(); }
                void disableLogFile() { if (!_exist()) return; _get(0)->disableLogFile(); }
                void enableScreenOutput() { if (!_exist()) return; _get(0)->enableScreenOutput(); }
                void disableScreenOutput() { if (!_exist()) return; _get(0)->disableScreenOutput(); }
                void resize(int x, int y, int w, int h) { if (!_exist()) return; _get(0)->resize(x, y, w, h); }
                void move(int x, int y) { if (!_exist()) return; _get(0)->move(x, y); }

            private:

                bool _exist()
                    {
                    if (_get(0) == nullptr) { MTOOLS_DEBUG("CoutConsole method called after object was destroyed!"); return false; }
                    return true;
                    }

                Console * _get(int);
                CoutConsole(const CoutConsole&) = delete;
                CoutConsole & operator=(const CoutConsole&) = delete;
            };





        }

 
  
#ifndef MTOOLS_BASIC_CONSOLE
    /* 'advanced' FLTK cout console */
    static internals_console::CoutConsole cout; ///< static object redirecting to the "cout" Console present in each compilation unit containing console.hpp. 
#else
    /* 'basic' default stdout console */
    static internals_console::CoutConsoleBasic cout; ///< static object redirecting to the "cout" Console present in each compilation unit containing console.hpp. 
#endif


}


/* end of file */

