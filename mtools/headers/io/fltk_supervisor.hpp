/** @file fltk_supervisor.hpp */
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


/*
* Commands for controlling the FLTK thread running in the background. 
* The thread is started on first use (intialization order fiaco should not be possible) and stopped when there is no more 
* object to manage (ie when deleteInFLTKThread() has been called on every object created with newInFLTKThread() ). 
* A dummy static object is also created to insure that the thread always start before main() and does not stop before 
* main return. 
*/

#include "../misc/error.hpp"
#include "../misc/indirectcall.hpp"


namespace mtools
    {

    namespace internals_fltk_supervisor
        {

        void deleteInFLTKThread(IndirectDtor * proxy);  // private methods.
        void newInFLTKThread(IndirectCtor * proxy);     //
        void runInFLTKThread(IndirectCall * proxycall); //
        void exitFLTK();                                //
        int nbObjectInFLTK();                           //
        bool isFLTKThread();                            //
        int FLTKThreadStatus();                         //
        void runFLTKloop();                             //
        void stopFLTKloop();                            //

        }


    /**
     * Query if the calling thread is the fltk thread.
     *
     * @return  true if it is and false otherwise.
     **/
    inline bool isFLTKThread() { return internals_fltk_supervisor::isFLTKThread(); }


    /**
     * Determines the current status of the FLTK thread.
     *
     * @return  -1 if stopped, 0 if never started, 1 if started but not active, 2 if thread is
     *          running and ready.
     **/
    inline int FLTKThreadStatus() { return internals_fltk_supervisor::FLTKThreadStatus(); }


    /**
     * Create an object of type T with constructor argument args within the FLTK thread. The object
     * is constructed on the heap via new. Do not call delete on the returned pointer! In order to
     * delete the object call the `deleteInFLTKThread()` function. Every object created in the FLTK
     * thread should be properly deleted at some point otherwise it will prevent the thread to stop
     * properly.
     *
     * @code{.cpp}
     *
     * class test
     *     {
     *     public:
     *     test(int a) : r(a) { mtools::cout << "ctor " << a << "\n"; }
     *     ~test() { mtools::cout << "dtor " << r << "\n"; }
     *     int r;
     *     };
     *
     * int main()
     *     {
     *     test * p = mtools::newInFLTKThread<test,int>(7); // create the object in the FLTK thread
     *     mtools::deleteInFLTKThread(p); // and then delete it
     *     return 0;
     *     }
     *
     * @endcode.
     *
     * @tparam  T           Type of object to construct.
     * @tparam  ...Params   Types of the parameters of the constructor of T.
     * @param   args    The arguments to passed to the constructor.
     *
     * @return  a pointer to the constucted object or nullptr in case of failure.
     **/
    template<typename T, typename ...Params> inline T * newInFLTKThread(Params&&... args)
        {
        IndirectConstructor<T, Params...> IC(std::forward<Params>(args)...);
        internals_fltk_supervisor::newInFLTKThread(&IC);
        return (T*)IC.adress();
        }


    /**
     * Call the destructor of an object within the FLTK Thread. The object MUST have been created
     * with the `newInFLTKThread()` method.
     *
     * @tparam  T   type of the object to delete.
     * @param [in,out]  adress  pointer to the object to delete.
     **/
    template<typename T> inline void deleteInFLTKThread(T * adress)
        {
        IndirectDestructor<T> ID(adress);
        internals_fltk_supervisor::deleteInFLTKThread(&ID);
        }


    /**
     * Execute a method/function inside the FLTK thread. The function returns only when the command
     * has finish executing (and then the result may be queried via the result() method of the
     * proxycall object).
     *
     * @param [in,out]  proxycall   The proxy object containing the call to make.
     **/
    inline void runInFLTKThread(IndirectCall & proxycall)
        {
        internals_fltk_supervisor::runInFLTKThread(&proxycall);
        }

    }



#ifdef __APPLE__
#define MTOOLS_SWAP_THREADS_FLAG
#endif

#ifdef MTOOLS_SWAP_THREADS_FLAG

    namespace mtools
        {
        namespace internals_switchthread
            {
            int result(bool setresult, int val = 0);
            bool barrier(int argc, char * argv[]);
            }
        }

#define MTOOLS_SWAP_THREADS(argc,argv) { if (mtools::internals_switchthread::barrier(argc, argv)) return mtools::internals_switchthread::result(false); }

#else 

    namespace mtools
        {
        namespace internals_fltk_supervisor
            {
            /* A static object which insures that the FLTK thread starts, at the latest,
            * when this object is constructed i.e. before main() and does not end
            * before it is destructed (after main() exit). */
            static class _dummyFLTKobject
                {
                public:
                _dummyFLTKobject() : p(nullptr) { if (nbObjectInFLTK() == 0) { MTOOLS_DEBUG("Creating the first FLTK (dummy) object.");  p = mtools::newInFLTKThread<int>(); } }
                ~_dummyFLTKobject() { if (p != nullptr) { MTOOLS_DEBUG("Destroying the first FLTK (dummy) object."); mtools::deleteInFLTKThread<int>(p); } }
                private:
                _dummyFLTKobject(const _dummyFLTKobject &) = delete;
                _dummyFLTKobject & operator=(const _dummyFLTKobject &) = delete;
                int * p;
                } _obj;
            }
        }

#define MTOOLS_SWAP_THREADS(argc,argv) ((void)0)

#endif


/* end of file */

