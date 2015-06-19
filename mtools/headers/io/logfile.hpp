/** @file logfile.hpp */
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



#include "misc/stringfct.hpp"

#include <fstream>
#include <string>


namespace mtools
{


    /**
     * Log file class.
     *
     * Write something into a file using << operator.
     **/
    class LogFile
    {
    public:

        /**
         * Constructor. Creates the log file.
         *
         * @param   fname       Name of the file.
         * @param   append      True if an already existing file should be appended. False to truncate it
         *                      (default true).
         * @param   writeheader True to write a header with the date at the beginning of the file
         *                      (default true).
         * @param   wenc        (default enc_iso8859). The encoding to use when writing wide chars.
         *                      Regular char strings are written in raw form without encoding conversion.
         **/
        LogFile(const std::string & fname, bool append = true, bool writeheader = true, StringEncoding wenc = enc_iso8859);


        /** Destructor. */
        ~LogFile();


        /**
         * Write something into the log file. Uses toString for converting v into a string. If v is
         * composed of wide chars (wchar, wchar * or wstring) the encoding used depends on the wenc
         * parameter passed to the constructor (default is ISO8859-1).
         * @param   v   the data to write.
         * @sa  toString
         **/
        template<class T> LogFile & operator<<(const T & v)
            {
            _m_log << toString(v, _m_wenc);
            _m_log.flush();
            return(*this);
            }


        /**
         * Get the filename.
         * @return  the name of the log file.
         **/
        std::string filename() const { return _m_filename; }

    private:


        /** write the header of the file */
        void _writeheader();

        LogFile(const LogFile &) = delete;
        LogFile(LogFile &&) = delete;
        LogFile & operator=(const LogFile &) = delete;
        LogFile & operator=(LogFile &&) = delete;

        std::string _m_filename;        ///< name of the log file.
        StringEncoding _m_wenc;         ///< encoding format used for writing wide strings to file.
        std::ofstream _m_log;           ///< the log file stream.

    };


}


/* end of file */

