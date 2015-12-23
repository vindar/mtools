/** @file metaprog.hpp */
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


#include <type_traits>
#include <typeinfo>
#include <sstream>

namespace mtools
{


    /* emulate std::remove_cv_t in case compiler doesn't support C++14 */
    template< class T > using remove_cv_t = typename std::remove_cv<T>::type;


	/* forward declaration */
	class IArchive;
	class OArchive;
	
    /**
    * Namespace containing meta-programing methods
    **/
    namespace metaprog
    {

        template<bool B> class dummy {};        ///< dummy class, useful for method overload depending on template parameter. 
        template<int K> class dummint {};       ///< dummy class, useful for method overload depending on template parameter. 

        struct yes { bool _yes; };
        struct no { bool _no[7]; };


        /**
         * has_operator_equal::value = true if T is comparable with ==.
         *
         * @tparam  T   Generic type parameter to test.
         **/
        template<typename T> class has_operator_equal
        {
            template<typename U> static decltype(  ((*(U*)(0)) == (*(U*)(0))) ) test(int); // test the existence comparison with ==
            template<typename> static no test(...);                                              // fallback for SFINAE
        public:
            
            static const bool value = std::is_convertible< typename std::decay<decltype(test<T>(0))>::type, bool >::value;
        };


        /**
        * has_assignementOperator::value = true if T has a method which call be called in this way: `T->operator=(T)`
        *
        * @tparam  T   Generic type parameter.
        **/
        template<typename T> class has_assignementOperator
        {
            template<typename U> static decltype(((*(U*)(0)) = (*(U*)(0))), yes()) test(int); // test the existence of operator=()
            template<typename> static no test(...);                                               // fallback for SFINAE
        public:
            static const bool value = (sizeof(test<T>(0)) == sizeof(yes));
        };




        /**
         * Check if a type is serialisable.
         *  is_serializable<T>::value_serialize return the type of serialization supported.
         *  is_serializable<T>::value_deserialize return the type of deserialization supported.
         * 
         * - The preference order for serialization is: METHOD_SERIALIZE > FUNCTION_SERIALIZE > NONE
         * 
         * - The preference order for deserialization is: METHOD_DESERIALIZE > FUNCTION_DESERIALIZE >
         * METHOD_SERIALIZE > FUNCTION SERIALIZE > NONE.
         **/

        template<typename T> class is_serializable
        {
            template<typename U> static decltype((*(U*)(0)).serialize(*(OArchive*)(0)), yes()) testserializeMO(int);
            template<typename> static no testserializeMO(...);
            static const bool _hasserializeMO = (sizeof(testserializeMO<T>(0)) == sizeof(yes));

            template<typename U> static decltype(serialize((*(OArchive*)(0)), (*(U*)(0))), yes()) testserializeGO(int);
            template<typename> static no testserializeGO(...);
            static const bool _hasserializeGO = (sizeof(testserializeGO<T>(0)) == sizeof(yes));

            template<typename U> static decltype((*(U*)(0)).serialize(*(IArchive*)(0)), yes()) testserializeMI(int);
            template<typename> static no testserializeMI(...);
            static const bool _hasserializeMI = (sizeof(testserializeMI<T>(0)) == sizeof(yes));

            template<typename U> static decltype(serialize((*(IArchive*)(0)), (*(U*)(0))), yes()) testserializeGI(int);
            template<typename> static no testserializeGI(...);
            static const bool _hasserializeGI = (sizeof(testserializeGI<T>(0)) == sizeof(yes));


            template<typename U> static decltype((*(U*)(0)).deserialize(*(IArchive*)(0)), yes()) testdeserializeMI(int);
            template<typename> static no testdeserializeMI(...);
            static const bool _hasdeserializeMI = (sizeof(testdeserializeMI<T>(0)) == sizeof(yes));

            template<typename U> static decltype(deserialize((*(IArchive*)(0)), (*(U*)(0))), yes()) testdeserializeGI(int);
            template<typename> static no testdeserializeGI(...);
            static const bool _hasdeserializeGI = (sizeof(testdeserializeGI<T>(0)) == sizeof(yes));

            public:

            static const int METHOD_SERIALIZE = 4;
            static const int METHOD_DESERIALIZE = 3;
            static const int FUNCTION_SERIALIZE = 2;
            static const int FUNCTION_DESERIALIZE = 1;
            static const int NONE = 0;

            static const int value_serialize = (_hasserializeMO ? METHOD_SERIALIZE : (_hasserializeGO ? FUNCTION_SERIALIZE : NONE));
            static const int value_deserialize = (_hasdeserializeMI ? METHOD_DESERIALIZE : (_hasdeserializeGI ? FUNCTION_DESERIALIZE : (_hasserializeMI ? METHOD_SERIALIZE : (_hasserializeGI ? FUNCTION_SERIALIZE : NONE))));

        };


