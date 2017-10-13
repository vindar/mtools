#include "stdafx_test.h"

#include "mtools.hpp"


using namespace mtools;





/** Structure containing a glyph image. */
struct Glyph
	{
	Glyph() : offx(0), offy(0), width(0), glyph() {}
	int64 offx;
	int64 offy;
	int64 width;
	Image glyph;
	};



Image IA;


class Font
	{

	public:

		/** Default constructor. Empty font */
		Font() : _fontsize(0), _tab() {}



		/**
		 * Constructor from a .BFF file.
		 *
		 * @param	filename	name of the .bff file.
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
			if (!_fontsize) _fontsize = cell_ly; else if (_fontsize > cell_ly) { MTOOLS_ERROR(std::string("readBFF() : Error, the fontsize is larger than cell_ly (file [") + filename + "])"); }
			// file is ok. we load the image. 
			Image im(im_lx, im_ly);
			for (uint32 j = 0; j < im_ly; j++)
				{
				for (uint32 i = 0; i < im_lx; i++)
					{
					uint8 A = bff[276 + 4 * (i + j*im_lx) + 3];
					if (A >= 0xFD) { A = 0xFF; } else if (A <= 0x02) { A = 0x00; }
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
			serialize<IBaseArchive>(ar);
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
		 * Return a new font obtain by rescaling this font to a given size.
		 *
		 * @param	newfontsize	The new font size.
		 *
		 * @return	The rescaled font.
		 **/
		Font rescale(int newfontsize)
			{
			return Font();
			}


		/**
		* Query the size of the font.
		**/
		int fontsize() const { return (int)_fontsize; }


		/**
		 * Return a given glyph of the font.
		 **/
		const Glyph glyph(char c) const { return ( (_fontsize) ? (_tab[c]) : (Glyph())); }


		/**
		* Return the size of the bounding box when drawing text txt with this font.
		**/
		iVec2 textDimension(const std::string & txt) const
			{

			}



		/* text positioning constants */
		static const int XCENTER = 0;
		static const int LEFT = 1;
		static const int RIGHT = 2;
		static const int YCENTER = 0;
		static const int TOP = 4;
		static const int BOTTOM = 8;


		/**
		 * Draws a text on an image using this font, with a given color.
		 *
		 * @param [in,out]	im	   	The image to draw onto.
		 * @param	x			   	x coordinate to put the text
		 * @param	y			   	y coordinate to put the text
		 * @param	txt			   	The text to draw.
		 * @param	txt_pos		   	Positioning of the text (combination of XCENTER, LEFT, RIGHT, TOP,
		 * 							BOTTOM, YCENTER).
		 * @param	color		   	The color to use for drawing the font.
		 * @param	clearbackground	true to clear the background of the image below the text prior to
		 * 							drawing it.
		 * @param	bkcolor		   	The color to use when clearing the background.
		 **/
		void draw(Image & im, int64 x, int64 y, const std::string & txt, int txt_pos, RGBc color, bool clearbackground = false, RGBc bkcolor = RGBc::c_White) const
			{
			if (!_fontsize) return;
			int64 x0 = x;
			for (size_t i = 0; i < txt.size(); i++)
				{
				const char c = txt[i];
				if (c == '\n') { x = x0; y += _fontsize; }
				else if (c == '\t') { x += 4 * _tab[' '].width;  }
				else if (c >= 32) 
					{ 
					im.mask(_tab[c].glyph, x + _tab[c].offx, y + _tab[c].offy, color); x += _tab[c].width;  
					}
				}
			return;
			}



		/**
		* serialize/deserialize the font.
		**/
		template<typename U> void serialize(U & ar, const int version = 0)
			{
			ar & _fontsize;
			ar & _tab;
			}


	private:



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
		void _untrim(int c)
			{
			MTOOLS_INSURE(_fontsize);
			Glyph & G = _tab[c];
			Image im(_fontsize, _fontsize, RGBc(0, 0, 0, 0));
			im.blit(G.glyph, G.offx, G.offy);
			}


		/* no copy */
		Font(const Font &) = delete;
		Font & operator=(const Font &) = delete;


		int64 _fontsize;			// size of the font
		std::vector<Glyph> _tab;	// vector containg the glyphs. 

	};






