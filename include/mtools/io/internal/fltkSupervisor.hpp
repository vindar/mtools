/** @file fltkSupervisor.hpp */
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


#include "../../misc/error.hpp"
#include "../../misc/indirectcall.hpp"

#include <thread>
#include <condition_variable>
// If we are on OSX, we must swap threads as only the master thread is allowed to
// allocate graphic resources
#ifdef __APPLE__
#define MTOOLS_SWAP_THREADS_FLAG
#endif


/* 

*************************
CONTROL FLOW OF A PROGRAM
*************************

When MTOOLS_SWAP_THREADS_FLAG is NOT set 
----------------------------------------    
(normal setting on Windows and Linux)

    - A global static 'fltkSentinel' object is constructed in each compilation unit which use the supervisor.
      These sentinel are constructed before any other global object (provided that the include "fltkSupervisor" 
      appear before a global object is defined). These sentinels do not create anything.

    - The fltk loop (and graphic related functions) are performed in another thread: the 'fltk thread'. 
    
    - The fltk thread is created on first use when newInFltkThread() or runInFltkThread() is called for the 
      first time. It stops automatically when the last sentinel is destroyed hence:
      -> The FLTK thread is available for the entire life of every objects (even gloabl ones).

    - NORMAL PROGRAM TERMINATION: returning from main() or calling mtools:exit().
      -> Global objects are constructed and destroyed by the main thread. 

    - BRUTAL SHUTDOWN: calling fltkExit() from the fltk thread.
      -> Global objects are constructed by the main thread but are destroyed by another 'helper' thread.


When MTOOLS_SWAP_THREADS_FLAG is set
----------------------------------------
(OSX setting)

    - The main() function must be guarded by MTOOLS_SWAP_THREADS(argc, argv), at least until the --wrap option is supported by Clang...

    - There are no fltkSentinel objects. 
    
    - function main() is run in an 'alternate main thread' whereas the fltk loop is run in the 'master thread'. 

    - The fltk loops only starts with main() and ends when main() returns. Hence, the fltk thread is NOT available in constructor and destructor
      of global objects. Any call to newInFltkThread(), runInFltkThread(), deleteInFltkThread() before or after main() will fail
      and return false.
      -> The FLTK thread is available only for main but not for ctor and dtor of global objects.
      -> global objects are created by the usual 'master' thread but the main() proc is run in another alternate thread.

    - SHUTDOWN: with fltkExit(), return from main() or mtools::exit()
      -> Global objects are constructed and destroyed by the master thread (NOT the one running main). 
      -> Do not use std::exit() otherwise the global objects will be destroyed by the calling 'alternate main' thread which is not the same
         as the master thread used to create them.

*/


