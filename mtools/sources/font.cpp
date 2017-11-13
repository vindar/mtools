/** @file font.cpp */
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

#include "stdafx_mtools.h"

#include "graphics/font.hpp"


namespace mtools
	{



	Font::Font(const std::string & filename, int fontsize) : _fontsize(fontsize), _tab()
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


	void Font::createFrom(const Font & ft, int fontsize)
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


	void Font::drawText(Image & im, const iVec2 & pos, const std::string & txt, int txt_pos, RGBc color) const
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


	void Font::_trim(int c)
		{
		Glyph & G = _tab[c];
		if (G.glyph.isEmpty()) return;
		iBox2 B = G.glyph.minBoundingBox();
		if (B.isEmpty()) { G.glyph.empty(); G.offx = 0; G.offy = 0; return; }
		G.offx = B.min[0];
		G.offy = B.min[1];
		G.glyph.crop(B, false); // make independent copy.
		}


	void Font::_untrim(int c, int fontsize)
		{
		Glyph & G = _tab[c];
		if (G.glyph.isEmpty()) return;
		Image im(fontsize, fontsize, RGBc(0, 0, 0, 0));
		im.blit(G.glyph, G.offx, G.offy);
		_tab[c].glyph = im;
		}


	iVec2 Font::_upperleft(iVec2 pos, const iVec2 & txtdim, int txt_pos) const
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


	iVec2 Font::_upperleft(iVec2 pos, const std::string & txt, int txt_pos) const
		{
		if (((txt_pos & 3) == 1) && (((txt_pos >> 2) & 3) == 1)) { return pos; } //nothing to compute
		return _upperleft(pos, _textDimension(txt), txt_pos);
		}


	iVec2 Font::_textDimension(const std::string & txt) const
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






	int FontFamily::nearestSize(int fontsize, int  method) const 
		{
		if (method == MTOOLS_EXACT_FONT)
			{
			if (fontsize <= 0) return 0;
			if (fontsize >= MAX_FONT_SIZE) return MAX_FONT_SIZE;
			return fontsize;
			}
		if (_nativeset.size() == 0) return 0;
		auto it = _nativeset.lower_bound(fontsize);
		if (it == _nativeset.end()) { return *(_nativeset.rbegin()); } // past the last element, return the largest. 
		if ((method = MTOOLS_NATIVE_FONT_ABOVE)||(it == _nativeset.begin())) { return *it; }
		return *(--it);
		}


	void FontFamily::_empty()
		{
		std::lock_guard<std::mutex> lock(_mut); // mutex lock for concurrent access. 
		_nativeset.clear();
		_fonts.clear();
		_fonts.resize(MAX_FONT_SIZE + 1);
		}


	void FontFamily::_constructFont(int fontsize)
		{
		if (fontsize == 0) return;
		std::lock_guard<std::mutex> lock(_mut); // mutex lock for concurrent access. 
		if (!(_fonts[fontsize].isEmpty())) return; // already created, nothing to do.
		auto it = _nativeset.lower_bound(fontsize);
		if (it == _nativeset.end())
			{ // past the largest one. 
			_fonts[fontsize].createFrom(_fonts[*(_nativeset.rbegin())], fontsize);
			return;
			}
		_fonts[fontsize].createFrom(_fonts[*it], fontsize);
		}



	namespace internals_font
		{

		/* the buffer containing the global font data */
		extern const p_char OPEN_SANS_FONT_DATA[9680];

		/* pointer to the global font family object */
		FontFamily * _gfont = nullptr;

		/* make sure _gfont is created */ 
		inline void _creategFont()
			{
			static std::mutex mut;
			if (internals_font::_gfont == nullptr)
				{
				std::lock_guard<std::mutex> lock(mut); // mutex lock for concurrent access. 
				if (internals_font::_gfont == nullptr) internals_font::_gfont = new FontFamily(ICPPArchive(internals_font::OPEN_SANS_FONT_DATA));
				}
			}

		}


	const Font & gFont(int fontsize, int  method)
		{
		internals_font::_creategFont();
		return internals_font::_gfont->operator()(fontsize,method);
		}



	int gFontFindSize(const std::string & text, mtools::iVec2 boxsize, int  method, int minheight, int maxheight)
		{
		internals_font::_creategFont();
		if (minheight < 0) minheight = 0;
		if (maxheight > FontFamily::MAX_FONT_SIZE) maxheight = FontFamily::MAX_FONT_SIZE;
		MTOOLS_INSURE(minheight <= maxheight);

		if ((text.length() == 0) || ((boxsize.X()<0) && (boxsize.Y()<0)))  return internals_font::_gfont->nearestSize(maxheight, method);
		if ((boxsize.Y() >= 0) && (boxsize.Y()<maxheight)) { maxheight = (int)boxsize.Y(); }
		mtools::iVec2 TS = gFont(maxheight, method).textDimension(text);
		if (((boxsize.X() < 0) || (TS.X() <= boxsize.X())) && ((boxsize.Y()<0) || (TS.Y() <= boxsize.Y()))) return internals_font::_gfont->nearestSize(maxheight, method);
		TS = gFont(minheight, method).textDimension(text);
		if  (((boxsize.X() >= 0) && (TS.X() > boxsize.X())) || ((boxsize.Y() >= 0) && (TS.Y() > boxsize.Y()))) return internals_font::_gfont->nearestSize(minheight, method);
		while (maxheight - minheight >1) 
			{
			unsigned int f = (maxheight + minheight) / 2;
			TS = gFont(f, method).textDimension(text);
			if (((boxsize.X()<0) || (TS.X() <= boxsize.X())) && ((boxsize.Y()<0) || (TS.Y() <= boxsize.Y()))) { minheight = f; }
			else { maxheight = f; }
			}
		return internals_font::_gfont->nearestSize(minheight, method);
		}



	}

/* end of file */

