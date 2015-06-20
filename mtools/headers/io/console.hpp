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


#pragma once


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
            std::string s;
            while (1)
                {
                s = _getText();
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


        std::string _getText();
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

        std::string         _consoleName;
        mtools::LogFile *   _logfile;

        static int          _consoleNumber;

        friend class mtools::internals_console::CoutConsole;

    };



    namespace internals_console
    {


    /**
     * Wrapper class for the global "cout" console.
     **/
    class CoutConsole
        {
            public:
            CoutConsole()  { _get(1); }
            ~CoutConsole() { _get(-1); }
            template<typename T> CoutConsole & operator<<(const T & O) { _get(0)->operator<<(O); return(*this); }
            template<typename T> CoutConsole & operator>>(T & O) { _get(0)->operator>>(O); return(*this); }
            void clear() { _get(0)->clear(); }
            int getKey() { return _get(0)->getKey(); }
            void enableLogFile() { _get(0)->enableLogFile(); }
            void disableLogFile() { _get(0)->disableLogFile(); }
            void enableScreenOutput() { _get(0)->enableScreenOutput(); }
            void disableScreenOutput() { _get(0)->disableScreenOutput(); }
            void resize(int x, int y, int w, int h) { _get(0)->resize(x,y,w,h); }
            void move(int x, int y) { _get(0)->move(x, y); }
            private:
            Console * _get(int);
            CoutConsole(const CoutConsole&) = delete;
            CoutConsole & operator=(const CoutConsole&) = delete;
        };

    }

    static internals_console::CoutConsole cout; ///< static object redirecting to the "cout" Console present in each compilation unit containing console.hpp. 



}


/* end of file */

