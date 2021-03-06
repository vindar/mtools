/** @file console.cpp */
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


#include "io/internal/fltkSupervisor.hpp"
#include "misc/stringfct.hpp"
#include "misc/indirectcall.hpp"
#include "graphics/rgbc.hpp"
#include "io/console.hpp"

#if defined(_WIN32)
#define MTOOLS_HASCONIO
#elif defined(__linux__) || defined(__unix__) || defined(_POSIX_VERSION)
#define MTOOLS_HASUNISTD
#endif

#include <cstdio>
#if defined(MTOOLS_HASCONIO)
#include <conio.h>
#include <windows.h>
#elif defined(MTOOLS_HASUNISTD)
#include <termios.h>
#include <unistd.h>
#endif


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include <iostream>

namespace mtools
{




    namespace internals_console
    {



        /**
         * Subclassing of Fl_Double_Window. The Console widget.
         *
         * Methods of ConsoleWidget are called only from the FLTK thread.
         **/
        class ConsoleWidget : public Fl_Double_Window
        {
            public:

            /* constructor of the widget */
            ConsoleWidget(int X, int Y, int W, int H, std::string * name, std::string * wt, std::mutex * m, std::atomic<size_t> * t) : Fl_Double_Window(X, Y, W, H), waiting_text(*wt), mutext(*m), tl(*t), inputOn(false), keyed(0)
                {

                buff = new Fl_Text_Buffer(100000);  // create the text buffer
                copy_label(name->c_str()); // the console title
    
                begin(); // begin adding child widgets 

                disp = new Fl_Text_Display(5, 5, w() - 10, h() - 40); // the text display widget
                disp->buffer(buff);                       //
                disp->textfont(FL_COURIER);               // font and size of the text
                disp->textsize(12);                       //
                disp->wrap_mode(disp->WRAP_AT_BOUNDS, 1);       // we wrap lines
                disp->linenumber_size(10);                // line numbering
                disp->linenumber_format("%d");            //
                disp->linenumber_align(FL_ALIGN_RIGHT);   //
                disp->linenumber_font(FL_TIMES);        //
                disp->linenumber_bgcolor(FL_GRAY);        //
                disp->linenumber_width(40);               //

                subBox = new Fl_Window(5, h() - 30, w() - 10, 25);
                subBox->begin();
                pressText = new Fl_Box(0, 0, subBox->w() - 35, 25,"Press a key...");
                pressText->labelcolor(FL_RED);
                pressText->labelfont(FL_BOLD);
                pressText->labelsize(12);
                input = new Fl_Input(0, 0, subBox->w() - 35, 25);
                bscroll = new Fl_Toggle_Button(subBox->w() - 30, 0,30, 25, "scroll\nlock");
                bscroll->labelfont(FL_BOLD);
                bscroll->labelsize(8);
                bscroll->value(1);
                subBox->resizable(input);
                subBox->end();
                end();
                resizable(disp);
                callback(static_windows_callback, this);
                Fl::add_timeout(0.05,static_timer_callback,this);
                input->hide();
                pressText->hide();
                drawn = 1;
                show();
                }

            /* destructor */
            ~ConsoleWidget()
                {
                removeTimer();
                remove(disp);
                delete disp;
                delete buff;

                }

            /* Handle events in the Fl_Double_Window */
            virtual int handle(int e)
                {
                switch (e)
                    {
                    case FL_SHORTCUT:
                        {
                        if (Fl::event_key() == FL_Escape) { return 1; }  // prevent ESCAPE from closing the window
                        break;
                        }
                    case FL_KEYDOWN:
                        {
                        // keyed = Fl::event_key();
                        break;
                        }
                    case FL_KEYUP:
                        {
                        keyed = Fl::event_key();
                        if (keyed == FL_Enter)
                            {
                            if (inputOn == true)
                                {
                                inputText = input->value();
                                entered++;
                                return 1;
                                }
                            }
                        break;
                        }
                    }
                return(Fl_Double_Window::handle(e));
                }

            void chsize(int x, int y, int w, int h) { resize(x, y, w, h); }

            void chpos(int x, int y) { resize(x, y, w(), h()); }

            void startGetKey()
                {
                input->hide();
                subBox->resizable(pressText);
                pressText->resize(0, 0, subBox->w() - 35, 25);
                pressText->show();
                keyed = 0;
                take_focus();
                }

            void endGetKey()
                {
                pressText->hide();
                }

            void startInput(const std::string * str)
                {
                entered = 0;
                inputOn = true;
                input->value((str == nullptr) ? "" : str->c_str());
                subBox->resizable(input);
                input->resize(0, 0, subBox->w() - 35, 25);
                input->show();
                take_focus();
                input->take_focus();
                }

