/** @file fltkSupervisor.cpp */
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
#include "io/fltkSupervisor.hpp"
#include "misc/stringfct.hpp"

namespace mtools
    {

    namespace internals_fltkSupervisor
        {

        /** message structure used for communication with the FLTK thread **/
        struct Msg
            {
            Msg(void * pp, int t) : status(0), p(pp), type(t) {}    // ctor
            std::atomic<int>    status;             // status
            std::atomic<void *> p;                  // pointer to data
            std::atomic<int>    type;               // type
            static const int NEW_MSG = 0;
            static const int DELETE_MSG = 1;
            static const int RUN_MSG = 2;
            };


        /** A very simple thread safe message queue **/
        class MsgQueue
            {
            public:

                /**
                * Pushes a pointer at the back of the message queue
                *
                * @param [in,out]  p   pointer to add.
                **/
                void pushTop(Msg * p)
                    {
                    std::lock_guard<std::mutex> lock(_mut);
                    _msgList.push_back(p);
                    }


                /**
                * Removes and returns the oldest msg in the queue.
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


                /** Current number of msg in queue */
                size_t size() const
                    {
                    std::lock_guard<std::mutex> lock(_mut); // useless but it cannot hurt and it wont slow down too much anyway
                    return _msgList.size();
                    }

            private:

                std::list<Msg *> _msgList;
                mutable std::mutex _mut;
            };



        class FltkSupervisor
            {

            public:

                static const int MAX_PROCESS_MSG = 20; // max number of messages processed in each iteration of the fltk

                /**
                * Return a pointer to the singleton instance and a boolean
                * indicating whether the instance was just constructed.
                **/
                static std::pair<FltkSupervisor*, bool> getInst(bool sentinel = false)
                    {
                    static std::atomic<FltkSupervisor*> _inst((FltkSupervisor *)nullptr);
                    if (_inst == (FltkSupervisor *)nullptr)
                        {
                        if (!sentinel) { MTOOLS_DEBUG("FltkSupervisor singleton not created by a sentinel !"); }
                        _inst = new FltkSupervisor;
                        return std::pair<FltkSupervisor*, bool>(_inst, true);
                        }
                    return std::pair<FltkSupervisor*, bool>(_inst, false);
                    }


                /** Gets the thread status. either one of THREAD_NOT_STARTED, THREAD_ON, THREAD_STOPPING, THREAD_STOPPED **/
                int status() { return _status; }


                /**
                * Query if the current thread is the fltk thread.
                **/
                bool isFltkThread()
                    {
                    if (status() != THREAD_ON) return false;
                    if (_fltkid != std::this_thread::get_id()) return false;
                    return true;
                    }


                /* run a method in fltk, start the thread if needed */
                void runInFltk(IndirectCall * proxycall)
                    {
                    MTOOLS_DEBUG("Running a method in the FLTK thread...");
                    if (isFltkThread())
                        {
                        MTOOLS_DEBUG("from inside the FLTK thread ");
                        proxycall->call();
                        MTOOLS_DEBUG(" ...Finished running the method.");
                        return;
                        }
                    std::lock_guard<std::recursive_mutex> lock(_muthread);
                    _startThread();
                    if (status() != THREAD_ON) { MTOOLS_DEBUG(std::string("Cannot run the method: thread has status ") + mtools::toString(status())); return; }
                    std::mutex tmpmutex;
                    std::unique_lock<std::mutex> lck(tmpmutex);
                    Msg ms(proxycall, Msg::RUN_MSG);
                    _msgQueue.pushTop(&ms);
                    Fl::awake(&FltkSupervisor::_processMsgCB, nullptr);
                    Fl::awake();
                    int c = 0;
                    while (!ms.status)
                        {
                        _cv.wait_for(lck, std::chrono::milliseconds(1));
                        if (!ms.status) { Fl::awake(); }
                        if (c++ > 100) { MTOOLS_DEBUG("... runInFltk() hanging ..."); c = 0; }
                        }
                    MTOOLS_DEBUG("run completed.");
                    }


                /* create an object in fltk, start the thread if needed */
                void newInFltk(IndirectCtor * proxy)
                    {
                    MTOOLS_DEBUG("Creating an object in the FLTK thread.");
                    if (isFltkThread())
                        {
                        MTOOLS_DEBUG("from inside the FLTK thread ");
                        proxy->construct();
                        MTOOLS_DEBUG("Construction completed.");
                        return;
                        }
                    std::lock_guard<std::recursive_mutex> lock(_muthread);
                    _startThread();
                    if (status() != THREAD_ON) { MTOOLS_DEBUG(std::string("Cannot construct the object: thread has status ") + mtools::toString(status())); return; }
                    std::mutex tmpmutex;
                    std::unique_lock<std::mutex> lck(tmpmutex);
                    Msg ms(proxy, Msg::NEW_MSG); // create the message
                    _msgQueue.pushTop(&ms);
                    Fl::awake(&FltkSupervisor::_processMsgCB, nullptr);
                    Fl::awake();
                    int c = 0;
                    while (!ms.status)
                        {
                        _cv.wait_for(lck, std::chrono::milliseconds(1));
                        if (!ms.status) { Fl::awake(); }
                        if (c++ > 100) { MTOOLS_DEBUG("... newInFltk() hanging ..."); c = 0; }
                        }
                    MTOOLS_DEBUG("Construction completed.");
                    return;
                    }


                /* delete an object in fltk, if deleteAlways is set, delete the object
                in the current thread if the fltk thread is already stopped
                return if success (ie if the thread was on and the deletion took place inside the
                fltk thread. */
                bool deleteInFltk(IndirectDtor * proxy, bool deleteAlways)
                    {
                    MTOOLS_DEBUG("Deleting an object in the FLTK thread.");
                    if (isFltkThread())
                        {
                        MTOOLS_DEBUG("from inside the FLTK thread");
                        proxy->destroy();
                        MTOOLS_DEBUG("Destruction completed.");
                        return true;
                        }
                    MTOOLS_DEBUG("A");
                    std::lock_guard<std::recursive_mutex> lock(_muthread);
                    MTOOLS_DEBUG("B");
                    if (status() != THREAD_ON)
                        {
                        MTOOLS_DEBUG(std::string("Calling FltkSupervisor::deleteInFltk() while thread has status ") + mtools::toString(status()));
                        if (deleteAlways)
                            {
                            MTOOLS_DEBUG("destroying object anyway");
                            proxy->destroy();
                            MTOOLS_DEBUG("Destruction completed.");
                            }
                        else
                            {
                            MTOOLS_DEBUG("Destruction ignored.");
                            }
                        return false;
                        }
                    std::mutex tmpmutex;
                    std::unique_lock<std::mutex> lck(tmpmutex);
                    Msg ms(proxy, Msg::DELETE_MSG); // create the message                
                    _msgQueue.pushTop(&ms);
                    Fl::awake(&FltkSupervisor::_processMsgCB, nullptr);
                    Fl::awake();
                    int c = 0;
                    while (!ms.status)
                        {
                        _cv.wait_for(lck, std::chrono::milliseconds(1));
                        if (!ms.status) { Fl::awake(); }
                        if (c++ > 100) { MTOOLS_DEBUG("... deleteInFltk() hanging ..."); c = 0; }
                        }
                    MTOOLS_DEBUG("destruction completed.");
                    return true;
                    }




                /**
                * Stops the fltk thread and wait until it is stopped.
                * Must not be called from the fltk thread itself.
                **/
                void stopThread()
                    {
                    if (isFltkThread()) { MTOOLS_DEBUG("Calling FltkSupervisor::stopThread() from the fltk thread itself : do nothing !"); return; }
                    std::lock_guard<std::recursive_mutex> lock(_muthread);
                    if (status() != THREAD_ON) { MTOOLS_DEBUG(std::string("Calling FltkSupervisor::stopThread() while thread has status ") + mtools::toString(status())); return; }
                    MTOOLS_DEBUG("Stopping the FLTK thread...");
                    _status = THREAD_STOPPING;
#ifdef MTOOLS_SWAP_THREADS_FLAG
                    while (status() == THREAD_STOPPING) { std::this_thread::yield(); }
#else
                    ((std::thread *)_th)->join();
#endif                   
                    MTOOLS_DEBUG("...FLTK thread stopped.");
                    }


                /* Exit the process. Does nothing if not called from the fltk thread */
                void fltkExit()
                    {
                    if (!isFltkThread()) { MTOOLS_DEBUG("Calling FltkSupervisor::fltkExit() from outside of the fltk thread itself : do nothing !"); return; }
                    auto th = new std::thread(&FltkSupervisor::_exitThread, this);
                    th->detach();
                    }


                /* call the thread procedure directly */
                void threadProc()
                    {
                    _threadProc();
                    }

                /* return when the thread is active 
                   should be called after launching the thread procedure */
                void waitThreadReady()
                    {
                    std::lock_guard<std::recursive_mutex> lock(_muthread); // only one operation at a time, status() is atomic
                    std::mutex tmpmutex;
                    std::unique_lock<std::mutex> lck(tmpmutex);
                    Fl::awake(&FltkSupervisor::_initCB, nullptr);
                    Fl::awake();
                    int c = 0;
                    while(status() != THREAD_ON)
                        {
                        _cv.wait_for(lck, std::chrono::milliseconds(1));
                        if (c++ > 100) { MTOOLS_DEBUG("...waitThreadReady() hanging ..."); c = 0; }
                        }
                    }

            private:

                /** Private Constructor. */
                FltkSupervisor() : _status(THREAD_NOT_STARTED), _th((std::thread *)nullptr) {}

                /* no copy */
                FltkSupervisor(const FltkSupervisor &) = delete;
                FltkSupervisor & operator=(const FltkSupervisor &) = delete;

                /** Start the fltk thread and return when it is ready. **/
                void _startThread()
                    {
                    if (isFltkThread()) { MTOOLS_DEBUG("Calling FltkSupervisor::startThread() from the fltk thread itself : do nothing !"); return; }
                    std::lock_guard<std::recursive_mutex> lock(_muthread); // only one operation at a time, status() is atomic
                    if (status() == THREAD_ON) return;
                    if (status() != THREAD_NOT_STARTED) { MTOOLS_DEBUG(std::string("Calling FltkSupervisor::startThread() while thread has status ") + mtools::toString(status())); return; }
                    MTOOLS_DEBUG(" Starting the FLTK Thread...");
                    _th = new std::thread(&FltkSupervisor::_threadProc, this);
                    waitThreadReady();
                    MTOOLS_DEBUG(" ...FLTK Thread started.");
                    return;
                    }

                /* Exit Thread procedure. */
                void _exitThread()
                    {
                    exit(0);    // start exit procedure, call dtor of global/static object
                    }

                /** The FLTK Thread procedure. **/
                void _threadProc()
                    {
                    try
                        {
                        _fltkid = std::this_thread::get_id();
                        MTOOLS_DEBUG(" **** START: FLTK Thread " + toString(_fltkid) + " ****.");
                        Fl::lock();
                        Fl::args(0, nullptr);
                        while (status() != THREAD_STOPPING)
                            {
                            Fl::wait(0.1);
                            _processMsg();
                            if (status() == THREAD_NOT_STARTED) Fl::awake(&FltkSupervisor::_initCB, nullptr);
                            }
                        Fl::unlock();
                        MTOOLS_DEBUG(" **** STOP: FLTK Thread " + toString(_fltkid) + " ****.");
                        _status = THREAD_STOPPED;
                        return;
                        }
                    catch (std::exception & exc)
                        {
                        std::string msg = std::string("Exception caught in the FLTK Thread : [") + exc.what() + "].";
                        MTOOLS_DEBUG(msg.c_str());
                        throw;
                        }
                    }

                /* static init callback */
                static void _initCB(void* p) { MTOOLS_DEBUG("FLTK Init message received"); getInst().first->_init(); }

                /* set the thread as active */
                void _init()
                    {
                    if (status() == THREAD_NOT_STARTED) { _status = THREAD_ON; }
                    _cv.notify_all();
                    }

                /* static msg callback */
                static void _processMsgCB(void * p) { getInst().first->_processMsg(); }

                /* process up to MAX_PROCESS_MSG pending messages */
                void _processMsg()
                    {
                    int c = 0;
                    while (c++ < MAX_PROCESS_MSG)
                        {
                        Msg * m = _msgQueue.popBack();
                        if (m == nullptr) return; // no message to process.
                        void * q = m->p;
                        switch ((int)m->type)
                            {
                            case Msg::NEW_MSG: { ((IndirectCtor*)q)->construct(); break; }
                            case Msg::DELETE_MSG: { ((IndirectDtor*)q)->destroy(); break; }
                            case Msg::RUN_MSG: { ((IndirectCall*)q)->call(); break; }
                            default: { MTOOLS_DEBUG("fltkSupervisor::_processMsg, unknown message !"); return; }
                            }
                        m->status = 1;
                        _cv.notify_all();
                        }
                    }


                std::condition_variable _cv;                 // condition variable used for signaling the completion of an operation
                MsgQueue _msgQueue;                          // message queue
                std::atomic<int>                _status;     // thread status
                std::atomic<std::thread *>      _th;         // pointer to the thread object
                std::recursive_mutex            _muthread;   // mutex for thread operation
                std::atomic<std::thread::id>    _fltkid;     // id of the FLTK thread

            };


        void runInFltk(IndirectCall * proxycall) { internals_fltkSupervisor::FltkSupervisor::getInst().first->runInFltk(proxycall); }

        void newInFltk(IndirectCtor * proxy) { internals_fltkSupervisor::FltkSupervisor::getInst().first->newInFltk(proxy); }

        bool deleteInFltk(IndirectDtor * proxy, bool deleteAlways) { return internals_fltkSupervisor::FltkSupervisor::getInst().first->deleteInFltk(proxy, deleteAlways); }

        void stopThread() { internals_fltkSupervisor::FltkSupervisor::getInst().first->stopThread(); }

        bool instInit() { return internals_fltkSupervisor::FltkSupervisor::getInst(true).second; }

        /*
        bool insureFltkSentinel()
            {
            MTOOLS_DEBUG("insureFltkSentinel()");
            static std::atomic<bool> dummy(false);
            dummy = _fltkSentinel.isMaster();
            return dummy;
            }
            */

        }


    bool isFltkThread() { return internals_fltkSupervisor::FltkSupervisor::getInst().first->isFltkThread(); }

    int fltkThreadStatus() { return internals_fltkSupervisor::FltkSupervisor::getInst().first->status(); }

    bool fltkThreadStopped() { return ((internals_fltkSupervisor::FltkSupervisor::getInst().first->status() == internals_fltkSupervisor::THREAD_STOPPED)|| (internals_fltkSupervisor::FltkSupervisor::getInst().first->status() == internals_fltkSupervisor::THREAD_STOPPING)); }

    void fltkExit() { internals_fltkSupervisor::FltkSupervisor::getInst().first->fltkExit(); }

    }


