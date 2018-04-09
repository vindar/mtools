/** @file palette.hpp */
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
#include "../misc/misc.hpp"
#include "rgbc.hpp"


namespace mtools
{



	/**
	* POD structure that hold a palette of colors.
	*
	* Several palette schemes are predefined (c.f. palette.cpp)
	**/
	struct ColorPalette
	{

	public:


		/**
		* Return a given color in the palette and circle around  the palette.
		*
		* @param	n index of the color to get, modulo the palette size.
		*
		* @return	The corresponding color.
		*/
		MTOOLS_FORCEINLINE RGBc operator[](size_t n) const
		{
			MTOOLS_ASSERT(_size > 0);
			MTOOLS_ASSERT(_size <= MAX_PALETTE_SIZE);
			return _color[n % _size];
		}


		/**
		* Return the color asociated with a value in [0,1]
		*
		* @param	x		    Value to query. Clamped to [0,1].
		* @param	interpolate (Optional) True to interpolate between color.
		* @param	reverse	    (Optional) True to reverse the palette order.
		*
		* @return	The corresponding color.
		*/
		inline RGBc get(double x, bool interpolate = false, bool reverse = false) const
		{
			MTOOLS_ASSERT(_size > 0);
			MTOOLS_ASSERT(_size <= MAX_PALETTE_SIZE);
			if (reverse) { x = 1.0 - x; }
			x *= _size;
			const int64 ind = (int64)floor(x);
			if (ind < 0) return _color[0]; else if (ind >= (int64)(_size - 1)) return _color[_size - 1];
			if (!interpolate) { return _color[ind]; }
			const double  lB = x - ind;
			const double  lA = 1.0 - lB;
			const RGBc & A = _color[ind];
			const RGBc & B = _color[ind + 1];
			return RGBc((uint8)(lA*A.comp.R + lB * B.comp.R),
				        (uint8)(lA*A.comp.G + lB * B.comp.G),
				        (uint8)(lA*A.comp.B + lB * B.comp.B));
		}


		/**
		* Return the color associated with an (integer) value in [min(a,b), max(a,b)].
		*
		* @param	v		    value to query. clamped inside [min(a,b), max(a,b)].
		* @param	a		    lower/upper bound for the value range.
		* @param	b		    lower/upper bound for the value range.
		* @param	interpolate (Optional) True to interpolate between color.
		* @param	reverse	    (Optional) True to reverse the palette order.
		*
		* @return	The corresponding color.
		*/
		MTOOLS_FORCEINLINE RGBc get(int64 v, int64 a, int64 b, bool interpolate = false, bool reverse = false) const
		{
			if (a > b) mtools::swap(a, b);
			MTOOLS_ASSERT(b > a);
			return get(((double)(v - a)) / ((double)(b - a)), interpolate, reverse);
		}


		/**
		* Gets a log
		*
		* @param	x		    The x coordinate.
		* @param	expo	    (Optional) the logarithmic exponent to use.
		* @param	interpolate (Optional) True to interpolate between color.
		* @param	reverse	    (Optional) True to reverse the palette order.
		*
		* @return	The corresponding color.
		*/
		inline RGBc getLog(double x, double expo = 2.0, bool interpolate = false, bool reverse = false) const
		{
			if ((x <= 0.0) || (x >= 1.0) || (expo == 1.0)) return get(x, interpolate, reverse);	// linear scale
			if (expo < 1.0)  return getLog(1.0 - x, 1.0 / expo);								// exponent < 1.0 obtain from exponent > 1.0 by symetry
			const double ee = pow(expo, _size);
			const double eps = ((expo - 1.0) / (ee - 1));
			if (eps == 0.0)
			{
				MTOOLS_DEBUG("Palette exponent too large. Default to linear scale.");
				return get(x, interpolate, reverse);
			}
			const double z = (log(x*(expo - 1) / eps + 1) / log(expo));
			return get(z / _size, interpolate, reverse);
		}


