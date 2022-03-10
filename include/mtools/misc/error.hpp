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

	void _stopWithMsg(const std::string & title, const std::string & msg);

	void _error(const std::string & file, int line, const std::string & s);

	bool _insures1(const std::string & file, int line, const std::string & s);

	bool _insures2(const std::string & file, int line, const std::string & s, const std::string & msg);

	bool _asserts1(const std::string & file, int line, const std::string & s);

	bool _asserts2(const std::string & file, int line, const std::string & s, const std::string & msg);

    void _debugs(const std::string & file, int line, const std::string & s);

	void _throws_debug(const std::string & file, int line, const std::string & s);

	void _throws_nodebug(const std::string & file, int line, const std::string & s);



	class ErrorOSS
		{
	public:
		ErrorOSS() : oss() {}
		template<typename T> ErrorOSS& operator<<(const T& v) { oss << v;  return *this;}
		const std::string str() { return oss.str();}

	private:
		std::ostringstream oss; 
		};


    }
}


#define MTOOLS_ERROR_OSS(_ex) (((internals_error::ErrorOSS()) << _ex).str())

/* used for overloading macro with 1 or 2 parameters, see : https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments/11763196 */
#define MTOOLS_MSVC_BUGFIX_EXPAND(x) x
#define MTOOLS_GET_MACRO_1_2_PARAM(_1, _2, _name, ...) _name


/* Error macro : use stringstream so we can use << to concatenate messages */
#define MTOOLS_ERROR(_ex) mtools::internals_error::_error(__FILE__,__LINE__, MTOOLS_ERROR_OSS(_ex) )

/* Insure macro */
#define MTOOLS_INSURE_1(_ex)       ((void)( (!!(_ex)) || (mtools::internals_error::_insures1(__FILE__ , __LINE__,(#_ex))) ))
#define MTOOLS_INSURE_2(_ex, _msg) ((void)( (!!(_ex)) || (mtools::internals_error::_insures2(__FILE__ , __LINE__, (#_ex) , MTOOLS_ERROR_OSS(_ex) ))))
#define MTOOLS_INSURE(...) MTOOLS_MSVC_BUGFIX_EXPAND(MTOOLS_GET_MACRO_1_2_PARAM(__VA_ARGS__, MTOOLS_INSURE_2, MTOOLS_INSURE_1, _UNUSED)(__VA_ARGS__))



#ifdef MTOOLS_DEBUG_FLAG

	#define MTOOLS_THROW(_ex) mtools::internals_error::_throws_debug(__FILE__ , __LINE__, MTOOLS_ERROR_OSS(_ex))

	#define MTOOLS_DEBUG(_ex) mtools::internals_error::_debugs(__FILE__ , __LINE__, MTOOLS_ERROR_OSS(_ex))

	#define MTOOLS_DEBUG_CODE(_code) { _code }

	#define MTOOLS_ASSERT_1(_ex)	   ((void)( (!!(_ex)) || (mtools::internals_error::_asserts1(__FILE__ , __LINE__,(#_ex)))))
	#define MTOOLS_ASSERT_2(_ex, _msg) ((void)( (!!(_ex)) || (mtools::internals_error::_asserts2(__FILE__ , __LINE__,(#_ex), MTOOLS_ERROR_OSS(_ex)))))
	#define MTOOLS_ASSERT(...) MTOOLS_MSVC_BUGFIX_EXPAND(MTOOLS_GET_MACRO_1_2_PARAM(__VA_ARGS__, MTOOLS_ASSERT_2, MTOOLS_ASSERT_1, _UNUSED)(__VA_ARGS__))

#else

	#define MTOOLS_THROW(_ex) mtools::internals_error::_throws_nodebug(__FILE__ , __LINE__, MTOOLS_ERROR_OSS(_ex))

	#define MTOOLS_DEBUG(_ex) ((void)0)

	#define MTOOLS_DEBUG_CODE(_code) ((void)0);

	#define MTOOLS_ASSERT_1(_ex) ((void)0)
	#define MTOOLS_ASSERT_2(_ex, _msg) ((void)0)
	#define MTOOLS_ASSERT(...) ((void)0)

#endif



/* end of file */



