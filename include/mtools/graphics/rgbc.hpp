/** @file rgbc.hpp */
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
#include "../misc/stringfct.hpp"

#include <algorithm>
#include <string>


namespace mtools
{
	/**
	 * Convert a value in the range [0,0xFF] to the range [0,0x100]. Maps 0 to 0 and 0xFF to 0x100.
	 * convertAlpha_0x100_to_0xFF( convertAlpha_0xFF_to_0x100(v)) = v.
	 *
	 * @param	v	The value in [0,0xFF].
	 *
	 * @return	The scaled value in [0,0x100].
	 **/
	MTOOLS_FORCEINLINE uint32 convertAlpha_0xFF_to_0x100(uint32 v) { return(v + ((v & 128) >> 7)); }


	/**
	* Convert a value in the range [0,0xFF] to the range [0,0x100]. Maps 0 to 0 and 0xFF to 0x100.
	* 
	* convertAlpha_0x100_to_0xFF( convertAlpha_0xFF_to_0x100(v)) = v.
	* 
	* @param	v	The value in [0,0x100].
	*
	* @return	The scaled value in [0,0xFF].
	 **/
	MTOOLS_FORCEINLINE uint32 convertAlpha_0x100_to_0xFF(uint32 v) { return(v - (((~(v - 128)) >> 31))); }


    /* forward definition */
    union RGBc64;



    /**
     * A color in BGRA format.
     * 
	 * ONLY FOR LITTLE ENDIAN COMPUTERS. 
     **/
    union RGBc
    {

    public:


        /**
         ****************************************************************************
         * LAYOUT
         **************************************************************************** 
         * Color seen as a uint32. Low byte is Blue, high byte is Alpha.  
         * 
         * 31--------------------------------0
		 * AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB  
         **/
        uint32 color; 
							        
		/**
		* Assume Little endian computer. 
		* memory layout BGRA
		**/
		struct _RGBc_component
            {
            uint8 B;    // byte 0 : Blue.  (low byte in uint32)
            uint8 G;    // byte 1 : Green.
            uint8 R;    // byte 2 : Red
            uint8 A;    // byte 3 : alpha. (high byte in uint32)
            } comp;


        static const uint8 OPAQUEALPHA = 255;       ///< fully opaque
        static const uint8 TRANSPARENTALPHA = 0;    ///< fully transparent



		/****************************************************************************
		* CONSTRUCTOR / CONVERSION
		*****************************************************************************/

        /**
         * Default constructor.
         **/
        RGBc() = default;


        /**
         * Constructor.
         *
         * @param   r   color red.
         * @param   g   color green.
         * @param   b   color black.
         * @param   a   alpha channel in 0..255 
         **/
        RGBc(uint8 r, uint8 g, uint8 b, uint8 a = OPAQUEALPHA) : comp{b,g,r,a} 
			{
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
			}


        /**
        * Default copy constructor.
        **/
        RGBc(const RGBc & c) = default;


        /**
         * Constructor from a RGBc64. normalisation value is 1. 
         *
         * @param   c   The color.
         **/
		RGBc(const RGBc64 & c) { fromRGBc64(c); }


		/**
		* Constructor from a RGBc64. use a given normalisation value
		*
		 * @param	c the color
		 * @param	N normalisation value (must be >0). 
		 */
		RGBc(const RGBc64 & c, uint32 N) { fromRGBc64(c, N); }


        /**
         * Raw constructor from int32
         **/
        RGBc(int32 c)  { color = (uint32)c; }


        /**
        * Raw constructor from uint32
        **/
        RGBc(uint32 c) { color = (uint32)c; }


        /**
         * Constructor from a buffer. Use all 4 bytes.
         **/
        RGBc(const unsigned char * p) : color(*((uint32*)p)) 
			{
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
			}
        

        /**
         * Constructor from a buffer. Use only 3 bytes and set the alpha value separately
         **/
        RGBc(const unsigned char * p, unsigned char a) : comp{ p[0], p[1], p[2], a } 
			{
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
			}


        /**
         * Default assignment operator.
         **/
        RGBc & operator=(const RGBc & c) = default;


        /**
         * Assignment operator from a buffer. Use all 4 bytes.
         *
         * @warning The buffer must have size at least 4 !
         * @param   p   The buffer of size 3 with p[0] = blue, p[1] = green, p[2] = red, p[3] = alpha.
         **/
        RGBc & operator=(const unsigned char * p) 
			{ 
			color = *((uint32*)p);  
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
			return(*this); 
			}


		/**
		 * Set the color by copying from a RGBc64 color. normalization is set to 1
		 *
		 * @param	coul The color in RGBc64 format
		 */
		MTOOLS_FORCEINLINE void fromRGBc64(const RGBc64 & coul);


		/**
		 * Set the color by copying from a RGBc64 color
		 *
		 * @param	coul The color in RGBc64 format
		 * @param	N    The normalization (must be >0). 
		 */
		MTOOLS_FORCEINLINE void fromRGBc64(const RGBc64 & coul, const uint32 N);


