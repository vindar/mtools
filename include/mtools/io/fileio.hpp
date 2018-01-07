/** @file fileio.hpp */
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

#include "../misc/internal/mtools_export.hpp"

#include <string>
#include <vector>


namespace mtools
{
    /**
     * Check if a file exist (more preciseley, test if it can be open...)
     *
     * @param   filename    file to check
     *
     * @return  true if it exist, false otherwise
     **/
    bool doFileExist(std::string filename);

    /**
     * Loads a text file into a string.
     *
     * @param   filename    Name of the file.
     * @param   enc         The encoding format for the returned string  (default = enc_unknown).
     *                      | enc         | meaning                                |
     *                      |-------------|----------------------------------------|
     *                      | enc_unknown | no conversion from file, raw text      |
     *                      | enc_iso8859 | convert the string to ISO8859-1 format |
     *                      | enc_utf8    | convert the string to UTF-8 format     |
     *                      |             |                                        |
     *
     * @return  The text file in the required format (or an empty string if error).
     **/
    std::string loadStringFromFile(const std::string & filename, StringEncoding enc = enc_unknown);


    /**
     * Saves a string into a text file.
     *
     * @param   filename    Name of the file.
     * @param   s           the string to save.
     * @param   append      Set to true to append the file if is already exist and sto false to
     *                      truncate an existing file (default).
     * @param   enc         The encoding format to convert the string into before writing it on file
     *                      (default = enc_unknown).
     *                              | enc         | meaning                                |
     *                              |-------------|----------------------------------------|
     *                              | enc_unknown | no conversion, raw text                |
     *                              | enc_iso8859 | save to file in ISO8859-1 format       |
     *                              | enc_utf8    | save to file in UTF-8 format           |
     *                              |             |                                        |
     *
     * @return  true if it succeeds, false if it fails.
     **/
    bool saveStringToFile(const std::string & filename, const std::string & s, bool append = false, StringEncoding enc = enc_unknown);


    /**
     * Replace all occurence of oldstr by newstr in the file name filename. Does a linear search :
     * continu the search just after the previous replacement.
     *
     * @param   filename    Name of the file where the replacement will take place.
     * @param   oldstr      The old string to look for.
     * @param   newstr      The new string to replace with.
     *
     * @return  number of replacement done.
     *
     * @sa  replace
     **/
    size_t replaceInFile(const std::string & filename, const std::string & oldstr, const std::string & newstr);


    /**
     * Check whether a filename match a specific mask. The mask may contain the special symbol '*'
     * (replace any sequence of chars) and '|' (logical or). For example mask = "*.hpp|*.h" for
     * checking for header file.
     *
     * @param   filename        filename to check against mask.
     * @param   mask            the mask.
     * @param   case_sensitive  true to perform case sensitive comparison when matching the
     *                          mask and false to use case insentive comparisons.
     *
     * @return  true if the name match the mask, false otherwise.
     **/
    bool matchFileMask(std::string filename, std::string mask, bool case_sensitive = true);


    /** matchFileMask, wstring version */
    bool matchFileMask(std::wstring filename, std::wstring mask, bool case_sensitive = true);


    /**
     * creates a vector of the files in a given path. Returns a list of of the files/directory in a
     * given path, matching a given mask.
     * 
     * - the vector tabfiles created contain the name of the files/directories found, relatively to
     * path. i.e. in order to get the complete path, one should append the path ending with "/"  to
     * each name in the vector.
     * 
     * - The returned vector tabfiles do not contain the subdirectories  "." and "..".  
     * 
     * - the mask criterion is only applied to files (not directory names). Uses the matchFileMask()
     * method.
     * 
     * - the vector returns is not ordered in a specific way but guarantees that the directories (if
     * listed) always appear before their contents.
     * 
     * - all strings are in ISO8859-1 encoding.
     *
     * @param   path                the path to look into (may or may not end with '\', and may be
     *                              relative e.g. "..").
     * @param   mask                see MatchFileMask.
     * @param   case_sensitive      true to perform case sensitive comparison when matching
     *                              the mask and false to use case insentive comparisons.
     * @param [in,out]  tabfiles    the vector where the list is returned, it is NOT emptied first.
     * @param   rec                 Set to true for recursive search inside sub-directories.
     * @param   addFiles            Set to true if the files found should be added in the vector.
     * @param   addDir              set to true is the directories should be added to the vector (in
     *                              this case, they end by '\'). Remember that the mask is not used for
     *                              directories.
     *
     * @return  true if successful , false if an error occured.
     **/
    bool getFileList(std::string path, std::string mask, bool case_sensitive, std::vector<std::string> & tabfiles, bool rec, bool addFiles, bool addDir);