            void endInput()
                {
                input->value("");
                input->hide();
                inputOn = false;
                }

            /* static callback, redirect to windows_callback */
            static void static_windows_callback(Fl_Widget* W, void* p) { if (p == nullptr) { return; } ((ConsoleWidget*)p)->window_callback(W); }

            /* ask whether we should force quit the program */
            void window_callback(Fl_Widget* W)
                {
                if (fl_choice("Do you want to quit?\n Choosing [Yes] will abort the process...", "No", "Yes", nullptr) == 1)
                    {
                    mtools::fltkExit(0);
                    }
                return;
                }

            /* clear the screen */
            void clearScreen() { buff->text(nullptr); }

            /* static callback, redirect to timer_callback */
            static void static_timer_callback(void* p) { if (p == nullptr) { return; } ((ConsoleWidget*)p)->window_timer(); }

            /* timer used to check for new text to add in the console */
            void window_timer()
                {
                if ((tl != 0)&&(drawn != 0)) // check if there is something to print to the screen and the screen was already updated from last time
                    {
                        {
                        std::lock_guard<std::mutex> lock(mutext);    // mutext protected swap to get the
                        copy_text.swap(waiting_text);                // content of nexttext into nexttext2 and
                        tl = 0;                                     // erase nextex
                        waiting_text.resize(0);                     //
                        }
                    drawn = 0;
                    disp->buffer()->append(copy_text.c_str());

                    if (bscroll->value() != 0)
                        {
                        disp->insert_position(disp->buffer()->length());
                        disp->show_insert_position();
                        }
                    }
                else { redraw(); }
                Fl::repeat_timeout(0.05, static_timer_callback, this); // refresh 20 times per second, if possible...
                return;
                }

            virtual void draw()
                {
                Fl_Double_Window::draw();   // call base method
                drawn++;                    // and then indicate that a redraw as taken place
                }

            void removeTimer()
                {
                Fl::remove_timeout(static_timer_callback, this); // remove the timer
                }

            public:

            std::atomic<int>      drawn;
            std::string           copy_text;
            std::string &         waiting_text;
            std::mutex &          mutext;
            std::atomic<size_t> & tl;

            std::atomic<bool>     inputOn;
            std::atomic<int>      entered;
            std::string           inputText;
            std::atomic<int>      keyed;

            private:

            ConsoleWidget(const ConsoleWidget &) = delete;              // no copy
            ConsoleWidget & operator=(const ConsoleWidget &) = delete;  //

            Fl_Text_Display *   disp;  //  the main text window
            Fl_Input *          input; //  the text input widget
            Fl_Toggle_Button *  bscroll;    // the scroll on/off button
            Fl_Box *            pressText; // press a key text.
            Fl_Window *         subBox; // the box containing the imput widget, scroll button and getKey text.
            Fl_Text_Buffer *    buff;    // text buffer associated with the text display widget

        };



    }



    int Console::_consoleNumber = 0;





    Console::Console(const std::string & name, bool showAtCreation) :  _waiting_text(), _tl(0), _CW(nullptr),  _disabled(0), _enableLogging(true), _enableScreen(true), _showDefaultInputValue(false), _consoleName(name), _logfile(nullptr)
        {        
        if (showAtCreation) { _logfile = new LogFile(_consoleName + ".txt"); _startProtect(); _endProtect(); }
        }


    Console::Console() : _waiting_text(), _tl(0), _CW(nullptr), _disabled(0) , _enableLogging(true), _enableScreen(true), _showDefaultInputValue(false)
        {
        ++_consoleNumber;
        _consoleName = std::string("Console-") + toString(_consoleNumber);
        _logfile = new LogFile(_consoleName + ".txt");
        }


    Console::~Console()
        {
        _disableConsole();
        }


    void Console::_disableConsole()
        {
        if (_CW == (internals_console::ConsoleWidget *)nullptr) { _disabled = 3;  return; }
        _disabled = 1;
        ((internals_console::ConsoleWidget *)_CW)->removeTimer(); // remove the timer
        _disabled = 2;
        _mustop.lock();
        mtools::deleteInFltkThread<internals_console::ConsoleWidget>(_CW);
        _CW = nullptr;
        delete _logfile;
        _logfile = nullptr;
        _mustop.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }


    inline void Console::_makeWindow()
        {
        if (_CW == (internals_console::ConsoleWidget *)nullptr)  { _CW = mtools::newInFltkThread<internals_console::ConsoleWidget, int, int, int, int, std::string *, std::string *, std::mutex *, std::atomic<size_t> *>(0, 0, 650, 400, &_consoleName, &_waiting_text, &_mutext, &_tl); }
        }