#ifdef MTOOLS_SWAP_THREADS_FLAG

    /* forward declaration of main */
    int main(int argc, char *argv[]);

    namespace mtools
        {
        namespace internals_switchthread
            {

            int result(bool setresult, int val)
                {
                static std::atomic<int> res(0);
                if (setresult) res = val;
                return val;
                }


            void newMainThread(int argc, char * argv[])
                {
                MTOOLS_DEBUG("starting the 'main' replacement thread");
                mtools::internals_fltkSupervisor::FltkSupervisor::getInst().first->waitThreadReady();
                result(true, main(argc, argv));
                mtools::internals_fltkSupervisor::stopThread();
                MTOOLS_DEBUG("'main' replacement thread stopped");
                }


            bool barrier(int argc, char * argv[])
                {
                MTOOLS_DEBUG("barrier start");
                mtools::internals_fltkSupervisor::instInit();
                static std::atomic<int> nb(0);
                if (nb++ > 0) { return false; }
                if (fltkThreadStatus() != mtools::internals_fltkSupervisor::THREAD_NOT_STARTED)
                    {
                    MTOOLS_ERROR("MTOOLS_SWITCH_THREADS() macro called when the FLTK thread is already running !");
                    }
                std::thread th(&newMainThread, argc, argv); // start the new 'main' thread
                mtools::internals_fltkSupervisor::FltkSupervisor::getInst(true).first->threadProc(); // start the fltk loop
                th.join(); // wait for the 'main' thread to complete
                MTOOLS_DEBUG("barrier end");
                return true;
                }
            }
        }
#endif 


/* end of file */