        /**
         * The buffer for the B G R A color.
         **/
         unsigned char* buf() { return((unsigned char *)&color); }


        /**
         * The buffer for the B G R A color (const version).
         **/
        inline const unsigned char* buf() const { return((unsigned char *)&color); }


		/****************************************************************************
		* COMPARISON
		*****************************************************************************/


        /**
         * Equality operator.
         **/
        inline bool operator==(const RGBc & c) const { return (color == c.color); }


        /**
         * Inequality operator.
         **/
        inline bool operator!=(const RGBc & c) const { return (color != c.color); }




		/****************************************************************************
		* OPACITY
		*
		* All these methods assume PREMULTIPLIED ALPHA colors !
		*****************************************************************************/


		/** Convert a color that is not premultiplied to its premultiplied version */
		MTOOLS_FORCEINLINE void premultiply()
			{
			comp.R = (uint8)((((uint32)comp.R) * comp.A) / 255);
			comp.G = (uint8)((((uint32)comp.G) * comp.A) / 255);
			comp.B = (uint8)((((uint32)comp.B) * comp.A) / 255);
			}


		/** Convert a premultiplied color to its non-premultiplied version */
		MTOOLS_FORCEINLINE void unpremultiply()
			{
			MTOOLS_ASSERT((comp.R <= comp.A) && (comp.G <= comp.A) && (comp.B <= comp.A)); 
			if (comp.A == 0) return;
			comp.R = (uint8)((((uint32)comp.R) * 255) / comp.A);
			comp.G = (uint8)((((uint32)comp.G) * 255) / comp.A);
			comp.B = (uint8)((((uint32)comp.B) * 255) / comp.A);
			}



		/**
		* Query if the color is fully opaque
		*
		* @return	True if opaque, false if not.
		**/
		MTOOLS_FORCEINLINE bool isOpaque() const { return(comp.A== 255); }


		/**
		 * Query if the color is fully transparent
		 *
		 * @return	True if transparent, false if not.
		 **/
		MTOOLS_FORCEINLINE bool isTransparent() const { return(comp.A == 0); }



        /**
         * Return the opacity of the color as a float.
         *
         * @return  The opacity between 0.0 (fully transparent) and 1.0 (fully opaque). 
         **/
		MTOOLS_FORCEINLINE float opacity() const { return ((float)comp.A)/(255.0f); }


		/**
		* Return the opacity of the color as a int
		*
		* @return  The opacity in the range [0x00, 0x100].
		**/
		MTOOLS_FORCEINLINE uint32 opacityInt() const { return convertAlpha_0xFF_to_0x100(comp.A); }


        /**
         * Change opacity of the color. Create a pre-multiplied color !
         * 
         * Slow : faster to use multOpacity() to change the opacity.
		 *
         * @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
         **/
		MTOOLS_FORCEINLINE void opacity(float o) { *this = getOpacity(o); }


		/**
		* Return the same color but with a given opacity (with premultiplied alpha).
		*
		* @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
		**/
		MTOOLS_FORCEINLINE RGBc getOpacity(float o) const 
			{ 
			MTOOLS_ASSERT((o >= 0.0f) && (o <= 1.0f)); 
			float mo = o * 255.0f;
			float mult = (comp.A == 0) ? 0.0f : (mo/((float)comp.A));
			return RGBc((uint8)(comp.R*mult), (uint8)(comp.G*mult), (uint8)(comp.B*mult), (uint8)mo);
			}


		/**
		* Return the same color but completely opaque.
		*
		* @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
		**/
		MTOOLS_FORCEINLINE RGBc getOpaque() const { return getOpacity(1.0f); }


		/**
		* Multiply the opacity by a given factor. 
		*
		* @param   o   the multiplication factor between 0.0f and 1.0f.
		**/
		MTOOLS_FORCEINLINE void multOpacity(float o) { *this = getMultOpacity(o); }


		/**
		* Return the same color but with its opacity multiplied by a given factor.
		*
		* @param   o   the multiplication factor between 0.0f and 1.0f.
		**/
		MTOOLS_FORCEINLINE RGBc getMultOpacity(float o) const 
			{ 
			MTOOLS_ASSERT((o >= 0.0f) && (o <= 1.0f)); 
			uint32 op = (uint32)(256 * o); 
			return getMultOpacityInt(op);
			}


		/**
		* Multiply the opacity by a given factor.
		*
		* @param   op   the multiplication factor in the range [0, 0x100] (use convertAlpha_0xFF_to_0x100() 
		* 				to convert a value in [0,0xFF] to this range).
		**/
		MTOOLS_FORCEINLINE void multOpacityInt(uint32 op) { *this = getMultOpacityInt(op); }


