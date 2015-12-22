/** @file watch.hpp */
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

#include "../misc/metaprog.hpp"

#include <mutex>
#include <atomic>
#include <string>
#include <type_traits>
#include <string>

#include <FL/Fl_Button.H>

namespace mtools
    {


    namespace internals_watch
        {

        /* forward declaration */
        class FltkWatchWin;


        /** Pure virtual base class for a watchable object */
        struct WatchObj
            {
                WatchObj(const std::string & name, int rate);
                virtual ~WatchObj();
                std::string get() const;
                void set(const std::string & value);
                virtual bool writable() const;
                virtual std::string type() const;
                int refreshRate() const;
                int refreshRate(int newrate);
                void assignFltkWin(FltkWatchWin * p, Fl_Button * nameButton, Fl_Button * valueButton);

                virtual std::string _get() const;
                virtual size_t _set(const std::string & value);

                int _rate;                          // refresh rate (number of time per minutes)
                FltkWatchWin * _fltkwin;            // the associated watch window
                Fl_Button * _name_button;           // fltk button with the name
                Fl_Button * _value_button;          // fltk button with the value
                std::string _name;                  // identifier name of the variable
            };


        /* emulate std::remove_cv_t in case compiler doesn't support C++14 */
        template< class T > using remove_cv_tt = typename std::remove_cv<T>::type; 

        /**
        * A watch object containing a variable of type T
        **/
        template<typename T, bool allowWrite> struct WatchObjVar : public WatchObj
            {
            typedef remove_cv_tt<T> cvT;

                /** Constructor **/
                WatchObjVar(const std::string & name, const T & val, int rate) : WatchObj(name, rate), _p(const_cast<T*>(&val)) {}

                /** Destructor. */
                virtual ~WatchObjVar() override {}

                /**
                * Return true if the object is writable and false otherwise.
                **/
                virtual bool writable() const override { return _writable; }

                /** Gets the string representing the object  **/
                virtual std::string _get() const override { return mtools::toString(*(const_cast<cvT*>(_p))); }

                /* return the type of T */
                virtual std::string type() const { return  typeid(T).name(); }

                /**
                * Sets the given value and return the number of character read from the input.
                * Do nothing and return 0 if if writable is false or if the object cannot
                * be set using mtools::fromString().
                **/
                virtual size_t _set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<_writable>()); }

                size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { return mtools::fromString(str, *(const_cast<cvT*>(_p))); } // set value using fromString()
                size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

                mutable volatile T * _p;      // pointer to the watched object
                static const bool _writable = (allowWrite && mtools::metaprog::has_from_istream<cvT>::value);  // true if the object is writable and false otherwise
            };


        /**
        * A watch object containing a variable of type T, use a function/functor for output
        **/
        template<typename T, typename OutFun, bool allowWrite> struct WatchObjVarOut : public WatchObj
            {
            typedef remove_cv_tt<T> cvT;
                
                /** Constructor **/
                WatchObjVarOut(const std::string & name, const T & val, OutFun & outfun, int rate) : WatchObj(name, rate), _p(const_cast<T*>(&val)), _outfun(&outfun) {}

                /** Destructor. */
                ~WatchObjVarOut() override {}

                /**
                * Return true if the object is writable and false otherwise.
                **/
                virtual bool writable() const override { return _writable; }

                /** Gets the string representing the object  **/
                virtual std::string _get() const override { return mtools::toString((*_outfun)(*(const_cast<cvT*>(_p)))); }

                /* return the type of T */
                virtual std::string type() const { return  typeid(T).name(); }

                /**
                * Sets the given value and return the number of character read from the input.
                * Do nothing and return 0 if if writable is false or if the object cannot
                * be set using mtools::fromString().
                **/
                virtual size_t _set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<_writable>()); }

                size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { return mtools::fromString(str, *(const_cast<cvT*>(_p))); } // set value using fromString()
                size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

                mutable volatile T * _p;                 // pointer to the watched object
                OutFun * _outfun;       // pointer to the output functor
                static const bool _writable = (allowWrite && mtools::metaprog::has_from_istream<cvT>::value);  // true if the object is writable and false otherwise
            };


        /**
        * A watch object containing a variable of type T, use functions/functors for output and input
        **/
        template<typename T, typename OutFun, typename InFun, bool allowWrite> struct WatchObjVarOutIn : public WatchObj
            {
            typedef remove_cv_tt<T> cvT;

                /** Constructor **/
                WatchObjVarOutIn(const std::string & name, const T & val, OutFun & outfun, InFun & infun, int rate) : WatchObj(name, rate), _p(const_cast<T*>(&val)), _outfun(&outfun), _infun(&infun) {}

                /** Destructor. */
                ~WatchObjVarOutIn() override {}

                /**
                * Return true if the object is writable and false otherwise (ie if allowWrite was set to false)
                **/
                virtual bool writable() const override { return allowWrite; }

                /** Gets the string representing the object  **/
                virtual std::string _get() const override { return mtools::toString((*_outfun)(*(const_cast<cvT*>(_p)))); }

                /* return the type of T */
                virtual std::string type() const { return  typeid(T).name(); }

                /**
                * Sets the given value (if writable, otherwise do nothing), then return 0.
                **/
                virtual size_t _set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<allowWrite>()); }

                size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { (*_infun)(str, *(const_cast<cvT*>(_p))); return 0; } // set value using infun
                size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

                
                mutable volatile T * _p;// pointer to the watched object
                OutFun * _outfun;       // pointer to the output functor
                InFun *  _infun;        // pointer to the output functor
            };


        /* forward declaration */
        class FltkWatchWin;

        }



        /** Form for viewing the watch. */


        /**
         * Class defining a watch window.
         * 
         * Variables to watch can be added using the spy() method or operator(). In order to work
         * correctly, spied variables should be tagged as volatile or should be accessed through a
         * 'transform' function outfun/infin (which also allows for correct synchronization and memory
         * barrier).
         * 
         * a Global watch window named 'mtools::watch' is automatically created at startup (but is only
         * displayed when at least one variable is being watched).
         **/
        class WatchWindow
            {

            public:


                static const int DEFAULT_REFRESHRATE = 180; ///< The default refresh rate : 3 times per second


                /**
                 * Default constructor. The window is NOT displayed until there is something to spy on.
                 **/
                WatchWindow();


                /**
                 * Constructor, with a given name and initial position. The window is NOT displayed until there
                 * is something to spy on.
                 *
                 * @param   name    The name of the window.
                 * @param   X       The X coordinate of the upper left corner.
                 * @param   Y       The Y coordinate of the upper left corner.
                 **/
                WatchWindow(const std::string & name, int X = DEFAULT_X, int Y = DEFAULT_Y);


                /** Destructor. */
                ~WatchWindow();


                /**
                 * Moves the watch window so that its upper left corner is at position (X,Y).
                 *
                 * @param   X   The X coordinate of the upper left corner.
                 * @param   Y   The Y coordinate of the upper left corner.
                 **/
                void move(int X, int Y);


                /**
                 * Remove a variable from the spy window. If not more variables are being watched, the window is
                 * hidden.
                 *
                 * @param   name    name identifier of the variable to remove.
                 **/
                void remove(const std::string & name);


                /**
                *  Remove all variables inside the watch window (makes the window disappear from the screen)
                **/
                void clear();


                /**
                * Set the refresh rate for a given variable.
                *
                * @param   name    name identifier
                * @param   newrate The new refresh rate (number of times per seconds). Set to 0 to disable refresh.
                **/
                void refreshRate(const std::string & name, int newrate);


                /**
                 * Attach a variable to the window.
                 *
                 * @tparam  allowWrite  true to authorize writing to the variable.
                 * @tparam  T           Generic type parameter.
                 * @param   name        name identifier.
                 * @param [in,out]  val the variable to spy on.
                 **/
                template<bool allowWrite = true, typename T> void spy(const std::string & name, T & val)
                    {
                    createIfNeeded();
                    internals_watch::WatchObjVar<T, allowWrite> * p = new internals_watch::WatchObjVar<T, allowWrite>(name, val, DEFAULT_REFRESHRATE);
                    transmit(name,p);
                    }


                /**
                * Attach a variable to the window. The value to display is obtained by calling 'outfun(val)'
                *
                * @tparam  allowWrite      true to authorize writing to the variable.
                * @param   name            name identifier.
                * @param [in,out]  val     the variable to spy on.
                * @param [in,out]  outfun  a function/functor called before displaying the value of the
                *                          variable. The value displayed in the watch window is 'outfun(val)',
                *                          (converted to string if necessary using mtools::toString).
                **/
                template<bool allowWrite = true, typename T, typename OutFun> void spy(const std::string & name, T & val, OutFun & outfun)
                    {
                    createIfNeeded();
                    internals_watch::WatchObjVarOut<T, OutFun, allowWrite> * p = new internals_watch::WatchObjVarOut<T, OutFun, allowWrite>(name, val, outfun, DEFAULT_REFRESHRATE);
                    transmit(name, p);
                    }


                /**
                * Attach a variable to the window. The value to display is obtained by calling 'outfun(val)'.
                * Uses a custom function/functor for setting the variable value.
                *
                * @tparam  allowWrite      true to authorize writing to the variable.
                * @param   name            name identifier.
                * @param [in,out]  val     the variable to spy on.
                * @param [in,out]  outfun  a function/functor called before displaying the value of the
                *                          variable. The value displayed in the watch window is 'outfun(val)',
                *                          (converted to string if necessary using mtools::toString).
                * @param [in,out]  infun   a function/functor used to set the variable value 'val' from a
                *                          std::string 'str' by calling of the form 'infun(str,val)'. Return
                *                          value of infun, if any is discarded.
                **/
                template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void spy(const std::string & name, T & val, OutFun & outfun, InFun & infun)
                    {
                    createIfNeeded();
                    internals_watch::WatchObjVarOutIn<T, OutFun, InFun, allowWrite> * p = new internals_watch::WatchObjVarOutIn<T, OutFun, InFun, allowWrite>(name, val, outfun, infun, DEFAULT_REFRESHRATE);
                    transmit(name, p);
                    }


                /** Add a spy variable. Same as spy(name,val) **/
                template<bool allowWrite = true, typename T> void operator()(const std::string & name, T & val) { spy<allowWrite, T>(name, val); }

                /** Add a spy variable. Same as spy(name,val,outfun) **/
                template<bool allowWrite = true, typename T, typename OutFun> void operator()(const std::string & name, T & val, OutFun & outfun) { spy<allowWrite, T, OutFun>(name, val, outfun); }

                /** Add a spy variable. Same as spy(name,val,outfun,infun) **/
                template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void operator()(const std::string & name, T & val, OutFun & outfun, InFun & infun) { spy<allowWrite, T, OutFun, InFun>(name, val, outfun, infun); }


                /**
                 * Flushes a variable (dirty trick). Useful if the variable was not declared volatile as the
                 * compiler may optimied away writing the variable to memory (in which case the value in the
                 * watch window does not change).
                 * 
                 * @note It is better, if possible, to declare variable being watched as volatile. Especially is
                 * its value is to be modified by the watch window.
                 *
                 * @param   v       The variable to flush to memory.
                 * @param   tick    define how often the flush must be performed (usually, 100 is usually enough
                 *                  for maximum performance). Set to zero to flush every time the methos is called.
                 **/
                template<typename T> inline void flush(const T & v, const size_t tick = 100) 
                    { 
                    MTOOLS_ASSERT(sizeof(v) > 0); 
                    static size_t counter = 0;
                    if (counter >= tick) { _nc = *((char*)((&v) + _ind)); counter = 0; } else { counter++; }
                    }


                /**
                 * Flush a variable to memory (dirty trick). Same as 'flush(v)'. Useful to watch non-volatile
                 * variables.
                 * 
                 * @note It is better, if possible, to declare variable being watched as volatile. Especially is
                 * its value is to be modified by the watch window.
                 *
                 * @param   v   The variable to flush to memory.
                 **/
                template<typename T> inline void operator[](const T & v) { flush(v); }

            private:


                /** Creates the fltk object if it does not exist yet */
                void createIfNeeded();

                /** transmit to the fltk object */
                void transmit(const std::string & name, internals_watch::WatchObj * p);

                WatchWindow(const WatchWindow &) = delete;              // no copy
                WatchWindow & operator=(const WatchWindow &) = delete;  //

                internals_watch::FltkWatchWin* _fltkobj;  // the object running inside the fltk thread.

                volatile char _nc;
                volatile size_t _ind;

                int _X, _Y;
                int _nb;
                std::string _name;

                static int _nbWatchWin;

                static const int DEFAULT_X = 0;
                static const int DEFAULT_Y = 480;


            };


        namespace internals_watch
            {

#ifndef MTOOLS_BASIC_CONSOLE
    #define NOBASICWATCH(_ex) _ex
#else 
    #define NOBASICWATCH(_ex) ((void)0)
#endif

            /* used to create the global singleton watch window */
            class GlobalWatchWindow
                {

                public:

                    GlobalWatchWindow() { _get(1); }
                    ~GlobalWatchWindow() { _get(-1); }
                    void move(int X, int Y) { NOBASICWATCH(_get(0)->move(X,Y)); }
                    void remove(const std::string & name) { NOBASICWATCH(_get(0)->remove(name)); }
                    void clear() { NOBASICWATCH(_get(0)->clear()); }                    
                    void refreshRate(const std::string & name, int newrate) { NOBASICWATCH(_get(0)->refreshRate(name,newrate)); }
                    template<bool allowWrite = true, typename T> void spy(const std::string & name, T & val) { NOBASICWATCH(_get(0)->spy(name,val)); }                 
                    template<bool allowWrite = true, typename T, typename OutFun> void spy(const std::string & name, T & val, OutFun & outfun) { NOBASICWATCH(_get(0)->spy(name,val,outfun)); }
                    template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void spy(const std::string & name, T & val, OutFun & outfun, InFun & infun) { NOBASICWATCH(_get(0)->spy(name,val,outfun,infun)); }                 
                    template<bool allowWrite = true, typename T> void operator()(const std::string & name, T & val) { NOBASICWATCH(_get(0)->operator()(name,val)); }                 
                    template<bool allowWrite = true, typename T, typename OutFun> void operator()(const std::string & name, T & val, OutFun & outfun) { NOBASICWATCH(_get(0)->operator()(name, val,outfun)); }                 
                    template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void operator()(const std::string & name, T & val, OutFun & outfun, InFun & infun) { NOBASICWATCH(_get(0)->operator()(name, val,outfun,infun)); }                 
                    template<typename T> inline void flush(const T & v, const size_t tick = 100) { NOBASICWATCH(_get(0)->flush(v,tick));  }
                    template<typename T> inline void operator[](const T & v) { NOBASICWATCH(_get(0)->operator[](v)); }

                private:

                    WatchWindow * _get(int);
                    GlobalWatchWindow(const GlobalWatchWindow &) = delete;              // no copy
                    GlobalWatchWindow & operator=(const GlobalWatchWindow &) = delete;  //
                };

#undef NOBASICWATCH

            }

        static internals_watch::GlobalWatchWindow watch; ///< static object redirecting to the global watch window.


}


/* end of file */

