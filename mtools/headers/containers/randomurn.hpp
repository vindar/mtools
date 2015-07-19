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
#include "../misc/stringfct.hpp"
#include "../misc/misc.hpp"

namespace mtools
{
    /**
     * A random urn containing element of type T. Element can be added and drawn from the urn with
     * or without putting them back.
     * 
     * @code{.cpp}
     * int main()
     * {
     * RandomUrn<int> Urn(20);
     * cout << Urn.toString(true);
     * for (int i = 0;i<9;i++) { Urn.Add(i); } // add 9 elements
     * cout << Urn.toString(true);
     * cout << "Get: " << Urn.Retrieve(0.5) << "\n\n";
     * cout << Urn.toString(true);
     * cout << "Get (and put back): " << Urn.PickAndPutBack(0.5) << "\n\n";
     * cout << Urn.toString(true);
     * cout << "Get: " << Urn.Retrieve(0.5) << "\n\n";
     * cout << Urn.toString(true);
     * cout << "Adding element 9.\n\n";
     * Urn.Add(9);
     * cout << Urn.toString(true);
     * cout.getKey();
     * return 0;
     * }
     * @endcode.
     *
     * @tparam  T   must implement, default constructor, copy constructor, assignement operator=.
    **/
    template<typename T> class RandomUrn
    {
    public: 

        /**
         * Constructor.
         *
         * @param   size    Max number of item in the urn.
        **/
        RandomUrn(uint32 size) : maxsize(size)
        { 
            tab = new T[maxsize];
            Clear();
        }
       

        /**
         * Destructor.
        **/
        ~RandomUrn() {delete [] tab;}


        /**
         *  Remove all the elments in the Urn
        **/
        inline void Clear() {nb=0;}


        /**
         * Return the number of elements the urn contains
        **/
        inline uint64 NbElements() const {return (uint64)nb;}


        /**
         * Return the maximum number of element the urn can contain
        **/
        inline uint64 UrnSize() const {return (uint64)maxsize;}


        /**
         * Retrieve a random element from the urn AND REMOVE IT FROM THE URN
         *
         * @param   a   a random number in [0,1).
         *
         * @return  the element removed.
        **/
        inline T Retrieve(double a)
            {
            if (nb == 0) MTOOLS_ERROR("Urn empty.");
            size_t p = (size_t)(nb*a);
            if (p >= nb)  { MTOOLS_ERROR("Invalid random number."); }
            T val = tab[p]; if (p!= (nb-1)) {tab[p] = tab[nb-1];}
            nb--;
            return val;
            }


        /**
         * Retrieves the elepment at position pos. return element at position 0 if out of bound.
         *
         * @param   pos The position.
         *
         * @return  the element at position pos.
        **/
		inline T RetrieveAtPos(size_t pos)
			{
            if (pos >= nb) { return tab[0]; }
			return tab[pos];
			}


        /**
         * Remove the element located at position pos in the urn
         *
         * @param   pos The position.
        **/
		inline void RemoveAtPos(size_t pos)
			{
            if (pos >= nb) { return; }
            T val = tab[pos]; if (pos != (nb-1)) {tab[pos] = tab[nb-1];}
            nb--;
			}


  
        /**
         * Retrieve a random element from the urn AND PUT IT BACK IN THE URN
         *
         * @param   a random number in [0,1).  
         *
         * @return  A the element.
        **/
        inline T PickAndPutBack(double a) const
            {
            if (nb == 0) { MTOOLS_ERROR("Empty urn"); }
            size_t p = (size_t)(nb*a);
            if (p >= nb)  { MTOOLS_ERROR("RandomUrn::PickAndPutBack(), invalid random number a !"); }
            return tab[p];
            }


        /**
         * Add a new element in the Urn.
         *
         * @param   val The value to add.
        **/
        inline void Add(const T & val)
            {
            if (nb == maxsize) { MTOOLS_ERROR("RandomUrn::Add(), Urn is full !");}
            tab[nb] = val; 
            nb++;
            return;
            }


        /**
         * Get some stats about the object.
        **/
        std::string toString(bool debug = false)
            {
            std::string s;
            s += "*****************************************************\n";
            s += "Random Urn object statistics\n\n";
            s += "- memory size : " + mtools::toString((maxsize*sizeof(T))/(1024*1024)) + "Mb\n";
            s += "- Size of each Element : " + mtools::toString(sizeof(T)) + " octets\n";
            s += "- maximal number of elements : " + mtools::toString(maxsize) + "\n";
            s += "- actual number of elements  : " + mtools::toString(nb) + "  (" + mtools::toString((int)(100.0*((double)nb)/((double)maxsize))) + "% occupied)\n";
            s += "*****************************************************\n";
            if (debug) { s += printDebug(); }
            return s;
            }



        /**
         * Print the content of the urn in a std::tring. For debugging purpose only.
        **/
        std::string printDebug()
            {
                std::string s;
                if (nb == 0) {s+= "The urn is empty !\n"; return s;}
                s += "The Urn contains " + mtools::toString(nb) + " elements:\n[";
               for(size_t i =0; i<nb;i++) { if (i!=0) {s+=",";} s += mtools::toString(tab[i]);}
                s+= "]\n";
                return s;
            }


    /* private member */
    private:

        T *     tab;     // the tab containing the elements of the urn
        size_t  nb;      // number of elements in the urn
        size_t  maxsize; // maximum number of elements the urn can contain

        /* no copy */
        RandomUrn(const RandomUrn &);
        RandomUrn & operator=(const RandomUrn &);

    };


}


/* end of file */



