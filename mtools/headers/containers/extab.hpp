/** @file extab.hpp */
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
#include "../misc/error.hpp"
#include "../misc/stringfct.hpp"
#include "../misc/misc.hpp"
#include "../io/serialization.hpp"

#include <string>

namespace mtools
{

// TODO
template<class T> class ExTab
    {
     
public:

    ExTab(size_t L);
    ExTab(const std::string & filename);
	ExTab(mtools::IArchive & A);
    ~ExTab();

	void Save(const std::string & filename) const;
	void Reset();
    void serialize(mtools::OArchive & ar);
    void deserialize(mtools::IArchive & ar);
    inline void Add(const T & val);

    inline T medV(uint64 pos) const;
    inline T medV(double pos) const;
    inline T maxV(uint64 pos) const;
    inline T maxV(double pos) const;
    inline T minV(uint64 pos) const;
    inline T minV(double pos) const;
    inline int64 NbEntries() const;

    double maxV() const;
	double minV() const;
    std::string toString() const;
    void append(const ExTab<T> & tab);
    void append(const T * tab,size_t len);
    void operator+=(const ExTab<T> & tab);
    void operator-=(const ExTab<T> & tab);
    void operator*=(const ExTab & tab);
    void operator/=(const ExTab & tab);
    void operator+=(double x);
    void operator-=(double x);
    void operator*=(double x);
    void operator/=(double x);
 


private:

	// no copy allowed
	ExTab(const ExTab &);
	ExTab & operator=(const ExTab &);

};


}


/* end of file */