using namespace mtools;

Image im;


RGBc fimg(mtools::int64 x, mtools::int64 y)
	{
	if ((x < im.lx()) && (x >= 0) && (y < im.ly()) && (y >= 0))
		{
		return im((int32)x, im.ly() - 1 - (int32)y);
		}
	return RGBc::c_Cyan;
	}


typedef char * pchar;



void testImg()
	{


	int64 x = 0;
	int64 y = 400;


	Font F("SUI3.bff");



	std::string txt("The brown fox jumps over the lazy dog\nYEAH!!!!\nThat's nice! Here is a number: 1.2345678999e-678");

	im.load_png("lenna.png");
	F.draw(im, x, y, txt, 0, RGBc::c_White.getOpacity(1));

	/*
	{
	cimg_library::CImg<unsigned char> im1("lenna.png");

	cimg_library::CImg<unsigned char>  im2(10000, 3000, 1, 3);

	Chronometer();
	for (int i = 0;i < 100; i++)
	{
	im2 = im1.get_resize(10000, 3000, 1, 3, 1);
	//im.blit_rescaled(im.UPSCALE_FASTEST, im.DOWNSCALE_FASTEST, im1, 0, 0, 10000, 3000);
	}

	cout << "Elapsed CIMG = " << Chronometer() << "\n";

	}
	*/


	/*
	Image im2(10000, 5000, RGBc::c_Blue);

	MT2004_64 gen(0);

	int N = 100;

	Chronometer();

	int64 x1 = 10000 * Unif(gen);
	int64 y1 = 5000 * Unif(gen);

	for (int i = 0;i < N; i++)
	{
	x1 = (x1 + 7 ) % 10000;
	y1 = (y1 + 13)  % 5000;

	//im2.blitSprite(im1, x1, y1);

	im2.clear(RGBc::c_Red);
	}
	*/




	/*
	Chronometer();
	Image im1("lenna.png");


	for (int hx = 0;hx < im1.lx(); hx++)
	{
	for (int hy = 0; hy < im1.ly(); hy++)
	{
	im1(hx, hy).comp.A = im1(hx, hy).comp.R;
	}
	}

	cout << "Image loaded = " << Chronometer() << "\n";

	int lx = im1.lx() * 2;
	int ly = im1.ly();

	im = Image(lx, ly);
	im.clear(RGBc::c_White);

	cout.getKey();

	int N = 0;

	Chronometer();
	for (int i = 0; i <= N; i++)
	{
	im.mask(im1, i * 5, 0, opacity(RGBc::c_Red,1));
	}

	cout << "done in " << Chronometer() << "\n";
	*/

	//PixelDrawer

	/*
	{
	Chronometer();
	cimg_library::CImg<unsigned char> im1("world.png");
	cout << "CIMG loaded = " << Chronometer() << "\n";
	cimg_library::CImg<unsigned char> im2;
	Chronometer();
	for (int i = 0;i < N; i++)
	{
	im2 = im1.get_resize(lx, ly, 1, 3, 2);
	}
	cout << "Elapsed CIMG = " << Chronometer() << "\n";

	for (int j = 0;j < im2.height(); j++)
	{
	for (int i = 0;i < im2.width(); i++)
	{
	im(lx + (lx - 1 - i), ly-1-j) = RGBc(im2(i, j, 0, 0), im2(i, j, 0, 1), im2(i, j, 0, 2));
	}
	}
	}
	*/
	/*
	Chronometer();
	for (int i = 0;i < N;i++)
	{
	im.blit_rescaled(0, 0, im1, lx + 10, 0, lx, ly);
	}
	cout << "done = " << Chronometer() << "\n";
	*/



	mtools::Plotter2D plotter;
	auto P1 = mtools::makePlot2DImage(im, 4, "Img");
	plotter[P1];
	plotter.plot();


	return;
	mtools::cout << "Hello !\n";
	mtools::cout.getKey();
	}




int main(int argc, char *argv[])
	{
	MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc, argv, true); // parse the command line, interactive mode



	testImg();


	cout << "Hello World\n";
	cout.getKey();

	return 0;
	}

/* end of file main.cpp */

