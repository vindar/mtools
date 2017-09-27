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

#include "zlib.h"       // fltk zlib


namespace mtools
	{


	void OArchive::_openFile()
		{
		if (_compress)
			{
			_gzhandle = gzopen(_filename.c_str(), "wb4");
			if (_gzhandle == nullptr) { MTOOLS_THROW("OArchive error (openfile 1)"); }
			if (gzbuffer((gzFile)_gzhandle, GZIPBUFFERSIZE) != 0) { MTOOLS_THROW("OArchive error (openfile 2)"); }
			return;
			}
		_handle = fopen(_filename.c_str(), "wb");
		if (_handle == nullptr) { MTOOLS_THROW("OArchive error (openfile 3)"); }
		return;
		}


	void OArchive::_closeFile()
		{
		newline();
		_flush(true);
		if (_compress) { if (gzclose((gzFile)_gzhandle) != Z_OK) { MTOOLS_THROW("OArchive error (closefile 1)"); } return; }
		if (fclose(_handle) != 0) { MTOOLS_THROW("OArchive error (closefile 2)"); }
		return;
		}


	void OArchive::_flush(bool force)
		{
		if ((force) || (_writeBuffer.length() > WRITEBUFFERSIZE))
			{ // ok we do flush
			if (_compress)
				{
				if ((unsigned int)gzwrite((gzFile)_gzhandle, _writeBuffer.c_str(), (unsigned int)_writeBuffer.length()) != _writeBuffer.length()) { MTOOLS_THROW("OArchive error (_flush 1)"); }
				}
			else
				{
				if (fwrite(_writeBuffer.c_str(), 1, _writeBuffer.length(), _handle) != _writeBuffer.length()) { MTOOLS_THROW("OArchive error (_flush 2)"); }
				}
			_writeBuffer.clear();
			}
		}



	const char * IArchive::refill(size_t & len)
		{
		int l = gzread((gzFile)_gzhandle, _readBuffer, READBUFFERSIZE);
		if (l < 0) { MTOOLS_THROW("IArchive error"); } // something went wrong
		_readSize = (size_t)l; // save the new number of char in the buffer
		len = _readSize;
		if (l == 0) { return nullptr; } // end of file
		return _readBuffer; // ok
		}


	void IArchive::_openFile()
		{
		_gzhandle = gzopen(_filename.c_str(), "rb");
		if (_gzhandle == nullptr) { MTOOLS_THROW("IArchive error"); }
		if (gzbuffer((gzFile)_gzhandle, GZIPBUFFERSIZE) != 0) { MTOOLS_THROW("IArchive error"); }
		return;
		}


	void IArchive::_closeFile()
		{
		if (gzclose((gzFile)_gzhandle) != Z_OK) { MTOOLS_THROW("IArchive error"); }
		return;
		}


	}


/* end of file */