		/**
		* Return the same color but with its opacity multiplied by a given factor.
		*
		* @param   op   the multiplication factor in the range [0, 0x100] (use convertAlpha_0xFF_to_0x100()
		* 				to convert a value in [0,0xFF] to this range).
		**/
		MTOOLS_FORCEINLINE RGBc getMultOpacityInt(uint32 op) const
		{
			MTOOLS_ASSERT((op >= 0) && (op <= 256));
			uint32 ag = (color & 0xFF00FF00) >> 8;
			uint32 rb = color & 0x00FF00FF;
			uint32 sag = op * ag;
			uint32 srb = op * rb;
			sag = sag & 0xFF00FF00;
			srb = (srb >> 8) & 0x00FF00FF;
			return sag | srb;
		}


		/****************************************************************************
		* ALPHA BLENDING
		* 
		* All these methods assume PREMULTIPLIED ALPHA colors !
		*****************************************************************************/


		/**
		 * Blends colorB over this color.
		 * Colors are assumed to have pre-multiplied alpha.
		 *
		 * @param	colorB	The 'top' color to blend over this one.
		 **/
		MTOOLS_FORCEINLINE void blend(const RGBc colorB) { (*this) = get_blend(colorB); }


		/**
		 * Blends colorB over this color.
		 * Colors are assumed to have pre-multiplied alpha.
		 *
		 * @param	colorB 	The 'top' color to blend over this one.
		 * @param	opacity	The opacity to multiply colorB alpha channel with before blending, must be in
		 * 					the range [0, 0x100] (use convertAlpha_0xFF_to_0x100() to convert a value in
		 * 					[0,0xFF] to this range).
		 **/
		MTOOLS_FORCEINLINE void blend(const RGBc colorB, const uint32 opacity) { (*this) = get_blend(colorB,opacity); }


		/**
		* Blends colorB over this color.
		* Colors are assumed to have pre-multiplied alpha.
		*
		* @param	colorB 	The 'top' color to blend over this one.
		* @param	opacity	The opacity to multiply colorB alpha channel with before blending, must be in
		* 					the range [0,1.0f].
		**/
		MTOOLS_FORCEINLINE void blend(const RGBc colorB, const float opacity) {	(*this) = get_blend(colorB, opacity); }


		/**
		 * Return the color obtained by blending colorB over this one.
		 * Colors are assumed to have pre-multiplied alpha.
		 *
		 * @param	colorB	The 'top' color to blend over this one.
		 *
		 * @return	the color obtained by blending colorB over this.
		 **/
		MTOOLS_FORCEINLINE RGBc get_blend(const RGBc colorB) const
			{
			const uint32 o = 0x100 - convertAlpha_0xFF_to_0x100(colorB.comp.A); 
			uint32 ag = (color & 0xFF00FF00) >> 8;
			uint32 rb = color & 0x00FF00FF;
			uint32 sag = o * ag;
			uint32 srb = o * rb;
			sag = sag & 0xFF00FF00;
			srb = (srb >> 8) & 0x00FF00FF;
			return (sag | srb) + colorB.color;
			}


		/**
		 * Return the color obtained by blending colorB over this one.
		 * Colors are assumed to have pre-multiplied alpha.
		 *
		 * @param	colorB 	The 'top' color to blend over this one.
		 * @param	opacity	The opacity to multiply colorB alpha channel with before blending, must be in
		 * 					the range [0, 0x100] (use convertAlpha_0xFF_to_0x100() to convert a value in
		 * 					[0,0xFF] to this range).
		 *
		 * @return	the color obtained by blending colorB over this.
		 **/
		MTOOLS_FORCEINLINE RGBc get_blend(const RGBc colorB, const uint32 opacity) const
			{
			MTOOLS_ASSERT((opacity >= 0) && (opacity <= 256));
			// premultiply colorB by opacity			
			uint32 Bag = (colorB.color & 0xFF00FF00) >> 8;
			uint32 Brb = (colorB.color & 0x00FF00FF);
			uint32 Bsag = opacity * Bag;
			uint32 Bsrb = opacity * Brb;
			Bsag = Bsag & 0xFF00FF00;
			Bsrb = (Bsrb >> 8) & 0x00FF00FF;			
			// blend
			const uint32 o = 0x100 - convertAlpha_0xFF_to_0x100(Bsag >> 24);
			uint32 ag = (color & 0xFF00FF00) >> 8;
			uint32 rb = color & 0x00FF00FF;
			uint32 sag = o * ag;
			uint32 srb = o * rb;
			sag = sag & 0xFF00FF00;
			srb = (srb >> 8) & 0x00FF00FF;
			return (sag | srb) + (Bsag | Bsrb);
			}


		/**
		* Return the color obtained by blending colorB over this one.
		* Colors are assumed to have pre-multiplied alpha.
		*
		* @param	colorB 	The 'top' color to blend over this one.
		* @param	opacity	The opacity to multiply colorB alpha channel with before blending, must be in
		* 					the range [0, 0x100] (use convertAlpha_0xFF_to_0x100() to convert a value in
		* 					[0,0xFF] to this range).
		*
		* @return	the color obtained by blending colorB over this.
		**/
		MTOOLS_FORCEINLINE RGBc get_blend(const RGBc colorB, const float opacity) const
			{
			MTOOLS_ASSERT((opacity >= 0.0f) && (opacity <= 1.0f));
			return get_blend(colorB, (uint32)(256 * opacity));
			}


