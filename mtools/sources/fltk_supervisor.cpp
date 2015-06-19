/** @file fltk_supervisor.cpp */
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
#include "misc/indirectcall.hpp"
#include "io/fltk_supervisor.hpp"
#include "misc/stringfct.hpp"



namespace mtools
{

    namespace internals_fltk_supervisor
    {


        /* simple message structure used for communication */
        struct Msg
            {
            Msg(void * pp,int t) : status(0), p(pp), type(t) {}    // ctor
            std::atomic<int>    status;             // status
            std::atomic<void *> p;                  // pointer to data
            std::atomic<int>    type;               // type
            static const int NEW_MSG = 0;
            static const int DELETE_MSG = 1;
            static const int RUN_MSG = 2;
            };


    /**
    * A very simple thread safe message list
    **/
    class SimpleMsgList
        {
        public:

        /**
         * Pushes a pointer at the back of the message list
         *
         * @param [in,out]  p   pointer to add.
         **/
        void pushTop(Msg * p)
            {
            std::lock_guard<std::mutex> lock(_mut);
            _msgList.push_back(p);
            }


        /**
         * Removes and returns the oldest object in the list.
         *
         * @return  nullptr if there is no object in the list. The corresponding pointer otherwise.
         **/
        Msg * popBack()
            {
            std::lock_guard<std::mutex> lock(_mut);
            if (_msgList.size() == 0) return nullptr;
            Msg * p = _msgList.front();
            _msgList.pop_front();
            return p;
            }


        /**
         * Current size of the list
         *
         * @return  the number of messages in the list
         **/
        size_t size() const
            {
            std::lock_guard<std::mutex> lock(_mut); // useless but it cannot hurt and it wont slow down too much anyway
            return _msgList.size();
            }

    private:

        std::list<Msg *> _msgList;
        mutable std::mutex _mut;
    };



        /* The FLTK supervisor class : singleton class that manage the fltk thread. */
        class FLTK_Supervisor
        {
        public:


            /* return true is the current thread is the calling thread */
            static bool isFLTKThread() { return getInst()->_isFLTKThread();}

            /* construct the object in the FLTK thread
             * should not be called from the FLTK thread */
            static void newInFLTKThread(IndirectCtor * proxy) { getInst()->_newInFLTKThread(proxy); }

            /* construct the object in the FLTK thread
             *should not be called from the FLTK thread */
            static void deleteInFLTKThread(IndirectDtor * proxy) { getInst()->_deleteInFLTKThread(proxy); }

            /* run the proxy method in the FLTK thread
             * should not be called from the FLTK thread */
            static void runInFLTKThread(IndirectCall * proxycall) { getInst()->_runInFLTKThread(proxycall); }

            /* force to exit the FLTK thread
             * must be called from WITHIN the FLTK thread */
            static void exitFLTK() { Fl::awake(&FLTK_Supervisor::force_exit_callback, nullptr); }

            /* return the number of object currently active in the FLTK thread */
            static int nbObject() { return getInst()->_nbObject(); }


        private:



            bool _isFLTKThread()
                {
                if (_stopped)    return false;
                if (_init == 0)  return false;
                if (!_active)    return false;
                if (_thid != std::this_thread::get_id()) return false;
                return true;
                }


