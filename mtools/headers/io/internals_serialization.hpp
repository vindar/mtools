/** @file internals_serialization.hpp */
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



namespace mtools
    {

namespace internals_serialization
    {


    /**
     * OArchive helper classes.
     *
     * Used for partial template specialization for srializating different basic types.
     *
     * @tparam  T   Generic type parameter to serialize.
    **/
    template<typename T> class OArchiveHelper
    {
    public:

        typedef T typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { _write(nbitem, ar, obj, dest, metaprog::dummint<metaprog::is_serializable<T>::value_serialize>()); }

    private:

        static inline void _write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest, metaprog::dummint<metaprog::is_serializable<int>::METHOD_SERIALIZE> dum) { typeT * po = const_cast<typeT *>(&obj); po->serialize(ar); return; }
        static inline void _write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest, metaprog::dummint<metaprog::is_serializable<int>::FUNCTION_SERIALIZE> dum) { typeT * po = const_cast<typeT *>(&obj); serialize(ar, *po); return; }
        static inline void _write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest, metaprog::dummint<metaprog::is_serializable<int>::NONE> dum) {nbitem++; createToken(dest, &obj, sizeof(obj), true, ((sizeof(obj)==0) ? true : false)); }

        OArchiveHelper() = delete;                  // cannot be created, static methods only.
        ~OArchiveHelper() = delete;                 //
        OArchiveHelper(OArchiveHelper &) = delete;  //
    };



    /* specialization for char */
    template<> class OArchiveHelper < char >
    {
    public:
        typedef char typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) {nbitem++; createToken(dest, &obj, 1, doesTokenNeedQuotes(&obj, 1), false); }
    };

    /* specialization for signed char */
    template<> class OArchiveHelper < signed char >
    {
    public:
        typedef signed char typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) {nbitem++; createTokenI((int64)obj, dest); }
    };

    /* specialization for unsigned char */
    template<> class OArchiveHelper < unsigned char >
    {
    public:
        typedef unsigned char typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) {nbitem++; createTokenI((uint64)obj, dest); }
    };

    /* specialization for short int */
    template<> class OArchiveHelper < short int >
    {
    public:
        typedef short int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((int64)obj, dest); }
    };

    /* specialization for unsigned short int */
    template<> class OArchiveHelper < unsigned short int >
    {
    public:
        typedef unsigned short int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((uint64)obj, dest); }
    };

    /* specialization for int */
    template<> class OArchiveHelper < int >
    {
    public:
        typedef int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((int64)obj, dest); }
    };

    /* specialization for unsigned int */
    template<> class OArchiveHelper < unsigned int >
    {
    public:
        typedef unsigned int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((uint64)obj, dest); }
    };

    /* specialization for long int */
    template<> class OArchiveHelper < long int >
    {
    public:
        typedef long int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((int64)obj, dest); }
    };

    /* specialization for unsigned long int */
    template<> class OArchiveHelper < unsigned long int >
    {
    public:
        typedef unsigned long int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((uint64)obj, dest); }
    };

    /* specialization for long long int */
    template<> class OArchiveHelper < long long int >
    {
    public:
        typedef long long int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((int64)obj, dest); }
    };

    /* specialization for unsigned long long int */
    template<> class OArchiveHelper < unsigned long long int >
    {
    public:
        typedef unsigned long long int typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((uint64)obj, dest); }
    };

    /* specialization for float */
    template<> class OArchiveHelper < float >
    {
    public:
        typedef float typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenFP(obj, dest); }
    };

    /* specialization for double */
    template<> class OArchiveHelper < double >
    {
    public:
        typedef double typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenFP(obj, dest); }
    };

    /* specialization for long double */
    template<> class OArchiveHelper < long double >
    {
    public:
        typedef long double typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenFP(obj, dest); }
    };

    /* specialization for bool */
    template<> class OArchiveHelper < bool >
        {
        public:
            typedef bool typeT;
            static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { nbitem++; createTokenI((int64)((obj == true) ? 1 : 0), dest); }
        };


    /* specialization for fixed size C-arrays */
    template<typename T, size_t N> class OArchiveHelper < T[N] >
    {
    public:
        typedef T typeT[N];
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.array(obj, N); }
    };


    /* specialization for fixed size char-arrays */
    template<size_t N> class OArchiveHelper < char[N] >
    {
    public:
        typedef char typeT[N];
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
        {
        MTOOLS_ASSERT(obj != nullptr);
        nbitem++;
        createToken(dest, obj, sizeof(obj), false, true);
        }
    };

    /* specialization for fixed size const char-arrays */
    template<size_t N> class OArchiveHelper < const char[N] >
    {
    public:
        typedef const char typeT[N];
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
        {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, sizeof(obj), false, true);
        }
    };

    /* specialization for fixed size wchar_t-arrays */
    template<size_t N> class OArchiveHelper < wchar_t[N] >
    {
    public:
        typedef wchar_t typeT[N];
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
        {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, sizeof(obj), false, true);
        }
    };

    /* specialization for fixed size const wchar_t-arrays */
    template<size_t N> class OArchiveHelper < const wchar_t[N] >
    {
    public:
        typedef const wchar_t typeT[N];
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
        {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, sizeof(obj), false, true);
        }
    };

    /* specialization for null terminated C-string (const char *) */
    template<> class OArchiveHelper < const char * >
    {
    public:
        typedef const char * typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
        {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, std::strlen(obj), false, true);
        }
    };

    /* specialization for null terminated C-string (char *) */
    template<> class OArchiveHelper < char * >
    {
    public:
        typedef char * typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
        {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, std::strlen(obj), false, true);
        }
    };

    /* specialization for null terminated C-widestring (const wchar_t *) */
    template<> class OArchiveHelper < const wchar_t * >
    {
    public:
        typedef const wchar_t * typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
            {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, wcslen(obj)*sizeof(wchar_t), false, true);
            }
    };

    /* specialization for null terminated C-widestring (wchar_t *) */
    template<> class OArchiveHelper < wchar_t * >
    {
    public:
        typedef wchar_t * typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
            {
            MTOOLS_ASSERT(obj != nullptr);
            nbitem++;
            createToken(dest, obj, wcslen(obj)*sizeof(wchar_t), false, true);
            }
    };

    /* prevent serializing pointers */
    template<typename T> class OArchiveHelper < T * >
    {
    public:
        typedef T * typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { static_assert((sizeof(obj) != sizeof(typeT)), "pointer T* are not serializable. Use opaque() to serialize the adress or cast it to char/wchar_t *  if the memory pointed is null terminated"); }
    };

    /* prevent serializing pointers */
    template<typename T> class OArchiveHelper < const T * >
    {
    public:
        typedef const T * typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { static_assert((sizeof(obj) != sizeof(typeT)), "pointer const T* are not serializable. Use opaque() to serialize the adress or cast it to const char/wchar_t * if the memory pointed is null terminated"); }
    };

    /* specialization for std::pair */
    template<typename T1, typename T2> class OArchiveHelper < std::pair<T1, T2> >
    {
    public:
        typedef std::pair<T1, T2> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&(obj.first); ar.operator&(obj.second); }
    };

    /* specialization for std::string */
    template<> class OArchiveHelper < std::string >
    {
    public:
        typedef std::string typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
            {
            nbitem++;
            createToken(dest, &obj.front(), obj.length(), false, true);
            }
    };

    /* specialization for std::wstring */
    template<> class OArchiveHelper < std::wstring >
    {
    public:
        typedef std::wstring typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest)
            {
            nbitem++;
            createToken(dest, &obj.front(), obj.length()*sizeof(wchar_t), false, true);
            }
    };

    /* specialization for std::array */
    template<typename T, size_t N> class OArchiveHelper < std::array<T, N> >
    {
    public:
        typedef std::array<T, N> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.array(&obj.front(), N); }
    };

    /* specialization for std::vector */
    template<typename T, typename Alloc> class OArchiveHelper < std::vector<T, Alloc> >
    {
    public:
        typedef std::vector<T, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); ar.array(&obj.front(), obj.size()); }
    };

    /* specialization for std::deque */
    template<typename T, typename Alloc> class OArchiveHelper < std::deque<T, Alloc> >
    {
    public:
        typedef std::deque<T, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::forward_list */
    template<typename T, typename Alloc> class OArchiveHelper < std::forward_list<T, Alloc> >
    {
    public:
        typedef std::forward_list<T, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { size_t size = 0; for (auto it = obj.begin(); it != obj.end(); ++it) { size++; } ar.operator&((uint64)size); if (size == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::list */
    template<typename T, typename Alloc> class OArchiveHelper < std::list<T, Alloc> >
    {
    public:
        typedef std::list<T, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* no specialization for std::stack */
    template<typename T, typename Container> class OArchiveHelper < std::stack<T, Container> >
    {
    public:
        typedef std::stack<T, Container> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { static_assert((sizeof(obj) != sizeof(typeT)), "std::stack is not serializable."); }
    };

    /* no specialization for std::queue */
    template<typename T, typename Container> class OArchiveHelper < std::queue<T, Container> >
    {
    public:
        typedef std::queue<T, Container> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { static_assert((sizeof(obj) != sizeof(typeT)), "std::queue is not serializable."); }
    };

    /* no specialization for std::priority_queue */
    template<typename T, typename Container> class OArchiveHelper < std::priority_queue<T, Container> >
    {
    public:
        typedef std::priority_queue<T, Container> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { static_assert((sizeof(obj) != sizeof(typeT)), "std::priority_queue is not serializable."); }
    };

    /* specialization for std::set */
    template<typename T, typename Comp, typename Alloc> class OArchiveHelper < std::set<T, Comp, Alloc> >
    {
    public:
        typedef std::set<T, Comp, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::multiset */
    template<typename T, typename Comp, typename Alloc> class OArchiveHelper < std::multiset<T, Comp, Alloc> >
    {
    public:
        typedef std::multiset<T, Comp, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::map */
    template<typename Key, typename T, typename Comp, typename Alloc> class OArchiveHelper < std::map<Key, T, Comp, Alloc> >
    {
    public:
        typedef std::map<Key, T, Comp, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::multimap */
    template<typename Key, typename T, typename Comp, typename Alloc> class OArchiveHelper < std::multimap<Key, T, Comp, Alloc> >
    {
    public:
        typedef std::multimap<Key, T, Comp, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::unordered_set */
    template<typename Key, typename Hash, typename Pred, typename Alloc> class OArchiveHelper < std::unordered_set<Key, Hash, Pred, Alloc> >
    {
    public:
        typedef std::unordered_set<Key, Hash, Pred, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::unordered_multiset */
    template<typename Key, typename Hash, typename Pred, typename Alloc> class OArchiveHelper < std::unordered_multiset<Key, Hash, Pred, Alloc> >
    {
    public:
        typedef std::unordered_multiset<Key, Hash, Pred, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::unordered_map */
    template<typename Key, typename T, typename Hash, typename Pred, typename Alloc> class OArchiveHelper < std::unordered_map<Key, T, Hash, Pred, Alloc> >
    {
    public:
        typedef std::unordered_map<Key, T, Hash, Pred, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };

    /* specialization for std::unordered_multimap */
    template<typename Key, typename T, typename Hash, typename Pred, typename Alloc> class OArchiveHelper < std::unordered_multimap<Key, T, Hash, Pred, Alloc> >
    {
    public:
        typedef std::unordered_multimap<Key, T, Hash, Pred, Alloc> typeT;
        static inline void write(uint64 & nbitem, OArchive & ar, const typeT & obj, std::string & dest) { ar.operator&((uint64)obj.size()); if (obj.size() == 0) return; for (auto it = obj.begin(); it != obj.end(); ++it) { ar.operator&((*it)); } }
    };


/**
 * IArchive helper classes.
 *
 * Used for partial template specialization for serializating different basic types.
 *
 * @tparam  T   Generic type parameter to serialize.
**/
template<typename T> class IArchiveHelper
{
public:

    typedef T typeT;

    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { _read(nbitem, ar, obj, metaprog::dummint<metaprog::is_serializable<T>::value_deserialize>()); }

private:

    static inline void _read(uint64 & nbitem, IArchive & ar, typeT & obj, mtools::metaprog::dummint<mtools::metaprog::is_serializable<int>::METHOD_DESERIALIZE> dum) { typeT * po = &obj; po->deserialize(ar); return; }
    static inline void _read(uint64 & nbitem, IArchive & ar, typeT & obj, mtools::metaprog::dummint<mtools::metaprog::is_serializable<int>::FUNCTION_DESERIALIZE> dum) { typeT * po = &obj; deserialize(ar, *po); return; }
    static inline void _read(uint64 & nbitem, IArchive & ar, typeT & obj, mtools::metaprog::dummint<mtools::metaprog::is_serializable<int>::METHOD_SERIALIZE> dum) { typeT * po = &obj; po->serialize(ar); return; }
    static inline void _read(uint64 & nbitem, IArchive & ar, typeT & obj, mtools::metaprog::dummint<mtools::metaprog::is_serializable<int>::FUNCTION_SERIALIZE> dum) { typeT * po = &obj; serialize(ar, *po); return; }
    static inline void _read(uint64 & nbitem, IArchive & ar, typeT & obj, mtools::metaprog::dummint<mtools::metaprog::is_serializable<int>::NONE> dum)  { nbitem++; if (ar.readTokenFromArchive(&obj, sizeof(obj)) != sizeof(obj)) throw "IArchive error"; }

    IArchiveHelper() = delete;                  // cannot be created, static methods only.
    ~IArchiveHelper() = delete;                 //
    IArchiveHelper(IArchiveHelper &) = delete;  //
};


/* specialization for char */
template<> class IArchiveHelper < char >
{
public:
    typedef char typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) {if (ar.readTokenFromArchive(&obj, 1) != 1) throw "IArchive error"; nbitem++; return;}
};

/* specialization for signed char */
template<> class IArchiveHelper < signed char >
{
public:
    typedef signed char typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { int64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for unsigned char */
template<> class IArchiveHelper < unsigned char >
{
public:
    typedef unsigned char typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for short int */
template<> class IArchiveHelper < short int >
{
public:
    typedef short int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { int64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for unsigned short int */
template<> class IArchiveHelper < unsigned short int >
{
public:
    typedef unsigned short int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for int */
template<> class IArchiveHelper < int >
{
public:
    typedef int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { int64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for unsigned int */
template<> class IArchiveHelper < unsigned int >
{
public:
    typedef unsigned int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for long int */
template<> class IArchiveHelper < long int >
{
public:
    typedef long int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { int64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for unsigned long int */
template<> class IArchiveHelper < unsigned long int >
{
public:
    typedef unsigned long int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for long long int */
template<> class IArchiveHelper < long long int >
{
public:
    typedef long long int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { int64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};

/* specialization for unsigned long long int */
template<> class IArchiveHelper < unsigned long long int >
{
public:
    typedef unsigned long long int typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) {throw "IArchive error"; } obj = (typeT)v; nbitem++; }
};


/* specialization for float */
template<> class IArchiveHelper < float >
{
public:
    typedef float typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenFP(ar._tempstr, obj) != ar._tempstr.length()) {throw "IArchive error"; }  nbitem++;}
};

/* specialization for double */
template<> class IArchiveHelper < double >
{
public:
    typedef double typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenFP(ar._tempstr, obj) != ar._tempstr.length()) { throw "IArchive error"; }  nbitem++; }
};

/* specialization for long double */
template<> class IArchiveHelper < long double >
{
public:
    typedef long double typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenFP(ar._tempstr, obj) != ar._tempstr.length()) { throw "IArchive error"; }  nbitem++; }
};

/* specialization for bool */
template<> class IArchiveHelper < bool >
    {
    public:
        typedef bool typeT;
        static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { int64 v; ar._tempstr.clear(); ar.readTokenFromArchive(ar._tempstr); if (readTokenI(ar._tempstr, v) != ar._tempstr.length()) { throw "IArchive error"; } obj = ((v == 0) ? false : true); nbitem++; }
    };

/* specialization for fixed size C-arrays */
template<typename T, size_t N> class IArchiveHelper < T[N] >
{
public:
    typedef T typeT[N];
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { ar.array(obj, N); }
};


/* specialization for fixed size char-arrays */
template<size_t N> class IArchiveHelper < char[N] >
{
public:
    typedef char typeT[N];
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj)
        {
        MTOOLS_ASSERT(obj != nullptr);
        if (ar.readTokenFromArchive(obj, sizeof(obj)) != sizeof(obj)) throw "IArchive error";
        nbitem++;
        }
};

/* specialization for fixed size wchar_t-arrays */
template<size_t N> class IArchiveHelper < wchar_t[N] >
{
public:
    typedef wchar_t typeT[N];
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj)
        {
        MTOOLS_ASSERT(obj != nullptr);
        if (ar.readTokenFromArchive(obj, sizeof(obj)) != sizeof(obj)) throw "IArchive error";
        nbitem++;
        }
};

/* specialization for null terminated C-string (char *) */
template<> class IArchiveHelper < char * >
{
public:
    typedef char * typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj)
        {
        MTOOLS_ASSERT(obj != nullptr);
        ar._tempstr.clear();
        ar.readTokenFromArchive(ar._tempstr);
        std::strcpy(obj, ar._tempstr.c_str());
        nbitem++;
        }
};

/* specialization for null terminated C-widestring (wchar_t *) */
template<> class IArchiveHelper < wchar_t * >
{
public:
    typedef wchar_t * typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj)
        {
        MTOOLS_ASSERT(obj != nullptr);
        ar._tempstr.clear();
        ar.readTokenFromArchive(ar._tempstr);
        ar._tempstr.push_back(0); ar._tempstr.push_back(0); // make sure the bufer end with double zero for widestring
        wcscpy(obj, (const wchar_t *)(ar._tempstr.c_str()));
        nbitem++;
        }
};

/* prevent serializing pointers */
template<typename T> class IArchiveHelper < T * >
{
public:
    typedef T * typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { static_assert((sizeof(obj) != sizeof(typeT)), "pointer T* are not deserializable."); }
};


/* prevent serializing pointers */
template<typename T> class IArchiveHelper < const T * >
{
public:
    typedef const T * typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { static_assert((sizeof(obj) != sizeof(typeT)), "pointer const T* are not deserializable."); }
};

/* specialization for std::pair */
template<typename T1, typename T2> class IArchiveHelper < std::pair<T1, T2> >
{
public:
    typedef std::pair<T1, T2> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { ar.operator&(obj.first); ar.operator&(obj.second); }
};

/* specialization for std::string */
template<> class IArchiveHelper < std::string >
{
public:
    typedef std::string typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj)
        {
        obj.clear();
        ar.readTokenFromArchive(obj);
        nbitem++;
        }
};

/* specialization for std::wstring */
template<> class IArchiveHelper < std::wstring >
{
public:
    typedef std::wstring typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj)
        {
        ar._tempstr.clear();
        ar.readTokenFromArchive(ar._tempstr);
        obj.resize(ar._tempstr.length() / sizeof(wchar_t));
        std::memcpy(&obj.front(), &(ar._tempstr.front()), ar._tempstr.length());
        nbitem++;
        }
};

/* specialization for std::array */
template<typename T, size_t N> class IArchiveHelper < std::array<T, N> >
{
public:
    typedef std::array<T, N> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { ar.array(&obj.front(), N); }
};


/* specialization for std::vector */
template<typename T, typename Alloc> class IArchiveHelper < std::vector<T, Alloc> >
{
public:
    typedef std::vector<T, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 n; ar.operator&(n); obj.resize((size_t)n); ar.array(&obj.front(), obj.size()); }
};

/* specialization for std::deque */
template<typename T, typename Alloc> class IArchiveHelper < std::deque<T, Alloc> >
{
public:
    typedef std::deque<T, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 n; ar.operator&(n); obj.resize((size_t)n); if (n == 0) return; auto it = obj.begin(); for (uint64 i = 0; i < n; ++i) { ar.operator&((*it)); ++it; } }
};

/* specialization for std::forward_list */
template<typename T, typename Alloc> class IArchiveHelper < std::forward_list<T, Alloc> >
{
public:
    typedef std::forward_list<T, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 n; ar.operator&(n); obj.resize((size_t)n); if (n == 0) return; auto it = obj.begin(); for (uint64 i = 0; i < n; ++i) { ar.operator&((*it)); ++it; } }
};

/* specialization for std::list */
template<typename T, typename Alloc> class IArchiveHelper < std::list<T, Alloc> >
{
public:
    typedef std::list<T, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { uint64 n; ar.operator&(n); obj.resize((size_t)n); if (n == 0) return; auto it = obj.begin(); for (uint64 i = 0; i < n; ++i) { ar.operator&((*it)); ++it; } }
};

/* no specialization for std::stack */
template<typename T, typename Container> class IArchiveHelper < std::stack<T, Container> >
{
public:
    typedef std::stack<T, Container> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { static_assert((sizeof(obj) != sizeof(typeT)), "std::stack is not deserializable."); }
};

/* no specialization for std::queue */
template<typename T, typename Container> class IArchiveHelper < std::queue<T, Container> >
{
public:
    typedef std::queue<T, Container> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { static_assert((sizeof(obj) != sizeof(typeT)), "std::queue is not deserializable."); }
};

/* no specialization for std::priority_queue */
template<typename T, typename Container> class IArchiveHelper < std::priority_queue<T, Container> >
{
public:
    typedef std::priority_queue<T, Container> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { static_assert((sizeof(obj) != sizeof(typeT)), "std::priority_queue is not deserializable."); }
};

/* specialization for std::set */
template<typename T, typename Comp, typename Alloc> class IArchiveHelper < std::set<T, Comp, Alloc> >
{
public:
    typedef std::set<T, Comp, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; T a; for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::multiset */
template<typename T, typename Comp, typename Alloc> class IArchiveHelper < std::multiset<T, Comp, Alloc> >
{
public:
    typedef std::multiset<T, Comp, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; T a; for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::map */
template<typename Key, typename T, typename Comp, typename Alloc> class IArchiveHelper < std::map<Key, T, Comp, Alloc> >
{
public:
    typedef std::map<Key, T, Comp, Alloc> typeT;

    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; std::pair<Key,T> a;  for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::multimap */
template<typename Key, typename T, typename Comp, typename Alloc> class IArchiveHelper < std::multimap<Key, T, Comp, Alloc> >
{
public:
    typedef std::multimap<Key, T, Comp, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; std::pair<Key, T> a;  for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::unordered_set */
template<typename Key, typename Hash, typename Pred, typename Alloc> class IArchiveHelper < std::unordered_set<Key, Hash, Pred, Alloc> >
{
public:
    typedef std::unordered_set<Key, Hash, Pred, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; Key a; for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::unordered_multiset */
template<typename Key, typename Hash, typename Pred, typename Alloc> class IArchiveHelper < std::unordered_multiset<Key, Hash, Pred, Alloc> >
{
public:
    typedef std::unordered_multiset<Key, Hash, Pred, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; Key a; for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::unordered_map */
template<typename Key, typename T, typename Hash, typename Pred, typename Alloc> class IArchiveHelper < std::unordered_map<Key, T, Hash, Pred, Alloc> >
{
public:
    typedef std::unordered_map<Key, T, Hash, Pred, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; std::pair<Key, T> a;  for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};

/* specialization for std::unordered_multimap */
template<typename Key, typename T, typename Hash, typename Pred, typename Alloc> class IArchiveHelper < std::unordered_multimap<Key, T, Hash, Pred, Alloc> >
{
public:
    typedef std::unordered_multimap<Key, T, Hash, Pred, Alloc> typeT;
    static inline void read(uint64 & nbitem, IArchive & ar, typeT & obj) { obj.clear(); uint64 n; ar.operator&(n); if (n == 0) return; std::pair<Key, T> a;  for (uint64 i = 0; i < n; ++i) { ar.operator&(a); obj.insert(obj.end(), a); } }
};



}

}


/* end if file */