    void Console::clear()
        {
        if ((fltkThreadStopped()) || (!_startProtect())) { return; }
        if (_enableScreen)
            {
            mtools::IndirectMemberProc<internals_console::ConsoleWidget> proxy(*_CW, &internals_console::ConsoleWidget::clearScreen);
            mtools::runInFltkThread(proxy);
            }
        if (_enableLogging)
            {
            if (_logfile == nullptr) _logfile = new LogFile(_consoleName + ".txt");
            _logfile->operator<<("\n\n************************** CLEAR ****************************\n\n");
            }
        _endProtect();
        }


    void Console::resize(int x, int y, int w, int h)
        {
        if ((fltkThreadStopped()) || (!_startProtect())) { return; }
        mtools::IndirectMemberProc<internals_console::ConsoleWidget,int,int,int,int> proxy(*_CW, &internals_console::ConsoleWidget::chsize,x,y,w,h);
        mtools::runInFltkThread(proxy);
        _endProtect();
        }


    void Console::move(int x, int y)
        {
        if ((fltkThreadStopped()) || (!_startProtect())) { return; }
        mtools::IndirectMemberProc<internals_console::ConsoleWidget,int,int> proxy(*_CW, &internals_console::ConsoleWidget::chpos,x,y);
        mtools::runInFltkThread(proxy);
        _endProtect();
        }


