/** @file stringfct.cpp */
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


#include "stdafx_mtools.h"

#include "misc/stringfct.hpp"


namespace mtools
{


    size_t replace(std::string & buffer, const std::string & oldstr, const std::string & newstr)
        {
        if ((!buffer.length()) || (!oldstr.length())) { return 0; }
        size_t pos = 0, r = 0;
        while (1)
            {
            pos = buffer.find(oldstr, pos);
            if (pos == std::string::npos) { return r; }
            buffer.replace(pos, oldstr.length(), newstr);
            r++; pos += newstr.length();
            }
        }


    std::string memoryToString(const void * p, size_t len)
    {
        if ((p == NULL) || (len == 0)) { return std::string(""); }
        const unsigned char * pc = static_cast<const uint8_t *>(p);
        std::string r(2 * len, ' ');
        char nb[] = "0123456789ABCDEF";
        auto it = r.begin();
        for (size_t i = 0; i < len; i++) { (*it) = nb[pc[i] / 16]; ++it;  (*it) = nb[pc[i] % 16]; ++it; }
        return r;
    }


    size_t stringToMemory(const std::string & s, void * p)
    {
        if ((!p) || (s.length() < 2)) { return 0; }
        uint8_t * pc = static_cast<uint8_t *>(p);
        size_t n = 0;
        auto it = s.begin();
        while (1)
        {
            uint8_t v;
            if (it == s.end()) { return n; }
            if ((((*it) >= '0') && ((*it) <= '9'))) { v = 16 * static_cast<uint8_t>((*it) - '0'); }
            else if ((((*it) >= 'A') && ((*it) <= 'F'))) { v = 16 * static_cast<uint8_t>((*it) - 'A' + 10); }
            else { return n; } ++it;
            if (it == s.end()) { return n; }
            if ((((*it) >= '0') && ((*it) <= '9'))) { v += static_cast<uint8_t>((*it) - '0'); }
            else if ((((*it) >= 'A') && ((*it) <= 'F'))) { v += static_cast<uint8_t>((*it) - 'A' + 10); }
            else { return n; } ++it;
            (*pc) = v;
            ++pc; ++n;
        }
    }


    size_t createToken(std::string & dest, const void * source_buf, size_t source_len, bool opaqueHex, bool surroundWithQuotes)
    {
        const unsigned char * source = (const unsigned char *)source_buf;
        MTOOLS_ASSERT(source != nullptr);
        size_t startlength = dest.length();
        dest.reserve(startlength + 2 * (source_len + 1));
        if (surroundWithQuotes) { dest.push_back('\"'); }
        if (opaqueHex)
        {
            dest.push_back('\\'); dest.push_back('x');
            for (size_t i = 0; i < source_len; i++)
            {
                unsigned char c = source[i];
                auto d = (c / 16); dest.push_back((d < 10) ? ('0' + d) : (d - 10 + 'A'));
                auto e = (c % 16); dest.push_back((e < 10) ? ('0' + e) : (e - 10 + 'A'));
            }
            if (surroundWithQuotes) { dest.push_back('\"'); }
            return(dest.length() - startlength);
        }
        size_t n = 0;
        while (n < source_len)
        {
            unsigned char c = source[n];  n++;
            if (c == '\\') { dest.push_back('\\'); dest.push_back('\\'); continue; }//
            if (c == '\'') { dest.push_back('\\'); dest.push_back('\''); continue; }//
            if (c == '\"') { dest.push_back('\\'); dest.push_back('\"'); continue; }//
            if (((c > 32) && (c < 127)) || ((c == 32) && (surroundWithQuotes))) { dest.push_back(c); continue; } // normal char, just push it
            if (c == '\a') { dest.push_back('\\'); dest.push_back('a'); continue; } // predefined escape sequences
            if (c == '\b') { dest.push_back('\\'); dest.push_back('b'); continue; } //
            if (c == '\f') { dest.push_back('\\'); dest.push_back('f'); continue; } //
            if (c == '\n') { dest.push_back('\\'); dest.push_back('n'); continue; } //
            if (c == '\r') { dest.push_back('\\'); dest.push_back('r'); continue; } //
            if (c == '\t') { dest.push_back('\\'); dest.push_back('t'); continue; } //
            if (c == '\v') { dest.push_back('\\'); dest.push_back('v'); continue; } //
                                                                                    // special char, we write it in the form "\XXX"
            if ((source[n] > '7') || (source[n] < '0') || (n == source_len))
            { // ok, we can try to print a short version
                if (c < 8) { dest.push_back('\\'); dest.push_back(c + '0'); continue; }
                if (c < 64) { dest.push_back('\\'); dest.push_back((c / 8) + '0'); dest.push_back((c % 8) + '0'); continue; }
            }
            // we must print all 3 digits
            dest.push_back('\\'); dest.push_back((c / 64) + '0');  c = c % 64; dest.push_back((c / 8) + '0'); dest.push_back((c % 8) + '0');
        }
        // done
        if (surroundWithQuotes) { dest.push_back('\"'); }
        return(dest.length() - startlength);
    }


