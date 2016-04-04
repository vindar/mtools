/** @file threadworker.hpp */
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

#include "../misc/misc.hpp"
#include "../misc/error.hpp"

#include <ctime>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mtools
{

    /**
     * Return the number of hardware threads.
     *
     * @return  Number of hardware threads. 1 if not detected. 
     **/
    inline int nbHardwareThreads()
        {
        unsigned int nb = std::thread::hardware_concurrency();
        if (nb == 0) return 1;
        return (int)nb;
        }


    /**
    * Class used for creating a simple worker thread.
    *
    * Two virtual methods must be overidden:
    *
    * - work() : performs the thread's work()
    * - message(int64 code) : process incomming messages and determine how the thread should behave.
    * 
    */
    class ThreadWorker
        {

        public:

            /**
            * Constructor.
            * The thread is initially disabled and its work status set to false (i.e. waiting).
            **/
            ThreadWorker() :
                _progress(PROGRESS_NONE),
                _thread_status(false),
                _work_status(false),
                _msg(MSG_NONE),
                _code(0)
                {
                _th = new std::thread(&ThreadWorker::_threadProc, this);
                sync();
                }


            /** Destructor. Stop the thread. */
            virtual ~ThreadWorker()
                {
                sync();
                _signal(MSG_QUIT);
                _th->join();
                delete _th;
                }


            /**
            * Return the current progress value.
            **/
            inline int progress() const { return _progress; }


            /**
            * Enables/Disable the thread. A disable thread can still process signals but cannot perform any
            * work.
            *
            * Returns immediately. Call sync() to wait for the completion of the command.
            *
            * @param   newstatus   true to enable and false to disable the thread.
            **/
            void enable(bool newstatus)
                {
                sync();
                if (newstatus == (bool)_thread_status) return;
                _signal((newstatus ? MSG_ENABLE : MSG_DISABLE));
                }


            /**
            * Query if the thread is currently enabled.
            **/
            inline bool enable() const { return (bool)_thread_status; }


            /** Query the work status: true for work on and false for wait on. */
            inline bool workStatus() const { return (bool)_work_status; }


            /**
            * True if there is no pending command and false if there is one pending.
            **/
            inline bool ready() const { return(((int)_msg) == MSG_NONE); }


            /** Wait for the completion of any pending command. */
            void sync()
                {
                if (((int)_msg) == MSG_NONE) return;
                std::unique_lock<std::mutex> lock(_mut_wait);
                while (((int)_msg) != MSG_NONE) { _cv_wait.wait_for(lock,std::chrono::milliseconds(1)); }
                }


            /**
            * Send a signal to the thread.
            *
            * Wait for previous command to finish before sending the new one then return immediately after
            * sending the new command. Called sync()  to wait for the completion of the new command.
            *
            * @param   code    The code.
            **/
            void signal(int64 code)
                {
                sync();
                _signal(MSG_CODE, code);
                }


        protected:

            static const int THREAD_CONTINUE = 9;
            static const int THREAD_RESET = 10;
            static const int THREAD_WAIT = 11;
            static const int THREAD_RESET_AND_WAIT = 12;


            /**
            * Sets the value of the current work progress.
            **/
            inline void setProgress(int val) { _progress = val; }


            /**
             * Checks if there are any pending signal and process it if needed.
             * 
             * !!! This method is fast. It must be called regularly inside work() to keep the thread
             * responsive !!!
             **/
            inline void check() { if (((int)_msg) == MSG_NONE) { return; } _processInside(); }


            /**
            * Pure virtual method that performs the thread's work.
            * Must be overloaded in derived class.
            *
            * The method must call check() regularly in order to process messages. The check() method will
            * throw an exception if work must stop. Do not catch it !
            **/
            virtual void work() = 0;


            /**
            * Pure virtual method called when there is a message to process.
            * Must be overloaded in derived class.
            *
            * @param   code    The message code
            *
            * @return  Determine how work should continue from now on. One of
            *          THREAD_CONTINUE : continue work as before.
            *          THREAD_RESET : restart the work() method
            *          THREAD_WAIT : wait for an other message before continuing to work.
            *          THREAD_RESET_AND_WAIT: exit the work() method and wait for another message.
            **/
            virtual int message(int64 code) = 0;



        private:


            // no copy
            ThreadWorker(const ThreadWorker &) = delete;
            ThreadWorker & operator=(const ThreadWorker &) = delete;

            std::atomic<int>    _progress;          // progress indicator
            std::atomic<bool>   _thread_status;     // true id the thread is enabled and false otherwise
            std::atomic<bool>   _work_status;       // true if work on and false if waiting

            std::atomic<int>    _msg;               // message type, one of MSG_NONE, MSG_CODE, MSG_QUIT, MSG_ENABLE, MSG_DISABLE
            std::atomic<int64>  _code;              // code used for communication

            std::condition_variable _cv_wakeup;     // used for waking up the thread
            std::mutex _mut_wakeup;                 // associated condition variable.
            std::condition_variable _cv_wait;       // used for waiting for the thread to answer
            std::mutex _mut_wait;                   // associated condition variable.
            std::thread * _th;                      // the thread object.


            static const int PROGRESS_NONE = 0;

            static const int MSG_NONE = 4;
            static const int MSG_CODE = 5;
            static const int MSG_ENABLE = 6;
            static const int MSG_DISABLE = 7;
            static const int MSG_QUIT = 8;

            static const int64 CODE_NONE = 0;


            /* send a signal to the thread */
            void _signal(int msg, int64 code = CODE_NONE)
                {
                MTOOLS_ASSERT(((int)_msg) == MSG_NONE);
                std::unique_lock<std::mutex> lock(_mut_wakeup);
                _code = code;
                _msg = msg;
                _cv_wakeup.notify_one();
                }


            /* put the thread to sleep, return when there is a message */
            void _threadSleep()
                {
                if (((int)_msg) != MSG_NONE) return;
                std::unique_lock<std::mutex> lock(_mut_wakeup);
                while (((int)_msg) == MSG_NONE) { _cv_wakeup.wait_for(lock, std::chrono::milliseconds(10)); }
                }


            /* indicate that the thread is ready */
            void _threadReady()
                {
                MTOOLS_ASSERT(((int)_msg) != MSG_NONE);
                std::unique_lock<std::mutex> lock(_mut_wait);
                _msg = MSG_NONE;
                _code = CODE_NONE;
                _cv_wait.notify_one();
                }


            /* the thread procedure */
            void _threadProc()
                {

            wait_label:
                _threadSleep();
                switch ((int)_msg)
                    {
                    case MSG_ENABLE: { _thread_status = true; break; }
                    case MSG_DISABLE: { _thread_status = false; break; }
                    case MSG_QUIT: { _threadReady(); return; }
                    case MSG_CODE:
                        {
                        int r = message(_code);
                        switch (r)
                            {
                            case THREAD_CONTINUE: { break; }
                            case THREAD_RESET: { _work_status = true; break; }
                            case THREAD_WAIT: { _work_status = false; break; }
                            case THREAD_RESET_AND_WAIT: { _work_status = false; break; }
                            default: { MTOOLS_ERROR("wtf?"); }
                            }
                        break;
                        }
                    default: { MTOOLS_ERROR("wtf?"); }
                    }
                _threadReady();

            work_label:
                if ((!((bool)_thread_status)) || (!((bool)_work_status))) goto wait_label;
                try
                    {
                    work();
                    _work_status = false;
                    goto wait_label;
                    }
                catch (int r)
                    {
                    if ((int)_msg == MSG_QUIT) { _threadReady(); return; }
                    switch (r)
                        {
                        case THREAD_RESET: { _work_status = true; _threadReady(); goto work_label; }
                        case THREAD_RESET_AND_WAIT: { _work_status = false; _threadReady(); goto wait_label; }
                        default: { MTOOLS_ERROR("wtf?"); }
                        }
                    }
                }


            /* process a message inside check() */
            void _processInside()
                {
                while (1)
                    {
                    switch ((int)_msg)
                        {
                        case MSG_ENABLE:
                            {
                            _thread_status = true;
                            if ((bool)_work_status) { _threadReady(); return; }
                            break;
                            }
                        case MSG_DISABLE: { _thread_status = false; break; }
                        case MSG_QUIT: { throw((int)0); }
                        case MSG_CODE:
                            {
                            int r = message(_code);
                            switch (r)
                                {
                                case THREAD_CONTINUE:
                                    {
                                    _work_status = true;
                                    if ((bool)_thread_status) { _threadReady(); return; }
                                    break;
                                    }
                                case THREAD_WAIT: { _work_status = false; break; }
                                case THREAD_RESET: { throw((int)r); }
                                case THREAD_RESET_AND_WAIT: { throw((int)r); }
                                default: { MTOOLS_ERROR("wtf?"); }
                                }
                            break;
                            }
                        default: { MTOOLS_ERROR("wtf?"); }
                        }
                    _threadReady();
                    _threadSleep();
                    }
                }

        };




}

/* end of file */