/** @file triple.hpp */
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

#include <string>
#include "stringfct.hpp"

namespace mtools
	{

	/**
	 * A triple template class
	 *
	 * copied from std::pair<>. 
	 **/
	template<class _Ty1, class _Ty2, class _Ty3> struct triple
		{	// store a triple of values

		typedef triple<_Ty1, _Ty2, _Ty3> _Myt;
		typedef _Ty1 first_type;
		typedef _Ty2 second_type;
		typedef _Ty3 third_type;

		constexpr triple() : first(), second(), third()
			{	// default construct
			}

		constexpr triple(const _Ty1& _Val1, const _Ty2& _Val2, const _Ty3& _Val3) : first(_Val1), second(_Val2), third(_Val3)
			{	// construct from specified values
			}

		triple(const triple&) = default;
		triple(triple&&) = default;

		template<class _Other1, class _Other2, class _Other3,
		class = typename enable_if<is_convertible<const _Other1&, _Ty1>::value && is_convertible<const _Other2&, _Ty2>::value && is_convertible<const _Other3&, _Ty3>::value, void>::type>
			constexpr triple(const triple<_Other1, _Other2, _Other3> & _Right) : first(_Right.first), second(_Right.second), third(_Right.third)
			{	// construct from compatible triple
			}

		template<class _Other1, class _Other2, class _Other3> _Myt& operator=(const triple<_Other1, _Other2, _Other3>& _Right)
			{	// assign from compatible triple
			first = _Right.first;
			second = _Right.second;
			third = _Right.third;
			return (*this);
			}

		/* no implemented
		template<class _Tuple1, class _Tuple2, class _Tuple3, size_t... _Indexes1, size_t... _Indexes2, size_t... _Indexes3> inline
		triple(_Tuple1& _Val1, _Tuple2& _Val2, _Tuple3& _Val3, integer_sequence<size_t, _Indexes1...>, integer_sequence<size_t, _Indexes2...>, integer_sequence<size_t, _Indexes3...>);

		template<class... _Types1,	class... _Types2, class... _Types3> inline
		triple(piecewise_construct_t, tuple<_Types1...> _Val1, tuple<_Types2...> _Val2, tuple<_Types3...> _Val3);
		*/

		template<class _Other1, class _Other2, class _Other3,
		class = typename enable_if<is_convertible<_Other1, _Ty1>::value && is_convertible<_Other2, _Ty2>::value && is_convertible<_Other3, _Ty3>::value, void>::type>
			constexpr triple(_Other1&& _Val1, _Other2&& _Val2, _Other3&& _Val3)
			noexcept((is_nothrow_constructible<_Ty1, _Other1&&>::value && is_nothrow_constructible<_Ty2, _Other2&&>::value && is_nothrow_constructible<_Ty3, _Other3&&>::value))
			: first(_STD forward<_Other1>(_Val1)), second(_STD forward<_Other2>(_Val2)), second(_STD forward<_Other3>(_Val3))
			{	// construct from moved values
			}

		template<class _Other1, class _Other2, class _Other3,
		class = typename enable_if<is_convertible<_Other1, _Ty1>::value && is_convertible<_Other2, _Ty2>::value && is_convertible<_Other2, _Ty3>::value, void>::type>
			constexpr triple(triple<_Other1, _Other2, _Other3>&& _Right)
			noexcept((is_nothrow_constructible<_Ty1, _Other1&&>::value && is_nothrow_constructible<_Ty2, _Other2&&>::value && is_nothrow_constructible<_Ty3, _Other3&&>::value))
			: first(_STD forward<_Other1>(_Right.first)), second(_STD forward<_Other2>(_Right.second)), third(_STD forward<_Other3>(_Right.third))
			{	// construct from moved compatible triple
			}

		template<class _Other1, class _Other2, class _Other3>
		_Myt& operator=(triple<_Other1, _Other2, _Other3>&& _Right)
			{	// assign from moved compatible triple
			first = _STD forward<_Other1>(_Right.first);
			second = _STD forward<_Other2>(_Right.second);
			third = _STD forward<_Other3>(_Right.third);
			return (*this);
			}

		_Myt& operator=(_Myt&& _Right)
			noexcept((is_nothrow_move_assignable<_Ty1>::value && is_nothrow_move_assignable<_Ty2>::value && is_nothrow_move_assignable<_Ty3>::value))
			{	// assign from moved triple
			first = _STD forward<_Ty1>(_Right.first);
			second = _STD forward<_Ty2>(_Right.second);
			third = _STD forward<_Ty3>(_Right.third);
			return (*this);
			}

		void swap(_Myt& _Right)
			{
			if (this != &_Right) { _Swap_adl(first, _Right.first); _Swap_adl(second, _Right.second); _Swap_adl(third, _Right.third); }
			}


		_Myt& operator=(const _Myt& _Right)
			{	// assign from copied triple
			first = _Right.first;
			second = _Right.second;
			third = _Right.third;
			return (*this);
			}



		/**
		* serialise/deserialize the triple. Works with boost and with the custom serialization classes
		* OArchive and IArchive. the method performs both serialization and deserialization.
		**/
		template<typename U> void serialize(U & Archive, const int version = 0)
			{
			Archive & first;
			Archive & second;
			Archive & third;
			}

		/**
		* Convert into a string.
		**/
		std::string toString(bool includeTypeInfo = false) const
			{
			std::string s;
			if (includeTypeInfo) { s += std::string("triple<") + typeid(_Ty1).name() + "," + typeid(_Ty2).name() + "," + typeid(_Ty3).name() + ">"; }
			s += std::string("(") + mtools::toString(first) + "," + mtools::toString(second) + "," + mtools::toString(third) + ")";
			return s;
			}


		_Ty1 first;		// the first stored value
		_Ty2 second;	// the second stored value
		_Ty3 third;		// the second stored value

		};

	// triple TEMPLATE FUNCTIONS

	template<class _Ty1, class _Ty2, class _Ty3> inline
		void swap(triple<_Ty1, _Ty2, _Ty3>& _Left, triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// swap _Left and _Right triples
		_Left.swap(_Right);
		}

	template<class _Ty1, class _Ty2, class _Ty3> inline
		constexpr bool operator==(const triple<_Ty1, _Ty2, _Ty3>& _Left, const triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// test for triple equality
		return (_Left.first == _Right.first && _Left.second == _Right.second && _Left.third == _Right.third);
		}

	template<class _Ty1, class _Ty2, class _Ty3> inline
		constexpr bool operator!=(const triple<_Ty1, _Ty2, _Ty3>& _Left, const triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// test for triple inequality
		return (!(_Left == _Right));
		}

	template<class _Ty1, class _Ty2, class _Ty3> inline
		constexpr bool operator<(const triple<_Ty1, _Ty2, _Ty3>& _Left, const triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// test if _Left < _Right for triple
		return (
			(_Left.first < _Right.first) ||
			((!(_Right.first < _Left.first)) && (_Left.second < _Right.second)) ||
			((!(_Right.first < _Left.first)) && (!(_Right.second < _Left.second)) && (_Left.third < _Right.third))
			);
		}

	template<class _Ty1, class _Ty2, class _Ty3> inline
		constexpr bool operator>(const triple<_Ty1, _Ty2, _Ty3>& _Left, const triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// test if _Left > _Right for triple
		return (_Right < _Left);
		}

	template<class _Ty1, class _Ty2, class _Ty3> inline
		constexpr bool operator<=(const triple<_Ty1, _Ty2, _Ty3>& _Left, const triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// test if _Left <= _Right for triple
		return (!(_Right < _Left));
		}

	template<class _Ty1, class _Ty2, class _Ty3> inline
		constexpr bool operator>=(const triple<_Ty1, _Ty2, _Ty3>& _Left, const triple<_Ty1, _Ty2, _Ty3>& _Right)
		{	// test if _Left >= _Right for triple
		return (!(_Left < _Right));
		}


	}


/* end of file */