            void _newInFLTKThread(IndirectCtor * proxy)
                {
                MTOOLS_DEBUG("Creating object " + toString(_counter) + " in FLTK thread.");
                if (isFLTKThread())
                    {
                    MTOOLS_DEBUG("  -- called from the FLTK thread");
                    proxy->construct();
                    ++_counter; // the new object has been successfully created.
                    MTOOLS_DEBUG("Construction completed.");
                    return;
                    }
                if (_stopped)
                    {
                    MTOOLS_DEBUG("!!!! Construction failed : the FLTK thread is stopped !!!!");
                    return;   // do nothing it the thread was already stopped
                    }
                if (_init++ == 0) { _startThread(); } else { _init = 1; } // start the thread if needed
                while (!_active) { std::this_thread::yield(); } // wait until the thread is ready
                int cc = 0;
                std::mutex tmpmut; // temporary mutex for condition variable
                std::unique_lock<std::mutex> lck(tmpmut);   // unique lock over the mutex, lock it
                Msg ms(proxy, Msg::NEW_MSG); // create the message
                _thMsgList.pushTop(&ms); // push it
                Fl::awake(&FLTK_Supervisor::_processMsgCB, nullptr); // inform fltk
                while (!ms.status)
                    {
                    _cv.wait_for(lck, std::chrono::milliseconds(10)); // wait for signalization of the condition variable (or time elapsed).
                    if (!ms.status) { Fl::awake(); }
                    if (_stopped) {MTOOLS_DEBUG("!!!! Construction failed : the FLTK thread stopped while waiting for an answer !!!!"); return;}
                    cc++; if (cc > 100) { MTOOLS_DEBUG("... hanging ..."); cc = 0; }
                    } // wait for the completion of the procedure
                ++_counter; // the new object has been successfully created.
                MTOOLS_DEBUG("Construction completed.");
                }


            /* delete the proxy object in the FLTK thread
             * should not be called from the FLTK thread */
            void _deleteInFLTKThread(IndirectDtor * proxy)
                {
                MTOOLS_DEBUG("Deleting object " + toString(_counter -1) + " in FLTK thread.");
                if (isFLTKThread())
                    {
                    MTOOLS_DEBUG("  -- deleteInFLTKThread called from the FLTK thread");
                    proxy->destroy();
                    MTOOLS_INSURE((--_counter) != 0); // we cannot stop the fltk thread from the fltk thread !
                    MTOOLS_DEBUG("Destruction completed.");
                    return;
                    }
                if (_stopped)
                    {
                    MTOOLS_DEBUG("!!!! Destruction failed : the FLTK thread is stopped !!!!");
                    return;   // do nothing it the thread was already stopped
                    }
                MTOOLS_ASSERT(_active);
                MTOOLS_ASSERT(_init > 0);
                MTOOLS_ASSERT(_counter > 0);
                int cc = 0;
                std::mutex tmpmut; // temporary mutex for condition variable
                std::unique_lock<std::mutex> lck(tmpmut);   // unique lock over the mutex, lock it
                Msg ms(proxy, Msg::DELETE_MSG); // create the message
                _thMsgList.pushTop(&ms); // push it
                Fl::awake(&FLTK_Supervisor::_processMsgCB, nullptr); // inform fltk
                while (!ms.status)
                    {
                    _cv.wait_for(lck, std::chrono::milliseconds(10)); // wait for signalization of the condition variable (or time elapsed).
                    if (!ms.status) { Fl::awake(); }
                    if (_stopped) { MTOOLS_DEBUG("!!!! Destruction failed : the FLTK thread stopped while waiting for an answer !!!!"); return; }
                    cc++; if (cc > 100) { MTOOLS_DEBUG("... hanging ..."); cc = 0; }
                    } // wait for the completion of the procedure
                if (--_counter == 0) { _stopThread(); }
                MTOOLS_DEBUG("Destruction completed.");
                }


            void _runInFLTKThread(IndirectCall * proxycall)
                {
                MTOOLS_DEBUG("Running a method in the FLTK thread...");
                if (isFLTKThread())
                    {
                    MTOOLS_DEBUG("  -- runInFLTKThread called from the FLTK thread");
                    proxycall->call();
                    MTOOLS_DEBUG(" ...Finished running the method.");
                    return;
                    }
                if (_stopped)
                    {
                    MTOOLS_DEBUG("!!!! run failed : the FLTK thread is stopped !!!!");
                    return;   // do nothing it the thread was already stopped
                    }
                if (_init++ == 0) { _startThread(); } else { _init = 1; } // start the thread if needed
                while (!_active) { std::this_thread::yield(); } // wait until the thread is ready  (skipped except the first ime)
                int cc = 0;
                std::mutex tmpmut; // temporary mutex for condition variable
                std::unique_lock<std::mutex> lck(tmpmut);   // unique lock over the mutex, lock it
                Msg ms(proxycall, Msg::RUN_MSG); // create the message
                _thMsgList.pushTop(&ms); // push it
                Fl::awake(&FLTK_Supervisor::_processMsgCB, nullptr); // inform fltk
                while (!ms.status)
                    {
                    _cv.wait_for(lck, std::chrono::milliseconds(10)); // wait for signalization of the condition variable (or time elapsed).
                    if (!ms.status) { Fl::awake(); }
                    if (_stopped) { MTOOLS_DEBUG("!!!! run failed : the FLTK thread stopped while waiting for an answer !!!!"); return; }
                    cc++; if (cc > 100) { MTOOLS_DEBUG("... hanging ..."); cc = 0; }
                    } // wait for the completion of the procedure
                MTOOLS_DEBUG(" ...Finished running the method.");
                }