		/**
		* A color of the palette in logarithmic scale.
		*
		* Logaritmic interpolation of the color between [min(a,b), max(a,b)] The color interval are of
		* the form [A0,A1] [A1,A2] ...  where |Ai+1 - Ai| = exponent * |Ai - Ai-1|.
		*
		* @param	v		    value to query. clamped inside [min(a,b), max(a,b)].
		* @param	a		    lower/upper bound for the value range.
		* @param	b		    lower/upper bound for the value range.
		* @param	expo	    (Optional) the logarithmic exponent to use.
		* @param	interpolate (Optional) True to interpolate between color.
		* @param	reverse	    (Optional) True to reverse the palette order.
		*
		* @return	The corresponding color.
		*/
		MTOOLS_FORCEINLINE RGBc getLog(int64 v, int64 a, int64 b, double expo = 2.0, bool interpolate = false, bool reverse = false) const
		{
			if (a > b) mtools::swap(a, b);
			MTOOLS_ASSERT(b > a);
			const double x = ((double)(v - a)) / ((double)(b - a));
			return getLog(x, expo, interpolate, reverse);
		}


		/**
		* Return the color asociated with a value in [0,1]. Same as the get() method.
		*
		* @param	x		    Value to query. Clamped to [0,1].
		* @param	interpolate (Optional) True to interpolate between color.
		* @param	reverse	    (Optional) True to reverse the palette order.
		*
		* @return	The corresponding color.
		*/
		MTOOLS_FORCEINLINE RGBc operator()(double x, bool interpolate = false, bool reverse = false) const { return get(x, interpolate, reverse); }


		/**
		* Return the color associated with an (integer) value in [min(a,b), max(a,b)]. Same as the
		* get() method.
		*
		* @param	v		    value to query. clamped inside [min(a,b), max(a,b)].
		* @param	a		    lower/upper bound for the value range.
		* @param	b		    lower/upper bound for the value range.
		* @param	interpolate (Optional) True to interpolate between color.
		* @param	reverse	    (Optional) True to reverse the palette order.
		*
		* @return	The corresponding color.
		*/
		MTOOLS_FORCEINLINE RGBc operator()(int64 v, int64 a, int64 b, bool interpolate = false, bool reverse = false) const { return get(v, a, b, interpolate, reverse); }



		static const size_t	MAX_PALETTE_SIZE = 128;		// maximum number of color in the palette.
		size_t				_size;						// number of colors in the palette
		RGBc				_color[MAX_PALETTE_SIZE];	// colors that compose it

	};




	/** Predefined color palettes */
	namespace Palette
		{

		// unicolor palettes
		extern const ColorPalette Blue;
		extern const ColorPalette Green;
		extern const ColorPalette Black;
		extern const ColorPalette Orange;
		extern const ColorPalette Violet;
		extern const ColorPalette Red;

		// unicolor palette that degrading from white or yellow.
		extern const ColorPalette Yellow_to_Red;
		extern const ColorPalette Yellow_to_Blue;
		extern const ColorPalette Yellow_to_Green;
		extern const ColorPalette White_to_Green;
		extern const ColorPalette White_to_Violet;
		extern const ColorPalette White_to_Blue;

		// diverging palette (between 2 colors)
		extern const ColorPalette Red_to_Violet;
		extern const ColorPalette Red_to_Green;
		extern const ColorPalette Red_to_Blue;
		extern const ColorPalette Red_to_Black;
		extern const ColorPalette Maroon_to_Violet;
		extern const ColorPalette Violet_to_Green;
		extern const ColorPalette Maroon_to_blue;

		// multicolor palette
		extern const ColorPalette matlabJet;		// similar to matlab jet color palette

		// qualitative (distinct colors)
		extern const ColorPalette hard_12;			// hard color
		extern const ColorPalette soft_12;			// soft 'pastel' oclors
		extern const ColorPalette mix_32;			// mix of 32 diferent oclor starting from RGB. 

		}

}




/* end of file */