namespace mtools
    {


    namespace internals_fltkSupervisor
        {

        bool runInFltk(IndirectCall * proxycall);

        bool newInFltk(IndirectCtor * proxy);

        bool deleteInFltk(IndirectDtor * proxy, bool deleteAlways);

        bool instInit();

        void stopThread();

        /* possible thread status */
        static const int THREAD_NOT_STARTED = 0;
        static const int THREAD_ON = 1;
        static const int THREAD_STOPPING = 2;
        static const int THREAD_STOPPED = 3;

        /* typedef for callback function for registerAtFltkExit() */
        typedef void (*cbFltkExit)(void * data);

        }


    /**
     * Request the process to terminate in the near futur and then return.
     * 
     * This method must be called from within the FLTK thread. If called from another thread, the
     * method does nothing and returns immediatly. Use exit() in this case.
     *
     * @param   code    The exit code.
     **/
    void fltkExit(int code);


    /**
     * Replacement for std::exit(). Should be prefered to the std version as it insure destructor
     * are called nicely in any cases. This method never returns.
     * 
     * This method must NOT be called from the fltk thread. If called from the FLTK thread, the
     * method does nothing and returns immediatly. Use fltkExit() in this case.
     *
     * @param   code    The exit code.
     **/
    void exit(int code);


    /**
    * Query if the calling thread is the fltk thread.
    *
    * @return  true if it is and false otherwise.
    **/
    bool isFltkThread();


    /**
    * Determines the current status of the FLTK thread.
    **/
    int fltkThreadStatus();


    /**
     * Determines the fltk thread is ready. Return no if it is either not yet started or already
     * stopped.
     **/
    bool fltkThreadStopped();


    /**
     * Create an object of type T with constructor argument args within the FLTK thread. The object
     * is constructed on the heap via new. Call the `deleteInFltkThread()` to delete the function.
     *
     * @tparam  ...Params   Types of the parameters of the constructor of T.
     * @param   args    The arguments to passed to the constructor.
     *
     * @return  a pointer to the constucted object or nullptr in case of failure (for instance if the
     *          fltk thread is not available).
     **/
    template<typename T, typename ...Params> inline T * newInFltkThread(Params&&... args)
        {
        IndirectConstructor<T, Params...> IC(std::forward<Params>(args)...);
        if (internals_fltkSupervisor::newInFltk(&IC)) return (T*)IC.adress();
        return nullptr;
        }


    /**
     * Call the destructor of an object within the FLTK Thread.
     *
     * @param [in,out]  adress  pointer to the object to delete.
     * @param   deleteAlways    true to delete the object from the calling thread if the fltk thread
     *                          is not available.
     *
     * @return  true if deletion occured inside the fltk thread, false otherwise.
     **/
    template<typename T> inline bool deleteInFltkThread(T * adress, bool deleteAlways = false)
        {
        IndirectDestructor<T> ID(adress);
        return internals_fltkSupervisor::deleteInFltk(&ID, deleteAlways);
        }


    /**
     * Execute a method/function inside the FLTK thread. The function returns only when the command
     * has finish executing (and then the result may be queried via the result() method of the
     * proxycall object).
     *
     * @param [in,out]  proxycall   The proxy object containing the call to make.
     *
     * @return  true if the call was made and false if it fail (i.e. if the fltk thread was not
     *          available).
     **/
    inline bool runInFltkThread(IndirectCall & proxycall)
        {
        return internals_fltkSupervisor::runInFltk(&proxycall);
        }


    /**
     * Registers a function that should be called when fltk exits().
     * 
     * The called are made in the reverse order of registration and they all happen after the fltk
     * loop ends (i.e. no other fltk calls will be made). To unregister a function, use
     * unregisterAtFltkExit() with the same arguments.
     * 
     * It is possible to register several time the same function with the same argument and it will
     * be called as many times as it was registered.
     *
     * @param   cb              The function to register. Signature: void (*func)(void * data)
     * @param [in,out]  data    Opaque date transmitted along.
     **/
    void registerAtFltkExit(internals_fltkSupervisor::cbFltkExit  cb, void * data);


    /**
     * Unregisters one or more functions previously registered by registerAtFltkExit(). This method
     * unregister all callbacks with the same function and the same data.
     *
     * @param   cb              The function to unregister. Signature: void (*func)(void * data)
     * @param [in,out]  data    The data that was passed along.
     *
     * @return  The number of callbacks successfully removed.
     **/
    int unregisterAtFltkExit(internals_fltkSupervisor::cbFltkExit  cb, void * data);


    }
    

#ifndef MTOOLS_SWAP_THREADS_FLAG

    #define MTOOLS_SWAP_THREADS(argc,argv) ((void)0)

    namespace mtools
        {
        namespace internals_fltkSupervisor
            {

            /**
            *  Sentinel for the fltk thread in each compilation unit that includes fltkSupervisor.hpp. The
            *  last one makes sure the thread is stopped.
            **/
            class FltkThreadSentinel
                {
                public:

                    FltkThreadSentinel() : _master(instInit())
                        {
                        if ((bool)_master) MTOOLS_DEBUG("Master FltkThreadSentinel created.");
                        }

                    ~FltkThreadSentinel()
                        {
                        if ((bool)_master == true)
                            {
                            MTOOLS_DEBUG("destroying FltkThreadSentinel : request thread stop.");
                            stopThread();
                            }
                        }

                    bool isMaster() { return (bool)_master; }

                private:

                    FltkThreadSentinel(const FltkThreadSentinel &) = delete;
                    FltkThreadSentinel & operator=(const FltkThreadSentinel &) = delete;
                    std::atomic<bool> _master;

                };

            static FltkThreadSentinel _fltkSentinel;

            /**
            * force the sentinel to be instanciated before any object that calls this function.
            **/
            inline bool insureFltkSentinel()
                {
                static std::atomic<bool> dummy(false);
                dummy = _fltkSentinel.isMaster();
                return dummy;
                }

            }

        }

#else

    #define MTOOLS_SWAP_THREADS(argc,argv) { if (mtools::internals_switchthread::barrier(argc, argv)) return mtools::internals_switchthread::result(); }

    namespace mtools
        {

        namespace internals_fltkSupervisor
            {

            inline bool insureFltkSentinel() { return false; }

            }

        namespace internals_switchthread
            {
            int result(bool setresult = false, int val = 0);
            bool barrier(int argc, char * argv[]);
            }
        }

#endif 


/* end of file */

