/** @file fileio.cpp */
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
#include "io/fileio.hpp"


#if defined (_MSC_VER)
#include <windows.h>
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif


namespace mtools
{


    bool doFileExist(std::string filename)
        {
        FILE *file = fopen(filename.c_str(), "r");
        if (file != nullptr) {fclose(file); return true;} else { return false; }
        }


    std::string loadStringFromFile(const std::string & filename, StringEncoding enc)
        {
        std::ifstream t(filename);
        if (t.fail()) { return ""; }
        t.seekg(0, std::ios::end);
        std::string s;
        s.reserve(static_cast<size_t>(t.tellg()));
        t.seekg(0, std::ios::beg);
        s.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        t.close();
        if (enc == enc_utf8)    { return toUtf8(s); }
        if (enc == enc_iso8859) { return toIso8859(s); }
        return s;
        }


    bool saveStringToFile(const std::string & filename, const std::string & s, bool append, StringEncoding enc)
        {
        std::ofstream of(filename, std::ofstream::out | (append ? std::ofstream::app : std::ofstream::trunc));
        if (of.fail()) { return false; }
        if (enc == enc_utf8)    { of << toUtf8(s); }
        if (enc == enc_iso8859) { of << toIso8859(s); }
        if (enc == enc_unknown) { of << s; }
        if (of.fail() || of.bad()) { return false; }
        of.close();
        return true;
        }


    size_t replaceInFile(const std::string & filename, const std::string & oldstr, const std::string & newstr)
        {
        std::string s = loadStringFromFile(filename);
        size_t r = replace(s, oldstr, newstr);
        if (r) { saveStringToFile(filename, s, false); }
        return r;
        }


    bool matchFileMask(std::string filename, std::string mask,bool case_sensitive)
        {
        if (!case_sensitive) { filename = toLowerCase(filename); mask = toLowerCase(mask); }
        if (!mask.length()) { return false; }
        size_t pos = 0; while ((pos < mask.length()) && (mask[pos] == '*')) { pos++; }
        if (pos > 1) { mask = mask.substr(pos - 1, std::string::npos); }
        if (!mask.compare("*")) { return true; }
        if (!filename.length()) { return false; }
        pos = mask.find("|");
        if (pos != std::string::npos)
            {
            if (pos) { if (matchFileMask(filename, mask.substr(0, pos), case_sensitive)) { return true; } }
            if (pos < mask.length() - 1) { if (matchFileMask(filename, mask.substr(pos + 1, std::string::npos), case_sensitive)) { return true; } }
            return false;
            }
        pos = mask.find("*");
        if (pos == std::string::npos) { if (!filename.compare(mask)) { return true; } return false; }
        if (pos != 0)
            {
            if (filename.find(mask.substr(0, pos)) != 0) { return false; }
            return matchFileMask(filename.substr(pos, std::string::npos), mask.substr(pos, std::string::npos), case_sensitive);
            }
        mask = mask.substr(1, std::string::npos);
        for (size_t i = 0; i < filename.length(); i++) { if ((filename[i] == mask[0]) && (matchFileMask(filename.substr(i, std::string::npos), mask, case_sensitive))) return true; }
        return false;
        }


    bool matchFileMask(std::wstring filename, std::wstring mask, bool case_sensitive)
        {
        if (!case_sensitive) { filename = toLowerCase(filename); mask = toLowerCase(mask); }
        if (!mask.length()) { return false; }
        size_t pos = 0; while ((pos < mask.length()) && (mask[pos] == L'*')) { pos++; }
        if (pos > 1) { mask = mask.substr(pos - 1, std::wstring::npos); }
        if (!mask.compare(L"*")) { return true; }
        if (!filename.length()) { return false; }
        pos = mask.find(L"|");
        if (pos != std::wstring::npos)
            {
            if (pos) { if (matchFileMask(filename, mask.substr(0, pos), case_sensitive)) { return true; } }
            if (pos < mask.length() - 1) { if (matchFileMask(filename, mask.substr(pos + 1, std::wstring::npos)), case_sensitive) { return true; } }
            return false;
            }
        pos = mask.find(L"*");
        if (pos == std::wstring::npos) { if (!filename.compare(mask)) { return true; } return false; }
        if (pos != 0)
            {
            if (filename.find(mask.substr(0, pos)) != 0) { return false; }
            return matchFileMask(filename.substr(pos, std::wstring::npos), mask.substr(pos, std::wstring::npos), case_sensitive);
            }
        mask = mask.substr(1, std::wstring::npos);
        for (size_t i = 0; i < filename.length(); i++) { if ((filename[i] == mask[0]) && (matchFileMask(filename.substr(i, std::wstring::npos), mask, case_sensitive))) return true; }
        return false;
        }


