/** @file stringfct.hpp */
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


#include "misc.hpp"
#include "error.hpp"

#include <cstring>

namespace mtools
{
    /**
     * Enum for diferent character encodings.
     **/
    enum StringEncoding { enc_utf8, enc_iso8859, enc_unknown };
}

#include "internal/internals_stringfct.hpp"






namespace mtools
{

    /**
     * Reverse a string/wstring.
     *
     * @param   s   a string/wstring.
     *
     * @return  the reversed string.
     *
     * @sa  reverseInPlace
     **/
    template<class T> T reverse(T s) { std::reverse(s.begin(), s.end()); return s; }


    /**
     * Reverse a string/wstring. In place version.
     *
     * @param [in,out]  s   the string/wstring to reverse.
     *
     * @sa  reverse
     **/
    template<class T> void reverseInPlace(T & s) { std::reverse(s.begin(), s.end()); return; }


    /**
     * Replace all the occurence of oldstr by newstr in buffer. Linear search: continu the search
     * just after the previous replacement.
     *
     * @param [in,out]  buffer  The buffer where the replacement take place.
     * @param   oldstr          string to search for.
     * @param   newstr          string to replace with.
     *
     * @return  the number of replacement done.
     **/
    size_t replace(std::string & buffer, const std::string & oldstr, const std::string & newstr);


    /**
     * Truncate a string/wstring keeping only the first nb characters
     *
     * @param   s   the string to truncate.
     * @param   nb  number of chars to keep.
     *
     * @return  the truncated string/wstring.
     *
     * @sa  removeBeginning
     **/
    template<class T> T keepBeginning(T s, size_t nb) { if (nb >= s.length()) { return s; } s.resize(nb, 0); return s; }


    /**
     * Remove a specified number chars from the beginning of a string/wstring.
     *
     * @param   s   the string/wstring to truncate.
     * @param   nb  number of chars to remove at the beginning.
     *
     * @return  the truncated string/wstring.
     *
     * @sa  keepBeginning
     **/
    template<class T> T removeBeginning(T s, size_t nb) { if (nb == 0) { return s; } if (nb >= s.length()) { s.clear(); return s; } T r(s, nb, s.length() - nb); return r; }

	
	/**
	 * Remove everything after the first null char.
	 *
	 * @param [in,out]	s	The string to process.
	 *
	 * @return	the string troncated after its first null char. 
	 **/
	template<class T> T troncateAfterNullChar(const T & s)
		{
		const size_t n = s.find((char)0, 0);
		return((n == std::string::npos) ? s : s.substr(0, n));
		}
	

    /**
     * Removes the left spaces of a string.
     *
     * @param   s   the string to process.
     *
     * @return  the string without space at the beginnning.
     **/
    template<class T> T removeLeftSpaces(const T & s)
        {
        size_t i;
        for (i = 0; i < s.length(); i++) { if (!isspace(s[i])) break; }
        T res(s, i, T::npos);
        return res;
        }


    /**
     * Removes the right spaces of a string.
     *
     * @param   s   the string to process.
     *
     * @return  the string without space at the end.
     **/
    template<class T> T removeRightSpaces(const T & s) { T r = removeLeftSpaces(reverse(s)); reverseInPlace(r); return r; }


    /**
     * Removes the left and right spaces of a string.
     *
     * @param   s   the string to process.
     *
     * @return  the string without space at the beginnning and end.
     **/
    template<class T> T removeLeftRightSpaces(const T & s) { return removeRightSpaces(removeLeftSpaces(s)); }


    /**
     * Convert a string to lower case.
     *
     * @param   s   the string to process.
     *
     * @return  the string in lower case.
     **/
    template<class T> T toLowerCase(T s) { for (auto it = s.begin(); it != s.end(); it++) { (*it) = tolower((*it)); } return s; }


    /**
     * Convert a string to upper case.
     *
     * @param   s   the string to process.
     *
     * @return  the string in upper case.
     **/
    template<class T> T toUpperCase(T s) { for (auto it = s.begin(); it != s.end(); it++) { (*it) = toupper((*it)); } return s; }


    /**
     * Converts an integer type to an hexadecimal string.
     *
     * @tparam  T   integer type parameter.
     * @param   val     Value to convert.
     * @param   width   number of chars for the resulting string (negative for default value).
     *
     * @return  The given data converted to a std::string.
     **/
    template<class T> std::string toHexString(const T& val, int width = -1)
        {
        std::ostringstream oss;
        oss << std::hex;
        if (width > 0) { oss << std::setfill('0') << std::setw(width); }
        oss << val;
        return oss.str();
        }


    /**
     * Save a portion of memory into a string. The string is written in hexadecimal format
     * (uppercase) hence the lenght of the returned string is twice the number of bytes read (len).
     *
     * @param   p   pointer to the beginning of the memory to save.
     * @param   len number of bytes to save.
     *
     * @return  the string containing the memory in hexadecimal format.
     *
     * @sa  stringToMemory
     **/
    std::string memoryToString(const void * p, size_t len);


