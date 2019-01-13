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


	/**
	* Enum for different character encodings.
	 **/
	enum StringEncoding { enc_utf8, enc_iso8859, enc_unknown };


	/* forward declaration */ 

	std::string doubleToStringNice(double val);

	std::string doubleToStringHighPrecision(double val, int precision = 15, bool scientific = true);


	/* shortcut */
	class ostringstream;
	using OSS =  mtools::ostringstream;



	/** mtools version of an 'ostringstream' */
	class ostringstream
		{


		public:

		/** Default constructor with empty string */
		ostringstream(bool format_nice = true, StringEncoding enc = enc_unknown) : _str(), _fnice(format_nice), _enc(enc)
			{
			}


		/** Destructor */
		~ostringstream() = default; 

		ostringstream(const ostringstream & os) = default;					// default assingement/copy/move semantics
		ostringstream(ostringstream && os) = default;						//
		ostringstream & operator=(const ostringstream & os) = default;		//
		ostringstream & operator=(ostringstream && os) = default;			//


		/**
		 * Select encoding for unicode string
		 **/
		void encoding(StringEncoding enc) { _enc = enc; }


		/**
		* Return the selected encoding for unicode string
		**/
		StringEncoding encoding() const { return _enc; }


		/**
		 * Select whether we use 'nice formatting'. 
		 **/
		void format_nice(bool fnice) { _fnice = fnice; }


		/**
		* Return whether we use 'nice' formatting. 
		**/
		bool format_nice() const { return _fnice; }


		/** Clears the stream/string */
		void clear() { _str.clear(); }


		/** Return the number of char in the stream/string */
		size_t size() const { return _str.size(); }


		/**
		 * Return the string.
		 **/
		inline std::string str() const { return _str; }


		/**
		 * The toString() method and we go full circle :-)
		 **/
		inline std::string toString() const { return _str; }


		 /**
		* Stream insertion operator.
		* 
		* - implement conversion for all basic types.
		* - Floating point types printing mode depend on the 'format_nice' flag' of the stream.
		* - wstring objects are converted using the specified encoding.
		* - Types char and wchar_t are printed as characters but signed/unsigned char/wchar_t are printed as integers.
		* - Types char * and wchar_t * are printed as null terminated C (wide)strings (beware of buffer overflow).
		* - Other pointer types T * have their adress printed in hexadecimal format.
		* - Fixed size C-arrays T[N], ProxyArray<T> and STL containers are printed by iterating over the elements and recursively pushing them in the stream  
		* - 
		* - If T does not match in of the cases above, then the following conversion order is used:  
		*    - (1) Check if T implements the method std::string T.toString() [const].
		*    - (2) Check if T implements the method std::string T.to_string() [const].
		*    - (3) Check if T is convertible into an std::string.
		*    - (4) Check if T may be pushed to std::ostream via operator<<.
		*    - (5) Fall back to printing generic informations about the object (its type, size and adress).
		*
		* @param	obj	The object to push to the stream
		*
		* @return	The shifted result.
		**/
		template<typename T> ostringstream & operator<<(const T & obj);


		ostringstream & operator<<(std::string & str) { _str += str; return(*this); }

		ostringstream & operator<<(const std::string & str) { _str += str; return(*this); }

		ostringstream & operator<<(char c) { _str.append(1, c);  return (*this); }

		ostringstream & operator<<(const char * cstr) { if (cstr == nullptr) { _str.append("(const char *)nullptr"); } else { _str.append(cstr); } return (*this); }

		ostringstream & operator<<(char * cstr) { if (cstr == nullptr) { _str.append("(char *)nullptr"); } else { _str.append(cstr); } return (*this); }


		private:

			std::string		_str;			// the string 
			bool			_fnice;			// true to use nice formatting (especially for floating point values)
			StringEncoding	_enc;			// encoding to use for wstring
		};



	/**
	 * Push an mtools::ostringstream content into a regular std::ostream.
	 **/
	inline std::ostream & operator<<(std::ostream & oss, const mtools::ostringstream & os)
		{
		oss << os.str(); 
		return oss; 
		}






	namespace internals_ostringstream
	{


		/* converter class (used for partial specialization) */
		template<typename T> class ostringstreamConverter
		{

		public:

			static inline  void push(const T & val, mtools::ostringstream & os) { return _push1(val, os, metaprog::dummy<metaprog::has_toString<T>::value >()); }

		private:

			static inline void _push1(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<true> D)	{ T * p = const_cast<T*>(&val); os << (p->toString()); }

			static inline void _push1(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<false> D) { _push2(val, os, metaprog::dummy<metaprog::has_to_string<T>::value >()); }

			static inline void _push2(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<true> D)	{ T * p = const_cast<T*>(&val); os << (p->to_string()); }

			static inline void _push2(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<false> D) { _push3(val, os, metaprog::dummy<std::is_convertible<T, std::string>::value >()); }

			static inline void _push3(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<true> D)	{ os << ((std::string)(val)); }

			static inline void _push3(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<false> D) { _push4(val, os, metaprog::dummy<metaprog::has_to_ostream<T>::value >()); }

			static inline void _push4(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<true> D)	{ T * p = const_cast<T*>(&val); std::ostringstream oss(std::ostringstream::out); oss << (*p); os << oss.str(); }

			static inline void _push4(const T & val, mtools::ostringstream & os, mtools::metaprog::dummy<false> D) { os << "[type: " <<  typeid(T).name() << " size:" << sizeof(T) << " adr:" << (&val) << "]"; }

		};



        template<> class ostringstreamConverter<std::wstring>
			{
            typedef std::wstring T;
            public:
            static inline void push(const T & ws, mtools::ostringstream & os)
				{
                if (os.encoding() == enc_utf8)
                    {
                    #if defined (COMPILER_HAS_CODECVT)
					#if defined (_MSC_VER)
					#pragma warning (push)
					#pragma warning (disable:4996)
					#endif
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter; 
					os << converter.to_bytes(ws);
					return;
					#if defined (_MSC_VER)
					#pragma warning (pop)
					#endif
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
				os << s;
				}
        };

        template<> class ostringstreamConverter<bool>
			{
            typedef bool T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) {  os << (val ? std::string("true") : std::string("false")); }
			};

        template<> class ostringstreamConverter<signed char>
			{
            typedef signed char T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<short>
			{
            typedef short T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<int>
			{
            typedef int T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<long>
			{
            typedef long T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<long long>
			{
            typedef long long T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<unsigned char>
			{
            typedef unsigned char T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<unsigned short>
			{
            typedef unsigned short T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

			template<> class ostringstreamConverter<unsigned int>
			{
			typedef unsigned int T;
			public:
			static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<unsigned long>
			{
            typedef unsigned long T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<unsigned long long>
			{
            typedef unsigned long long T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::to_string(val); }
			};

        template<> class ostringstreamConverter<float>
			{
            typedef float T;
            public:
			static inline void push(const T & val, mtools::ostringstream & os) { os << ((double)val); }
			};

        template<> class ostringstreamConverter<double>
			{
            typedef double T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << ((os.format_nice()) ? doubleToStringNice(val)  : doubleToStringHighPrecision(val)); }
			};

        template<> class ostringstreamConverter<long double>
			{
            typedef long double T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) 
				{ 
				if (sizeof(long double) == sizeof(double))
					{
					os << ((double)val);
					}
				else
					{ // bigger than usual double so we use default representation.
					std::stringstream stream; stream << val; 
					os << stream.str();
					}
				}
			};

        template<> class ostringstreamConverter<wchar_t>
			{
            typedef wchar_t T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << std::wstring(1, val); }
			};


        template<> class ostringstreamConverter<wchar_t *>
			{
            typedef wchar_t * T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) 
				{ 
				if (val == nullptr) { os << "(wchar_t *)nullptr"; } else { os << std::wstring(val); }
				}
			};

        template<> class ostringstreamConverter<const wchar_t *>
			{
            typedef const wchar_t * T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) 
				{ 
				if (val == nullptr) { os << "(const wchar_t *)nullptr"; } else { os << std::wstring(val); }
				}
		    };

        template<typename U> class ostringstreamConverter< U* >
			{
            typedef U * T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { std::stringstream stream; stream << val; os << stream.str(); }
	        };

        template<size_t N> class ostringstreamConverter< char[N] >
			{
            typedef char T[N];
            public:
			static inline void push(const T & val, mtools::ostringstream & os)
				{
				const chat * p = (const char *)val; 
				if (p == nullptr)  { os << "(char[" << N << "])nullptr"; }
				else
					{
					std::string s((const char *)val, N); 
					size_t l = s.find(0, 0); 
					if (l != string::npos) { s.resize(l); }
					os << s; 
					}
				}
			};

        template<size_t N> class ostringstreamConverter< const char[N] >
			{
            typedef const char T[N];
            public:
            static inline void push(const T & val, mtools::ostringstream & os) 
				{ 
				const chat * p = (const char *)val; 
				if (p == nullptr)  { os << "(const char[" << N << "])nullptr"; }
				else
					{
					std::string s((const char *)val, N); 
					size_t l = s.find(0, 0); 
					if (l != string::npos) { s.resize(l); }
					os << s; 
					}
				}
			};

        template<size_t N> class ostringstreamConverter< wchar_t[N] >
			{
            typedef wchar_t T[N];
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << ((const wchar_t *)val); }  // UNSAFE !!! -> todo: improve that by checking length/null terminating char. 
			};

        template<size_t N> class ostringstreamConverter< const wchar_t[N] >
			{
            typedef const wchar_t T[N];
            public:
            static inline void push(const T & val, mtools::ostringstream & os) { os << ((const wchar_t *)val); }  // UNSAFE !!! -> todo: improve that by checking length/null terminating char. 
			};

        template<typename U, size_t N> class ostringstreamConverter< U[N] >
		    {
            typedef U T[N];
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "Array T[N] with T= '" << typeid(U).name() << "' and N = " << N << "\n";
                for (size_t i = 0; i < N; i++) { os << i << "\t -> " << val[i] << "\n"; }
				}
	        };


        template<typename U, typename V> class ostringstreamConverter< std::pair<U, V> >
	        {
            typedef std::pair<U, V> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os <<"std::pair<" << typeid(U).name() << " , " << typeid(V).name() << "> = (" << val.first << " , " << val.second << ")";
				}
		    };


		template<typename... U> class ostringstreamConverter< std::tuple<U...> >
			{

			template <size_t n> static inline typename std::enable_if<(n >= sizeof...(U))>::type print_tuple_rec(mtools::ostringstream &, const std::tuple<U...>&) {}

			template <size_t n> static inline typename std::enable_if<(n < sizeof...(U))>::type print_tuple_rec(mtools::ostringstream & os, const std::tuple<U...>& tup)
				{
				if (n != 0) { os << ", "; }
				os << std::get<n>(tup);
				print_tuple_rec<n + 1>(os, tup);
				}

			template <size_t n> static inline typename std::enable_if<(n >= sizeof...(U))>::type print_tuple_type_rec(mtools::ostringstream &, const std::tuple<U...>&) {}

			template <size_t n> static inline typename std::enable_if<(n < sizeof...(U))>::type print_tuple_type_rec(mtools::ostringstream & os, const std::tuple<U...>& tup)
				{
				if (n != 0) { os << ", "; }
				os << typeid(decltype(std::get<n>(tup))).name();
				print_tuple_type_rec<n + 1>(os, tup);
				}

			public:

				static inline void push(const std::tuple<U...> & val, mtools::ostringstream & os)
					{
					os << "std::tuple<";
					print_tuple_type_rec<0>(os, val);
					os << "> = (";
					print_tuple_rec<0>(os, val);
					os << ")";
					}
			};


        template<typename U, size_t N> class ostringstreamConverter < std::array<U, N> >
	        {
            typedef std::array<U, N> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
				os << "std::array<" << typeid(U).name() << ">. Size = " << val.size() << "\n";
                for (size_t i = 0; i < N; i++) { os << i << "\t -> " << val[i] << "\n"; }
			    }
		    };


        template<typename U, typename Alloc> class ostringstreamConverter < std::vector<U, Alloc> >
			{
            typedef std::vector<U, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "std::vector<" << typeid(U).name() << "> Size = " << val.size() << "\n";
                for (size_t i = 0; i < val.size(); i++) { os << i << "\t -> " << val[i] << "\n"; }
				}
			};

        template<typename U, typename Alloc> class ostringstreamConverter < std::deque<U, Alloc> >
			{
            typedef std::deque<U, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "std::deque<" << typeid(U).name() << "> Size = " << val.size() << "\n";
				size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " << (*it) << "\n";  ++i; }
				}
			};

        template<typename U, typename Alloc> class ostringstreamConverter < std::forward_list<U, Alloc> >
		    {
            typedef std::forward_list<U, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                size_t n = 0; for (auto it = val.begin(); it != val.end(); ++it) { ++n; }
                os << "std::forward_list<" << typeid(U).name() << "> Size = " << n << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s << i << "\t -> " << (*it) << "\n"; ++i; }
				}
			};


        template<typename U, typename Alloc> class ostringstreamConverter < std::list<U, Alloc> >
			{
            typedef std::list<U, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "std::list<" < typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { s << i  << "\t -> " << (*it) << "\n"; ++i; }
				}
			};


        template<typename U, typename Comp, typename Alloc> class ostringstreamConverter < std::set<U, Comp, Alloc> >
			{
            typedef std::set<U, Comp, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
	            {
                os << "std::set<" << typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " << (*it) << "\n"; ++i; }
		        }
			};


        template<typename U, typename Comp, typename Alloc> class ostringstreamConverter < std::multiset<U, Comp, Alloc> >
			{
            typedef std::multiset<U, Comp, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
		        {
                os << "std::multiset<" << typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " <<  (*it) << "\n"; ++i; }
	            }
			};


        template<typename Key, typename U, typename Comp, typename Alloc> class ostringstreamConverter < std::map<Key, U, Comp, Alloc> >
			{
            typedef std::map<Key, U, Comp, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
			    {
                os << "std::map<" << typeid(Key).name() << "," << typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " << (*it) << "\n"; ++i; }
				}
			};


        template<typename Key, typename U, typename Comp, typename Alloc> class ostringstreamConverter < std::multimap<Key, U, Comp, Alloc> >
			{
            typedef std::multimap<Key, U, Comp, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
		        {
                os << "std::multimap<" <<  typeid(Key).name() <<  "," << typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " << (*it) << "\n"; ++i; }
	            }
			};

        template<typename Key, typename Hash, typename Pred, typename Alloc> class ostringstreamConverter < std::unordered_set<Key, Hash, Pred, Alloc> >
			{
            typedef std::unordered_set<Key, Hash, Pred, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "std::unordered_set<" << typeid(Key).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " << (*it) << "\n"; ++i; }
				}
			};

        template<typename Key, typename Hash, typename Pred, typename Alloc> class ostringstreamConverter < std::unordered_multiset<Key, Hash, Pred, Alloc> >
			{
            typedef std::unordered_multiset<Key, Hash, Pred, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "std::unordered_multiset<" << typeid(Key).name() << "> Size = " <<  val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i  << "\t -> " << (*it) << "\n"; ++i; }
				}
			};

        template<typename Key, typename U, typename Hash, typename Pred, typename Alloc> class ostringstreamConverter < std::unordered_map<Key, U, Hash, Pred, Alloc> >
			{
            typedef std::unordered_map<Key, U, Hash, Pred, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
				{
                os << "std::unordered_map<" << typeid(Key).name() << "," << typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os << i << "\t -> " << it->first << " \t:\t " << it->second << "\n"; ++i; }
			    }
			};

        template<typename Key, typename U, typename Hash, typename Pred, typename Alloc> class ostringstreamConverter < std::unordered_multimap<Key, U, Hash, Pred, Alloc> >
			{
            typedef std::unordered_multimap<Key, U, Hash, Pred, Alloc> T;
            public:
            static inline void push(const T & val, mtools::ostringstream & os)
		        {
                os << "std::unordered_multimap<" << typeid(Key).name() <<  "," << typeid(U).name() << "> Size = " << val.size() << "\n";
                size_t i = 0; for (auto it = val.begin(); it != val.end(); ++it) { os  << i  << "\t -> " << it->first  << " \t:\t " << it->second << "\n"; ++i; }
	            }
			};


	}






	template<typename T> ostringstream & ostringstream::operator<<(const T & obj)
		{
		internals_ostringstream::ostringstreamConverter<T>::push(obj, *this);
		return (*this); 
		}









}



/* end of file */ 