            /* start the thread and return without waiting for completion of the thread initialization */
            void _startThread()
                {
                MTOOLS_DEBUG("Starting the FLTK Thread...");
                MTOOLS_ASSERT(_active == false);
                MTOOLS_ASSERT(_stopped == false);
                std::thread th(&FLTK_Supervisor::_threadProc, this); // start the FLTK thread
                th.detach(); // detach it.
                while (!_active)
                    {
                    Fl::awake(&FLTK_Supervisor::test_init_callback, nullptr);   // send the test message
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // and wait a little before checking the response
                    }
                MTOOLS_DEBUG(" ...FLTK Thread started.");
                }


            /* Stop the thread. This should not be called from the FLTK thread ! */
            void _stopThread()
                {
                MTOOLS_INSURE(!isFLTKThread());
                MTOOLS_DEBUG("Stopping the FLTK Thread...");
                MTOOLS_ASSERT(_active == true);
                MTOOLS_ASSERT(_stopped == false);
                _exitloop = true; // request stop
                Fl::awake();      // and send a message to wake up the thread
                while (!_stopped) { (void)0; }
                MTOOLS_DEBUG(" ...FLTK Thread stopped.");
            }

            std::condition_variable _cv;    // condition variable used for signaling the completion of an operation
            SimpleMsgList _thMsgList;       // th message list itself

            static void _processMsgCB(void * ignoredParam) { getInst()->_processMsg(); }
            void _processMsg()
                {
                    while (1)
                        {
                        Msg * m = _thMsgList.popBack();
                        if (m == nullptr) return; // no message to process.
                        if (m->type == m->NEW_MSG)
                            {
                            void * q = m->p;
                            ((IndirectCtor*)q)->construct();
                            m->status = 1;          // done
                            _cv.notify_all();   // and send notification to everyone
                            }
                        else if (m->type == m->DELETE_MSG)
                            {
                            void * q = m->p;
                            ((IndirectDtor*)q)->destroy();
                            m->status = 1;          // done
                            _cv.notify_all();   // and send notification to everyone
                            }
                        else if (m->type == m->RUN_MSG)
                            {
                            void * q = m->p;
                            ((IndirectCall*)q)->call();
                            m->status = 1;          // done
                            _cv.notify_all();   // and send a notification
                            }
                        }
                }




            /* the FLTK thread procedure */
            void _threadProc()
                {
                _thid = std::this_thread::get_id(); // get the thread id
                MTOOLS_DEBUG("[FLTK Thread " + toString(_thid) + "] started." );
                Fl::lock();             // lock FLTK for multithread use.
                Fl::args(0, nullptr);   // seems to help, don't know why...
                while (!_exitloop)
                    {
                    Fl::wait(0.1);
                    _processMsg(); // check for messages
                    }
                Fl::unlock(); // seems to do nothing but cannot hurt....
                MTOOLS_DEBUG("[FLTK THREAD " + toString(_thid) + "] stopped.");
                _exitloop = false;
                _active = false;
                _stopped = true;    // ok, pretend that we stopped.
                while (1) { (void)0; }    // but do not really stop otherwise we sometime get a "mutex destroyed while busy" from FLTK
                }


