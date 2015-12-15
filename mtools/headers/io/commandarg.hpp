/** @file commandarg.hpp */
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

#include "../misc/stringfct.hpp"
#include "../misc/error.hpp"

#include <string>
#include <vector>
#include <map>


namespace mtools
    {

    namespace internals_commandarg
        {


        /** info about an command line option */
        struct OptObj
            {
            OptObj() : hasvalue(false), hasdefaultvalue(false), displayed(false), value(), defaultvalue(), info() {} // default ctor
            OptObj(bool hval, bool hdefval, const std::string & val, const std::string & defval) : hasvalue(hval), hasdefaultvalue(hdefval), displayed(false), value(val), defaultvalue(defval), info() {} // ctor
            OptObj(const OptObj &) = default;               // default copy
            OptObj & operator=(const OptObj &) = default;   // and assigmement operators

            bool hasvalue;              // true if there is a value associated
            bool hasdefaultvalue;       // true if there is a default value
            bool displayed;             // true if the option was already displayed
            std::string value;          // associated value if any 
            std::string defaultvalue;   // associated default value if any 
            std::string info;           // info string 
            };


        static bool interactive = true;            // true to use interactive mode for missing argument and false otherwise
        static bool display = true;                // true if we should display the option as they are queried for the first time. 
        static bool parsed = false;                // true if we already parsed the command line

        static std::map<std::string, OptObj> mapOpt;    // map with all command options and associated values
        static std::vector<std::string> freeArgsVec;    // vector containing the remaining free arguments
        static std::string command;                     // the command used to run the program



                                                        /* Remove the '-' at the beginning of str.
                                                        * Return true if there was at least one */
        bool removeMinus(std::string & str)
            {
            size_t i = 0;
            while ((i < str.length()) && (str[i] == '-')) { i++; }
            if (i == 0) return false;
            str = str.substr(i);
            return true;
            }


        /* Parse an string of the form "name[=value]"
        * @return  true if the option has a value and false otherwise. */
        bool parseArg(const std::string & str, std::string & name, std::string & value)
            {
            size_t p = str.find('=');
            if (p == str.npos) { name = str; value.clear(); if (name.length() == 0) { MTOOLS_ERROR("parseArg: no name"); } return false; }
            name = str.substr(0, p); value = str.substr(p + 1);
            if (name.length() == 0) { MTOOLS_ERROR("parseArg: no name"); }
            return true;
            }


        /* return the option name  */
        std::string optName(std::string str)
            {
            std::string name, value;
            str = removeMinus(str);
            parseArg(str, name, value);
            return name;
            }


        /** A proxy class used from simulating return type template deduction */
        class ProxyArg
            {

            public:

                /**
                * Implicit conversion from argument to type T
                **/
                template<typename T> operator T()
                    {
                    OptObj & opt = mapOpt[_name];
                    if (!opt.hasvalue)
                        { // no value yet. 
                        if (!interactive)
                            {
                            if (!opt.hasdefaultvalue) { MTOOLS_ERROR(std::string("Error : command line argument [") + _name + "] has neither value nor default value..."); }
                            opt.value = opt.defaultvalue;
                            }
                        else
                            {
                            // interactive mode : query the value
                            auto status = cout.useDefaultInputValue();  // save the way cout display input values
                            cout << ((opt.info.length() > 0) ? opt.info : std::string("Parameter")) << " [" << _name << "] : "; // display the question 
                            T val;
                            if (opt.hasdefaultvalue) { fromString(opt.defaultvalue, val); cout.useDefaultInputValue(true); }
                            else { cout.useDefaultInputValue(false); } // load the default value if any 
                            cout >> val; opt.value = toString(val); cout << opt.value << "\n"; // query, save and display the value
                            opt.displayed = true; // set as already displayed
                            cout.useDefaultInputValue(status);  // restore the way cout display input values
                            }
                        opt.hasvalue = true;
                        }
                    T val;
                    fromString(opt.value, val); // convert to the corresponding type
                    if ((display) && (!opt.displayed))
                        { // display on the console if required
                        cout << ((opt.info.length() > 0) ? opt.info : std::string("Parameter")) << " [" << _name << "] : " << toString(val) << "\n";
                        opt.displayed = true;
                        }
                    return val; // return the value
                    }


                /**
                * Associate/update the info string of the option
                **/
                ProxyArg & info(const std::string & infostr)
                    {
                    mapOpt[_name].info = infostr;
                    return(*this);
                    }


                /* return a proxy to a given option, create the option if it does not exist yet */
                static ProxyArg _get(std::string  str, std::string defaultvalue, bool hasdefaultvalue)
                    {
                    if (!internals_commandarg::parsed) { MTOOLS_ERROR(std::string("The command line was not yet parsed using parseCommandine().")); }
                    removeMinus(str);  // remove leading '-' if any
                    std::string name, val;
                    if (parseArg(str, name, val)) // get theo ption name (and possible default value)
                        { // there is a default value
                        if (!hasdefaultvalue)
                            { // use it if no other default value given
                            hasdefaultvalue = true; defaultvalue = val;
                            }
                        }
                    if (mapOpt.find(name) == mapOpt.end())
                        { // option does not exist so we create it;
                        mapOpt.insert(std::pair< std::string, OptObj>(name, OptObj(false, hasdefaultvalue, std::string(""), defaultvalue)));
                        }
                    else
                        { // update the default value if required.
                        if (hasdefaultvalue) { mapOpt[name].hasdefaultvalue = true; mapOpt[name].defaultvalue = defaultvalue; }
                        }
                    return ProxyArg(name); // return a proxy to the option
                    }


                ProxyArg(const ProxyArg &) = default;               // default copy ctor
                ProxyArg& operator=(const ProxyArg &) = default;    // and assignement operator

            private:

                /** Private constructor: bind the proxy with a given option in the map */
                ProxyArg(const std::string & name) : _name(name) { }

                const std::string _name;    // the option name
            };

        }


    /**
    * Return the argument associated with an option
    * try to create it if it does not exist.
    **/
    inline internals_commandarg::ProxyArg arg(const std::string & name) { return internals_commandarg::ProxyArg::_get(name, std::string(""), false); }


    /**
    * Return the argument associated with an option
    * try to create it if it does not exist.
    **/
    template<typename T> inline internals_commandarg::ProxyArg arg(const std::string & name, const T & defaultval) { return internals_commandarg::ProxyArg::_get(name, mtools::toString(defaultval), true); }


    /**
    * Return the argument associated with an option
    * try to create it if it does not exist.
    **/
    inline internals_commandarg::ProxyArg arg(char c) { return internals_commandarg::ProxyArg::_get(std::string(1, c), std::string(""), false); }


    /**
    * Return the argument associated with an option
    * try to create it if it does not exist.
    **/
    template<typename T> inline internals_commandarg::ProxyArg arg(char c, const T & defaultval) { return internals_commandarg::ProxyArg::_get(std::string(1, c), mtools::toString(defaultval), true); }


    /**
    * Query if a given option exists.
    *
    * @param   name    The name of the option to query.
    *
    * @return  Return 0 is the option does not exist, 1 if it exist and has no value associated to
    *          it and 2 if it exists and has a value associated to it.
    **/
    inline int isarg(const std::string name)
        {
        auto it = internals_commandarg::mapOpt.find(internals_commandarg::optName(name));
        if (it == internals_commandarg::mapOpt.end()) return 0;
        if (it->second.hasvalue) return 2;
        return 1;
        }


    /**
    * Return the number of free arguments in the command line (ie arguments that are not options).
    **/
    inline int freearg() { return internals_commandarg::freeArgsVec.size(); }


    /**
    * Return a given (zero indexed) free argument from the command line.
    **/
    inline std::string freearg(size_t index)
        {
        MTOOLS_ASSERT(index < internals_commandarg::freeArgsVec.size());
        return internals_commandarg::freeArgsVec[index];
        }


    /**
    * Parse the command line.
    * This method must be called only once
    *
    * @param   argc            The argc passed to main
    * @param [in,out]  argv    The argv passed to main
    * @param   interactive     true to query missing paramter on the lfy using cout.
    * @param   display         true to display information about each paramter used using cout.
    **/
    void parseCommandLine(int argc, char** argv, bool interactive = true, bool display = true)
        {
        if (internals_commandarg::parsed) { MTOOLS_ERROR("parseCommandLine() method was already called."); }
        else { internals_commandarg::parsed = true; }
        internals_commandarg::interactive = interactive;    // set the flags
        internals_commandarg::display = display;            //
        if (argc < 1) { MTOOLS_ERROR("parseCommandLine() : argc < 1."); }
        if (argv == nullptr) { MTOOLS_ERROR("parseCommandLine() : argv = nullptr."); }
        internals_commandarg::command = argv[0]; // save the command invoqued for running the program
        for (int i = 1; i < argc; i++)
            {
            std::string str(argv[i]);
            if (internals_commandarg::removeMinus(str))
                { // option
                std::string name, value;
                bool hasvalue = internals_commandarg::parseArg(str, name, value);
                if (internals_commandarg::mapOpt.find(internals_commandarg::optName(name)) != internals_commandarg::mapOpt.end()) { MTOOLS_ERROR(std::string("parseCommandLine() error, option [") + name + "] already defined."); }
                internals_commandarg::mapOpt.insert(std::pair< std::string, internals_commandarg::OptObj>(name, internals_commandarg::OptObj(hasvalue, false, value, std::string(""))));
                }
            else
                { // free arg
                internals_commandarg::freeArgsVec.push_back(str);
                }
            }
        return;
        }

    }

	
/* end of file */


