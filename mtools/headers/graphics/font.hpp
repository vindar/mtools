/** @file font.hpp */
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

#include "../mtools_config.hpp"
#include "../misc/misc.hpp"
#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "image.hpp"
#include "rgbc.hpp"
#include "../io/serialization.hpp"


namespace mtools
	{


	/**
	* Structure containing a glyph.
	*
	*  Glyph are images that store the apparence of a character in a Font object.
	**/
	struct Glyph
		{

		/** Default constructor: empty glyph. */
		Glyph() : offx(0), offy(0), width(0), glyph() {}

		/** serialize / deserialize the glyph. **/
		template<typename ARCHIVE> void serialize(ARCHIVE & ar) { ar & offx & offy & width & glyph; }

		int64 offx;		///< x offset to apply before drawing the glyph
		int64 offy;		///< y offset to apply before drawing the glyph.
		int64 width;	///< width of the glyph (may before larger than glyph.lx() ). 
		Image glyph;	///< The glyph image.
		};


	/**
	* Class object representing a font at a given size.
	*
	*
	*  Support the .bff font format of 'Codehead's bitmap font generator'. c.f.
	*  http://www.codehead.co.uk/cbfg/.
	**/
	class Font
		{

		public:

			/** Default constructor. Empty font */
			Font() : _fontsize(0), _tab() {}


			/**
			* Constructor from a .BFF file (Codehead's bitmap font generator format).
			* c.f. http://www.codehead.co.uk/cbfg/
			*
			* @param	filename	name of the .bff file (must be 32 bit color depth).
			* @param	fontsize	Size of the font. Null or negative value to set the font size equal to
			* 						the height of a cell in the .bff image.
			**/
			Font(const std::string & filename, int fontsize = 0) : _fontsize(fontsize), _tab()
				{
				std::string bff = mtools::loadStringFromFile(filename);
				if (bff.size() == 0) { MTOOLS_ERROR(std::string("readBFF() : Cannot read file [") + filename + "]"); }
				if (bff.size() <= 276) { MTOOLS_ERROR(std::string("readBFF() : File [") + filename + "] too small."); }
				if (((uint8)bff[0] != 0xBF) || ((uint8)bff[1] != 0xF2)) { MTOOLS_ERROR(std::string("readBFF() : incorrect BFF tag for file [") + filename + "]"); }
				const uint32 im_lx = *((uint32*)(bff.data() + 2));
				const uint32 im_ly = *((uint32*)(bff.data() + 6));
				const uint32 cell_lx = *((uint32*)(bff.data() + 10));
				if (cell_lx == 0) { MTOOLS_ERROR(std::string("readBFF() : incorrect cell_lx =0 for file [") + filename + "]"); }
				const uint32 cell_ly = *((uint32*)(bff.data() + 14));
				if (cell_ly == 0) { MTOOLS_ERROR(std::string("readBFF() : incorrect cell_ly =0 for file [") + filename + "]"); }
				const uint32 nbx = im_lx / cell_lx;
				const uint32 nby = im_ly / cell_ly;
				const uint8 bpp = (uint8)bff[18];
				if (bpp != 32) { MTOOLS_ERROR(std::string("readBFF() : Error, image format must be 32bit. (file [") + filename + "])"); }
				if (bff.size() < 276 + 4 * im_lx*im_ly) { MTOOLS_ERROR(std::string("readBFF() : Error, file [") + filename + " to small to contain the whole image."); }
				const uint8 char_offset = (uint8)bff[19];
				if ((uint32)(256 - char_offset) > nbx*nby) { MTOOLS_ERROR(std::string("readBFF() : Error, the image cannot contain all the glyphs (file [") + filename + "])"); }
				if (_fontsize <= 0) _fontsize = cell_ly; else if (_fontsize > cell_ly) { MTOOLS_ERROR(std::string("readBFF() : Error, the fontsize is larger than cell_ly (file [") + filename + "])"); }
				// file is ok. we load the image. 
				Image im(im_lx, im_ly);
				for (uint32 j = 0; j < im_ly; j++)
					{
					for (uint32 i = 0; i < im_lx; i++)
						{
						uint8 A = bff[276 + 4 * (i + j*im_lx) + 3];
						if (A >= 0xFD) { A = 0xFF; }
						else if (A <= 0x02) { A = 0x00; }
						im(i, j) = RGBc(0, 0, 0, A);
						}
					}
				// we construct the glyphs. 			
				_tab.resize(256);
				for (int k = 0; k < 256 - char_offset; k++)
					{
					const int c = k + char_offset;
					const int i = k % nbx;
					const int j = k / nbx;
					_tab[c].width = (uint8)bff[20 + c];
					_tab[c].glyph = im.sub_image(i*cell_lx, j*cell_ly, cell_lx, cell_ly);
					_trim(c);
					}
				}


			/**
			* Constructor. Deserialization from an archive.
			*
			* @param [in,out]	ar	The archive.
			**/
			Font(IBaseArchive & ar) : _fontsize(0), _tab()
				{
				serialize(ar);
				}


			/**
			* Constructor. Create the font by rescaling another one.
			*
			* @param	ft			the font to copy.
			* @param	fontsize	the font size.
			**/
			Font(const Font & ft, int fontsize) : _fontsize(0), _tab()
				{
				createFrom(ft, fontsize);
				}


			/**
			* Move constructor.
			**/
			Font(Font && ft) : _fontsize(ft._fontsize), _tab(std::move(ft._tab))
				{
				ft._fontsize = 0;
				ft._tab.clear();
				}


			/**
			* Copy constructor (shallow !)
			**/
			Font(const Font & ft) : _fontsize(ft._fontsize), _tab(ft._tab)
				{
				}


			/**
			* Move assignment operator.
			**/
			Font & operator=(Font && ft)
				{
				if (this != &ft)
					{
					_tab.clear();
					_fontsize = ft._fontsize;
					_tab = std::move(ft._tab);
					ft._fontsize = 0;
					ft._tab.clear();
					}
				return(*this);
				}


			/**
			* Assignment operator (shallow !)
			**/
			Font & operator=(const Font & ft)
				{
				if (this != &ft)
					{
					_fontsize = ft._fontsize;
					_tab = ft._tab;
					}
				return(*this);
				}


			/**
			* Create the font by rescaling another one.
			* Discard the current font if any.
			*
			* @param	ft			the font to copy.
			* @param	fontsize	the font size.
			**/
			void createFrom(const Font & ft, int fontsize)
				{
				_tab.clear();
				_fontsize = (fontsize <= 0) ? 0 : fontsize;
				if ((_fontsize <= 0) || (ft._fontsize <= 0)) return;
				double scale = ((double)_fontsize) / ((double)ft._fontsize);
				_tab.resize(256);
				for (int c = 0; c < 256; c++)
					{
					_tab[c].width = (int64)(ft._tab[c].width * scale + 0.5);
					if (ft._tab[c].glyph.isEmpty()) { _tab[c].glyph.empty(); _tab[c].offx = 0; _tab[c].offy = 0; }
					else
						{
						_tab[c].glyph = ft._tab[c].glyph;
						_tab[c].offx = ft._tab[c].offx;
						_tab[c].offy = ft._tab[c].offy;
						_untrim(c, (int)ft._fontsize);
						_tab[c].glyph.rescale(10, _fontsize, _fontsize);
						_trim(c);
						}
					}
				}



			/**
			 * Query if the font is empty.
			 **/
			bool isEmpty() const
				{
				return (_fontsize == 0);
				}



			/**
			* Query the size of the font.
			* Return 0 if the font is empty.
			**/
			int fontsize() const { return (int)_fontsize; }


			/**
			* Return a glyph of the font.
			**/
			const Glyph glyph(char c) const { return ((_fontsize) ? (_tab[c]) : (Glyph())); }


			/**
			* Return the size of the bounding box when drawing text txt with this font.
			**/
			iVec2 textDimension(const std::string & txt) const { return _textDimension(txt); }


			/**
			* Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			* image.
			*
			* @param [in,out]	im	The image to draw onto.
			* @param	x		  	x coordinate of the text reference position.
			* @param	y		  	y coordinate of the text reference position.
			* @param	txt		  	the text.
			* @param	txt_pos   	Positioning method (combination of XCENTER, LEFT, RIGHT, TOP, BOTTOM,
			* 						YCENTER).
			* @param	bkcolor   	The color to blend over.
			**/
			void drawBackground(Image & im, int64 x, int64 y, const std::string & txt, int txt_pos, RGBc bkcolor) const
				{
				drawBackground(im, { x,y }, txt, txt_pos, bkcolor);
				}


			/**
			* Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			* image.
			*
			* @param [in,out]	im	The image to draw onto.
			* @param	pos		  	reference position.
			* @param	txt		  	the text.
			* @param	txt_pos   	Positioning method (combination of XCENTER, LEFT, RIGHT, TOP, BOTTOM,
			* 						YCENTER).
			* @param	bkcolor   	The color to blend over.
			**/
			void drawBackground(Image & im, iVec2 pos, const std::string & txt, int txt_pos, RGBc bkcolor) const
				{
				if ((!_fontsize) || (txt.size() == 0)) return;
				iVec2 dim = textDimension(txt);
				pos = _upperleft(pos, dim, txt_pos);
				im.draw_filled_rectangle(pos.X(), pos.Y(), dim.X(), dim.Y(), bkcolor);
				}


			/**
			* Draws a text on an image using this font, with a given color.
			*
			* @param [in,out]	im	The image to draw onto.
			* @param	pos		  	reference position.
			* @param	txt		  	The text to draw.
			* @param	txt_pos   	Positioning method (combination of XCENTER, LEFT, RIGHT, TOP, BOTTOM,
			* 						YCENTER).
			* @param	color	  	The color to use for drawing the font.
			**/
			void drawText(Image & im, const iVec2 & pos, const std::string & txt, int txt_pos, RGBc color) const
				{
				if ((!_fontsize) || (txt.size() == 0)) return;
				auto npos = _upperleft(pos, txt, txt_pos);
				int64 x = npos.X();
				int64 y = npos.Y();
				int64 x0 = x;
				for (size_t i = 0; i < txt.size(); i++)
					{
					const char c = txt[i];
					if (c == '\n') { x = x0; y += _fontsize; }
					else if (c == '\t') { x += 4 * _tab[' '].width; }
					else if (c >= 32)
						{
						im.mask(_tab[c].glyph, x + _tab[c].offx, y + _tab[c].offy, color);
						x += _tab[c].width;
						}
					}
				return;
				}


			/**
			* Draws a text on an image using this font, with a given color.
			*
			* @param [in,out]	im	The image to draw onto.
			* @param	x		  	x coordinate of the text reference position.
			* @param	y		  	y coordinate of the text reference position.
			* @param	txt		  	The text to draw.
			* @param	txt_pos   	Positioning method (combination of XCENTER, LEFT, RIGHT, TOP, BOTTOM,
			* 						YCENTER).
			* @param	color	  	The color to use for drawing the font.
			**/
			void drawText(Image & im, int64 x, int64 y, const std::string & txt, int txt_pos, RGBc color) const
				{
				drawText(im, { x,y }, txt, txt_pos, color);
				}


			/**
			* serialize/deserialize the font.
			**/
			template<typename ARCHIVE> void serialize(ARCHIVE & ar)
				{
				ar & _fontsize;
				ar & _tab;
				}




			/* text positioning constants */
			static const int XCENTER = 0;
			static const int LEFT = 1;
			static const int RIGHT = 2;
			static const int YCENTER = 0;
			static const int TOP = 4;
			static const int BOTTOM = 8;
			static const int TOPLEFT = TOP | LEFT;
			static const int TOPRIGHT = TOP | RIGHT;
			static const int BOTTOMLEFT = BOTTOM | LEFT;
			static const int BOTTOMRIGHT = BOTTOM | RIGHT;
			static const int CENTER = XCENTER | YCENTER;
			static const int CENTERLEFT = YCENTER | LEFT;
			static const int CENTERRIGHT = YCENTER | RIGHT;
			static const int CENTERTOP = XCENTER | TOP;
			static const int CENTERBOTTOM = XCENTER | BOTTOM;


		private:

			friend class FontFamily;

			/**
			* Empty the font (can only be used by a friend class)
			**/
			void empty()
				{
				_fontsize = 0;
				_tab.clear();
				}


			/* trim the glyph image and set glyph.offx and glyph.offy values */
			void _trim(int c)
				{
				Glyph & G = _tab[c];
				if (G.glyph.isEmpty()) return;
				iBox2 B = G.glyph.minBoundingBox();
				if (B.isEmpty()) { G.glyph.empty(); G.offx = 0; G.offy = 0; return; }
				G.offx = B.min[0];
				G.offy = B.min[1];
				G.glyph.crop(B, false); // make independent copy.
				}


			/* reverse operation of the trim operation, make the glyph size fontsize x fontsize */
			void _untrim(int c, int fontsize)
				{
				Glyph & G = _tab[c];
				if (G.glyph.isEmpty()) return;
				Image im(fontsize, fontsize, RGBc(0, 0, 0, 0));
				im.blit(G.glyph, G.offx, G.offy);
				_tab[c].glyph = im;
				}


			/* compute the position of upper left corner of the text box depending on the positionning method. */
			iVec2 _upperleft(iVec2 pos, const iVec2 & txtdim, int txt_pos) const
				{
				switch (txt_pos & 3)
					{
					case 0: { pos.X() -= txtdim.X() / 2; break;}
					case 2: { pos.X() -= txtdim.X() - 1; break; }
					case 3: { pos.X() -= txtdim.X() / 2; break; }
					}
				switch ((txt_pos >> 2) & 3)
					{
					case 0: { pos.Y() -= txtdim.Y() / 2; break; }
					case 2: { pos.Y() -= txtdim.Y() - 1; break; }
					case 3: { pos.Y() -= txtdim.Y() / 2; break; }
					}
				return pos;
				}


			/* compute the position of upper left corner of the text box depending on the positionning method. */
			iVec2 _upperleft(iVec2 pos, const std::string & txt, int txt_pos) const
				{
				if (((txt_pos & 3) == 1) && (((txt_pos >> 2) & 3) == 1)) { return pos; } //nothing to compute
				return _upperleft(pos, _textDimension(txt), txt_pos);
				}


			/* Return the size of the bounding box when drawing text txt with this font. */
			iVec2 _textDimension(const std::string & txt) const
				{
				if ((!_fontsize) || (txt.size() == 0)) return iVec2(0, 0);
				int64 mx = 0, x = 0;
				int64 my = _fontsize;
				for (size_t i = 0; i < txt.size(); i++)
					{
					const char c = txt[i];
					if (c >= 32) { x += _tab[c].width; }
					else if (c == '\t') { x += 4 * _tab[' '].width; }
					else if (c == '\n') { if (x > mx) { mx = x; } x = 0;  my += _fontsize; }
					}
				if (x > mx) { mx = x; }
				return iVec2(mx, my);
				}


			int64 _fontsize;			// size of the font
			std::vector<Glyph> _tab;	// vector containing the glyphs. 

		};




		/* method to choose a font */
		enum 
			{
			MTOOLS_EXACT_FONT = 0,
		    MTOOLS_NATIVE_FONT_BELOW = 1,
		    MTOOLS_NATIVE_FONT_ABOVE = 2
			};



		class FontFamily
			{

			public:


			/** Default constructor. */
			FontFamily() { _empty(); }


			/**
			 * Constructor. Deserialize the object.
			 **/
			FontFamily(IBaseArchive & ar) { _empty(); deserialize(ar); }


			/**
			 * Serialization of the object.
			 **/
			void serialize(OBaseArchive & ar)
				{
				ar & _nativeset.size();
				for (auto it = _nativeset.begin(); it != _nativeset.end(); it++)
					{
					ar & (*it); 
					ar & _fonts[*it];
					}
				}


			/**
			* Deserialization of the object.
			**/
			void deserialize(IBaseArchive & ar)
				{
				_empty();
				size_t l; ar & l;
				for (int i = 0;i < l; i++)
					{
					int fs; ar & fs;
					_nativeset.insert(fs);
					ar & _fonts[fs];
					}
				}


			/**
			 * Inserts a font. If a font with the same fontsize already exists. It is replaced.
			 *
			 * @param	font	The font to insert.
			 **/
			void insertFont(const Font & font)
				{
				const int size = font.fontsize();
				if ((size <= 0)||(size >= MAX_FONT_SIZE)) return; 
				_fonts[size] = font;
				_nativeset.insert(size);
				}


			/**
			 * Query if the font with a given size is a native one. 
			 *
			 * @param	fontsize	The font size.
			 *
			 * @return	true if native, false if it is rescaled from another font. 
			 **/
			bool isNative(int fontsize)
				{
				return (_nativeset.find(fontsize) == _nativeset.end());
				}


			/**
			 * Return a font with a given fontsize.
			 *
			 * @param	fontsize	the requested size of the font.
			 * @param	method  	method to choose the font in case no native font matches this size. One
			 * 						of MTOOLS_EXACT_FONT, MTOOLS_NATIVE_FONT_BELOW, MTOOLS_NATIVE_FONT_ABOVE.
			 *
			 * @return	A reference to the matching font. If fontsize is negative or larger than
			 * 			MAX_FONT_SIZE, return an empty font.
			 **/
			const Font & operator()(int fontsize, int  method = MTOOLS_NATIVE_FONT_BELOW)
				{
				if ((fontsize <= 0) || (fontsize > MAX_FONT_SIZE) || (_nativeset.size() == 0)) return _fonts[0];
				if (method == MTOOLS_EXACT_FONT)
					{
					if (_fonts[fontsize].isEmpty()) { _constructFont(fontsize); }
					return _fonts[fontsize];
					}
				auto it = _nativeset.lower_bound(fontsize); 
				if (it == _nativeset.end()) { return _fonts[*(_nativeset.rbegin())]; } // past the last element, return the largest. 
				if (method = MTOOLS_NATIVE_FONT_ABOVE) { return _fonts[*it]; }
				if (it == _nativeset.begin()) { return _fonts[*it]; } // return the smallest available
				it--;
				return _fonts[*it];
				}


			private:

				static const int MAX_FONT_SIZE = 4096;


				/* Empty the object. */
				void _empty()
					{
					_nativeset.clear();
					_fonts.clear();
					_fonts.resize(MAX_FONT_SIZE);
					}


				/* construct the font using the smallest larger font */ 
				void _constructFont(int fontsize)
					{
					auto it = _nativeset.lower_bound(fontsize);
					if (it == _nativeset.end()) 
						{ // past the largest one. 
						_fonts[fontsize].createFrom(_fonts[*(_nativeset.rbegin())], fontsize);
						return; 
						}  
					_fonts[fontsize].createFrom(_fonts[*it], fontsize);
					}


			std::set<int>		_nativeset;				// set that keep tracks of native fonts
			std::vector<Font>	_fonts;					// vector of fonts. 

			};



		
		/**
		* Return the default font with a given size.
		*
		* @param	fontsize	the requested size of the font.
		* @param	method  	method to choose the font in case no native font matches this size. One
		* 						of MTOOLS_EXACT_FONT, MTOOLS_NATIVE_FONT_BELOW, MTOOLS_NATIVE_FONT_ABOVE.
		*
		* @return	A reference to the matching font. If fontsize is negative or larger than
		* 			MAX_FONT_SIZE, return an empty font.
		**/
		const Font & defaultFont(int fontsize, int  method = MTOOLS_NATIVE_FONT_BELOW);




	}

/* end of file */