            /* called when testing initialization. Set the _active flag to confirm */
            static void test_init_callback(void* p) { getInst()->_test_init_callback(); }

            void _test_init_callback()
                {
                _active = true; // yes, we are ready !
                return;
                }


            /* called when we want to force the program to exit */
            static void force_exit_callback(void * p) { getInst()->_force_exit_callback(p); }

            void _force_exit_callback(void * p)
                {
                _startrace = false;
                std::atomic_thread_fence(std::memory_order_seq_cst);
                std::thread thE(&FLTK_Supervisor::exitThread, this); // start the exitThread
                thE.detach(); // detach it.
                std::thread thA(&FLTK_Supervisor::abortThread, this); // start the abortThread
                thA.detach(); // detach it.
                _startrace = true; // signify that we are ready to proceed
                return;
                }

            void exitThread()
                {
                MTOOLS_DEBUG("Starting exit thread sequence...");
                while (!_startrace) { std::this_thread::yield(); } // wait for the signal before proceeding
                exit(0); // exit the program, call the dtor of all static objects (with this thread), closes streams etc...
                }

            void abortThread()
                {
                MTOOLS_DEBUG("Starting aborting sequence...");
                while (!_startrace) { std::this_thread::yield(); } // wait for the signal before proceeding
                std::this_thread::sleep_for(std::chrono::seconds(5)); // wait for 5 seconds, giving a chance to exitThread to succeed.
                MTOOLS_DEBUG("!!!! Graceful exit took too long: ABORT !!!!!");
                std::abort(); // still not finished: ok we force termination via abort...
                }


            int _nbObject() const { return _counter; }

            /* return the singleton instance, created on first use */
            static FLTK_Supervisor * getInst()
                {
                static std::atomic<FLTK_Supervisor *> _inst((FLTK_Supervisor *)0); // much better to use a local static variable, avoid many initialization problems !
                if (_inst == (FLTK_Supervisor *)nullptr) { _inst = new FLTK_Supervisor; } return(FLTK_Supervisor*)_inst;
                }

            /* default ctor */
            FLTK_Supervisor() : _init(0), _counter(0), _active(false), _stopped(false), _exitloop(false), _startrace(false), _thid() {}

            ~FLTK_Supervisor() = delete;                                    // the singleton cannot be destroyed. It lives throughout the program
            FLTK_Supervisor(const FLTK_Supervisor&) = delete;               // no copy
            FLTK_Supervisor & operator=(const FLTK_Supervisor&) = delete;   //

            std::atomic<int>   _init;     // 0 if the thread was neverstarted, different from zero otherwise
            std::atomic<int>   _counter;  // number of object currently "alive" in thread
            std::atomic<bool>  _active;   // true is the thread is alive and ready to process requests.
            std::atomic<bool>  _stopped;  // true if the thread was stopped (hence, it was started at some point before).
            std::atomic<bool>  _exitloop; // true when we want to exit the thred loop
            std::atomic<bool>  _startrace;// used for synchronizing both exit thread
            std::atomic<std::thread::id> _thid; // id of the FLTK thread
        };




        /* Helper functions that prevent having to declare the FLTKThread_Supervisor class in fltk_supervisor.hpp header  */

        void deleteInFLTKThread(IndirectDtor * proxy) { FLTK_Supervisor::deleteInFLTKThread(proxy); }

        void newInFLTKThread(IndirectCtor * proxy) { FLTK_Supervisor::newInFLTKThread(proxy); }

        void runInFLTKThread(IndirectCall * proxycall) { FLTK_Supervisor::runInFLTKThread(proxycall); }

        bool isFLTKThread() { return FLTK_Supervisor::isFLTKThread(); }


        /* Request the FLTK thread to perform program termination with cleanup the best it can.
         * Should only be called from within the FLTK thread.
         * To stop the program from another thread, just use exit(0) */
        void exitFLTK() {FLTK_Supervisor::exitFLTK(); return;}

        /* return the number of object curently in the FLTK thread */
        int nbObjectInFLTK() { return FLTK_Supervisor::nbObject(); }


    }

}
/* end of file */


