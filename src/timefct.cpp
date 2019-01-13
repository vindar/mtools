/** @file timefct.cpp */
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


#include "misc/timefct.hpp"
#include "misc/indirectcall.hpp"
#include "io/internal/fltkSupervisor.hpp"
#include "misc/stringfct.hpp"


#include <random>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#endif



#include <FL/Fl.H>
#include <FL/Fl_Window.H>
//#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
//#include <FL/Fl_Input.H>
//#include <FL/Fl_Button.H>
//#include <FL/Fl_Check_Button.H>
//#include <FL/Fl_Round_Button.H>
//#include <FL/Fl_Toggle_Button.H>
//#include <FL/Fl_Value_Slider.H>
//#include <FL/Fl_Counter.H>
//#include <FL/Fl_Scroll.H>
#include <FL/Fl_Progress.H>
//#include <FL/Fl_Pack.H>
//#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>
//#include <FL/Fl_Color_Chooser.H> 
//#include <FL/Fl_File_Chooser.H> 
//#include <FL/filename.H>
//#include <FL/fl_draw.H>

#include <thread>
#include <mutex>
#include <atomic>

namespace mtools
{


    size_t randomID()
        {
	static std::mutex mut;
	static std::atomic<int> counter((int)345); 
		
	std::lock_guard<std::mutex> lockmut(mut); // lock the mutex
	counter.fetch_add(7);
		
        size_t h[5];		
		
        std::random_device rd;		
        std::hash<std::random_device::result_type> hash_rd;
        h[0] = hash_rd(rd());      // try to get a real random number 
		
        std::hash<std::time_t> hash_time;
        h[1] = hash_time(time(nullptr));  // number of seconds since 1970
		
        std::hash<std::thread::id> hash_thread;
        h[2] = hash_thread(std::this_thread::get_id());  // the thread id
		
        h[3] = (int)counter; // use counter
		
    #if defined(__linux__) || defined(__APPLE__)
        auto pid = getpid();  // process id on linux
    #elif _WIN32
        auto pid = GetCurrentProcessId();   // process id on windows
    #endif
        std::hash<decltype(pid)> hash_pid;
        h[4] = hash_pid(pid);  // process id
		
		OSS os; 
		os << h[0] << "_" << h[1] << "_" << h[2] <<  "_" << h[3] <<  "_" << h[4];
        std::hash<std::string> hash_str;
        return hash_str(os.str());
        }




    uint64 Chronometer()
        {
        static std::chrono::high_resolution_clock::time_point prev = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point next = std::chrono::high_resolution_clock::now();
        std::chrono::duration<long double> elapsed = std::chrono::duration_cast<std::chrono::duration<long double>>(next - prev);
        prev = next;
        return (uint64)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        }


    std::string durationToString(uint64 milliseconds, bool printMilliseconds)
        {
		OSS os;
        uint64 days = milliseconds / (1000 * 60 * 60 * 24);
        milliseconds = milliseconds % (1000 * 60 * 60 * 24);
        uint64 hours = milliseconds / (1000 * 60 * 60);
        milliseconds = milliseconds % (1000 * 60 * 60);
        uint64 min = milliseconds / (1000 * 60);
        milliseconds = milliseconds % (1000 * 60);
        uint64 sec = milliseconds / 1000;
        milliseconds = milliseconds % 1000;
        if (days)  { os << days << ((days == 1) ? " day " : " days "); }
        if (hours) { os << hours << ((hours == 1) ? " hour " : " hours "); }
        if (min)   { os << min << " min. "; }
        if (sec)   { os << sec << " sec. "; }
        if ((printMilliseconds) && (milliseconds)) { os << milliseconds << " ms. "; }
        return os.str();
        }



	Chrono::Chrono()
		{
		reset();
		}


	void Chrono::reset()
		{
		_t = std::chrono::high_resolution_clock::now();
		}


	uint64 Chrono::elapsed()
		{
		std::chrono::high_resolution_clock::time_point n = std::chrono::high_resolution_clock::now();
		std::chrono::duration<long double> e = std::chrono::duration_cast<std::chrono::duration<long double>>(n - _t);
		return (uint64)std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
		}


	std::string Chrono::toString()
		{
		uint64 e = elapsed();
		return durationToString(e, (e < 61000));
		}


	/**
	* Return a chrono object. (Convinience function).
	**/
	Chrono chrono() { return Chrono(); }



    namespace internals_timefct
    {

        class ProgressWidget : public Fl_Window
        {
            public:


