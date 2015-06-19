/** @file internals_stringfct.hpp */
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


#include "metaprog.hpp"
#include "misc.hpp"

#if defined (_MSC_VER)
// header missing with libstdc++
#include <codecvt>
#define COMPILER_HAS_CODECVT
#endif



#include <locale>
#include <sstream>
#include <cfloat>
#include <algorithm>
#include <iomanip>
#include <utility>
#include <type_traits>
#include <wchar.h>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>


namespace mtools
{
    template<typename T> std::string toString(const T & val, StringEncoding output_enc = enc_unknown);

    namespace internals_stringfct
    {


        /* class for conversion to a string, specialization for basic types, partial specialization for arrays and containers */
        template<typename T> class StringConverter
        {
            public:
            static inline  std::string print(const T & val, StringEncoding enc) { return _print1(val, metaprog::dummy<metaprog::has_toString<T>::value >()); }

            private:
            static inline std::string _print1(const T & val, mtools::metaprog::dummy<true> D) { T * p = const_cast<T*>(&val); return(p->toString()); }
            static inline std::string _print1(const T & val, mtools::metaprog::dummy<false> D) { return _print2(val, metaprog::dummy<metaprog::has_to_string<T>::value >()); }
            static inline std::string _print2(const T & val, mtools::metaprog::dummy<true> D) { T * p = const_cast<T*>(&val); return(p->to_string()); }
            static inline std::string _print2(const T & val, mtools::metaprog::dummy<false> D) { return _print3(val, metaprog::dummy<std::is_convertible<T,std::string>::value >()); }
            static inline std::string _print3(const T & val, mtools::metaprog::dummy<true> D) { return ((std::string)(val)); }
            static inline std::string _print3(const T & val, mtools::metaprog::dummy<false> D) { return _print4(val, metaprog::dummy<metaprog::has_to_ostream<T>::value >()); }
            static inline std::string _print4(const T & val, mtools::metaprog::dummy<true> D) { T * p = const_cast<T*>(&val); std::ostringstream oss(std::ostringstream::out); oss << (*p); return(oss.str()); }
            static inline std::string _print4(const T & val, mtools::metaprog::dummy<false> D) { return std::string("[type: ") + typeid(T).name() + " size:" + std::to_string(sizeof(T)) + " adr:" + toString(&val) + "]"; }
        };

        template<> class StringConverter<std::string>
        {
            typedef std::string T;
            public:
            static inline std::string print(const T & val, StringEncoding enc) { return(val); }
        };

        template<> class StringConverter<std::wstring>
        {
            typedef std::wstring T;
            public:
            static inline std::string print(const T & ws, StringEncoding output_enc)
            {
                if (output_enc == enc_utf8)
                    {
                    #if defined (COMPILER_HAS_CODECVT)
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter; return(converter.to_bytes(ws));
                    #else
                    std::string s; s.reserve(ws.size()+1);
                    for (auto wit = ws.begin(); wit != ws.end(); wit++)
                        {
                        auto c = *wit;
                        if (c<0x80) {s.append(1,c);}
                        else if (c<0x800) { s.append(1,192+c/64), s.append(1,128+c%64); }
                        else if (c-0xd800u<0x800) {s.append(1,' ');}
                        else if (c<0x10000) {s.append(1,224+c/4096); s.append(1,128+c/64%64); s.append(1,128+c%64);}
                        else if (c<0x110000) {s.append(1,240+c/262144); s.append(1,128+c/4096%64); s.append(1,128+c/64%64); s.append(1,128+c%64);}
                        else {s.append(1,' ');}
                        }
                    return s;
                    #endif
                    }
                std::string s(ws.length(), ' ');
                auto it = s.begin();
                for (std::wstring::const_iterator wit = ws.begin(); wit != ws.end(); wit++)
                {
                    int v = static_cast<int>(*wit);
                    if (v < 256) { (*it) = static_cast<unsigned char>(v); }
                    it++;
                }
                return s;
            }
        };