		/**
		 * Return the color obtained by blending colorB in RGBc64 format over this one.
		 * Colors are assumed to have pre-multiplied alpha.
		 *
		 * @param	colorB  The 'top' color to blend over this one, in RGBc64 format.
		 * @param	N	    Normalisation to use with colorB (must be >0).
		 * @param	opacity The opacity to multiply colorB alpha channel with before blending, must be in
		 * 					the range [0, 0x100] (use convertAlpha_0xFF_to_0x100() to convert a value in
		 * 					[0,0xFF] to this range).
		 *
		 * @return	The blended color.
		 */
		MTOOLS_FORCEINLINE RGBc get_blend(const RGBc64 & colorB, const uint32 N, const uint32 opacity) const;

 
		/**
		 * blend colorB (in RGBc64 format) over this color.
		 * Colors are assumed to have pre-multiplied alpha.
		 *
		 * @param	colorB  The 'top' color to blend over this one, in RGBc64 format.
		 * @param	N	    Normalisation to use with colorB (must be >0).
		 * @param	opacity The opacity to multiply colorB alpha channel with before blending, must be in
		 * 					the range [0, 0x100] (use convertAlpha_0xFF_to_0x100() to convert a value in
		 * 					[0,0xFF] to this range).
		 */
		MTOOLS_FORCEINLINE void blend(const RGBc64 & colorB, const uint32 N, const uint32 opacity) { *this = get_blend(colorB,  N, opacity); }


		/**
		 * Return the color obtained by blending colorB in RGBc64 format over this one while artificially
		 * removing all the fully transparent white pixels that compose coul.
		 *
		 * @param	coul The 'top' color to blend over this one, in RGBc64 format. All transparent white
		 * 				 pixels are removed.
		 * @param	N    Normalisation to use (must be >0).
		 * @param	op   The opacity to multiply before blending, must be in the range [0.0f, 1.0f].
		 *
		 * @return	The blended color.
		 */
		MTOOLS_FORCEINLINE RGBc get_blend_removeWhite(const RGBc64 & coul, const uint32 N, const float op) const;


		/**
		 * Blend colorB (in RGBc64 format) over this color while artificially removing all the fully
		 * transparent white pixels that compose coul.
		 *
		 * @param	coul The 'top' color to blend over this one, in RGBc64 format. All transparent white
		 * 				 pixels are removed.
		 * @param	N    Normalisation to use (must be >0).
		 * @param	op   The opacity to multiply before blending, must be in the range [0.0f, 1.0f].
		 */
		MTOOLS_FORCEINLINE void blend_removeWhite(const RGBc64 & coul, const uint32 N, const float op) { *this = get_blend_removeWhite(coul, N,  op); }


		/**
		* Return the color obtained by blending colorB in RGBc64 format over this one while artificially
		* removing all the fully transparent black pixels that compose coul.
		*
		* @param	coul The 'top' color to blend over this one, in RGBc64 format. All transparent black
		* 				 pixels are removed.
		* @param	N    Normalisation to use (must be >0).
		* @param	op   The opacity to multiply before blending, must be in the range [0.0f, 1.0f].
		*
		* @return	The blended color.
		*/
		MTOOLS_FORCEINLINE RGBc get_blend_removeBlack(const RGBc64 & coul, const uint32 N, const float op) const;


		/**
		 * Blend colorB (in RGBc64 format) over this color while artificially removing all the fully
		 * transparent black pixels that compose coul.
		 *
		 * @param	coul The 'top' color to blend over this one, in RGBc64 format. All transparent black
		 * 				 pixels are removed.
		 * @param	N    Normalisation to use (must be >0).
		 * @param	op   The opacity to multiply before blending, must be in the range [0.0f, 1.0f].
		 */
		MTOOLS_FORCEINLINE void blend_removeBlack(const RGBc64 & coul, const uint32 N, const float op) { *this = get_blend_removeBlack(coul, N, op); }




		/****************************************************************************
		* PALETTE METHODS
		* 
		* DEPRECATED : Use the ColorPalette class instead. 
		* 
		*****************************************************************************/


        /**
         * A color of the palette in linear scale for a value between
         * 0 and 1.
         *
         * @param   v   the value in [0,1].
         *
         * @return  the corresponding color. A boundary color if v is outside of [0,1].
         **/
        static inline RGBc jetPalette(double v)
            {
            int i = (int)(72 * v); i = ((i<0) ? 0 : ((i>71) ? 71 : i));
            return jetPaletteRaw(i);
            }


        /**
         * A color of the palette in linear scale. Linear interpolation for the value in the range
         * [min(A,B),max(A,B)].
         *
         * @param   value   the value to convert to a color.
         * @param   a       lower/upper bound for the value range.
         * @param   b       lower/upper bound for the value range.
         *
         * @return  the corresponding color. A boundary color if value is outside of [min(A,B),max(A,B)].
         **/
        static inline RGBc jetPalette(int64 value, int64 a, int64 b = 0)
            {
			if (b < a) { mtools::swap<int64>(a, b); }
            if (b - a <= 0) { if (value < a) return jetPalette(0.0); else if (value > b) return jetPalette(1.0); return jetPalette(0.5); }
            return jetPalette(((double)(value - a)) / ((double)(b - a)));
            }


