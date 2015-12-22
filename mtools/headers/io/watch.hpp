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

#include <FL/Fl_Button.H>

namespace mtools
    {


    /* forward declarations */
    namespace internals_watch
        {

        /* forward declaration */
        class FltkWatchWin;


        /** Pure virtual base class for a watchable object */
        struct WatchObj
            {
                WatchObj(const std::string & name, int rate);
                virtual ~WatchObj();
                std::string get(bool & changed) const;
                bool set(const std::string & value);
                virtual bool writable() const;
                int refreshRate() const;
                int refreshRate(int newrate);
                void assignFltkWin(FltkWatchWin * p, Fl_Button * nameButton, Fl_Button * valueButton);

                virtual std::string _get() const;
                virtual size_t _set(const std::string & value);

                int _rate;                          // refresh rate (number of time per minutes)
                mutable std::string _prev_value;    // previous value
                FltkWatchWin * _fltkwin;            // the associated watch window
                Fl_Button * _name_button;           // fltk button with the name
                Fl_Button * _value_button;          // fltk button with the value
                std::string _name;                  // identifier name of the variable
            };


        /**
        * A watch object containing a variable of type T
        **/
        template<typename T, bool allowWrite> struct WatchObjVar : public WatchObj
            {
                /** Constructor **/
                WatchObjVar(const std::string & name, const T & val, int rate) : WatchObj(name, rate), _p(const_cast<T*>(&val)) {}

                /** Destructor. */
                virtual ~WatchObjVar() override {}

                /**
                * Return true if the object is writable and false otherwise.
                **/
                virtual bool writable() const override { return _writable; }

                /** Gets the string representing the object  **/
                virtual std::string _get() const override { return mtools::toString(*_p); }

                /**
                * Sets the given value and return the number of character read from the input.
                * Do nothing and return 0 if if writable is false or if the object cannot
                * be set using mtools::fromString().
                **/
                virtual size_t _set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<_writable>()); }

                size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { return mtools::fromString(str, *(const_cast<T*>(_p))); } // set value using fromString()
                size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

                mutable volatile T * _p;      // pointer to the watched object
                static const bool _writable = (allowWrite && mtools::metaprog::has_from_istream<T>::value);  // true if the object is writable and false otherwise
            };



        /**
        * A watch object containing a variable of type T, use a function/functor for output
        **/
        template<typename T, typename OutFun, bool allowWrite> struct WatchObjVarOut : public WatchObj
            {
                /** Constructor **/
                WatchObjVarOut(const std::string & name, const T & val, OutFun & outfun, int rate) : WatchObj(name, rate), _p(const_cast<T*>(&val)), _outfun(&outfun) {}

                /** Destructor. */
                ~WatchObjVarOut() override {}

                /**
                * Return true if the object is writable and false otherwise.
                **/
                virtual bool writable() const override { return _writable; }

                /** Gets the string representing the object  **/
                virtual std::string _get() const override { return mtools::toString((*_outfun)(*_p)); }

                /**
                * Sets the given value and return the number of character read from the input.
                * Do nothing and return 0 if if writable is false or if the object cannot
                * be set using mtools::fromString().
                **/
                virtual size_t _set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<_writable>()); }

                size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { return mtools::fromString(str, *(const_cast<T*>(_p))); } // set value using fromString()
                size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

                mutable volatile T * _p;                 // pointer to the watched object
                OutFun * _outfun;       // pointer to the output functor
                static const bool _writable = (allowWrite && mtools::metaprog::has_from_istream<T>::value);  // true if the object is writable and false otherwise
            };


        /**
        * A watch object containing a variable of type T, use functions/functors for output and input
        **/
        template<typename T, typename OutFun, typename InFun, bool allowWrite> struct WatchObjVarOutIn : public WatchObj
            {
                /** Constructor **/
                WatchObjVarOutIn(const std::string & name, const T & val, OutFun & outfun, InFun & infun, int rate) : WatchObj(name, rate), _p(const_cast<T*>(&val)), _outfun(&outfun), _infun(&infun) {}

                /** Destructor. */
                ~WatchObjVarOutIn() override {}

                /**
                * Return true if the object is writable and false otherwise (ie if allowWrite was set to false)
                **/
                virtual bool writable() const override { return allowWrite; }

                /** Gets the string representing the object  **/
                virtual std::string _get() const override { return mtools::toString((*_outfun)(*_p)); }

                /**
                * Sets the given value (if writable, otherwise do nothing), then return 0.
                **/
                virtual size_t _set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<allowWrite>()); }

                size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { (*_infun)(str, *(const_cast<T*>(_p))); return 0; } // set value using infun
                size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

                mutable volatile T * _p;                 // pointer to the watched object
                OutFun * _outfun;       // pointer to the output functor
                InFun *  _infun;        // pointer to the output functor
            };



        class FltkWatchWin;

        }



        class WatchWindow
            {

            public:


                static const int DEFAULT_REFRESHRATE = 60; // default refresh rate = every second.


                /** Default constructor. */
                WatchWindow();


                /** Destructor. */
                ~WatchWindow();


                /**
                * Removes a variable from the spy window.
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


                /** Same as spy(name,val) **/
                template<bool allowWrite = true, typename T> void operator()(const std::string & name, T & val) { spy<allowWrite, T>(name, val); }

                /** Same as spy(name,val,outfun) **/
                template<bool allowWrite = true, typename T, typename OutFun> void operator()(const std::string & name, T & val, OutFun & outfun) { spy<allowWrite, T, OutFun>(name, val, outfun); }

                /** Same as spy(name,val,outfun,infun) **/
                template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void operator()(const std::string & name, T & val, OutFun & outfun, InFun & infun) { spy<allowWrite, T, OutFun, InFun>(name, val, outfun, infun); }


            private:

                /** Creates the fltk object if it does not exist yet */
                void createIfNeeded();

                /** transmit to the fltk object */
                void transmit(const std::string & name, internals_watch::WatchObj * p);

                WatchWindow(const WatchWindow &) = delete;              // no copy
                WatchWindow & operator=(const WatchWindow &) = delete;  //

                internals_watch::FltkWatchWin* _fltkobj;  // the object running inside the fltk thread.
            };





}


/* end of file */

