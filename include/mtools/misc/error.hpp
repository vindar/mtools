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

#include "../misc/internal/mtools_export.hpp"

#include <string>
#include <iostream>
#include <sstream>



namespace mtools
{




	/* typedef for the error callback function */
	typedef void(*mtools_error_cb)(const std::string & title, const std::string & msg);


	/**
	 * Set the user defined callback function called by MTOOLS_ASSERT(), MTOOLS_INSURES(),  MTOOLS_ERROR() and MTOOLS_THROW()
	 *
	 * @param	cb	The callback with signature void 'error_cb(const std::string & error_title, const std::string & error_msg)'
	 **/
	void setErrorCallback(mtools_error_cb cb);


	/** Removes the error callback function (if any). */
	void removeErrorCallBack();





    namespace internals_error
    {


	extern mtools_error_cb _error_cb; // the error callback function



    std::string truncateFilename(std::string s);

    void display(const std::string & title, const std::string & msg);

    void displayGraphics(const std::string & title, const std::string & msg);

    inline void _stopWithMsg(const std::string & title, const std::string & msg)
        {
		if (_error_cb != nullptr) _error_cb(title, msg);
        display(title, msg);
        #if (MTOOLS_BASIC_CONSOLE == 0)
        displayGraphics(title, msg);
        #endif
        exit(EXIT_FAILURE);
        }

    inline void _error(const std::string & file, int line, const std::string & s)
        {
		std::ostringstream os;
		os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nMessage : " << s;
		if (_error_cb != nullptr) _error_cb("MTOOLS_ERROR", os.str());
        _stopWithMsg("MTOOLS_ERROR", os.str());
        }

    inline bool _insures(const std::string & file, int line, const std::string & s)
        {
		std::ostringstream os; 
		os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s;
		if (_error_cb != nullptr) _error_cb("MTOOLS_INSURE_FAILURE", os.str());
		_stopWithMsg("MTOOLS_INSURE FAILURE", os.str());
        return true;
        }

	inline bool _insures(const std::string & file, int line, const std::string & s, const std::string & msg)
		{
		std::ostringstream os;
		os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s << "\nMessage : " << msg;
		if (_error_cb != nullptr) _error_cb("MTOOLS_INSURE_FAILURE", os.str());
		_stopWithMsg("MTOOLS_INSURE FAILURE", os.str());
		return true;
		}


    inline bool _assert(const std::string & file, int line, const std::string & s)
        {
		std::ostringstream os;
		os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s;
		if (_error_cb != nullptr) _error_cb("MTOOLS_ASSERT_FAILURE", os.str());
		_stopWithMsg("MTOOLS_ASSERT FAILURE", os.str());
        return true;
        }


	inline bool _assert(const std::string & file, int line, const std::string & s, const std::string & msg)
		{
		std::ostringstream os;
		os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nCondition : " << s << "\nMessage : " << msg;
		if (_error_cb != nullptr) _error_cb("MTOOLS_ASSERT_FAILURE", os.str());
		_stopWithMsg("MTOOLS_ASSERT FAILURE", os.str());
		return true;
		}


    void _debugs(const std::string & file, int line, const std::string & s);


    inline void _throws_debug(const std::string & file, int line, const std::string & s)
        {
		_debugs(file, line, s);
		if (_error_cb != nullptr)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nThrow : " << s;
			_error_cb("MTOOLS_THROW", os.str());
			}
		throw s.c_str();
        }


    inline void _throws_nodebug(const std::string & file, int line, const std::string & s)
        {
		if (_error_cb != nullptr)
			{
			std::ostringstream os;
			os << "File : " << truncateFilename(file) << "\nLine : " << line << "\nThrow : " << s;
			_error_cb("MTOOLS_THROW", os.str());
			}
		throw s.c_str();
        }

    }
}




/* used for overloading macro with 1 or 2 parameters, see : https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments/11763196 */
#define MTOOLS_MSVC_BUGFIX_EXPAND(x) x
#define MTOOLS_GET_MACRO_1_2_PARAM(_1, _2, _name, ...) _name


/* Error macro : use stringstream so we can use << to concatenate messages */
#define MTOOLS_ERROR(_ex) mtools::internals_error::_error(__FILE__,__LINE__,(std::ostringstream() << _ex).str())

/* Insure macro */
#define MTOOLS_INSURE_1(_ex)       ((void)( (!!(_ex)) || (mtools::internals_error::_insures(__FILE__ , __LINE__,(#_ex))) ))
#define MTOOLS_INSURE_2(_ex, _msg) ((void)( (!!(_ex)) || (mtools::internals_error::_insures(__FILE__ , __LINE__, (#_ex) , (std::ostringstream() << _msg).str()))))

#define MTOOLS_INSURE(...) MTOOLS_MSVC_BUGFIX_EXPAND(MTOOLS_GET_MACRO_1_2_PARAM(__VA_ARGS__, MTOOLS_INSURE_2, MTOOLS_INSURE_1, _UNUSED)(__VA_ARGS__))



#ifdef MTOOLS_DEBUG_FLAG

	#define MTOOLS_THROW(_ex) mtools::internals_error::_throws_debug(__FILE__ , __LINE__, (std::ostringstream() << _ex).str())

	#define MTOOLS_DEBUG(_ex) mtools::internals_error::_debugs(__FILE__ , __LINE__, (std::ostringstream() << _ex).str())
	
	#define MTOOLS_DEBUG_CODE(_code) { _code }

	#define MTOOLS_ASSERT_1(_ex) ((void)( (!!(_ex)) || (mtools::internals_error::_assert(__FILE__ , __LINE__,(#_ex))) ))
	#define MTOOLS_ASSERT_2(_ex, _msg) ((void)( (!!(_ex)) || (mtools::internals_error::_assert(__FILE__ , __LINE__,(#_ex), (std::ostringstream() << _msg).str()))))

#else

	#define MTOOLS_THROW(_ex) mtools::internals_error::_throws_nodebug(__FILE__ , __LINE__, (std::ostringstream() << _ex).str())

	#define MTOOLS_DEBUG(_ex) ((void)0)

	#define MTOOLS_DEBUG_CODE(_code) ((void)0);

	#define MTOOLS_ASSERT_1(_ex) ((void)0)
	#define MTOOLS_ASSERT_2(_ex, _msg) ((void)0)


#endif


#define MTOOLS_ASSERT(...) MTOOLS_MSVC_BUGFIX_EXPAND(MTOOLS_GET_MACRO_1_2_PARAM(__VA_ARGS__, MTOOLS_ASSERT_2, MTOOLS_ASSERT_1, _UNUSED)(__VA_ARGS__))



/* end of file */