        /**
         * A color of the palette in logarithmic scale. 
         * Logaritmic interpolation of the color between [0,1].
         * The color interval are of the form [0,a1] [a1,a2] ... [a70,a71] [a71,1]
         * where |ai+1- ai| = exponent * |ai - ai-1|
         * 
         * @param   v           the value in [0,1]
         * @param   exponent    the exponent (default = 2).
         *
         * @return  the corresponding color. A boundary color if v is outside of [0,1].
         **/
        static inline RGBc jetPaletteLog(double v, double exponent = 2)
            {
            if ((v <= 0.0) || (v >= 1.0) || (exponent == 1.0)) return jetPalette(v);	// linear scale
            if (exponent < 1.0)  return jetPaletteLog(1.0 - v, 1.0 / exponent); // exponent < 1.0 obtain from exponent > 1.0 by symetry
            double ee = pow(exponent, 72); double epsilon = ((exponent - 1.0) / (ee - 1));
            if (epsilon == 0.0) return jetPaletteLog(v, exponent / 2); // exponent too large, we divide it by 2
            double x = (log(v*(exponent - 1) / epsilon + 1) / log(exponent));
            return jetPalette(x / 72.0);
            }


        /**
         * A color of the palette in logarithmic scale.
         * 
         * Logaritmic interpolation of the color between [A,B] (or [B,A] if
         * B\<A). The color interval are of the form 
         * [A,A1] [A1,A2] ...  [A70,A71] [A71,B] 
         * where |Ai+1 - Ai| = exponent * |Ai - Ai-1|.
         *
         * @param   value       The value.
         * @param   a           lower/upper bound for the value range.
         * @param   b           lower/upper bound for the value range.
         * @param   exponent    the exponent (default = 2).
         *
         * @return  the corresponding color. A boundary color if value outside of [min(A,B),max(A,B)].
         **/
        static inline RGBc jetPaletteLog(int64 value, int64 a, int64 b, double exponent = 2)
            {
            if (b < a)  { int64 c = a; a = b; b = c; }
            if ((b == a) || (value < a) || (value > b)) { jetPalette(value, a, b); } // linear scale in this case
            return jetPaletteLog(((double)(value - a)) / ((double)(b - a)), exponent);
            }


        /**
         * Return a color of the palette (72 colors). The palette is similar to matlab's jet palette. 
         *
         * @param   i   Zero-based index of the color (i < 72).
         *
         * @return  The color.
         **/
        static inline RGBc jetPaletteRaw(size_t i)
            {
            MTOOLS_ASSERT(i<72);
            static const RGBc tab[] = { 
                RGBc(0, 0, 127), RGBc(0, 0, 141), RGBc(0, 0, 155), RGBc(0, 0, 169), RGBc(0, 0, 183), RGBc(0, 0, 198), RGBc(0, 0, 212), RGBc(0, 0, 226), RGBc(0, 0, 240),
                RGBc(0, 0, 255), RGBc(0, 14, 255), RGBc(0, 28, 255), RGBc(0, 42, 255), RGBc(0, 56, 255), RGBc(0, 70, 255), RGBc(0, 84, 255), RGBc(0, 98, 255), RGBc(0, 112, 255),
                RGBc(0, 127, 255), RGBc(0, 141, 255), RGBc(0, 155, 255), RGBc(0, 169, 255), RGBc(0, 183, 255), RGBc(0, 198, 255), RGBc(0, 212, 255), RGBc(0, 226, 255), RGBc(0, 240, 255),
                RGBc(0, 255, 255), RGBc(14, 255, 240), RGBc(28, 255, 226), RGBc(42, 255, 212), RGBc(56, 255, 198), RGBc(70, 255, 183), RGBc(84, 255, 169), RGBc(98, 255, 155), RGBc(112, 255, 141),
                RGBc(127, 255, 127), RGBc(141, 255, 112), RGBc(155, 255, 98), RGBc(169, 255, 84), RGBc(183, 255, 70), RGBc(198, 255, 56), RGBc(212, 255, 42), RGBc(226, 255, 28), RGBc(240, 255, 14),
                RGBc(255, 255, 0), RGBc(255, 240, 0), RGBc(255, 226, 0), RGBc(255, 212, 0), RGBc(255, 198, 0), RGBc(255, 183, 0), RGBc(255, 169, 0), RGBc(255, 155, 0), RGBc(255, 141, 0),
                RGBc(255, 127, 0), RGBc(255, 112, 0), RGBc(255, 98, 0), RGBc(255, 84, 0), RGBc(255, 70, 0), RGBc(255, 56, 0), RGBc(255, 42, 0), RGBc(255, 28, 0), RGBc(255, 14, 0),
                RGBc(255, 0, 0), RGBc(240, 0, 0), RGBc(226, 0, 0), RGBc(212, 0, 0), RGBc(198, 0, 0), RGBc(183, 0, 0), RGBc(169, 0, 0), RGBc(155, 0, 0), RGBc(141, 0, 0) };
            return tab[i];
            }