    /** getFileList, std::wstring version */
    bool getFileList(std::wstring & path, std::wstring & mask, bool case_sensitive, std::vector<std::wstring> & tabfiles, bool rec, bool addFiles, bool addDir);


    /**
     * Copy all the file matching a pattern from one directory to another. All stirng are in iso-
     * 8859-1 encoding.
     * 
     * @warning Implemented only for MS Windows. Return -1 (failure) on all other OSes.
     *
     * @param   sourcepath      the source path from which files are to be copied.
     * @param   destpath        the destination path to where ther file are copied.
     * @param   mask            see matchFileMask().
     * @param   case_sensitive  true to perform case sensitive comparison when matching the
     *                          mask and false to use case insentive comparisons.
     * @param   rec             true to recursively copy the subdirectories.
     * @param   overwrite       true to overwrite existing file. If set to false, fails if a file
     *                          already exist in the destination.
     *
     * @return  the number of files/directories copied or -1 if error.
     **/
    size_t copyFiles(std::string sourcepath, std::string destpath, std::string mask, bool case_sensitive, bool rec, bool overwrite);


    /** copyFiles, wstring specialization  */
    size_t copyFiles(std::wstring sourcepath, std::wstring destpath, std::wstring mask, bool case_sensitive, bool rec, bool overwrite);


    /**
     * Return only the path part of a string. If withslash is true, the path returned ends with a
     * backslash ie "aaa/bbb/ccc",false  -> "aaa/bbb" and "aaa/bbb/ccc",true   -> "aaa/bbb/".
     *
     * @param   pathname    string with path+name.
     * @param   withslash   should the path end with a backslash.
     *
     * @return  The extracted path.
     **/
    std::string extractPath(const std::string & pathname, bool withslash);


    /** extractPath, swtring specialization */
    std::wstring extractPath(const std::wstring & pathname, bool withslash);


    /**
     * Extract the filename of a path+name string. Ex: "/aaa/bbb/ccc" -> "ccc".
     *
     * @param   pathname    the path+name string.
     *
     * @return  The extracted name.
     **/
    std::string extractName(const std::string & pathname);


    /** extractName, wstring specialization */
    std::wstring extractName(const std::wstring & pathname);


    /**
     * Extract the extension of a filename. Examples:
     * 
     * | pathname          | result |
     * |-------------------|--------|
     * |"/a/path/file.ext" | "ext"  |
     * |"/path/file"       | ""     |
     * |"file.ext1.ext2"   | "ext2" |
     * |                   |        |
     * 
     * @param   pathname    the path+name string.
     *
     * @return  The extracted name.
     **/
    std::string extractExtension(const std::string & pathname);


    /** extractExtension, wstring specialization */
    std::wstring extractExtension(const std::wstring & pathname);


    /**
     * extract the file name from a path+name without its (last) extension. Examples:
     * 
     * | pathname          | result          |
     * |-------------------|-----------------|
     * |"/a/path/file.ext" | "file"          |
     * |"/path/file2"      | "file2"         |
     * |"/path/subpath/"   | ""              |
     * |"file.ext1.ext2"   | "file.ext1.ext2 |
     * |                   |                 |
     *
     * @param   pathname    pathname of the file.
     *
     * @return  The extracted name without its (last) extension.
     **/
    std::string extractNameWithoutExtension(const std::string & pathname);


    /** extractNameWithoutExtension, string specialization */
    std::wstring extractNameWithoutExtension(const std::wstring & pathname);


    /**
     * Change the (last) extension of a filename, append it if no previous extension.
     * 
     * | pathname          | ext   | result             |
     * |-------------------|-------|--------------------|
     * |"/a/path/file.ext" | "bmp" | "/a/path/file.bmp" |
     * |"/path/file2"      | "b.d" | "/path/file2.b.d"  |
     * |"file.ext1.ext2"   | "bmp" | "file.ext1.bmp"    |
     * |                   |       |                    |
     *
     * @param   pathname    path name of the file.
     * @param   ext         the new extension.
     *
     * @return  the pathname with the file's last extension changed to newextension.
     **/
    std::string changeExtension(const std::string & pathname, const std::string & ext);


    /** changeExtension, string specialization */
    std::wstring changeExtension(const std::wstring & pathname, const std::wstring & ext);


    /**
     * Return the string but with or without a backslash at the end depending on the value of
     * withslash.
     *
     * @param   str         the string to add/remove trailing slash.
     * @param   withslash   true to add slash and false to remove.
     *
     * @return  The resulting string.
     **/
    std::string trailingSlash(const std::string & str, bool withslash);


    /** trailingSlash, wstring specialization */
    std::wstring trailingSlash(const std::wstring & str, bool withslash);


}


/* end of file fileio.h */