    /**
     * Restore a portion of memory from a string. The string must be in (uppercase) hexadecimal
     * format (created by memoryToString for instance).
     *
     * @param   s       the string to containing the data in hexadecimal format.
     * @param [out] p   pointer to the beginning of the memory to write.
     *
     * @return  the number of byte written in the buffer pointed by p. (should be length(s)/2).
     *
     * @sa  memoryToString
     **/
    size_t stringToMemory(const std::string & s, void * p);


    /**
     * Creates a 'Escaped C-string' from a a buffer. The resulting string contain only characters
     * between 32 and 126. Other characters are escaped in C -style with either \\n, \\\", \\a, etc...
     * or using \\XXX for the octal representation. The returned sting is perfectly compatible with C-
     * strings.
     *
     * @param [in,out]  dest        the destination buffer, if the string is not empty the token is
     *                              appended to the current content of the string.
     * @param   source_buf          the source buffer to tokenize.
     * @param   source_len          the lenght of the source buffer.
     * @param   opaqueHex           true to use opaque hexadecimal format ie the whole buffer is
     *                              encoded in the form '\\xXXXXXXXXXXXXXXX...'.
     * @param   surroundWithQuotes  true to surround the token with double quotes \" \". Call
     *                              doTokenNeedQuotes() to determine if quotes must be added to
     *                              insure that token are correctly identified as such when decoding
     *                              it. When not using quotes, Space characters are replaced by \\40.
     *
     * @return  the size of the created token.
    **/
    size_t createToken(std::string & dest, const void * source_buf, size_t source_len, bool opaqueHex, bool surroundWithQuotes);


    /**
     * Determine if the token created from a given buffer must be surounded with quotes.This is the
     * case, if the source as lenght 0, or start with "%" or contain space characters.
     *
     * @param   source      the source buffer.
     * @param   source_len  the lenght of the buffer.
     *
     * @return  true if quotes must be used and false if they are unnecessary.
    **/
    bool doesTokenNeedQuotes(const void * source, const size_t source_len);


    /* prototype for the readmore function */
    typedef const char* (*preadMoreToken)(size_t &, void *);


    namespace internals_stringfct
        {

        /* default "read more function" return nullptr which menas there is no more data to read */
        inline const char* readNoMore(size_t & len, void * data) { return ((const char*)nullptr); }

        /* write a char in the output buffer */
        inline void _readToken_put(unsigned char * dest, size_t dest_len, size_t & m, unsigned char c) { if (m >= dest_len) MTOOLS_THROW("destination buffer too small"); dest[m++] = c; }

        /* write a char in the output buffer */
        inline void _readToken_put(std::string & dest, size_t & m, unsigned char c) { dest.push_back(c); m++; }

        /* return the value of the hexadecimal letter */
        inline unsigned char _readToken_hexValue(unsigned char c)
            {
            if ((c >= '0') && (c <= '9')) return (c - '0');
            if ((c >= 'A') && (c <= 'F')) return (c - 'A' + 10);
            if ((c >= 'a') && (c <= 'f')) return (c - 'a' + 10);
            return 255;
            }

        /* read a char from the input buffer, set eof=true if there is no more data */
        template< preadMoreToken inputFun  > unsigned char _readToken_get(const unsigned char * & source, size_t & source_len, size_t & n, bool usequote, void * data, bool & eof)
            {
            if (n >= source_len)
                {
                auto old_source = source; auto old_len = source_len; n = 0;
                source = (const unsigned char *)inputFun(source_len, data);
                if (source == nullptr) { if (usequote) MTOOLS_THROW("parse error"); source = old_source; source_len = old_len; eof = true; return 0; }
				MTOOLS_ASSERT(source_len > 0); // when a non null pointer is returned, there should always be at least one byte to read...
                }
            return source[n++];
            }

        }