        /**
         * Gets colors which are relatively distinct (32 colors).
         *
         * @param   i   Zero-based index of the color.
         *
         * @return  The color.
         **/
        static inline RGBc getDistinctColor(size_t i)
            {
            const RGBc tab[] = { RGBc(0xFF, 0x00, 0x00), RGBc(0x00, 0x00, 0xFF), RGBc(0x00, 0xFF, 0x00), RGBc(0x01, 0xFF, 0xFE), RGBc(0xFF, 0xA6, 0xFE), RGBc(0xFF, 0xDB, 0x66), RGBc(0x00, 0x64, 0x01), RGBc(0xFE, 0x89, 0x00),
                RGBc(0x95, 0x00, 0x3A), RGBc(0x00, 0x7D, 0xB5), RGBc(0x7E, 0x2D, 0xD2), RGBc(0x6A, 0x82, 0x6C), RGBc(0x77, 0x4D, 0x00), RGBc(0x90, 0xFB, 0x92), RGBc(0x01, 0x00, 0x67), RGBc(0xD5, 0xFF, 0x00),
                RGBc(0xFF, 0x93, 0x7E), RGBc(0xFF, 0xFF, 0x10), RGBc(0xFF, 0x02, 0x9D), RGBc(0x00, 0x76, 0xFF), RGBc(0x7A, 0x47, 0x82), RGBc(0xBD, 0xD3, 0x93), RGBc(0x85, 0xA9, 0x00), RGBc(0xFF, 0x00, 0x56),
                RGBc(0xA4, 0x24, 0x00), RGBc(0x00, 0xAE, 0x7E), RGBc(0x68, 0x3D, 0x3B), RGBc(0xBD, 0xC6, 0xFF), RGBc(0x26, 0x34, 0x00), RGBc(0xFF, 0x00, 0xF6), RGBc(0x00, 0xB9, 0x17), RGBc(0x00, 0x00, 0x00) };
            return tab[i & 31];
            }



		/****************************************************************************
		* MISC
		*****************************************************************************/

		/**
		* Convert the RGBc object into a std::string.
		*
		* @return  A std::string that represent the color in the form "RGB(rrr,ggg,bbb)".
		**/
		std::string toString() const 
			{ 
			OSS os; 
			os << "RGBc(" << comp.R << "," << comp.G << ","  << comp.B << ":"  << opacity() << ")";
			return os.str();
			}


        /**
         * Serialize/deserialize the object. The method work for boost and the custom serialization classe. 
         * (the method is used for both serialization and deserialization).
         **/
        template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0) const
            {
            ar & comp.R;
            ar & comp.G;
            ar & comp.B;
            ar & comp.A;
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
			}


		/****************************************************************************
		* PREDEFINED COLORS
		*****************************************************************************/


        static const RGBc c_Black;            ///< color black
        static const RGBc c_White;            ///< color white
        static const RGBc c_Red;              ///< color red
        static const RGBc c_Blue;             ///< color blue
        static const RGBc c_Green;            ///< color green
        static const RGBc c_Purple;           ///< color purple
        static const RGBc c_Orange;           ///< color orange
        static const RGBc c_Cyan;             ///< color cyan
        static const RGBc c_Lime;             ///< color lime
        static const RGBc c_Salmon;           ///< color salmon
        static const RGBc c_Maroon;           ///< color maroon
        static const RGBc c_Yellow;           ///< color yellow
        static const RGBc c_Magenta;          ///< color magenta
        static const RGBc c_Olive;            ///< color olive
        static const RGBc c_Teal;             ///< color teal
        static const RGBc c_Gray;             ///< color gray
        static const RGBc c_Silver;           ///< color silver
        static const RGBc c_Navy;             ///< color navy blue
		static const RGBc c_Transparent;	  ///< fully transparent color


    };




	/****************************************************************************
	* EXTERNAL (HELPER) FUNCTIONS. 
	*****************************************************************************/


    /**
    * Return the same color with a given opacity.
	* Colors are assumed to have pre-multiplied alpha.
	*
    * @param   color   The color.
    * @param   op      The new opacity between 0.0f and 1.0f
    *
    * @return the same color with the new opacity
    **/
	MTOOLS_FORCEINLINE RGBc opacity(mtools::RGBc color, float op) { return color.getOpacity(op); }


	/**
	* Return the same color with the opacity multiplied by a given factor.
	* Colors are assumed to have pre-multiplied alpha.
	*
	* @param   color   The color.
	* @param   op      The multiplication factor for the opacity between 0.0f and 1.0f.
	*
	* @return the same color with the new opacity.
	**/
	MTOOLS_FORCEINLINE RGBc multOpacity(mtools::RGBc color, float op) { return color.getMultOpacity(op); }


	/**
	* Blend color B over color A.
	* Colors are assumed to have pre-multiplied alpha.
	*
	* @param	colorA	bottom color.
	* @param	colorB	top color.
	*
	* @return  colorB over colorA.
	**/
	MTOOLS_FORCEINLINE RGBc blend(RGBc colorA, RGBc colorB) { return colorA.get_blend(colorB); }


	/**
	* Blend color B over color A.
	* Colors are assumed to have pre-multiplied alpha.
	*
	* @param	colorA 	bottom color.
	* @param	colorB 	top color.
	* @param	opacity	The opacity to multiply colorB before blending. Must be
	* 					in the range [0x100] (use convertAlpha_0xFF_to_0x100() to convert from uchar
	* 					to this range).
	*
	* @return  colorB over colorA.
	**/
	MTOOLS_FORCEINLINE RGBc blend(RGBc colorA, RGBc colorB, uint32 opacity) { return colorA.get_blend(colorB,opacity); }


	/**
	* Blend color B over color A.
	* Colors are assumed to have pre-multiplied alpha.
	*
	* @param	colorA 	bottom color.
	* @param	colorB 	top color.
	* @param	opacity	The opacity to multiply colorB before blending. Must be
	* 					in the range [0.0f,1.0f]
	*
	* @return  colorB over colorA.
	**/
	MTOOLS_FORCEINLINE RGBc blend(RGBc colorA, RGBc colorB, float opacity) { return colorA.get_blend(colorB, opacity); }