    bool getFileList(std::string path, std::string mask, bool case_sensitive, std::vector<std::string> & tabfiles, bool rec, bool addFiles, bool addDir)
        {
        path = mtools::toUtf8(mtools::trailingSlash(path, true));
        dirent** list = nullptr;
        int nb = fl_filename_list(path.c_str(), &list);
        if (nb < 0) return false;
        for (int i = 0; i < nb; i++)
            {
            std::string f = mtools::toIso8859(list[i]->d_name);
            if (( f == std::string("./")) || (f == std::string("../"))) continue;

            if (!fl_filename_isdir((path + list[i]->d_name).c_str()))
                {
                if ((addFiles) && (mtools::matchFileMask(f, mask,case_sensitive))) tabfiles.push_back(f);
                }
            else
                {
                if (addDir) tabfiles.push_back(f);
                if (rec)
                    {
                    size_t pos = tabfiles.size();
                    if (!getFileList(path + list[i]->d_name, mask, case_sensitive, tabfiles, rec, addFiles, addDir)) { fl_filename_free_list(&list, nb); return false; }
                    for (size_t j = pos; j < tabfiles.size(); j++) { tabfiles[j] = f + tabfiles[j]; }
                    }
                }
            }
        fl_filename_free_list(&list, nb);
        return true;
        }


    bool getFileList(std::wstring & path, std::wstring & mask, bool case_sensitive, std::vector<std::wstring> & tabfiles, bool rec, bool addFiles, bool addDir)
        {
        std::vector<std::string> tab;
        bool r = getFileList(toString(path,enc_iso8859), toString(mask,enc_iso8859), case_sensitive, tab, rec, addFiles, addDir);
        for (auto it = tab.begin(); it != tab.end(); it++) { tabfiles.push_back(toWString(*it,enc_iso8859)); }
        return r;
        }


    size_t copyFiles(std::wstring sourcepath, std::wstring destpath, std::wstring mask, bool case_sensitive, bool rec, bool overwrite)
        {
        #if defined (_MSC_VER)
        int copied = 0;
        if ((sourcepath.length() != 0) && (sourcepath[sourcepath.length() - 1] != L'\\'))  { sourcepath += L"\\"; }
        if ((destpath.length() != 0) && (destpath[destpath.length() - 1] != L'\\'))        { destpath += L"\\"; }
        std::vector<std::wstring> tabsource;
        if (getFileList(sourcepath, mask, case_sensitive, tabsource, rec, true, rec) == false) { return(-1); }
        for (std::vector<std::wstring>::iterator it = tabsource.begin(); it != tabsource.end(); it++)
            {
            if ((it->length() != 0) && ((*it)[it->length() - 1] == L'\\'))
                {
                if (CreateDirectoryW((destpath + (*it)).c_str(), NULL) == 0) { if (GetLastError() != ERROR_ALREADY_EXISTS) { return -1; } }
                }
            else
                {
                bool fie = false; if (overwrite == false) { fie = true; }
                if (CopyFileW((sourcepath + (*it)).c_str(), (destpath + (*it)).c_str(), fie) == 0) { return -1; }
                }
            copied++;
            }
        return copied;
        #else
        // TODO
        #endif
        return -1;
        }


