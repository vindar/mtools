#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;







class FltkSupervisor
    {

    public:



        /**
         * Return a pointer to the singleton instance and a boolean indicating
         * whether the instance was just constructed.
         **/
        static std::pair<FltkSupervisor*, bool> getInst()
            {
            static std::atomic<FltkSupervisor *> _inst((FltkSupervisor *)nullptr);
            if (_inst == (FltkSupervisor *)nullptr)
                {
                _inst = new FltkSupervisor;
                return std::pair<FltkSupervisor*, bool>(_inst, true);
                }
            return std::pair<FltkSupervisor*, bool>(_inst, false);
            }


        FltkSupervisor() : _status(THREAD_NOT_STARTED), _th((std::thread *)nullptr) {}

        /**
         * Gets the thread status.
         **/
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



        /**
        * Start the fltk thread and return when it is ready.
        **/
        void startThread()
            {
            if (isFltkThread()) { MTOOLS_DEBUG("Calling FltkSupervisor::startThread() from the fltk thread itself : do nothing !"); return; }
            std::lock_guard<std::recursive_mutex> lock(_muthread);
            if (status() == THREAD_ON) return;
            if (status() != THREAD_NOT_STARTED) { MTOOLS_DEBUG(std::string("Calling FltkSupervisor::startThread() while thread has status ") + mtools::toString(status())); return; }
            MTOOLS_DEBUG(" Starting the FLTK Thread...");
            _th = new std::thread(&FltkSupervisor::_threadProc, this, true);
            Fl::awake(&FltkSupervisor::test_init_callback, nullptr);
            while(status() != THREAD_ON) { std::this_thread::yield(); }
            MTOOLS_DEBUG(" ...FLTK Thread started.");
            return;
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
            ((std::thread *)_th)->join();
            _status = THREAD_STOPPED;
            MTOOLS_DEBUG("...FLTK thread stopped.");
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
            startThread();
            if (status() != THREAD_ON) { MTOOLS_DEBUG(std::string("Cannot run the method: thread has status ") + mtools::toString(status())); return; }

            // send msg and wait for completion of the task. 
            MTOOLS_DEBUG("run completed.");
            }

        /* create an object in fltk, start the thread if needed */
        void newInFltk(IndirectCtor * proxy)
            {
            MTOOLS_DEBUG("Creating an object in the FLTK thread.");
            if (isFLTKThread())
                {
                MTOOLS_DEBUG("from inside the FLTK thread ");
                proxy->construct();
                MTOOLS_DEBUG("Construction completed.");
                return;
                }
            std::lock_guard<std::recursive_mutex> lock(_muthread);
            startThread();
            if (status() != THREAD_ON) { MTOOLS_DEBUG(std::string("Cannot construct the object: thread has status ") + mtools::toString(status())); return; }

            int cc = 0;
            std::mutex tmpmut; // temporary mutex for condition variable
            std::unique_lock<std::mutex> lck(tmpmut);   // unique lock over the mutex, lock it
            Msg ms(proxy, Msg::NEW_MSG); // create the message
            _thMsgList.pushTop(&ms); // push it
            Fl::awake(&FLTK_Supervisor::_processMsgCB, nullptr); // inform fltk
            Fl::awake(); // again...
            while (!ms.status)
                {
                _cv.wait_for(lck, std::chrono::milliseconds(5)); // wait for signalization of the condition variable (or time elapsed).
                if (!ms.status) { Fl::awake(); }
                cc++; if (cc > 100) { MTOOLS_DEBUG("... hanging ..."); cc = 0; }
                } // wait for the completion of the procedure
            MTOOLS_DEBUG("Construction completed.");
            return;
            }

        /* delete an object in fltk, if deleteAlways is set, deltee the object
        in the current thread if the fltk thread is already stopped
        return if success (ie if the thread was on and the deletion took place inside the
        fltk thread. */
        bool deleteInFltk(IndirectDtor * proxy, bool deleteAlways = false)
            {
            MTOOLS_DEBUG("Deleting an object in the FLTK thread.");
            if (isFLTKThread())
                {
                MTOOLS_DEBUG("from inside the FLTK thread");
                proxy->destroy();
                MTOOLS_DEBUG("Destruction completed.");
                return;
                }
            std::lock_guard<std::recursive_mutex> lock(_muthread);
            if (status() != THREAD_ON) 
                { 
                MTOOLS_DEBUG(std::string("Calling FltkSupervisor::deleteInFltk() while thread has status ") + mtools::toString(status())); 
                if (deleteAlways)
                    {
                    // delete it
                    }
                return false; 
                }
            // send msg and wait for completion of the task. 
            MTOOLS_DEBUG("delete completed.");
            return true;
            }





        private:


        /**
         * The FLTK Thread procedure.
         **/
        void _threadProc()
            {
            try
                {
                _fltkid = std::this_thread::get_id();
                MTOOLS_DEBUG(" *** START: FLTK Thread " + toString(_thid) + " ****.");
                Fl::lock();
                Fl::args(0, nullptr); 
                while(status() != THREAD_STOPPING)
                    {
                    Fl::wait(0.1);
                    _processMsg();
                    if (status() == THREAD_NOT_STARTED) Fl::awake(&FltkSupervisor::initCB, nullptr);
                    }                
                Fl::unlock();
                MTOOLS_DEBUG(" *** STOP: FLTK Thread " + toString(_thid) + " ****.");
                return;
                }
            catch (std::exception & exc)
                {
                std::string msg = std::string("Exception caught in the FLTK Thread : [") + exc.what() + "].";
                MTOOLS_DEBUG(msg.c_str());
                throw;
                }
            }

        /* set the thread as active */
        static void initCB(void* p) { if (getInst().first->_status = THREAD_NOT_STARTED) { getInst().first->_status = THREAD_ON; } return; }


        static void _processMsgCB(void * ignoredParam) { getInst().first->_processMsg(); }

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
                    _cv.notify_all();       // and send notification to everyone
                    }
                else if (m->type == m->DELETE_MSG)
                    {
                    void * q = m->p;
                    ((IndirectDtor*)q)->destroy();
                    m->status = 1;          // done
                    _cv.notify_all();       // and send notification to everyone
                    }
                else if (m->type == m->RUN_MSG)
                    {
                    void * q = m->p;
                    ((IndirectCall*)q)->call();
                    m->status = 1;          // done
                    _cv.notify_all();       // and send a notification
                    }
                }
            }

        std::condition_variable _cv;    // condition variable used for signaling the completion of an operation
        MsgList _thMsgList;             // thread message list

        std::atomic<int>                _status;     // thread status
        std::atomic<std::thread *>      _th;         // pointer to the thread object
        std::recursive_mutex            _muthread;   // mutex for thread operation
        std::atomic<std::thread::id>    _fltkid;     // id of the FLTK thread

        static const int THREAD_NOT_STARTED = 0;
        static const int THREAD_ON = 1;
        static const int THREAD_STOPPING = 2;
        static const int THREAD_STOPPED = 3;


    };








class ThreadSentinel
    {
    public:

        FltkThreadSentinel() : _nb(FltkSupervisor::getInst()->nbCheck()) { }

        ~FltkThreadSentinel()
            {
            if (nb == 0) { FltkSupervisor::quit(); }
            }

    private:

        FltkThreadSentinel(const FltkThreadSentinel &) = delete;
        FltkThreadSentinel & operator=(const FltkThreadSentinel &) = delete;
        std::atomic<bool> _nb;
    };




MT2004_64 gen;

double sinus(double x)
    {
    return sin(x);
    }



int main(int argc, char *argv[])
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv, false, false);

    Plotter2D PL;
    auto P1 = makePlot2DFun(sinus, -1,5, "sinus");
    PL[P1];
    PL.autorangeXY();
    PL.plot();

    return 0;

    }

/* end of file main.cpp */