    void Console::_print(const std::string & s)
        {
        if ((fltkThreadStopped()) || (!_startProtect())) { return; }
        std::string us = mtools::toUtf8(s);
        if (us.length() != 0)
            {
            if (_enableScreen)
                {
                std::lock_guard<std::mutex> lock(_mutext);
                _waiting_text += us;
                _tl = _waiting_text.size();
                }
            if (_enableLogging)
                {
                if (_logfile == nullptr) _logfile = new LogFile(_consoleName + ".txt");
                _logfile->operator<<(us);
                }
            }
        int i =  0;
        while ((_tl > 16383) && (i < 20)) { i++;  std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // more than 16KB of text to write, wait a little to let the thread catch up...
        _endProtect();
        }

    
    std::string Console::_getText(const std::string & initText)
        {
        if ((fltkThreadStopped())||(!_startProtect())) { return std::string(""); }
        mtools::IndirectMemberProc<internals_console::ConsoleWidget, const std::string *> proxy1(*_CW, &internals_console::ConsoleWidget::startInput, &initText);
        mtools::runInFltkThread(proxy1);
        while (((internals_console::ConsoleWidget*)(_CW))->entered == 0)
            {
            if (_disabled > 0) { _endProtect(); std::this_thread::sleep_for(std::chrono::milliseconds(50)); return std::string(""); }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        std::string res = ((internals_console::ConsoleWidget*)(_CW))->inputText;
        mtools::IndirectMemberProc<internals_console::ConsoleWidget> proxy2(*_CW, &internals_console::ConsoleWidget::endInput);
        mtools::runInFltkThread(proxy2);
        _endProtect();
        return res;
        }



    Console & Console::operator>>(bool & b)
        {
        while (1)
            {
            if (fltkThreadStopped()) { return (*this); }
            int k = getKey();
            if ((k == 'O') || (k == 'o') || (k == 'Y') || (k == 'y') || (k == '1')) { b = true; return *this;; }
            if ((k == 'N') || (k == 'n') || (k == 27)) { b = false; return *this;; }
            }
        }


    Console & Console::operator>>(char & c)
        {
        while(1)
            {
            if (fltkThreadStopped()) { return (*this); }
            int k = getKey();
            if (k < 256) { c = (char)k; return(*this); }
            }
        }


    int Console::getKey()
        {
        if ((fltkThreadStopped()) || (!_startProtect())) { return 0; }
        mtools::IndirectMemberProc<internals_console::ConsoleWidget> proxy1(*_CW, &internals_console::ConsoleWidget::startGetKey);
        mtools::runInFltkThread(proxy1);
        while (((internals_console::ConsoleWidget*)(_CW))->keyed == 0)
            {
            if (_disabled > 0) { _endProtect(); std::this_thread::sleep_for(std::chrono::milliseconds(50)); return 0; }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        int res = ((internals_console::ConsoleWidget*)(_CW))->keyed;
        mtools::IndirectMemberProc<internals_console::ConsoleWidget> proxy2(*_CW, &internals_console::ConsoleWidget::endGetKey);
        mtools::runInFltkThread(proxy2);
        _endProtect();
        return res;
        }


    inline bool Console::_startProtect()
        {
        if (_disabled > 0)  { return false; }
        _mustop.lock();
        if (_disabled > 0) { _mustop.unlock(); return false; }
        _makeWindow();
        return true;
        }


    inline void Console::_endProtect()
        {
        _mustop.unlock();
        }


    void hideMSConsole()
        {
        #if defined (_MSC_VER)
        HWND hwnd = GetConsoleWindow();
        if (hwnd != 0) { ShowWindow(hwnd, SW_HIDE); }
        #endif
        }


    void showMSConsole()
        {
        #if defined (_MSC_VER)
        HWND hwnd = GetConsoleWindow();
        if (hwnd != 0) { ShowWindow(hwnd, SW_SHOW); }
        #endif
        }



    namespace internals_console
    {

    ConsoleBasic::ConsoleBasic(const std::string & name) : _enableLogging(true), _enableScreen(true), _showDefaultInputValue(false), _consoleName(name), _logfile(nullptr)
        {
        }

    ConsoleBasic::~ConsoleBasic()
        {
        delete _logfile;
        }

    ConsoleBasic & ConsoleBasic::operator>>(bool & b)
        {
        while (1)
            {
            int k = getKey();
            if ((k == 'O') || (k == 'o') || (k == 'Y') || (k == 'y') || (k == '1')) { b = true; return *this;; }
            if ((k == 'N') || (k == 'n') || (k == 27)) { b = false; return *this;; }
            }
        }

    ConsoleBasic & ConsoleBasic::operator>>(char & c)
        {
        while (1)
            {
            int k = getKey();
            if (k < 256) { c = (char)k; return(*this); }
            }
        }

    /* get a char without echo (if possible) */ 
    int ConsoleBasic::getKey()
        {
        int ch = 0;
        #if defined(MTOOLS_HASUNISTD)
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt; newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        if (ch == 27) { ch = getchar() + 65536; }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        #elif defined(MTOOLS_HASCONIO)
        ch = _getch();
        if ((ch == 0) || (ch == 0xE0)) { return(_getch() + 66536); }
        #else
        ch = std::getchar(); // fallback
        #endif
        if (ch == 10) { ch = 13; }  // map 10 and 13 to 13 (enter key)
        if (ch == 127) { ch = 8; }  // map 8 and 127 to 8 (backspace key)
        return ch;
        }

    /* get a text without and remove it afterward */
    std::string ConsoleBasic::_getText(const std::string & initText)
        {
        std::string s(initText);
        if (s.length() > 0) { std::cout << s; }
        int ch;
        do
            {
            ch = getKey();
            if ((ch >= 32) && (ch <= 126)) {s+=(char)ch; std::cout << (char)ch;}
            if ((ch == 8) || (ch == 127)) {if (s.length()>0) {s.resize(s.length()-1); std::cout << "\b \b";}}
            }
        while ((ch != 10) && (ch != 13));
        for(size_t i=0;i< s.length(); i++) {std::cout << "\b \b";}
        return s;
        }


    void ConsoleBasic::_print(const std::string & s)
        {
        if (_enableLogging) 
            { 
            if (_logfile == nullptr) _logfile = new LogFile(_consoleName + ".txt");
            _logfile->operator<<(s); 
            }
        if (_enableScreen) { std::cout << s; }
        }


        /* keep tracks of the number of CoutConsole objects, create the one and only "cout" Console
         * when the first CoutConsole object is created and delete it when the last CoutConsole is deleted. */
        Console * CoutConsole::_get(int mode)
            {
            static std::atomic<int> init((int)0);  // using local static variables : no initialization problem !
            static Console * pcout = nullptr;
            if (mode > 0)
                {
                if (init++ == 0) { MTOOLS_DEBUG("Creating the global FLTK cout console."); pcout = new mtools::Console("cout", false); } // first time, create the console
                }
            if (mode < 0)
                {
                if (--init == 0) { MTOOLS_DEBUG("Destroying the global FLTK cout console."); pcout->_disableConsole(); pcout = nullptr;  } // last one, do not delete but disable it...
                }
            return pcout; // mode = 0, just return a pointer to cout
            }


        ConsoleBasic * CoutConsoleBasic::_get(int mode)
            {
            static std::atomic<int> init((int)0);  // using local static variables : no initialization problem !
            static std::atomic<ConsoleBasic *> pcout((ConsoleBasic *)nullptr);
            if (mode > 0)
                {
                if (init++ == 0) { MTOOLS_DEBUG("Creating the global FLTK cout console (basic)."); pcout = new ConsoleBasic("cout"); } // first time, create the console
                }
            if (mode < 0)
                {
                if (--init == 0) { MTOOLS_DEBUG("Destroying the global FLTK cout console (basic)."); ConsoleBasic * p = pcout; pcout = (ConsoleBasic *)nullptr; std::this_thread::yield(); delete p; } // last one, delete the console
                }
            return pcout; // mode = 0, just return a pointer to cout
            }

    }


}


/* end of file */