            /* constructor of the widget */
            ProgressWidget(bool sht,const char * tit) : Fl_Window(0,0,300,110), showtime(sht), newval(0), updatetime(0)
                {
                startTime = std::chrono::high_resolution_clock::now();
                resize((Fl::w() - 300) / 2, (Fl::h() -110) / 2, 300, 110);
                size_range(300, 110, 300, 110);
                copy_label(tit);
                begin();
                progBar = new Fl_Progress(10,20, 280, 30);
                progBar->minimum(0.0);
                progBar->maximum(1.0);
                progBar->value(0.0);
                progBar->color(fl_darker(FL_GRAY));
                progBar->selection_color(FL_BLUE);
                progBar->labelcolor(FL_WHITE);
                textBar1 = new Fl_Box(10, 60, 280, 15);
                textBar1->labelsize(10);
                textBar1->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                textBar2 = new Fl_Box(10, 75, 280, 15);
                textBar2->labelsize(10);
                textBar2->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                end();
                callback(static_windows_callback, this);
                Fl::add_timeout(0.05, static_timer_callback, this);
                show();
                }

            ~ProgressWidget()
                {
                Fl::remove_timeout(static_timer_callback, this);
                }

            /* Handle events */
            virtual int handle(int e)
                {
                switch (e)
                    {
                    case FL_SHORTCUT:
                        {
                        if (Fl::event_key() == FL_Escape) { return 1; }  // prevent ESCAPE from closing the window
                        break;
                        }
                    }
                return(Fl_Window::handle(e));
                }

            /* static callback, redirect to windows_callback */
            static void static_windows_callback(Fl_Widget* W, void* p) { if (p == nullptr) { return; } ((ProgressWidget*)p)->window_callback(W); }

            /* ask whether we should force quit the program */
            void window_callback(Fl_Widget* W)
                {
                if (fl_choice("Do you want to quit?\n Choosing YES will abort the process...", "YES", "NO", nullptr) == 0)
                    {
                    mtools::fltkExit(0);
                    }
                return;
                }

            /* static callback, redirect to timer_callback */
            static void static_timer_callback(void* p) { if (p == nullptr) { return; } ((ProgressWidget*)p)->window_timer(); }

            /* timer function */
            void window_timer()
                {
                if (progBar->value() != (float)newval)
                    {
                    progBar->value((float)newval); progBar->redraw();
                    auto text_pourcentage = mtools::toString((int)(100 * newval)) + "%";
                    progBar->copy_label(text_pourcentage.c_str());
                    progBar->redraw_label();
                    if (shown()&&(newval > 1.0)) { hide(); } else if ((!shown())&& (newval <= 1.0)) { show(); }
                    }
                if (showtime)
                    {
                    updatetime++;
                    if (updatetime > 15)
                        {
                        updatetime = 0;
                        auto e = std::chrono::duration_cast<std::chrono::duration<long double>>(std::chrono::high_resolution_clock::now() - startTime);
                        uint64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
                        uint64 remain = ((newval < 0.0000001)||(newval>=1.0)) ? 0 : (uint64)(((1.0 - newval) / newval)*elapsed);
                        auto text_elapsed = std::string("Elapsed: ") + durationToString(elapsed + 999, false);
                        auto text_remaining = std::string("Remaining: ") + durationToString(remain + 999, false);
                        textBar1->copy_label(text_elapsed.c_str());
                        textBar2->copy_label(text_remaining.c_str());
                        }
                    }
                Fl::repeat_timeout(0.05, static_timer_callback, this); // refresh 20 times per second, if possible...
                return;
                }


            private:

            friend void setProgressWidgetValue(ProgressWidget *, double);
            friend void deleteProgressWidget(ProgressWidget *);

            ProgressWidget(const ProgressWidget &) = delete;              // no copy
            ProgressWidget & operator=(const ProgressWidget &) = delete;  //

            bool                showtime;   // show remaining time
            Fl_Progress *       progBar;    // the progress bar widget.
            Fl_Box *            textBar1;   // elapsed time widget
            Fl_Box *            textBar2;   // remaining time widget
            std::atomic<double> newval;     // new value

            std::chrono::high_resolution_clock::time_point startTime; // time when the progress bar was created.
            int updatetime; // when we should update remaining time
        };


        ProgressWidget * makeProgressWidget(bool sh, const std::string & name)
            {
             return newInFltkThread<ProgressWidget, bool&, const char *>(sh, name.c_str());
            }


        void setProgressWidgetValue(ProgressWidget * PW, double val)
            {
            PW->newval = val;
            }


        void deleteProgressWidget(ProgressWidget * PW)
            {
            deleteInFltkThread<ProgressWidget>(PW);
            }



    }







}
/* end of file */