    bool doesTokenNeedQuotes(const void * source, const size_t source_len)
        {
        MTOOLS_ASSERT(source != nullptr);
        if (source_len == 0) return true;
        const unsigned char * p = (const unsigned char *)source;
        for (size_t i = 0;i < source_len; i++) { auto c = p[i]; if ((c == ' ')||(c == '%')) return true; }
        return false;
        }


    size_t createTokenI(uint64 n, std::string & dest)
    {
        size_t l = 0;
        { const uint64 d = 10000000000000000000u; const uint64 r = (n / d); n = (n%d); if (r != 0) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 1000000000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 100000000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 10000000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 1000000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 100000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 10000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 1000000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 100000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 10000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 1000000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 100000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 10000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 1000000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 100000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 10000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 1000u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 100u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { const uint64 d = 10u; const uint64 r = (n / d); n = (n%d); if ((l != 0) || (r != 0)) { dest.push_back('0' + (char)r); ++l; } }
        { dest.push_back('0' + (char)n); ++l; }
        return l;
    }


    size_t createTokenI(int64 n, std::string & dest)
        {
        if (n < 0) { dest.push_back('-'); return(createTokenI((uint64)(-n), dest) + 1); }
        return(createTokenI((uint64)(n), dest));
        }


    size_t readTokenI(const std::string & str, uint64 & v)
        {
        bool neg = false;
        v = 0;
        size_t n = 0;
        if ((str.length() > 0) && (str[0] == '-')) { n++; neg = true; }
        while (n < str.length())
        {
            unsigned char c = (unsigned char)(str[n]);
            if ((c < '0') || (c > '9')) { if (neg) { v = (uint64)(-(int64)v); } return n; }
            v *= 10; v += (c - '0');
            ++n;
        }
        if (neg) { v = (uint64)(-(int64)v); }
        return n;
        }


    size_t readTokenI(const std::string & str, int64 & v)
        {
        bool neg = false;
        v = 0;
        size_t n = 0;
        if ((str.length() > 0) && (str[0] == '-')) { n++; neg = true; }
        while (n < str.length())
        {
            unsigned char c = (unsigned char)(str[n]);
            if ((c < '0') || (c > '9')) { if (neg) { v = -v; } return n; }
            v *= 10; v += (c - '0');
            ++n;
        }
        if (neg) { v = -v; }
        return n;
        }


    size_t createTokenFP(float v, std::string & dest)
        {
        switch (std::fpclassify(v))
            {
            case FP_INFINITE: { if (v > 0.0) { dest += "INF"; return 3; } dest += "-INF"; return 4; }
            case FP_NAN: { dest += "NAN"; return 3; }
            case FP_SUBNORMAL: { if (v > 0.0) { dest += "0"; return 1; } dest += "-0";  return 2; }
            case FP_ZERO: { float positivezero = 0.0; if (memcmp(&v, &positivezero, sizeof(v)) == 0) { dest += "0"; return 1; } dest += "-0";  return 2; }
            case FP_NORMAL:
                {
                int64 n = (int64)floor(v);
                float v2 = (float)n;
                if (v == v2) { std::string s = std::to_string(n); dest += s; return s.length(); }
                std::ostringstream os;
                os << std::scientific;
                os.precision(std::numeric_limits<float>::max_digits10);
                os << v;
                auto s = os.str();
                dest += s;
                return s.length();
                }
            }
        MTOOLS_ERROR("wtf ?");
        return 0;
        }


    size_t createTokenFP(double v, std::string & dest)
        {
        switch (std::fpclassify(v))
            {
            case FP_INFINITE: { if (v > 0.0) { dest += "INF"; return 3; } dest += "-INF"; return 4; }
            case FP_NAN: { dest += "NAN"; return 3; }
            case FP_SUBNORMAL: { if (v > 0.0) { dest += "0"; return 1; } dest += "-0";  return 2; }
            case FP_ZERO: { double positivezero = 0.0; if (memcmp(&v, &positivezero, sizeof(v)) == 0) { dest += "0"; return 1; } dest += "-0";  return 2; }
            case FP_NORMAL:
                {
                int64 n = (int64)floor(v);
                double v2 = (double)n;
                if (v == v2) { std::string s = std::to_string(n); dest += s; return s.length(); }
                std::ostringstream os;
                os << std::scientific;
                os.precision(std::numeric_limits<double>::max_digits10);
                os << v;
                auto s = os.str();
                dest += s;
                return s.length();
                }
            }
        MTOOLS_ERROR("wtf ?");
        return 0;
        }


    size_t createTokenFP(long double v, std::string & dest)
        {
        switch (std::fpclassify(v))
            {
            case FP_INFINITE: { if (v > 0.0) { dest += "INF"; return 3; } dest += "-INF"; return 4; }
            case FP_NAN: { dest += "NAN"; return 3; }
            case FP_SUBNORMAL: { if (v > 0.0) { dest += "0"; return 1; } dest += "-0";  return 2; }
            case FP_ZERO: { long double positivezero = 0.0; if (memcmp(&v, &positivezero, sizeof(v)) == 0) { dest += "0"; return 1; } dest += "-0";  return 2; }
            case FP_NORMAL:
                {
                int64 n = (int64)floor(v);
                long double v2 = (long double)n;
                if (v == v2) { std::string s = std::to_string(n); dest += s; return s.length(); }
                std::ostringstream os;
                os << std::scientific;
                os.precision(std::numeric_limits<long double>::max_digits10);
                os << v;
                auto s = os.str();
                dest += s;
                return s.length();
                }
            }
        MTOOLS_ERROR("wtf ?");
        return 0;
        }


    size_t readTokenFP(const std::string & str, float & v)
        {
        if ((str.length() >= 3) && (memcmp(str.c_str(), "INF", 3) == 0)) { v = std::numeric_limits<float>::infinity(); return 3; }
        if ((str.length() >= 4) && (memcmp(str.c_str(), "-INF", 4) == 0)) { v = -std::numeric_limits<float>::infinity(); return 4; }
        if ((str.length() >= 3) && (memcmp(str.c_str(), "NAN", 3) == 0)) { v = -std::numeric_limits<float>::quiet_NaN(); return 3; }
        size_t n;
        v = std::stof(str, &n);
        return n;
        }


    size_t readTokenFP(const std::string & str, double & v)
        {
        if ((str.length() >= 3) && (memcmp(str.c_str(), "INF", 3) == 0)) { v = std::numeric_limits<double>::infinity(); return 3; }
        if ((str.length() >= 4) && (memcmp(str.c_str(), "-INF", 4) == 0)) { v = -std::numeric_limits<double>::infinity(); return 4; }
        if ((str.length() >= 3) && (memcmp(str.c_str(), "NAN", 3) == 0)) { v = -std::numeric_limits<double>::quiet_NaN(); return 3; }
        size_t n;
        v = std::stod(str, &n);
        return n;
        }


    size_t readTokenFP(const std::string & str, long double & v)
        {
        if ((str.length() >= 3) && (memcmp(str.c_str(), "INF", 3) == 0)) { v = std::numeric_limits<long double>::infinity(); return 3; }
        if ((str.length() >= 4) && (memcmp(str.c_str(), "-INF", 4) == 0)) { v = -std::numeric_limits<long double>::infinity(); return 4; }
        if ((str.length() >= 3) && (memcmp(str.c_str(), "NAN", 3) == 0)) { v = std::numeric_limits<long double>::quiet_NaN(); return 3; }
        size_t n;
        v = std::stold(str, &n);
        return n;
        }


    bool isValidUtf8(const std::string & s)
    {
        auto it = s.begin();
        while (it != s.end())
        {
            int sizechar;
            unsigned char c = static_cast<unsigned char>(*it);
            if (c <= 0x7F) /*01111111 */                                       sizechar = 1;
            else if ((c >= 0xC0) /*11000000*/ && (c <= 0xDF) /*11011111*/)     sizechar = 2;
            else if ((c >= 0xE0) /*11100000*/ && (c <= 0xEF) /*11101111*/)     sizechar = 3;
            else if ((c >= 0xF0) /*11110000*/ && (c <= 0xF4) /*11110111*/)     sizechar = 4;
            else { return false; }
            ++it;
            while (sizechar > 1)
            {
                if (it == s.end()) return false;
                unsigned char c = static_cast<unsigned char>(*it);
                if ((c < 0x80) /*10000000*/ || (c > 0xBF /*10111111 */)) { return false; }
                --sizechar;
                ++it;
            }
        }
        return true;
    }


    std::string doubleToStringNice(double val)
        {
            std::stringstream stream;
            stream << std::setprecision(DBL_DIG - 3);
            if ((val > 9999999) || (val < -9999999) || ((val > -0.0001) && (val < 0.0001))) stream << std::scientific;
            stream << val;
            std::string s(stream.str());
            // here we remove trailing zeros
            size_t n1 = s.find_first_of("eE", 0);
            std::string ma = s.substr(0, n1);
            std::string ex = (n1 == std::string::npos) ? std::string("") : s.substr(n1, s.length() - n1);
            if (ma.length() == 0) return s;
            size_t pos = ma.length();
            while ((pos > 0) && (ma[pos - 1] == '0')) { --pos; }
            if (pos == 0) return std::string("0");
            if (ma.find_first_of(".", 0) == std::string::npos) { return s; }
            if (ma[pos - 1] == '.') { --pos; if (pos == 0) return std::string("0"); }
            return ma.substr(0, pos) + ex;
        }


    std::wstring doubleToWStringNice(double val) { return toWString(doubleToStringNice(val)); }


    std::string doubleToStringHighPrecision(double val, int precision, bool scientific)
        {
        std::stringstream ss;
        if (scientific) ss << std::scientific; else ss << std::fixed;
        ss << std::setprecision(precision) << val;
        return ss.str();
        }


    std::wstring doubleToWStringHighPrecision(double val, int precision, bool scientific)
        {
        std::wstringstream ss;
        if (scientific) ss << std::scientific; else ss << std::fixed;
        ss << std::setprecision(precision) << val;
        return ss.str();
        }


    std::string utf8ToIso8859(const std::string & s)
        {
        std::wstring ws = toWString(s, enc_utf8);
        return toString(ws, enc_iso8859);
        }


    std::string toIso8859(const std::string & s)
        {
        std::wstring ws = toWString(s);
        return toString(ws, enc_iso8859);
        }


    std::string iso8859ToUtf8(const std::string & s)
        {
        std::wstring ws = toWString(s, enc_iso8859);
        return toString(ws, enc_iso8859);
        }


    std::string toUtf8(const std::string & s)
        {
        std::wstring ws = toWString(s);
        return toString(ws, enc_utf8);
        }



}


/* end of file */