        template<> class StringConverter<bool>
        {
            typedef bool T;
            public:
            static inline std::string print(const T & val, StringEncoding enc) { return(val ? std::string("true") : std::string("false")); }
        };

        template<> class StringConverter<signed char>
        {
            typedef signed char T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<short>
        {
            typedef short T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<int>
        {
            typedef int T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<long>
        {
            typedef long T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<long long>
        {
            typedef long long T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<unsigned char>
        {
            typedef unsigned char T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<unsigned short>
        {
            typedef unsigned short T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<unsigned int>
        {
            typedef unsigned int T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<unsigned long>
        {
            typedef unsigned long T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<unsigned long long>
        {
            typedef unsigned long long T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::to_string(val); }
        };

        template<> class StringConverter<float>
        {
            typedef float T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { std::stringstream stream; stream << val; return std::string(stream.str()); }
        };

        template<> class StringConverter<double>
        {
            typedef double T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { std::stringstream stream; stream << val; return std::string(stream.str()); }
        };

        template<> class StringConverter<long double>
        {
            typedef long double T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { std::stringstream stream; stream << val; return std::string(stream.str()); }
        };

        template<> class StringConverter<char>
        {
            typedef char T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::string(1, val); }
        };

        template<> class StringConverter<wchar_t>
        {
            typedef wchar_t T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return toString<std::wstring>(std::wstring(1, val), output_enc); }
        };


        template<> class StringConverter<char *>
        {
            typedef char * T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::string(val); }
        };

        template<> class StringConverter<const char *>
        {
            typedef const char * T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return std::string(val); }
        };

        template<> class StringConverter<wchar_t *>
        {
            typedef wchar_t * T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return toString<std::wstring>(std::wstring(val), output_enc); }
        };

        template<> class StringConverter<const wchar_t *>
        {
            typedef const wchar_t * T;
            public:
            static inline std::string print(const T & val, StringEncoding output_enc) { return toString<std::wstring>(std::wstring(val), output_enc); }
        };

        template<typename U> class StringConverter < U* >
        {
            typedef U * T;
            public:
            static inline std::string print(const T & val, StringEncoding enc) { std::stringstream stream; stream << val; return std::string(stream.str()); }
        };

        template<size_t N> class StringConverter < char[N] >
        {
            typedef char T[N];
            public:
            static inline std::string print(const T & val, StringEncoding enc) { return toString<const char*>((const char *)val, enc); }
        };

        template<size_t N> class StringConverter < const char[N] >
        {
            typedef const char T[N];
            public:
            static inline std::string print(const T & val, StringEncoding enc) { return toString<const char*>((const char *)val, enc); }
        };

        template<size_t N> class StringConverter < wchar_t[N] >
        {
            typedef wchar_t T[N];
            public:
            static inline std::string print(const T & val, StringEncoding enc) { return toString<const wchar_t *>((const wchar_t *)val, enc); }
        };

        template<size_t N> class StringConverter < const wchar_t[N] >
        {
            typedef const wchar_t T[N];
            public:
            static inline std::string print(const T & val, StringEncoding enc) { return toString<const wchar_t *>((const wchar_t *)val, enc); }
        };

        template<typename U, size_t N> class StringConverter < U[N] >
        {
            typedef U T[N];
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("Array T[N] with T= '" + std::string(typeid(U).name()) + "' and N = " + toString(N) + "\n");
                for (size_t i = 0; i < N; i++) { s += toString(i) + "\t -> " + toString(val[i], enc) + "\n"; }
                return s;
            }
        };

        template<typename U, typename V> class StringConverter < std::pair<U, V> >
        {
            typedef std::pair<U, V> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                return std::string("std::pair<") + typeid(U).name() + " , " + typeid(V).name() + "> = (" + toString(val.first) + " , " + toString(val.second) + ")";
            }
        };

        template<typename U, size_t N> class StringConverter < std::array<U, N> >
        {
            typedef std::array<U, N> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::array<" + std::string(typeid(U).name()) + ">. Size = " + toString(val.size()) + "\n");
                for (size_t i = 0; i < N; i++) { s += toString(i) + "\t -> " + toString(val[i], enc) + "\n"; }
                return s;
            }
        };

