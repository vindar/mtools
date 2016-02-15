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


namespace mtools
    {


    namespace internals_fltkSupervisor
        {

        void runInFltk(IndirectCall * proxycall);

        void newInFltk(IndirectCtor * proxy);

        bool deleteInFltk(IndirectDtor * proxy, bool deleteAlways);

        bool instInit();

        void stopThread();

        }



    /**
    * Request the process to terminate in the near futur and return. 
    * This method should only be called from within the fltk thread.  
    **/
    void fltkExit();

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
        internals_fltkSupervisor::newInFltk(&IC);
        return (T*)IC.adress();
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
    **/
    inline void runInFltkThread(IndirectCall & proxycall)
        {
        internals_fltkSupervisor::runInFltk(&proxycall);
        }

    

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
                    if ((bool)_master) MTOOLS_DEBUG("FltkThreadSentinel ctor: Master"); else MTOOLS_DEBUG("FltkThreadSentinel ctor.");
                    }

                ~FltkThreadSentinel()
                    {
                    if ((bool)_master == true)
                        {
                        MTOOLS_DEBUG("FltkThreadSentinel dtor Master: request thread stop");
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
            MTOOLS_DEBUG("insureFltkSentinel()");
            static std::atomic<bool> dummy(false);
            dummy = _fltkSentinel.isMaster();
            return dummy;
            }

        }

    }


#ifdef __APPLE__
#define MTOOLS_SWAP_THREADS_FLAG
#endif

#define MTOOLS_SWAP_THREADS_FLAG


#ifdef MTOOLS_SWAP_THREADS_FLAG

    namespace mtools
        {
        namespace internals_switchthread
            {
            int result(bool setresult = false, int val = 0);
            bool barrier(int argc, char * argv[]);
            }
        }

#define MTOOLS_SWAP_THREADS(argc,argv) { if (mtools::internals_switchthread::barrier(argc, argv)) return mtools::internals_switchthread::result(); }

#else

#define MTOOLS_SWAP_THREADS(argc,argv) ((void)0)

#endif 


/* end of file */