        /**
         * has_toString::value = true if T has a method which call be called in this way: `T->toString()`  and return a value convertible to std::string.
         *
         * @tparam  T   Generic type parameter.
         **/
        template<typename T> class has_toString
        {
            template<typename U> static decltype((*((U*)0)).toString()) test(int); // test the existence of the method toString()
            template<typename> static no test(...);                                       // fallback for SFINAE
        public:
            static const bool value = std::is_convertible< typename std::decay<decltype(test<T>(0))>::type, std::string >::value;
        };


        /**
         * has_to_string::value = true if T has a method which call be called in this way: `T.to_string()` and return a value convertible to std::string.
         *
         * @tparam  T   Generic type parameter.
         **/
        template<typename T> class has_to_string
        {
            template<typename U> static decltype((*((U*)0)).to_string()) test(int); // test the existence of the method toString()
            template<typename> static no test(...);                                       // fallback for SFINAE
        public:
            static const bool value = std::is_convertible< typename std::decay<decltype(test<T>(0))>::type, std::string >::value;
        };


        /**
         * has_getColor::value = true if T has a method which call be called in this way: `T->getColor(V)` and return a value convertible to type O.
         *
         * @tparam  T   Generic type parameter.
         * @tparam  O   Generic type parameter.
         * @tparam  V   Generic type parameter.
         **/
        template<typename T,typename O, typename V> class has_getColor
        {
            template<typename U> static decltype((*((U*)0)).getColor(*((V*)0))) test(int); // test the existence of the method getColor(iVec2 )
            template<typename> static no test(...);                                       // fallback for SFINAE
        public:
            static const bool value = std::is_convertible< typename std::decay<decltype(test<T>(0))>::type, typename std::decay<O>::type >::value;
        };

        /**
        * has_getColorExt::value = true if T has a method which call be called in this way: `T->getColor(V,W)` and returns a value convertible to type O.
        *
        * @tparam  T   Generic type parameter.
        * @tparam  O   Generic type parameter.
        * @tparam  V   Generic type parameter.
        * @tparam  W   Generic type parameter.
        **/
        template<typename T, typename O, typename V,typename W> class has_getColorExt
            {
            template<typename U> static decltype((*((U*)0)).getColor(*((V*)0), *((W*)0))) test(int); // test the existence of the method getColor(iVec2 )
            template<typename> static no test(...);                                       // fallback for SFINAE
            public:
                static const bool value = std::is_convertible< typename std::decay<decltype(test<T>(0))>::type, typename std::decay<O>::type >::value;
            };



        /**
         * has_getImage::value = true if T has a method which call be called in this way: `T->getImage(V,W)` and return a value convertible to type O.
         *
         * @tparam  T   Generic type parameter.
         **/
        template<typename T, typename O, typename V, typename W> class has_getImage
        {
            template<typename U> static decltype((*((U*)0)).getImage((*((V*)0)), (*((W*)0)))) test(int); // test the existence of the method getImage(iVec2 )
            template<typename> static no test(...);                                       // fallback for SFINAE
        public:
            static const bool value = std::is_convertible< typename std::decay<decltype(test<T>(0))>::type, typename std::decay<O>::type >::value;
        };



