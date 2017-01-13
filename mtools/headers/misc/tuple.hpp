/** @file tuple.hpp */
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

namespace mtools
	{

	/* commodity function: return the first element of a tuple */
	template<typename... U>  auto first(std::tuple<U...> tup) -> decltype(std::get<0>(tup)) { return std::get<0>(tup); }

	/* commodity function: return the second element of a tuple */
	template<typename... U>  auto second(std::tuple<U...> tup) -> decltype(std::get<1>(tup)) { return std::get<1>(tup); }

	/* commodity function: return the third element of a tuple */
	template<typename... U>  auto third(std::tuple<U...> tup) -> decltype(std::get<2>(tup)) { return std::get<2>(tup); }

	/* commodity function: return the fourth element of a tuple */
	template<typename... U>  auto fourth(std::tuple<U...> tup) -> decltype(std::get<3>(tup)) { return std::get<3>(tup); }

	/* commodity function: return the fifth element of a tuple */
	template<typename... U>  auto fifth(std::tuple<U...> tup) -> decltype(std::get<4>(tup)) { return std::get<4>(tup); }


	}


/* end of file */




