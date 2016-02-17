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


#include "../misc/error.hpp"
#include "../misc/indirectcall.hpp"


// If we are on OSX, we must swap threads as only the master thread is allowed to
// allocate graphic resources
#ifdef __APPLE__
#define MTOOLS_SWAP_THREADS_FLAG
#endif


/* CONTROL FLOW OF A PROGRAM

When MTOOLS_SWAP_THREADS_FLAG is NOT set 
----------------------------------------    
(normal setting on Windows and Linux)

    - a global static 'fltkSentinel' object is constructed in each compilation unit which use the supervisor.
      These sentinel are constructed before any other global object (provided that the include "fltkSupervisor" 
      appear before a global object is defined). These sentinel does not created anything !

    - The fltk loop (and all graphic related function) are performed in another thread: the fltk thread. 
      This thread created 'on first use' i.e. only when newInFltkThread() or runInFltkThread() is called. 

    - The fltk thread stops automatically when the last sentinel is destroyed (ie after all other global objects). 

    BRUTAL SHUTDOWN:

        - from the main thread: just call std::exit() or mtools::exit(). this calls the dtor of global object and 
          stops the fltk thread when the sentinel is destroyed. 

        - from the fltk thread: call fltkExit() : this stops the fltk thread and then call exit() from within
          the fltk thread. This behavior as 2 important consequences:

              1) destructor of global objects are called from the fltk Thread and NOT from the main thread 
                 that was used to create them.

              2) after fltkExit() is called, any call to newInFltkThread(),runInFltkThread(), deleteInFltkThread()
                will fail (return false). In particular, any of these method placed in dtor global objects will fail.


When MTOOLS_SWAP_THREADS_FLAG is set
----------------------------------------
(OSX setting)

    - The main() function must be guarded by MTOOLS_SWAP_THREADS(argc, argv) 
      (at least until the --wrap option is supported by Clang...)

    - fltkSentinel object do not play any role. 

    - global objects are created by the usual 'master' thread but the main() procedure itself is run in another thread called 'main thread'. 
      The master thread is used to run the fltk loop. 

    - the fltk loop is started at the begining of main et stops when main returns. This main that supervisor methods newInFltkThread()
      runInFltkThread(), deleteInFltkThread() can only be called while the main() proc is running. In particular, any call of these
      method in constructor or destructor of global object will fail.

    - when the program finishes, dtors of global object are again called with the master thread (the same used to construct them). 
      
      BRUTAL SHUTDOWN:

          - from the main thread: calling std::exit() is NOT recommended as destructor of global object would be run by the main thread 
            and not the master thread. Call mtools::exit() instead which insure that the fltk loop stops and then the master thread call
            the destructor of global objects 

          - from the master fltk thread. Call fltkExit(), this stops the fltk loop, detach the main() thread and then run the destructor 
            of global object (in the master thread). 
      
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

        }



    /**
    * Request the process to terminate in the near futur and return.
    * MUST ONLY BE CALLED FROM THE FLTK THREAD.
    **/
    void fltkExit();


    /**
     * Replacement for std::exit(). Should be prefered to the std version as it
     * insure destructor are called nicely in any cases.
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
    * is constructed on the heap via new. Do not call delete on the returned pointer! In order to
    * delete the object call the `deleteInFltkThread()` function. Every object created in the FLTK
    * thread should be properly deleted at some point otherwise it will prevent the thread to stop
    * properly.
    *
    * @tparam  T           Type of object to construct.
    * @tparam  ...Params   Types of the parameters of the constructor of T.
    * @param   args    The arguments to passed to the constructor.
    *
    * @return  a pointer to the constucted object or nullptr in case of failure.
    **/
    template<typename T, typename ...Params> inline T * newInFltkThread(Params&&... args)
        {
        IndirectConstructor<T, Params...> IC(std::forward<Params>(args)...);
        if (internals_fltkSupervisor::newInFltk(&IC)) return (T*)IC.adress();
        return nullptr;
        }

    /**
    * Call the destructor of an object within the FLTK Thread. The object MUST have been created
    * with the `newInFltkThread()` method.
    *
    * @tparam  T   type of the object to delete.
    * @param [in,out]  adress  pointer to the object to delete.
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
     * @return  true if it succeeds, false operation failed.
     **/
    inline bool runInFltkThread(IndirectCall & proxycall)
        {
        return internals_fltkSupervisor::runInFltk(&proxycall);
        }



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

