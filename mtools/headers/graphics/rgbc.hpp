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

#include <FL/Fl.H>

#include "../misc/error.hpp"
#include "../misc/misc.hpp"
#include "../misc/stringfct.hpp"

#include <algorithm>
#include <string>


namespace mtools
{

    /**
     * A color in (R,G,B,A) format.
     **/
    class RGBc
    {

    public:

        static const uint8 DEFAULTALPHA = 255; ///< default value for transparency : fully opaque

        /**
         * Default constructor. Create an instance in Color black with default transparency.
         **/
        RGBc() : R(0), G(0), B(0), A(DEFAULTALPHA) {}


        /**
         * Constructor.
         *
         * @param   r   color red.
         * @param   g   color green.
         * @param   b   color black.
         * @param   a   alpha channel (DEFAULTALPHA)
         **/
        RGBc(int r, int g, int b, int a = DEFAULTALPHA) : R(r), G(g), B(b), A(a) {}


        /**
         * Constructor from a Fl_Color type.
         *
         * @param   c   the color.
         * @param   a   the transparency (DEFAULTALPHA)
         **/
        RGBc(Fl_Color c, int a = DEFAULTALPHA) { operator=(c); A = a; }


        /**
         * Constructor from a buffer.
         *
         * @param   p               the buffer of sizeat least  3 with p[0] = RED p[1] = GREEN, p[2] =
         *                          BLUE and eventually p[3] = alpha.
         * @param   useDefaultAlpha true to use the default alpha value DEFAULTALPHA (in this case, the
         *                          buffer need only to have size >=3). If flase, the buffer must have
         *                          size at least 4 and p[3] contain the alpha value.
         **/
        RGBc(const unsigned char * p, bool useDefaultAlpha = true) : R(p[0]), G(p[1]), B(p[2]) 
            {
            if (useDefaultAlpha) A = DEFAULTALPHA; else A = p[3];
            }


        /**
         * Default copy constructor.
         **/
        RGBc(const RGBc & c) = default;


        /**
         * Default assignment operator.
         **/
        RGBc & operator=(const RGBc & c) = default;


        /**
         * Assignment operator from a Fl_Color, alpha value set to DEFAULTALPHA.
         **/
        RGBc & operator=(Fl_Color c) 
            {
            if (c % 256 != 0) { Fl::get_color(c, R, G, B); return(*this); };
            c = c >> 8; B = c % 256;
            c = c >> 8; G = c % 256;
            c = c >> 8; R = c % 256;
            A = DEFAULTALPHA;
            return(*this);
            }


        /**
         * Assignment operator from a buffer. 
         *
         * @warning The buffer must have size at least 4 !
         * @param   p   The buffer of size 3 with p[0] = red, p[1] = green, p[2] = blue, p[3] = alpha.
         **/
        RGBc & operator=(const unsigned char * p) { R = p[0]; G = p[1]; B = p[2]; A = p[3];  return(*this); }


        /**
         * Converts the color into a Fl_Color type (ignore the alpha channel).
         **/
        operator Fl_Color() const { return fl_rgb_color(R, G, B); }


        /**
         * The buffer for the R G B A color.
         **/
        unsigned char* buf() { return(&R); }


        /**
         * The buffer for the R G B A color (const version).
         **/
        const unsigned char* buf() const { return(&R); }


        /**
         * Equality operator.
         **/
        bool operator==(const RGBc & c) const { return(((R == c.R) && (G == c.G) && (B == c.B)) && (A == c.A)); }


        /**
         * Inequality operator.
         **/
        bool operator!=(const RGBc & c) const { return (!(this->operator==(c))); }


        /**
         * Convert the RGBc object into a std::string.
         *
         * @return  A std::string that represent the color in the form "RGB(rrr,ggg,bbb)".
         **/
        std::string toString() const { return std::string("RGB(") + mtools::toString(R) + "," + mtools::toString(G) + "," + mtools::toString(B) + ":", mtools::toString(opacity()) + ")"; }


        /**
         * Makes the color fully transparent.
         **/
        inline void makeTransparent() { A = 0; }