        /**
         * has_to_ostream::value = true if T can be printed using `std::ostringstream << T`
         *
         * @tparam  T   Generic type parameter.
         **/
        template<typename T> class has_to_ostream
            {
            template<typename U> static decltype((((*((::std::ostringstream*)0)) << (*((U*)0))))) test(int);
            template<typename U> static no test(...);
            template<typename U> static decltype((((*((::std::wostringstream*)0)) << (*((U*)0))))) testW(int);
            template<typename U> static no testW(...);
            public:
                static const bool value_ostream  = !(std::is_same< decltype(test<T>(0)), no>::value);
                static const bool value_wostream = !(std::is_same< decltype(testW<T>(0)), no>::value);
                static const bool value = (value_ostream || value_wostream);
            };


        /**
        * has_from_istream::value = true if T can be obtained via `std::istringstream >> T`
        *
        * @tparam  T   Generic type parameter.
        **/
        template<typename T> class has_from_istream
            {
            template<typename U> static decltype((((*((::std::istringstream*)0)) >> (*((U*)0))))) test(int);
            template<typename U> static no test(...);
            template<typename U> static decltype((((*((::std::wistringstream*)0)) >> (*((U*)0))))) testW(int);
            template<typename U> static no testW(...);
            public:
                static const bool value_istream = !(std::is_same< decltype(test<T>(0)), no>::value);
                static const bool value_wistream = !(std::is_same< decltype(testW<T>(0)), no>::value);
                static const bool value = (value_istream || value_wistream);
            };



        /**
        * Internal namespace with helper class for computing the GCD/LCM of two integer at compile time
        * 
        * Adapted from boost implementation of GCD/LCM
        **/
        namespace internals_gcdlcm
            {
            template < unsigned long Value1, unsigned long Value2 > struct static_gcd_helper_t
                {
                private:
                    static const unsigned long new_value1 = Value2;
                    static const unsigned long new_value2 = Value1 % Value2;
                    typedef static_gcd_helper_t< static_cast<unsigned long>(new_value1), static_cast<unsigned long>(new_value2) >  next_step_type;
                public:
                 static const unsigned long value = next_step_type::value;
                };

            template < unsigned long Value1 > struct static_gcd_helper_t< Value1, 0UL > {static const unsigned long value = Value1;};

            template < unsigned long Value1, unsigned long Value2 > struct static_lcm_helper_t
                {
                typedef static_gcd_helper_t<Value1, Value2>  gcd_type;
                static const unsigned long value = Value1 / gcd_type::value * Value2;
                };

            template < > struct static_lcm_helper_t< 0UL, 0UL > { static const unsigned long value = 0UL; };
            }


        /**
         * Compile-time computation of the greatest common divisor of two integers. 
         * static_gcd<A,B>::value return the GCD of A and B.
         **/
        template < unsigned long Value1, unsigned long Value2 > struct static_gcd 
            {
            static const unsigned long value = (internals_gcdlcm::static_gcd_helper_t<Value1, Value2>::value);
            };


        /**
         * Compile-time computation of the least common multiple of two integers. 
         * static_lcm<A,B>::value return the LCM of A and B.
         **/
        template < unsigned long Value1, unsigned long Value2 > struct static_lcm
            {
            static const unsigned long value = (internals_gcdlcm::static_lcm_helper_t<Value1, Value2>::value);
            };


        /**
         * Compile-time computation of powers.
         * power<B,N>::value returns B^N.
         **/
        template<int B, int N> struct power { enum{ value = B*power<B, N - 1>::value }; };
        template<int B> struct power<B,0> { enum{ value = 1 }; };

    }

}


/* end of file */



