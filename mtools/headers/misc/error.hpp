/** @file error.hpp */
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


#include <string>

/* uncomment the next line to enable MTOOLS_DEBUG even in release mode */
//#define MTOOLS_DEBUG_IN_RELEASEMODE



/* forward declaration */
namespace mtools
{
    namespace internals_error
    {

        void _error(const std::string & file, int line, const std::string & s);
        bool _insures(const std::string & file, int line, const std::string & s);
        bool _assert(const std::string & file, int line, const std::string & s);
        void _debugs(const std::string & file, int line, const std::string & s);
        void _throws_debug(const std::string & file, int line, const std::string & s);
        void _throws_nodebug(const std::string & file, int line, const std::string & s);
        }
}


/**
 * A macro that define an error.
 *
 * @param   _ex The message associated with the error.
 **/
#define MTOOLS_ERROR(_ex) mtools::internals_error::_error(__FILE__,__LINE__,(std::string("") + (_ex)))

#undef MTOOLS_ASSERT
#ifdef NDEBUG 
/**
 * assert macro. report the error and exit if the ocndition fails. compile to nothing if NDEBUG 
 * is defined.
 **/
#define MTOOLS_ASSERT(_ex) ((void)0)    ///< Assert macro, compile to nothing if NDEBUG is defined
#else
#define MTOOLS_ASSERT(_ex) ((void)( (!!(_ex)) || (mtools::internals_error::_assert(__FILE__ , __LINE__,(#_ex))) ))
#endif

/**
 * Insure macro, report an error and exit the program if the conditon fails. Same as MTOOLS_ASSERT 
 * but always active, even if NDEBUG is defined.
 **/
#define MTOOLS_INSURE(_ex) ((void)( (!!(_ex)) || (mtools::internals_error::_insures(__FILE__ , __LINE__,(#_ex))) ))


#ifndef NDEBUG
#define MTOOLS_DEBUG_IN_RELEASEMODE
#endif
#ifdef MTOOLS_DEBUG_IN_RELEASEMODE
/**
 * DEBUG macro for printing out info on clog
 **/
#define MTOOLS_DEBUG(_ex) mtools::internals_error::_debugs(__FILE__ , __LINE__,(std::string("") + (_ex)))
#define MTOOLS_THROW(_ex) mtools::internals_error::_throws_debug(__FILE__ , __LINE__,(std::string("") + (_ex)))
#else
#define MTOOLS_DEBUG(_ex) ((void)0)
#define MTOOLS_THROW(_ex) mtools::internals_error::_throws_nodebug(__FILE__ , __LINE__,(std::string("") + (_ex)))
#endif


/* end of file */