        /**
         * Return the same color but transparent
         *
         * @return  The same color but with its alpha channel set to 0.
         **/
        inline RGBc getTransparent() const { RGBc c(*this); c.A = 0; return c; }


        /**
         * Makes the color fully opaque.
         **/
        inline void makeOpaque() { A = 255; }


        /**
         * Return the same color but opaque
         *
         * @return  The same color but with its alpha channel set to 255.
         **/
        inline RGBc getOpaque() const { RGBc c(*this); c.A = 255; return c; }


        /**
         * Give the color the default transparency (DEFAULTALPHA).
         **/
        inline void makeDefaultTransparency() { A = DEFAULTALPHA; }


        /**
         * Return the opacity of the color as a float.
         *
         * @return  The opacity between 0.0 (fully transparent) and 1.0 (fully opaque). 
         **/
        inline float opacity() const { return(((float)A) / ((float)255.0)); }


        /**
         * Set the opacity of the color.
         *
         * @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
         **/
        inline void opacity(float o) { MTOOLS_ASSERT((o >= 0.0) && (o <= 1.0)); A = (uint8)(o * 255); }


        /**
         * Return the same color but with a given opacity.
         *
         * @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
         **/
        inline RGBc getOpacity(float o) const { MTOOLS_ASSERT((o >= 0.0) && (o <= 1.0));  RGBc c(*this); c.A = (uint8)(o * 255); return c; }


        /**
        * Create a blending with another background color using the 'over' operator.
        *
        * @param   B       The color to blend over (background color)
        *
        * @return the 'this over B' color
        **/
        inline RGBc over(RGBc coulB) const
            {
            const float op = opacity();
            const float po = 1.0f - op;
            const float opB = coulB.opacity();
            const float nop = op + opB*po;
            const int nR = (int)((R*op + coulB.R*opB*po) / nop);
            const int nG = (int)((G*op + coulB.G*opB*po) / nop);
            const int nB = (int)((B*op + coulB.B*opB*po) / nop);
            return RGBc(nR, nG, nB, (int)(255*nop));
            }


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
            if (b < a)  {int64 c = a; a = b; b = c;}
            if (b == a) { if (value < a) return jetPalette(0.0); else if (value > b) return jetPalette(1.0); return jetPalette(0.5); }
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
            static const RGBc tab[] = { RGBc(0, 0, 127), RGBc(0, 0, 141), RGBc(0, 0, 155), RGBc(0, 0, 169), RGBc(0, 0, 183), RGBc(0, 0, 198), RGBc(0, 0, 212), RGBc(0, 0, 226), RGBc(0, 0, 240),
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
            return tab[i%32];
            }

        /**
         * Serialize/deserialize the object. The method work for boost and the custom serialization classe. 
         * (the method is used for both serialization and deserialization).
         **/
        template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0)
            {
            ar & R;
            ar & G;
            ar & B; 
            ar & A;
            }


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
        static const RGBc c_TransparentWhite; ///< transparent white color


        uint8 R;        ///< the Red component
        uint8 G;        ///< the Green component
        uint8 B;        ///< the Blue component
        uint8 A;        ///< the alpha channel


    };


    /**
     * Helper function, return a transparent version of the color.
     *
     * @param   color   The color.
     *
     * @return  the same color but with its alpha channel set to 0.
     **/
    inline RGBc transparent(mtools::RGBc color) { return color.getTransparent(); }


    /**
     * Helper function. Return an opaque version of the color
     *
     * @param   color   The color.
     *
     * @return the same color but with its alpha channel set to 255.
     **/
    inline RGBc opaque(mtools::RGBc color) { return color.getOpaque(); }


    /**
    * Helper function. Return the same color with a given opacity.
    *
    * @param   color   The color.
    * @param   op      The new opacity between 0.0f and 1.0f
    *
    * @return the same color with the new opacity
    **/
    inline RGBc opacity(mtools::RGBc color, float op) { return color.getOpacity(op); }



    /**
    * Return the blended color using the 'A over B' operator.
    *
    * @param   A       The first color.
    * @param   B       The secodn color 
    *
    * @return the 'A over B' color
    **/
    inline RGBc blendOver(RGBc A, RGBc B) { return A.over(B); }

}


/* end of file */