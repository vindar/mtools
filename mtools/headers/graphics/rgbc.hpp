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

    /* forward definition */
    union RGBc64;

    /**
     * A color in (R,G,B,A) format.
     **/
    union RGBc
    {

    public:

        /*
         * Color is ordered in RGBA layout : Reversed compared with Fl_Color ! 
         */
        uint32 color;       // color seen as a uint32 : low byte is R, high byte is A      

        struct _RGBc_component   // color seen as 4 unsigned char in RGBA layout 
            {
            uint8 R;    // byte 1 : Red.  (low byte in uint32)
            uint8 G;    // byte 2 : Green.
            uint8 B;    // byte 3 : Blue
            uint8 A;    // byte 4 : alpha. (high byte in uint32)
            } comp;


        static const uint8 DEFAULTALPHA = 255; ///< default value for transparency : fully opaque

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
         * @param   a   alpha channel (DEFAULTALPHA)
         **/
        RGBc(uint8 r, uint8 g, uint8 b, uint8 a = DEFAULTALPHA) : comp{r,g,b,a} {}


        /**
        * Default copy constructor.
        **/
        RGBc(const RGBc & c) = default;


        /**
         * Constructor from a RGBc64.
         *
         * \param   c   The color.
         **/
        inline RGBc(const RGBc64 & c);


        /**
         * Constructor from a Fl_Color type.
         *
         * @param   c   the color.
         * @param   a   the transparency
         **/
        RGBc(Fl_Color c, int a = DEFAULTALPHA) 
            {
            Fl::get_color(c, comp.R, comp.G, comp.B);
            comp.A = a;
            }


        /**
         * Constructor from a buffer. Use all 4 bytes.
         **/
        RGBc(const unsigned char * p) : color(*((uint32*)p)) {}
        

        /**
         * Constructor from a buffer. Use only 3 bytes and set the alpha value separately
         **/
        RGBc(const unsigned char * p, unsigned char a) : comp{ p[0], p[1], p[2], 0 }
            {
            comp.A = a;
            }


        /**
         * Default assignment operator.
         **/
        RGBc & operator=(const RGBc & c) = default;


        /**
        * assignment operator from RGBc64.
        **/
        RGBc & operator=(const RGBc64 & c);


        /**
         * Assignment operator from a Fl_Color, alpha value set to DEFAULTALPHA.
         **/
        RGBc & operator=(Fl_Color c) 
            {
            Fl::get_color(c, comp.R, comp.G, comp.B);
            comp.A = DEFAULTALPHA;
            return(*this);
            }


        /**
         * Assignment operator from a buffer. Use all 4 bytes.
         *
         * @warning The buffer must have size at least 4 !
         * @param   p   The buffer of size 3 with p[0] = red, p[1] = green, p[2] = blue, p[3] = alpha.
         **/
        RGBc & operator=(const unsigned char * p) { color = *((uint32*)p);  return(*this); }


        /**
        * Converts the color into a RGBc64.
        **/
        explicit operator RGBc64() const;


        /**
         * Converts the color into a Fl_Color type (ignore the alpha channel).
         **/
        explicit operator Fl_Color() const { return fl_rgb_color(comp.R, comp.G, comp.B); }


        /**
         * The buffer for the R G B A color.
         **/
        unsigned char* buf() { return((unsigned char *)&color); }


        /**
         * The buffer for the R G B A color (const version).
         **/
        const unsigned char* buf() const { return((unsigned char *)&color); }


        /**
         * Equality operator.
         **/
        bool operator==(const RGBc & c) const { return (color == c.color); }


        /**
         * Inequality operator.
         **/
        bool operator!=(const RGBc & c) const { return (color != c.color); }


        /**
         * Convert the RGBc object into a std::string.
         *
         * @return  A std::string that represent the color in the form "RGB(rrr,ggg,bbb)".
         **/
        std::string toString() const { return std::string("RGBc(") + mtools::toString(comp.R) + "," + mtools::toString(comp.G) + "," + mtools::toString(comp.B) + ":" + mtools::toString(opacity()) + ")"; }


        /**
         * Makes the color fully transparent.
         **/
        inline void makeTransparent() { comp.A = 0; }


        /**
         * Return the same color but transparent
         *
         * @return  The same color but with its alpha channel set to 0.
         **/
        inline RGBc getTransparent() const { RGBc c(*this); c.comp.A = 0; return c; }


        /**
         * Makes the color fully opaque.
         **/
        inline void makeOpaque() { comp.A = 255; }


        /**
         * Return the same color but opaque
         *
         * @return  The same color but with its alpha channel set to 255.
         **/
        inline RGBc getOpaque() const { RGBc c(*this); c.comp.A = 255; return c; }


        /**
         * Give the color the default transparency (DEFAULTALPHA).
         **/
        inline void makeDefaultTransparency() { comp.A = DEFAULTALPHA; }


        /**
         * Return the opacity of the color as a float.
         *
         * @return  The opacity between 0.0 (fully transparent) and 1.0 (fully opaque). 
         **/
        inline float opacity() const { return(((float)comp.A) / ((float)255.0)); }


        /**
         * Set the opacity of the color.
         *
         * @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
         **/
        inline void opacity(float o) { MTOOLS_ASSERT((o >= 0.0) && (o <= 1.0)); comp.A = (uint8)(o * 255); }


        /**
         * Return the same color but with a given opacity.
         *
         * @param   o   the opacity between 0.0 (transparent) and 1.0 (opaque)
         **/
        inline RGBc getOpacity(float o) const { MTOOLS_ASSERT((o >= 0.0) && (o <= 1.0));  RGBc c(*this); c.comp.A = (uint8)(o * 255); return c; }


        /**
        * Create a blending with another background color using the 'over' operator.
        *
        * @param   B       The color to blend over (background color)
        *
        * @return the 'this over B' color
        *
        * TODO : make it faster !!!! 
        *
        **/
        inline RGBc over(RGBc coulB) const
            {
            const float op = opacity();
            const float po = 1.0f - op;
            const float opB = coulB.opacity();
            const float nop = op + opB*po;
            const int nR = (int)((comp.R*op + coulB.comp.R*opB*po) / nop);
            const int nG = (int)((comp.G*op + coulB.comp.G*opB*po) / nop);
            const int nB = (int)((comp.B*op + coulB.comp.B*opB*po) / nop);
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
        template<typename ARCHIVE> void serialize(ARCHIVE & ar, const int version = 0) const
            {
            ar & comp.R;
            ar & comp.G;
            ar & comp.B;
            ar & comp.A;
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
        static const RGBc c_TransparentBlack; ///< transparent white color
        static const RGBc c_TransparentRed;   ///< transparent white color
        static const RGBc c_TransparentGreen; ///< transparent white color
        static const RGBc c_TransparentBlue;  ///< transparent white color

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










/**
* A color in (R,G,B,A,N) format with 14 bit precision per channel.
**/
union RGBc64
    {

    public:

        /*
        * Color ordered in RGBA layout
        */
        uint64 color;
        struct _RGBc64_component   // color seen as 4 unsigned char in RGBA layout 
            {
            uint16 R;    // byte 1 : Red.  (low byte in uint32)
            uint16 G;    // byte 2 : Green.
            uint16 B;    // byte 3 : Blue
            uint16 A;    // byte 4 : alpha. (high byte in uint32)
            } comp;

        static const uint8 DEFAULTALPHA = 255; ///< default value for transparency : fully opaque


        /** Default constructor. */
        RGBc64() = default;

        /**
        * Constructor.
        *
        * @param   r   color red.
        * @param   g   color green.
        * @param   b   color black.
        * @param   a   alpha channel (DEFAULTALPHA)
        **/
        RGBc64(uint16 r, uint16 g, uint16 b, uint16 a = DEFAULTALPHA)
            {
            color = ((uint64)r) + (((uint64)g) << 16) + (((uint64)b) << 32) + (((uint64)a) << 48);
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
         * Converts the color into a RGBc. Truncate all bits over the 8 first bits of each component.
         **/
        explicit inline operator RGBc() const 
            {
            return RGBc(*this);
            }


        /**
        * The buffer for the R G B A color.
        **/
        inline unsigned char* buf() { return((unsigned char *)&color); }


        /**
        * The buffer for the R G B A color (const version).
        **/
        inline const unsigned char* buf() const { return((unsigned char *)&color); }


        /**
        * Equality operator.
        **/
        inline bool operator==(const RGBc & c) const { return (color == c.color); }


        /**
        * Inequality operator.
        **/
        inline bool operator!=(const RGBc & c) const { return (color != c.color); }


        /**
        * Convert the RGBc64 object into a std::string.
        **/
        std::string toString() const { return std::string("RGBc64(") + mtools::toString(comp.R) + "," + mtools::toString(comp.G) + "," + mtools::toString(comp.B) + "," + mtools::toString(comp.A) + ")"; }


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
            comp.R /= n;
            comp.G /= n;
            comp.B /= n;
            comp.A /= n;
            }

        /**
        * Get the color in RGBc format without normalization.
        **/
        inline RGBc getRGBc()
            {
            return RGBc(*this);
            }

        /**
         * Get the color in RGBc format and normalize it with a given multiplier n. 
         **/
        inline RGBc getRGBc(int n)
            {
            RGBc64 c(*this);
            c.normalize(n);
            return RGBc(c);
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
            }

    };



    inline RGBc::RGBc(const RGBc64 & c)
        {
        const uint64 cc = c.color;
        const uint64 r = (cc & ((uint64)255)) + 
                         ((cc >> 8)  & (((uint64)255) << 8)) + 
                         ((cc >> 16) & (((uint64)255) << 16)) + 
                         ((cc >> 24) & (((uint64)255) << 24));
        color = (uint32)(r);
        }


    inline RGBc & RGBc::operator=(const RGBc64 & c)
        {
        const uint64 cc = c.color;
        const uint64 r = (cc & ((uint64)255)) +
            ((cc >> 8)  & (((uint64)255) << 8)) +
            ((cc >> 16) & (((uint64)255) << 16)) +
            ((cc >> 24) & (((uint64)255) << 24));
        color = (uint32)(r);
        return(*this);
        }


    inline RGBc::operator RGBc64() const
        {
        return RGBc64(*this);
        }


}







/* end of file */