    /**
    * Find the begining of the first token, excluding comments lines.
    * (comments are starting with '%' and continue until a new line char '\n' or another '%' is
    * found).
    *
    * @tparam  putFun      Type of the put fun.
    * @tparam  inputFun    optional function which return the addtional data.
    * @param   source_buf          source buffer. When the function return, this pointer can be
    *                              modified (if inputFun was defined) to point to the segment
    *                              containing the start of the token.
    * @param [in,out]  source_len  length of the buffer. Modified when the function return to point
    *                              to offset of the first char of the token.
    * @param [in,out]  data        opaque additonal data to pass to the inputFun function for
    *                              personal use.
    *
    * @return  true if a token was found and false if end of file was reached without finding any
    *          token, if true, source_buf[source_len] now point to the first char of the token.
    **/
    template< preadMoreToken inputFun = internals_stringfct::readNoMore > bool findNextToken(const char * & source_buf, size_t & source_len, void * data = nullptr)
        {
        const unsigned char * source = (const unsigned char *)source_buf;
        if ((source==nullptr)||(source_len == 0)) // if the buffer source buffer is empty, we try to get something in it.
			{
            source = (const unsigned char *)inputFun(source_len, data);
            if (source == nullptr) { return false; } // nothing to read... so there is no token to find.
			MTOOLS_ASSERT(source_len > 0); // there must be at least one byte to read in source[].
			}
        size_t n = 0;
        bool eof = false;
        bool commentOn = false;
        while (1)
            {
            unsigned char c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, false, data, eof);
            if (eof) { source_buf = (const char *)source; return false; } // end of file reached without finding any token.
            if (commentOn)
                { // we are in a comment
                if ((c == '%') || (c == '\n')) { commentOn = false; continue; } // deactivate comment mode
                continue; // just ignore the char otherwise.
                }
            // not in a comment
            if (c == '%') { commentOn = true; continue; } // activate comment mode
            if ((c > 32) && (c < 127))
                { // found it !
                source_len = n-1;
                source_buf = (const char *)source;
                return true;
                }
            }
        }


    /**
     * Reads a token. Throws an exception if the destination is not long enough to fit the entire
     * decoded token or if the token is ill-formed.
     *
     * @tparam  inputFun    The function that is called when running out of source input data. This
     *                      method should return a pointer to the next input character and set the size_t
     *                      parameter passed by reference to the number of char available at the returned
     *                      pointer. It should return nullptr when there is no more input data. The
     *                      default function readNoMore() simply return nullptr so it can be used when
     *                      the entire input data fit in the source buffer.
     * @param [in,out]  dest_buf    destination buffer.
     * @param   dest_len            length of the destination buffer.
     * @param   source_buf          Source buffer.
     * @param [in,out]  source_len  Length of the source buffer (more input data will be queried via
     *                              inputFun).
     * @param [in,out]  data        (Optional) If non-null, the opaque data to pass to the inputFun
     *                              function for internal use.
     *
     * @return  Number of bytes written at the destination. when the method return,
     *          source_buf[source_len] point to the first char past the decoded token or source_buf
	 *          is set to nullptr if end of file was reached.
    **/
    template< preadMoreToken inputFun = internals_stringfct::readNoMore > size_t readToken(void * dest_buf, size_t dest_len, const char * & source_buf, size_t & source_len, void * data = nullptr)
        {
        const unsigned char * source = (const unsigned char *)source_buf;
        if ((source==nullptr)||(source_len == 0)) // if the buffer source buffer is empty, we try to get something in it.
			{
            source = (const unsigned char *)inputFun(source_len, data);
            if (source == nullptr) { source_len = 0; return 0; } // nothing to read... hence nothing to do...
			MTOOLS_ASSERT(source_len > 0); // there must be at least one byte to read in source[].
			}
        unsigned char * dest = (unsigned char *)dest_buf; MTOOLS_ASSERT(dest != nullptr);
        bool eof = false;
        bool usequote = (source[0] == '\"');
        size_t n = (usequote ? 1 : 0);
        size_t m = 0;
        unsigned char c;
        while (1)
            {
            c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof); // read the next char
        readToken_goto:
            if (eof) { source_buf = nullptr; source_len = 0; return m; } // reached end of input, we are done...
            if ((c <= 32) || (c>126))
                {
                if (!usequote) { source_buf = (const char *)source; source_len = n-1; return m; } // found the end token when not using quote, source_len poitn to this char
                internals_stringfct::_readToken_put(dest, dest_len, m, c); continue; // otherwise, we consider it normal char...
                }
            if (c == '\"')
                {
                if (usequote) { source_buf = (const char *)source; source_len = n; return m; } // found the end token when using quote, source_len point to the next char.
                internals_stringfct::_readToken_put(dest, dest_len, m, c); continue; // otherwise, we consider it normal char...
                }
            if (c == '\\')
                {
                // special escape sequence
                c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                if (eof) MTOOLS_THROW("parse error");
                if (c == 'a') { internals_stringfct::_readToken_put(dest, dest_len, m, '\a'); continue; } // predefined escape sequences
                if (c == 'b') { internals_stringfct::_readToken_put(dest, dest_len, m, '\b'); continue; } //
                if (c == 'f') { internals_stringfct::_readToken_put(dest, dest_len, m, '\f'); continue; } //
                if (c == 'n') { internals_stringfct::_readToken_put(dest, dest_len, m, '\n'); continue; } //
                if (c == 'r') { internals_stringfct::_readToken_put(dest, dest_len, m, '\r'); continue; } //
                if (c == 't') { internals_stringfct::_readToken_put(dest, dest_len, m, '\t'); continue; } //
                if (c == 'v') { internals_stringfct::_readToken_put(dest, dest_len, m, '\v'); continue; } //
                if (c == '\?') { internals_stringfct::_readToken_put(dest, dest_len, m, '\?'); continue; }//
                if (c == '\\') { internals_stringfct::_readToken_put(dest, dest_len, m, '\\'); continue; }//
                if (c == '\'') { internals_stringfct::_readToken_put(dest, dest_len, m, '\''); continue; }//
                if (c == '\"') { internals_stringfct::_readToken_put(dest, dest_len, m, '\"'); continue; }//
                if (c == 'x')
                    { // hexadecimal sequence
                    while (1)
                        {
                        c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                        unsigned char d = internals_stringfct::_readToken_hexValue(c);
                        if (d > 15) goto readToken_goto; // done for the hexadecimal sequence
                        c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                        unsigned char e = internals_stringfct::_readToken_hexValue(c);
                        if (e > 15) { internals_stringfct::_readToken_put(dest, dest_len, m, d); goto readToken_goto; } // done for the hexadecimal sequence
                        d *= 16; d += e;
                        internals_stringfct::_readToken_put(dest, dest_len, m, d);
                        }
                    }
                if (c == 'u')
                    { // 16bit unicode sequence
                    char16_t code, w;
                    code = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (code > 15)  MTOOLS_THROW("parse error");
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    // TODO: fix this
                    //   std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
                    //   std::string res = convert.to_bytes(std::u16string(1, code));
                    //   for (size_t i = 0;i < res.length(); i++) { if (m >= dest_len) { MTOOLS_THROW("buffer too small"); } dest[m] = res[i]; m++;}
                    continue;
                    }
                if (c == 'U')
                    { // 32 bit unicode sequence
                    char16_t code, w;
                    code = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (code > 15)  MTOOLS_THROW("parse error");
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    // TODO: fix this
                    // std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
                    //   std::string res = convert.to_bytes(std::u32string(1, code));
                    //   for (size_t i = 0;i < res.length(); i++) { if (m >= dest_len) { MTOOLS_THROW("buffer too small"); } dest[m] = res[i]; m++; }
                    continue;
                    }
                if ((c >= '0') && (c <= '7'))
                    { // octal sequence
                    unsigned char v = c - '0';
                    c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                    if ((eof) || (c > '7') || (c < '0')) { internals_stringfct::_readToken_put(dest, dest_len, m, v); goto readToken_goto; }
                    v *= 8; v += (c - '0');
                    c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                    if ((eof) || (c > '7') || (c < '0')) { internals_stringfct::_readToken_put(dest, dest_len, m, v); goto readToken_goto; }
                    v *= 8; v += (c - '0');
                    internals_stringfct::_readToken_put(dest, dest_len, m, v);
                    continue;
                    }
                MTOOLS_THROW("parse error"); // wrong escaped sequence
                }
            // normal char
            internals_stringfct::_readToken_put(dest, dest_len, m, c);
            }
        }


    /**
     * Reads a token. Throws an exception if the token is ill formed.
     *
     * @tparam  putFun      Type of the put fun.
     * @tparam  inputFun    The function that is called when running out of source input data. This
     *                      method should return a pointer to the next input character and set the size_t
     *                      parameter passed by reference to the number of char available at the returned
     *                      pointer. It should return nullptr when there is no more input data. The
     *                      default function readNoMore() simply return nullptr so it can be used when
     *                      the entire input data fit in the source buffer.
     * @param [in,out]  dest        Destination string. The decoded buffer is appended to the current
     *                              content of the string if any.
     * @param   source_buf          Source buffer.
     * @param [in,out]  source_len  Length of the source buffer (more input data will be queried via
     *                              inputFun).
     * @param [in,out]  data        (Optional) If non-null, the opaque data to pass to the inputFun
     *                              function for internal use.
     *
     * @return  Number of bytes written at the destination. when the method return,
     *          source_buf[source_len] point to the first char past the decoded token or source_buf
     *          is set to nullptr if end of file was reached.
     **/
    template< preadMoreToken inputFun = internals_stringfct::readNoMore > size_t readToken(std::string & dest, const char * & source_buf, size_t & source_len, void * data = nullptr)
        {
        const unsigned char * source = (const unsigned char *)source_buf;
        if ((source==nullptr)||(source_len == 0)) // if the buffer source buffer is empty, we try to get something in it.
			{
            source = (const unsigned char *)inputFun(source_len, data);
            if (source == nullptr) { source_len = 0; return 0; } // nothing to read... hence nothing to do...
			MTOOLS_ASSERT(source_len > 0); // there must be at least one byte to read in source[].
			}
        bool eof = false;
        bool usequote = (source[0] == '\"');
        size_t n = (usequote ? 1 : 0);
        size_t m = 0;
        unsigned char c;
        while (1)
            {
            c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof); // read the next char
        readToken_goto:
            if (eof) { source_buf = nullptr; source_len = 0; return m; } // reached end of input, we are done...
            if ((c <= 32) || (c>126))
                {
                if (!usequote) { source_buf = (const char *)source; source_len = n - 1; return m; } // found the end token when not using quote
                internals_stringfct::_readToken_put(dest, m, c); continue; // otherwise, we consider it normal char...
                }
            if (c == '\"')
                {
                if (usequote) { source_buf = (const char *)source; source_len = n; return m; } // found the end token when using quote
                internals_stringfct::_readToken_put(dest, m, c); continue; // otherwise, we consider it normal char...
                }
            if (c == '\\')
                {
                // special escape sequence
                c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                if (eof) MTOOLS_THROW("parse error");
                if (c == 'a') { internals_stringfct::_readToken_put(dest, m, '\a'); continue; } // predefined escape sequences
                if (c == 'b') { internals_stringfct::_readToken_put(dest, m, '\b'); continue; } //
                if (c == 'f') { internals_stringfct::_readToken_put(dest, m, '\f'); continue; } //
                if (c == 'n') { internals_stringfct::_readToken_put(dest, m, '\n'); continue; } //
                if (c == 'r') { internals_stringfct::_readToken_put(dest, m, '\r'); continue; } //
                if (c == 't') { internals_stringfct::_readToken_put(dest, m, '\t'); continue; } //
                if (c == 'v') { internals_stringfct::_readToken_put(dest, m, '\v'); continue; } //
                if (c == '\?') { internals_stringfct::_readToken_put(dest, m, '\?'); continue; }//
                if (c == '\\') { internals_stringfct::_readToken_put(dest, m, '\\'); continue; }//
                if (c == '\'') { internals_stringfct::_readToken_put(dest, m, '\''); continue; }//
                if (c == '\"') { internals_stringfct::_readToken_put(dest, m, '\"'); continue; }//
                if (c == 'x')
                    { // hexadecimal sequence
                    while (1)
                        {
                        c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                        unsigned char d = internals_stringfct::_readToken_hexValue(c);
                        if (d > 15) goto readToken_goto; // done for the hexadecimal sequence
                        c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                        unsigned char e = internals_stringfct::_readToken_hexValue(c);
                        if (e > 15) { internals_stringfct::_readToken_put(dest, m, d); goto readToken_goto; } // done for the hexadecimal sequence
                        d *= 16; d += e;
                        internals_stringfct::_readToken_put(dest, m, d);
                        }
                    }
                if (c == 'u')
                    { // 16bit unicode sequence
                    char16_t code, w;
                    code = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (code > 15)  MTOOLS_THROW("parse error");
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    // TODO: fix this
                    //   std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
                    //   std::string res = convert.to_bytes(std::u16string(1, code));
                    //   for (size_t i = 0;i < res.length(); i++) { if (m >= dest_len) { MTOOLS_THROW("buffer too small"); } dest[m] = res[i]; m++;}
                    continue;
                    }
                if (c == 'U')
                    { // 32 bit unicode sequence
                    char16_t code, w;
                    code = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (code > 15)  MTOOLS_THROW("parse error");
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    w = (char16_t)internals_stringfct::_readToken_hexValue(internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof)); if (w > 15)  MTOOLS_THROW("parse error"); code *= 16; code += w;
                    // TODO: fix this
                    // std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
                    //   std::string res = convert.to_bytes(std::u32string(1, code));
                    //   for (size_t i = 0;i < res.length(); i++) { if (m >= dest_len) { MTOOLS_THROW("buffer too small"); } dest[m] = res[i]; m++; }
                    continue;
                    }
                if ((c >= '0') && (c <= '7'))
                    { // octal sequence
                    unsigned char v = c - '0';
                    c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                    if ((eof) || (c > '7') || (c < '0')) { internals_stringfct::_readToken_put(dest, m, v); goto readToken_goto; }
                    v *= 8; v += (c - '0');
                    c = internals_stringfct::_readToken_get<inputFun>(source, source_len, n, usequote, data, eof);
                    if ((eof) || (c > '7') || (c < '0')) { internals_stringfct::_readToken_put(dest, m, v); goto readToken_goto; }
                    v *= 8; v += (c - '0');
                    internals_stringfct::_readToken_put(dest, m, v);
                    continue;
                    }
                MTOOLS_THROW("parse error"); // wrong escaped sequence
                }
            // normal char
            internals_stringfct::_readToken_put(dest, m, c);
            }
        }


        /**
        * Creates a token from an integer (uint64 version)
        *
        * @param   n               The number.
        * @param [in,out]  dest    The destination string. the token is appended to the current cotnent if any.
        *
        * @return  The size of the token appended in bytes.
        **/
        size_t createTokenI(uint64 n, std::string & dest);


        /**
        * Creates a token from an integer (int64 version)
        *
        * @param   n               The number.
        * @param [in,out]  dest    The destination string. the token is appended to the current cotnent if any.
        *
        * @return  The size of the token appended in bytes.
        **/
        size_t createTokenI(int64 n, std::string & dest);


        /**
        * Reads a token containing an integer value (uint64 version)
        *
        * @param   str         The string with the token
        * @param [in,out]  v   The varible to store the integer into.
        *
        * @return  The number of byte read in the string to parse the value.
        **/
        size_t readTokenI(const std::string & str, uint64 & v);


        /**
        * Reads a token containing an integer value (int64 version)
        *
        * @param   str         The string with the token
        * @param [in,out]  v   The varible to store the integer into.
        *
        * @return  The number of byte read in the string to parse the value.
        **/
        size_t readTokenI(const std::string & str, int64 & v);


        /**
        * Creates a token from a floating point number (float version).
        *
        * @param   v               The floating point to process.
        * @param [in,out]  dest    destination string. If not empty, the token is appeded to the current
        *                          string content.
        *
        * @return  the size of the token appended to the string.
        **/
        size_t createTokenFP(float v, std::string & dest);


        /**
        * Creates a token from a floating point number (double version).
        *
        * @param   v               The floating point to process.
        * @param [in,out]  dest    destination string. If not empty, the token is appeded to the current
        *                          string content.
        *
        * @return  the size of the token appended to the string.
        **/
        size_t createTokenFP(double v, std::string & dest);


        /**
        * Creates a token from a floating point number (long double version).
        *
        * @param   v               The floating point to process.
        * @param [in,out]  dest    destination string. If not empty, the token is appeded to the current
        *                          string content.
        *
        * @return  the size of the token appended to the string.
        **/
        size_t createTokenFP(long double v, std::string & dest);


        /**
        * Reads a token containing a floating point (float version).
        *
        * @param   str         The string containing the value of the floating point.
        * @param [in,out]  v   The variable to store the value into.
        *
        * @return  Number of byte read i.e. point at the first char which was not used for the
        *          conversion.
        **/
        size_t readTokenFP(const std::string & str, float & v);


        /**
        * Reads a token containing a floating point (double version).
        *
        * @param   str         The string containing the value of the floating point.
        * @param [in,out]  v   The variable to store the value into.
        *
        * @return  Number of byte read i.e. point at the first char which was not used for the
        *          conversion.
        **/
        size_t readTokenFP(const std::string & str, double & v);


        /**
        * Reads a token containing a floating point (long double version).
        *
        * @param   str         The string containing the value of the floating point.
        * @param [in,out]  v   The variable to store the value into.
        *
        * @return  Number of byte read i.e. point at the first char which was not used for the
        *          conversion.
        **/
        size_t readTokenFP(const std::string & str, long double & v);



    /**
     * Query whether as std::string is a Valid utf-8 string.
     *
     * @param   s   The string to check.
     *
     * @return  true if the string is a valid UTF 8 string, false if not.
    **/
    bool isValidUtf8(const std::string & s);




        /**
     * Print a given number of bytes in nice form (ex: 10523 yields "10KB")
     *
     * @param   nb  The number of bytes.
     *
     * @return  A string representing this memory size
     **/
    inline std::string toStringMemSize(size_t nb)
        {
        int res;
        std::string unit;
        if (nb < 1024) { unit = "B"; nb *= 1024; } else {
        if (nb < 1024*1024) { unit = "KB"; } else { nb /= 1024;
        if (nb < 1024*1024) { unit = "MB"; } else { nb /= 1024;
        if (nb < 1024*1024) { unit = "GB"; } else { nb /= 1024;
        if (nb < 1024*1024) { unit = "TB"; } else { nb /= 1024;
        if (nb < 1024*1024) { unit = "PB"; } else { nb /= 1024; }}}}}}
        res = ((nb % 1024) * 100) / 1024;
        return toString(nb / 1024) + ((res == 0) ? std::string("") : (std::string(".") + toString(res))) + unit;
        }


    /**
     * Convert an object into an std::string.
     *
     * - Type `bool` is printed as 'true" or "false"
     * - Integer types are printed in decimal format.
     * - Floating point types are printed in default pseudo-scientific format.
     * - `std::wstring` objects are converted using the specified encoding.
     * - Types `char` and `wchar_t` are printed as characters (but `signed/unsigned char/wchar_t` are
     * printed as integers).
     * - Types `char *` and `wchar_t *` are printed as null terminated C (wide)strings (beware of
     * buffer overflow).
     * - Other pointer types `T *` have their adress printed in hexadecimal format.
     * - Type `std::pair<U,V>` print both elements of the pair.
     * - Fixed size C-arrays `T[N]`, ProxyArray<T> and STL containers are printed by iterating over
     * the elements and using `toString()` to print their content.
     * - If `T` does not match in of the cases above, then the following conversion order is used:
     *     - (1) Check if `T` implements the method `std::string T.toString() [const]`.
     *     - (2) Check if `T` implements the method `std::string T.to_string() [const]`.
     *     - (3) Check if `T` is convertible into an `std::string`.
     *     - (4) Check if `T` may be pushed to `std::ostream` via `operator<<`.
     *     - (5) Fall back to printing generic informations about the object (its type, size and
     *     adress).
     *
     * @tparam  T   Generic type parameter.
     * @param   val         The object to print into an `std::string`.
     * @param   output_enc  The desired encoding format for the output string. Relevant only if `T`
     *                      contain wide characters (i.e. `wstring`, `wchar_t *` or `wchar_t`).
     *                      Characters which cannot be translated are replaced by ' '. (default =
     *                      unknown_enc = ISO8859-1).
     *
     * @return  The string representing the object.
     *
     * @sa  ToWString
     **/
    template<typename T> inline std::string toString(const T & val, StringEncoding output_enc) { return internals_stringfct::StringConverter<T>::print(val, output_enc); }


    /**
     * Convert an object into a wstring. See `toString() for details`
     *
     * @tparam  T           Generic type parameter.
     * @param   val         The object to convert into a std::wstring.
     * @param   input_enc   The input encoding format. Relevant only if T is char, char* or std::string.
     *                      default = enc_unknown is which case the function try to guess the encoding
     *                      format using the isValidUtf8 method.
     *
     * @return  The wstring representing the object.
     *
     * @sa  ToString, isValidUtf8
     **/
    template<class T> inline std::wstring toWString(const T & val, StringEncoding input_enc = enc_unknown) { return toWString<std::string>(toString<T>(val, input_enc), input_enc); }


    template<> inline std::wstring toWString<std::string>(const std::string & s, StringEncoding input_enc)
    {
        if (input_enc == enc_unknown) { input_enc = (isValidUtf8(s) ? enc_utf8 : enc_iso8859); }
        if (input_enc == enc_utf8)
            {
            #if defined (COMPILER_HAS_CODECVT)
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return(converter.from_bytes(s));
            #else
            //TODO, for the time being, we just convert as if in iso 8859-1 ....
            #endif
            }
        std::wstring ws(s.length(), L' ');
        for (size_t i = 0; i < s.length(); i++) { unsigned char c = static_cast<unsigned char>(s[i]);  ws[i] = static_cast<wchar_t>(c); }
        return ws;
    }

    template<> inline std::wstring toWString<std::wstring>(const std::wstring & s, StringEncoding input_enc) { return s; }
    template<> inline std::wstring toWString<char>(const char & c, StringEncoding input_enc) { return toWString<std::string>(std::string(1, c), input_enc); }
    template<> inline std::wstring toWString<wchar_t>(const wchar_t & c, StringEncoding input_enc) { return std::wstring(1, c); }
    template<> inline std::wstring toWString<cp_char>(const cp_char & val, StringEncoding input_enc) { return toWString<std::string>(std::string(val), input_enc); }
    template<> inline std::wstring toWString<p_char>(const p_char & val, StringEncoding input_enc) { return toWString<std::string>(std::string(val), input_enc); }
    template<> inline std::wstring toWString<cp_wchar_t>(const cp_wchar_t & val, StringEncoding input_enc) { return std::wstring(val); }
    template<> inline std::wstring toWString<p_wchar_t>(const p_wchar_t & val, StringEncoding input_enc) { return std::wstring(val); }



    /**
     * Conversion of a double into a string with nice printing. Decrease the number of digits
     * written in order to make it more readable.
     *
     * @param   val The value to convert.
     *
     * @return  A the string representing the value.
     *
     * @sa  doubleToWStringNice
     **/
    std::string doubleToStringNice(double val);


    /**
     * Conversion double to a std::string with a given precison (number of digits written) and
     * chosing if we use scientific notation. 
     * 
     * @param   val         The value to convert
     * @param   precision   Number of digits (default = 15)
     * @param   scientific  true to use scientific notation
     *
     * @return  A std::string representing the value
     **/
    std::string doubleToStringHighPrecision(double val, int precision = 15, bool scientific = true);


    /**
     * Conversion of a double into a wstring with nice printing. Decrease the number of digits
     * written in order to make it more readable.
     *
     * @param   val The value to convert.
     *
     * @return  A the wstring representing the value.
     *
     * @sa  doubleToStringNice
     **/
    std::wstring doubleToWStringNice(double val);


    /**
    * Conversion double to a std::wstring with a given precison (number of digits written) and
    * chosing if we use scientific notation.
    *
    * @param   val         The value to convert
    * @param   precision   Number of digits (default = 15)
    * @param   scientific  true to use scientific notation
    *
    * @return  A std::wstring representing the value
    **/
    std::wstring doubleToWStringHighPrecision(double val, int precision = 15, bool scientific = true);


    /**
     * Convert a std::string in UTF8 encoding to a string in ISO8859-1 encoding.
     *
     * @param   s   the string in UTF8 encoding.
     *
     * @return  the string encoded in ISO8859-1.
     *
     * @sa  toUtf8, ToIso8859-1, iso8859ToUtf8
     **/
    std::string utf8ToIso8859(const std::string & s);


    /**
     * Convert a std::string to ISO8859-1 format. Try to guess the encoding format using isValidUtf8.
     *
     * @param   s   the string to convert.
     *
     * @return  the string encoded in ISO8859-1.
     *
     * @sa  isValidUtf8, toUtf8, utf8ToIso8859, iso8859ToUtf8
     **/
    std::string toIso8859(const std::string & s);


    /**
     * Convert a std::string in ISO8859-1 encoding to a string in UTF-8 encoding.
     *
     * @param   s   the string in ISO8859-1 encoding.
     *
     * @return  the string encoded in UTF-8.
     *
     * @sa  toUtf8, toIso8859-1, utf8ToIso8859
     **/
    std::string iso8859ToUtf8(const std::string & s);


    /**
     * Convert a std::string to UTF-8 format. Try to guess the encoding format using isValidUtf8.
     *
     * @param   s   the string to convert.
     *
     * @return  the string encoded in UTF-8 format.
     *
     * @sa  isValidUtf8, toIso8859-1, utf8ToIso8859, iso8859ToUtf8
     **/
    std::string toUtf8(const std::string & s);


    /**
     * Conversion of a string into a given type T.
     *
     * @tparam  T   type to convert to (should be convertible with <<)
     * @param   s       The string to process.
     * @param [out] val The variable to store the converted value.
     *
     * @return  the number of chars read in the string during the conversion.
     *
     * @sa  toString
     **/
    template<class T> inline size_t fromString(const std::string & s, T & val)
        {
        std::istringstream iss(s + " ");
        iss >> val;
        std::streampos r = iss.tellg();
        if (r < 0) return 0;
        return(static_cast<size_t>(r));
        }


    template<> inline size_t fromString<std::string>(const std::string & s, std::string & val) { val = s; return s.length(); }
    template<> inline size_t fromString<std::wstring>(const std::string & s, std::wstring & val) { val = toWString(s); return s.length(); }
    template<> inline size_t fromString<wchar_t>(const std::string & s, wchar_t & val) { std::wstring ws = toWString(s); val = ((ws.length() > 0) ? s[0] : 0); return ((ws.length() > 0) ? 1 : 0); }
    template<> inline size_t fromString<char>(const std::string & s, char & val) { val = ((s.length() > 0) ? s[0] : 0); return ((s.length() > 0) ? 1 : 0); }


    /**
     *  fromString, wchar_t * specialization. Try to deduce the encoding via isValidUtf8
     *
     * @warning Do not check for buffer overflow.
     **/
    template<> inline size_t fromString<p_wchar_t>(const std::string & s, p_wchar_t & val) { std::wstring ws = toWString(s); std::memcpy(val, ws.c_str(), (ws.length() + 1)*sizeof(wchar_t)); return s.length(); }


    /**
     * fromString, char * specialization.
     *
     * @warning Do not check for buffer overflow.
     **/
    template<> inline size_t fromString<p_char>(const std::string & s, p_char & val) { std::memcpy(val, s.c_str(), s.length() + 1); return s.length(); }


    /**
     *  fromString, bool specialization.
     *
     *  return true if "oui", "vrai", "yes", "true" (cap ignorant) or a non zero integer.
     *  return false otherwise
     **/
    template<> inline size_t fromString<bool>(const std::string & s, bool & val)
        {
        val = false;
        std::string s2 = toLowerCase(s);
        if ((!s2.compare("true")) || (!s2.compare("oui")) || (!s2.compare("vrai")) || (!s2.compare("yes"))) { val = true; return s.length(); }
        uint64_t v;
        if (fromString(s2, v) == s2.length()) { if (v != 0) val = true; }
        return s.length();
        }


    /**
     * Conversion of a wstring into a given type.
     *
     * @tparam  T   Generic type parameter (must be convertible with >>).
     * @param   ws      The wstring to process.
     * @param [out] val The variable to store the converted value.
     *
     * @return  the number of chars read in the wstring for the conversion.
     *
     * @sa  toWString
     **/
    template<class T> inline size_t fromWString(const std::wstring & ws, T & val)
        {
        std::wistringstream iss(ws + L" ");
        iss >> val;
        std::streampos r = iss.tellg();
        if (r < 0) return 0;
        return(static_cast<size_t>(r));
        }


    /** fromWString, std::string specialization. Convert the wstring in a string (use default encoding : iso8859-1). */
    template<> inline size_t fromWString<std::string>(const std::wstring & ws, std::string & val) { val = toString(ws); return ws.length(); }


    /** fromWString, std::wstring specialization. Just copy the string in val */
    template<> inline size_t fromWString<std::wstring>(const std::wstring & ws, std::wstring & val) { val = ws; return ws.length(); }


    /** fromWString, std::wchar_t specialization. Simply copy the first char if the string is not empty or put 0 otherwise. */
    template<> inline size_t fromWString<wchar_t>(const std::wstring & ws, wchar_t & val) { val = ((ws.length() > 0) ? ws[0] : 0); return ((ws.length() > 0) ? 1 : 0); }


    /** fromWString, std::char specialization. Simply copy the first char (converted in ISO8859-1) if the string is not empty or put 0 otherwise. */
    template<> inline size_t fromWString<char>(const std::wstring & ws, char & val) { std::string s = toString(ws); val = ((s.length() > 0) ? s[0] : 0); return ((s.length() > 0) ? 1 : 0); }


    /**
     *  fromWString, wchar_t * specialization.
     *
     * @warning Do not check for buffer overflow.
     **/
    template<> inline size_t fromWString<p_wchar_t>(const std::wstring & ws, p_wchar_t & val) { std::memcpy(val, ws.c_str(), (ws.length() + 1)*sizeof(wchar_t)); return ws.length(); }


    /**
     * fromWString, char * specialization. Convert to ISO9958-1 encoding.
     *
     * @warning Do not check for buffer overflow.
     **/
    template<> inline size_t fromWString<p_char>(const std::wstring & ws, p_char & val) { std::string s = toString(ws); std::memcpy(val, s.c_str(), s.length() + 1); return ws.length(); }


    /**
     *  fromWString, bool specialization.
     *
     *  return true if "oui", "vrai", "yes", "true" (cap ignorant) or a non zero integer.
     *  return false otherwise
     **/
    template<> inline size_t fromWString<bool>(const std::wstring & ws, bool & val) { return fromString<bool>(toString(ws), val); }


}


/* end of file */

