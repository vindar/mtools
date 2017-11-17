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



#include "../misc/misc.hpp"
#include "../misc/internal/mtools_export.hpp"
#include "../misc/error.hpp"
#include "../misc/metaprog.hpp"
#include "../misc/stringfct.hpp"
#include "../io/fileio.hpp"


/* include the helper class definiton */
#include "internal/internals_serialization.hpp"


#if defined (_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif

namespace mtools
    {


	/**
	* Base Serializer class. This class is used for serializing objects. Inspired by (but much simpler)
	* than boost serialization classes.
	* 
	* This class should not be used directly. Use instead one of the derived class:
	*	- OFileArchive  
	*	- OStringArchive  
	*	- OCPPArchive
	*
	* Remarks: 
	* 
	* - Operator&  is used for serialising an object into the file.
	* - Operator<< is used to add a commment in the file; It does not affect the serialization itself
	* - The output file is in text format. Comments start with '%' and stop at the next '%' or next new line.
	* - Serialization is defined for all fundamental types. Serialization of a pointer is forbidden
	* except for (const) char* and (const) wchar_t* which are interpreted as C string hence must be
	* valid 0 terminated strings.
	*
	* When serializing an object of type T, the archive looks for custom defined serialize method
	* in the following order of preference:
	*
	* - 1) `void T->serialize(OBaseArchive & ar)` (member function)
	* - 2) `void serialize(OBaseArchive & ar, [const] T & obj)` (global function)
	*
	* If no such method can be found, serialization is performed using memcpy of the memory
	* associated with the object.
	*
	* It is recommended to use a member method  of the form: `template<typename ARCHIVE> void
	* serialize(ARCHIVE & ar, const int version = 0)` which makes it compatible with the
	* boost::serialization classes.
	*
	* If there is an error while performing serialization, an exception is thrown.
	*
	* @sa  class IBaseArchive
	**/
	class OBaseArchive
		{

		public:

			/** Default constructor. */
			OBaseArchive() :  _startline(true), _comment(false), _indent(0), _nbitem(0), _writeBuffer()
				{
				const size_t BUFFER_SIZE = 512000;
				_writeBuffer.reserve(BUFFER_SIZE);
				}


			/** Destructor. */
			virtual ~OBaseArchive()
				{
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
			template<typename T> OBaseArchive & operator&(const T & obj)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
				_makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; } // exit comment mode if needed
				internals_serialization::OArchiveHelper<cvT, OBaseArchive>::write(_nbitem, *this, (*p), _writeBuffer); // serialize the object into the archive using the helper class
				_flush();
				return(*this);
				}


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
			template<typename T> OBaseArchive & opaque(const T & obj)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
				_makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; } // exit comment mode if needed
				if (sizeof(T) == 0) { _flush(); return(*this); }
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
			template<typename T> OBaseArchive & array(const T * pp, size_t len)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
				MTOOLS_ASSERT(p != nullptr);
				_makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; } // exit comment mode if needed
				if (len == 0) { _flush(); return(*this); }
				for (size_t i = 0; i < len; i++) { operator&(p[i]); } // serialize each element of the array.
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
			template<typename T> OBaseArchive & opaqueArray(const T * pp, size_t len)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
				MTOOLS_ASSERT(p != nullptr);
				_makeSpace(); if (_comment) { _writeBuffer.append("% "); _comment = false; }
				if (len * sizeof(T) == 0) { _flush(); return(*this); }
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
			template<typename T> OBaseArchive & operator<<(const T & obj)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
				_insertComment(toString(*p)); 
				_flush();
				return(*this);
				}


			/**
			* Insert a given number of tabulation.
			*
			* @param   nb  Number of tabulation to add (default = 1).
			**/
			OBaseArchive & tab(size_t nb = 1)
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
			OBaseArchive & newline(size_t nb = 1)
				{
				_newline(nb);
				_flush();
				return(*this);
				}


			/**
			* Sets the indentation at each new line.
			*
			* @param   n   Number of tabulation at the beginning of each new line (default 0).
			**/
			OBaseArchive & setIndent(unsigned int n = 0) { _indent = n; return(*this); }


			/**
			* Increment the indentation (number of tabulations) at each new line.
			**/
			OBaseArchive & incIndent() { _indent++; return(*this); }


			/**
			* decrement the indentation (number of tabulations) at each new line.
			**/
			OBaseArchive & decIndent() { _indent--; return(*this); }


			/**
			* Return the number of items that have been deserialized.
			**/
			uint64 nbItem() const { return _nbitem; }


		protected:

			/** Return the current write buffer. */
			std::string & getbuffer() {	return _writeBuffer; }

			/** Return the current write buffer. */
			const std::string & getbuffer() const { return _writeBuffer; }

			/** This method should be called by the derived  class constructor to write the header. */
			void header()
				{
				static const char * ARCHIVE_HEADER = "mtools::archive version 1.0\n";
				setIndent(0);
				_insertComment(std::string(ARCHIVE_HEADER));
				_flush();
				}


			/** This method should be called by the derived class destructor to write the footer. */
			void footer()
				{
				static const char * ARCHIVE_TRAILER1 = "\nnumber of items: ";
				static const char * ARCHIVE_TRAILER2 = "\nend of archive\n";
				setIndent(0);
				_insertComment(std::string(ARCHIVE_TRAILER1) + toString(_nbitem) + std::string(ARCHIVE_TRAILER2));
				_flush();
				}


			/**
			* Called when there is some serialized data to process.
			* Must be overloaded in the derived class.
			**/
			virtual void output(std::string & str) { MTOOLS_ERROR("Base virtual method OBaseArchive::output() should not be called!"); }


		private:

			/* no copy */
			OBaseArchive(const OBaseArchive &) = delete;
			OBaseArchive & operator=(const  OBaseArchive &) = delete;


			/* friend with the helper class */
			template<typename T, typename OARCHIVE> friend class internals_serialization::OArchiveHelper;


			/* insert new lines */
			void _newline(size_t nb = 1)
				{
				if (nb > 0) { _writeBuffer.append(nb, '\n'); _comment = false; _startline = true; _comment = false; }
				}


			/* add a comment string */
			inline void _insertComment(std::string str)
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
						}
					if (pos == str.npos) { return; }
					_newline();
					str = str.substr(pos + 1, str.npos);
					if (str.length() == 0) return;
					}
				}


			/* add a space between token and new line tabulation if needed */
			inline void _makeSpace()
				{
				if (_startline)
					{ // add tabulation at the beginning of the new line if needed.
					_startline = false;
					if (_indent > 0) { _writeBuffer.append(_indent, '\t'); }
					return;
					}
				else
					{ // add a space if not in comment mode.
					if (!_comment) { _writeBuffer.append(1, ' '); }
					}
				}


			/* write the buffer to the file (if needed or forced) */
			inline void _flush()
				{
				output(_writeBuffer);
				}


			bool _startline;                 // true if we are at the beginning of a new line
			bool _comment;                   // true if we are in comment mode
			size_t _indent;                  // number of indentations at the beginning of each new line
			uint64 _nbitem;                  // number of items in the archive
			std::string _writeBuffer;        // the write buffer

		};




	/** Class performing serialization into an std:string.  */
	class OStringArchive : public OBaseArchive
		{

		public: 

		/** Default constructor. */
		OStringArchive() : OBaseArchive() { header(); }

		/** Destructor. */
		virtual ~OStringArchive() {}

		/**
		 * Return the current serialized string. 
		 */
		const std::string & get() const { return getbuffer(); }


		protected:

			virtual void output(std::string & str) override { }

		};




	/**
	 * Class used to serialize directly into a source file: construct an std::string which can be
	 * directly pasted into a .cpp file.
	 **/
	class OCPPArchive : public OBaseArchive
		{

		public:

			/**
			 * Constructor.
			 *
			 * @param	name Name of the object to create. (of type const p_char[])
			 */
			OCPPArchive(const std::string name) : OBaseArchive(), _name(name) { }


			/** Destructor. */
			virtual ~OCPPArchive() {}


			/**
			 * Return a string containing the serialized object formatted in such way that it can be
			 * included in a .cpp file.
			 */
			std::string get() const;


		protected:

			virtual void output(std::string & str) override { }

		private:

			std::string _name;

		};




	/** Class to serialize into a file (use compression depending on the file extension) */
	class OFileArchive : public OBaseArchive
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
			OFileArchive(const std::string & filename);

			/**
			* Destructor. Save and close the file containing the archive.
			**/
			virtual ~OFileArchive();


		protected:

			virtual void output(std::string & str) override { _write(str, false);  }

		private:

			void _openFile();
			void _closeFile();
			void _write(std::string & str, bool force);

			static const size_t GZIPBUFFERSIZE = 512000;
			static const size_t WRITEBUFFERSIZE = 512000;

			std::string _filename;           // name of the archive file
			bool _compress;                  // true if we are using compression
			void * _handle;                  // gzFile handle when using compression
		};





	/**
	* Deserializer base class. This class is used for deserializing objects. Inspired (but much simpler
	* than boost serialize classes).
	*
	* This class should not be used directly. Use instead one of the derived class:
	*	- IFileArchive
	*	- IStringArchive
	*	- ICPPArchive
	*
	* This class perform the reversed operation of the OBaseArchive class. When deserialization of an
	* object is performed the archive looks for custom defined deserialize method in the following
	* order of preference:
	*
	* - 1) `void T->deserialize(IBaseArchive & ar)` (member function)
	* - 2) `void deserialize(IBaseArchive & ar, [const] T & obj)` (global function)
	* - 3) `void T->serialize(IBaseArchive & ar)` (member function)
	* - 4) `void serialize(IBaseArchive & ar, [const] T & obj)` (global function)
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
	* @sa  class OBaseArchive
	**/
	class IBaseArchive
		{

		public:


			/**
			* Constructor.
			**/
			IBaseArchive() : _buffer(nullptr), _bufsize(0), _nbitem(0), _tempstr()
				{
				}


			/**
			* Destructor. Close the file containing the archive.
			**/
			virtual ~IBaseArchive()
				{
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
			template<typename T> IBaseArchive & operator&(T & obj)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
				internals_serialization::IArchiveHelper<cvT, IBaseArchive>::read(_nbitem, *this, (*p)); // deserialize the object into the archive using the helper class
				return(*this);
				}


			/**
			* deserialize an object in the archive that was previously stored using OBaseArchive::opaque. The
			* memory representing the object is simply copied using a memcpy.
			*
			* @tparam  T   Generic type parameter.
			* @param   obj The object.
			*
			* @return  the archive for chaining.
			**/
			template<typename T> IBaseArchive & opaque(T & obj)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(&obj); // pointer to obj without qualifiers
				if (sizeof(T) == 0) return(*this);
				_nbitem++;
				if (readTokenFromArchive(p, sizeof(T)) != sizeof(obj)) { MTOOLS_THROW("IBaseArchive error (opaque)"); }
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
			template<typename T> IBaseArchive & array(T * pp, size_t len)
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
			template<typename T> IBaseArchive & opaqueArray(T * pp, size_t len)
				{
				typedef mtools::remove_cv_t<T> cvT; // type T but without qualifiers
				cvT * p = const_cast<cvT*>(pp); // pointer without qualifiers
				MTOOLS_ASSERT(p != nullptr);
				if (len * sizeof(T) == 0) return(*this);
				_nbitem++;
				if (readTokenFromArchive(p, sizeof(T)*len) != (sizeof(T)*len)) { MTOOLS_THROW("IBaseArchive error (opaquearray)"); }
				return(*this);
				}


			/** For compatibility with OArchive, does nothing... **/
			template<typename T> IBaseArchive & operator<<(const T & obj) { return(*this); }

			/** For compatibility with OArchive, does nothing... **/
			IBaseArchive & tab(size_t nb = 1) { return(*this); }

			/** For compatibility with OArchive, does nothing... **/
			IBaseArchive & newline(size_t nb = 1) { return(*this); }

			/** For compatibility with OArchive, does nothing... **/
			IBaseArchive & setIndent(unsigned int n = 0) { return(*this); }

			/** For compatibility with OArchive, does nothing... **/
			IBaseArchive & incIndent() { return(*this); }

			/** For compatibility with OArchive, does nothing... **/
			IBaseArchive & decIndent() { return(*this); }


			/**
			* Return the number of items that have been deserialized.
			**/
			uint64 nbItem() const { return _nbitem; }


		protected:

			/**
			* Called when the read buffer must be refilled.
			* The method must return a pointer to the buffer and set len to the number of chars.
			*/
			virtual const char * refill(size_t & len)
				{
				MTOOLS_ERROR("method IBaseArchive::refill() must be overloaded !");
				return nullptr;
				}


		private:

			/* no copy */
			IBaseArchive(const IBaseArchive &) = delete;
			IBaseArchive & operator=(const  IBaseArchive &) = delete;


			/* friend with the helper class */
			template<typename T, typename IARCHIVE> friend class internals_serialization::IArchiveHelper;


			/* read a token and put it in a given buffer. throws if the buffer is too small
			or if there is no more token to read
			return the size of the token. */
			size_t readTokenFromArchive(void * dest_buffer, size_t dest_len)
				{
				size_t nb = _bufsize;
				bool found = findNextToken<IBaseArchive::_refillStatic>(_buffer, nb, this); // find the beginning of the next token
				if (!found) { MTOOLS_THROW("IBaseArchive error, no more token"); } // no more token found
				_buffer  += nb; 
				_bufsize -= nb;
				nb = _bufsize;
				size_t l = readToken<IBaseArchive::_refillStatic>(dest_buffer, dest_len, _buffer, nb, this); // read the token
				if (_buffer == nullptr) { _bufsize = 0; return l; } // ok, but the end of the file was reached
				_buffer += nb;
				_bufsize -= nb;
				return l;
				}


			/* read a token and append it to a given buffer. throws if there is no more token to read
			return the size of the token. */
			size_t readTokenFromArchive(std::string & dest)
				{
				size_t nb = _bufsize;
				bool found = findNextToken<IBaseArchive::_refillStatic>(_buffer, nb, this); // find the beginning of the next token
				if (!found) { MTOOLS_THROW("IBaseArchive error, no more token"); } // no more token found
				_buffer += nb;
				_bufsize -= nb;
				nb = _bufsize;
				size_t l = readToken<IBaseArchive::_refillStatic>(dest, _buffer, nb, this); // read the token
				if (_buffer == nullptr) { _bufsize = 0; return l; } // ok, but the end of the file was reached
				_buffer += nb;
				_bufsize -= nb;
				return l;
				}


			/* static method called when the read buffer must be refilled */
			static const char * _refillStatic(size_t & len, void * data) 
				{ 
				MTOOLS_ASSERT(data != nullptr); 
				return ((IBaseArchive *)data)->_refill(len);
				}

			/* call the virtual refill() method and save the new buffer */
			const char * _refill(size_t & len)
				{
				_buffer = refill(len);
				_bufsize = len;
				return _buffer;
				}

			const char * _buffer;            // read buffer
			size_t		 _bufsize;           // number of char in the read buffer
			uint64		 _nbitem;            // number of item in the archive
			std::string	 _tempstr;           // temporary string used for reconstructing objects (used by IArchiveHelper). 

		};




	/** Class to deserialize from a string created with OStringArchive */
	class IStringArchive : public IBaseArchive
		{

		public: 

			/**
			 * Constructor.
			 *
			 * @param	str	The string to deserialize.
			 * 				
			 * The string must remain accessible until the object is finished deserializing.
			 **/
			IStringArchive(const std::string & str) : _buf(str.c_str()), _len(str.size()) { }


			/**
			 * Constructor.
			 *
			 * @param	buf	The buffer to deserialize.
			 * @param	len	The length of the buffer. 
			 * 				
			 * The buffer must remain accessible until the object is finished deserializing. 
			 **/
			IStringArchive(const char * buf, size_t len) : _buf(buf), _len(len)	{ }


			virtual ~IStringArchive() {}


		protected:

			virtual const char * refill(size_t & len) override 
				{ 
				if (_len) { len = _len; _len = 0; return _buf; }
				len = 0;
				return nullptr; 
				}

		private:

			const char * _buf;
			size_t _len; 

		};


	

	/** Class to deserialize an object of type const p_char[] created with OCPPArchive. */
	class ICPPArchive : public IBaseArchive
		{

		public: 

			ICPPArchive(const cp_char obj[] );

			virtual ~ICPPArchive() {}

			/* for debug purpose, return the buffer */
			const std::string buffer() const { return _buf; }

		protected:

			virtual const char * refill(size_t & len) override 
				{
				if (_firsttime) { _firsttime = false; len = _buf.size(); return _buf.data(); }
				len = 0;
				return nullptr;
				}

		private:

			std::string _buf;
			bool _firsttime;

		};




	/** Class to deserialize from a file create with OFileArchive. */
	class IFileArchive : public IBaseArchive
		{

		public:

			IFileArchive(const std::string & filename); 

			virtual ~IFileArchive();

		protected: 

			virtual const char * refill(size_t & len) override { return _readfile(len); }

		private:

			void _openfile();
			void _closefile();
			const char * _readfile(size_t & len);

			static const size_t FILEBUFFERSIZE = 512000;
			static const size_t GZIPBUFFERSIZE = 512000;

			char * _filebuffer;             // read buffer
			void * _handle;                 // gzfile handle
			std::string _filename;          // name of the archive file

		};





    }



#if defined (_MSC_VER)
#pragma warning( pop )
#endif


 /* end of file */