        template<typename U, typename Alloc> class StringConverter < std::vector<U, Alloc> >
        {
            typedef std::vector<U, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::vector<" + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                for (size_t i = 0; i < val.size(); i++) { s += toString(i) + "\t -> " + toString(val[i], enc) + "\n"; }
                return s;
            }
        };

        template<typename U, typename Alloc> class StringConverter < std::deque<U, Alloc> >
        {
            typedef std::deque<U, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::deque<" + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };

        template<typename U, typename Alloc> class StringConverter < std::forward_list<U, Alloc> >
        {
            typedef std::forward_list<U, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                size_t n = 0; for (auto it = val.begin(); it != val.end(); ++it) { ++n; }
                std::string s("std::forward_list<" + std::string(typeid(U).name()) + "> Size = " + toString(n) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };


        template<typename U, typename Alloc> class StringConverter < std::list<U, Alloc> >
        {
            typedef std::list<U, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::list<" + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };


        template<typename U, typename Comp, typename Alloc> class StringConverter < std::set<U, Comp, Alloc> >
        {
            typedef std::set<U, Comp, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::set<" + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };


        template<typename U, typename Comp, typename Alloc> class StringConverter < std::multiset<U, Comp, Alloc> >
        {
            typedef std::multiset<U, Comp, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::multiset<" + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };


        template<typename Key, typename U, typename Comp, typename Alloc> class StringConverter < std::map<Key, U, Comp, Alloc> >
        {
            typedef std::map<Key, U, Comp, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::map<" + std::string(typeid(Key).name()) + "," + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };


        template<typename Key, typename U, typename Comp, typename Alloc> class StringConverter < std::multimap<Key, U, Comp, Alloc> >
        {
            typedef std::multimap<Key, U, Comp, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::multimap<" + std::string(typeid(Key).name()) + "," + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };

        template<typename Key, typename Hash, typename Pred, typename Alloc> class StringConverter < std::unordered_set<Key, Hash, Pred, Alloc> >
        {
            typedef std::unordered_set<Key, Hash, Pred, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::unordered_set<" + std::string(typeid(Key).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };


        template<typename Key, typename Hash, typename Pred, typename Alloc> class StringConverter < std::unordered_multiset<Key, Hash, Pred, Alloc> >
        {
            typedef std::unordered_multiset<Key, Hash, Pred, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::unordered_multiset<" + std::string(typeid(Key).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString((*it), enc) + "\n"; ++i; }
                return s;
            }
        };

        template<typename Key, typename U, typename Hash, typename Pred, typename Alloc> class StringConverter < std::unordered_map<Key, U, Hash, Pred, Alloc> >
        {
            typedef std::unordered_map<Key, U, Hash, Pred, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::unordered_map<" + std::string(typeid(Key).name()) + "," + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString(it->first, enc) + " \t:\t " + toString(it->second, enc) + "\n"; ++i; }
                return s;
            }
        };

        template<typename Key, typename U, typename Hash, typename Pred, typename Alloc> class StringConverter < std::unordered_multimap<Key, U, Hash, Pred, Alloc> >
        {
            typedef std::unordered_multimap<Key, U, Hash, Pred, Alloc> T;
            public:
            static inline std::string print(const T & val, StringEncoding enc)
            {
                std::string s("std::unordered_multimap<" + std::string(typeid(Key).name()) + "," + std::string(typeid(U).name()) + "> Size = " + toString(val.size()) + "\n");
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s += toString(i) + "\t -> " + toString(it->first, enc) + " \t:\t " + toString(it->second, enc) + "\n"; ++i; }
                return s;
            }
        };


    }

}


/* end of file */
