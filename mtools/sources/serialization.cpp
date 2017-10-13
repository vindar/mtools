/** @file serialization.cpp */
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

#include "io/serialization.hpp"

#include <zlib.h>       // fltk zlib


namespace mtools
	{



	OFileArchive::OFileArchive(const std::string & filename) : OBaseArchive(), _filename(filename), _compress(false), _handle(nullptr)
		{
		std::string ext = toLowerCase(extractExtension(filename));
		if ((ext == std::string("gz")) || (ext == std::string("gzip")) || (ext == std::string("z"))) { _compress = true; }
		_openFile();
		header();
		}


	OFileArchive::~OFileArchive()
		{
		footer();
		_closeFile();
		}



	void OFileArchive::_openFile()
		{
		if (_compress)
			{
			_handle = gzopen(_filename.c_str(), "wb4");
			if (_handle == nullptr) { MTOOLS_THROW("OFileArchive error (openfile 1)"); }
			if (gzbuffer((gzFile)_handle, GZIPBUFFERSIZE) != 0) { MTOOLS_THROW("OFileArchive error (openfile 2)"); }
			return;
			}
		#if defined (_MSC_VER) 
		#pragma warning( push )				
		#pragma warning( disable : 4996 )	
		#endif
		_handle = fopen(_filename.c_str(), "wb");
		#if defined (_MSC_VER) 
		#pragma warning( pop )
		#endif
		if (_handle == nullptr) { MTOOLS_THROW("OFileArchive error (openfile 3)"); }
		return;
		}


	void OFileArchive::_closeFile()
		{
		newline();
		_write(getbuffer(),true); // flush 
		if (_compress) { if (gzclose((gzFile)_handle) != Z_OK) { MTOOLS_THROW("OFileArchive error (closefile 1)"); } return; }
		if (fclose((FILE*)_handle) != 0) { MTOOLS_THROW("OFileArchive error (closefile 2)"); }
		return;
		}


	void OFileArchive::_write(std::string & buffer, bool force)
		{
		if ((force) || (buffer.length() > WRITEBUFFERSIZE))
			{ // ok we do flush
			if (_compress)
				{
				if ((unsigned int)gzwrite((gzFile)_handle, buffer.c_str(), (unsigned int)buffer.length()) != buffer.length()) { MTOOLS_THROW("OFileArchive error (_flush 1)"); }
				}
			else
				{
				if (fwrite(buffer.c_str(), 1, buffer.length(), (FILE*)_handle) != buffer.length()) { MTOOLS_THROW("OFileArchive error (_flush 2)"); }
				}
			buffer.clear();
			}
		}



	IFileArchive::IFileArchive(const std::string & filename) : IBaseArchive(), _filebuffer(nullptr), _handle(nullptr), _filename(filename)
		{
		_filebuffer = new char[FILEBUFFERSIZE];
		_openfile();
		}


	IFileArchive::~IFileArchive()
		{
		_closefile();
		delete[] _filebuffer;
		}


	void IFileArchive::_openfile()
		{
		_handle = gzopen(_filename.c_str(), "rb");
		if (_handle == nullptr) { MTOOLS_THROW("IFileArchive::_openfile() error 1"); }
		if (gzbuffer((gzFile)_handle, GZIPBUFFERSIZE) != 0) { MTOOLS_THROW("IFileArchive::_openfile() error 2"); }
		return;
		}


	void IFileArchive::_closefile()
		{
		if (gzclose((gzFile)_handle) != Z_OK) { MTOOLS_THROW("IFileArchive::_closeFile() error"); }
		return;
		}

	const char * IFileArchive::_readfile(size_t & len)
		{
		int l = gzread((gzFile)_handle, _filebuffer, FILEBUFFERSIZE);
		if (l < 0) { MTOOLS_THROW("IFileArchive::_readfile() error"); } // something went wrong
		len = (size_t)l; // number of char in the buffer
		if (len == 0) return nullptr;
		return _filebuffer;
		}



	std::string OCPPArchive::get() const 
		{
		const size_t CHUNKSIZE = 64;
		const char * src = getbuffer().data();
		const size_t src_len = getbuffer().size();
		std::string dest(src_len + 128, ' ');
		char * dst = &(dest[0]);
		size_t dst_len = dest.size();
		z_stream defstream;
		defstream.zalloc = Z_NULL;
		defstream.zfree = Z_NULL;
		defstream.opaque = Z_NULL;
		defstream.avail_in = (uInt)src_len;
		defstream.next_in = (Bytef *)src;
		defstream.avail_out = (uInt)dst_len;
		defstream.next_out = (Bytef *)dst;
		deflateInit(&defstream, Z_BEST_COMPRESSION);
		deflate(&defstream, Z_FINISH);
		deflateEnd(&defstream);
		dst_len = defstream.total_out;
		std::vector<std::string> tab;
		while (dst_len >= CHUNKSIZE)
			{
			tab.push_back(mtools::memoryToString(dst, CHUNKSIZE));
			dst += CHUNKSIZE;
			dst_len -= CHUNKSIZE;
			}
		if (dst_len > 0) { tab.push_back(mtools::memoryToString(dst, dst_len)); }
		std::string res("const p_char ");
		res += _name + "[" + mtools::toString(tab.size() + 2) + "] = { ";
		res += "\"" + mtools::toString(tab.size()) + "\", \"" + mtools::toString(src_len)  + "\"";
		for (size_t i = 0; i < tab.size(); i++)
			{
			res += ",\n\"" + tab[i] + "\"";
			}
		res += "\n};\n";
		return res; 
		}


	ICPPArchive::ICPPArchive(const cp_char obj[]) : _buf(), _firsttime(true)
		{
		size_t tabsize, src_len;
		mtools::fromString(obj[0], tabsize);
		mtools::fromString(obj[1], src_len);
		_buf.resize(src_len,' ');
		std::string tmp;
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.total_in = 0;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		strm.total_out = 0;
		strm.avail_out = (uInt)src_len;
		strm.next_out = (Bytef *)&_buf[0];
		inflateInit(&strm);
		for (size_t k = 0; k < tabsize; k++)
			{
			size_t l = (strlen(obj[k + 2]) >> 1);
			tmp.resize(l);
			stringToMemory(obj[k + 2], &(tmp[0]));
			strm.avail_in = (uInt)l;
			strm.next_in = (Bytef *)(&(tmp[0]));
			inflate(&strm, Z_NO_FLUSH);
			}
		inflateEnd(&strm);
		}


	}


/* end of file */


