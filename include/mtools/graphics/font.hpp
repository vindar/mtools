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



	/* text positioning constants */
	static const int MTOOLS_TEXT_XCENTER		= 0;
	static const int MTOOLS_TEXT_LEFT			= 1;
	static const int MTOOLS_TEXT_RIGHT			= 2;
	static const int MTOOLS_TEXT_YCENTER		= 0;
	static const int MTOOLS_TEXT_TOP			= 4;
	static const int MTOOLS_TEXT_BOTTOM			= 8;
	static const int MTOOLS_TEXT_TOPLEFT		= MTOOLS_TEXT_TOP | MTOOLS_TEXT_LEFT;
	static const int MTOOLS_TEXT_TOPRIGHT		= MTOOLS_TEXT_TOP | MTOOLS_TEXT_RIGHT;
	static const int MTOOLS_TEXT_BOTTOMLEFT		= MTOOLS_TEXT_BOTTOM | MTOOLS_TEXT_LEFT;
	static const int MTOOLS_TEXT_BOTTOMRIGHT	= MTOOLS_TEXT_BOTTOM | MTOOLS_TEXT_RIGHT;
	static const int MTOOLS_TEXT_CENTER			= MTOOLS_TEXT_XCENTER | MTOOLS_TEXT_YCENTER;
	static const int MTOOLS_TEXT_CENTERLEFT		= MTOOLS_TEXT_YCENTER | MTOOLS_TEXT_LEFT;
	static const int MTOOLS_TEXT_CENTERRIGHT	= MTOOLS_TEXT_YCENTER | MTOOLS_TEXT_RIGHT;
	static const int MTOOLS_TEXT_CENTERTOP		= MTOOLS_TEXT_XCENTER | MTOOLS_TEXT_TOP;
	static const int MTOOLS_TEXT_CENTERBOTTOM	= MTOOLS_TEXT_XCENTER | MTOOLS_TEXT_BOTTOM;



	/* method to choose a font */
	enum
		{
		MTOOLS_EXACT_FONT = 0,
		MTOOLS_NATIVE_FONT_BELOW = 1,
		MTOOLS_NATIVE_FONT_ABOVE = 2
		};



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
			Font(const std::string & filename, int fontsize = 0);


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
			void createFrom(const Font & ft, int fontsize);


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
			 * @param	txt_pos   	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM,
			 * 						MTOOLS_TEXT_YCENTER).
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
			* @param	txt_pos   	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT, MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM,
			* 						MTOOLS_TEXT_YCENTER).
			* @param	bkcolor   	The color to blend over.
			**/
			void drawBackground(Image & im, iVec2 pos, const std::string & txt, int txt_pos, RGBc bkcolor) const
				{
				if ((!_fontsize) || (txt.size() == 0)) return;
				iVec2 dim = textDimension(txt);
				pos = _upperleft(pos, dim, txt_pos);
				im.draw_box(iBox2(pos.X(), pos.X() + dim.X() - 1, pos.Y(), pos.Y() + dim.Y() - 1), bkcolor,true);
				}


			/**
			* Draws a text on an image using this font, with a given color.
			*
			* @param [in,out]	im	The image to draw onto.
			* @param	pos		  	reference position.
			* @param	txt		  	The text to draw.
			* @param	txt_pos   	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT, MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM,
			* 						MTOOLS_TEXT_YCENTER).
			* @param	color	  	The color to use for drawing the font.
			**/
			void drawText(Image & im, const iVec2 & pos, const std::string & txt, int txt_pos, RGBc color) const;


			/**
			* Draws a text on an image using this font, with a given color.
			*
			* @param [in,out]	im	The image to draw onto.
			* @param	x		  	x coordinate of the text reference position.
			* @param	y		  	y coordinate of the text reference position.
			* @param	txt		  	The text to draw.
			* @param	txt_pos   	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT, MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM,
			* 						MTOOLS_TEXT_YCENTER).
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



		private:

			friend class FontFamily;

			/** Empty the font (only for friend class) **/
			void empty() { _fontsize = 0; _tab.clear(); }

			/* trim the glyph image and set glyph.offx and glyph.offy values */
			void _trim(int c);

			/* reverse operation of the trim operation, make the glyph size fontsize x fontsize */
			void _untrim(int c, int fontsize);

			/* compute the position of upper left corner of the text box depending on the positionning method. */
			iVec2 _upperleft(iVec2 pos, const iVec2 & txtdim, int txt_pos) const;

			/* compute the position of upper left corner of the text box depending on the positionning method. */
			iVec2 _upperleft(iVec2 pos, const std::string & txt, int txt_pos) const;

			/* Return the size of the bounding box when drawing text txt with this font. */
			iVec2 _textDimension(const std::string & txt) const;



			int64 _fontsize;			// size of the font
			std::vector<Glyph> _tab;	// vector containing the glyphs. 

		};







		/**
		 * Font Family class.
		 * 
		 * This object encapsdulated a font family i.e. different version of a given font different font size. 
		 **/
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
			void serialize(OBaseArchive & ar) const
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
				std::lock_guard<std::mutex> lock(_mut); // mutex lock for concurrent access. 
				size_t l; ar & l;
				for (size_t i = 0; i < l; i++)
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
				std::lock_guard<std::mutex> lock(_mut); // mutex lock for concurrent access. 
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
			 * @return	A reference to the matching font. If fontsize is negative return the empty font with
			 * 			size 0 and if larger than MAX_FONT_SIZE, return the largest font matching the
			 * 			criteria.
			 **/
			inline const Font & operator()(int fontsize, int  method = MTOOLS_NATIVE_FONT_BELOW)
				{
				int fs = nearestSize(fontsize, method);
				if (_fonts[fontsize].isEmpty()) { _constructFont(fs); }
				return _fonts[fs];
				}


			/**
			 * Return the size of the nearest font matching the method criteria. This is the size of the
			 * font returned when calling operator() with the same arguments.
			 *
			 * @param	fontsize	the requested size of the font.
			 * @param	method  	method to choose the font in case no native font matches this size. One
			 * 						of MTOOLS_EXACT_FONT, MTOOLS_NATIVE_FONT_BELOW, MTOOLS_NATIVE_FONT_ABOVE.
			 *
			 * @return	The corresponding size.
			 **/
			int nearestSize(int fontsize, int  method = MTOOLS_NATIVE_FONT_BELOW) const;


			/**
			 * Return the set of font size which are 'native' i.e. that were not rescale from another size.
			 **/
			std::set<int> nativeSizeSet() const { return _nativeset; }


			static const int MAX_FONT_SIZE = 4096;  ///< maximum font size. 

		private:


			/* Empty the object. */
			void _empty();

			/* construct the font using the smallest larger font */ 
			void _constructFont(int fontsize);


			std::set<int>		_nativeset;				// set that keep tracks of native fonts
			std::vector<Font>	_fonts;					// vector of fonts. 
			std::mutex			_mut;					// mutex for mutlithread access to global font objects. 
		};



		
		/**
		* Return the global font with a given size.
		*
		* @param	fontsize	the requested size of the font.
		* @param	method  	method to choose the font in case no native font matches this size. One
		* 						of MTOOLS_EXACT_FONT, MTOOLS_NATIVE_FONT_BELOW, MTOOLS_NATIVE_FONT_ABOVE.
		*
		* @return	A reference to the matching font. If fontsize is negative or larger than
		* 			MAX_FONT_SIZE, return an empty font.
		**/
		const Font & gFont(int fontsize, int  method = MTOOLS_NATIVE_FONT_BELOW);


		/**
		 * Compute the global font required size in order to adjust a text to a given box.
		 * 
		 * Set boxsize.X() (resp boxsize.Y() ) to negative value for removing a constrain on X (resp Y).
		 * Use minheight and maxheight for addtionnal constrained on the size of the font.
		 *
		 * @param	text	 	The text.
		 * @param	boxsize  	the size of the box.
		 * @param	method   	method to choose the font in case no native font matches this size. One
		 * 						of MTOOLS_EXACT_FONT, MTOOLS_NATIVE_FONT_BELOW, MTOOLS_NATIVE_FONT_ABOVE.
		 * @param	minheight	The optional minimum font height requested.
		 * @param	maxheight	The optional maximum font height requsted.
		 *
		 * @return	the font height.
		 **/
		int gFontFindSize(const std::string & text, mtools::iVec2 boxsize, int  method = MTOOLS_NATIVE_FONT_BELOW, int minheight = 0, int maxheight = FontFamily::MAX_FONT_SIZE);


	}

/* end of file */

