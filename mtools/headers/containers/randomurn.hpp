/** @file randomurn.hpp */
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


/* headers */
#include <string>
#include <vector>

#include "../misc/stringfct.hpp"
#include "../misc/misc.hpp"
#include "../misc/error.hpp"

namespace mtools
{


    /**
     * A random urn container. Element can be added and removed from the urn. It is possible to pick
     * an element at random via operator() by providing a uniform random number in [0,1[.
     *
     * @tparam  T   Type of object that the urn contains.
     **/
    template<typename T> class RandomUrn
    {
    public:

        /**
         * Default constructor. An empty urn
         **/
        RandomUrn() {}


        /**
         * Constructor. Load the urn from a file. Throw if error.
         * Throws if error.
         * 
         * @param   filename    name of the file.
         **/
        RandomUrn(const std::string & filename) { load(filename); }


        /**
         * Loads from a file. The current content of the urn is discarded. Throws if eror.
         *
         * @param   filename    name o the file.
         **/
        void load(const std::string & filename)
            {
            _tab.clear();
            IArchive ar(filename);
            ar & (*this);
            }


        /**
         * Saves the urn into a file. Throws if error.
         * Use .z or .gz to save in compressed format.
         * 
         * @param   filename    name of the file.
         **/
        void save(const std::string & filename)
            {
            OArchive ar(filename);
            ar & (*this);
            }


        /**
         * Number of elements in the urn.
         *
         * @return  The current number of elements in the urn
         **/
        inline size_t size() const { return _tab.size(); }


        /**
         * Access an element according to its index in the urn.
         * 
         * @warning The reference is invalidated after a call to insert(), remove() or clear().
         *
         * @param   pos The position between 0 and Urn.size()-1.
         *
         * @return  A reference to the corresponding element.
         **/
        inline T & operator[](size_t pos)
            {
            MTOOLS_ASSERT(pos < Urn.size());
            return _tab[pos];
            }


        /**
         * Access the element associated with a value in [0,1[. Useful for choosing an element a random
         * given a uniform random number.
         * 
         * @warning The reference is invalidated after a call to insert(), remove() or clear().
         *
         * @param   v   The double in [0,1[.
         *
         * @return  The corresponding element obtained by linear interpolation.
         **/
        inline T & operator()(double v)
            {
            MTOOLS_ASSERT(((v >= 0.0) && (v<1.0)));
            size_t n = (size_t)(v*_tab.size());
            MTOOLS_ASSERT(n < _tab.size());
            return _tab[n];
            }


        /**
         * Inserts an element in the Urn.
         *
         * @param   obj The object to insert
         *
         * @return  A reference to the object inside the urn.
         **/
        inline T & insert(const T & obj)
            {
            _tab.emplace_back(obj);
            return _tab.back();
            }


        /**
         * Removes an element from the urn.
         *
         * @param   obj The object to remove.
         **/
        inline void remove(const T & obj)
            {
            auto index = (&obj) - _tab.data();
            MTOOLS_ASSERT(((index >= 0) && (index < (int64)_tab.size())));
            if ((size_t)(index + 1) < _tab.size()) { _tab[index] = _tab.back(); }
            _tab.pop_back();
            }


        /**
         * Remove every elements in the urn, leaving it empty.
         **/
        void clear() { _tab.clear(); }


        /**
         * Print information about the urn into a string.
         *
         * @return  A std::string that represents this object.
         **/
        std::string toString(bool debug = false) const
            {
            return std::string("RandomUrn<") + typeid(T).name() + "> size: " + mtools::toString(size()) + " (" + toStringMemSize(memoryUsed()) + " / " + toStringMemSize(memoryAllocated())  + ")" + (debug ? std::string("\n") + mtools::toString(_tab) : std::string(""));
            }


        /**
         * Memory used by the urn.
         *
         * @return  The number of byte used by the urn (does not count memory dynamiccally allocate by T
         *          objects).
         **/
        size_t memoryUsed() const { return MEM_FOR_OBJ(T, _tab.size()) + sizeof(*this); }

        /**
        * Memory allocated by the urn.
        *
        * @return  The number of byte used by the urn (does not count memory dynamiccally allocate by T
        *          objects).
        **/
        size_t memoryAllocated() const { return MEM_FOR_OBJ(T, _tab.capacity()) + sizeof(*this); }


        /**
        * serialise/deserialize the urn. Works with boost and with the custom serialization classes
        * OArchive and IArchive. the method performs both serialization and deserialization.
        **/
        template<typename U> void serialize(U & Archive, const int version = 0)
            {
            Archive & _tab;
            }


    private: 
        std::vector<T> _tab;
    };


}


/* end of file */



