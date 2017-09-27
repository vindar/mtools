/** @file serialization.hpp */
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




#include "../misc/error.hpp"
#include "../misc/metaprog.hpp"
#include "../misc/stringfct.hpp"
#include "../io/fileio.hpp"

#include <utility>
#include <fstream>
#include <type_traits>
#include <wchar.h>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <complex>
#include <unordered_set>
#include <unordered_map>




#if defined (_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif

namespace mtools
    {

    namespace internals_serialization
        {
        /* forward declaration of helper classes */
        template<typename T> class OArchiveHelper;
        template<typename T> class IArchiveHelper;
        };


    /**
     * Serializer class. This class is used for serializing objects. Inspired by (but much simpler
     * than boost serialization classes).
     *
     * - Operator&  is used for serialising an object into the file.
     * - Operator<< is used to add a commment in the file; It does not affect the serialization itself
     * .
     * - The output file is in tesxt format. Comments start with '%' and stop at the next '%' or
     * next new line.
     * - Serialization is defined for all fundamental types. Serialization of a pointer is forbidden
     * except for (const) char* and (const) wchar_t* which are interpreted as C string hence must be
     * valid 0 terminated strings.
     *
     * When serializing an object of type T, the archive looks for custom defined serialize method
     * in the following order of preference:
     *
     * - 1) `void T->serialize(OArchive & ar)` (member function)
     * - 2) `void serialize(OArchive & ar, [const] T & obj)` (global function)
     *
     * If no such method can be found, serialization is performed using memcpy of the memory
     * associated with the object.
     *
     * It is recommended to use a member method  of the form: `template<typename ARCHIVE> void
     * serialize(ARCHIVE & ar, const int version = 0)` which makes it compatible with the
     * boost::serialization classes.
     *
     * If there is an error while performing serialization, an exception (type const char *) is
     * thrown.
     *
     * @code{.cpp}
     * class GPS
     * {
     * public:
     *
     * GPS(mtools::iVec2 p, double deg) : pos(p), degree(deg) {}
     *
     * template<typename T> void serialize(T & ar, const int version = 0) const
     * {
     * ar << "GPS class object";
     * ar & pos;
     * ar & degree;
     * }
     *
     * // the deserialize method is not necessary here since serialize() above also accept IArchive.
     * // But since the method is present,it is chosen over serialize() when performing deserialization.
     * void deserialize(IArchive & ar, const int version = 0)
     * {
     * ar & pos;
     * ar & degree;
     * }
     *
     * mtools::iVec2 pos;
     * double degree;
     * };
     *
     *
     * int main()
     * {
     * { // serialize
     * GPS gps({ 1, 2 }, 1.3);
     * std::set<int> myset = { 8, 1, 2, 7, 0 };
     * char tab[10] = "azerty";
     *      p = tab;
     * std::wstring s = L"hello";
     * int ti[5] = { 5, 2, 6, 7, 1 };
     * OArchive ar("save.ar.gz"); // use compression
     * ar << "Ceci est un commentaire"; // a comment
     * ar & gps;           // save gps
     * ar.opaque(gps);     // save again but in an opaque way using memcpy
     * ar & myset;         // save the set
     * ar & tab;           // save the C array
     * ar & p;             // save as a null terminated C array
     * ar.array(ti, 4);    // save as an array of size 4
     * ar.opaqueArray(ti, 4); // save again but in an opaque way
     * ar & s;             // save the wstring
     * }
     * GPS gps({ 0, 0 }, 0);
     * std::set<int> myset;
     * char tab[10];
     * char tab2[10];
     *      p = tab2;
     * std::wstring s;
     * int ti[5];
     * IArchive ar("save.ar.gz"); // open the file
     * ar & gps;
     * ar.opaque(gps);
     * ar & myset & tab & p;
     * ar.array(ti, 4);
     * ar.opaqueArray(ti, 4);
     * ar & s; // deserialization
     * return 0;
     * }
     * @endcode.
     *
     * @sa  class IArchive
    **/
    class OArchive
        {
        public:

            /**
             * Constructor. Create a new archive. If a file with the same name already exist, it is
             * truncated without warning.
             *
             * @param   filename    Filename of the archive. Compression is determined depending on the file
             *                      extension : use a ".gz",".gzip" or ".z" extension to create a compressed
             *                      archive (for example "distrib.ar.gz"), oitherwise, the archive is in plain
             *                      text format.
            **/
            OArchive(const std::string & filename) : _filename(filename), _startline(true), _comment(false), _indent(0), _compress(false),  _nbitem(0), _gzhandle(nullptr) , _handle(nullptr)
                {
                std::string ext = toLowerCase(extractExtension(filename));
                if ((ext == std::string("gz")) || (ext == std::string("gzip")) || (ext == std::string("z"))) { _compress = true; }
                static const char * ARCHIVE_HEADER = "mtools::archive version 1.0\n";
                _openFile();
                _insertComment(std::string(ARCHIVE_HEADER));
                }


            /**
             * Destructor. Save and close the file containing the archive.
             **/
            ~OArchive()
                {
                setIndent(0);
                static const char * ARCHIVE_TRAILER1 = "\nnumber of items: ";
                static const char * ARCHIVE_TRAILER2 = "\nend of archive\n";
                _insertComment(std::string(ARCHIVE_TRAILER1) + toString(_nbitem) + std::string(ARCHIVE_TRAILER2));
                _closeFile();
                }


            /**
             * Serialize an object into the archive. If the object implement a "serialize" method (local or
             * global) it is used. Otherwise, basic memcpy of the object is used.  Specialization for all
             * basic type and std container is provided.
             *
             * Serialization method:
             * - char and wchar_t : serialized as characters
             * - integer types : written in decimal form
             * - floating point type : written in decimal scientific notation. This is user readable
             * friendly but not very efficient. Use opaque() for fast reliable serialization fo floating
             * point values.
             * - char and wchar pointer : interpreted as null-terminated C-string which are written on file
             * using a C-escaped sequence. Beware of buffer overflow !
             * - Other pointers : not serializable ! Use array() or opaqueArray() to serialize the values
             * pointed.
             * - fixed-size C-array : size automatically detected. Serialized as a C-escaped string  for
             * type char and wchar_t, serialized as a fixed size array otherwise (i.e. same as the array()
             * method).
             * - std::container : the size followed by the objects contained in it.
             * - std::string and std::wstring : serialized as C-string written using a C-escape sequence.
             * - std::pair : both object written.
             *
             * @tparam  T   Generic type parameter.
             * @param   obj The object to serialize.
             *
             * @return  The archive for chaining.
            **/
            template<typename T> OArchive & operator&(const T & obj);


            /**
             * Serialize an object in the archive in an opaque way : the memory representing the object is
             * simply copied using a memcpy and save in the form "\xXXXXXX...". This is the fastest
             * serialization method and should be used when serializing very large objects (but it is also
             * the least portable and not readable).
             *
             * @tparam  T   Generic type parameter.
             * @param   obj The object to serialize.
             *
             * @return  The archive for chaining.
             **/
            template<typename T> OArchive & opaque(const T & obj)
                {
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
                _makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; } // exit comment mode if needed
                if (sizeof(T) == 0) return(*this);
                _nbitem++;
                createToken(_writeBuffer, p, sizeof(obj), true, false);
                _flush();
                return(*this);
                }


            /**
             * Serialize an array of objects of type T. Each object in the array is serialized using the
             * object serialization method. If the array has zero element, nothing is written.
             *
             * @tparam  T   Generic type parameter.
             * @param   p   pointer the the array of objects.
             * @param   len number of objects in the array.
             *
             * @return  The archive for chaining.
             **/
            template<typename T> OArchive & array(const T * pp, size_t len)
                {
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
                MTOOLS_ASSERT(p != nullptr);
                _makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; } // exit comment mode if needed
                if (len == 0) return(*this);
                for (size_t i = 0; i < len; i++) { operator&(p[i]); } // serialize each element of the array.
                _flush();
                return(*this);
                }


            /**
             * Serialize an array of objects of type T in raw form : the memory represented by the whole
             * array is simply copied using memcpy() and save in the form "\xXXXXXX...". This is the fastest
             * serialization method and should be used when serializing very large objects (but the least
             * portable and not readable). If the array has 0 elements, nothing is written.
             *
             * @tparam  T   Generic type parameter.
             * @param   p   pointer to the aray of T objects.
             * @param   len number of objects in the array.
             *
             * @return  The archive for chaining.
             **/
            template<typename T> OArchive & opaqueArray(const T * pp, size_t len)
                {
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
                MTOOLS_ASSERT(p != nullptr);
                _makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; }
                if (len*sizeof(T) == 0) return(*this);
                _nbitem++;
                createToken(_writeBuffer, p, sizeof(T)*len, true, false);
                _flush();
                return(*this);
                }


            /**
             * Add a comment into the archive. Not very efficient: should be used sparringly when writing
             * very large archives.
             *
             * @param   obj The comment to add, formatted using the toString() method.
             *
             * @return  The archive for chaining.
             **/
            template<typename T> OArchive & operator<<(const T & obj) 
                { 
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
                _insertComment(toString(*p)); return(*this); 
                }


            /**
             * Insert a given number of tabulation.
             *
             * @param   nb  Number of tabulation to add (default = 1).
             **/
            OArchive & tab(size_t nb = 1)
                {
                if (nb > 0) { _writeBuffer.append(nb, '\t'); _flush(); }
                return(*this);
                }


            /**
             * Skip a given number of lines.
             *
             * @param   nb  Number of new lines (default = 1).
             *
             * @return  The archive for chaining.
             **/
            OArchive & newline(size_t nb = 1)
                {
                if (nb > 0) { _writeBuffer.append(nb, '\n'); _flush(); _comment = false; _startline = true; _comment = false; }
                return(*this);
                }


            /**
             * Sets the indentation at each new line.
             *
             * @param   n   Number of tabulation at the beginning of each new line (default 0).
             **/
            OArchive & setIndent(unsigned int n = 0) { _indent = n; return(*this); }


            /**
             * Increment the indentation (number of tabulations) at each new line.
             **/
            OArchive & incIndent() { _indent++; return(*this); }


            /**
            * decrement the indentation (number of tabulations) at each new line.
            **/
            OArchive & decIndent() { _indent--; return(*this); }


            /**
            * Return the number of items that have been deserialized.
            **/
            uint64 nbItem() const { return _nbitem; }

        private:

            /* friend with the helper class */
            template<typename T> friend class internals_serialization::OArchiveHelper;

            /* add a comment string */
            void _insertComment(std::string str)
                {
                if (str.length() == 0) return; // nothing to do
                replace(str, "%", "#"); // replace every % by #
                while (1)
                    {
                    size_t pos = str.find('\n');
                    auto str2 = str.substr(0, pos);
                    if (str2.length() != 0)
                        {
                        _makeSpace(); // make indentation if beginning of new line or space if not yet in comment mode
                        if (!_comment) { _writeBuffer.append(1, '%'); _comment = true; } // go in comment mode if this is not yet the case
                        _writeBuffer += str2;
                        _flush();
                        }
                    if (pos == str.npos) { return; }
                    newline();
                    str = str.substr(pos + 1, str.npos);
                    if (str.length() == 0) return;
                    }
                }


            /* open the archive file for writing, overwrite any existing file */
			void _openFile();


            /* close the archive file */
			void _closeFile();



            /* add a space between token and new line tabulation if needed */
            inline void _makeSpace()
                {
                if (_startline)
                    { // add tabulation at the beginning of the new line if needed.
                    _startline = false;
                    if (_indent > 0) { _writeBuffer.append(_indent, '\t'); }
                    _flush();
                    return;
                    }
                else
                    { // add a space if not in comment mode.
                    if (!_comment) { _writeBuffer.append(1, ' '); _flush(); }
                    }
                }



            /* write the buffer to the file (if needed or forced) */
			void _flush(bool force = false);



            static const size_t GZIPBUFFERSIZE = 512000;
            static const size_t WRITEBUFFERSIZE = 200000;
            std::string _filename;           // name of the archive file
            std::string _writeBuffer;        // the buffer where the archve is written
            bool _startline;                 // true if we are at the beginning of a new line
            bool _comment;                   // true if we are in comment mode
            size_t _indent;                  // number of indentation at the beginning of each new line
            bool _compress;                  // true if we are using compression
            uint64 _nbitem;                  // number of item in the archive
            void * _gzhandle;                // gzFile handle when using compression
            FILE * _handle;                  // handle when not using compression

        };


        /**
         * Deserializer class. This class is used for deserializing objects. Inspired (but much simpler
         * than boost serialize classes).
         *
         * This class perform the reversed operation of the OArchive class. When deserialization of an
         * object is performed the archive looks for custom defined deserialize method in the following
         * order of preference:
         *
         * - 1) `void T->deserialize(IArchive & ar)` (member function)
         * - 2) `void deserialize(IArchive & ar, [const] T & obj)` (global function)
         * - 3) `void T->serialize(IArchive & ar)` (member function)
         * - 4) `void serialize(IArchive & ar, [const] T & obj)` (global function)
         *
         * If no such method can be found, deserialization is performed using memcpy of the memory
         * associated with the object.
         *
         * When the object is simple enough, it is possible to use the same method as that used for
         * serialization with signature `template<typename ARCHIVE> void serialize(ARCHIVE & ar, const
         * int version = 0)` which makes it compatible with the boost::serialization classes.
         *
         * If there is an error while deserialization, an exception (type const char *) is thrown.
         *
         * @sa  class OArchive
        **/
        class IArchive
        {

        public:

            /**
             * Constructor. Open an archive. Determine whether the archive is compressed automatically .
             * Throw an excpetion if the file cannot be open.
             *
             * @param   filename    Filename of the archive.
            **/
            IArchive(const std::string & filename) : _readSize(0), _readPos(0), _filename(filename), _nbitem(0), _gzhandle(nullptr)
                {
                _readBuffer = new char[READBUFFERSIZE];
                _openFile();
                }


            /**
            * Destructor. Close the file containing the archive.
            **/
            ~IArchive()
                {
                _closeFile();
                delete[] _readBuffer;
                }


            /**
             * Deserialize an object from the archive. Oppsite of the serailize method of OArchive.
             *
             * Beware that when deserializing char * and wchar *, the pointed memory is NOT allocated and no
             * checking is performed to insure that it is long enough to contain the whole deserialized
             * string.
             *
             * @tparam  T   Generic type parameter.
             * @param [in,out]  obj The object to serialize.
             *
             * @return  The archive for chaining.
            **/
            template<typename T> IArchive & operator&(T & obj);


            /**
             * deserialize an object in the archive that was previously stored using Oarchive::opaque : the
             * memory representing the object is simply copied using a memcpy.
             *
             * @tparam  T   Generic type parameter.
             * @param   obj The object.
             *
             * @return  the archive for chaining.
             **/
            template<typename T> IArchive & opaque(T & obj)
                {
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
                if (sizeof(T) == 0) return(*this);
                _nbitem++;
                if (readTokenFromArchive(p, sizeof(T)) != sizeof(obj)) { MTOOLS_THROW("IArchive error (opaque)"); }
                return(*this);
                }


            /**
             * Deserialize an array of objects of type T. Each object in the array is deserialized using the
             * object deserialization method. If the array has zero element, nothing is done.
             *
             * @tparam  T   Generic type parameter.
             * @param [in,out]  p   pointer the the array of T objects.
             * @param   len         number of objects in the array.
             *
             * @return  The archive for chaining.
             **/
            template<typename T> IArchive & array(T * pp, size_t len)
                {
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
                MTOOLS_ASSERT(p != nullptr);
                if (len == 0) return(*this);
                for (size_t i = 0; i < len; i++) { operator&(p[i]); } // unserialize each element of the array.
                return(*this);
                }


            /**
             * Deserialize an array of objects of type T that was previously serialized with opaqueArray().
             * The whole memory represented by the array is obtained using simple memcpy(). If the array has
             * 0 elements, nothing is done.
             *
             * @tparam  T   Generic type parameter.
             * @param   p   pointer to the aray of T objects.
             * @param   len number of objects in the array.
             *
             * @return  The archive for chaining.
             **/
            template<typename T> IArchive & opaqueArray(T * pp, size_t len)
                {
                typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
                cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
                MTOOLS_ASSERT(p != nullptr);
                if (len*sizeof(T) == 0) return(*this);
                _nbitem++;
                if (readTokenFromArchive(p, sizeof(T)*len) != (sizeof(T)*len)) { MTOOLS_THROW("IArchive error (opaquearray)"); }
                return(*this);
                }


            /** For compatibility with OArchive, does nothing... **/
            template<typename T> IArchive & operator<<(const T & obj) { return(*this); }

            /** For compatibility with OArchive, does nothing... **/
            IArchive & tab(size_t nb = 1) { return(*this); }

            /** For compatibility with OArchive, does nothing... **/
            IArchive & newline(size_t nb = 1) { return(*this); }

            /** For compatibility with OArchive, does nothing... **/
            IArchive & setIndent(unsigned int n = 0) { return(*this); }

            /** For compatibility with OArchive, does nothing... **/
            IArchive & incIndent() { return(*this); }

            /** For compatibility with OArchive, does nothing... **/
            IArchive & decIndent() { return(*this); }


            /**
             * Return the number of items that have been deserialized.
             **/
            uint64 nbItem() const { return _nbitem; }


    private:

            /* friend with the helper class */
            template<typename T> friend class internals_serialization::IArchiveHelper;


            /* read a token and put it in a given buffer. throws if the buffer is too small
               or if there is no more token to read
               return the size of the token. */
            size_t readTokenFromArchive(void * dest_buffer, size_t dest_len)
                {
                const char * buf = _readBuffer + _readPos; // compute the position of the first char available in the buffer
                size_t nb = _readSize - _readPos;          // number of chars available starting from this position
                bool found = findNextToken<IArchive::_refillStatic>(buf, nb, this); // find the beginning of the next token
                if (!found) { MTOOLS_THROW("IArchive error"); } // no more token found
                const char * buf2 = buf + nb; // ok, token found at buf2 = buf + nb
                size_t nb2 = (_readBuffer + _readSize) - buf2; // number of byte remaining in the buffer starting at buf2
                size_t l = readToken<IArchive::_refillStatic>(dest_buffer, dest_len, buf2, nb2, this); // read the token
                if (buf2 == nullptr) { _readPos = 0; _readSize = 0; return l; } // ok, but the end of the file was reached
                _readPos = (buf2 + nb2) - _readBuffer; // compute the new position of the first available char w.r.t. _readBuffer
                return l;
                }


            /* read a token and append it to a given buffer. throws if there is no more token to read
               return the size of the token. */
            size_t readTokenFromArchive(std::string & dest)
                {
                const char * buf = _readBuffer + _readPos; // compute the position of the first char available in the buffer
                size_t nb = _readSize - _readPos;          // number of chars available starting from this position
                bool found = findNextToken<IArchive::_refillStatic>(buf, nb, this); // find the beginning of the next token
                if (!found) { MTOOLS_THROW("IArchive error"); } // no more token found
                const char * buf2 = buf + nb; // ok, token found at buf2 = buf + nb
                size_t nb2 = (_readBuffer + _readSize) - buf2; // number of byte remaining in the buffer starting at buf2
                size_t l = readToken<IArchive::_refillStatic>(dest, buf2, nb2, this); // read the token
                if (buf2 == nullptr) { _readPos = 0; _readSize = 0; return l; } // ok, but the end of the file was reached
                _readPos = (buf2 + nb2) - _readBuffer; // compute the new position of the first available char w.r.t. _readBuffer
                return l;
                }


            /* static method called when the read buffer must be refilled */
            static const char * _refillStatic(size_t & len, void * data) { MTOOLS_ASSERT(data != nullptr); return ((IArchive *)data)->refill(len); }

            /* refill the read buffer */
			const char * refill(size_t & len);

            /* open the archive file for reading, throw if error */
			void _openFile();

            /* close the archive file */
			void _closeFile();

            static const size_t GZIPBUFFERSIZE = 512000;
            static const size_t READBUFFERSIZE = 512000;

            char * _readBuffer;             // read buffer
            size_t _readSize;               // number of char in the read buffer
            size_t _readPos;                // pos of the first char not read in the read buffer
            std::string _tempstr;           // temporary string used for reconstructing objects
            std::string _filename;           // name of the archive file
            uint64 _nbitem;                  // number of item in the archive
            void * _gzhandle;                // gzfile handle
        };

    }


/* include the helper class definiton */
#include "internals_serialization.hpp"


namespace mtools
{

    template<typename T> OArchive & OArchive::operator&(const T & obj)
        {
        typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
        cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
        _makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; } // exit comment mode if needed
        internals_serialization::OArchiveHelper<cvT>::write(_nbitem, *this, (*p), _writeBuffer); // serialize the object into the archive using the helper class
        _flush();
        return(*this);
        }


    template<typename T> IArchive & IArchive::operator&(T & obj)
        {
        typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
        cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
        internals_serialization::IArchiveHelper<cvT>::read(_nbitem, *this, (*p)); // deserialize the object into the archive using the helper class
        return(*this);
        }


 }

#if defined (_MSC_VER)
#pragma warning( pop )
#endif


 /* end of file */