/**
* A color in (B,G,R,A) format with 16 bit precision per channel.
**/
union RGBc64
    {

    public:

        /*
		*****************************************************************************
		* LAYOUT
		*****************************************************************************
		* Color ordered in BGRA layout
        */
        uint64 color;

        struct _RGBc64_component   // color seen as 4 unsigned char in BGRA layout 
            {
            uint16 B;    // word 1 : Blue.  (low word in uint64)
            uint16 G;    // word 2 : Green.
            uint16 R;    // word 3 : Red
            uint16 A;    // word 4 : alpha. (high word in uint64)
            } comp;

		static const uint8 OPAQUEALPHA = 255;       ///< fully opaque
		static const uint8 TRANSPARENTALPHA = 0;    ///< fully transparent
		

		/****************************************************************************
		* CONSTRUCTOR / CONVERSION
		*****************************************************************************/


        /** Default constructor. */
        RGBc64() = default;


        /** Constructor from uint64 */
        RGBc64(uint64 col) : color(col) 
			{
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
			}


        /**
        * Constructor.
        *
        * @param   r   color red.
        * @param   g   color green.
        * @param   b   color black.
        * @param   a   alpha channel.
        **/
        RGBc64(uint16 r, uint16 g, uint16 b, uint16 a = OPAQUEALPHA)
            {
            color = ((uint64)b) + (((uint64)g) << 16) + (((uint64)r) << 32) + (((uint64)a) << 48);
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
            }


        /**
        * Default copy constructor.
        **/
        RGBc64(const RGBc64 & c) = default;


        /**
        * Constructor from a RGBc
        **/
        RGBc64(const RGBc & c) 
            {
            const uint64 v = c.color;
            color = (v & 255) +
                ((v & (((uint64)255) << 8)) << 8) +
                ((v & (((uint64)255) << 16)) << 16) +
                ((v & (((uint64)255) << 24)) << 24);
            }


        /**
        * Default assignment operator.
        **/
        RGBc64 & operator=(const RGBc64 & c) = default;


        /**
        * assignment from RGBc.
        **/
        inline RGBc64 & operator=(const RGBc & c)
            {
            const uint64 v = c.color;
            color = (v & 255) +
                ((v & (((uint64)255) << 8)) << 8) +
                ((v & (((uint64)255) << 16)) << 16) +
                ((v & (((uint64)255) << 24)) << 24);
            return(*this);
            }


        /**
         * Converts the color into a RGBc without normalization.
         **/
        explicit inline operator RGBc() const { return RGBc(*this); }


		/**
		* Get the color in RGBc format without normalization.
		**/
		inline RGBc getRGBc() { return RGBc(*this);  }


		/**
		* Get the color in RGBc format and normalize it with a given multiplier n.
		**/
		inline RGBc getRGBc(int n)
			{
			RGBc64 c(*this);
			c.normalize(n);
			return RGBc(c);
			}


		/****************************************************************************
		* COMPARISON
		*****************************************************************************/

        /**
        * Equality operator.
        **/
        inline bool operator==(const RGBc & c) const { return (color == c.color); }


        /**
        * Inequality operator.
        **/
        inline bool operator!=(const RGBc & c) const { return (color != c.color); }



		/****************************************************************************
		* TRANSFORMATION / NORMALIZATION
		*****************************************************************************/


        /**
        * Sum of color. Each component is summed. Do not check for overflow.
        **/
        inline void add(const RGBc & c)
            {
            const uint64 v = c.color;
            color += (v & 255) +
                ((v & (((uint64)255)<< 8 )) << 8) +
                ((v & (((uint64)255)<< 16)) << 16) +
                ((v & (((uint64)255)<< 24)) << 24); 
            }


        /**
         * Sum of color. Each component is summed. Do not check for overflow.
         **/
        inline void add(const RGBc64 & c)
            {
            color += c.color;
            }


        /**
         * Normalizes the color
         **/
        inline void normalize(int n)
            {
            comp.G /= n;
            comp.B /= n;
            comp.R /= n;
            comp.A /= n;
            }


        /**
        * Divide by two the value of each channel
        **/
        inline void div2()
            {
            const uint64 mask = (((uint64)32767) | (((uint64)32767) << 16) | (((uint64)32767) << 32) | (((uint64)32767) << 48));
            color >>= 1;    // divide by 2.
            color &= mask;  // remove bits that may have overflow 
            }



		/****************************************************************************
		* MISC
		*****************************************************************************/

		/**
		* Convert the RGBc64 object into a std::string.
		**/
		std::string toString() const { return std::string("RGBc64(") + mtools::toString(comp.R) + "," + mtools::toString(comp.G) + "," + mtools::toString(comp.B) + "," + mtools::toString(comp.A) + ")"; }


        /**
        * Serialize/deserialize the object. The method work for boost and the custom serialization classe.
        * (the method is used for both serialization and deserialization).
        **/
        template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0) const
            {
            ar & comp.R;
            ar & comp.G;
            ar & comp.B;
            ar & comp.A;
			MTOOLS_ASSERT(comp.R <= comp.A);
			MTOOLS_ASSERT(comp.G <= comp.A);
			MTOOLS_ASSERT(comp.B <= comp.A);
            }

    };




	/****************************************************************************
	* IMPLEMENTATION OF METHODS THAT DEPEND ON RGBc AND RGBc64
	*****************************************************************************/


	MTOOLS_FORCEINLINE void RGBc::fromRGBc64(const RGBc64 & coul)
		{
		const uint64 cc = coul.color;
		const uint64 r = (cc & ((uint64)255)) +
			((cc >> 8)  & (((uint64)255) << 8)) +
			((cc >> 16) & (((uint64)255) << 16)) +
			((cc >> 24) & (((uint64)255) << 24));
		color = (uint32)(r);
		MTOOLS_ASSERT(comp.R <= comp.A);
		MTOOLS_ASSERT(comp.G <= comp.A);
		MTOOLS_ASSERT(comp.B <= comp.A);
		}


	MTOOLS_FORCEINLINE void RGBc::fromRGBc64(const RGBc64 & coul, const uint32 N)
		{
		comp.R = (uint8)(coul.comp.R / N);
		comp.G = (uint8)(coul.comp.G / N);
		comp.B = (uint8)(coul.comp.B / N);
		comp.A = (uint8)(coul.comp.A / N);
		}


	MTOOLS_FORCEINLINE RGBc RGBc::get_blend(const RGBc64 & colorB, const uint32 N, const uint32 opacity) const
		{
		return get_blend(RGBc(colorB, N), opacity);
		}


	MTOOLS_FORCEINLINE RGBc RGBc::get_blend_removeWhite(const RGBc64 & coul, const uint32 N, const float op) const
		{
		MTOOLS_DEBUG("*** DEPRECATED METHOD [RGBc RGBc::get_blend_removeWhite] ***");
		if (coul.comp.A == 0) return *this;
		const float g = ((float)(N * 255)) / ((float)coul.comp.A);
		const float nR = g*(((float)coul.comp.R / N) - 255) + 255;
		const float nG = g*(((float)coul.comp.G / N) - 255) + 255;
		const float nB = g*(((float)coul.comp.B / N) - 255) + 255;
		const float alpha = op / g;
		const float beta = 1.0f - alpha;
		return RGBc(
			(uint8)(beta*comp.R + (alpha*nR)),
			(uint8)(beta*comp.G + (alpha*nG)),
			(uint8)(beta*comp.B + (alpha*nB)),
			255);
		}


	/* blending and artificially removing fully transparent black pixel */
	MTOOLS_FORCEINLINE RGBc RGBc::get_blend_removeBlack(const RGBc64 & coul, const uint32 N, const float op) const
		{
		MTOOLS_DEBUG("*** DEPRECATED METHOD [RGBc RGBc::get_blend_removeBlack] ***");
		if (coul.comp.A == 0) return *this;
		const float g = ((float)(N * 255)) / ((float)coul.comp.A);
		const float goverN = g / N;
		const float nR = goverN*coul.comp.R;
		const float nG = goverN*coul.comp.G;
		const float nB = goverN*coul.comp.B;
		const float alpha = op / g;
		const float beta = 1.0f - alpha;
		return RGBc(
			(uint8)(beta*comp.R + (alpha*nR)),
			(uint8)(beta*comp.G + (alpha*nG)),
			(uint8)(beta*comp.B + (alpha*nB)),
			255);
		}



}







/* end of file */