    size_t copyFiles(std::string sourcepath, std::string destpath, std::string mask, bool case_sensitive, bool rec, bool overwrite)
        {
        return copyFiles(toWString(sourcepath, enc_iso8859), toWString(destpath, enc_iso8859), toWString(mask, enc_iso8859), case_sensitive, rec, overwrite);
        }


    std::string extractPath(const std::string & pathname, bool withslash)
        {
        size_t pos = pathname.find_last_of('\\');
        if (pos == std::string::npos) { return (withslash ? "\\" : ""); }
        return pathname.substr(0, (withslash ? pos + 1 : pos));
        }


    std::wstring extractPath(const std::wstring & pathname, bool withslash)
        {
        size_t pos = pathname.find_last_of(L'\\');
        if (pos == std::wstring::npos) { return (withslash ? L"\\" : L""); }
        return pathname.substr(0, (withslash ? pos + 1 : pos));
        }


    std::string extractName(const std::string & pathname)
        {
        size_t pos = pathname.find_last_of('\\');
        if (pos == std::string::npos) { return pathname; }
        if (pos == pathname.length() - 1) { return ""; }
        return(pathname.substr(pos + 1, std::string::npos));
        }


    std::wstring extractName(const std::wstring & pathname)
        {
        size_t pos = pathname.find_last_of(L'\\');
        if (pos == std::wstring::npos) { return pathname; }
        if (pos == pathname.length() - 1) { return L""; }
        return(pathname.substr(pos + 1, std::wstring::npos));
        }


    std::string extractExtension(const std::string & pathname)
        {
        std::string r = extractName(pathname);
        size_t pos = r.find_last_of('.');
        if (pos == std::string::npos) { return (""); }
        return r.substr(pos + 1, std::string::npos);
        }


    std::wstring extractExtension(const std::wstring & pathname)
        {
        std::wstring r = extractName(pathname);
        size_t pos = r.find_last_of(L'.');
        if (pos == std::wstring::npos) { return (L""); }
        return r.substr(pos + 1, std::wstring::npos);
        }


    std::string extractNameWithoutExtension(const std::string & pathname)
        {
        std::string r = extractName(pathname);
        size_t pos = r.find_last_of('.');
        if (pos == std::string::npos) { return r; }
        return r.substr(0, pos);
        }


    std::wstring extractNameWithoutExtension(const std::wstring & pathname)
        {
        std::wstring r = extractName(pathname);
        size_t pos = r.find_last_of(L'.');
        if (pos == std::wstring::npos) { return r; }
        return r.substr(0, pos);
        }


    std::string changeExtension(const std::string & pathname, const std::string & ext)
        {
        std::string path = extractPath(pathname, true); if (path == std::string("\\")) { path = ""; }
        std::string name = extractNameWithoutExtension(pathname);
        return(path + name + "." + ext);
        }


    std::wstring changeExtension(const std::wstring & pathname, const std::wstring & ext)
        {
        std::wstring path = extractPath(pathname, true); if (path == std::wstring(L"\\")) { path = L""; }
        std::wstring name = extractNameWithoutExtension(pathname);
        return(path + name + L"." + ext);
        }


    std::string trailingSlash(const std::string & str, bool withslash)
        {
        if (withslash) { if ((str.length() != 0) && (str[str.length() - 1] == '\\')) { return str; } return(str + '\\'); }
        if ((str.length() != 0) && (str[str.length() - 1] == '\\')) { return(str.substr(0, str.length() - 1)); }
        return str;
        }


    std::wstring trailingSlash(const std::wstring & str, bool withslash)
        {
        if (withslash) { if ((str.length() != 0) && (str[str.length() - 1] == L'\\')) { return str; } return(str + L'\\'); }
        if ((str.length() != 0) && (str[str.length() - 1] == L'\\')) { return(str.substr(0, str.length() - 1)); }
        return str;
        }


}

#if defined (_MSC_VER)
#pragma warning( pop )
#endif


/* end of file */


